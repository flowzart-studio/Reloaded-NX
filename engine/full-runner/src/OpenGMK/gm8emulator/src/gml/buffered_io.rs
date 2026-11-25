//! Preserve BufReader's unread bytes when GML queries or rewinds a file.
//! No whole-file cache: memory remains bounded by the existing reader capacity.
use std::io::{self, BufRead, BufReader, Read, Seek, SeekFrom};

/// One summary per closed read handle, never one log per byte or seek.
#[derive(Debug)]
pub struct ReadFile {
    pub file: std::fs::File,
    path: String,
    bytes: u64,
    reads: u64,
    seeks: u64,
    io_ns: u64,
}
impl ReadFile { pub fn new(file: std::fs::File, path: &str) -> Self {
    Self { file, path: if cfg!(target_os="horizon") {path.into()} else {String::new()},bytes:0,reads:0,seeks:0,io_ns:0 }
}}
impl Read for ReadFile {fn read(&mut self,out:&mut [u8])->io::Result<usize>{
    #[cfg(target_os="horizon")] let start=std::time::Instant::now();
    let result=self.file.read(out);
    #[cfg(target_os="horizon")] {self.reads+=1;self.bytes+=result.as_ref().copied().unwrap_or(0) as u64;self.io_ns+=start.elapsed().as_nanos() as u64;}
    result
}}
impl Seek for ReadFile {fn seek(&mut self,pos:SeekFrom)->io::Result<u64>{
    #[cfg(target_os="horizon")] {self.seeks+=1;}
    self.file.seek(pos)
}}
impl Drop for ReadFile {fn drop(&mut self){
    #[cfg(target_os="horizon")] if self.reads>16 || self.bytes>=131072 || self.io_ns>=10_000_000 {
        eprintln!("FILE03 path={:?} read_calls={} read_bytes={} seek_calls={} read_ms={:.2}",self.path,self.reads,self.bytes,self.seeks,self.io_ns as f64/1e6);
    }
}}

pub fn eof<R: Read + Seek>(f: &mut BufReader<R>) -> io::Result<bool> {
    Ok(f.fill_buf()?.is_empty())
}

pub fn eoln<R: Read + Seek>(f: &mut BufReader<R>) -> io::Result<bool> {
    let mut bytes = [0; 2];
    let n = f.read(&mut bytes)?;
    // Read may stop at the buffer boundary. Preserve the old two-byte lookahead.
    let n = if n == 1 { n + f.read(&mut bytes[1..])? } else { n };
    f.seek_relative(-(n as i64))?;
    Ok(n == 0 || (n == 2 && bytes == *b"\r\n"))
}

pub fn seek<R: Read + Seek>(f: &mut BufReader<R>, pos: u64) -> io::Result<()> {
    let current = f.stream_position()?;
    let delta = i128::from(pos) - i128::from(current);
    if let Ok(delta) = i64::try_from(delta) {
        f.seek_relative(delta)?;
    } else {
        f.seek(SeekFrom::Start(pos))?;
    }
    if f.buffer().is_empty() && f.capacity() > 0 {
        // Original GetTextData reads .savb backwards, one byte/seek at a time.
        // Read-ahead starting AT each byte cannot help that traversal. Refill
        // from the containing page instead, using the same bounded buffer.
        let page = pos / f.capacity() as u64 * f.capacity() as u64;
        f.seek(SeekFrom::Start(page))?;
        let offset = (pos - page) as usize;
        if offset <= f.fill_buf()?.len() { f.consume(offset); }
        else { f.seek(SeekFrom::Start(pos))?; }
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::{cell::Cell, io::Cursor, rc::Rc};
    struct Counted { file: Cursor<Vec<u8>>, bytes: Rc<Cell<usize>> }
    impl Read for Counted { fn read(&mut self, out: &mut [u8]) -> io::Result<usize> {
        let n = self.file.read(out)?; self.bytes.set(self.bytes.get() + n); Ok(n)
    }}
    impl Seek for Counted { fn seek(&mut self, pos: SeekFrom) -> io::Result<u64> { self.file.seek(pos) } }
    #[test] fn repeated_eof_and_seek_preserve_data_and_avoid_rereads() {
        let data: Vec<u8> = (0..100_000).map(|i| (i % 251) as u8).collect();
        let bytes = Rc::new(Cell::new(0));
        let mut f = BufReader::new(Counted { file: Cursor::new(data.clone()), bytes: bytes.clone() });
        for expected in data { assert!(!eof(&mut f).unwrap()); let pos=f.stream_position().unwrap();
            seek(&mut f,pos).unwrap(); let mut b=[0]; f.read_exact(&mut b).unwrap(); assert_eq!(b[0],expected); }
        assert!(eof(&mut f).unwrap()); assert_eq!(bytes.get(),100_000);
    }
    #[test] fn crlf_boundaries_and_eof_do_not_consume_input() {
        for capacity in 1..=5 { for data in [b"\r\nx".as_slice(),b"\rx",b"\n",b"",b"\r"] {
            let mut f=BufReader::with_capacity(capacity,Cursor::new(data));
            for _ in 0..3 { assert_eq!(eoln(&mut f).unwrap(), data.is_empty() || data.starts_with(b"\r\n")); assert_eq!(f.stream_position().unwrap(),0); }
            let mut got=Vec::new();f.read_to_end(&mut got).unwrap();assert_eq!(got,data);
        }}
    }
    #[test] fn seek_outside_buffer_matches_cursor() {
        let mut f=BufReader::with_capacity(4,Cursor::new(b"0123456789"));
        let mut b=[0];f.read_exact(&mut b).unwrap();seek(&mut f,8).unwrap();f.read_exact(&mut b).unwrap();assert_eq!(b[0],b'8');
        seek(&mut f,0).unwrap();f.read_exact(&mut b).unwrap();assert_eq!(b[0],b'0');
    }
    #[test] fn original_reverse_byte_pattern_is_linear_io() {
        let data: Vec<u8>=(0..100_000).map(|i|(i%251) as u8).collect();
        let bytes=Rc::new(Cell::new(0));
        let mut f=BufReader::new(Counted { file:Cursor::new(data.clone()),bytes:bytes.clone() });
        for i in (0..data.len()).rev() { seek(&mut f,i as u64).unwrap();let mut b=[0];f.read_exact(&mut b).unwrap();assert_eq!(b[0],data[i]); }
        assert_eq!(bytes.get(),100_000);
        seek(&mut f,200_000).unwrap();assert_eq!(f.stream_position().unwrap(),200_000);assert!(eof(&mut f).unwrap());
    }
}
