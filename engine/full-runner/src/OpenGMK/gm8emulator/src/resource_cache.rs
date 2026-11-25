//! Read-only acceleration for the large battle-sprite directory. No persisted
//! index, no sprite selection, no shared GPU handles. Other paths use native IO.
#[cfg(all(target_os = "horizon", not(test)))]
use crate::battle_trace::{self, CacheAccess};
use std::{collections::{HashMap,HashSet},io,path::{Path,PathBuf},sync::{Arc,Mutex,OnceLock,atomic::{AtomicBool,Ordering}},thread,time::{Duration,Instant}};
const INDEX_LIMIT:usize=4*1024*1024;
const BYTE_LIMIT:usize=8*1024*1024;
const FILE_LIMIT:usize=1024*1024;
const ENTRY_LIMIT:usize=2048;
struct Index{names:HashSet<String>,charge:usize}
struct State{epoch:u64,index:Option<Index>,data:HashMap<String,(Arc<[u8]>,u64)>,bytes:usize,peak:usize,tick:u64,hits:u64,negative:u64,fallback:u64,read_hits:u64,reads:u64,invalidations:u64}
impl State{
 fn new()->Self{Self{epoch:u64::MAX,index:None,data:HashMap::new(),bytes:0,peak:0,tick:0,hits:0,negative:0,fallback:0,read_hits:0,reads:0,invalidations:0}}
 fn refresh(&mut self,epoch:u64){if self.epoch!=epoch{self.index=None;self.data.clear();self.bytes=0;self.epoch=epoch;self.invalidations+=1;}}
}
pub struct Cache{dir:PathBuf,epoch:Arc<dyn Fn()->u64+Send+Sync>,state:Mutex<State>,stop:AtomicBool,limit:usize}
impl Cache{
 pub fn new(dir:PathBuf,epoch:Arc<dyn Fn()->u64+Send+Sync>)->Self{Self{dir,epoch,state:Mutex::new(State::new()),stop:AtomicBool::new(false),limit:BYTE_LIMIT}}
 fn key(&self,path:&str)->Option<String>{
  let path=PathBuf::from(path.replace('\\',"/"));
  let path=if path.is_absolute() || path.to_string_lossy().starts_with("sdmc:"){path}else{std::env::current_dir().ok()?.join(path)};
  // Lexically resolve dot components without stat/canonicalize on the leaf.
  fn normalized(p:&Path)->String{let text=p.to_string_lossy().replace('\\',"/");let mut parts=Vec::new();for part in text.split('/') {match part{""|"."=>(),".."=>{parts.pop();},_=>parts.push(part)}}parts.join("/").to_ascii_lowercase()}
  let dir=normalized(&self.dir);let full=normalized(&path);let name=full.strip_prefix(&(dir+"/"))?;
  if name.contains('/')||!name.ends_with(".gmspr"){return None}Some(name.to_owned())
 }
 /// A single background pass. Never publish a partial/error/over-budget index.
 pub fn build_index(&self)->io::Result<bool>{
  let epoch=(self.epoch)();if epoch&1!=0{return Ok(false)}
  let start=Instant::now();let mut names=HashSet::new();let mut charge=0;
  for (i,entry) in std::fs::read_dir(&self.dir)?.enumerate(){
   if self.stop.load(Ordering::Relaxed)||(self.epoch)()!=epoch{return Ok(false)}
   if start.elapsed()>Duration::from_secs(10){eprintln!("CACHE05 index_aborted reason=time_budget native_fallback=true");return Ok(false)}
   let entry=entry?;let name=entry.file_name().to_string_lossy().into_owned();
   if name.to_ascii_lowercase().ends_with(".gmspr"){
    let kind=entry.file_type()?;
    // Symlinks/unknown entry types require native behavior, not a false negative.
    if kind.is_symlink(){
      #[cfg(target_os = "horizon")]
      eprintln!("BATTLE11 index result=symlink epoch={epoch}");
      return Ok(false)
    }
    if kind.is_file(){charge+=name.len()+96;if charge>INDEX_LIMIT{eprintln!("CACHE05 index_aborted reason=memory_budget native_fallback=true");return Ok(false)}names.insert(name.to_ascii_lowercase());}
   }
   if i%128==0{thread::yield_now();}
  }
  let mut s=self.state.lock().unwrap();if (self.epoch)()!=epoch{return Ok(false)}s.refresh(epoch);
  let count=names.len();s.index=Some(Index{names,charge});drop(s);
  eprintln!("CACHE05 index_ready entries={count} charged_bytes={charge} build_ms={:.2} background=true",start.elapsed().as_secs_f64()*1000.0);Ok(true)
 }
 pub fn exists(&self,path:&str)->Option<bool>{
  let key=self.key(path)?;let mut s=self.state.lock().unwrap();let epoch=(self.epoch)();s.refresh(epoch);
  if epoch&1!=0 || s.index.is_none(){
   s.fallback+=1;
   #[cfg(all(target_os = "horizon", not(test)))]
   battle_trace::cache(CacheAccess::ExistsFallback);
   return None
  }
  let exists=s.index.as_ref().unwrap().names.contains(&key);s.hits+=1;if !exists{s.negative+=1;}
  #[cfg(all(target_os = "horizon", not(test)))]
  battle_trace::cache(if exists {CacheAccess::ExistsHit} else {CacheAccess::ExistsAbsent});
  Some(exists)
 }
 pub fn read(&self,path:&str)->io::Result<Vec<u8>>{
  let Some(key)=self.key(path) else{return std::fs::read(path)};
  let epoch={
   let mut s=self.state.lock().unwrap();let epoch=(self.epoch)();s.refresh(epoch);s.tick+=1;let tick=s.tick;
   if epoch&1==0 {if let Some((bytes,used))=s.data.get_mut(&key){*used=tick;let bytes=bytes.clone();s.read_hits+=1;
    #[cfg(all(target_os = "horizon", not(test)))]
    battle_trace::cache(CacheAccess::ByteHit);
    return Ok(bytes.to_vec())}}
   s.reads+=1;
   #[cfg(all(target_os = "horizon", not(test)))]
   battle_trace::cache(CacheAccess::ByteNative);
   epoch
  };
  let bytes=std::fs::read(path)?;
  if epoch&1==0 && bytes.len()<=FILE_LIMIT && bytes.len()<=self.limit && (self.epoch)()==epoch{
   let mut s=self.state.lock().unwrap();if s.epoch==epoch && (self.epoch)()==epoch{
    while s.bytes+bytes.len()>self.limit || s.data.len()>=ENTRY_LIMIT{
     let Some(old)=s.data.iter().min_by_key(|(_,(_,tick))|*tick).map(|(k,_)|k.clone()) else{break};
     let (old,_)=s.data.remove(&old).unwrap();s.bytes-=old.len();
    }
    if let Some((old,_))=s.data.remove(&key){s.bytes-=old.len();}
    s.bytes+=bytes.len();s.peak=s.peak.max(s.bytes);let tick=s.tick;s.data.insert(key,(Arc::from(bytes.as_slice()),tick));
   }
  }
  Ok(bytes)
 }
 pub fn report(&self){let mut s=self.state.lock().unwrap();s.refresh((self.epoch)());
  eprintln!("CACHE05 totals index_ready={} entries={} index_charged_bytes={} exists_hits={} negative_hits={} native_exists_fallbacks={} byte_hits={} native_read_calls={} byte_memory={} byte_peak={} byte_limit={} invalidations={} gpu_shared=false",s.index.is_some(),s.index.as_ref().map_or(0,|i|i.names.len()),s.index.as_ref().map_or(0,|i|i.charge),s.hits,s.negative,s.fallback,s.read_hits,s.reads,s.bytes,s.peak,self.limit,s.invalidations);
 }
}
static CACHE:OnceLock<Arc<Cache>>=OnceLock::new();
pub fn exists(path:&str)->Option<bool>{CACHE.get()?.exists(path)}
pub fn read(path:&str)->io::Result<Vec<u8>>{match CACHE.get(){Some(cache)=>cache.read(path),None=>std::fs::read(path)}}
pub fn report(){if let Some(cache)=CACHE.get(){cache.report();}}
pub struct Worker{cache:Arc<Cache>,thread:Option<thread::JoinHandle<()>>}
impl Drop for Worker{fn drop(&mut self){self.cache.stop.store(true,Ordering::Relaxed);if let Some(t)=self.thread.take(){t.thread().unpark();let _=t.join();}self.cache.report();}}
#[cfg(target_os="horizon")]
pub fn start(root:&str)->Option<Worker>{
 extern "C"{fn nx_resource_epoch()->u64;}
 if std::path::Path::new("sdmc:/switch/reloaded-nx/disable-resource-cache.flag").exists(){eprintln!("CACHE05 disabled_by_flag=true");return None}
 let cache=Arc::new(Cache::new(PathBuf::from(format!("{root}/data/pkmnbattle")),Arc::new(||unsafe{nx_resource_epoch()})));
 if CACHE.set(cache.clone()).is_err(){return None}
 let task=cache.clone();let thread=thread::Builder::new().name("sprite-index".into()).stack_size(512*1024).spawn(move||{
  extern "C" { fn nx_trace_ticks() -> u64; }
  let mut observed=None;
  let mut attempted=None;
  while !task.stop.load(Ordering::Relaxed){
   let epoch=(task.epoch)();
   if observed!=Some(epoch){
    observed=Some(epoch);
    let start=unsafe{nx_trace_ticks()};
    if epoch&1!=0 {
     eprintln!("BATTLE11 index result=blocked_writer tick={start} epoch={epoch}");
    } else if attempted!=Some(epoch) {
     attempted=Some(epoch);
     eprintln!("BATTLE11 index result=start tick={start} epoch={epoch}");
     let result=match task.build_index(){
      Ok(true)=>"published",
      Ok(false)=>"aborted",
      Err(error)=>{
       eprintln!("CACHE05 index_unavailable error={error}; native fallback");
       "error"
      }
     };
     let end=unsafe{nx_trace_ticks()};
     eprintln!("BATTLE11 index result={result} tick={end} epoch={epoch} epoch_end={}",(task.epoch)());
    }
   }
   thread::park_timeout(Duration::from_millis(250));
  }
 }).map_err(|e|eprintln!("CACHE05 worker_failed={e}; native fallback")).ok();
 Some(Worker{cache,thread})
}

#[cfg(test)]mod tests{
 use super::*;use std::sync::atomic::AtomicU64;
 fn fixture()->(PathBuf,Arc<AtomicU64>,Cache){let dir=std::env::current_dir().unwrap().join(format!("resource-test-{}-{}",std::process::id(),SystemTime::now().duration_since(SystemTime::UNIX_EPOCH).unwrap().as_nanos()));std::fs::create_dir(&dir).unwrap();let epoch=Arc::new(AtomicU64::new(0));let e=epoch.clone();let cache=Cache::new(dir.clone(),Arc::new(move||e.load(Ordering::SeqCst)));(dir,epoch,cache)}
 use std::time::SystemTime;
 #[test]fn variants_missing_and_bytes_match_native(){let(dir,_,cache)=fixture();std::fs::write(dir.join("P418_2.gmspr"),b"original").unwrap();std::fs::write(dir.join("P418_2_ALT.gmspr"),b"alternate").unwrap();assert!(cache.build_index().unwrap());for name in ["P418_2.gmspr","P418_2_ALT.gmspr","missing.gmspr"]{let p=dir.join(name);assert_eq!(cache.exists(p.to_str().unwrap()),Some(p.is_file()));if p.is_file(){for _ in 0..3{assert_eq!(cache.read(p.to_str().unwrap()).unwrap(),std::fs::read(&p).unwrap());}}}assert_eq!(cache.state.lock().unwrap().reads,2);}
 #[test]fn mutation_creation_deletion_and_replacement_invalidate(){let(dir,e,cache)=fixture();let p=dir.join("P1.gmspr");std::fs::write(&p,b"old").unwrap();cache.build_index().unwrap();cache.read(p.to_str().unwrap()).unwrap();e.store(3,Ordering::SeqCst);assert_eq!(cache.exists(p.to_str().unwrap()),None);std::fs::write(&p,b"new").unwrap();e.store(4,Ordering::SeqCst);assert_eq!(cache.read(p.to_str().unwrap()).unwrap(),b"new");std::fs::remove_file(&p).unwrap();e.store(6,Ordering::SeqCst);cache.build_index().unwrap();assert_eq!(cache.exists(p.to_str().unwrap()),Some(false));assert!(cache.read(p.to_str().unwrap()).is_err());std::fs::write(&p,b"created").unwrap();e.store(8,Ordering::SeqCst);cache.build_index().unwrap();assert_eq!(cache.exists(p.to_str().unwrap()),Some(true));}
 #[test]fn byte_lru_is_bounded(){let(dir,_,mut cache)=fixture();cache.limit=8;for n in 0..6{let p=dir.join(format!("P{n}.gmspr"));std::fs::write(&p,[n;4]).unwrap();assert_eq!(cache.read(p.to_str().unwrap()).unwrap(),[n;4]);}let s=cache.state.lock().unwrap();assert!(s.peak<=8);assert_eq!(s.data.len(),2);}
 #[test]fn unavailable_index_and_unrelated_paths_use_native_behavior(){let(dir,e,cache)=fixture();let p=dir.join("P1.gmspr");assert_eq!(cache.exists(p.to_str().unwrap()),None);e.store(1,Ordering::SeqCst);assert!(!cache.build_index().unwrap());let other=dir.join("save.savc");std::fs::write(&other,b"save").unwrap();assert_eq!(cache.exists(other.to_str().unwrap()),None);assert_eq!(cache.read(other.to_str().unwrap()).unwrap(),b"save");assert!(cache.state.lock().unwrap().data.is_empty());}
 #[test]fn generation_change_during_scan_never_publishes_partial_index(){let(dir,_,_)=fixture();for i in 0..20{std::fs::write(dir.join(format!("P{i}.gmspr")),[1]).unwrap();}let calls=Arc::new(AtomicU64::new(0));let c=calls.clone();let cache=Cache::new(dir.clone(),Arc::new(move||if c.fetch_add(1,Ordering::SeqCst)<3{0}else{2}));assert!(!cache.build_index().unwrap());assert!(cache.state.lock().unwrap().index.is_none());}
 #[test]fn oversized_files_are_read_exactly_without_being_cached(){let(dir,_,cache)=fixture();let p=dir.join("huge.gmspr");let bytes=vec![17;FILE_LIMIT+1];std::fs::write(&p,&bytes).unwrap();assert_eq!(cache.read(p.to_str().unwrap()).unwrap(),bytes);assert!(cache.state.lock().unwrap().data.is_empty());}
}
