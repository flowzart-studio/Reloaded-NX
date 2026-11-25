/* SPDX-License-Identifier: MIT. Cumulative profiling: actual SD devoptab, no syscall replacements. */
#include <switch.h>
#include <sys/iosupport.h>
#include <stdatomic.h>
#include <EGL/egl.h>
#include <stdio.h>
#include "horizon-resource-epoch.h"
static _Atomic uint64_t counters[19];
uint64_t nx_perf_counter(uint32_t n){return n<19?atomic_load_explicit(&counters[n],memory_order_relaxed):0;}
uint64_t nx_perf_frequency(void){return armGetSystemTickFreq();}
void nx_perf_add(uint32_t n,uint64_t v){if(n<19)atomic_fetch_add_explicit(&counters[n],v,memory_order_relaxed);}
/* Foreground-only deltas. The index and logger keep their existing global
 * calls on their own threads are excluded. Foreground log writes may be in
 * TraceOther. Clock ticks are system counter ticks. */
enum { TraceOpen, TraceStat, TraceRead, TraceFstat, TraceOther, TraceIoCount };
static _Atomic(void*) trace_thread;
static uint64_t trace_io[TraceIoCount * 2 + 1];
void nx_trace_init(void){atomic_store(&trace_thread,armGetTls());}
void nx_trace_io(uint64_t *out){memcpy(out,trace_io,sizeof trace_io);}
uint64_t nx_trace_ticks(void){return armGetSystemTick();}
uint64_t nx_trace_cpu(void){
 uint64_t ticks;
 InfoType type=hosversionAtLeast(13,0,0)?InfoType_ThreadTickCount:InfoType_ThreadTickCountDeprecated;
 return R_SUCCEEDED(svcGetInfo(&ticks,type,CUR_THREAD_HANDLE,TickCountInfo_Total))?ticks:UINT64_MAX;
}
uint64_t nx_trace_ram(void){
 uint64_t bytes;
 return R_SUCCEEDED(svcGetInfo(&bytes,InfoType_UsedMemorySize,CUR_PROCESS_HANDLE,0))?bytes:UINT64_MAX;
}
static void trace_io_done(unsigned phase,uint64_t ticks,uint64_t bytes){
 if(atomic_load_explicit(&trace_thread,memory_order_relaxed)!=armGetTls())return;
 trace_io[phase*2]+=ticks;
 trace_io[phase*2+1]++;
 trace_io[TraceIoCount*2]+=bytes;
}
static devoptab_t original,profiled;
static FsFileSystem *resource_sd;
static _Atomic uint64_t resource_attempts[ResourcePathCount];
static ResourcePath resource_resolve(struct _reent *r,const char *path){
 char translated[FS_MAX_PATH];
 FsFileSystem *device=NULL;
 const int saved_errno=r->_errno;
 int rc=fsdevTranslatePath(path,&device,translated);
 r->_errno=saved_errno;
 if(rc!=0||device!=resource_sd)return ResourceUnknown;
 resource_collapse_slashes(translated);
 return resource_path(translated);
}
static ResourcePath resource_classify(struct _reent *r,const char *path){
 ResourcePath kind=resource_resolve(r,path);
 atomic_fetch_add_explicit(&resource_attempts[kind],1,memory_order_relaxed);
 return kind;
}
void nx_perf_resource_report(void){
 fprintf(stderr,"EPOCH11 attempts save=%llu profile=%llu sprites=%llu unknown=%llu epoch=%llu\n",
  (unsigned long long)atomic_load(&resource_attempts[ResourceSave]),
  (unsigned long long)atomic_load(&resource_attempts[ResourceProfile]),
  (unsigned long long)atomic_load(&resource_attempts[ResourceSprites]),
  (unsigned long long)atomic_load(&resource_attempts[ResourceUnknown]),
  (unsigned long long)nx_resource_epoch());
}
static void io_done(uint64_t start,unsigned phase,uint64_t bytes){
 uint64_t ticks=armGetSystemTick()-start;
 nx_perf_add(0,1);nx_perf_add(3,ticks);
 trace_io_done(phase,ticks,bytes);
}
/* Only completed operations >=100 ms, capped at 64/session. Avoid recursive
 * diagnostics through the SD logger; preserve the real operation's errno. */
static atomic_flag slow_log_busy=ATOMIC_FLAG_INIT;
static _Atomic unsigned slow_logs;
static void slow_io(struct _reent*r,const char*op,uint64_t start,const char*path){
 uint64_t ticks=armGetSystemTick()-start;
 if(ticks<armGetSystemTickFreq()/10)return;
 if(atomic_flag_test_and_set_explicit(&slow_log_busy,memory_order_relaxed))return;
 int saved_errno=r->_errno;unsigned n=atomic_fetch_add_explicit(&slow_logs,1,memory_order_relaxed);
 if(n<64)fprintf(stderr,"IO04 op=%s ms=%.2f path=%s\n",op,1000.0*(double)ticks/(double)armGetSystemTickFreq(),path?path:"(see enclosing LOAD03)");
 else if(n==64)fprintf(stderr,"IO04 further_slow_operations_suppressed=true\n");
 r->_errno=saved_errno;atomic_flag_clear_explicit(&slow_log_busy,memory_order_relaxed);
}
#define WRAP(ret,name,args,call) static ret perf_##name args {uint64_t t=armGetSystemTick();ret result=original.name call;io_done(t,TraceOther,0);slow_io(r,#name,t,NULL);return result;}
static int perf_open_r(struct _reent*r,void*f,const char*p,int flags,int mode){ResourcePath kind=resource_is_write(flags)?resource_classify(r,p):ResourceUnknown;bool tracked=resource_open_begin(f,kind,flags);uint64_t t=armGetSystemTick();int v=original.open_r(r,f,p,flags,mode);if(v<0&&tracked)resource_close(f);io_done(t,TraceOpen,0);nx_perf_add(14,1);slow_io(r,"open_r",t,p);return v;}
static int perf_close_r(struct _reent*r,void*f){uint64_t t=armGetSystemTick();int v=original.close_r(r,f);resource_close(f);io_done(t,TraceOther,0);slow_io(r,"close_r",t,NULL);return v;}
WRAP(off_t,seek_r,(struct _reent*r,void*f,off_t o,int d),(r,f,o,d))
static int perf_stat_r(struct _reent*r,const char*p,struct stat*s){uint64_t t=armGetSystemTick();int v=original.stat_r(r,p,s);io_done(t,TraceStat,0);nx_perf_add(15,1);slow_io(r,"stat_r",t,p);return v;}
static int perf_fstat_r(struct _reent*r,void*f,struct stat*s){uint64_t t=armGetSystemTick();int v=original.fstat_r(r,f,s);io_done(t,TraceFstat,0);nx_perf_add(17,1);slow_io(r,"fstat_r",t,NULL);return v;}
WRAP(int,fsync_r,(struct _reent*r,void*f),(r,f))
WRAP(DIR_ITER*,diropen_r,(struct _reent*r,DIR_ITER*d,const char*p),(r,d,p))
static int perf_dirnext_r(struct _reent*r,DIR_ITER*d,char*p,struct stat*s){uint64_t t=armGetSystemTick();int v=original.dirnext_r(r,d,p,s);io_done(t,TraceOther,0);nx_perf_add(18,1);return v;}
WRAP(int,dirclose_r,(struct _reent*r,DIR_ITER*d),(r,d))
static ssize_t perf_read_r(struct _reent*r,void*f,char*p,size_t n){uint64_t t=armGetSystemTick();ssize_t v=original.read_r(r,f,p,n);io_done(t,TraceRead,v>0?(uint64_t)v:0);if(v>0)nx_perf_add(1,v);nx_perf_add(16,1);slow_io(r,"read_r",t,NULL);return v;}
static ssize_t perf_write_r(struct _reent*r,void*f,const char*p,size_t n){uint64_t t=armGetSystemTick();ssize_t v=original.write_r(r,f,p,n);io_done(t,TraceOther,0);if(v>0)nx_perf_add(2,v);return v;}

#define MUTATE(name,args,call,track) static int perf_##name args { \
 bool tracked=(track);if(tracked)resource_mutation_begin(); \
 uint64_t t=armGetSystemTick();int v=original.name call; \
 if(tracked){resource_mutation_end();}io_done(t,TraceOther,0);return v; \
}
static bool resource_rename_tracks(struct _reent *r,const char *from,const char *to){
 ResourcePath a=resource_classify(r,from),b=resource_classify(r,to);
 return !resource_disjoint(a)||!resource_disjoint(b);
}
MUTATE(unlink_r,(struct _reent*r,const char*p),(r,p),!resource_disjoint(resource_classify(r,p)))
MUTATE(rename_r,(struct _reent*r,const char*a,const char*b),(r,a,b),resource_rename_tracks(r,a,b))
MUTATE(mkdir_r,(struct _reent*r,const char*p,int m),(r,p,m),!resource_disjoint(resource_classify(r,p)))
MUTATE(rmdir_r,(struct _reent*r,const char*p),(r,p),!resource_disjoint(resource_classify(r,p)))
/* A link can introduce aliasing. Keep it conservative even with two known paths. */
MUTATE(link_r,(struct _reent*r,const char*a,const char*b),(r,a,b),(resource_classify(r,a),resource_classify(r,b),true))
int nx_perf_io_init(void){
 const devoptab_t*d=GetDeviceOpTab("sdmc:");resource_sd=fsdevGetDeviceFileSystem("sdmc");if(!d||!resource_sd)return -1;
 int index=-1;for(int i=3;i<STD_MAX;i++)if(devoptab_list[i]==d){index=i;break;}if(index<0)return -1;
 original=*d;profiled=*d;
 #define INSTALL(name) if(original.name)profiled.name=perf_##name
 INSTALL(unlink_r);INSTALL(rename_r);INSTALL(mkdir_r);INSTALL(rmdir_r);INSTALL(link_r);INSTALL(open_r);INSTALL(close_r);INSTALL(read_r);INSTALL(write_r);INSTALL(seek_r);INSTALL(stat_r);INSTALL(fstat_r);INSTALL(fsync_r);INSTALL(diropen_r);INSTALL(dirnext_r);INSTALL(dirclose_r);
 #undef INSTALL
 devoptab_list[index]=&profiled;return 0;
}
/* Async GL_TIME_ELAPSED queries. Availability is checked; never glFinish or wait for a query. */
static void(*gen)(int,unsigned*);static void(*del)(int,const unsigned*);
static void(*begin)(unsigned,unsigned);static void(*end)(unsigned);
static void(*available)(unsigned,unsigned,int*);static void(*result64)(unsigned,unsigned,uint64_t*);
static unsigned queries[3];static bool pending[3],ready;static int active=-1;
void nx_gpu_init(void){
 gen=(void*)eglGetProcAddress("glGenQueries");del=(void*)eglGetProcAddress("glDeleteQueries");
 begin=(void*)eglGetProcAddress("glBeginQuery");end=(void*)eglGetProcAddress("glEndQuery");
 available=(void*)eglGetProcAddress("glGetQueryObjectiv");result64=(void*)eglGetProcAddress("glGetQueryObjectui64v");
 ready=gen&&del&&begin&&end&&available&&result64;
 if(ready)gen(3,queries);
 fprintf(stderr,"PERF_GPU async_timer_query=%d (elapsed GPU timeline, not utilization)\n",ready);
}
void nx_gpu_begin(void){
 if(!ready||active>=0)return;
 for(int i=0;i<3;i++)if(pending[i]){int done=0;available(queries[i],0x8867,&done);if(done){uint64_t ns=0;result64(queries[i],0x8866,&ns);nx_perf_add(11,1);nx_perf_add(12,ns);pending[i]=false;}}
 for(int i=0;i<3;i++)if(!pending[i]){active=i;begin(0x88BF,queries[i]);return;}
 nx_perf_add(13,1);
}
void nx_gpu_end(void){if(active>=0){end(0x88BF);pending[active]=true;active=-1;}}
void nx_gpu_close(void){if(ready){nx_gpu_end();del(3,queries);}ready=false;for(int i=0;i<3;i++)pending[i]=false;}
