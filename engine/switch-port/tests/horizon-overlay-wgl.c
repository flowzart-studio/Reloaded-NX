/* SPDX-License-Identifier: MIT. Host test for the production overlay shader. */
#include <windows.h>
#include <EGL/egl.h>
#include <GL/gl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../horizon-controls.h"
#include "../horizon-party-overlay.h"

__eglMustCastToProperFunctionPointerType EGLAPIENTRY eglGetProcAddress(const char *name) {
    PROC result = wglGetProcAddress(name);
    if (!result) result = GetProcAddress(GetModuleHandleA("opengl32.dll"), name);
    return (__eglMustCastToProperFunctionPointerType)result;
}

static unsigned image_height=360;
static void pixel(unsigned x, unsigned y, unsigned char rgb[3]) {
    glReadPixels(x, image_height - 1 - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, rgb);
}

static void capture(const char *path, unsigned width, unsigned height) {
    unsigned stride=width*3;
    unsigned char *rgb=malloc(stride*height);
    assert(rgb);
    glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,rgb);
    FILE *file=fopen(path,"wb");
    assert(file);
    fprintf(file,"P6\n%u %u\n255\n",width,height);
    for(unsigned y=height;y>0;--y)assert(fwrite(rgb+(y-1)*stride,1,stride,file)==stride);
    assert(fclose(file)==0);
    free(rgb);
}

int nx_control_save(const NxProfile *profile) { (void)profile; return 0; }

int main(int argc, char **argv) {
    HWND window = CreateWindowA("STATIC", "", WS_POPUP, 0, 0, 1280, 720, 0, 0, 0, 0);
    assert(window);
    HDC dc = GetDC(window);
    PIXELFORMATDESCRIPTOR pfd = {
        .nSize = sizeof(pfd), .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
        .iPixelType = PFD_TYPE_RGBA, .cColorBits = 32,
    };
    int format = ChoosePixelFormat(dc, &pfd);
    assert(format && SetPixelFormat(dc, format, &pfd));
    HGLRC context = wglCreateContext(dc);
    assert(context && wglMakeCurrent(dc, context));
    printf("GL_VERSION=%s\n", glGetString(GL_VERSION));
    assert(nx_party_overlay_open());
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glViewport(2, 3, 40, 50);
    glScissor(0, 0, 1, 1);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_CULL_FACE);
    assert(nx_party_overlay_draw(640, 360, 2, true));
    assert(glGetError() == 0);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    assert(viewport[0] == 2 && viewport[1] == 3 && viewport[2] == 40 && viewport[3] == 50);
    assert(glIsEnabled(GL_SCISSOR_TEST) && glIsEnabled(GL_CULL_FACE) && !glIsEnabled(GL_BLEND));
    unsigned char rgb[3];
    pixel(589, 40, rgb);
    assert(rgb[0] >= 126 && rgb[0] <= 129);
    const char *digits[] = {"010110010010111","111001111100111","111001111001111",
                            "101101111001001","111100111001111","111100111101111"};
    for (int i = 0; i < 6; ++i) {
        pixel(595, 50 + i * 48, rgb);
        assert(rgb[0] < 90 && rgb[2] > rgb[0]);
        if (i == 1) assert(rgb[2] > 110);
        for (int j = 0; j < 15; ++j) {
            pixel(605 + (j % 3) * 5, 50 + i * 48 + (j / 3) * 5, rgb);
            assert((rgb[0] > 220 && rgb[2] > 220) == (digits[i][j] == '1'));
        }
        pixel(610, 82 + i * 48, rgb);
        assert(rgb[0] >= 126 && rgb[0] <= 129);
    }
    bool flee_label=false;
    for(unsigned x=517;x<566;++x)for(unsigned y=306;y<320;++y){
        pixel(x,y,rgb);
        if(rgb[0]>220 && rgb[1]>190)flee_label=true;
    }
    assert(flee_label);
    bool mega_label=false;
    pixel(510,276,rgb);
    assert(rgb[0]<65 && rgb[2]>rgb[0]);
    for(unsigned x=515;x<564;++x)for(unsigned y=270;y<285;++y){
        pixel(x,y,rgb);
        if(rgb[0]>220 && rgb[1]>190)mega_label=true;
    }
    assert(mega_label);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
    assert(nx_safari_overlay_draw(640,360,2));
    pixel(510,276,rgb);
    assert(rgb[0]>=126 && rgb[0]<=129); /* Safari has no Mega button. */
    pixel(595,98,rgb);
    assert(rgb[2] > 110 && rgb[0] < 90); /* Safari Baya is highlighted. */
    pixel(600,290,rgb);
    assert(rgb[0] >= 126 && rgb[0] <= 129); /* Party slot 6 is absent. */
    for(unsigned i=0;i<NX_SAFARI_COUNT;++i){
        bool safari_label=false;
        for(unsigned x=591;x<629;++x)for(unsigned y=56+i*48;y<67+i*48;++y){
            pixel(x,y,rgb);
            if(rgb[0]>220 && rgb[1]>220)safari_label=true;
        }
        assert(safari_label);
    }
    NxPanelView view = {.rows = {{"Árbol y Pokédex"},{"","Combate"},{"",""},{"B"}}, .row=3, .column=-1};
    assert(nx_control_overlay_draw(640,360,&view));
    assert(glGetError() == 0);
    pixel(500,100,rgb);
    assert(rgb[2] > 90 && rgb[0] < 40); /* Selected row. */
    pixel(500,125,rgb);
    assert(rgb[2] < 45); /* Plain backdrop. */
    bool glyph = false, smooth = false;
    for(unsigned x=40;x<142;++x)for(unsigned y=32;y<48;++y){
        pixel(x,y,rgb);
        if(rgb[1]>175)glyph=true;
        if(rgb[1]>55 && rgb[1]<155)smooth=true;
    }
    assert(glyph && smooth);
    glGetIntegerv(GL_VIEWPORT, viewport);
    assert(viewport[0] == 2 && viewport[1] == 3 && viewport[2] == 40 && viewport[3] == 50);
    assert(glIsEnabled(GL_SCISSOR_TEST) && glIsEnabled(GL_CULL_FACE) && !glIsEnabled(GL_BLEND));

    image_height=720;
    NxProfile profile=nx_safe_profile;
    NxControlPanel panel;
    nx_panel_begin(&panel,&profile,NULL,false);
    nx_panel_input(&panel,NX_A,&profile);
    nx_panel_input(&panel,NX_A,&profile);
    nx_panel_text(&panel,&nx_safe_profile,&view);
    assert(nx_control_overlay_draw(1280,720,&view));
    assert(glGetError()==0);
    bool native_glyph=false;
    for(unsigned x=80;x<280;++x)for(unsigned y=62;y<90;++y){
        pixel(x,y,rgb);
        if(rgb[1]>175)native_glyph=true;
    }
    assert(native_glyph);
    if (argc>1) capture(argv[1],1280,720);
    nx_pointer_overlay_draw(1280,720,640,360,false);
    pixel(640,360,rgb);
    assert(rgb[0]>245 && rgb[1]>245 && rgb[2]>245);
    nx_pointer_overlay_draw(1280,720,640,360,true);
    pixel(640,360,rgb);
    assert(rgb[0]>245 && rgb[1]>190 && rgb[2]<70);
    glGetIntegerv(GL_VIEWPORT, viewport);
    assert(viewport[0] == 2 && viewport[1] == 3 && viewport[2] == 40 && viewport[3] == 50);
    assert(glIsEnabled(GL_SCISSOR_TEST) && glIsEnabled(GL_CULL_FACE) && !glIsEnabled(GL_BLEND));
    assert(glGetError()==0);
    nx_party_overlay_close();
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(context);
    ReleaseDC(window, dc);
    DestroyWindow(window);
    puts("Horizon party, Huir, Safari, Noto antialiasing, PC pointer and native GL state: PASS");
}
