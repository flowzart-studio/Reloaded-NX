"""Preflight by default. An isolated NRO build requires explicit --execute-approved-build.

Never installs an NRO, reads an SD, or touches the stable release artifacts.
"""
import argparse,datetime,hashlib,json,os,subprocess,zipfile
from pathlib import Path

FROZEN_SOURCE='b72e149618aeb8c3b13437e130d87bf093b812d1b8b68f0e4375d10cea8baa63'
FROZEN_NRO='266dbe88fecf9a582dd141057f8e5a9fdbfc61b23435b6122a9277c7823b504f'
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def save(p,data):p.write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8')
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('--source-archive',required=True,type=Path)
 ap.add_argument('--work-directory',required=True,type=Path)
 ap.add_argument('--report',required=True,type=Path)
 ap.add_argument('--devkit-root',required=True,type=Path)
 ap.add_argument('--rust-root',required=True,type=Path)
 ap.add_argument('--native-tools',required=True,type=Path)
 ap.add_argument('--cmake-root',required=True,type=Path)
 ap.add_argument('--execute-approved-build',action='store_true')
 a=ap.parse_args();work=a.work_directory.resolve();report=a.report.resolve()
 if work.exists():raise ValueError('Use a new work directory; no existing outputs may be reused.')
 if report.exists():raise ValueError('Choose a new report filename; existing evidence is never overwritten.')
 if report.is_relative_to(work):raise ValueError('The report must be outside the new work directory.')
 report.parent.mkdir(parents=True,exist_ok=True)
 source_hash=sha(a.source_archive)
 tools={'rustc':a.rust_root/'bin/rustc.exe','cargo':a.rust_root/'bin/cargo.exe','devkitA64':a.devkit_root/'devkitA64/bin/aarch64-none-elf-gcc.exe','host_gcc':a.native_tools/'bin/gcc.exe','cmake':a.cmake_root/'bin/cmake.exe','elf2nro':a.devkit_root/'tools/bin/elf2nro.exe','nacptool':a.devkit_root/'tools/bin/nacptool.exe','libnx':a.devkit_root/'libnx/lib/libnx.a'}
 inventory={k:{'path':str(p.resolve()),'exists':p.is_file(),'sha256':sha(p) if p.is_file() else None} for k,p in tools.items()}
 result={'mode':'PREFLIGHT_ONLY','source_archive_sha256':source_hash,'source_matches_frozen_1_6':source_hash==FROZEN_SOURCE,'reference_nro_sha256':FROZEN_NRO,'work_directory':str(work),'tools':inventory,'build_attempted':False,'build_static_validation':'NOT_RUN','feature_equivalence':'NOT_ESTABLISHED','byte_for_byte_reproducibility':'NOT_TESTED','hardware_gameplay':'NOT_TESTED','sd_accessed':False}
 with zipfile.ZipFile(a.source_archive) as z:
  unsafe=[i.filename for i in z.infolist() if Path(i.filename).is_absolute() or '..' in Path(i.filename).parts or ':' in i.filename or '\\' in i.filename]
  if unsafe:raise ValueError('Unsafe source archive entries')
  result['required_source_paths_present']=all(n in z.namelist() for n in ['scripts/build-full-runner-horizon.ps1','.cargo/config.toml','SOURCE_MANIFEST.json','third_party/CARGO_DEPENDENCIES.json'])
 save(report,result)
 if not result['source_matches_frozen_1_6'] or not result['required_source_paths_present'] or not all(x['exists'] for x in inventory.values()):raise SystemExit('PREFLIGHT_FAILED; inspect the private report')
 if not a.execute_approved_build:
  print('PREFLIGHT_PASS. Build not executed; explicit approval is required.');return
 result['tool_versions']={}
 for name,args in [('rustc',['-vV']),('cargo',['--version']),('devkitA64',['--version']),('host_gcc',['--version']),('cmake',['--version'])]:
  version=subprocess.run([str(tools[name].resolve()),*args],capture_output=True,text=True,errors='replace')
  result['tool_versions'][name]={'command':[str(tools[name].resolve()),*args],'exit_code':version.returncode,'stdout':version.stdout,'stderr':version.stderr}
 result['approval']='User explicitly authorized isolated reconstruction without source changes, installation or SD writes.'
 result['phase']='VERIFY_AND_EXTRACT_FROZEN_SOURCES'
 save(report,result)
 # This branch is deliberately opt-in. It builds only in a brand-new local tree.
 work.mkdir();src=work/'source';src.mkdir()
 with zipfile.ZipFile(a.source_archive) as z:
  z.extractall(src)
  manifest=json.loads(z.read('SOURCE_MANIFEST.json'))
 for n,h in manifest.items():
  if sha(src/n)!=h:raise ValueError('Source integrity mismatch: '+n)
 result['verified_source_files']=len(manifest)
 # build-std reads rust-src from the external toolchain. Bind those inputs too.
 std_mismatch=[]
 for n,h in manifest.items():
  prefix='third_party/rust-library/'
  if n.startswith(prefix):
   p=a.rust_root/'lib/rustlib/src/rust/library'/n[len(prefix):]
   if not p.is_file() or sha(p)!=h:std_mismatch.append(n)
 result['external_rust_src_mismatches']=std_mismatch
 if std_mismatch:save(report,result);raise ValueError('External rust-src differs from the documented source snapshot')
 (src/'migration').mkdir(exist_ok=True)
 cargo_home=work/'cargo-home';cargo_home.mkdir()
 env=os.environ.copy()
 for k in ['RUSTC_WRAPPER','RUSTC_WORKSPACE_WRAPPER','RUSTFLAGS','CARGO_ENCODED_RUSTFLAGS','CARGO_TARGET_DIR','CARGO_BUILD_TARGET','CARGO_BUILD_RUSTC_WRAPPER','CARGO_BUILD_RUSTC_WORKSPACE_WRAPPER']:
  env.pop(k,None)
 env.update(DEVKITPRO=str(a.devkit_root.resolve()),RELOADED_RUST_TOOLCHAIN=str(a.rust_root.resolve()),RELOADED_NATIVE_TOOLS=str(a.native_tools.resolve()),RELOADED_CMAKE=str(a.cmake_root.resolve()),RELOADED_CARGO_HOME=str(cargo_home),RELOADED_PORTLIBS=str(src/'migration/horizon-portlibs/opt/devkitpro/portlibs/switch'))
 result.update(mode='ISOLATED_APPROVED_BUILD',build_attempted=True,started_utc=datetime.datetime.now(datetime.timezone.utc).isoformat())
 save(report,result)
 commands=[['python',str(src/'scripts/fetch-horizon-mesa.py')],['pwsh','-NoProfile','-File',str(src/'scripts/build-full-runner-horizon.ps1'),'-DevkitRoot',str(a.devkit_root.resolve()),'-OutputDirectory',str(work/'output'),'-TargetDirectory',str(work/'target')]]
 result['commands']=commands
 result['build_environment']={k:env[k] for k in ['DEVKITPRO','RELOADED_RUST_TOOLCHAIN','RELOADED_NATIVE_TOOLS','RELOADED_CMAKE','RELOADED_CARGO_HOME','RELOADED_PORTLIBS']}
 result['phase']='EXECUTE_DOCUMENTED_BUILD'
 save(report,result)
 for i,cmd in enumerate(commands):
  with (work/f'command-{i}.log').open('wb') as log:code=subprocess.run(cmd,cwd=src,env=env,stdout=log,stderr=subprocess.STDOUT).returncode
  if code:result.update(build_static_validation='FAIL',failed_command=i,exit_code=code);save(report,result);raise SystemExit(code)
 nro=work/'output/reloaded-nx.nro';validation=json.loads((work/'output/static-validation.json').read_text(encoding='utf-8-sig'))
 result.update(phase='COMPLETE',build_static_validation=validation['status'],rebuilt_nro_sha256=sha(nro),byte_for_byte_reproducibility='MATCH' if sha(nro)==FROZEN_NRO else 'DIFFERENT',feature_equivalence='SOURCE_AND_BUILD_CONFIGURATION_PRESERVED; RUNTIME_NOT_TESTED',completed_utc=datetime.datetime.now(datetime.timezone.utc).isoformat())
 save(report,result);print(json.dumps(result,indent=2))
if __name__=='__main__':main()
