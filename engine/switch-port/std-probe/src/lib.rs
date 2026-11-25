//! Real std filesystem, threads and OS-key TLS; executed only by the diagnostic NRO.
use std::{cell::Cell, sync::{Arc, Mutex, Condvar}, time::Instant};
static DROPPED: std::sync::atomic::AtomicU32=std::sync::atomic::AtomicU32::new(0);
struct TlsDrop;
impl Drop for TlsDrop { fn drop(&mut self) { DROPPED.fetch_add(1,std::sync::atomic::Ordering::SeqCst); } }
thread_local! { static VALUE: Cell<u32> = Cell::new(7); static GUARD: TlsDrop = const { TlsDrop }; }
#[no_mangle]
pub extern "C" fn nx_std_probe() -> i32 {
    match run() { Ok(()) => 0, Err(e) => { eprintln!("std probe: {e}"); 1 } }
}
fn run() -> std::io::Result<()> {
    let before=Instant::now();
    let mut map=std::collections::HashMap::new(); map.insert("entropy",1); assert_eq!(map["entropy"],1);
    let path="sdmc:/switch/reloaded-nx/diagnostics/std-probe-data.txt";
    std::fs::write(path,b"Reloaded NX std probe\n")?;
    assert_eq!(std::fs::read(path)?,b"Reloaded NX std probe\n");
    assert_eq!(std::fs::metadata(path)?.len(),22);
    assert!(std::fs::read_dir("sdmc:/switch/reloaded-nx/diagnostics")?.any(|e|e.unwrap().file_name()=="std-probe-data.txt"));
    VALUE.with(|v| { assert_eq!(v.get(),7); v.set(11); });
    let pair=Arc::new((Mutex::new(false),Condvar::new()));
    let shared=pair.clone();
    let child=std::thread::Builder::new().name("nx-std-probe".into()).spawn(move || {
        GUARD.with(|_|());
        VALUE.with(|v| { assert_eq!(v.get(),7); v.set(29); });
        let (lock,cv)=&*shared; *lock.lock().unwrap()=true; cv.notify_one();
        VALUE.with(|v|assert_eq!(v.get(),29));
        42
    })?;
    let (lock,cv)=&*pair;
    let (done,timeout)=cv.wait_timeout_while(lock.lock().unwrap(),std::time::Duration::from_secs(10),|v|!*v).unwrap();
    assert!(*done && !timeout.timed_out()); drop(done);
    assert_eq!(child.join().unwrap(),42);
    assert_eq!(DROPPED.load(std::sync::atomic::Ordering::SeqCst),1);
    VALUE.with(|v|assert_eq!(v.get(),11));
    std::hint::black_box(before.elapsed());
    std::fs::remove_file(path)?;
    assert_eq!(std::fs::read(path).unwrap_err().kind(), std::io::ErrorKind::NotFound);
    Ok(())
}
