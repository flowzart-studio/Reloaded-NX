/* SPDX-License-Identifier: MIT */
#include <EGL/egl.h>
#include <GL/glcorearb.h>
#include <stdio.h>
#include <string.h>
#include "horizon-controls.h"
#include "horizon-party-overlay.h"

#define GL_PROCS(X) \
    X(CREATESHADER, CreateShader) X(SHADERSOURCE, ShaderSource) \
    X(COMPILESHADER, CompileShader) X(GETSHADERIV, GetShaderiv) \
    X(GETSHADERINFOLOG, GetShaderInfoLog) X(DELETESHADER, DeleteShader) \
    X(CREATEPROGRAM, CreateProgram) X(ATTACHSHADER, AttachShader) \
    X(LINKPROGRAM, LinkProgram) X(GETPROGRAMIV, GetProgramiv) \
    X(GETPROGRAMINFOLOG, GetProgramInfoLog) X(DELETEPROGRAM, DeleteProgram) \
    X(GETUNIFORMLOCATION, GetUniformLocation) X(UNIFORM1I, Uniform1i) \
    X(GENVERTEXARRAYS, GenVertexArrays) X(DELETEVERTEXARRAYS, DeleteVertexArrays) \
    X(BINDVERTEXARRAY, BindVertexArray) X(USEPROGRAM, UseProgram) \
    X(GETINTEGERV, GetIntegerv) X(ISENABLED, IsEnabled) X(ENABLE, Enable) X(DISABLE, Disable) \
    X(BINDFRAMEBUFFER, BindFramebuffer) X(VIEWPORT, Viewport) \
    X(BLENDFUNCSEPARATE, BlendFuncSeparate) X(BLENDEQUATIONSEPARATE, BlendEquationSeparate) \
    X(DRAWARRAYS, DrawArrays) X(GENBUFFERS, GenBuffers) X(DELETEBUFFERS, DeleteBuffers) \
    X(BINDBUFFER, BindBuffer) X(BUFFERDATA, BufferData) X(VERTEXATTRIBPOINTER, VertexAttribPointer) \
    X(ENABLEVERTEXATTRIBARRAY, EnableVertexAttribArray) X(ACTIVETEXTURE, ActiveTexture) \
    X(GENTEXTURES, GenTextures) X(DELETETEXTURES, DeleteTextures) X(BINDTEXTURE, BindTexture) \
    X(BINDSAMPLER, BindSampler) X(TEXIMAGE2D, TexImage2D) X(TEXPARAMETERI, TexParameteri) \
    X(PIXELSTOREI, PixelStorei)

#define DECLARE(type, name) static PFNGL##type##PROC gl##name;
GL_PROCS(DECLARE)
#undef DECLARE

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
static const char *vertex_source =
    "#version 330 core\n"
    "out vec2 point;\n"
    "void main() {\n"
    "  vec2 corner = vec2(gl_VertexID & 1, gl_VertexID >> 1);\n"
    "  point = mix(vec2(" STRINGIFY(NX_FLEE_LEFT) "," STRINGIFY(NX_PARTY_TOP) "), vec2("
                       STRINGIFY(NX_PARTY_LEFT) " + " STRINGIFY(NX_PARTY_SIZE) ", "
                       STRINGIFY(NX_FLEE_TOP) " + " STRINGIFY(NX_FLEE_HEIGHT) "), corner);\n"
    "  vec2 uv = point / vec2(" STRINGIFY(NX_TOUCH_WIDTH) "," STRINGIFY(NX_TOUCH_HEIGHT) ");\n"
    "  gl_Position = vec4(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0, 0.0, 1.0);\n"
    "}\n";
static const char *fragment_source =
    "#version 330 core\n"
    "in vec2 point;\n"
    "uniform int pressed;\n"
    "uniform int label_ready;\n"
    "uniform int safari;\n"
    "uniform int mega;\n"
    "out vec4 color;\n"
    /* Each 15-bit mask encodes a 3-by-5 glyph, top row first, left to right. */
    "const int glyphs[6] = int[6](29850, 29671, 31207, 18925, 31183, 31695);\n"
    "void main() {\n"
    "  vec2 flee = point - vec2(" STRINGIFY(NX_FLEE_LEFT) "," STRINGIFY(NX_FLEE_TOP) ");\n"
    "  const vec2 flee_size = vec2(" STRINGIFY(NX_FLEE_WIDTH) "," STRINGIFY(NX_FLEE_HEIGHT) ");\n"
    "  vec2 mega_point = point - vec2(" STRINGIFY(NX_FLEE_LEFT) "," STRINGIFY(NX_MEGA_TOP) ");\n"
    "  if (mega != 0 && all(greaterThanEqual(mega_point, vec2(0))) && all(lessThan(mega_point, flee_size))) {\n"
    "    vec2 q = abs(mega_point - flee_size * 0.5) - (flee_size * 0.5 - 10.0);\n"
    "    float edge = length(max(q, vec2(0))) + min(max(q.x,q.y),0.0) - 10.0;\n"
    "    color = (pressed == " STRINGIFY(NX_MEGA_SLOT) ") ? vec4(0.13,0.36,0.55,0.94) : vec4(0.04,0.10,0.20,0.80);\n"
    "    if (edge > -2.0) color = vec4(0.46,0.69,0.90,0.84);\n"
    "    color.a *= 1.0 - smoothstep(-1.0, 0.5, edge);\n"
    "    return;\n"
    "  }\n"
    "  if (all(greaterThanEqual(flee, vec2(0))) && all(lessThan(flee, flee_size))) {\n"
    "    vec2 q = abs(flee - flee_size * 0.5) - (flee_size * 0.5 - 10.0);\n"
    "    float edge = length(max(q, vec2(0))) + min(max(q.x,q.y),0.0) - 10.0;\n"
    "    float alpha = (1.0 - smoothstep(-1.0, 0.5, edge));\n"
    "    color = (pressed == " STRINGIFY(NX_FLEE_SLOT) ") ? vec4(0.58,0.23,0.15,0.94) : vec4(0.20,0.10,0.09,0.80);\n"
    "    if (edge > -2.0) color = vec4(0.83,0.43,0.28,0.84);\n"
    "    color.a *= alpha;\n"
    "    if (label_ready == 0) {\n"
    "      const int fallback[4] = int[4](23533,9367,29847,23275);\n"
    "      ivec2 t = ivec2(floor((flee - vec2(49,20))/4.0));\n"
    "      if (t.x>=0 && t.x<16 && t.y>=0 && t.y<5 && t.x%4<3\n"
    "          && (fallback[t.x/4] & (1 << (t.y*3 + t.x%4))) != 0) color=vec4(1,0.92,0.88,1);\n"
    "    }\n"
    "    return;\n"
    "  }\n"
    "  const float pitch = float(" STRINGIFY(NX_PARTY_PITCH) ");\n"
    "  const float size = float(" STRINGIFY(NX_PARTY_SIZE) ");\n"
    "  vec2 p = point - vec2(" STRINGIFY(NX_PARTY_LEFT) "," STRINGIFY(NX_PARTY_TOP) ");\n"
    "  int slot = clamp(int(p.y / pitch), 0, " STRINGIFY(NX_PARTY_COUNT) " - 1);\n"
    "  p.y -= float(slot) * pitch;\n"
    "  if (p.x < 0.0 || p.y >= size || (safari != 0 && slot >= " STRINGIFY(NX_SAFARI_COUNT) ")) discard;\n"
    "  color = (pressed == slot + 1) ? vec4(0.13, 0.36, 0.55, 0.84) : vec4(0.04, 0.06, 0.10, 0.66);\n"
    "  vec2 edge = min(p, vec2(size) - p);\n"
    "  if (min(edge.x, edge.y) < 2.0) color = vec4(0.64, 0.75, 0.86, 0.48);\n"
    "  if (safari != 0) return;\n"
    "  vec2 digit = floor((p - (vec2(size) - vec2(30, 50)) * 0.5) / 10.0);\n"
    "  if (all(greaterThanEqual(digit, vec2(0))) && all(lessThan(digit, vec2(3, 5)))\n"
    "      && (glyphs[slot] & (1 << (int(digit.y) * 3 + int(digit.x)))) != 0)\n"
    "    color = vec4(0.96, 0.98, 1.0, 0.97);\n"
    "}\n";
#undef STRINGIFY
#undef STRINGIFY_

static GLuint program, vao, panel_program, font_program, font_vao, font_buffer, font_texture;
static GLuint pointer_program;
static GLint pointer_pressed;
static bool pointer_attempted;
static GLint pressed_location, label_location, safari_location, mega_location, panel_selected;
static bool font_attempted, font_ready;
static bool font_open(void);
static void draw_battle_labels(bool mega);

static GLuint compile_shader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    if (!shader) return 0;
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok) return shader;
    char log[512] = {0};
    glGetShaderInfoLog(shader, sizeof(log), NULL, log);
    fprintf(stderr, "TOUCH06 shader error: %s\n", log);
    glDeleteShader(shader);
    return 0;
}

static GLuint link_program(const char *vertex, const char *fragment) {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment);
    GLuint linked_program = 0;
    GLint linked = 0;
    if (vs && fs && (linked_program = glCreateProgram())) {
        glAttachShader(linked_program, vs);
        glAttachShader(linked_program, fs);
        glLinkProgram(linked_program);
        glGetProgramiv(linked_program, GL_LINK_STATUS, &linked);
        if (!linked) {
            char log[512] = {0};
            glGetProgramInfoLog(linked_program, sizeof(log), NULL, log);
            fprintf(stderr, "TOUCH06 program error: %s\n", log);
            glDeleteProgram(linked_program);
            linked_program = 0;
        }
    }
    if (vs) glDeleteShader(vs);
    if (fs) glDeleteShader(fs);
    return linked_program;
}

bool nx_party_overlay_open(void) {
    if (program) return true;
#define LOAD(type, name) \
    gl##name = (PFNGL##type##PROC)eglGetProcAddress("gl" #name); \
    if (!gl##name) return false;
    GL_PROCS(LOAD)
#undef LOAD
    program = link_program(vertex_source, fragment_source);
    if (program) {
        pressed_location = glGetUniformLocation(program, "pressed");
        label_location = glGetUniformLocation(program, "label_ready");
        safari_location = glGetUniformLocation(program, "safari");
        mega_location = glGetUniformLocation(program, "mega");
        glGenVertexArrays(1, &vao);
        if (vao && pressed_location >= 0) return true;
    }
    nx_party_overlay_close();
    return false;
}

static const GLenum tests[] = {GL_DEPTH_TEST, GL_SCISSOR_TEST, GL_CULL_FACE, GL_STENCIL_TEST};
typedef struct {
    GLboolean enabled[sizeof(tests) / sizeof(tests[0])], blended;
    GLint fbo, viewport[4], program, vao, src_rgb, dst_rgb, src_alpha, dst_alpha, eq_rgb, eq_alpha;
} OverlayState;

static OverlayState overlay_begin(uint32_t width, uint32_t height) {
    OverlayState state;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &state.fbo);
    glGetIntegerv(GL_VIEWPORT, state.viewport);
    glGetIntegerv(GL_CURRENT_PROGRAM, &state.program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &state.vao);
    glGetIntegerv(GL_BLEND_SRC_RGB, &state.src_rgb);
    glGetIntegerv(GL_BLEND_DST_RGB, &state.dst_rgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &state.src_alpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &state.dst_alpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &state.eq_rgb);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &state.eq_alpha);
    state.blended = glIsEnabled(GL_BLEND);
    for (unsigned i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        state.enabled[i] = glIsEnabled(tests[i]);
        if (state.enabled[i]) glDisable(tests[i]);
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    if (!state.blended) glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(vao);
    return state;
}

static void overlay_end(const OverlayState *state) {
    glBindVertexArray(state->vao);
    glUseProgram(state->program);
    glBlendFuncSeparate(state->src_rgb, state->dst_rgb, state->src_alpha, state->dst_alpha);
    glBlendEquationSeparate(state->eq_rgb, state->eq_alpha);
    if (!state->blended) glDisable(GL_BLEND);
    glViewport(state->viewport[0], state->viewport[1], state->viewport[2], state->viewport[3]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state->fbo);
    for (unsigned i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i)
        if (state->enabled[i]) glEnable(tests[i]);
}

static void draw_battle_buttons(uint32_t width, uint32_t height, unsigned slot, bool safari, bool label_ready, bool mega) {
    OverlayState state = overlay_begin(width, height);
    glUseProgram(program);
    glUniform1i(pressed_location, (GLint)slot);
    glUniform1i(label_location, label_ready);
    glUniform1i(safari_location, safari);
    glUniform1i(mega_location, mega && label_ready);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    if (label_ready) draw_battle_labels(mega);
    overlay_end(&state);
}

bool nx_party_overlay_draw(uint32_t width, uint32_t height, unsigned slot, bool mega) {
    bool labels = font_open();
    draw_battle_buttons(width, height, slot, false, labels, mega);
    return mega && labels;
}

void nx_party_overlay_close(void) {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (program) glDeleteProgram(program);
    if (panel_program) glDeleteProgram(panel_program);
    if (pointer_program) glDeleteProgram(pointer_program);
    pointer_program = 0;
    pointer_attempted = false;
    if (font_program) glDeleteProgram(font_program);
    if (font_vao) glDeleteVertexArrays(1, &font_vao);
    if (font_buffer) glDeleteBuffers(1, &font_buffer);
    if (font_texture) glDeleteTextures(1, &font_texture);
    vao = program = panel_program = 0;
    font_program = font_vao = font_buffer = font_texture = 0;
    font_attempted = font_ready = false;
}

#define PANEL_STR_(x) #x
#define PANEL_STR(x) PANEL_STR_(x)
static const char *panel_vertex =
    "#version 330 core\n"
    "out vec2 uv;\n"
    "void main() {\n"
    "  uv = vec2(gl_VertexID & 1, gl_VertexID >> 1);\n"
    "  gl_Position = vec4(uv.x*2.0-1.0, 1.0-uv.y*2.0, 0.0, 1.0);\n"
    "}\n";
static const char *pointer_fragment =
    "#version 330 core\n"
    "in vec2 uv; out vec4 color; uniform int pressed;\n"
    "void main() {\n"
    "  vec2 p = abs(uv - 0.5);\n"
    "  float d = min(max(p.x - 0.10,p.y - 0.46),max(p.x - 0.46,p.y - 0.10));\n"
    "  if (d > 0.0) discard;\n"
    "  vec3 ink = pressed != 0 ? vec3(1.0,0.82,0.2) : vec3(1.0);\n"
    "  color = vec4(d < -0.050 ? ink : vec3(0.07),1.0);\n"
    "}\n";

void nx_pointer_overlay_draw(uint32_t width, uint32_t height, unsigned x, unsigned y, bool down) {
    if (!pointer_attempted) {
        pointer_attempted = true;
        if (nx_party_overlay_open()) pointer_program = link_program(panel_vertex, pointer_fragment);
        if (pointer_program) pointer_pressed = glGetUniformLocation(pointer_program, "pressed");
    }
    if (!pointer_program) return;
    OverlayState state = overlay_begin(width, height);
    int size = (int)(24 * height / NX_TOUCH_HEIGHT);
    int px = (int)(x * width / NX_TOUCH_WIDTH);
    int py = (int)height - 1 - (int)(y * height / NX_TOUCH_HEIGHT);
    glViewport(px - size / 2, py - size / 2, size, size);
    glUseProgram(pointer_program);
    glUniform1i(pointer_pressed, down);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    overlay_end(&state);
}

static const char *panel_fragment =
    "#version 330 core\n"
    "in vec2 uv;\n"
    "out vec4 color;\n"
    "uniform int selected;\n"
    "void main() {\n"
    "  vec2 p = uv*vec2(" PANEL_STR(NX_TOUCH_WIDTH) "," PANEL_STR(NX_TOUCH_HEIGHT) ");\n"
    "  color = vec4(mix(vec3(0.052,0.071,0.11),vec3(0.019,0.031,0.058),uv.y),1);\n"
    "  vec2 q = abs(p-vec2(640,360))-vec2(593,317);\n"
    "  float d = length(max(q,vec2(0)))+min(max(q.x,q.y),0.0)-15.0;\n"
    "  if (d > 0.0) return;\n"
    "  color=vec4(d>-1.0?vec3(0.15,0.22,0.31):vec3(0.064,0.088,0.135),1);\n"
    "  vec2 cell=p-vec2(" PANEL_STR(NX_PANEL_LEFT) "," PANEL_STR(NX_PANEL_TOP) ");\n"
    "  const float h=float(" PANEL_STR(NX_PANEL_ROW_HEIGHT) "), w=float(" PANEL_STR(NX_PANEL_WIDTH) ");\n"
    "  if (cell.x<0.0 || cell.x>=w || cell.y<0.0) return;\n"
    "  int line=int(cell.y/h);\n"
    "  if (line==2 || line==12) { if (mod(cell.y,h)<1.0) color=vec4(0.19,0.27,0.36,1); }\n"
    "  if (selected<0 || line!=selected/3) return;\n"
    "  int col=selected%3;\n"
    "  if ((col==1 && cell.x>=w*0.5) || (col==2 && cell.x<w*0.5)) return;\n"
    "  if (mod(cell.y,h)>2.0 && mod(cell.y,h)<h-2.0) {\n"
    "    float x=col==2?cell.x-w*0.5:cell.x;\n"
    "    color=x<4.0?vec4(0.40,0.78,0.98,1):vec4(0.12,0.24,0.36,1);\n"
    "  }\n"
    "}\n";
#undef PANEL_STR
#undef PANEL_STR_

typedef struct { uint16_t x,y,w,h; int8_t left,top; float advance; } NxGlyph;
#include "fonts/noto-atlas.inc"

typedef struct { float x,y,u,v,r,g,b,a; } NxFontVertex;
static NxFontVertex text_vertices[NX_PANEL_ROWS * 2 * NX_PANEL_TEXT_BYTES * 6];
static unsigned text_count;

static const char *font_vertex =
    "#version 330 core\n"
    "layout(location=0) in vec4 geometry;\n"
    "layout(location=1) in vec4 tint;\n"
    "out vec2 uv; out vec4 rgba;\n"
    "void main() { gl_Position=vec4(geometry.xy,0,1); uv=geometry.zw; rgba=tint; }\n";
static const char *font_fragment =
    "#version 330 core\n"
    "in vec2 uv; in vec4 rgba; out vec4 color;\n"
    "uniform sampler2D atlas;\n"
    "void main() { color=vec4(rgba.rgb,rgba.a*texture(atlas,uv).r); }\n";

typedef struct { GLint active, texture, sampler, array; } FontState;
static FontState font_bind(void) {
    FontState old;
    glGetIntegerv(GL_ACTIVE_TEXTURE,&old.active);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&old.array);
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D,&old.texture);
    glGetIntegerv(GL_SAMPLER_BINDING,&old.sampler);
    glBindSampler(0,0);
    glBindTexture(GL_TEXTURE_2D,font_texture);
    glBindBuffer(GL_ARRAY_BUFFER,font_buffer);
    return old;
}
static void font_unbind(FontState old) {
    glBindBuffer(GL_ARRAY_BUFFER,old.array);
    glBindTexture(GL_TEXTURE_2D,old.texture);
    glBindSampler(0,old.sampler);
    glActiveTexture(old.active);
}

static bool font_open(void) {
    if (font_attempted) return font_ready;
    font_attempted=true;
    font_program=link_program(font_vertex,font_fragment);
    if (!font_program) return false;
    glGenVertexArrays(1,&font_vao);
    glGenBuffers(1,&font_buffer);
    glGenTextures(1,&font_texture);
    if (!font_vao || !font_buffer || !font_texture) return false;
    uint8_t pixels[FONT_WIDTH * FONT_HEIGHT];
    size_t src=0,dst=0;
    while (src<sizeof(font_pixels)) {
        unsigned command=font_pixels[src++], count=(command&127u)+1;
        if (dst+count>sizeof(pixels) || src+(command&128u?1:count)>sizeof(font_pixels)) return false;
        if (command&128u) memset(pixels+dst,font_pixels[src++],count);
        else { memcpy(pixels+dst,font_pixels+src,count); src+=count; }
        dst+=count;
    }
    if (dst!=sizeof(pixels)) return false;
    FontState old=font_bind();
    GLint old_vao, unpack_buffer, alignment;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&old_vao);
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING,&unpack_buffer);
    glGetIntegerv(GL_UNPACK_ALIGNMENT,&alignment);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_R8,FONT_WIDTH,FONT_HEIGHT,0,GL_RED,GL_UNSIGNED_BYTE,pixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT,alignment);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,unpack_buffer);
    glBindVertexArray(font_vao);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NxFontVertex),NULL);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(NxFontVertex),(void *)(4*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(old_vao);
    font_unbind(old);
    font_ready=true;
    return true;
}

static const NxGlyph *next_glyph(const unsigned char **string) {
    const unsigned char *p=*string;
    unsigned code=*p++;
    if (code>=0xc2 && code<=0xc3 && (*p&0xc0)==0x80)
        code=((code&0x1f)<<6)|(*p++&0x3f);
    else if (code>=0x80) {
        while ((*p&0xc0)==0x80) ++p;
        code='-';
    }
    *string=p;
    return &glyphs[glyphs[code].advance>0 ? code : '?'];
}

static float text_width(const char *text) {
    float width=0;
    for (const unsigned char *p=(const unsigned char *)text;*p;)
        width+=next_glyph(&p)->advance;
    return width;
}

static void text_line(const char *text, float x, float baseline, float max_width, float size, const float color[4]) {
    float scale=size/FONT_SIZE, width=text_width(text)*scale;
    if (width>max_width) scale*=max_width/width;
    static const unsigned corners[]={0,1,2,2,1,3};
    for (const unsigned char *p=(const unsigned char *)text;*p;) {
        const NxGlyph *g=next_glyph(&p);
        if (g->w && g->h) {
            for (unsigned i=0;i<6;++i) {
                unsigned u=corners[i]&1, v=corners[i]>>1;
                float px=x+(g->left+(int)(u*g->w))*scale;
                float py=baseline+(g->top+(int)(v*g->h))*scale;
                text_vertices[text_count++]=(NxFontVertex){
                    2*px/NX_TOUCH_WIDTH-1, 1-2*py/NX_TOUCH_HEIGHT,
                    (g->x+u*g->w)/(float)FONT_WIDTH, (g->y+v*g->h)/(float)FONT_HEIGHT,
                    color[0],color[1],color[2],color[3]};
            }
        }
        x+=g->advance*scale;
    }
}

static void text_draw(void) {
    FontState old=font_bind();
    glUseProgram(font_program); /* The sampler's default is texture unit zero. */
    glBindVertexArray(font_vao);
    glBufferData(GL_ARRAY_BUFFER,text_count*sizeof(NxFontVertex),text_vertices,GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES,0,(GLsizei)text_count);
    font_unbind(old);
}

static void draw_battle_labels(bool mega) {
    const float size=29, tint[]={1.0f,0.92f,0.88f,1};
    text_count=0;
    float x=NX_FLEE_LEFT+(NX_FLEE_WIDTH-text_width("Huir")*size/FONT_SIZE)/2;
    text_line("Huir",x,NX_FLEE_TOP+40,NX_FLEE_WIDTH,size,tint);
    if (mega) {
        x=NX_FLEE_LEFT+(NX_FLEE_WIDTH-text_width("Mega")*size/FONT_SIZE)/2;
        text_line("Mega",x,NX_MEGA_TOP+40,NX_FLEE_WIDTH,size,tint);
    }
    text_draw();
}

bool nx_safari_overlay_draw(uint32_t width, uint32_t height, unsigned slot) {
    if (!font_open()) return false;
    draw_battle_buttons(width, height, slot, true, true, false);
    OverlayState state=overlay_begin(width,height);
    static const char *labels[NX_SAFARI_COUNT]={"Ball", "Baya", "%", "Aceptar", "Cancelar"};
    const float size=25, tint[]={0.96f,0.98f,1,1};
    text_count=0;
    for (unsigned i=0; i<NX_SAFARI_COUNT; ++i) {
        float width=text_width(labels[i])*size/FONT_SIZE;
        float x=NX_PARTY_LEFT+(NX_PARTY_SIZE-(width<NX_PARTY_SIZE?width:NX_PARTY_SIZE))/2;
        text_line(labels[i],x,NX_PARTY_TOP+i*NX_PARTY_PITCH+49,NX_PARTY_SIZE,size,tint);
    }
    text_draw();
    overlay_end(&state);
    return true;
}

bool nx_control_overlay_draw(uint32_t width, uint32_t height, const NxPanelView *view) {
    if (!nx_party_overlay_open() || !font_open()) return false;
    if (!panel_program) {
        panel_program=link_program(panel_vertex,panel_fragment);
        if (!panel_program) return false;
        panel_selected=glGetUniformLocation(panel_program,"selected");
    }
    OverlayState state=overlay_begin(width,height);
    glUseProgram(panel_program);
    glUniform1i(panel_selected,view->row<0?-1:3*view->row+view->column+1);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    static const float title[]={0.56f,0.85f,1,1}, main[]={0.93f,0.95f,0.99f,1},
                       muted[]={0.70f,0.79f,0.89f,1}, note[]={0.95f,0.79f,0.59f,1};
    text_count=0;
    for (unsigned r=0;r<NX_PANEL_ROWS;++r) {
        for (unsigned c=0;c<2;++c) {
            float x=NX_PANEL_LEFT + c*NX_PANEL_WIDTH/2 + 16;
            float size=r==0?34:r==1?28:r>=11?23:26;
            float max_width=(c || view->rows[r][1][0] ? NX_PANEL_WIDTH/2 : NX_PANEL_WIDTH)-32;
            const float *color=r==0?title:r==13?note:r>=11 || c?muted:main;
            text_line(view->rows[r][c],x,NX_PANEL_TOP+r*NX_PANEL_ROW_HEIGHT+31,max_width,size,color);
        }
    }
    text_draw();
    overlay_end(&state);
    return true;
}
