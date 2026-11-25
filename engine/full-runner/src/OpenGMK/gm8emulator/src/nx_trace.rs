//! Reloaded-NX optional, read-only diagnostics. Never writes game state or injects input.
use crate::game::{Game,GetAsset};
use std::{sync::atomic::{AtomicU64,Ordering},path::Path};
static FRAME:AtomicU64=AtomicU64::new(0);
pub fn enabled()->bool {std::env::var_os("RELOADED_NX_TRACE").is_some()}
pub fn step(g:&Game) {
    if !enabled(){return}
    let n=FRAME.fetch_add(1,Ordering::Relaxed)+1;
    if n%15!=0 {return}
    let mut actors=Vec::new();
    let mut iter=g.room.instance_list.iter_by_drawing();
    while let Some(h)=iter.next(&g.room.instance_list) {
        let i=g.room.instance_list.get(h);
        if let Some(o)=g.assets.objects.get_asset(i.object_index.get()) {
            if o.name.as_ref()==b"obj_Prota" {
                actors.push(serde_json::json!({"id":i.id.get(),"object":i.object_index.get(),"name":"obj_Prota","x":i.x.get().into_inner(),"y":i.y.get().into_inner(),"sprite":i.sprite_index.get(),"direction":i.direction.get().into_inner()}));
            }
        }
    }
    eprintln!("NX_STEP {}",serde_json::json!({"n":n,"room":g.room.id,"actors":actors,"keys":{"left":g.input.keyboard_check(37),"up":g.input.keyboard_check(38),"right":g.input.keyboard_check(39),"down":g.input.keyboard_check(40)}}));
    if let Some(dir)=std::env::var_os("RELOADED_NX_CAPTURE") {
        let dir=Path::new(&dir);
        if dir.join("capture.request").is_file() {
            let (w,h)=(g.unscaled_width,g.unscaled_height);
            let bytes=g.renderer.get_pixels(0,0,w as i32,h as i32).into_vec();
            if let Some(im)=image::RgbaImage::from_raw(w,h,bytes) {
                let im=image::imageops::flip_vertical(&im);
                let file=dir.join(format!("frame-{n}-room-{}.png",g.room.id));
                if let Err(e)=im.save(&file){eprintln!("NX_CAPTURE_ERROR {e}")}else{eprintln!("NX_CAPTURE {}",file.display());}
            }
            let _=std::fs::remove_file(dir.join("capture.request"));
        }
    }
}
