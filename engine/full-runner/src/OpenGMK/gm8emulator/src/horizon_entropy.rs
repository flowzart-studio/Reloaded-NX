//! Uses the same libnx csrng adapter as Rust std; never synthesizes entropy.
fn fill(bytes: &mut [u8]) -> Result<(),getrandom::Error> {
    unsafe extern "C" { fn getrandom(buffer: *mut u8, length: usize, flags: u32) -> isize; }
    let result=unsafe { getrandom(bytes.as_mut_ptr(),bytes.len(),0) };
    if result==bytes.len() as isize { Ok(()) }
    else { Err(getrandom::Error::from(core::num::NonZeroU32::new(getrandom::Error::CUSTOM_START).unwrap())) }
}
getrandom::register_custom_getrandom!(fill);
