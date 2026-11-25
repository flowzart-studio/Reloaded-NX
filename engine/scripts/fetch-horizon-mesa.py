"""Fetch pinned official devkitPro packages into workspace only; verify repository SHA256."""
from pathlib import Path
import os,tarfile,urllib.request,hashlib,json,subprocess
r=Path(__file__).resolve().parents[1];out=r/'migration/horizon-portlibs';out.mkdir(exist_ok=True)
records=json.loads((r/'switch-port/mesa-packages.json').read_text())
for d in records:
 filename=d['url'].rsplit('/',1)[1];p=out/filename
 if not p.exists():
  request=urllib.request.Request(d['url'],headers={'User-Agent':'pacman/7.0.0 (Windows)'})
  with urllib.request.urlopen(request,timeout=60) as response:p.write_bytes(response.read())
 digest=hashlib.sha256(p.read_bytes()).hexdigest();assert digest==d['sha256']
 # Windows bsdtar understands zstd packages without modifying the installed toolchain.
 subprocess.run([str(Path(os.environ['SystemRoot'])/'System32/tar.exe'),'-xf',str(p.resolve()),'-C',str(out.resolve())],check=True)

(out/'packages.json').write_text(json.dumps(records,indent=2)+'\n');print(json.dumps(records,indent=2))
