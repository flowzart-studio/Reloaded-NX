//! Inspect relevant control consumers without running or extracting the game.
//! Keep the output local. It contains short GML excerpts for manual audit.
fn main() {
    let mut args = std::env::args_os().skip(1);
    let mut bytes = std::fs::read(args.next().expect("EXE path")).unwrap();
    let needles: Vec<_> = args.map(|arg| arg.into_string().expect("UTF-8 query")).collect();
    assert!(!needles.is_empty(), "provide key field names or script numbers");
    let assets = gm8exe::reader::from_exe(&mut bytes, None::<fn(&str)>, false, false).unwrap();
    for id in needles.iter().filter_map(|needle| needle.strip_prefix("room-object:")) {
        let id: i32 = id.parse().expect("object ID");
        for (number, room) in assets.rooms.iter().enumerate() {
            let Some(room) = room else { continue };
            if room.instances.iter().any(|instance| instance.object == id) {
                println!("R{number} {} object={id} views={}", String::from_utf8_lossy(&room.name.0),
                    room.views_enabled);
            }
        }
    }
    for needle in needles.iter().filter_map(|needle| needle.strip_prefix("object:")) {
        let ids: Vec<usize> = needle.split(':').map(|part| part.parse().unwrap()).collect();
        assert_eq!(ids.len(), 3, "object:ID:event:subevent");
        let object = assets.objects[ids[0]].as_ref().expect("object asset");
        for (sub, actions) in &object.events[ids[1]] {
            if *sub as usize != ids[2] { continue }
            for action in actions {
                for source in std::iter::once(&action.fn_code).chain(&action.param_strings) {
                    if source.0.is_empty() || source.0.as_ref() == b"0" { continue }
                    println!("O{} {} {}:{}: {}", ids[0], String::from_utf8_lossy(&object.name.0),
                        ids[1], sub, String::from_utf8_lossy(&source.0).chars().take(1800).collect::<String>());
                }
            }
        }
    }
    for needle in needles.iter().filter_map(|needle| needle.strip_prefix("event-call:")) {
        for (id, object) in assets.objects.iter().enumerate() {
            let Some(object) = object else { continue };
            for (event, list) in object.events.iter().enumerate() {
                for (sub, actions) in list {
                    if actions.iter().any(|action| std::iter::once(&action.fn_code)
                        .chain(&action.param_strings)
                        .any(|source| String::from_utf8_lossy(&source.0).contains(needle))) {
                        println!("O{id} {} {event}:{sub} calls {needle}", String::from_utf8_lossy(&object.name.0));
                    }
                }
            }
        }
    }
    for (id, script) in assets.scripts.iter().enumerate() {
        let Some(script) = script else { continue };
        let name = String::from_utf8_lossy(&script.name.0);
        let source = String::from_utf8_lossy(&script.source.0);
        for needle in &needles {
            if needle.starts_with("object:") || needle.starts_with("event-call:") || needle.starts_with("room-object:") { continue }
            if needle.parse::<usize>() == Ok(id) || needle.strip_prefix("script:") == Some(name.as_ref()) {
                println!("S{id} {name}: {}", source.chars().take(1800).collect::<String>());
                continue;
            }
            for (offset, _) in source.match_indices(needle).take(5) {
                let before = source[..offset].chars().rev().take(80).collect::<Vec<_>>();
                let after = source[offset..].chars().take(850).collect::<String>();
                println!("S{id} {name} [{needle}]: {}{after}", before.into_iter().rev().collect::<String>());
            }
        }
    }
}
