//! Map the GM virtual window to the real fullscreen drawable, without an
//! intermediate low-resolution render target. GM decides its own inner region.
pub fn map_rect(logical:(u32,u32),drawable:(u32,u32),rect:(i32,i32,i32,i32))->(i32,i32,i32,i32) {
    let (lw,lh)=(i64::from(logical.0),i64::from(logical.1));
    let (dw,dh)=(i64::from(drawable.0),i64::from(drawable.1));
    if lw==0 || lh==0 || dw==0 || dh==0 {return (0,0,0,0)}
    let (num,den)=if dw*lh<=dh*lw {(dw,lw)}else{(dh,lh)};
    let (outer_w,outer_h)=(lw*num/den,lh*num/den);
    let (ox,oy)=((dw-outer_w)/2,(dh-outer_h)/2);
    let (x,y,w,h)=rect;
    // Transform endpoints together so adjacent edges stay coincident.
    let x0=i64::from(x)*num/den;let y0=i64::from(y)*num/den;
    let x1=(i64::from(x)+i64::from(w))*num/den;let y1=(i64::from(y)+i64::from(h))*num/den;
    ((ox+x0) as i32,(oy+y0) as i32,(x1-x0) as i32,(y1-y0) as i32)
}

const HID: (i64, i64) = (1280, 720);

#[derive(Clone, Copy)]
pub struct PointerGeometry {
    pub drawable: (u32, u32),
    pub rect: (i32, i32, i32, i32),
    pub framebuffer: (u32, u32),
}

impl PointerGeometry {
    pub fn bounds(self) -> (u16, u16, u16, u16) {
        let (x, y, w, h) = self.rect;
        let edge = |point: i32, reference: i64, drawable: u32| {
            let drawable = i64::from(drawable);
            ((i64::from(point.max(0)) * reference + drawable - 1) / drawable)
                .min(reference) as u16
        };
        let (dw, dh) = self.drawable;
        (edge(x, HID.0, dw), edge(y, HID.1, dh),
         edge(x + w, HID.0, dw), edge(y + h, HID.1, dh))
    }

    pub fn to_game(self, point: (u16, u16)) -> (i32, i32) {
        let axis = |point: u16, origin: i32, length: i32, reference: i64, drawable: u32, fb: u32| {
            let offset = i64::from(point) * i64::from(drawable) - i64::from(origin) * reference;
            (offset * i64::from(fb) / (i64::from(length) * reference))
                .clamp(0, i64::from(fb) - 1) as i32
        };
        let (x, y, w, h) = self.rect;
        (axis(point.0, x, w, HID.0, self.drawable.0, self.framebuffer.0),
         axis(point.1, y, h, HID.1, self.drawable.1, self.framebuffer.1))
    }

    pub fn to_hid(self, point: (i32, i32)) -> (u16, u16) {
        let axis = |point: i32, origin: i32, length: i32, reference: i64, drawable: u32, fb: u32| {
            let center = i64::from(origin) as f64 + (f64::from(point) + 0.5) * f64::from(length) / f64::from(fb);
            (center * reference as f64 / f64::from(drawable)) as u16
        };
        let (x, y, w, h) = self.rect;
        let (left, top, right, bottom) = self.bounds();
        (axis(point.0, x, w, HID.0, self.drawable.0, self.framebuffer.0).clamp(left, right - 1),
         axis(point.1, y, h, HID.1, self.drawable.1, self.framebuffer.1).clamp(top, bottom - 1))
    }
}
#[cfg(test)]mod tests{use super::*;
 #[test]fn exploration_battle_exploration_has_no_low_resolution_stage(){
  for fb in [(320,180),(576,324),(320,180)] {
   let rect=map_rect((320,180),(1280,720),(0,0,320,180));
   assert_eq!(rect,(0,0,1280,720));assert!(rect.2>=fb.0 && rect.3>=fb.1);
  }
 }
 #[test]fn four_by_three_window_is_letterboxed_and_restores_wide(){
  assert_eq!(map_rect((640,480),(1280,720),(0,0,640,480)),(160,0,960,720));
  assert_eq!(map_rect((320,180),(1280,720),(0,0,320,180)),(0,0,1280,720));
 }
 #[test]fn fixed_scale_and_gm_inner_letterbox_are_preserved(){
  assert_eq!(map_rect((640,360),(1280,720),(0,0,640,360)),(0,0,1280,720));
  assert_eq!(map_rect((320,180),(1280,720),(40,0,240,180)),(160,0,960,720));
 }
 #[test]fn native_dimensions_are_queried_not_assumed(){
  assert_eq!(map_rect((320,180),(1920,1080),(0,0,320,180)),(0,0,1920,1080));
  assert_eq!(map_rect((320,180),(1024,768),(0,0,320,180)),(0,96,1024,576));
  assert_eq!(map_rect((0,0),(1280,720),(0,0,0,0)),(0,0,0,0));
 }
 #[test]
 fn pointer_projects_both_framebuffers_through_full_and_aspect_rects() {
    let logical = (640, 480);
    for drawable in [(1280, 720), (1920, 1080), (1024, 768)] {
        for framebuffer in [(320, 180), (576, 324)] {
            for inner in [(0, 0, 640, 480), (0, 60, 640, 360)] {
                let rect = map_rect(logical, drawable, inner);
                let g = PointerGeometry { drawable, rect, framebuffer };
                let (left, top, right, bottom) = g.bounds();
                assert!(left < right && top < bottom);
                assert_eq!(g.to_game((left, top)), (0, 0));
                assert_eq!(g.to_game((right - 1, bottom - 1)), (framebuffer.0 as i32 - 1, framebuffer.1 as i32 - 1));
                assert_eq!(g.to_game((640, 360)), (framebuffer.0 as i32 / 2, framebuffer.1 as i32 / 2));
                let slot = (147, 69);
                assert_eq!(g.to_game(g.to_hid(slot)), slot);
            }
        }
    }
 }
}
