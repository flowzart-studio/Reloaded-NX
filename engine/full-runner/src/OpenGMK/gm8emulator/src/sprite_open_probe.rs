use std::{
    fs::File,
    io::{self, Read, Seek, SeekFrom},
    path::Path,
    time::{Duration, Instant},
};

const FILES: [&str; 8] = [
    "P41_2.gmspr", "P41_3.gmspr", "P41_6.gmspr", "P41_8.gmspr",
    "P41_9.gmspr", "P41_A.gmspr", "P41_H.gmspr", "P41_S.gmspr",
];
const MAGIC: &[u8; 8] = b"NXPK41v1";
const HEADER: usize = 8 + 4 * FILES.len();
const MAX_PAYLOAD: usize = 64 * 1024;

#[derive(Default)]
struct Measures {
    open: Duration,
    read: Duration,
    seeks: Duration,
    opens: u32,
    reads: u32,
    bytes: usize,
}
impl Measures {
    fn open(&mut self, path: &Path) -> io::Result<File> {
        let start = Instant::now();
        let file = File::open(path)?;
        self.open += start.elapsed();
        self.opens += 1;
        Ok(file)
    }

    fn read_exact(&mut self, file: &mut File, buf: &mut [u8]) -> io::Result<()> {
        let start = Instant::now();
        file.read_exact(buf)?;
        self.read += start.elapsed();
        self.reads += 1;
        self.bytes += buf.len();
        Ok(())
    }

    fn seek(&mut self, file: &mut File, offset: u64) -> io::Result<()> {
        let start = Instant::now();
        file.seek(SeekFrom::Start(offset))?;
        self.seeks += start.elapsed();
        Ok(())
    }

    fn report(&self, round: usize, mode: &str, wall: Duration, pack_bytes: u64) {
        let ms = |t: Duration| t.as_secs_f64() * 1000.0;
        eprintln!(
            "SPRITEPROBE11 round={round} mode={mode} wall_ms={:.3} opens={} open_ms={:.3} read_calls={} read_ms={:.3} seek_ms={:.3} bytes={} pack_bytes={pack_bytes}",
            ms(wall), self.opens, ms(self.open), self.reads, ms(self.read), ms(self.seeks), self.bytes,
        );
    }
}

fn invalid() -> io::Error {
    io::Error::new(io::ErrorKind::InvalidData, "invalid P41 sample pack")
}

fn offsets(header: &[u8; HEADER], file_len: u64) -> io::Result<[usize; 9]> {
    if &header[..8] != MAGIC || file_len > (HEADER + MAX_PAYLOAD) as u64 {
        return Err(invalid());
    }
    let mut ends = [0; 9];
    for (i, size) in header[8..].chunks_exact(4).enumerate() {
        let len = u32::from_le_bytes(size.try_into().unwrap()) as usize;
        if len == 0 || len > MAX_PAYLOAD - ends[i] {
            return Err(invalid());
        }
        ends[i + 1] = ends[i] + len;
    }
    if (HEADER + ends[8]) as u64 != file_len {
        return Err(invalid());
    }
    Ok(ends)
}

// An opt-in experiment only. It never returns resources to GML or the renderer.
fn compare(original: &Path, pack: &Path) -> io::Result<()> {
    let mut pack_file = File::open(pack)?;
    let file_len = pack_file.metadata()?.len();
    let mut header = [0; HEADER];
    pack_file.read_exact(&mut header)?;
    let ends = offsets(&header, file_len)?;
    drop(pack_file);

    let mut loose = vec![0; ends[8]];
    let mut grouped = vec![0; ends[8]];
    let mut results = Vec::with_capacity(4);
    for (round, packed) in [false, true, true, false].into_iter().enumerate() {
        let mut measures = Measures::default();
        let start = Instant::now();
        if packed {
            let mut f = measures.open(pack)?;
            measures.seek(&mut f, HEADER as u64)?;
            measures.read_exact(&mut f, &mut grouped)?;
        } else {
            for (i, name) in FILES.iter().enumerate() {
                let mut f = measures.open(&original.join(name))?;
                measures.read_exact(&mut f, &mut loose[ends[i]..ends[i + 1]])?;
                let mut extra = [0; 1];
                // A stale length must fail even when the common prefix matches.
                let start = Instant::now();
                let trailing = f.read(&mut extra)?;
                measures.read += start.elapsed();
                measures.reads += 1;
                if trailing != 0 {
                    return Err(invalid());
                }
            }
        }
        let wall = start.elapsed();
        if round != 0 && loose != grouped {
            return Err(invalid());
        }
        results.push((round + 1, if packed { "pack" } else { "loose" }, measures, wall));
    }
    for (round, mode, measures, wall) in results {
        measures.report(round, mode, wall, file_len);
    }
    eprintln!("SPRITEPROBE11 result=verified payload_bytes={} buffer_bytes={} order=ABBA", ends[8], 2 * ends[8]);
    Ok(())
}

pub fn run(game: &str) {
    let pack = Path::new("sdmc:/switch/reloaded-nx/sprite-open-probe.bin");
    if !pack.is_file() {
        return;
    }
    if let Err(error) = compare(&Path::new(game).join("data/pkmnbattle"), pack) {
        eprintln!("SPRITEPROBE11 result=rejected error={error:?}");
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn rejects_bad_magic_truncation_and_oversize() {
        let mut header = [0; HEADER];
        header[..8].copy_from_slice(MAGIC);
        for size in header[8..].chunks_exact_mut(4) {
            size.copy_from_slice(&1u32.to_le_bytes());
        }
        assert_eq!(offsets(&header, HEADER as u64 + 8).unwrap()[8], 8);
        assert!(offsets(&header, HEADER as u64 + 7).is_err());
        assert!(offsets(&header, (HEADER + MAX_PAYLOAD + 1) as u64).is_err());
        header[0] = 0;
        assert!(offsets(&header, HEADER as u64 + 8).is_err());
    }
}
