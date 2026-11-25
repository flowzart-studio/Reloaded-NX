//! Read-only asset/source inspection; never runs GML or writes game files.
fn inspect(label:&str,bytes:&[u8]) {
 let text=String::from_utf8_lossy(bytes);
 if !text.is_empty() {for fragment in text.split(';') {if fragment.contains("key_") && fragment.contains('=') && (fragment.contains("ord(") || fragment.contains("vk_")) && fragment.len()<300 {println!("KEY {label}: {}",fragment.trim());}}}
 for key in ["window_set_", "view_wview", "view_hview", "view_wport", "view_hport", "surface_resize", "Music_Intro", "musicaIntro", "file_find_first", "keyboard_check", "key_ini_"] {
  for (p,_) in text.match_indices(key).take(12) {
   let start=text[..p].char_indices().rev().nth(80).map_or(0,|(i,_)|i);
   let end=text[p..].char_indices().nth(200).map_or(text.len(),|(i,_)|p+i);
   println!("{label} key={key}: {}",text[start..end].replace('\n'," "));
  }
 }
}
fn main(){
 let mut bytes=std::fs::read(std::env::args().nth(1).expect("original EXE")).unwrap();
 let assets=gm8exe::reader::from_exe(&mut bytes,None::<fn(&str)>,false,false).unwrap();
 for id in [2usize,4,22,30,43,51,52,54,191] {let r=assets.rooms[id].as_ref().unwrap();
  println!("ROOM id={id} name={} size={}x{} views_enabled={}",String::from_utf8_lossy(&r.name.0),r.width,r.height,r.views_enabled);
  for (i,v) in r.views.iter().enumerate().filter(|(_,v)|v.visible){println!("VIEW id={i} src={},{},{}x{} port={},{},{}x{}",v.source_x,v.source_y,v.source_w,v.source_h,v.port_x,v.port_y,v.port_w,v.port_h);}
  inspect(&format!("room:{id}"),&r.creation_code.0);
 }
 for s in assets.scripts.iter().flatten(){inspect(&String::from_utf8_lossy(&s.name.0),&s.source.0);}
 for o in assets.objects.iter().flatten(){for (event,list) in o.events.iter().enumerate(){for (sub,actions) in list {for a in actions {
  let label=format!("{} event={event}:{sub}",String::from_utf8_lossy(&o.name.0));inspect(&label,&a.fn_code.0);for p in &a.param_strings{inspect(&label,&p.0);}
 }}}}
}
