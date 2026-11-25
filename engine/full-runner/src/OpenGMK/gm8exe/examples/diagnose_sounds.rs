//! Read-only inspection of the two reported embedded sounds; never executes GML.
fn main(){let path=std::env::args().nth(1).expect("original EXE path");let mut bytes=std::fs::read(path).unwrap();let assets=gm8exe::reader::from_exe(&mut bytes,None::<fn(&str)>,false,false).unwrap();
 for sound in assets.sounds.iter().flatten(){let name=String::from_utf8_lossy(&sound.name.0);if name!="snd_computer_open"&&name!="snd_computer_close"{continue}
  if let Some(b)=&sound.data{println!("sound={name} bytes={} header={:02x?}",b.len(),&b[..b.len().min(48)]);
   let mut p=12usize;while p+8<=b.len(){let n=u32::from_le_bytes(b[p+4..p+8].try_into().unwrap()) as usize;println!("chunk={:?} offset={p} len={n}",String::from_utf8_lossy(&b[p..p+4]));if &b[p..p+4]==b"fmt "&&n>=16&&p+24<=b.len(){println!("format={} channels={} rate={} bits={}",u16::from_le_bytes(b[p+8..p+10].try_into().unwrap()),u16::from_le_bytes(b[p+10..p+12].try_into().unwrap()),u32::from_le_bytes(b[p+12..p+16].try_into().unwrap()),u16::from_le_bytes(b[p+22..p+24].try_into().unwrap()));}p=match p.checked_add(8+n+(n&1)){Some(next)=>next,None=>break};}
  }else{println!("sound={name} no data");}
 }
}
