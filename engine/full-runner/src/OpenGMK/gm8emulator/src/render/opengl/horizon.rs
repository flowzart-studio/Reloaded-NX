//! EGL/OpenGL 3.3 core on libnx. Required symbols are loaded without substitutions.
use ramen::{connection::Connection,window::Window};
use std::{ffi::{c_void,CString},cell::Cell};
#[path="../horizon_geometry.rs"] pub mod geometry;
extern "C" {
 fn nx_gl_open(window:*mut c_void)->i32;
 fn nx_gl_close();
 fn nx_gl_proc(name:*const i8)->*const c_void;
 fn nx_gl_swap()->i32;
 fn nx_gl_interval(interval:u32)->i32;
 fn nx_gl_wait_vsync()->i32;
 fn nx_gl_drawable_size(w:*mut u32,h:*mut u32)->i32;
}
pub struct PlatformImpl { interval:Cell<u32> }
impl PlatformImpl {
 pub fn drawable_size(&self)->(u32,u32){let(mut w,mut h)=(0,0);if unsafe{nx_gl_drawable_size(&mut w,&mut h)}!=0{panic!("Horizon EGL drawable query failed")} (w,h)}
 pub unsafe fn new(_connection:&Connection,window:&Window)->Result<Self,String>{
    if nx_gl_open(window.native_window())!=0{return Err("Horizon EGL context initialization failed; see runner.log".into());}
    Ok(Self{interval:Cell::new(1)})
 }
 pub unsafe fn get_function_loader()->Result<Box<dyn FnMut(&'static str)->*const c_void>,String>{
    Ok(Box::new(|name| {let name=CString::new(name).unwrap();nx_gl_proc(name.as_ptr().cast())}))
 }
 pub unsafe fn clean_function_loader(){}
 pub unsafe fn swap_buffers(&self){
    {
        let _trace = crate::battle_trace::Scope::new(crate::battle_trace::Phase::Swap);
        if nx_gl_swap()!=0{panic!("eglSwapBuffers failed")}
    }
    crate::battle_trace::present();
    crate::horizon_perf::present();
 }
 pub unsafe fn set_swap_interval(&self,n:u32)->bool{if nx_gl_interval(n)==0{self.interval.set(n);true}else{false}}
 pub unsafe fn get_swap_interval(&self)->u32{self.interval.get()}
 pub unsafe fn wait_vsync(&self){
    let _trace = crate::battle_trace::Scope::new(crate::battle_trace::Phase::Vsync);
    if nx_gl_wait_vsync()!=0{panic!("Horizon display vsync wait failed")}
 }
}
impl Drop for PlatformImpl {fn drop(&mut self){unsafe{nx_gl_close()}}}
