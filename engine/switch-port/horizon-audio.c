/* SPDX-License-Identifier: MIT. libnx audout sink; mixer and decoding stay in OpenGMK.
 * Reuses the page-aligned buffer/stop-before-free ownership pattern from H11-H13.
 */
#include <switch.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdatomic.h>
#include "horizon-pcm.h"
static AudioOutBuffer buffers[3],*current;
static bool initialized;static _Atomic uint64_t stats[4];
uint64_t nx_pcm_counter(uint32_t n){return n<4?atomic_load_explicit(&stats[n],memory_order_relaxed):0;}
static int failure(const char*stage,Result rc){atomic_store(&stats[3],rc?rc:UINT32_MAX);fprintf(stderr,"AUDIO_FAIL %s rc=%08x\n",stage,rc);return -1;}
void nx_pcm_close(void){
 if(!initialized)return;
 Result rc=audoutStopAudioOut();if(R_FAILED(rc))failure("stop",rc);
 audoutExit(); /* release driver ownership before freeing buffers */
 for(int i=0;i<3;i++){free(buffers[i].buffer);memset(&buffers[i],0,sizeof(buffers[i]));}
 current=NULL;initialized=false;
 fprintf(stderr,"AUDIO_STOP frames=%llu error=%llu\n",(unsigned long long)nx_pcm_counter(0),(unsigned long long)nx_pcm_counter(3));
}
int nx_pcm_open(void){
 if(initialized)return failure("already open",0);
 for(int i=0;i<4;i++)atomic_store(&stats[i],0);
 Result rc=audoutInitialize();if(R_FAILED(rc))return failure("initialize",rc);
 initialized=true;
 if(audoutGetSampleRate()!=48000||audoutGetChannelCount()!=2||audoutGetPcmFormat()!=PcmFormat_Int16){failure("PCM requires stereo S16 48000",0);nx_pcm_close();return -1;}
 for(int i=0;i<3;i++){
  buffers[i].buffer=aligned_alloc(4096,4096);if(!buffers[i].buffer){failure("allocation",0);nx_pcm_close();return -1;}
  memset(buffers[i].buffer,0,4096);buffers[i].buffer_size=4096;buffers[i].data_size=4096;
  rc=audoutAppendAudioOutBuffer(&buffers[i]);if(R_FAILED(rc)){failure("prime",rc);nx_pcm_close();return -1;}
 }
 rc=audoutStartAudioOut();if(R_FAILED(rc)){failure("start",rc);nx_pcm_close();return -1;}
 fprintf(stderr,"AUDIO_READY audout rate=48000 channels=2 format=S16 buffers=3 frames_per_buffer=1024\n");return 0;
}
int nx_pcm_wait(void){
 if(!initialized||current)return failure("wait state",0);
 u32 count=0;AudioOutBuffer*b=NULL;Result rc=audoutWaitPlayFinish(&b,&count,10000000);
 if(rc==KERNELRESULT(TimedOut)){atomic_fetch_add(&stats[1],1);return 0;}
 if(R_FAILED(rc))return failure("wait",rc);
 if(!count)return 0;
 /* libnx's API supplies storage for one released buffer pointer per call. */
 if(count!=1||(b!=&buffers[0]&&b!=&buffers[1]&&b!=&buffers[2]))return failure("released buffer",0);
 current=b;u32 queued=0;if(R_SUCCEEDED(audoutGetAudioOutBufferCount(&queued))&&!queued)atomic_fetch_add(&stats[2],1);
 return 1;
}
int nx_pcm_submit(const float*samples,size_t count){
 if(!current||!samples||count!=2048)return failure("submit state/size",0);
 int16_t*out=current->buffer;for(size_t i=0;i<count;i++)out[i]=nx_pcm_i16(samples[i]);
 Result rc=audoutAppendAudioOutBuffer(current);current=NULL;
 if(R_FAILED(rc))return failure("append",rc);
 atomic_fetch_add(&stats[0],1024);return 0;
}
