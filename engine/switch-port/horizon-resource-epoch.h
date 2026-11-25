/* SPDX-License-Identifier: MIT. Coherent epoch for the battle sprite cache. */
#pragma once
#include <stdatomic.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>

typedef enum {
 ResourceUnknown, ResourceSprites, ResourceSave, ResourceProfile, ResourcePathCount
} ResourcePath;

/* Classify only canonical-looking absolute paths on the already verified SD
 * device. An ambiguous spelling is handled by the conservative path. */
static ResourcePath resource_path(const char *path) {
 static const char saves[]="/switch/reloaded-nx/session/game/data/partidas";
 static const char sprites[]="/switch/reloaded-nx/session/game/data/pkmnbattle";
 static const char config[]="/switch/reloaded-nx/config";
 if(!path || *path!='/')return ResourceUnknown;
 const char *part=path+1;
 for(const char *p=part;;++p){
  const char c=*p;
  if(c=='/'||!c){
   if(p==part||p[-1]=='.')return ResourceUnknown;
   if(!c)break;
   part=p+1;
  }else if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||
             (c>='0'&&c<='9')||c=='_'||c=='-'||c=='.')){
   return ResourceUnknown;
  }
 }
 if(strncmp(path,saves,sizeof saves-1)==0 && (!path[sizeof saves-1]||path[sizeof saves-1]=='/'))
  return ResourceSave;
 if(strncmp(path,sprites,sizeof sprites-1)==0 && (!path[sizeof sprites-1]||path[sizeof sprites-1]=='/'))
  return ResourceSprites;
 if(strncmp(path,config,sizeof config-1)!=0)return ResourceUnknown;
 const char *name=path+sizeof config-1;
 if(!*name)return ResourceProfile;
 if(*name++!='/')return ResourceUnknown;
 static const char *const profiles[]={"controls-1.4.json","controls-1.3.json","controls-1.1.json","controls-1.0.1.json","controls-h06d.json","controls.json"};
 for(unsigned i=0;i<sizeof profiles/sizeof profiles[0];++i){
  const size_t n=strlen(profiles[i]);
  if(strncmp(name,profiles[i],n)==0 &&
     (!name[n]||strcmp(name+n,".tmp")==0||strcmp(name+n,".bak")==0))return ResourceProfile;
 }
 return ResourceUnknown;
}

/* libnx can join a relative path to a cwd already ending in '/'. */
static void resource_collapse_slashes(char *path){
 char *out=path;
 for(const char *p=path;*p;++p)
  if(*p!='/'||out==path||out[-1]!='/')*out++=*p;
 *out=0;
}
static bool resource_disjoint(ResourcePath path){return path==ResourceSave||path==ResourceProfile;}

static atomic_flag resource_lock=ATOMIC_FLAG_INIT;
static _Atomic uint64_t resource_epoch;
static uint64_t resource_generation;
static unsigned resource_writers;
static bool resource_overflow;
static void *resource_fds[64];
static void resource_acquire(void){while(atomic_flag_test_and_set(&resource_lock)){}}
static void resource_release(void){atomic_flag_clear(&resource_lock);}
static void resource_publish(void){
 resource_generation+=2;
 atomic_store(&resource_epoch,resource_generation|(resource_writers||resource_overflow?1u:0u));
}
uint64_t nx_resource_epoch(void){return atomic_load(&resource_epoch);}
void nx_resource_invalidate(void){resource_acquire();resource_publish();resource_release();}
static void resource_mutation_begin(void){
 resource_acquire();++resource_writers;resource_publish();resource_release();
}
static void resource_mutation_end(void){
 resource_acquire();--resource_writers;resource_publish();resource_release();
}
static bool resource_is_write(int flags){return flags&(O_WRONLY|O_RDWR|O_CREAT|O_TRUNC|O_APPEND);}
static bool resource_open_begin(void *f,ResourcePath path,int flags){
 if(!resource_is_write(flags)||resource_disjoint(path))return false;
 resource_acquire();
 ++resource_writers;
 bool found=false;
 for(unsigned i=0;f&&i<64;++i)if(!resource_fds[i]){resource_fds[i]=f;found=true;break;}
 if(!found)resource_overflow=true;
 resource_publish();
 resource_release();
 return true;
}
static void resource_close(void *f){
 if(!f)return;
 resource_acquire();
 for(unsigned i=0;i<64;++i)if(resource_fds[i]==f){
  resource_fds[i]=NULL;
  --resource_writers;
  resource_publish();
  break;
 }
 resource_release();
}
