// Generated from the bundled MIT glcorearb.h; see licenses/Khronos-header-MIT.txt.
// Original loader/wrappers: MIT, RELOADED-NX contributors. No gl-rs templates.
pub mod types {
    pub type GLenum = u32;
    pub type GLint = i32;
    pub type GLsizei = i32;
    pub type GLbitfield = u32;
    pub type GLdouble = f64;
    pub type GLuint = u32;
    pub type GLboolean = u8;
    pub type GLubyte = u8;
    pub type GLfloat = f32;
    pub type GLchar = std::os::raw::c_char;
    pub type GLintptr = isize;
    pub type GLsizeiptr = isize;
}
use self::types::*;
pub const ARRAY_BUFFER: GLenum = 0x8892;
pub const ARRAY_BUFFER_BINDING: GLenum = 0x8894;
pub const BACK: GLenum = 0x0405;
pub const BGRA: GLenum = 0x80E1;
pub const BLEND: GLenum = 0x0BE2;
pub const CLAMP_TO_EDGE: GLenum = 0x812F;
pub const COLOR_ATTACHMENT0: GLenum = 0x8CE0;
pub const COLOR_BUFFER_BIT: GLenum = 0x00004000;
pub const COMPILE_STATUS: GLenum = 0x8B81;
pub const CULL_FACE: GLenum = 0x0B44;
pub const CURRENT_PROGRAM: GLenum = 0x8B8D;
pub const DEPTH_ATTACHMENT: GLenum = 0x8D00;
pub const DEPTH_BUFFER_BIT: GLenum = 0x00000100;
pub const DEPTH_COMPONENT: GLenum = 0x1902;
pub const DEPTH_COMPONENT16: GLenum = 0x81A5;
pub const DEPTH_COMPONENT24: GLenum = 0x81A6;
pub const DEPTH_TEST: GLenum = 0x0B71;
pub const DRAW_FRAMEBUFFER: GLenum = 0x8CA9;
pub const DRAW_FRAMEBUFFER_BINDING: GLenum = 0x8CA6;
pub const DST_ALPHA: GLenum = 0x0304;
pub const DST_COLOR: GLenum = 0x0306;
pub const FALSE: GLboolean = 0;
pub const FIRST_VERTEX_CONVENTION: GLenum = 0x8E4D;
pub const FLOAT: GLenum = 0x1406;
pub const FRAGMENT_SHADER: GLenum = 0x8B30;
pub const FRAMEBUFFER: GLenum = 0x8D40;
pub const FRAMEBUFFER_COMPLETE: GLenum = 0x8CD5;
pub const INFO_LOG_LENGTH: GLenum = 0x8B84;
pub const INVALID_INDEX: GLenum = 0xFFFFFFFF;
pub const LEQUAL: GLenum = 0x0203;
pub const LINEAR: GLenum = 0x2601;
pub const LINES: GLenum = 0x0001;
pub const LINK_STATUS: GLenum = 0x8B82;
pub const MAJOR_VERSION: GLenum = 0x821B;
pub const MAX_TEXTURE_SIZE: GLenum = 0x0D33;
pub const MINOR_VERSION: GLenum = 0x821C;
pub const NEAREST: GLenum = 0x2600;
pub const ONE: GLenum = 1;
pub const ONE_MINUS_DST_ALPHA: GLenum = 0x0305;
pub const ONE_MINUS_DST_COLOR: GLenum = 0x0307;
pub const ONE_MINUS_SRC_ALPHA: GLenum = 0x0303;
pub const ONE_MINUS_SRC_COLOR: GLenum = 0x0301;
pub const PACK_ALIGNMENT: GLenum = 0x0D05;
pub const POINTS: GLenum = 0x0000;
pub const READ_FRAMEBUFFER: GLenum = 0x8CA8;
pub const READ_FRAMEBUFFER_BINDING: GLenum = 0x8CAA;
pub const REPEAT: GLenum = 0x2901;
pub const RGBA: GLenum = 0x1908;
pub const SCISSOR_TEST: GLenum = 0x0C11;
pub const SRC_ALPHA: GLenum = 0x0302;
pub const SRC_ALPHA_SATURATE: GLenum = 0x0308;
pub const SRC_COLOR: GLenum = 0x0300;
pub const STATIC_DRAW: GLenum = 0x88E4;
pub const STREAM_DRAW: GLenum = 0x88E0;
pub const TEXTURE0: GLenum = 0x84C0;
pub const TEXTURE_2D: GLenum = 0x0DE1;
pub const TEXTURE_BINDING_2D: GLenum = 0x8069;
pub const TEXTURE_HEIGHT: GLenum = 0x1001;
pub const TEXTURE_MAG_FILTER: GLenum = 0x2800;
pub const TEXTURE_MIN_FILTER: GLenum = 0x2801;
pub const TEXTURE_WIDTH: GLenum = 0x1000;
pub const TEXTURE_WRAP_S: GLenum = 0x2802;
pub const TEXTURE_WRAP_T: GLenum = 0x2803;
pub const TRIANGLES: GLenum = 0x0004;
pub const TRUE: GLboolean = 1;
pub const UNIFORM_BUFFER: GLenum = 0x8A11;
pub const UNSIGNED_BYTE: GLenum = 0x1401;
pub const UNSIGNED_INT: GLenum = 0x1405;
pub const UNSIGNED_SHORT: GLenum = 0x1403;
pub const VENDOR: GLenum = 0x1F00;
pub const VERSION: GLenum = 0x1F02;
pub const VERTEX_ARRAY_BINDING: GLenum = 0x85B5;
pub const VERTEX_SHADER: GLenum = 0x8B31;
pub const VIEWPORT: GLenum = 0x0BA2;
pub const ZERO: GLenum = 0;
#[derive(Clone)]
pub struct Gl {
    ActiveTexture: Option<unsafe extern "system" fn(GLenum) -> ()>,
    AttachShader: Option<unsafe extern "system" fn(GLuint, GLuint) -> ()>,
    BindBuffer: Option<unsafe extern "system" fn(GLenum, GLuint) -> ()>,
    BindBufferBase: Option<unsafe extern "system" fn(GLenum, GLuint, GLuint) -> ()>,
    BindFramebuffer: Option<unsafe extern "system" fn(GLenum, GLuint) -> ()>,
    BindTexture: Option<unsafe extern "system" fn(GLenum, GLuint) -> ()>,
    BindVertexArray: Option<unsafe extern "system" fn(GLuint) -> ()>,
    BlendFunc: Option<unsafe extern "system" fn(GLenum, GLenum) -> ()>,
    BlitFramebuffer: Option<unsafe extern "system" fn(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum) -> ()>,
    BufferData: Option<unsafe extern "system" fn(GLenum, GLsizeiptr, *const std::ffi::c_void, GLenum) -> ()>,
    CheckFramebufferStatus: Option<unsafe extern "system" fn(GLenum) -> GLenum>,
    Clear: Option<unsafe extern "system" fn(GLbitfield) -> ()>,
    ClearColor: Option<unsafe extern "system" fn(GLfloat, GLfloat, GLfloat, GLfloat) -> ()>,
    CompileShader: Option<unsafe extern "system" fn(GLuint) -> ()>,
    CopyTexImage2D: Option<unsafe extern "system" fn(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint) -> ()>,
    CreateProgram: Option<unsafe extern "system" fn() -> GLuint>,
    CreateShader: Option<unsafe extern "system" fn(GLenum) -> GLuint>,
    DeleteFramebuffers: Option<unsafe extern "system" fn(GLsizei, *const GLuint) -> ()>,
    DeleteShader: Option<unsafe extern "system" fn(GLuint) -> ()>,
    DeleteTextures: Option<unsafe extern "system" fn(GLsizei, *const GLuint) -> ()>,
    DepthFunc: Option<unsafe extern "system" fn(GLenum) -> ()>,
    DepthMask: Option<unsafe extern "system" fn(GLboolean) -> ()>,
    Disable: Option<unsafe extern "system" fn(GLenum) -> ()>,
    DrawArrays: Option<unsafe extern "system" fn(GLenum, GLint, GLsizei) -> ()>,
    Enable: Option<unsafe extern "system" fn(GLenum) -> ()>,
    EnableVertexAttribArray: Option<unsafe extern "system" fn(GLuint) -> ()>,
    FramebufferTexture2D: Option<unsafe extern "system" fn(GLenum, GLenum, GLenum, GLuint, GLint) -> ()>,
    GenBuffers: Option<unsafe extern "system" fn(GLsizei, *mut GLuint) -> ()>,
    GenFramebuffers: Option<unsafe extern "system" fn(GLsizei, *mut GLuint) -> ()>,
    GenTextures: Option<unsafe extern "system" fn(GLsizei, *mut GLuint) -> ()>,
    GenVertexArrays: Option<unsafe extern "system" fn(GLsizei, *mut GLuint) -> ()>,
    GetError: Option<unsafe extern "system" fn() -> GLenum>,
    GetIntegerv: Option<unsafe extern "system" fn(GLenum, *mut GLint) -> ()>,
    GetProgramInfoLog: Option<unsafe extern "system" fn(GLuint, GLsizei, *mut GLsizei, *mut GLchar) -> ()>,
    GetProgramiv: Option<unsafe extern "system" fn(GLuint, GLenum, *mut GLint) -> ()>,
    GetShaderInfoLog: Option<unsafe extern "system" fn(GLuint, GLsizei, *mut GLsizei, *mut GLchar) -> ()>,
    GetShaderiv: Option<unsafe extern "system" fn(GLuint, GLenum, *mut GLint) -> ()>,
    GetString: Option<unsafe extern "system" fn(GLenum) -> *const GLubyte>,
    GetTexImage: Option<unsafe extern "system" fn(GLenum, GLint, GLenum, GLenum, *mut std::ffi::c_void) -> ()>,
    GetTexLevelParameteriv: Option<unsafe extern "system" fn(GLenum, GLint, GLenum, *mut GLint) -> ()>,
    GetUniformBlockIndex: Option<unsafe extern "system" fn(GLuint, *const GLchar) -> GLuint>,
    GetUniformLocation: Option<unsafe extern "system" fn(GLuint, *const GLchar) -> GLint>,
    IsEnabled: Option<unsafe extern "system" fn(GLenum) -> GLboolean>,
    LinkProgram: Option<unsafe extern "system" fn(GLuint) -> ()>,
    PixelStorei: Option<unsafe extern "system" fn(GLenum, GLint) -> ()>,
    ProvokingVertex: Option<unsafe extern "system" fn(GLenum) -> ()>,
    ReadBuffer: Option<unsafe extern "system" fn(GLenum) -> ()>,
    ReadPixels: Option<unsafe extern "system" fn(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, *mut std::ffi::c_void) -> ()>,
    Scissor: Option<unsafe extern "system" fn(GLint, GLint, GLsizei, GLsizei) -> ()>,
    ShaderSource: Option<unsafe extern "system" fn(GLuint, GLsizei, *const *const GLchar, *const GLint) -> ()>,
    TexImage2D: Option<unsafe extern "system" fn(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, *const std::ffi::c_void) -> ()>,
    TexParameteri: Option<unsafe extern "system" fn(GLenum, GLenum, GLint) -> ()>,
    TexSubImage2D: Option<unsafe extern "system" fn(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, *const std::ffi::c_void) -> ()>,
    Uniform1f: Option<unsafe extern "system" fn(GLint, GLfloat) -> ()>,
    Uniform1i: Option<unsafe extern "system" fn(GLint, GLint) -> ()>,
    Uniform2f: Option<unsafe extern "system" fn(GLint, GLfloat, GLfloat) -> ()>,
    UniformBlockBinding: Option<unsafe extern "system" fn(GLuint, GLuint, GLuint) -> ()>,
    UseProgram: Option<unsafe extern "system" fn(GLuint) -> ()>,
    VertexAttribPointer: Option<unsafe extern "system" fn(GLuint, GLint, GLenum, GLboolean, GLsizei, *const std::ffi::c_void) -> ()>,
    Viewport: Option<unsafe extern "system" fn(GLint, GLint, GLsizei, GLsizei) -> ()>,
}
impl Gl {
    pub fn load_with<F: FnMut(&'static str) -> *const std::ffi::c_void>(mut resolver: F) -> Self {
        Self {
            ActiveTexture: {
                let mut address = std::ptr::null();
                for symbol in ["glActiveTexture", "glActiveTextureARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            AttachShader: {
                let mut address = std::ptr::null();
                for symbol in ["glAttachShader", "glAttachObjectARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            BindBuffer: {
                let mut address = std::ptr::null();
                for symbol in ["glBindBuffer", "glBindBufferARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            BindBufferBase: {
                let mut address = std::ptr::null();
                for symbol in ["glBindBufferBase", "glBindBufferBaseEXT", "glBindBufferBaseNV"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            BindFramebuffer: {
                let mut address = std::ptr::null();
                for symbol in ["glBindFramebuffer"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            BindTexture: {
                let mut address = std::ptr::null();
                for symbol in ["glBindTexture", "glBindTextureEXT"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            BindVertexArray: {
                let mut address = std::ptr::null();
                for symbol in ["glBindVertexArray", "glBindVertexArrayOES"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            BlendFunc: {
                let mut address = std::ptr::null();
                for symbol in ["glBlendFunc"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            BlitFramebuffer: {
                let mut address = std::ptr::null();
                for symbol in ["glBlitFramebuffer", "glBlitFramebufferEXT", "glBlitFramebufferNV"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            BufferData: {
                let mut address = std::ptr::null();
                for symbol in ["glBufferData", "glBufferDataARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            CheckFramebufferStatus: {
                let mut address = std::ptr::null();
                for symbol in ["glCheckFramebufferStatus", "glCheckFramebufferStatusEXT"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            Clear: {
                let mut address = std::ptr::null();
                for symbol in ["glClear"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            ClearColor: {
                let mut address = std::ptr::null();
                for symbol in ["glClearColor"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            CompileShader: {
                let mut address = std::ptr::null();
                for symbol in ["glCompileShader", "glCompileShaderARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            CopyTexImage2D: {
                let mut address = std::ptr::null();
                for symbol in ["glCopyTexImage2D", "glCopyTexImage2DEXT"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            CreateProgram: {
                let mut address = std::ptr::null();
                for symbol in ["glCreateProgram", "glCreateProgramObjectARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            CreateShader: {
                let mut address = std::ptr::null();
                for symbol in ["glCreateShader", "glCreateShaderObjectARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            DeleteFramebuffers: {
                let mut address = std::ptr::null();
                for symbol in ["glDeleteFramebuffers", "glDeleteFramebuffersEXT"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            DeleteShader: {
                let mut address = std::ptr::null();
                for symbol in ["glDeleteShader"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            DeleteTextures: {
                let mut address = std::ptr::null();
                for symbol in ["glDeleteTextures"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            DepthFunc: {
                let mut address = std::ptr::null();
                for symbol in ["glDepthFunc"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            DepthMask: {
                let mut address = std::ptr::null();
                for symbol in ["glDepthMask"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            Disable: {
                let mut address = std::ptr::null();
                for symbol in ["glDisable"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            DrawArrays: {
                let mut address = std::ptr::null();
                for symbol in ["glDrawArrays", "glDrawArraysEXT"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            Enable: {
                let mut address = std::ptr::null();
                for symbol in ["glEnable"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            EnableVertexAttribArray: {
                let mut address = std::ptr::null();
                for symbol in ["glEnableVertexAttribArray", "glEnableVertexAttribArrayARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            FramebufferTexture2D: {
                let mut address = std::ptr::null();
                for symbol in ["glFramebufferTexture2D", "glFramebufferTexture2DEXT"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GenBuffers: {
                let mut address = std::ptr::null();
                for symbol in ["glGenBuffers", "glGenBuffersARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GenFramebuffers: {
                let mut address = std::ptr::null();
                for symbol in ["glGenFramebuffers", "glGenFramebuffersEXT"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GenTextures: {
                let mut address = std::ptr::null();
                for symbol in ["glGenTextures"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GenVertexArrays: {
                let mut address = std::ptr::null();
                for symbol in ["glGenVertexArrays", "glGenVertexArraysAPPLE", "glGenVertexArraysOES"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GetError: {
                let mut address = std::ptr::null();
                for symbol in ["glGetError"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GetIntegerv: {
                let mut address = std::ptr::null();
                for symbol in ["glGetIntegerv"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GetProgramInfoLog: {
                let mut address = std::ptr::null();
                for symbol in ["glGetProgramInfoLog"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GetProgramiv: {
                let mut address = std::ptr::null();
                for symbol in ["glGetProgramiv"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GetShaderInfoLog: {
                let mut address = std::ptr::null();
                for symbol in ["glGetShaderInfoLog"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GetShaderiv: {
                let mut address = std::ptr::null();
                for symbol in ["glGetShaderiv"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GetString: {
                let mut address = std::ptr::null();
                for symbol in ["glGetString"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GetTexImage: {
                let mut address = std::ptr::null();
                for symbol in ["glGetTexImage"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GetTexLevelParameteriv: {
                let mut address = std::ptr::null();
                for symbol in ["glGetTexLevelParameteriv"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GetUniformBlockIndex: {
                let mut address = std::ptr::null();
                for symbol in ["glGetUniformBlockIndex"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            GetUniformLocation: {
                let mut address = std::ptr::null();
                for symbol in ["glGetUniformLocation", "glGetUniformLocationARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            IsEnabled: {
                let mut address = std::ptr::null();
                for symbol in ["glIsEnabled"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            LinkProgram: {
                let mut address = std::ptr::null();
                for symbol in ["glLinkProgram", "glLinkProgramARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            PixelStorei: {
                let mut address = std::ptr::null();
                for symbol in ["glPixelStorei"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            ProvokingVertex: {
                let mut address = std::ptr::null();
                for symbol in ["glProvokingVertex", "glProvokingVertexEXT"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            ReadBuffer: {
                let mut address = std::ptr::null();
                for symbol in ["glReadBuffer"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            ReadPixels: {
                let mut address = std::ptr::null();
                for symbol in ["glReadPixels"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            Scissor: {
                let mut address = std::ptr::null();
                for symbol in ["glScissor"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            ShaderSource: {
                let mut address = std::ptr::null();
                for symbol in ["glShaderSource", "glShaderSourceARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            TexImage2D: {
                let mut address = std::ptr::null();
                for symbol in ["glTexImage2D"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            TexParameteri: {
                let mut address = std::ptr::null();
                for symbol in ["glTexParameteri"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            TexSubImage2D: {
                let mut address = std::ptr::null();
                for symbol in ["glTexSubImage2D", "glTexSubImage2DEXT"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            Uniform1f: {
                let mut address = std::ptr::null();
                for symbol in ["glUniform1f", "glUniform1fARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            Uniform1i: {
                let mut address = std::ptr::null();
                for symbol in ["glUniform1i", "glUniform1iARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            Uniform2f: {
                let mut address = std::ptr::null();
                for symbol in ["glUniform2f", "glUniform2fARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            UniformBlockBinding: {
                let mut address = std::ptr::null();
                for symbol in ["glUniformBlockBinding"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            UseProgram: {
                let mut address = std::ptr::null();
                for symbol in ["glUseProgram", "glUseProgramObjectARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            VertexAttribPointer: {
                let mut address = std::ptr::null();
                for symbol in ["glVertexAttribPointer", "glVertexAttribPointerARB"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
            Viewport: {
                let mut address = std::ptr::null();
                for symbol in ["glViewport"] {
                    address = resolver(symbol);
                    if !address.is_null() { break; }
                }
                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }
            },
        }
    }
    pub unsafe fn ActiveTexture(&self, a0: GLenum) -> () {
        (self.ActiveTexture.expect("OpenGL entry point not loaded: glActiveTexture"))(a0)
    }
    pub unsafe fn AttachShader(&self, a0: GLuint, a1: GLuint) -> () {
        (self.AttachShader.expect("OpenGL entry point not loaded: glAttachShader"))(a0, a1)
    }
    pub unsafe fn BindBuffer(&self, a0: GLenum, a1: GLuint) -> () {
        (self.BindBuffer.expect("OpenGL entry point not loaded: glBindBuffer"))(a0, a1)
    }
    pub unsafe fn BindBufferBase(&self, a0: GLenum, a1: GLuint, a2: GLuint) -> () {
        (self.BindBufferBase.expect("OpenGL entry point not loaded: glBindBufferBase"))(a0, a1, a2)
    }
    pub unsafe fn BindFramebuffer(&self, a0: GLenum, a1: GLuint) -> () {
        (self.BindFramebuffer.expect("OpenGL entry point not loaded: glBindFramebuffer"))(a0, a1)
    }
    pub unsafe fn BindTexture(&self, a0: GLenum, a1: GLuint) -> () {
        (self.BindTexture.expect("OpenGL entry point not loaded: glBindTexture"))(a0, a1)
    }
    pub unsafe fn BindVertexArray(&self, a0: GLuint) -> () {
        (self.BindVertexArray.expect("OpenGL entry point not loaded: glBindVertexArray"))(a0)
    }
    pub unsafe fn BlendFunc(&self, a0: GLenum, a1: GLenum) -> () {
        (self.BlendFunc.expect("OpenGL entry point not loaded: glBlendFunc"))(a0, a1)
    }
    pub unsafe fn BlitFramebuffer(&self, a0: GLint, a1: GLint, a2: GLint, a3: GLint, a4: GLint, a5: GLint, a6: GLint, a7: GLint, a8: GLbitfield, a9: GLenum) -> () {
        (self.BlitFramebuffer.expect("OpenGL entry point not loaded: glBlitFramebuffer"))(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9)
    }
    pub unsafe fn BufferData(&self, a0: GLenum, a1: GLsizeiptr, a2: *const std::ffi::c_void, a3: GLenum) -> () {
        (self.BufferData.expect("OpenGL entry point not loaded: glBufferData"))(a0, a1, a2, a3)
    }
    pub unsafe fn CheckFramebufferStatus(&self, a0: GLenum) -> GLenum {
        (self.CheckFramebufferStatus.expect("OpenGL entry point not loaded: glCheckFramebufferStatus"))(a0)
    }
    pub unsafe fn Clear(&self, a0: GLbitfield) -> () {
        (self.Clear.expect("OpenGL entry point not loaded: glClear"))(a0)
    }
    pub unsafe fn ClearColor(&self, a0: GLfloat, a1: GLfloat, a2: GLfloat, a3: GLfloat) -> () {
        (self.ClearColor.expect("OpenGL entry point not loaded: glClearColor"))(a0, a1, a2, a3)
    }
    pub unsafe fn CompileShader(&self, a0: GLuint) -> () {
        (self.CompileShader.expect("OpenGL entry point not loaded: glCompileShader"))(a0)
    }
    pub unsafe fn CopyTexImage2D(&self, a0: GLenum, a1: GLint, a2: GLenum, a3: GLint, a4: GLint, a5: GLsizei, a6: GLsizei, a7: GLint) -> () {
        (self.CopyTexImage2D.expect("OpenGL entry point not loaded: glCopyTexImage2D"))(a0, a1, a2, a3, a4, a5, a6, a7)
    }
    pub unsafe fn CreateProgram(&self) -> GLuint {
        (self.CreateProgram.expect("OpenGL entry point not loaded: glCreateProgram"))()
    }
    pub unsafe fn CreateShader(&self, a0: GLenum) -> GLuint {
        (self.CreateShader.expect("OpenGL entry point not loaded: glCreateShader"))(a0)
    }
    pub unsafe fn DeleteFramebuffers(&self, a0: GLsizei, a1: *const GLuint) -> () {
        (self.DeleteFramebuffers.expect("OpenGL entry point not loaded: glDeleteFramebuffers"))(a0, a1)
    }
    pub unsafe fn DeleteShader(&self, a0: GLuint) -> () {
        (self.DeleteShader.expect("OpenGL entry point not loaded: glDeleteShader"))(a0)
    }
    pub unsafe fn DeleteTextures(&self, a0: GLsizei, a1: *const GLuint) -> () {
        (self.DeleteTextures.expect("OpenGL entry point not loaded: glDeleteTextures"))(a0, a1)
    }
    pub unsafe fn DepthFunc(&self, a0: GLenum) -> () {
        (self.DepthFunc.expect("OpenGL entry point not loaded: glDepthFunc"))(a0)
    }
    pub unsafe fn DepthMask(&self, a0: GLboolean) -> () {
        (self.DepthMask.expect("OpenGL entry point not loaded: glDepthMask"))(a0)
    }
    pub unsafe fn Disable(&self, a0: GLenum) -> () {
        (self.Disable.expect("OpenGL entry point not loaded: glDisable"))(a0)
    }
    pub unsafe fn DrawArrays(&self, a0: GLenum, a1: GLint, a2: GLsizei) -> () {
        (self.DrawArrays.expect("OpenGL entry point not loaded: glDrawArrays"))(a0, a1, a2)
    }
    pub unsafe fn Enable(&self, a0: GLenum) -> () {
        (self.Enable.expect("OpenGL entry point not loaded: glEnable"))(a0)
    }
    pub unsafe fn EnableVertexAttribArray(&self, a0: GLuint) -> () {
        (self.EnableVertexAttribArray.expect("OpenGL entry point not loaded: glEnableVertexAttribArray"))(a0)
    }
    pub unsafe fn FramebufferTexture2D(&self, a0: GLenum, a1: GLenum, a2: GLenum, a3: GLuint, a4: GLint) -> () {
        (self.FramebufferTexture2D.expect("OpenGL entry point not loaded: glFramebufferTexture2D"))(a0, a1, a2, a3, a4)
    }
    pub unsafe fn GenBuffers(&self, a0: GLsizei, a1: *mut GLuint) -> () {
        (self.GenBuffers.expect("OpenGL entry point not loaded: glGenBuffers"))(a0, a1)
    }
    pub unsafe fn GenFramebuffers(&self, a0: GLsizei, a1: *mut GLuint) -> () {
        (self.GenFramebuffers.expect("OpenGL entry point not loaded: glGenFramebuffers"))(a0, a1)
    }
    pub unsafe fn GenTextures(&self, a0: GLsizei, a1: *mut GLuint) -> () {
        (self.GenTextures.expect("OpenGL entry point not loaded: glGenTextures"))(a0, a1)
    }
    pub unsafe fn GenVertexArrays(&self, a0: GLsizei, a1: *mut GLuint) -> () {
        (self.GenVertexArrays.expect("OpenGL entry point not loaded: glGenVertexArrays"))(a0, a1)
    }
    pub unsafe fn GetError(&self) -> GLenum {
        (self.GetError.expect("OpenGL entry point not loaded: glGetError"))()
    }
    pub unsafe fn GetIntegerv(&self, a0: GLenum, a1: *mut GLint) -> () {
        (self.GetIntegerv.expect("OpenGL entry point not loaded: glGetIntegerv"))(a0, a1)
    }
    pub unsafe fn GetProgramInfoLog(&self, a0: GLuint, a1: GLsizei, a2: *mut GLsizei, a3: *mut GLchar) -> () {
        (self.GetProgramInfoLog.expect("OpenGL entry point not loaded: glGetProgramInfoLog"))(a0, a1, a2, a3)
    }
    pub unsafe fn GetProgramiv(&self, a0: GLuint, a1: GLenum, a2: *mut GLint) -> () {
        (self.GetProgramiv.expect("OpenGL entry point not loaded: glGetProgramiv"))(a0, a1, a2)
    }
    pub unsafe fn GetShaderInfoLog(&self, a0: GLuint, a1: GLsizei, a2: *mut GLsizei, a3: *mut GLchar) -> () {
        (self.GetShaderInfoLog.expect("OpenGL entry point not loaded: glGetShaderInfoLog"))(a0, a1, a2, a3)
    }
    pub unsafe fn GetShaderiv(&self, a0: GLuint, a1: GLenum, a2: *mut GLint) -> () {
        (self.GetShaderiv.expect("OpenGL entry point not loaded: glGetShaderiv"))(a0, a1, a2)
    }
    pub unsafe fn GetString(&self, a0: GLenum) -> *const GLubyte {
        (self.GetString.expect("OpenGL entry point not loaded: glGetString"))(a0)
    }
    pub unsafe fn GetTexImage(&self, a0: GLenum, a1: GLint, a2: GLenum, a3: GLenum, a4: *mut std::ffi::c_void) -> () {
        (self.GetTexImage.expect("OpenGL entry point not loaded: glGetTexImage"))(a0, a1, a2, a3, a4)
    }
    pub unsafe fn GetTexLevelParameteriv(&self, a0: GLenum, a1: GLint, a2: GLenum, a3: *mut GLint) -> () {
        (self.GetTexLevelParameteriv.expect("OpenGL entry point not loaded: glGetTexLevelParameteriv"))(a0, a1, a2, a3)
    }
    pub unsafe fn GetUniformBlockIndex(&self, a0: GLuint, a1: *const GLchar) -> GLuint {
        (self.GetUniformBlockIndex.expect("OpenGL entry point not loaded: glGetUniformBlockIndex"))(a0, a1)
    }
    pub unsafe fn GetUniformLocation(&self, a0: GLuint, a1: *const GLchar) -> GLint {
        (self.GetUniformLocation.expect("OpenGL entry point not loaded: glGetUniformLocation"))(a0, a1)
    }
    pub unsafe fn IsEnabled(&self, a0: GLenum) -> GLboolean {
        (self.IsEnabled.expect("OpenGL entry point not loaded: glIsEnabled"))(a0)
    }
    pub unsafe fn LinkProgram(&self, a0: GLuint) -> () {
        (self.LinkProgram.expect("OpenGL entry point not loaded: glLinkProgram"))(a0)
    }
    pub unsafe fn PixelStorei(&self, a0: GLenum, a1: GLint) -> () {
        (self.PixelStorei.expect("OpenGL entry point not loaded: glPixelStorei"))(a0, a1)
    }
    pub unsafe fn ProvokingVertex(&self, a0: GLenum) -> () {
        (self.ProvokingVertex.expect("OpenGL entry point not loaded: glProvokingVertex"))(a0)
    }
    pub unsafe fn ReadBuffer(&self, a0: GLenum) -> () {
        (self.ReadBuffer.expect("OpenGL entry point not loaded: glReadBuffer"))(a0)
    }
    pub unsafe fn ReadPixels(&self, a0: GLint, a1: GLint, a2: GLsizei, a3: GLsizei, a4: GLenum, a5: GLenum, a6: *mut std::ffi::c_void) -> () {
        (self.ReadPixels.expect("OpenGL entry point not loaded: glReadPixels"))(a0, a1, a2, a3, a4, a5, a6)
    }
    pub unsafe fn Scissor(&self, a0: GLint, a1: GLint, a2: GLsizei, a3: GLsizei) -> () {
        (self.Scissor.expect("OpenGL entry point not loaded: glScissor"))(a0, a1, a2, a3)
    }
    pub unsafe fn ShaderSource(&self, a0: GLuint, a1: GLsizei, a2: *const *const GLchar, a3: *const GLint) -> () {
        (self.ShaderSource.expect("OpenGL entry point not loaded: glShaderSource"))(a0, a1, a2, a3)
    }
    pub unsafe fn TexImage2D(&self, a0: GLenum, a1: GLint, a2: GLint, a3: GLsizei, a4: GLsizei, a5: GLint, a6: GLenum, a7: GLenum, a8: *const std::ffi::c_void) -> () {
        (self.TexImage2D.expect("OpenGL entry point not loaded: glTexImage2D"))(a0, a1, a2, a3, a4, a5, a6, a7, a8)
    }
    pub unsafe fn TexParameteri(&self, a0: GLenum, a1: GLenum, a2: GLint) -> () {
        (self.TexParameteri.expect("OpenGL entry point not loaded: glTexParameteri"))(a0, a1, a2)
    }
    pub unsafe fn TexSubImage2D(&self, a0: GLenum, a1: GLint, a2: GLint, a3: GLint, a4: GLsizei, a5: GLsizei, a6: GLenum, a7: GLenum, a8: *const std::ffi::c_void) -> () {
        (self.TexSubImage2D.expect("OpenGL entry point not loaded: glTexSubImage2D"))(a0, a1, a2, a3, a4, a5, a6, a7, a8)
    }
    pub unsafe fn Uniform1f(&self, a0: GLint, a1: GLfloat) -> () {
        (self.Uniform1f.expect("OpenGL entry point not loaded: glUniform1f"))(a0, a1)
    }
    pub unsafe fn Uniform1i(&self, a0: GLint, a1: GLint) -> () {
        (self.Uniform1i.expect("OpenGL entry point not loaded: glUniform1i"))(a0, a1)
    }
    pub unsafe fn Uniform2f(&self, a0: GLint, a1: GLfloat, a2: GLfloat) -> () {
        (self.Uniform2f.expect("OpenGL entry point not loaded: glUniform2f"))(a0, a1, a2)
    }
    pub unsafe fn UniformBlockBinding(&self, a0: GLuint, a1: GLuint, a2: GLuint) -> () {
        (self.UniformBlockBinding.expect("OpenGL entry point not loaded: glUniformBlockBinding"))(a0, a1, a2)
    }
    pub unsafe fn UseProgram(&self, a0: GLuint) -> () {
        (self.UseProgram.expect("OpenGL entry point not loaded: glUseProgram"))(a0)
    }
    pub unsafe fn VertexAttribPointer(&self, a0: GLuint, a1: GLint, a2: GLenum, a3: GLboolean, a4: GLsizei, a5: *const std::ffi::c_void) -> () {
        (self.VertexAttribPointer.expect("OpenGL entry point not loaded: glVertexAttribPointer"))(a0, a1, a2, a3, a4, a5)
    }
    pub unsafe fn Viewport(&self, a0: GLint, a1: GLint, a2: GLsizei, a3: GLsizei) -> () {
        (self.Viewport.expect("OpenGL entry point not loaded: glViewport"))(a0, a1, a2, a3)
    }
}
