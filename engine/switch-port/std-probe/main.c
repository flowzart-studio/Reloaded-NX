/* SPDX-License-Identifier: MIT. Diagnostic only: never loads the game. */
#include <switch.h>
#include <stdio.h>
#include <sys/stat.h>
extern int nx_std_probe(void);
static int result=-1;
static void worker(void *unused) { (void)unused; result=nx_std_probe(); }
int main(int argc,char **argv) {
    (void)argc; (void)argv;
    consoleInit(NULL);
    mkdir("sdmc:/switch/reloaded-nx",0777);
    mkdir("sdmc:/switch/reloaded-nx/diagnostics",0777);
    FILE *log=fopen("sdmc:/switch/reloaded-nx/diagnostics/std-probe.log","a");
    if(log) { fprintf(log,"START std/fs/pthread/OS-key TLS probe\n"); fflush(log); }
    freopen("sdmc:/switch/reloaded-nx/diagnostics/std-errors.log","a",stderr);
    Thread thread;
    Result rc=csrngInitialize();
    if(R_SUCCEEDED(rc)) rc=threadCreate(&thread,worker,NULL,NULL,8*1024*1024,0x2c,-2);
    if(R_SUCCEEDED(rc)) { rc=threadStart(&thread); if(R_SUCCEEDED(rc)) threadWaitForExit(&thread); threadClose(&thread); }
    if(log) { fprintf(log,"END libnx=%08x rust=%d (0=PASS)\n",rc,result); fclose(log); }
    printf("std probe: libnx=%08x rust=%d (0=PASS)\nPress + to exit.\n",rc,result);
    PadState pad; padConfigureInput(1,HidNpadStyleSet_NpadStandard); padInitializeDefault(&pad);
    while(appletMainLoop()) { padUpdate(&pad); if(padGetButtonsDown(&pad)&HidNpadButton_Plus) break; consoleUpdate(NULL); }
    csrngExit(); consoleExit(NULL); return result;
}
