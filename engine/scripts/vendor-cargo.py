"""Create Cargo directory-source checksums from the original cached registry archives."""
import argparse,hashlib,json,tarfile,tomllib,urllib.request
from pathlib import Path
def prepare(vendor,cache,lockfiles,fetch_missing=False):
 expected={}
 for lock in lockfiles:
  for package in tomllib.loads(lock.read_text(encoding='utf-8')).get('package',[]):
   if 'checksum' in package:expected[package['name']+'-'+package['version']]=package['checksum']
 if fetch_missing:
  cache.mkdir(parents=True,exist_ok=True)
  for identity,checksum in expected.items():
   if (vendor/identity).is_dir():continue
   # Package name/version are split using the registry lock entry, not guessed.
   info=next(p for lock in lockfiles for p in tomllib.loads(lock.read_text(encoding='utf-8')).get('package',[]) if p['name']+'-'+p['version']==identity)
   archive=cache/(identity+'.crate')
   if not archive.exists():
    url='https://static.crates.io/crates/'+info['name']+'/'+identity+'.crate'
    with urllib.request.urlopen(url,timeout=45) as response:data=response.read()
    assert hashlib.sha256(data).hexdigest()==checksum,identity
    archive.write_bytes(data)
   assert hashlib.sha256(archive.read_bytes()).hexdigest()==checksum,identity
   with tarfile.open(archive,'r:gz') as tar:tar.extractall(vendor,filter='data')
   print('Prepared',identity,flush=True)
 inventory=[]
 for directory in sorted(vendor.iterdir()):
  if not directory.is_dir():continue
  archive=cache/(directory.name+'.crate')
  with archive.open('rb') as stream:checksum=hashlib.file_digest(stream,'sha256').hexdigest()
  if directory.name in expected:assert checksum==expected[directory.name],directory.name
  files={}
  with tarfile.open(archive,'r:gz') as tar:
   for item in tar.getmembers():
    if not item.isfile():continue
    relative=Path(item.name).relative_to(directory.name).as_posix()
    original=tar.extractfile(item).read();actual=(directory/relative).read_bytes()
    assert actual==original,(directory.name,relative,'cached source differs from original archive')
    files[relative]=hashlib.sha256(original).hexdigest()
  (directory/'.cargo-checksum.json').write_text(json.dumps(dict(package=checksum,files=files),sort_keys=True),encoding='utf-8')
  meta=tomllib.loads((directory/'Cargo.toml').read_text(encoding='utf-8'))['package']
  inventory.append(dict(name=meta['name'],version=meta['version'],license=meta.get('license'),license_file=meta.get('license-file'),archive_sha256=checksum,locked=directory.name in expected))
 return inventory
if __name__=='__main__':
 ap=argparse.ArgumentParser();ap.add_argument('vendor',type=Path);ap.add_argument('--cache',type=Path,required=True);ap.add_argument('--lock',type=Path,action='append',required=True);ap.add_argument('--fetch-missing',action='store_true')
 a=ap.parse_args();inventory=prepare(a.vendor,a.cache,a.lock,a.fetch_missing)
 (a.vendor.parent/'CARGO_DEPENDENCIES.json').write_text(json.dumps(inventory,indent=2)+'\n',encoding='utf-8')
 print('PASS original registry archives and',len(inventory),'directory-source checksums verified')
