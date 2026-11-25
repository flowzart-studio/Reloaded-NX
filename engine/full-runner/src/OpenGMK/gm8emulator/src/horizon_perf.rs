//! Low-frequency wall-time profiling. Inclusive spans overlap; never retimes the game.
use std::{sync::{Mutex,atomic::{AtomicU64,AtomicU32,Ordering}},time::Instant};
const ENABLED:bool=cfg!(target_os="horizon");
pub const GML:usize=0;pub const DRAW:usize=1;pub const PRESENT:usize=2;pub const WAIT:usize=3;pub const FRAME:usize=4;
static NS:[AtomicU64;5]=[const{AtomicU64::new(0)};5];
static DEPTH:[AtomicU32;5]=[const{AtomicU32::new(0)};5];
static STEPS:AtomicU64=AtomicU64::new(0);
static ROOM:AtomicU32=AtomicU32::new(0);
static TARGET:AtomicU32=AtomicU32::new(0);
static NO_SPRITE:AtomicU64=AtomicU64::new(0);
#[cfg(target_os="horizon")]
extern "C"{fn nx_perf_counter(id:u32)->u64;fn nx_perf_frequency()->u64;fn nx_gpu_begin();fn nx_gpu_end();fn nx_pcm_counter(id:u32)->u64;}
pub struct Scope{kind:usize,start:Option<Instant>,enabled:bool}
impl Scope{#[inline] pub fn new(kind:usize)->Self{
 let outer=ENABLED&&DEPTH[kind].fetch_add(1,Ordering::Relaxed)==0;
 #[cfg(target_os="horizon")] if outer&&kind==FRAME{unsafe{nx_gpu_begin()}}
 Self{kind,start:if outer{Some(Instant::now())}else{None},enabled:ENABLED}
}}
impl Drop for Scope{#[inline] fn drop(&mut self){
 if self.enabled{DEPTH[self.kind].fetch_sub(1,Ordering::Relaxed);}
 if let Some(start)=self.start{
  #[cfg(target_os="horizon")] if self.kind==FRAME{unsafe{nx_gpu_end()}}
  NS[self.kind].fetch_add(start.elapsed().as_nanos() as u64,Ordering::Relaxed);
  if self.kind==FRAME{STEPS.fetch_add(1,Ordering::Relaxed);}
 }
}}
#[inline]pub fn no_sprite(){if ENABLED{NO_SPRITE.fetch_add(1,Ordering::Relaxed);}}
fn rate(n:u64,seconds:f64)->f64{if seconds>0.0{n as f64/seconds}else{0.0}}
/// Rare, inclusive load/transition spans. No per-byte or per-instruction logging.
pub struct Operation { phase: &'static str, detail: String, start: Option<Instant>, io: [u64;3] }
impl Operation {
 pub fn new(phase: &'static str, detail: &str) -> Self {
  #[allow(unused_mut)] let mut io=[0;3];
  #[cfg(target_os="horizon")] unsafe {io=[nx_perf_counter(1),nx_perf_counter(2),nx_perf_counter(3)];}
  Self{phase,detail:if ENABLED{detail.to_owned()}else{String::new()},start:if ENABLED{Some(Instant::now())}else{None},io}
 }
}
impl Drop for Operation {fn drop(&mut self){
 #[cfg(target_os="horizon")] if let Some(start)=self.start {
  let ms=start.elapsed().as_secs_f64()*1000.0;
  if ms>=10.0 || self.phase.starts_with("room") || self.phase.starts_with("boot") {unsafe{
   eprintln!("LOAD03 phase={} detail={:?} wall_ms={ms:.2} io_read={} io_write={} io_ms={:.2} inclusive=true",self.phase,self.detail,nx_perf_counter(1).saturating_sub(self.io[0]),nx_perf_counter(2).saturating_sub(self.io[1]),1000.0*nx_perf_counter(3).saturating_sub(self.io[2]) as f64/nx_perf_frequency() as f64);
  }}
 }
}}
struct Stats{start:Option<Instant>,last:Option<Instant>,frames:u64,min:f64,max:f64,first:i32,last_room:i32,changes:u64,native:[u64;14],steps:u64}
static STATS:Mutex<Stats>=Mutex::new(Stats{start:None,last:None,frames:0,min:f64::INFINITY,max:0.0,first:0,last_room:0,changes:0,native:[0;14],steps:0});
#[inline]pub fn context(room:i32,target:u32){if ENABLED{ROOM.store(room as u32,Ordering::Relaxed);TARGET.store(target,Ordering::Relaxed);}}
#[inline]pub fn present(){
 if !ENABLED{return}
 #[cfg(target_os="horizon")] unsafe{
 let room=ROOM.load(Ordering::Relaxed) as i32;let target=TARGET.load(Ordering::Relaxed);
 let now=Instant::now();let mut s=STATS.lock().unwrap();

 if s.start.is_none(){s.start=Some(now);s.last=Some(now);s.first=room;s.last_room=room;s.native=std::array::from_fn(|i|nx_perf_counter(i as u32));s.steps=STEPS.load(Ordering::Relaxed);for n in &NS{n.store(0,Ordering::Relaxed);}NO_SPRITE.store(0,Ordering::Relaxed);return}
 let dt=now.duration_since(s.last.replace(now).unwrap()).as_secs_f64()*1000.0;
 s.frames+=1;s.min=s.min.min(dt);s.max=s.max.max(dt);
 if room!=s.last_room{s.changes+=1;s.last_room=room;}
 let seconds=now.duration_since(s.start.unwrap()).as_secs_f64();
 if seconds<5.0{return}
 let counters=std::array::from_fn(|i|nx_perf_counter(i as u32));
 let steps=STEPS.load(Ordering::Relaxed);let step_delta=steps-s.steps;
 let d:[u64;14]=std::array::from_fn(|i|counters[i].saturating_sub(s.native[i]));
 let ns:[u64;5]=std::array::from_fn(|i|NS[i].swap(0,Ordering::Relaxed));
 let ms=|n:u64|n as f64/1e6/s.frames as f64;
 let tick_ms=|n:u64|1000.0*n as f64/nx_perf_frequency() as f64;
 let suppressed=NO_SPRITE.swap(0,Ordering::Relaxed);
 let line=format!("PERF02 sec={seconds:.3} room_first={} room_last={room} changes={} target_hz={target} steps={} sps={:.2} presents={} fps={:.2} frame_ms(mean/min/max)={:.2}/{:.2}/{:.2} update_other_ms={:.2} gml_event_incl_ms={:.2} draw_incl_ms={:.2} present_incl_ms={:.2} wait_ms={:.2} gpu_samples={} gpu_elapsed_ms_mean={:.2} gpu_skips={} io_calls={} io_read={} io_write={} io_ms={:.2} log_calls={} log_bytes={} log_ms={:.2} swap_ms={:.2} vsync_wait_ms={:.2} no_sprite={} audio_frames={} audio_timeouts={} audio_queue_empty={} audio_error={}",s.first,s.changes,step_delta,rate(step_delta,seconds),d[7],rate(d[7],seconds),seconds*1000.0/s.frames as f64,s.min,s.max,ms(ns[FRAME].saturating_sub(ns[DRAW]+ns[PRESENT])),ms(ns[GML]),ms(ns[DRAW]),ms(ns[PRESENT]),ms(ns[WAIT]),d[11],if d[11]>0{d[12] as f64/1e6/d[11] as f64}else{0.0},d[13],d[0],d[1],d[2],tick_ms(d[3]),d[4],d[5],tick_ms(d[6]),tick_ms(d[8]),tick_ms(d[10]),suppressed,nx_pcm_counter(0),nx_pcm_counter(1),nx_pcm_counter(2),nx_pcm_counter(3));
 s.start=Some(now);s.frames=0;s.min=f64::INFINITY;s.max=0.0;s.first=room;s.changes=0;s.native=counters;s.steps=steps;drop(s);eprintln!("{line}");crate::resource_cache::report();
 eprintln!("IO05 totals open={} stat={} read={} fstat={} dirnext={}",nx_perf_counter(14),nx_perf_counter(15),nx_perf_counter(16),nx_perf_counter(17),nx_perf_counter(18));
 }

}
#[cfg(test)]mod tests{use super::*;#[test]fn rates_use_actual_window_not_room_speed(){assert_eq!(rate(5,1.0),5.0);assert_eq!(rate(75,2.5),30.0);assert_eq!(rate(0,5.0),0.0);assert_eq!(rate(1,0.0),0.0);}}

/// Log only a changed graphics configuration, including transitions back to exploration.
#[cfg(target_os="horizon")]
pub fn video(logical:(u32,u32),fb:(u32,u32),drawable:(u32,u32),rect:(i32,i32,i32,i32),scaling:crate::render::Scaling){
 static PREVIOUS:Mutex<Option<((u32,u32),(u32,u32),(u32,u32),(i32,i32,i32,i32),crate::render::Scaling)>>=Mutex::new(None);
 crate::battle_trace::framebuffer(fb);
 let state=(logical,fb,drawable,rect,scaling);let mut previous=PREVIOUS.lock().unwrap();
 if previous.as_ref()!=Some(&state){eprintln!("VIDEO04 logical={}x{} framebuffer={}x{} drawable={}x{} rect={rect:?} scaling={scaling:?} direct_blit=true",logical.0,logical.1,fb.0,fb.1,drawable.0,drawable.1);*previous=Some(state);}
}
