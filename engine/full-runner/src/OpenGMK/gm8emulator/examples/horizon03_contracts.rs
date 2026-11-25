//! Isolated file contracts and real MIDI synthesis. No game loop or game writes.
#[path="../src/gml/file.rs"] mod file;
#[path="../src/game/audio/midi.rs"] mod midi;
// Only the optional profiler is inert in this desktop diagnostic.
mod horizon_perf {pub struct Operation;impl Operation {pub fn new(_: &str,_:&str)->Self{Self}}}
#[derive(serde::Serialize,serde::Deserialize)]
pub struct SoundParams {pub volume:std::sync::atomic::AtomicU32}
fn make_volume(v:f64)->f32{1000f64.powf(v.clamp(0.,1.)-1.) as f32}
fn main(){
    use file::{BinaryHandle,TextHandle,AccessMode};
    use std::{fs::OpenOptions,io::{Write,Seek,SeekFrom}};
    use udon::source::Source;
    let mut args=std::env::args().skip(1);
    let dir=std::path::PathBuf::from(args.next().expect("fresh diagnostic directory"));
    std::fs::create_dir(&dir).expect("refuse existing directory");
    let data:Vec<u8>=(0..100_000).map(|i|(i%251) as u8).collect();
    let bin=dir.join("binary.bin");
    OpenOptions::new().write(true).create_new(true).open(&bin).unwrap().write_all(&data).unwrap();
    let mut f=BinaryHandle::open(bin.to_str().unwrap(),AccessMode::Read).unwrap();
    for i in (0..data.len()).rev(){f.seek(i as i32).unwrap();assert_eq!(f.size().unwrap(),data.len() as u64);assert_eq!(f.tell().unwrap(),i as u64);assert_eq!(f.read_byte().unwrap(),data[i]);}
    println!("PASS actual BinaryHandle reverse seek/read/size/tell 100000 bytes");
    let copy=dir.join("copy.bin");file::copy_contents(bin.to_str().unwrap(),copy.to_str().unwrap()).unwrap();assert_eq!(std::fs::read(&copy).unwrap(),data);
    assert!(file::copy_contents(bin.to_str().unwrap(),bin.to_str().unwrap()).is_err());assert_eq!(std::fs::read(&bin).unwrap(),data);
    assert!(file::copy_contents(dir.join("missing").to_str().unwrap(),copy.to_str().unwrap()).is_err());assert_eq!(std::fs::read(&copy).unwrap(),data);
    println!("PASS content copy: exact bytes, same-file rejection, missing-source preserves destination");
    for mode in [AccessMode::Write,AccessMode::Special] {
        let p=dir.join(format!("write-{mode:?}.bin"));let mut f=BinaryHandle::open(p.to_str().unwrap(),mode).unwrap();
        f.write_byte(7).unwrap();assert_eq!(f.size().unwrap(),1);assert_eq!(f.tell().unwrap(),1);
        f.seek(10).unwrap();f.write_byte(9).unwrap();assert_eq!(f.size().unwrap(),11);assert_eq!(f.tell().unwrap(),11);
        f.flush().unwrap();assert_eq!(std::fs::read(p).unwrap(),[7,0,0,0,0,0,0,0,0,0,9]);
    }
    println!("PASS binary writer/read-write size flush and sparse seek");
    let text=dir.join("text.txt");std::fs::write(&text,b" 12.5\r\nhello\r\n-7\r\n").unwrap();
    let mut t=TextHandle::open(text.to_str().unwrap(),AccessMode::Read).unwrap();
    assert!(!t.is_eof().unwrap());assert_eq!(t.read_real().unwrap(),12.5);assert!(t.is_eoln().unwrap());t.skip_line().unwrap();
    assert_eq!(t.read_string().unwrap(),b"hello");assert!(t.is_eoln().unwrap());t.skip_line().unwrap();assert_eq!(t.read_real().unwrap(),-7.);t.skip_line().unwrap();assert!(t.is_eof().unwrap());
    println!("PASS text real/string/CRLF/eof cursor contracts");
    // Benchmark read amplification with the old seek implementation and new helper.
    use std::{cell::Cell,rc::Rc,io::{self,Read,BufReader,Cursor}};
    struct Counted{f:Cursor<Vec<u8>>,bytes:Rc<Cell<u64>>}
    impl Read for Counted{fn read(&mut self,b:&mut[u8])->io::Result<usize>{let n=self.f.read(b)?;self.bytes.set(self.bytes.get()+n as u64);Ok(n)}}
    impl Seek for Counted{fn seek(&mut self,p:SeekFrom)->io::Result<u64>{self.f.seek(p)}}
    #[path="../src/gml/buffered_io.rs"] mod buffered;
    for optimized in [false,true]{let count=Rc::new(Cell::new(0));let mut f=BufReader::new(Counted{f:Cursor::new(data.clone()),bytes:count.clone()});
        for i in (0..data.len()).rev(){if optimized{buffered::seek(&mut f,i as u64).unwrap()}else{f.seek(SeekFrom::Start(i as u64)).unwrap();}let mut b=[0];f.read_exact(&mut b).unwrap();assert_eq!(b[0],data[i]);}
        println!("IO_MODEL optimized={optimized} input_bytes={} underlying_bytes={}",data.len(),count.get());
        if optimized{assert_eq!(count.get(),data.len() as u64);}
    }
    let sf=args.next().expect("SoundFont path");std::env::set_var("OPENGMK_SOUNDFONT",sf);
    for path in args {let b=std::fs::read(&path).unwrap();let h=midi::MidiHandle::new(b.into_boxed_slice(),52,1.,true).expect("parse original MIDI");
        let mut s=h.source(true).expect("real synth with SoundFont");let mut block=[0f32;2048];let mut peak=0f32;let mut samples=0;
        for _ in 0..150 {samples+=s.write_samples(&mut block);for x in block{assert!(x.is_finite());peak=peak.max(x.abs());}}
        assert!(peak>0.00001,"silent MIDI");s.reset();println!("MIDI_PCM PASS path={path:?} samples={samples} peak={peak} (not hardware output)");
    }
}
