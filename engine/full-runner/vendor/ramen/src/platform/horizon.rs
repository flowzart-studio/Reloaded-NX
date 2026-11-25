//! Native Horizon window backed by libnx NWindow; no X11 emulation.
use crate::{error::Error,event::Event,input::{Key, MouseButton},window::Builder};
use std::{ffi::c_void,cell::Cell};

#[repr(C)]
#[derive(Clone, Copy, Default)]
struct PointerSample {
    x: u16,
    y: u16,
    flags: u8,
    reserved: u8,
}
const _: [(); 6] = [(); std::mem::size_of::<PointerSample>()];
const POINTER_VALID: u8 = 1;
const POINTER_DOWN: u8 = 2;

extern "C" {
    fn nx_window_open(w:u32,h:u32)->i32;
    fn nx_window_resize(w:u32,h:u32)->i32;
    fn nx_window_handle()->*mut c_void;
    fn nx_window_poll(keys: &mut [u64; 4], pointer: &mut PointerSample) -> bool;
}
fn vk_to_key(vk: u8) -> Option<Key> {
    Some(match vk {
        8 => Key::Backspace, 13 => Key::Return, 16 | 160 => Key::LeftShift,
        17 => Key::LeftControl, 18 => Key::LeftAlt, 19 => Key::Pause, 27 => Key::Escape,
        32 => Key::Space, 33 => Key::PageUp, 34 => Key::PageDown, 35 => Key::End, 36 => Key::Home,
        37 => Key::LeftArrow, 38 => Key::UpArrow, 39 => Key::RightArrow, 40 => Key::DownArrow,
        44 => Key::PrintScreen, 45 => Key::Insert, 46 => Key::Delete,
        48..=57 => [Key::Alpha0, Key::Alpha1, Key::Alpha2, Key::Alpha3, Key::Alpha4, Key::Alpha5, Key::Alpha6, Key::Alpha7, Key::Alpha8, Key::Alpha9][usize::from(vk - 48)],
        65..=90 => [Key::A, Key::B, Key::C, Key::D, Key::E, Key::F, Key::G, Key::H, Key::I, Key::J, Key::K, Key::L, Key::M,
                    Key::N, Key::O, Key::P, Key::Q, Key::R, Key::S, Key::T, Key::U, Key::V, Key::W, Key::X, Key::Y, Key::Z][usize::from(vk - 65)],
        96..=105 => [Key::Keypad0, Key::Keypad1, Key::Keypad2, Key::Keypad3, Key::Keypad4, Key::Keypad5, Key::Keypad6, Key::Keypad7, Key::Keypad8, Key::Keypad9][usize::from(vk - 96)],
        106 => Key::KeypadMultiply, 107 => Key::KeypadAdd, 109 => Key::KeypadSubtract,
        110 => Key::KeypadDecimal, 111 => Key::KeypadDivide,
        112..=123 => [Key::F1, Key::F2, Key::F3, Key::F4, Key::F5, Key::F6, Key::F7, Key::F8, Key::F9, Key::F10, Key::F11, Key::F12][usize::from(vk - 112)],
        161 => Key::RightShift,
        _ => return None,
    })
}
pub struct Connection;
impl Connection { pub(crate) fn new()->Result<Self,Error>{Ok(Self)} }
pub struct Window { buffer:Vec<Event>, previous:[u64;4], pointer:PointerSample, size:Cell<(u16,u16)>, pending_resize:Cell<bool> }
impl Window {
 pub(crate) fn new(builder:Builder)->Result<Self,Error>{
    let (w,h)=builder.size;
    if unsafe{nx_window_open(w as u32,h as u32)}!=0{return Err(Error::Unsupported)}
    Ok(Self{buffer:Vec::new(),previous:[0;4],pointer:PointerSample::default(),size:Cell::new((w,h)),pending_resize:Cell::new(true)})
 }
 pub(crate) fn events(&self)->&[Event]{&self.buffer}
 pub(crate) fn poll_events(&mut self){
    self.buffer.clear();
    let mut current = [0; 4];
    let mut pointer = PointerSample::default();
    if unsafe { nx_window_poll(&mut current, &mut pointer) } { self.buffer.push(Event::CloseRequest); }
    if self.pending_resize.replace(false){self.buffer.push(Event::Resize(self.size.get()));}
    let position = (pointer.x, pointer.y);
    if pointer.flags & POINTER_VALID != 0 {
        // Horizon mouse events carry coordinates in the HID reference grid.
        self.buffer.push(Event::MouseMove(position));
    }
    if (pointer.flags ^ self.pointer.flags) & POINTER_DOWN != 0 {
        self.buffer.push(if pointer.flags & POINTER_DOWN != 0 {
            Event::MouseDown(MouseButton::Left)
        } else {
            Event::MouseUp(MouseButton::Left)
        });
    }
    self.pointer = pointer;
    for (word, (&now, &before)) in current.iter().zip(&self.previous).enumerate() {
        let mut changed = now ^ before;
        while changed != 0 {
            let bit = changed.trailing_zeros();
            changed &= changed - 1;
            if let Some(key) = vk_to_key((word * 64 + bit as usize) as u8) {
                self.buffer.push(if now & (1 << bit) != 0 { Event::KeyboardDown(key) } else { Event::KeyboardUp(key) });
            }
        }
    }
    self.previous=current;
 }
 pub(crate) fn set_size(&self,size:(u16,u16)){
    if size!=self.size.get() && unsafe{nx_window_resize(size.0 as u32,size.1 as u32)}==0 {self.size.set(size);self.pending_resize.set(true);}
 }
 pub(crate) fn native_window(&self)->*mut c_void{unsafe{nx_window_handle()}}
 // Desktop decorations have no meaning on Horizon's single application surface.
 pub(crate) fn set_maximised(&self,_v:bool){}
 pub(crate) fn set_position(&self,_v:(i16,i16)){}
 pub(crate) fn set_borderless(&self,_v:bool){}
 pub(crate) fn set_resizable(&self,_v:bool){}
 pub(crate) fn set_title(&self,_v:&str){}
 pub(crate) fn set_visible(&self,_v:bool){}
}
