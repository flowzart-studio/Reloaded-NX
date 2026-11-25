//! Bounded opt-in diagnostics; no GML source, save data or injected input.
use std::sync::{atomic::{AtomicU32, Ordering}, Mutex, OnceLock};
use std::time::Instant;
static ACTIVE: AtomicU32 = AtomicU32::new(0);
static ENABLED: OnceLock<bool> = OnceLock::new();
static STATE: OnceLock<Mutex<State>> = OnceLock::new();
const LIMIT: u32 = 2048;
struct State { start: Instant, second: u64, total: u32, queries: u32, events: u32, last_step: Option<u64>, context: (i32,u32,[u8;5]) }
fn enabled() -> bool {
    *ENABLED.get_or_init(|| std::env::var_os("RELOADED_NX_CASINO_TRACE").is_some()
        || std::path::Path::new("sdmc:/switch/reloaded-nx/enable-casino-trace.flag").is_file())
}
fn state() -> &'static Mutex<State> {
    STATE.get_or_init(|| Mutex::new(State { start: Instant::now(), second: 0, total: 0,
        queries: 0, events: 0, last_step: None, context: (-1,0,[0;5]) }))
}
pub fn context(room: i32, kind: u32, keys: [u8;5]) {
    ACTIVE.store(kind, Ordering::Relaxed);
    if !enabled() { return; }
    let Ok(mut s) = state().lock() else { return; };
    if s.context == (room,kind,keys) { return; }
    s.context = (room,kind,keys);
    if s.total >= LIMIT { return; }
    s.total += 1;
    eprintln!("CASINO13 trace_context room={room} kind={kind} keys={keys:?}");
}
pub fn event(label: &str, room: i32, key: i32, value: bool) {
    if ACTIVE.load(Ordering::Relaxed) == 0 || !enabled() { return; }
    let Ok(mut s) = state().lock() else { return; };
    let second = s.start.elapsed().as_secs();
    if second != s.second { s.second = second; s.queries = 0; s.events = 0; }
    if label == "step" {
        if s.last_step == Some(second) { return; }
        s.last_step = Some(second);
    }
    let query = label.starts_with("check");
    let used = if query { &mut s.queries } else { &mut s.events };
    if *used >= 8 { return; }
    *used += 1;
    if s.total >= LIMIT { return; }
    s.total += 1;
    eprintln!("CASINO13 t={second} room={room} op={label} vk={key} value={value}");
    if s.total == LIMIT { eprintln!("CASINO13 trace_limit=2048 restart_to_rearm=true"); }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn bounded_even_inside_a_busy_query_loop() {
        ENABLED.set(true).unwrap();
        context(312,1,[32,90,88,67,88]);
        for _ in 0..100_000 { event("check",312,67,false); }
        let s = state().lock().unwrap();
        assert!(s.total <= LIMIT);
        assert!(s.queries <= 8);
        assert_eq!(s.context,(312,1,[32,90,88,67,88]));
    }
}
