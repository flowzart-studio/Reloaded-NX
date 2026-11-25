//! Read-only source inspection. Never executes, writes, or exports the game.
fn inspect(label: &str, source: &[u8]) {
    let source = String::from_utf8_lossy(source);
    for (line, text) in source.lines().enumerate() {
        if ["file_bin_size", "file_bin_seek", "file_text_eof", "file_text_eoln", "Music_Intro", "snd_Alba", "snd_in", "snd_out", "reloaded2", "file_bin_open", "file_text_open"].iter().any(|s| text.contains(s)) {
            println!("{label}:{}: {}", line + 1, text.trim());
        }
    }
}
fn main() {
    let mut bytes=std::fs::read(std::env::args().nth(1).expect("original EXE")).unwrap();
    let assets=gm8exe::reader::from_exe(&mut bytes,None::<fn(&str)>,false,false).unwrap();
    for (id,sound) in assets.sounds.iter().enumerate() {if let Some(sound)=sound {if id<=8 {println!("SOUND id={id} name={:?} bytes={}",String::from_utf8_lossy(&sound.name.0),sound.data.as_ref().map_or(0,|b|b.len()));}}}
    for o in assets.objects.iter().flatten() {let name=String::from_utf8_lossy(&o.name.0);if ["escaler","stair"].iter().any(|key|name.to_lowercase().contains(key)) {for (e,list) in o.events.iter().enumerate(){for (sub,actions) in list {for a in actions {println!("STAIRS object={name} event={e}:{sub} fn={} params={:?}",String::from_utf8_lossy(&a.fn_code.0),a.param_strings.iter().map(|p|String::from_utf8_lossy(&p.0)).collect::<Vec<_>>());}}}}}
    for s in assets.scripts.iter().flatten() { inspect(&String::from_utf8_lossy(&s.name.0), &s.source.0); }
    for o in assets.objects.iter().flatten() { for (event,list) in o.events.iter().enumerate() { for (sub,actions) in list { for a in actions {
        let label=format!("{} event={event}:{sub}",String::from_utf8_lossy(&o.name.0));
        inspect(&label,&a.fn_code.0);for p in &a.param_strings { inspect(&label,&p.0); }
    }}}}
}
