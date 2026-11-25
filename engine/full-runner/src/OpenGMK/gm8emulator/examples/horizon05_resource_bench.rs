//! Brief, read-only A/B of the actual sprite cache against the same original files.
#[path="../src/resource_cache.rs"]mod resource_cache;
use std::{path::PathBuf,sync::Arc,time::Instant};
fn main(){let dir=PathBuf::from(std::env::args().nth(1).expect("original data/pkmnbattle path")).canonicalize().unwrap();
 let cache=resource_cache::Cache::new(dir.clone(),Arc::new(||0));let start=Instant::now();assert!(cache.build_index().unwrap());println!("INDEX_PC_MS {:.3}",start.elapsed().as_secs_f64()*1000.);
 let mut queries=Vec::new();for species in [418,290,447,2285]{for alt in ["","_ALT"]{for frame in ["1","2","3","6","8","9","A","H","S"]{queries.push(dir.join(format!("P{species}_{frame}{alt}.gmspr")));}}}
 // Read the reference once before either timed path, warming OS caches equally.
 let reference:Vec<_>=queries.iter().filter(|p|p.is_file()).map(|p|(p.clone(),std::fs::read(p).unwrap())).collect();
 let mut before_stats=0;let mut before_reads=0;let mut total=0;
 let start=Instant::now();let mut baseline=Vec::new();let mut baseline_round_ms=Vec::new();
 for _ in 0..4{let t=Instant::now();let mut round=Vec::new();for p in &queries{before_stats+=1;if p.is_file(){before_reads+=1;let b=std::fs::read(p).unwrap();total+=b.len();round.push((p.clone(),b));}}baseline_round_ms.push(t.elapsed().as_secs_f64()*1000.);baseline.push(round);}
 let baseline_ms=start.elapsed().as_secs_f64()*1000.;for round in baseline{assert_eq!(round,reference);}
 println!("BASELINE_PC exists_api_calls={before_stats} read_file_api_calls={before_reads} returned_bytes={total} elapsed_ms={baseline_ms:.3} rounds_ms={baseline_round_ms:?} os_cache=prewarmed");
 let start=Instant::now();let mut loaded=0;let mut cached=Vec::new();let mut cached_round_ms=Vec::new();
 for _ in 0..4{let t=Instant::now();let mut round=Vec::new();for p in &queries{let path=p.to_str().unwrap();if cache.exists(path).unwrap(){let bytes=cache.read(path).unwrap();round.push((p.clone(),bytes));loaded+=1;}}cached_round_ms.push(t.elapsed().as_secs_f64()*1000.);cached.push(round);}
 let cached_ms=start.elapsed().as_secs_f64()*1000.;for round in cached{assert_eq!(round,reference);}
 println!("CACHED_PC rounds=4 loaded={loaded} elapsed_ms={cached_ms:.3} rounds_ms={cached_round_ms:?} equality=PASS os_cache=prewarmed byte_cache=cold_at_start (not Switch timing)");cache.report();
}
