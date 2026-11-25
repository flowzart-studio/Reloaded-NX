//! Read-only control inventory. Prints identifiers, counts and event IDs, never GML bodies.
use std::collections::{BTreeMap, BTreeSet};

fn inspect(label: &str, source: &[u8], rows: &mut BTreeMap<String, BTreeSet<String>>) {
    let source = String::from_utf8_lossy(source);
    let words: BTreeSet<_> = source.split(|c: char| !c.is_ascii_alphanumeric() && c != '_').collect();
    for word in &words {
        let relevant = word.starts_with("vk_")
            || word.starts_with("mouse_check")
            || word.starts_with("mouse_wheel")
            || ["mouse_x","mouse_y","keyboard_lastkey","keyboard_string","keyboard_set_map",
                "keyboard_wait","keyboard_check_direct","keyboard_key_press","keyboard_key_release",
                "ESPERANDO_TECLA"].contains(word);
        if relevant {
            rows.entry((*word).into()).or_default().insert(label.into());
        }
    }
    for key in b'0'..=b'Z' {
        if !key.is_ascii_alphanumeric() { continue }
        let key = char::from(key);
        for pattern in [format!("ord('{key}')"),format!("ord(\"{key}\")")] {
            if source.contains(&pattern) {
                rows.entry(format!("ord({key})")).or_default().insert(label.into());
            }
        }
    }
}

fn main() {
    let mut bytes = std::fs::read(std::env::args_os().nth(1).expect("EXE")).unwrap();
    let assets = gm8exe::reader::from_exe(&mut bytes,None::<fn(&str)>,false,false).unwrap();
    let mut rows = BTreeMap::new();
    let mut events = BTreeMap::<(usize, i32), BTreeSet<String>>::new();
    for (id, script) in assets.scripts.iter().enumerate() {
        if let Some(s) = script {
            inspect(&format!("S{id}:{}",String::from_utf8_lossy(&s.name.0)),&s.source.0,&mut rows);
        }
    }
    for (id, room) in assets.rooms.iter().enumerate() {
        if let Some(r) = room {
            inspect(&format!("R{id}:{}",String::from_utf8_lossy(&r.name.0)),&r.creation_code.0,&mut rows);
            for instance in &r.instances {
                inspect(&format!("R{id}/I{}",instance.id),&instance.creation_code.0,&mut rows);
            }
        }
    }
    for o in assets.objects.iter().flatten() {
        let label = String::from_utf8_lossy(&o.name.0);
        for (event, list) in o.events.iter().enumerate() {
            for (sub, actions) in list {
                if matches!(event,5|9|10) {
                    events.entry((event,*sub as i32)).or_default().insert(label.to_string());
                }
                let label = format!("O:{label}/{event}:{sub}");
                for action in actions {
                    inspect(&label,&action.fn_code.0,&mut rows);
                    for p in &action.param_strings { inspect(&label,&p.0,&mut rows); }
                }
            }
        }
    }
    for (word, sources) in rows {
        println!("{word} count={} {}",sources.len(),sources.into_iter().collect::<Vec<_>>().join(" "));
    }
    for ((event,key), sources) in events {
        println!("EVENT {event}:{key} count={} {}",sources.len(),sources.into_iter().collect::<Vec<_>>().join(" "));
    }
}
