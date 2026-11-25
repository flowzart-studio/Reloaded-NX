"""Host tests of the exact native mutation tracker. No game files or SD writes."""
from pathlib import Path
import os,subprocess,sys
root=Path(__file__).resolve().parents[1];out=Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=False)
header=(root/'switch-port/horizon-resource-epoch.h').as_posix()
source='#include <assert.h>\n#include <stdio.h>\n#include "'+header+'"\n'+r'''
#define GAME "/switch/reloaded-nx/session/game"
#define CONFIG "/switch/reloaded-nx/config"
int main(void){
 const ResourcePath sprite=resource_path(GAME "/data/pkmnbattle/P41_2.gmspr");
 const ResourcePath save=resource_path(GAME "/data/partidas/reloaded1_bk.savc");
 const ResourcePath profile=resource_path(CONFIG "/controls-1.1.json.tmp");
 assert(sprite==ResourceSprites && save==ResourceSave && profile==ResourceProfile);
 assert(resource_path(CONFIG "/controls-1.1.json.bak")==ResourceProfile);
 assert(resource_path(CONFIG "/controls-1.3.json")==ResourceProfile);
 assert(resource_path(CONFIG "/controls-1.3.json.tmp")==ResourceProfile);
 assert(resource_path(CONFIG "/controls-1.3.json.bak")==ResourceProfile);
 assert(resource_path(CONFIG "/controls-1.4.json")==ResourceProfile);
 assert(resource_path(CONFIG "/controls-1.4.json.tmp")==ResourceProfile);
 assert(resource_path(CONFIG "/controls-1.4.json.bak")==ResourceProfile);
 assert(resource_path(CONFIG "/controls-1.0.1.json")==ResourceProfile);
 assert(resource_path(CONFIG)==ResourceProfile);
 char relative_translation[]=GAME "//data/partidas/reloaded1.savc";
 resource_collapse_slashes(relative_translation);
 assert(resource_path(relative_translation)==ResourceSave);
 const char *unknown[]={
  "data/partidas/reloaded1.savc", GAME "/data/partidas/../pkmnbattle/P41_2.gmspr",
  GAME "/data/partidas/.. /pkmnbattle/P41_2.gmspr", GAME "/data/partidas2/save.savc",
  GAME "/data/partidas/file:stream", GAME "/data/partidas/dir\\file",
  GAME "/data/partidas/sav.", GAME "/data", CONFIG "/controls-1.1.json.tmp.other",
  "/switch/reloaded-nx/logs/reloaded-nx-1.0.1.log", "/other/data/partidas/save.savc"
 };
 for(unsigned i=0;i<sizeof unknown/sizeof unknown[0];++i)
  assert(resource_path(unknown[i])==ResourceUnknown);

 uint64_t epoch=nx_resource_epoch();
 assert(!resource_open_begin((void*)1,sprite,O_RDONLY));
 assert(!resource_open_begin((void*)2,save,O_WRONLY|O_CREAT|O_TRUNC));
 assert(!resource_open_begin((void*)3,profile,O_RDWR));
 resource_close((void*)2);resource_close((void*)3);resource_close(NULL);
 assert(nx_resource_epoch()==epoch);
 assert(resource_open_begin((void*)1,sprite,O_WRONLY|O_TRUNC));
 assert(nx_resource_epoch()&1);
 epoch=nx_resource_epoch();
 assert(!resource_open_begin((void*)2,save,O_WRONLY));
 resource_close((void*)2);assert(nx_resource_epoch()==epoch);
 assert(resource_open_begin((void*)2,ResourceUnknown,O_RDWR));
 nx_resource_invalidate();assert(nx_resource_epoch()&1);
 resource_close((void*)1);assert(nx_resource_epoch()&1);
 resource_close((void*)2);assert(!(nx_resource_epoch()&1));
 /* The production open failure path calls resource_close for a tracked slot. */
 epoch=nx_resource_epoch();
 assert(resource_open_begin((void*)1,ResourceUnknown,O_CREAT));
 resource_close((void*)1);
 assert(!(nx_resource_epoch()&1) && nx_resource_epoch()!=epoch);
 epoch=nx_resource_epoch();
 resource_mutation_begin();assert(nx_resource_epoch()&1);
 resource_mutation_end();assert(!(nx_resource_epoch()&1) && nx_resource_epoch()!=epoch);
 for(uintptr_t i=1;i<=65;++i)resource_open_begin((void*)i,ResourceUnknown,O_WRONLY);
 for(uintptr_t i=1;i<=65;++i)resource_close((void*)i);
 assert(nx_resource_epoch()&1);
 puts("PASS native tracker: save/profile disjoint, unsafe paths conservative, overlapping writers, failed opens, focus and overflow");
}
'''
(out/'test.c').write_text(source,encoding='utf-8')
subprocess.run([str(Path(os.environ.get('RELOADED_NATIVE_TOOLS',str(root/'migration/w64devkit')))/'bin/gcc.exe'),'-std=c11','-Wall','-Wextra','-Werror',str(out/'test.c'),'-o',str(out/'test.exe')],check=True)
with (out/'results.txt').open('w',encoding='utf-8') as f:subprocess.run([str(out/'test.exe')],stdout=f,stderr=subprocess.STDOUT,check=True)
print((out/'results.txt').read_text())
