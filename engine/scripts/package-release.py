"""Complete Reloaded-NX 1.6 installation and source archives, with strict boundaries."""
import argparse,hashlib,json,struct,os,tarfile,datetime
from pathlib import Path
from zipfile import ZIP_DEFLATED,ZipFile,ZipInfo
ROOT=Path(__file__).resolve().parents[1]
DOCS=('INSTALL.md','SD_CLEANUP.md','FILE_MANIFEST.md','TEST_REPORT.md')
def sha(data):return hashlib.sha256(data).hexdigest()
def zipped(path,files,dirs=()):
 with ZipFile(path,'x') as z:
  for name in sorted(dirs):z.writestr(ZipInfo(name.rstrip('/')+'/'),b'')
  for name,data in sorted(files.items()):
   assert not name.startswith('/') and '..' not in Path(name).parts
   z.writestr(ZipInfo(name),data,compress_type=ZIP_DEFLATED)
 with ZipFile(path) as z:
  assert {n for n in z.namelist() if not n.endswith('/')}==set(files)
  assert all(z.read(k)==v for k,v in files.items())
def source_files(source,registry_archives=None,rust_archive=None):
 skip={'target','.git','__pycache__'}
 inventory=json.loads((source/'third_party/CARGO_DEPENDENCIES.json').read_text(encoding='utf-8'))
 locked={item['name']+'-'+item['version'] for item in inventory if item['locked']}
 result={}
 for base,dirs,files in os.walk(source):
  base=Path(base);rel=base.relative_to(source).as_posix()
  dirs[:]=[d for d in dirs if d not in skip and not (base==source and d in {'migration','build'})
           and not (rel=='third_party' and ((d=='cargo' and registry_archives) or (d=='rust-library' and rust_archive)))]
  for filename in files:
   p=base/filename;name=p.relative_to(source).as_posix()
   if p.is_symlink() or filename=='SOURCE_MANIFEST.json':continue
   if name.startswith('third_party/cargo/') and p.relative_to(source).parts[2] not in locked:continue
   result[name]=p.read_bytes()
 if registry_archives:
  for item in inventory:
   if not item['locked']:continue
   identity=item['name']+'-'+item['version'];p=registry_archives/(identity+'.crate')
   archive=p.read_bytes();assert sha(archive)==item['archive_sha256'],identity
   import io
   with tarfile.open(fileobj=io.BytesIO(archive),mode='r:gz') as z:
    for member in z.getmembers():
     if member.isfile():result['third_party/cargo/'+member.name]=z.extractfile(member).read()
   prefix='third_party/cargo/'+identity+'/'
   files={name[len(prefix):]:sha(data) for name,data in result.items() if name.startswith(prefix)}
   result[prefix+'.cargo-checksum.json']=json.dumps(dict(package=item['archive_sha256'],files=files),sort_keys=True).encode()
 if rust_archive:
  previous=json.loads((source/'SOURCE_MANIFEST.json').read_text(encoding='utf-8'))
  with tarfile.open(rust_archive,'r:xz') as z:
   for member in z.getmembers():
    token='/lib/rustlib/src/rust/library/'
    if not member.isfile() or token not in member.name:continue
    name='third_party/rust-library/'+member.name.split(token,1)[1]
    data=z.extractfile(member).read()
    expected=previous.get(name)
    if expected is None or sha(data)!=expected:
     data=(source/name).read_bytes()
    result[name]=data
 result['third_party/CARGO_DEPENDENCIES.json']=(json.dumps([i for i in inventory if i['locked']],indent=2)+'\n').encode()
 return result
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--build',type=Path,required=True);ap.add_argument('--source',type=Path,required=True)
 ap.add_argument('--soundfont',type=Path,required=True);ap.add_argument('--output',type=Path,required=True)
 ap.add_argument('--registry-archives',type=Path);ap.add_argument('--rust-source-archive',type=Path)
 ap.add_argument('--source-archive',type=Path,help='Reuse an already verified source archive, updating active documentation/scripts')
 ap.add_argument('--source-commit',help='Git commit identifying the active development line')
 a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=False)
 n=(a.build/'reloaded-nx.nro').read_bytes();v=json.loads((a.build/'static-validation.json').read_text(encoding='utf-8-sig'))
 assert v['display_name']=='Reloaded-NX 1.6' and v['display_version']=='1.6' and v['nro_sha256']==sha(n)
 assert v['status']=='PASS_STATIC_VALIDATION_NOT_HARDWARE'
 assert n[0x10:0x14]==b'NRO0'
 sf=a.soundfont.read_bytes();assert sf[:4]==b'RIFF' and sf[8:12]==b'sfbk'
 sf_source=ROOT/'third_party/soundfont'
 if not sf_source.is_dir():sf_source=ROOT/'migration/horizon03-soundfont'
 original=sf_source/'timgm6mb-soundfont_1.3.orig.tar.gz'
 # Verify bank against preferred-form upstream source, not a residual SD copy.
 import tarfile
 with tarfile.open(original) as archive:
  member=next(m for m in archive.getmembers() if m.isfile() and m.name.endswith('.sf2'))
  assert archive.extractfile(member).read()==sf
 if a.source_archive:
  with ZipFile(a.source_archive) as z:
   src={name:z.read(name) for name in z.namelist() if not name.endswith('/') and name!='SOURCE_MANIFEST.json'}
  for name in list(src):
   if name.startswith('scripts/') and (ROOT/name).is_file():src[name]=(ROOT/name).read_bytes()
  for name in DOCS+('README.md','ARCHITECTURE.md','BUILDING.md','CONTROLS.md','CHANGELOG.md','THIRD_PARTY_NOTICES.md'):
   src[name]=(ROOT/name).read_bytes()
 else:
  src=source_files(a.source,a.registry_archives,a.rust_source_archive)
 assert '.cargo/config.toml' in src and 'switch-port/horizon-casino.h' in src
 assert any(k.startswith('third_party/cargo/') for k in src)
 assert any(k.startswith('third_party/rust-library/') for k in src)
 forbidden=('private-inputs/','artifacts/','logs/','forwarder/','references/')
 assert not any(name.startswith(forbidden) for name in src)
 provenance=dict(product='Reloaded-NX 1.6',source_commit=a.source_commit,
  build_utc=datetime.datetime.fromtimestamp((a.build/'reloaded-nx.nro').stat().st_mtime,datetime.timezone.utc).isoformat(),
  nro_sha256=sha(n),physical_validation=False)
 src['BUILD_PROVENANCE.json']=(json.dumps(provenance,indent=2)+'\n').encode()
 index={k:sha(v) for k,v in sorted(src.items())}
 src['SOURCE_MANIFEST.json']=(json.dumps(index,indent=2)+'\n').encode()
 zipped(a.output/'Reloaded-NX-1.6-source.zip',src)
 files={'switch/reloaded-nx/reloaded-nx.nro':n,'switch/reloaded-nx/engine/TimGM6mb.sf2':sf}
 for name in DOCS+('CONTROLS.md','THIRD_PARTY_NOTICES.md'):
  files[name]=(ROOT/name).read_bytes()
 for name in ('audit-sd.py','check-installation.py'):
  files['tools/'+name]=(ROOT/'scripts'/name).read_bytes()
 # Notices travel with binaries and all source copies retain their own licenses.
 for name,data in src.items():
  p=Path(name)
  notice=p.name.upper().startswith(('LICENSE','LICENCE','COPYING','NOTICE','COPYRIGHT'))
  if name.startswith('licenses/') or (name.startswith('third_party/') and notice):
   files['switch/reloaded-nx/licenses/'+name]=data
 files['switch/reloaded-nx/licenses/TimGM6mb-copyright.txt']=(sf_source/'copyright').read_bytes()
 files['switch/reloaded-nx/licenses/Noto-OFL.txt']=(ROOT/'switch-port/fonts/OFL.txt').read_bytes()
 files['switch/reloaded-nx/licenses/RELOADED-NX-LICENSE.txt']=(ROOT/'LICENSE').read_bytes()
 assert not any(Path(name).suffix.lower() in {'.exe','.dll','.apk','.keys','.nsp','.sav','.savb','.savc'} for name in files)
 assert not any('/config/' in name or '/session/game/data/' in name or '/saves/' in name or '/user/' in name for name in files)
 manifest=dict(product='Reloaded-NX 1.6',canonical_nro='sdmc:/switch/reloaded-nx/reloaded-nx.nro',
  source_commit=a.source_commit,build_utc=provenance['build_utc'],
  source_archive='Reloaded-NX-1.6-source.zip',source_sha256=sha((a.output/'Reloaded-NX-1.6-source.zip').read_bytes()),
  game_included=False,personal_data_included=False,physical_installation_validated=False,
  files=[dict(path=k,size=len(v),sha256=sha(v)) for k,v in sorted(files.items())])
 files['manifest.json']=(json.dumps(manifest,indent=2)+'\n').encode()
 files['SHA256SUMS.txt']=''.join(f'{sha(data)}  {name}\n' for name,data in sorted(files.items())).encode()
 dirs=['switch/reloaded-nx/config','switch/reloaded-nx/logs','switch/reloaded-nx/session/game','switch/reloaded-nx/session/temp']
 zipped(a.output/'Reloaded-NX-1.6.zip',files,dirs)
 for name in DOCS:(a.output/name).write_bytes((ROOT/name).read_bytes())
 sums={p.name:sha(p.read_bytes()) for p in a.output.iterdir() if p.is_file()}
 (a.output/'SHA256SUMS.txt').write_text(''.join(f'{h}  {name}\n' for name,h in sorted(sums.items())),encoding='ascii')
 print(json.dumps(dict(status='PACKAGED_COMPLETE_INSTALLATION_PHYSICAL_TESTS_PENDING',runtime_files=len(files),source_files=len(src),sha256=sums),indent=2))
if __name__=='__main__':main()
