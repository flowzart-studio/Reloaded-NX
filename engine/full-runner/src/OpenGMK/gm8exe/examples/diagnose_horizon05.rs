//! Read-only inspection of the original GML resource loading chain.
fn inspect(label:&str,data:&[u8]){let text=String::from_utf8_lossy(data);for key in ["pkmnbattle",".gmspr","sprite_add_sprite","sprite_replace_sprite"]{
 for (p,_) in text.match_indices(key).take(5){let a=text[..p].char_indices().rev().nth(100).map_or(0,|(i,_)|i);let b=text[p..].char_indices().nth(200).map_or(text.len(),|(i,_)|p+i);println!("{label} key={key}: {}",text[a..b].replace('\n'," "));}
}}
fn main(){let mut bytes=std::fs::read(std::env::args().nth(1).expect("original EXE")).unwrap();let assets=gm8exe::reader::from_exe(&mut bytes,None::<fn(&str)>,false,false).unwrap();
 for s in assets.scripts.iter().flatten(){inspect(&String::from_utf8_lossy(&s.name.0),&s.source.0);}
 for o in assets.objects.iter().flatten(){for (event,list) in o.events.iter().enumerate(){for (sub,actions) in list{for a in actions{let label=format!("{}:{event}:{sub}",String::from_utf8_lossy(&o.name.0));inspect(&label,&a.fn_code.0);for p in &a.param_strings{inspect(&label,&p.0);}}}}}
}
