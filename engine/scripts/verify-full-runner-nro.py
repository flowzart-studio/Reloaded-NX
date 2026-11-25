"""Static NRO/ELF check. This never runs or simulates the game."""
from pathlib import Path
import os
import argparse,json,struct,subprocess,hashlib
ap=argparse.ArgumentParser();ap.add_argument('directory',type=Path);args=ap.parse_args();p=args.directory
b=(p/'reloaded-nx.elf').read_bytes();n=(p/'reloaded-nx.nro').read_bytes()
assert b[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',b,18)[0]==183
assert n[0x10:0x14]==b'NRO0'
nm=str(Path(os.environ.get('DEVKITPRO','C:/devkitPro'))/'devkitA64/bin/aarch64-none-elf-nm.exe')
u=subprocess.check_output([nm,'-u',str(p/'reloaded-nx.elf')],text=True);assert not u.strip(),u
symbols=subprocess.check_output([nm,'--defined-only',str(p/'reloaded-nx.elf')],text=True)
required=['nx_full_run','nx_gl_open','nx_window_poll','eglCreateContext','eglGetProcAddress','getrandom']
version=next((v for v in ('1.6.1','1.6','1.5.0','1.4.0','1.3.0','1.2.1','1.2.0','1.1.0','1.0.1','1.0.0') if f'version={v};'.encode() in b),None)
release=version is not None
final_identity=version=='1.6.1'
v16=version in ('1.6','1.6.1')
rc15=version=='1.5.0'
casino=v16 or rc15 or (version=='1.3.0' and b'candidate=casino-rc1;' in b)
rc13=version=='1.3.0' and not casino
rc14=version=='1.4.0' or casino
safari=version in ('1.2.1','1.3.0','1.4.0','1.5.0','1.6','1.6.1')
safari_rc2=version=='1.2.1' and b'candidate=safari-rc2;' in b
qol=safari or version=='1.2.0'
sprite_io=version=='1.1.0' and b'candidate=sprite-io;' in b
controls=qol or sprite_io or (version=='1.1.0' and b'candidate=controls;' in b)
display_name='Reloaded-NX' if final_identity else 'Reloaded-NX 1.6' if v16 else 'Reloaded-NX 1.5 RC1' if rc15 else 'Reloaded-NX 1.3 Casino RC1' if casino else 'Reloaded-NX 1.4 RC1' if rc14 else 'Reloaded-NX 1.3 RC1' if rc13 else ('Reloaded-NX Safari RC2' if safari_rc2 else 'Reloaded-NX Safari RC1') if safari else 'Reloaded-NX 1.2 RC1' if qol else 'Reloaded-NX 1.1 Sprite IO' if sprite_io else 'Reloaded-NX 1.1 RC1' if controls else {'1.1.0':'Reloaded-NX 1.1 Diagnostico','1.0.1':'Reloaded-NX 1.0.1 RC1'}.get(version,'Reloaded-NX')
if qol or version in ('1.0.1','1.1.0'):
 required+=['nx_window_pointer_rect','nx_window_pointer_warp','nx_pointer_overlay_draw']
 assert b'controls-1.0.1.json' in b
 assert (b'Clic izquierdo del puntero' if controls else b'Clic izquierdo del PC') in b
if release or any(f'version=Horizon-{v:02}'.encode() in b for v in range(2, 7)):
 required+=['nx_pcm_open','nx_pcm_wait','nx_pcm_submit','nx_perf_io_init','nx_gpu_begin']
 assert b'PERF02 sec=' in b
if b'version=Horizon-04' in b:
 required+=['nx_gl_drawable_size']
 for marker in [b'VIDEO04 logical=',b'INTRO04 config=',b'IO04 op=',b'sdmc:/switch/reloaded-nx-opengmk-h04/engine/TimGM6mb.sf2']:assert marker in b,marker
for horizon in ('05','06','06b','06c','06d'):
 if f'version=Horizon-{horizon};'.encode() in b:
  required+=['nx_gl_drawable_size','nx_resource_epoch','nx_resource_invalidate']
  for marker in [b'CACHE05 index_ready',b'IO05 totals',f'sdmc:/switch/reloaded-nx-opengmk-h{horizon}/engine/TimGM6mb.sf2'.encode()]:assert marker in b,marker
if release or b'version=Horizon-06' in b:
 required+=['nx_party_overlay_draw','hidGetTouchScreenStates']
 assert b'TOUCH06 presses=' in b
if release or any(f'version=Horizon-{v};'.encode() in b for v in ('06b','06c','06d')):
 required+=['nx_control_load','nx_control_save','nx_window_panel_allow','nx_control_overlay_draw']
 assert b'CONTROLS06B' in b
 assert b'[savestate] F9 guardado' not in b and b'[savestate] F10 cargado' not in b
if release or b'version=Horizon-06c;' in b or b'version=Horizon-06d;' in b:
 required+=['nx_panel_touch']
 assert b'SHORTCUT06C vk=' in b
if release or b'version=Horizon-06d;' in b:
 required+=['nx_action_count','nx_action_for_vk']
 assert b'TOUCH06D flee=' in b and b'controls-h06d.json' in b
if release:
 required+=['nx_gl_drawable_size','nx_resource_epoch','nx_resource_invalidate']
 release_paths=['sdmc:/switch/reloaded-nx/'+path for path in (
     'engine/TimGM6mb.sf2','session/game','session/temp','config','disable-resource-cache.flag',
     ({'Reloaded-NX Safari RC2':'logs/reloaded-nx-1.2.1-safari-rc2.log',
       'Reloaded-NX 1.3 Casino RC1':'logs/reloaded-nx-1.3.0-casino-rc1.log',
       'Reloaded-NX 1.5 RC1':'logs/reloaded-nx-1.5.0-rc1.log',
       'Reloaded-NX 1.6':'logs/reloaded-nx.log',
       'Reloaded-NX':'logs/reloaded-nx.log',
       'Reloaded-NX Safari RC1':'logs/reloaded-nx-1.2.1-safari-rc1.log',
       'Reloaded-NX 1.1 Sprite IO':'logs/reloaded-nx-1.1-sprite-io.log',
       'Reloaded-NX 1.1 RC1':'logs/reloaded-nx-1.1-rc1.log',
       'Reloaded-NX 1.1 Diagnostico':'logs/reloaded-nx-1.1-diagnostic.log'}.get(
           display_name,f'logs/reloaded-nx-{version}.log')))]
 for marker in [b'CACHE05 index_ready',b'IO05 totals',*(path.encode() for path in release_paths)]:
  assert marker in b,marker
 assert b'switch/reloaded-nx-opengmk' not in b and b'switch/reloaded-nx-opengmk' not in n
 asset=struct.unpack_from('<I',n,0x18)[0]
 assert n[asset:asset+4]==b'ASET'
 offset,size=struct.unpack_from('<QQ',n,asset+0x18)
 nacp=n[asset+offset:asset+offset+size]
 assert size==0x4000 and len(nacp)==size
 assert nacp[:0x200].split(b'\0',1)[0]==display_name.encode()
 assert nacp[0x3060:0x3070].split(b'\0',1)[0]==version.encode()
 if final_identity:
  for language in range(16):
   entry=nacp[language*0x300:(language+1)*0x300]
   if any(entry):
    assert entry[:0x200].split(b'\0',1)[0]==b'Reloaded-NX'
    assert entry[0x200:].split(b'\0',1)[0]==b'Flowzart Studio'
  assert nacp[0x200:0x300].split(b'\0',1)[0]==b'Flowzart Studio'
  icon_offset,icon_size=struct.unpack_from('<QQ',n,asset+8)
  icon=n[asset+icon_offset:asset+icon_offset+icon_size]
  original=(Path(__file__).resolve().parents[1]/'switch-port/branding/reloaded-nx.jpg').read_bytes()
  assert icon==original and 0 < icon_size <= 0x20000
  assert all(marker not in n.lower() for marker in (b'rights candidate',b'rights-test',b'rights-remediation'))
if qol or version=='1.1.0':
 required+=['nx_trace_init','nx_trace_ticks','nx_trace_io','nx_trace_cpu','nx_trace_ram']
 trace_flag=b'enable-battle-trace.flag' if controls else b'trace-legacy-log-epoch.flag'
 for marker in [b'BATTLE11 loop id=',b'BATTLE11 next_swap',trace_flag]:
  assert marker in b,marker
if controls:
 required+=['nx_window_battle_context','nx_window_scene_changing']
 for marker in [b'CONTEXT11 mode=',b'CONTEXT11 pointer=',b'controls-1.1.json']:
  assert marker in b,marker
 assert b'trace-legacy-log-epoch.flag' not in b
if sprite_io or qol:
 required+=['nx_perf_resource_report','fsdevTranslatePath']
 for marker in [b'EPOCH11 attempts save=',b'SPRITEPROBE11']:
  assert marker in b,marker
if qol and not v16:
 assert (b'candidate=cumulative-rc1;' if rc15 else b'candidate=casino-rc1;' if casino else b'candidate=mega-rs-rc1;' if rc14 else b'candidate=joycon-fly-rc1;' if rc13 else b'candidate=safari-rc2;' if safari_rc2 else b'candidate=safari-rc1;' if safari else b'candidate=qol-rc1;') in b
 assert b'MEMORY12 total=' in b
if safari:
 required+=['nx_safari_overlay_draw']
 assert b'SAFARI12 touch vk=' in b
if rc13 or rc14:
 for marker in (b'controls-1.3.json', b'SL derecho', b'SR derecho'):
  assert marker in b,marker
if safari_rc2 or rc13:
 required+=['nx_window_safari_step_finished']
if rc14:
 required+=['nx_window_step_finished']
 for marker in (b'controls-1.4.json', b'TOUCH14 mega=', *(b'Stick derecho '+direction for direction in
                (b'arriba', b'abajo', b'izquierda', b'derecha'))):
  assert marker in b,marker
for name in required:assert any(line.split()[-1:]==[name] for line in symbols.splitlines()),name
if casino:
 assert any(line.split()[-1:]==['nx_window_casino'] for line in symbols.splitlines())
 for marker in (b'CASINO13 context=', b'enable-casino-trace.flag', b'key_vicio_accion3', b'key_poker_accion'):
  assert marker in b,marker
if v16:
 assert b'switch/reloaded-nx-' not in n,'Historical installation directory in final NRO'
 profile=os.environ.get('USERPROFILE')
 if profile:
  assert profile.replace('\\','/').lower().encode() not in n.replace(b'\\',b'/').lower(),'Development profile path in NRO'
for text in ['BOOT full OpenGMK Horizon','FULL_RUNNER_START','RELOADED_NX mods=false overrides=false overlays=false']:assert text.encode() in b,text
sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
result={'status':'PASS_STATIC_VALIDATION_NOT_HARDWARE','elf_machine':'AArch64','nro_magic':'NRO0','unresolved_symbols':[],'required_defined_symbols':required,'nro_sha256':sha(p/'reloaded-nx.nro'),'nro_size':len(n),'elf_sha256':sha(p/'reloaded-nx.elf'),'build_directory':str(p),'gameplay_validated':False}
if release:
 result.update(display_name=display_name,display_version=version,historical_sd_path_literals=False,
               checked_sd_paths=release_paths)
if final_identity:
 result.update(display_author='Flowzart Studio',icon_sha256=hashlib.sha256(icon).hexdigest(),physical_validation=False)
print(json.dumps(result,indent=2))
