/* SPDX-License-Identifier: MIT. Real libnx NWindow + Mesa EGL/OpenGL backend.
 * EGL setup follows devkitPro switch-examples simple_triangle (OpenGL core).
 */
#include <switch.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>
#include "horizon-controls.h"
#include "horizon-party-overlay.h"
#include "horizon-pointer.h"
#include "horizon-casino.h"
extern void nx_resource_invalidate(void);
static AppletHookCookie resource_focus_cookie;
static bool resource_focus_hook_ready;
static atomic_bool reset_controls;
static void resource_focus_hook(AppletHookType type,void *param){
 (void)param;
 if(type==AppletHookType_OnFocusState)nx_resource_invalidate();
 if(type==AppletHookType_OnFocusState||type==AppletHookType_OnResume||type==AppletHookType_OnOperationMode)
  atomic_store(&reset_controls,true);
}
extern void nx_perf_add(uint32_t,uint64_t);
extern void nx_gpu_init(void);extern void nx_gpu_end(void);extern void nx_gpu_close(void);
int nx_gl_drawable_size(uint32_t *w,uint32_t *h);
static NWindow *window;
static PadState pad;
static bool hid_ready;
static NxControls controls;
static NxRightStick right_stick;
static NxMegaInput mega_input;
static bool overlay_presented,mega_presented,overlay_failed,combat_mode,scene_changing;
static bool safari_ready,safari_mode;
static NxBattleContext battle_context;
static uint32_t pointer_kind;
static bool map_pointer_seen;
static NxPointer pointer;
static NxPointerRect pointer_rect;
static NxPointerSample pointer_draw;
static uint64_t pointer_tick;
static NxMapper mapper;
static NxProfile profile,safari_profile;
static uint8_t casino_keys[5];
static bool casino_changed;
static unsigned casino_kind;
static uint32_t safari_chord_buttons;
static NxShortcutPulse shortcut_pulse;
static const char *profile_status;
static bool panel_allowed, panel_resumed, plus_armed;
static unsigned touch_presses[NX_PARTY_COUNT];
static unsigned flee_presses;
static unsigned mega_presses;
static EGLDisplay display=EGL_NO_DISPLAY;
static EGLContext context=EGL_NO_CONTEXT;
static EGLSurface surface=EGL_NO_SURFACE;
static ViDisplay vi_display;
static Event vsync;
static bool vsync_ready;
static int fail(const char *stage) {fprintf(stderr,"EGL_FAIL %s error=%04x\n",stage,eglGetError());fflush(stderr);return -1;}
int nx_window_open(uint32_t w,uint32_t h) {
 window=nwindowGetDefault(); if(!window||!w||!h)return -1;
 uint32_t dw=0,dh=0;Result rc=nwindowGetDimensions(window,&dw,&dh);
 if(R_SUCCEEDED(rc)&&dw&&dh)rc=nwindowSetDimensions(window,dw,dh);
 else return -1;
 fprintf(stderr,"WINDOW04 logical=%ux%u native=%ux%u rc=%08x\n",w,h,dw,dh,rc);fflush(stderr);
 if(R_FAILED(rc))return -1;
 padConfigureInput(1,HidNpadStyleSet_NpadStandard);padInitializeDefault(&pad);hidInitializeTouchScreen();hid_ready=true;
 profile=nx_safe_profile;
 padUpdate(&pad);
 uint64_t rescue=HidNpadButton_Plus | HidNpadButton_Minus;
 if((padGetButtons(&pad) & rescue)==rescue){
  profile.manual_battle=1;
  profile_status="RESCATE ACTIVO. PERFIL SEGURO EN MODO MANUAL";
 }else{
  int loaded=nx_control_load(&profile);
  if(loaded<0)profile_status="CONFIG INVALIDA. PERFIL SEGURO";
  if(loaded==2)profile_status="PERFIL RECUPERADO";
 }
 if(!resource_focus_hook_ready){appletHook(&resource_focus_cookie,resource_focus_hook,NULL);resource_focus_hook_ready=true;}
 return 0;
}
int nx_window_resize(uint32_t w,uint32_t h) {
 if(!window||!w||!h)return -1;
 /* Ramen retains the requested logical size. Horizon has one fullscreen layer.
  * Do not resize NWindow after EGL has registered buffers (libnx forbids it).
  * The renderer maps GM's region directly into the real drawable. */
 fprintf(stderr,"WINDOW04 logical_resize=%ux%u native_retained=true\n",w,h);return 0;
}
void *nx_window_handle(void){return window;}
void nx_window_panel_allow(bool allow){panel_allowed=allow;}
bool nx_window_panel_resumed(void){bool value=panel_resumed;panel_resumed=false;return value;}

static void reset_pointer(void) {
 pointer=(NxPointer){.x=pointer.x,.y=pointer.y};
 pointer_tick=0;
 pointer_draw=(NxPointerSample){0};
 map_pointer_seen=false;
}

void nx_window_scene_changing(void) {
 scene_changing=true;
 overlay_presented=false;
 right_stick=(NxRightStick){0};
 mega_input=(NxMegaInput){0};
 nx_controls_touch(&controls,false,0,0,0,0);
 reset_pointer();
}

void nx_window_battle_context(int32_t room,bool eligible,bool ready,bool safari) {
 safari_ready=safari;
 nx_battle_context(&battle_context,room,eligible,ready);
 scene_changing=false;
}

static void reset_context_input(void) {
 mapper=(NxMapper){0};
 right_stick=(NxRightStick){0};
 mega_input=(NxMegaInput){0};
 overlay_presented=false;
 nx_controls_touch(&controls,false,0,0,0,0);
 reset_pointer();
}

void nx_window_casino(unsigned kind, const uint8_t keys[5]) {
 bool changed=kind!=casino_kind;
 for(unsigned i=0;i<5;i++)if(keys[i]!=casino_keys[i])changed=true;
 if(!changed)return;
 casino_kind=kind;
 for(unsigned i=0;i<5;i++)casino_keys[i]=keys[i];
 reset_context_input();
 casino_changed=true;
 fprintf(stderr,"CASINO13 context=%u keys=%u,%u,%u,%u,%u\n",kind,
         keys[0],keys[1],keys[2],keys[3],keys[4]);
 if(kind){
  NxProfile effective=nx_casino_profile(&profile,keys);
  for(unsigned i=0;i<16;i++)if(effective.bindings[0][NX_MINUS][i]!=profile.bindings[0][NX_MINUS][i])
   fprintf(stderr,"CASINO13 fallback minus_source=%u vk=%u\n",i,effective.bindings[0][NX_MINUS][i]);
 }
}

void nx_window_pointer_rect(uint32_t kind,uint16_t left,uint16_t top,uint16_t right,uint16_t bottom) {
 NxPointerRect next={left,top,right,bottom};
 bool changed=kind!=pointer_kind;
 if(!changed && left==pointer_rect.left && top==pointer_rect.top &&
    right==pointer_rect.right && bottom==pointer_rect.bottom)return;
 if(changed) {
  reset_context_input();
  fprintf(stderr,"CONTEXT11 pointer=%u room=%d\n",(unsigned)kind,battle_context.room);
 }
 if(right) {
  if(changed || !pointer_rect.right)nx_pointer_position(&pointer,next,(left+right)/2.0f,(top+bottom)/2.0f);
  else nx_pointer_position(&pointer,next,pointer.x,pointer.y);
 }
 pointer_rect=next;
 pointer_kind=kind;
 overlay_presented=false;
 nx_controls_touch(&controls,false,0,0,0,0);
 reset_pointer();
}

void nx_window_pointer_warp(uint16_t x,uint16_t y) {
 if(!pointer_rect.right || pointer.owned)return;
 nx_pointer_position(&pointer,pointer_rect,x,y);
 if(pointer_draw.flags)pointer_draw=nx_pointer_sample(&pointer,pointer_draw.flags & NX_POINTER_DOWN);
}

static uint32_t physical_buttons(uint64_t b){
 uint32_t out=0;
 #define MAP(button,key) if(b & HidNpadButton_##button)out |= key
 MAP(A,NX_A);MAP(B,NX_B);MAP(X,NX_X);MAP(Y,NX_Y);
 MAP(Left,NX_LEFT);MAP(Up,NX_UP);MAP(Right,NX_RIGHT);MAP(Down,NX_DOWN);
 MAP(RightSL,NX_RIGHT_SL);MAP(RightSR,NX_RIGHT_SR);
 MAP(L,NX_L);MAP(R,NX_R);MAP(ZL,NX_ZL);MAP(ZR,NX_ZR);MAP(StickL,NX_L3);MAP(StickR,NX_R3);
 #undef MAP
 return out;
}

typedef struct { unsigned count, x, y; uint32_t id; } NxTouchSample;
static bool touch_sample(NxTouchSample *sample){
 HidTouchScreenState touch;
 if(appletGetOperationMode()!=AppletOperationMode_Handheld || !hidGetTouchScreenStates(&touch,1))return false;
 *sample=(NxTouchSample){0};
 for(int i=0;i<touch.count && i<16;i++){
  const HidTouchState *finger=&touch.touches[i];
  if(finger->attributes & HidTouchAttribute_End)continue;
  ++sample->count;
  sample->x=finger->x;
  sample->y=finger->y;
  sample->id=finger->finger_id;
 }
 return true;
}

static NxPointerSample poll_pointer_touch(bool *priority) {
 HidTouchScreenState state;
 if(!hidGetTouchScreenStates(&state,1)){
  *priority=pointer.owned;
  pointer.touch_armed=pointer.owned=false;
  return (NxPointerSample){0};
 }
 NxPointerFinger fingers[16];
 unsigned count=0;
 for(int i=0;i<state.count && i<16;i++){
  const HidTouchState *finger=&state.touches[i];
  if(finger->attributes & HidTouchAttribute_End){
   if(pointer.owned && finger->finger_id==pointer.owner)
    nx_pointer_position(&pointer,pointer_rect,finger->x,finger->y);
  }else
   fingers[count++]=(NxPointerFinger){finger->finger_id,finger->x,finger->y};
 }
 *priority=count!=0 || pointer.owned;
 return nx_pointer_touch(&pointer,pointer_rect,fingers,count);
}

static bool has_pointer_click(bool combat) {
 for(unsigned layer=0;layer<NX_LAYERS;layer++){
  const uint8_t *bindings=nx_bindings(&profile,combat,layer);
  for(unsigned i=0;i<NX_SOURCES;i++)
   if(!(NX_RS_DIRECTIONS & (1u << i)) && bindings[i]==NX_PC_CLICK)return true;
 }
 return false;
}

static bool run_control_panel(uint32_t previous){
 const uint32_t rail_buttons=NX_RIGHT_SL|NX_RIGHT_SR;
 previous&=~rail_buttons;
 NxControlPanel panel;
 nx_panel_begin(&panel,&profile,profile_status,combat_mode || safari_mode);
 bool dirty=true, was_focused=true, input_armed=false, touch_armed=false;
 bool was_connected=padIsConnected(&pad);
 while(appletMainLoop()){
  padUpdate(&pad);
  bool connected=padIsConnected(&pad);
  uint32_t current=connected?physical_buttons(padGetButtons(&pad)) & ~rail_buttons:0;
  bool focused=appletGetFocusState()==AppletFocusState_InFocus;
  if(focused){
   if(!was_focused || atomic_exchange(&reset_controls,false) || was_connected!=connected){
    dirty=true;
    input_armed=false;
    touch_armed=false;
    previous=current;
   }
   NxTouchSample touch={0};
   bool available=touch_sample(&touch);
   bool tap=nx_touch_begin(&touch_armed,available,touch.count);
   uint32_t pressed=input_armed?current & ~previous:0;
   if((pressed!=0)!=tap){
    int result=tap?nx_panel_touch(&panel,touch.x,touch.y,&profile):nx_panel_input(&panel,pressed,&profile);
    if(result==NX_PANEL_CLOSE)return true;
    if(result==NX_PANEL_BACK)return false;
    if(result==NX_PANEL_SEND){
     shortcut_pulse.queued=panel.shortcut_vk;
     return false;
    }
    dirty=true;
   }
   if(dirty){
    uint32_t width,height;
    NxPanelView view;
    nx_panel_text(&panel,&profile,&view);
    if(nx_gl_drawable_size(&width,&height)!=0 || !nx_control_overlay_draw(width,height,&view)
       || !eglSwapBuffers(display,surface)){
     fprintf(stderr,"CONTROLS06C panel presentation failed\n");
     return false;
    }
    dirty=false;
   }
   if(!current)input_armed=true;
  }
  previous=current;
  was_focused=focused;
  was_connected=connected;
  svcSleepThread(12000000);
 }
 return true;
}

static void combat_mode_changed(void) {
 reset_context_input();
 shortcut_pulse.active=0;
 if((combat_mode || safari_mode) && !overlay_failed && context!=EGL_NO_CONTEXT)overlay_failed=!nx_party_overlay_open();
 fprintf(stderr,"CONTEXT11 mode=%s policy=%s room=%d\n",safari_mode?"safari":combat_mode?"combat":"normal",
         profile.manual_battle?"manual":"auto",battle_context.room);
}

static void prepare_safari_profile(void) {
 safari_profile=nx_safari_profile(&profile);
 safari_chord_buttons=nx_safari_chord_buttons(&profile);
}

void nx_window_step_finished(void) {
 if(safari_mode)nx_safari_step_finished(&mapper);
 mega_input.pending=false;
}

bool nx_window_poll(uint64_t keys[4], NxPointerSample *sample) {
 *sample=(NxPointerSample){0};
 pointer_draw=*sample;
 if(!appletMainLoop())return true;
 if(!hid_ready)return false;
 padUpdate(&pad);uint64_t b=padGetButtons(&pad);
 bool focused=appletGetFocusState()==AppletFocusState_InFocus;static bool prior_focus=true;
 if(focused!=prior_focus){nx_resource_invalidate();prior_focus=focused;}
 bool reset=atomic_exchange(&reset_controls,false);
 if(reset||!focused){
  reset_context_input();
  shortcut_pulse=(NxShortcutPulse){0};
  plus_armed=false;
 }
 if(!focused)return false;
 bool next_combat=profile.manual_battle?combat_mode:battle_context.active;
 if(panel_allowed && (safari_mode!=safari_ready || combat_mode!=next_combat)){
  safari_mode=safari_ready;
  combat_mode=next_combat;
  if(safari_mode)prepare_safari_profile();
  combat_mode_changed();
  // Let Ramen publish releases before accepting any new keys or gestures.
  return false;
 }
 uint8_t shortcut=nx_shortcut_poll(&shortcut_pulse,panel_allowed);
 if(shortcut){
  nx_key_add(keys,shortcut);
  if(panel_allowed){
   fprintf(stderr,"SHORTCUT06C vk=%02X\n",(unsigned)shortcut);
   return false;
  }
 }
 uint32_t out=physical_buttons(b);
 if(panel_allowed && plus_armed && (b & HidNpadButton_Plus) && context!=EGL_NO_CONTEXT){
  reset_context_input();
  plus_armed=false;
  uint64_t start=armGetSystemTick();
  bool close=run_control_panel(out);
  if(safari_mode)prepare_safari_profile();
  double ms=1000.0*(double)(armGetSystemTick()-start)/(double)armGetSystemTickFreq();
  fprintf(stderr,"CONTROLS06C panel_ms=%.2f close=%u\n",ms,(unsigned)close);
  panel_resumed=true;
  return close;
 }
 if(!(b & HidNpadButton_Plus))plus_armed=true;
 HidAnalogStickState stick=padGetStickPos(&pad,0);
 if(stick.x < -16000)out|=NX_LEFT;
 if(stick.x > 16000)out|=NX_RIGHT;
 if(stick.y > 16000)out|=NX_UP;
 if(stick.y < -16000)out|=NX_DOWN;
 bool connected=padIsConnected(&pad);
 bool stick_keys=!scene_changing && !pointer_kind;
 HidAnalogStickState cursor_stick=padGetStickPos(&pad,1);
 out|=nx_right_stick_keys(&right_stick,connected && stick_keys,cursor_stick.x,cursor_stick.y);
 bool prior_combat=combat_mode;
 bool handheld=appletGetOperationMode()==AppletOperationMode_Handheld;
 bool default_click=!scene_changing && pointer_rect.right && !handheld &&
                    pointer_kind!=NX_POINTER_FLY && !has_pointer_click(combat_mode);
 bool r3=out & NX_R3;
 if(default_click)out&=~NX_R3;
 if(casino_changed){
  casino_changed=false;
  return false; /* Publish all releases before arming the new context. */
 }
 if(connected && casino_kind){
  NxProfile effective=nx_casino_profile(&profile,casino_keys);
  bool normal=false;
  nx_mapper_poll(&mapper,&effective,&normal,out,b & HidNpadButton_Minus,stick_keys,keys);
 }else if(connected && safari_mode)
  nx_safari_mapper_poll(&mapper,&safari_profile,safari_chord_buttons,out,b & HidNpadButton_Minus,stick_keys,keys);
 else if(connected)
  nx_mapper_poll(&mapper,&profile,&combat_mode,out,b & HidNpadButton_Minus,stick_keys,keys);
 else
  mapper=(NxMapper){0};
 if(prior_combat!=combat_mode){
  combat_mode_changed();
  for(unsigned i=0;i<4;i++)keys[i]=0;
  return false;
 }
 bool mapped_click=keys[0] & (UINT64_C(1)<<NX_PC_CLICK);
 keys[0]&=~(UINT64_C(1)<<NX_PC_CLICK);
 if(scene_changing)return false;
 if(pointer_kind){
  nx_controls_touch(&controls,false,0,0,0,0);
  if(!pointer_rect.right)return false;
  bool touch_priority=false;
  if(handheld)*sample=poll_pointer_touch(&touch_priority);
  if(touch_priority || !connected){
   pointer.stick_armed=pointer.stick_down=false;
   pointer_tick=0;
  }else{
   uint64_t now=armGetSystemTick();
   float seconds=pointer_tick?(float)(now-pointer_tick)/armGetSystemTickFreq():0;
   pointer_tick=now;
   *sample=nx_pointer_stick(&pointer,pointer_rect,cursor_stick.x,cursor_stick.y,seconds,
                            default_click?r3:mapped_click);
  }
  if(pointer_kind==NX_POINTER_MAP && (sample->flags & NX_POINTER_DOWN))map_pointer_seen=true;
  if(!handheld || pointer.stick_visible || map_pointer_seen || (sample->flags & NX_POINTER_DOWN))
   pointer_draw=*sample;
  return false;
 }
 bool mega=battle_context.active && !safari_mode;
 NxTouchSample touch;
 unsigned previous_slot=controls.slot;
 if(!(combat_mode || safari_mode) || !overlay_presented || !touch_sample(&touch)){
  nx_controls_touch(&controls,false,0,0,0,0);
 }else{
  nx_controls_touch_layout(&controls,true,touch.count,touch.id,touch.x,touch.y,safari_mode,mega && mega_presented);
 }
 uint8_t vk=safari_mode?nx_safari_touch_vk(controls.slot):nx_battle_touch_vk(controls.slot);
 nx_key_add(keys,vk);
 nx_mega_keep_until_step(&mega_input,mega,keys);
 if(safari_mode){
  if(vk && controls.slot!=previous_slot)fprintf(stderr,"SAFARI12 touch vk=%02X\n",(unsigned)vk);
 }else if(controls.slot==NX_FLEE_SLOT){
  if(previous_slot!=NX_FLEE_SLOT)++flee_presses;
 }else if(controls.slot==NX_MEGA_SLOT){
  if(previous_slot!=NX_MEGA_SLOT)++mega_presses;
 }else if(controls.slot){
  if(controls.slot!=previous_slot)++touch_presses[controls.slot-1];
 }
 return false;
}
void nx_gl_close(void) {
 if(resource_focus_hook_ready){appletUnhook(&resource_focus_cookie);resource_focus_hook_ready=false;}
 if(display!=EGL_NO_DISPLAY) {
  nx_party_overlay_close();
  nx_gpu_close();
  eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
  if(context!=EGL_NO_CONTEXT)eglDestroyContext(display,context);
  if(surface!=EGL_NO_SURFACE)eglDestroySurface(display,surface);
  eglTerminate(display);
 }
 display=EGL_NO_DISPLAY;context=EGL_NO_CONTEXT;surface=EGL_NO_SURFACE;
 if(vsync_ready){eventClose(&vsync);viCloseDisplay(&vi_display);vsync_ready=false;}
 if(hid_ready)fprintf(stderr,"TOUCH06 presses=%u,%u,%u,%u,%u,%u\n",
                     touch_presses[0],touch_presses[1],touch_presses[2],touch_presses[3],touch_presses[4],touch_presses[5]);
 if(hid_ready)fprintf(stderr,"TOUCH06D flee=%u\n",flee_presses);
 if(hid_ready)fprintf(stderr,"TOUCH14 mega=%u\n",mega_presses);
 hid_ready=false;
}
int nx_gl_open(void *native) {
 const char *stage="getDisplay";
 display=eglGetDisplay(EGL_DEFAULT_DISPLAY);if(display==EGL_NO_DISPLAY)return fail("getDisplay");
 stage="initialize";
 if(!eglInitialize(display,NULL,NULL))goto failure;
 stage="bindOpenGL";
 if(!eglBindAPI(EGL_OPENGL_API))goto failure;
 const EGLint attrs[]={EGL_RENDERABLE_TYPE,EGL_OPENGL_BIT,EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_DEPTH_SIZE,24,EGL_STENCIL_SIZE,8,EGL_NONE};
 EGLConfig config;EGLint count;
 stage="chooseConfig";
 if(!eglChooseConfig(display,attrs,&config,1,&count)||count!=1)goto failure;
 stage="windowSurface";
 surface=eglCreateWindowSurface(display,config,(EGLNativeWindowType)native,NULL);if(surface==EGL_NO_SURFACE)goto failure;
 const EGLint ctx[]={EGL_CONTEXT_MAJOR_VERSION_KHR,3,EGL_CONTEXT_MINOR_VERSION_KHR,3,EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,EGL_NONE};
 stage="OpenGL3.3coreContext";
 context=eglCreateContext(display,config,EGL_NO_CONTEXT,ctx);if(context==EGL_NO_CONTEXT)goto failure;
 stage="makeCurrent";
 if(!eglMakeCurrent(display,surface,surface,context))goto failure;
 stage="swapInterval";
 if(!eglSwapInterval(display,1))goto failure;
 Result rc=viOpenDefaultDisplay(&vi_display);
 if(R_SUCCEEDED(rc)){rc=viGetDisplayVsyncEvent(&vi_display,&vsync);if(R_SUCCEEDED(rc))vsync_ready=true;else viCloseDisplay(&vi_display);}
 nx_gpu_init();
 fprintf(stderr,"EGL_READY OpenGL3.3-core vsync_event=%08x\n",rc);fflush(stderr);
 return 0;
 failure:fail(stage);nx_gl_close();return -1;
}
const void *nx_gl_proc(const char *name) {
 const void *fn=(const void *)eglGetProcAddress(name);
 if(!fn){fprintf(stderr,"GL_MISSING %s\n",name);fflush(stderr);}
 return fn;
}
int nx_gl_drawable_size(uint32_t *w,uint32_t *h){
 if(!window||display==EGL_NO_DISPLAY||surface==EGL_NO_SURFACE||!w||!h)return -1;
 /* Pinned switch-mesa 20.1.0-5 allocates its buffers using exactly this
  * NWindow query, but never sets _EGLSurface.Width/Height. eglQuerySurface
  * therefore returns success with 0x0, even with a working context. Use
  * the actual allocation dimensions, not a guessed/fixed resolution. */
 uint32_t width=0,height=0;Result rc=nwindowGetDimensions(window,&width,&height);
 if(R_FAILED(rc)||!width||!height){fprintf(stderr,"DRAWABLE04 FAIL nwindow rc=%08x size=%ux%u\n",rc,width,height);return -1;}
 static bool reported;
 if(!reported){fprintf(stderr,"DRAWABLE04 source=NWindow allocation=%ux%u mesa_egl_size_metadata_unused=true\n",width,height);reported=true;}
 *w=width;*h=height;return 0;
}
int nx_gl_swap(void){
 uint32_t width,height;
 bool draw=!scene_changing && !pointer_kind && (combat_mode || safari_mode) && !overlay_failed && nx_party_overlay_open() && !atomic_load(&reset_controls)
  && appletGetFocusState()==AppletFocusState_InFocus
  && appletGetOperationMode()==AppletOperationMode_Handheld
  && nx_gl_drawable_size(&width,&height)==0;
 mega_presented=false;
 if(draw && safari_mode)draw=nx_safari_overlay_draw(width,height,controls.slot);
 else if(draw)mega_presented=nx_party_overlay_draw(width,height,controls.slot,battle_context.active);
 if(!scene_changing && pointer_draw.flags && !atomic_load(&reset_controls)
    && appletGetFocusState()==AppletFocusState_InFocus
    && nx_gl_drawable_size(&width,&height)==0)
  nx_pointer_overlay_draw(width,height,pointer_draw.x,pointer_draw.y,pointer_draw.flags & NX_POINTER_DOWN);
 nx_gpu_end();uint64_t t=armGetSystemTick();int ok=eglSwapBuffers(display,surface);
 nx_perf_add(7,1);nx_perf_add(8,armGetSystemTick()-t);
 overlay_presented=ok && draw;
 if(!ok)return fail("swapBuffers");
 return 0;
}
int nx_gl_interval(uint32_t n){fprintf(stderr,"VSYNC interval_requested=%u\n",n);if(!eglSwapInterval(display,(EGLint)n))return fail("swapInterval");return 0;}
int nx_gl_wait_vsync(void){if(!vsync_ready)return -1;uint64_t t=armGetSystemTick();Result rc=eventWait(&vsync,UINT64_MAX);nx_perf_add(9,1);nx_perf_add(10,armGetSystemTick()-t);return R_SUCCEEDED(rc)?0:-1;}
