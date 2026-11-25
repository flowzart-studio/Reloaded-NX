//! Read-only private diagnostic. Output may contain original GML: do not publish it.
fn relevant(name: &str) -> bool {
    let n = name.to_lowercase();
    ["casino", "tragamon", "tragap", "ruleta", "roulette", "cards", "poker", "voltorb"].iter().any(|s| n.contains(s))
}
fn inspect(label: &str, data: &[u8]) {
    let text = String::from_utf8_lossy(data);
    println!("RESOURCE {label}");
    for line in text.lines() {
        if ["keyboard", "mouse_", "io_", "room", "alarm", "sleep", "while", "repeat", "instance_", "key_"].iter().any(|s| line.contains(s)) {
            println!("  {}", line.trim());
        }
    }
}
fn main() {
    let path = std::env::args().nth(1).expect("original EXE path");
    let mut bytes = std::fs::read(path).unwrap();
    let assets = gm8exe::reader::from_exe(&mut bytes, None::<fn(&str)>, false, false).unwrap();
    for (id, s) in assets.scripts.iter().enumerate().filter_map(|(id, s)| s.as_ref().map(|s| (id,s))) {
        let name = String::from_utf8_lossy(&s.name.0);
        let src = String::from_utf8_lossy(&s.source.0);
        if relevant(&name) || src.contains("key_vicio") || src.contains("key_poker") || src.contains(".fichasCasino") || id == 8428 {
            inspect(&format!("script {id} {name}"), &s.source.0);
        }
    }
    for (id, o) in assets.objects.iter().enumerate().filter_map(|(id, o)| o.as_ref().map(|o| (id,o))) {
        let name = String::from_utf8_lossy(&o.name.0);
        if !relevant(&name) { continue; }
        println!("OBJECT {id} {name} parent={}", o.parent_index);
        for (event, list) in o.events.iter().enumerate() {
            for (sub, actions) in list {
                for a in actions {
                    let label = format!("object {id} {name} event={event}:{sub}");
                    inspect(&label, &a.fn_code.0);
                    for p in &a.param_strings { if !p.0.is_empty() { inspect(&label, &p.0); } }
                }
            }
        }
    }
}
