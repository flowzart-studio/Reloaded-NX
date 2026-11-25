"""Prepare the active source tree, dependency sources and notices without private data."""
import argparse, hashlib, json, shutil
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
SCRIPTS=['build-full-runner-horizon.ps1','build-full-runner-pc.ps1','check-full-runner-switch.ps1',
 'check-switch-abi.py','generate-horizon-abi.py','horizon-cache.ps1','rustc-full-wrapper.rs',
 'verify-full-runner-nro.py','setup-rust.ps1','fetch-horizon-mesa.py','fetch-horizon-soundfont.py',
 'test-casino.ps1','test-horizon06b.ps1','test-resource-epoch.py','check-installation.py',
 'prepare-source.py','setup-dependencies.ps1','vendor-cargo.py','audit-sd.py','package-release.py','test-clean-install.py','test-sd-audit.py']
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def copy_tree(src,dest):
 for p in src.rglob('*'):
  rel=p.relative_to(src)
  if any(s in {'target','.git','__pycache__'} for s in rel.parts):continue
  if not p.is_file() or p.is_symlink():continue
  if p.suffix.lower() in {'.exe','.dll','.nro','.nsp','.nca','.keys','.apk','.7z','.sav','.savb','.savc','.nfo'}:continue
  target=dest/rel;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,target)
def main():
 ap=argparse.ArgumentParser();ap.add_argument('output',type=Path);a=ap.parse_args()
 a.output.mkdir(parents=True,exist_ok=False)
 for directory in ('switch-port','full-runner/src','full-runner/vendor','licenses','tests/casino-input'):
  copy_tree(ROOT/directory,a.output/directory)
 for name in ('LICENSE','UPSTREAM.json','DEPENDENCIES.json'):
  shutil.copyfile(ROOT/'full-runner'/name,a.output/'full-runner'/name)
 for name in SCRIPTS:
  p=ROOT/'scripts'/name
  if p.is_file():
   target=a.output/'scripts'/name;target.parent.mkdir(exist_ok=True);shutil.copyfile(p,target)
 for name in ('README.md','BUILDING.md','ARCHITECTURE.md','CONTROLS.md','INSTALL.md','SD_CLEANUP.md','FILE_MANIFEST.md','CHANGELOG.md','THIRD_PARTY_NOTICES.md','LICENSE','.gitignore'):
  p=ROOT/name
  if p.is_file():shutil.copyfile(p,a.output/name)
 for name in ('rust-switch-toolchain.lock.json','rust-sysroot.Cargo.lock','installed-packages.lock.txt'):
  target=a.output/'tools'/name;target.parent.mkdir(exist_ok=True);shutil.copyfile(ROOT/'tools'/name,target)
 # Cargo directory sources, with upstream checksums and all per-package licenses.
 registry=ROOT/'build/cargo-home/registry/src/index.crates.io-1949cf8c6b5b557f'
 for p in registry.iterdir():
  if p.is_dir():shutil.copytree(p,a.output/'third_party/cargo'/p.name)
 (a.output/'.cargo').mkdir()
 (a.output/'.cargo/config.toml').write_text('[source.crates-io]\nreplace-with = "vendored-sources"\n[source.vendored-sources]\ndirectory = "third_party/cargo"\n',encoding='utf-8')
 copy_tree(ROOT/'tools/rust-switch-portable/lib/rustlib/src/rust/library',a.output/'third_party/rust-library')
 import subprocess,sys
 subprocess.run([sys.executable,str(ROOT/'scripts/vendor-cargo.py'),str(a.output/'third_party/cargo'),'--cache',str(ROOT/'build/cargo-home/registry/cache/index.crates.io-1949cf8c6b5b557f'),'--lock',str(ROOT/'full-runner/src/OpenGMK/Cargo.lock'),'--lock',str(ROOT/'tools/rust-switch-portable/lib/rustlib/src/rust/library/Cargo.lock'),'--lock',str(ROOT/'tests/casino-input/Cargo.lock'),'--fetch-missing'],check=True)
 copy_tree(ROOT/'migration/horizon-licenses',a.output/'third_party/sdk-sources')
 # Include preferred editable soundfont representation and upstream notice.
 sf=ROOT/'migration/horizon03-soundfont'
 for name in ('timgm6mb-soundfont_1.3.orig.tar.gz','copyright'):
  target=a.output/'third_party/soundfont'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(sf/name,target)
 index={p.relative_to(a.output).as_posix():sha(p) for p in a.output.rglob('*') if p.is_file()}
 (a.output/'SOURCE_MANIFEST.json').write_text(json.dumps(index,indent=2)+'\n',encoding='utf-8')
 print(json.dumps(dict(status='SOURCE_TREE_PREPARED',files=len(index),cargo_packages=len(list(registry.iterdir())))))
if __name__=='__main__':main()
