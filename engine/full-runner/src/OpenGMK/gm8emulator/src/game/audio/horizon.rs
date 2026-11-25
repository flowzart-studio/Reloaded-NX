//! Owns the original Mixer on a joined worker, feeding actual libnx PCM buffers.
use super::mixer::Mixer;
use std::sync::{Arc,atomic::{AtomicBool,Ordering}};
use udon::source::Source;
extern "C"{fn nx_pcm_open()->i32;fn nx_pcm_close();fn nx_pcm_wait()->i32;fn nx_pcm_submit(samples:*const f32,count:usize)->i32;}
pub struct Output{stop:Arc<AtomicBool>,worker:Option<std::thread::JoinHandle<()>>}
impl Output{
 pub fn open(mut mixer:Mixer)->Result<Self,String>{
  if unsafe{nx_pcm_open()}!=0{return Err("audout initialization failed (see AUDIO_FAIL)".into())}
  let stop=Arc::new(AtomicBool::new(false));let flag=stop.clone();
  let worker=std::thread::Builder::new().name("opengmk-audout".into()).stack_size(2*1024*1024).spawn(move||{
   let mut samples=[0.0f32;2048];
   while !flag.load(Ordering::Acquire){
    match unsafe{nx_pcm_wait()}{0=>continue,1=>(),_=>break}
    let count=mixer.write_samples(&mut samples);samples[count..].fill(0.0);
    if unsafe{nx_pcm_submit(samples.as_ptr(),samples.len())}!=0{break}
   }
   // mixer receiver remains alive until this worker finishes, then drops normally.
  });
  match worker{Ok(worker)=>Ok(Self{stop,worker:Some(worker)}),Err(e)=>{unsafe{nx_pcm_close()};Err(e.to_string())}}
 }
}
impl Drop for Output{fn drop(&mut self){self.stop.store(true,Ordering::Release);if let Some(worker)=self.worker.take(){if worker.join().is_err(){eprintln!("AUDIO_WORKER_PANIC")}}unsafe{nx_pcm_close()}}}
