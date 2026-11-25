import urllib.request,hashlib,tarfile,json,io
from pathlib import Path
root=Path(__file__).resolve().parents[1];p=root/'migration/horizon03-soundfont';p.mkdir(exist_ok=True)
base='https://deb.debian.org/debian/pool/main/t/timgm6mb-soundfont/'
def get(name):
 b=urllib.request.urlopen(base+name,timeout=45).read();(p/name).write_bytes(b);return b
dsc_bytes=get('timgm6mb-soundfont_1.3-5.dsc');assert hashlib.sha256(dsc_bytes).hexdigest()=='e5df6847285db192536358460d70d62963b58a278832b2d17f67b62293351e30';dsc=dsc_bytes.decode()
entries=[]
for line in dsc.split('Checksums-Sha256:\n',1)[1].split('\n'):
 if not line.startswith(' '):break
 sha,size,name=line.split(); b=get(name);assert len(b)==int(size) and hashlib.sha256(b).hexdigest()==sha
 entries.append(dict(name=name,sha256=sha,size=int(size),url=base+name))
 if name.endswith('.orig.tar.gz'):
  with tarfile.open(fileobj=io.BytesIO(b)) as t:
   for m in t.getmembers():
    if m.isfile() and m.name.endswith('.sf2'):
     sf=t.extractfile(m).read();(p/'TimGM6mb.sf2').write_bytes(sf);print('SF2',m.name,len(sf),hashlib.sha256(sf).hexdigest())
 if name.endswith('.debian.tar.xz'):
  with tarfile.open(fileobj=io.BytesIO(b)) as t:
   (p/'copyright').write_bytes(t.extractfile('debian/copyright').read())
(root/'switch-port/soundfont-packages.json').write_text(json.dumps({'package':'timgm6mb-soundfont 1.3-5','source':'Debian','files':entries},indent=2)+'\n')
