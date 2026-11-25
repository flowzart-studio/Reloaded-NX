/* SPDX-License-Identifier: MIT. Full OpenGMK runner bootstrap, not the H13 kernel. */
#include <switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/iosupport.h>
#include <string.h>
#include <reent.h>
#include "horizon-log.h"
extern int nx_full_run(void);
extern int nx_perf_io_init(void);
extern void nx_perf_resource_report(void);
extern void nx_perf_add(uint32_t,uint64_t);
extern void nx_gl_close(void);
static FILE *log_file;
static Mutex log_mutex;
static ssize_t log_write(struct _reent *r,void *fd,const char *data,size_t size){
 (void)r;(void)fd;uint64_t started=armGetSystemTick();mutexLock(&log_mutex);
 if(!log_file){mutexUnlock(&log_mutex);return (ssize_t)size;}
 size_t written=0;int rc=nx_log_emit(log_file,data,size,&written);
 if(rc!=0 || written!=size){fclose(log_file);log_file=NULL;}
 mutexUnlock(&log_mutex);
 nx_perf_add(4,1);nx_perf_add(5,written);nx_perf_add(6,armGetSystemTick()-started);
 // A read-only or full SD must not turn a diagnostic write into a Rust panic.
 return (ssize_t)size;
}
static const devoptab_t log_device={.name="reloaded-log",.write_r=log_write};
static int result=-1;
static void worker(void *unused){(void)unused;result=nx_full_run();nx_gl_close();}
static void exit_screen(const char *message){
 nwindowSetDimensions(nwindowGetDefault(),1280,720);
 consoleInit(NULL);
 char text[512];int n=snprintf(text,sizeof(text),"Reloaded-NX 1.6.1 - Flowzart Studio\n%s\n%s\nPress + to exit\n",message,
                             log_file?"Logs: /switch/reloaded-nx/logs/":"Log unavailable");
 // stdout/stderr deliberately remain log files. Write the status screen through
 // libnx's registered console device rather than redirecting or losing logs.
 if(n>0)devoptab_list[STD_OUT]->write_r(_REENT,NULL,text,(size_t)n);
 PadState p;padConfigureInput(1,HidNpadStyleSet_NpadStandard);padInitializeDefault(&p);
 while(appletMainLoop()){padUpdate(&p);if(padGetButtonsDown(&p)&HidNpadButton_Plus)break;consoleUpdate(NULL);}
 consoleExit(NULL);
}
int main(int argc,char **argv){
 (void)argc;(void)argv;
 mkdir("sdmc:/switch/reloaded-nx",0777);mkdir("sdmc:/switch/reloaded-nx/logs",0777);
 mkdir("sdmc:/switch/reloaded-nx/session",0777);
 const char *log_path="sdmc:/switch/reloaded-nx/logs/reloaded-nx.log";
 // This descriptor belongs to the engine. Registering it as a game writer
 // holds the resource epoch odd until exit, disabling both H05 caches.
 log_file=fopen(log_path,"a");
 if(nx_perf_io_init()!=0){
  if(log_file){fclose(log_file);log_file=NULL;}
  exit_screen("SD profiling initialization failed");return 1;
 }
 // Both C stdio and Rust std::io use fd 1/2. Keep those descriptors native and
 // redirect their devoptab, rather than changing only FILE* streams via freopen.
 devoptab_list[STD_OUT]=&log_device;devoptab_list[STD_ERR]=&log_device;
 setvbuf(stdout,NULL,_IONBF,0);setvbuf(stderr,NULL,_IONBF,0);
 printf("BOOT full OpenGMK Horizon; version=1.6.1; Application Mode required; applet=%d\n",appletGetAppletType());
 printf("BATTLE11 mode legacy_log_epoch=0 log_open=%d\n",log_file!=NULL);
 if(appletGetAppletType()!=AppletType_Application && appletGetAppletType()!=AppletType_SystemApplication){exit_screen("Application Mode required (hold R while launching a title)");return 1;}
 uint64_t total=0,used=0,is_application=0;
 Result total_rc=svcGetInfo(&total,InfoType_TotalMemorySize,CUR_PROCESS_HANDLE,0);
 Result used_rc=svcGetInfo(&used,InfoType_UsedMemorySize,CUR_PROCESS_HANDLE,0);
 Result application_rc=svcGetInfo(&is_application,InfoType_IsApplication,CUR_PROCESS_HANDLE,0);
 printf("MEMORY12 total=%llu used=%llu total_rc=%08x used_rc=%08x loader_heap=%zu kernel_application=%llu application_rc=%08x\n",
        (unsigned long long)total,(unsigned long long)used,total_rc,used_rc,
        envHasHeapOverride()?envGetHeapOverrideSize():0,(unsigned long long)is_application,application_rc);
 Result rc=csrngInitialize();if(R_FAILED(rc)){fprintf(stderr,"CSRNG_INIT %08x\n",rc);exit_screen("CSRNG initialization failed");return 1;}
 Thread t;rc=threadCreate(&t,worker,NULL,NULL,16*1024*1024,0x2c,-2);
 if(R_SUCCEEDED(rc)){rc=threadStart(&t);if(R_SUCCEEDED(rc))threadWaitForExit(&t);threadClose(&t);}
 nx_perf_resource_report();
 fprintf(stderr,"BOOT_END libnx=%08x rust=%d\n",rc,result);fflush(NULL);
 csrngExit();exit_screen(result==0&&R_SUCCEEDED(rc)?"Runner returned normally":"Runner failed");
 if(log_file){fclose(log_file);log_file=NULL;}
 return result;
}
