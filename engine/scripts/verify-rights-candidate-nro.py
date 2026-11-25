"""Check distinct candidate identity, then exercise the unchanged 1.6 feature verifier in memory."""
from pathlib import Path
import argparse,struct,subprocess,tempfile,shutil,json,hashlib
ap=argparse.ArgumentParser();ap.add_argument('directory',type=Path);a=ap.parse_args()
root=Path(__file__).resolve().parents[1]
n=(a.directory/'reloaded-nx.nro').read_bytes()
assert n[0x10:0x14]==b'NRO0'
asset=struct.unpack_from('<I',n,0x18)[0]
off,size=struct.unpack_from('<QQ',n,asset+0x18)
nacp=n[asset+off:asset+off+size]
assert nacp[:0x200].split(b'\0',1)[0]==b'Reloaded-NX rights candidate'
assert nacp[0x3060:0x3070].split(b'\0',1)[0]==b'rights-test'
assert b'candidate=rights-remediation; NOT STABLE;' in n
# Adapt only identity expectations of the prior verifier; never alter the actual NRO.
source=(root/'scripts/verify-full-runner-nro.py').read_text(encoding='utf-8')
source=source.replace("assert nacp[:0x200].split(b'\\0',1)[0]==display_name.encode()", "assert nacp[:0x200].split(b'\\0',1)[0]==b'Reloaded-NX rights candidate'")
source=source.replace("assert nacp[0x3060:0x3070].split(b'\\0',1)[0]==version.encode()", "assert nacp[0x3060:0x3070].split(b'\\0',1)[0]==b'rights-test'")
import sys,contextlib,io
old=sys.argv;sys.argv=['verify-full-runner-nro.py',str(a.directory)]
buf=io.StringIO()
try:
    with contextlib.redirect_stdout(buf):exec(compile(source,'verify-full-runner-nro.py','exec'),{'__name__':'__main__'})
finally:sys.argv=old
result=json.loads(buf.getvalue());result.update(display_name='Reloaded-NX rights candidate',display_version='rights-test',stable_release=False,physical_validation=False,base_feature_contracts='1.6 markers; no runtime equivalence claim')
print(json.dumps(result,indent=2))
