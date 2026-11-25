"""Generate the consumed API declarations from a self-licensed MIT C header.

Loader/wrappers below are original project code, not gl-rs template output.
No Khronos XML or gl_generator input is consumed.
"""
from pathlib import Path
import re,json,hashlib
ROOT=Path(__file__).resolve().parents[1]
ENGINE=ROOT/'full-runner/src/OpenGMK/gm8emulator'
header=ROOT/'third_party/khronos-mit/glcorearb.h'
h=header.read_text(encoding='utf-8')
render=(ENGINE/'src/render/opengl.rs').read_text(encoding='utf-8')
functions=sorted(set(re.findall(r'\bgl\.([A-Z]\w*)\s*\(',render)))
constants=sorted(set(re.findall(r'\bgl::([A-Z][A-Z_0-9]*)\b',render)))
# API symbol aliases, not implementation templates. Preserve existing lookup order.
aliases={
 'ActiveTexture':['ActiveTextureARB'],'AttachShader':['AttachObjectARB'],
 'BindBuffer':['BindBufferARB'],'BindBufferBase':['BindBufferBaseEXT','BindBufferBaseNV'],
 'BindTexture':['BindTextureEXT'],'BindVertexArray':['BindVertexArrayOES'],
 'BlitFramebuffer':['BlitFramebufferEXT','BlitFramebufferNV'],'BufferData':['BufferDataARB'],
 'CheckFramebufferStatus':['CheckFramebufferStatusEXT'],'CompileShader':['CompileShaderARB'],
 'CopyTexImage2D':['CopyTexImage2DEXT'],'CreateProgram':['CreateProgramObjectARB'],
 'CreateShader':['CreateShaderObjectARB'],'DeleteFramebuffers':['DeleteFramebuffersEXT'],
 'DrawArrays':['DrawArraysEXT'],'EnableVertexAttribArray':['EnableVertexAttribArrayARB'],
 'FramebufferTexture2D':['FramebufferTexture2DEXT'],'GenBuffers':['GenBuffersARB'],
 'GenFramebuffers':['GenFramebuffersEXT'],'GenVertexArrays':['GenVertexArraysAPPLE','GenVertexArraysOES'],
 'GetUniformLocation':['GetUniformLocationARB'],'LinkProgram':['LinkProgramARB'],
 'ProvokingVertex':['ProvokingVertexEXT'],'ShaderSource':['ShaderSourceARB'],
 'TexSubImage2D':['TexSubImage2DEXT'],'Uniform1f':['Uniform1fARB'],
 'Uniform1i':['Uniform1iARB'],'Uniform2f':['Uniform2fARB'],
 'UseProgram':['UseProgramObjectARB'],'VertexAttribPointer':['VertexAttribPointerARB'],
}
types={'GLenum':'u32','GLint':'i32','GLsizei':'i32','GLbitfield':'u32',
       'GLdouble':'f64','GLuint':'u32','GLboolean':'u8','GLubyte':'u8',
       'GLfloat':'f32','GLchar':'std::os::raw::c_char','GLintptr':'isize','GLsizeiptr':'isize'}
def rusttype(c):
    c=' '.join(c.split());parts=c.split('*');base=parts[0].replace('const','').strip()
    assert base in types or base=='void',c
    result='std::ffi::c_void' if base=='void' else base
    if len(parts)==1 and base=='void':return '()'
    for i in range(len(parts)-1):result=('*const ' if 'const' in parts[i].split() else '*mut ')+result
    return result
schema=[]
for name in functions:
    match=re.search(r'^GLAPI\s+(.+?)\s*APIENTRY\s+gl'+name+r'\s*\(([^;]*)\);',h,re.M)
    assert match,('Missing MIT-header declaration',name)
    args=[]
    if match[2].strip()!='void':
        for pos,c in enumerate(match[2].split(',')):
            typ=re.sub(r'\b[A-Za-z_]\w*\s*$','',c.strip()).strip()
            args.append([f'a{pos}',rusttype(typ)])
    schema.append({'name':name,'return':rusttype(match[1]),'arguments':args,'symbols':['gl'+name]+['gl'+x for x in aliases.get(name,[])]})
lines=['// Generated from the bundled MIT glcorearb.h; see licenses/Khronos-header-MIT.txt.',
       '// Original loader/wrappers: MIT, RELOADED-NX contributors. No gl-rs templates.',
       'pub mod types {']
for name,typ in types.items():lines.append(f'    pub type {name} = {typ};')
lines+=['}','use self::types::*;']
for name in constants:
    match=re.search(r'^#define GL_'+name+r'\s+(\S+)',h,re.M);assert match,name
    value=match[1].rstrip('uUlL');assert re.fullmatch(r'(?:0x[0-9a-fA-F]+|[0-9]+)',value),(name,value)
    lines.append(f'pub const {name}: '+('GLboolean' if name in ['TRUE','FALSE'] else 'GLenum')+f' = {value};')
lines+=['#[derive(Clone)]','pub struct Gl {']
for x in schema:
    signature=', '.join(t for _,t in x['arguments'])
    lines.append(f'    {x["name"]}: Option<unsafe extern "system" fn({signature}) -> {x["return"]}>,')
lines+=['}','impl Gl {','    pub fn load_with<F: FnMut(&\'static str) -> *const std::ffi::c_void>(mut resolver: F) -> Self {','        Self {']
for x in schema:
    syms=', '.join('"'+s+'"' for s in x['symbols'])
    lines += [f'            {x["name"]}: {{',f'                let mut address = std::ptr::null();',f'                for symbol in [{syms}] {{',
              '                    address = resolver(symbol);','                    if !address.is_null() { break; }','                }',
              '                if address.is_null() { None } else { Some(unsafe { std::mem::transmute(address) }) }','            },']
lines+=['        }','    }']
for x in schema:
    params=', '.join(f'{n}: {t}' for n,t in x['arguments']);args=', '.join(n for n,_ in x['arguments'])
    lines += [f'    pub unsafe fn {x["name"]}(&self'+(', '+params if params else '')+f') -> {x["return"]} {{',
              f'        (self.{x["name"]}.expect("OpenGL entry point not loaded: gl{x["name"]}"))({args})','    }']
lines+=['}','']
target=ENGINE/'src/render/opengl/horizon_mit.rs'
target.write_text('\n'.join(lines),encoding='utf-8',newline='\n')
metadata={'header_sha256':hashlib.sha256(header.read_bytes()).hexdigest(),'header_origin':'switch-mesa 20.1.0-5 SDK archive SHA-256 78328415d06bff75af07091243844ded5a63dc4d4520d2c708787b8d80ad386d','header_license':'MIT, 2013-2018 The Khronos Group Inc.','generated_sha256':hashlib.sha256(target.read_bytes()).hexdigest(),'functions':schema,'constants':constants,'alias_provenance':'OpenGL API symbol names and existing lookup order; no implementation templates copied','implementation':'Original typed Option<extern system fn> loader; core symbol first, aliases in prior order; fail on invocation when unresolved'}
(ENGINE/'src/render/opengl/horizon_mit.json').write_text(json.dumps(metadata,indent=2)+'\n',encoding='utf-8')
print(json.dumps({'functions':len(schema),'constants':len(constants),'generated_sha256':metadata['generated_sha256']}))
