use std::{cell::RefCell, fmt::Write};

#[cfg(target_os = "horizon")]
extern "C" {
    fn nx_trace_init();
    fn nx_trace_ticks() -> u64;
    fn nx_trace_cpu() -> u64;
    fn nx_trace_ram() -> u64;
    fn nx_trace_io(out: *mut u64);
    fn nx_perf_frequency() -> u64;
    fn nx_resource_epoch() -> u64;
}

const UNAVAILABLE: u64 = u64::MAX;
const MAX_ROWS: u64 = 512;
const MAX_MARKS: usize = 24;
const MAX_PENDING: usize = 8;
const IO_NAMES: [&str; 5] = ["open", "stat", "read", "fstat", "other_io"];

#[derive(Clone, Copy)]
pub enum Phase {
    Frame, Gml, Room, Poll, Draw, Present, Swap, Vsync, GmlSleep, Modal, Pacing,
    Script, Exists, Sprite, ReadAsset, Decode, Convert, Upload, Image, Delete,
}
const PHASES: [&str; 20] = [
    "frame", "gml", "room", "poll", "draw", "present", "swap", "vsync", "gml_sleep",
    "modal", "pacing", "script", "exists", "sprite", "read_asset", "decode",
    "convert", "upload", "image", "delete",
];

#[derive(Clone, Copy, Default)]
struct Time {
    inclusive: u64,
    exclusive: u64,
    calls: u32,
}

#[derive(Clone, Copy)]
struct Mark {
    tick: u64,
    end: Option<u64>,
    label: &'static str,
    detail: [u8; 48],
    len: usize,
}
impl Mark {
    fn new(tick: u64, label: &'static str, detail: &[u8]) -> Self {
        let mut mark = Self { tick, end: None, label, detail: [0; 48], len: detail.len().min(48) };
        for (dst, src) in mark.detail.iter_mut().zip(detail.iter().take(mark.len)) {
            *dst = if src.is_ascii_alphanumeric() || b"_.-".contains(src) { *src } else { b'_' };
        }
        mark
    }
}

struct Iteration {
    id: u64,
    room: i32,
    start: u64,
    cpu: u64,
    epoch: u64,
    io: [u64; 11],
    times: [Time; PHASES.len()],
    children: u64,
    marks: [Option<Mark>; MAX_MARKS],
    count: usize,
    first_swap: Option<u64>,
    first_after_mark: Option<u64>,
    work_end: Option<u64>,
    work_swap: Option<u64>,
    cache: [u32; 5],
    max_gap: u64,
}
#[derive(Default)]
struct Trace {
    frequency: u64,
    next_id: u64,
    rows: u64,
    #[cfg(target_os = "horizon")]
    last_swap: Option<u64>,
    framebuffer: (u32, u32),
    current: Option<Iteration>,
    pending: Vec<Pending>,
}
struct Pending {
    id: u64,
    anchor: Option<u64>,
    work: Option<u64>,
}

thread_local! {
    static TRACE: RefCell<Trace> = RefCell::new(Trace::default());
}
fn tick() -> u64 {
    #[cfg(target_os = "horizon")]
    unsafe { return nx_trace_ticks() }
    #[cfg(not(target_os = "horizon"))]
    0
}
fn cpu() -> u64 {
    #[cfg(target_os = "horizon")]
    unsafe { return nx_trace_cpu() }
    #[cfg(not(target_os = "horizon"))]
    UNAVAILABLE
}
fn epoch() -> u64 {
    #[cfg(target_os = "horizon")]
    unsafe { return nx_resource_epoch() }
    #[cfg(not(target_os = "horizon"))]
    0
}
fn io() -> [u64; 11] {
    #[cfg(target_os = "horizon")]
    {
        let mut out = [0; 11];
        unsafe { nx_trace_io(out.as_mut_ptr()); }
        out
    }
    #[cfg(not(target_os = "horizon"))]
    [0; 11]
}
fn ram() -> u64 {
    #[cfg(target_os = "horizon")]
    unsafe { return nx_trace_ram() }
    #[cfg(not(target_os = "horizon"))]
    UNAVAILABLE
}

#[cfg(target_os = "horizon")]
pub fn init() {
    if !std::path::Path::new("sdmc:/switch/reloaded-nx/enable-battle-trace.flag").exists()
        || std::path::Path::new("sdmc:/switch/reloaded-nx/disable-battle-trace.flag").exists()
    {
        eprintln!("BATTLE11 disabled=true");
        return;
    }
    let frequency = unsafe { nx_trace_init(); nx_perf_frequency() };
    TRACE.with(|trace| trace.borrow_mut().frequency = frequency);
    eprintln!("BATTLE11 clock_hz={frequency} ram_unavailable={UNAVAILABLE} max_rows={MAX_ROWS}");
}

pub struct Loop(bool);
impl Loop {
    #[inline]
    pub fn start(room: i32) -> Self {
        if !cfg!(target_os = "horizon") {
            return Self(false);
        }
        TRACE.with(|trace| {
            let mut trace = trace.borrow_mut();
            if trace.frequency == 0 || trace.rows > MAX_ROWS {
                return Self(false);
            }
            debug_assert!(trace.current.is_none());
            trace.next_id += 1;
            trace.current = Some(Iteration {
                id: trace.next_id, room, start: tick(), cpu: cpu(), epoch: epoch(), io: io(),
                times: [Time::default(); PHASES.len()], children: 0, marks: [None; MAX_MARKS],
                count: 0, first_swap: None, first_after_mark: None, work_end: None,
                work_swap: None, cache: [0; 5], max_gap: 0,
            });
            Self(true)
        })
    }
}
impl Drop for Loop {
    fn drop(&mut self) {
        if !self.0 {
            return;
        }
        TRACE.with(|trace| {
            let mut trace = trace.borrow_mut();
            let frame = trace.current.take().unwrap();
            let end = tick();
            let elapsed = end - frame.start;
            if !should_report(elapsed, frame.max_gap, frame.count, trace.frequency) {
                return;
            }
            trace.rows += 1;
            if trace.rows > MAX_ROWS {
                eprintln!("BATTLE11 capped=true");
                return;
            }

            let end_cpu = cpu();
            let cpu_ticks = cpu_delta(frame.cpu, end_cpu);
            let after = frame.first_after_mark.or(frame.first_swap.filter(|_| frame.count == 0));
            let pending = Pending {
                id: frame.id,
                anchor: if after.is_none() { Some(frame.marks[0].map_or(frame.start, |mark| mark.tick)) } else { None },
                work: frame.work_end.filter(|_| frame.work_swap.is_none()),
            };
            if (pending.anchor.is_some() || pending.work.is_some()) && trace.pending.len() < MAX_PENDING {
                trace.pending.push(pending);
            }
            let mut line = format!(
                "BATTLE11 loop id={} room={} start={} wall={} cpu={} gap={} fb={}x{} first_swap={} anchor_swap={} work_end={} work_swap={} epoch_start={} epoch_end={} ram={} marks_lost={}",
                frame.id, frame.room, frame.start, elapsed, cpu_ticks, frame.max_gap, trace.framebuffer.0, trace.framebuffer.1,
                frame.first_swap.unwrap_or(0), after.unwrap_or(0), frame.work_end.unwrap_or(0),
                frame.work_swap.unwrap_or(0), frame.epoch, epoch(), ram(),
                frame.count.saturating_sub(MAX_MARKS),
            );
            let io_end = io();
            for (index, name) in IO_NAMES.iter().enumerate() {
                let n = index * 2;
                let _ = write!(line, " {name}={}:{}", io_end[n] - frame.io[n], io_end[n + 1] - frame.io[n + 1]);
            }
            let _ = write!(line, " sd_read_bytes={} cache={},{},{},{},{}",
                io_end[10] - frame.io[10], frame.cache[0], frame.cache[1], frame.cache[2],
                frame.cache[3], frame.cache[4]);
            for (name, time) in PHASES.iter().zip(&frame.times) {
                if time.calls != 0 {
                    let _ = write!(line, " {name}={}:{}:{}", time.inclusive, time.exclusive, time.calls);
                }
            }
            for mark in frame.marks.iter().flatten() {
                let detail = std::str::from_utf8(&mark.detail[..mark.len]).unwrap();
                let elapsed = mark.end.map_or(0, |end| end - mark.tick);
                let _ = write!(line, " mark={}:{}:{}:{}", mark.tick - frame.start, mark.label, detail, elapsed);
            }
            eprintln!("{line}");
        });
    }
}

fn should_report(wall: u64, gap: u64, marks: usize, frequency: u64) -> bool {
    marks != 0 || wall.max(gap) >= frequency / 10
}

fn cpu_delta(start: u64, end: u64) -> u64 {
    if start == UNAVAILABLE || end == UNAVAILABLE || end < start { UNAVAILABLE } else { end - start }
}

pub struct Scope {
    span: Option<(Phase, u64, u64)>,
    mark: Option<usize>,
}
impl Scope {
    const DISABLED: Self = Self { span: None, mark: None };
}
impl Scope {
    #[inline]
    pub fn new(phase: Phase) -> Self {
        if !cfg!(target_os = "horizon") {
            return Self::DISABLED;
        }
        let span = TRACE.with(|trace| {
            let mut trace = trace.borrow_mut();
            let current = trace.current.as_mut()?;
            Some((phase, tick(), std::mem::take(&mut current.children)))
        });
        Self { span, mark: None }
    }

    fn marked(phase: Phase, label: &'static str, detail: &[u8]) -> Self {
        let mut scope = Self::new(phase);
        scope.mark = mark(label, detail);
        scope
    }
}
impl Drop for Scope {
    fn drop(&mut self) {
        if let Some((phase, start, siblings)) = self.span {
            let end = tick();
            let elapsed = end - start;
            TRACE.with(|trace| {
                let mut trace = trace.borrow_mut();
                let current = trace.current.as_mut().unwrap();
                add_span(&mut current.times[phase as usize], elapsed, current.children);
                current.children = siblings + elapsed;
                if let Some(index) = self.mark {
                    if let Some(Some(mark)) = current.marks.get_mut(index) {
                        mark.end = Some(end);
                    }
                    current.work_end = Some(end);
                    current.work_swap = None;
                }
            });
        }
    }
}
fn add_span(time: &mut Time, elapsed: u64, children: u64) {
    time.inclusive += elapsed;
    time.exclusive += elapsed.saturating_sub(children);
    time.calls += 1;
}

#[inline]
fn mark(label: &'static str, detail: &[u8]) -> Option<usize> {
    if !cfg!(target_os = "horizon") {
        return None;
    }
    TRACE.with(|trace| {
        let mut trace = trace.borrow_mut();
        let frame = trace.current.as_mut()?;
        let index = frame.count;
        frame.count += 1;
        if let Some(slot) = frame.marks.get_mut(index) {
            *slot = Some(Mark::new(tick(), label, detail));
        }
        Some(index)
    })
}

#[cfg(target_os = "horizon")]
pub enum CacheAccess { ExistsHit, ExistsAbsent, ExistsFallback, ByteHit, ByteNative }
#[cfg(target_os = "horizon")]
pub fn cache(access: CacheAccess) {
    TRACE.with(|trace| {
        if let Some(frame) = trace.borrow_mut().current.as_mut() {
            frame.cache[access as usize] += 1;
        }
    });
}
/// IDs from the original 1.9.2.2 EXE. Verify the name as well so a
/// different asset table cannot silently attribute an unrelated script.
#[cfg(target_os = "horizon")]
#[inline]
pub fn script_label(id: usize, name: &[u8]) -> Option<&'static [u8]> {
    let expected: &[u8] = match id {
        5406 => b"Begin_Sprite",
        8014 => b"ImportBattlePokemon",
        8038 => b"IniciaSalvaje",
        5397 => b"BattleStartTrainer1",
        9932 => b"Trainer1CambioPokemon",
        9935 => b"Trainer2CambioPokemon",
        6612 => b"DefinePokemon1",
        6613 => b"DefinePokemon1Auto",
        6615 => b"DefinePokemon2",
        6616 => b"DefinePokemon2Manual",
        _ => return None,
    };
    (name == expected).then_some(expected)
}
#[cfg(target_os = "horizon")]
#[inline]
pub fn script(label: Option<&'static [u8]>) -> Scope {
    label.map_or(Scope::DISABLED, |name| Scope::marked(Phase::Script, "gml", name))
}

#[cfg(target_os = "horizon")]
pub fn framebuffer(size: (u32, u32)) {
    TRACE.with(|trace| trace.borrow_mut().framebuffer = size);
}

#[inline]
pub fn switch_key(vk: u8) {
    if matches!(vk, b'1'..=b'6' | b'S' | b'Y') {
        let _ = mark("candidate_key", &[vk]);
    }
}

#[inline]
pub fn sprite(path: &str, label: &'static str) -> Scope {
    if cfg!(target_os = "horizon") {
        let file = path.rsplit(['/', '\\']).next().unwrap_or(path);
        return Scope::marked(Phase::Sprite, label, file.as_bytes());
    }
    Scope::DISABLED
}

#[cfg(target_os = "horizon")]
pub fn present() {
    TRACE.with(|trace| {
        let mut trace = trace.borrow_mut();
        if trace.frequency == 0 {
            return;
        }
        let now = tick();
        let gap = trace.last_swap.replace(now).map_or(0, |last| now - last);
        if let Some(frame) = trace.current.as_mut() {
            frame.first_swap.get_or_insert(now);
            if frame.count != 0 {
                frame.first_after_mark.get_or_insert(now);
            }
            if frame.work_end.is_some() {
                frame.work_swap.get_or_insert(now);
            }
            frame.max_gap = frame.max_gap.max(gap);
        }
        for pending in trace.pending.drain(..) {
            eprintln!("BATTLE11 next_swap id={} tick={now} anchor_swap={} work_swap={}",
                pending.id, pending.anchor.map_or(0, |_| now), pending.work.map_or(0, |_| now));
        }
    });
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn exclusive_time_does_not_double_count_nested_scopes() {
        let mut total = Time::default();
        add_span(&mut total, 120, 80);
        add_span(&mut total, 40, 10);
        assert_eq!((total.inclusive, total.exclusive, total.calls), (160, 70, 2));
    }

    #[test]
    fn reports_fast_anchors_and_slow_gaps_even_with_a_short_loop() {
        assert!(should_report(3, 3, 1, 1000));
        assert!(should_report(3, 101, 0, 1000));
        assert!(!should_report(20, 30, 0, 1000));
        assert_eq!(cpu_delta(UNAVAILABLE, 2), UNAVAILABLE);
    }
}
