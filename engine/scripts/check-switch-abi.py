"""Compare compiler-emitted AArch64 layout facts, without executing on PC/Switch."""
from pathlib import Path
import os,json,struct,subprocess,uuid,argparse
parser=argparse.ArgumentParser()
parser.add_argument("--scope", choices=["core","network"],default="core")
args=parser.parse_args()
network=args.scope=="network"
probe="network-abi-probe" if network else "abi-probe"
crate="reloaded_nx_network_abi_probe" if network else "reloaded_nx_abi_probe"
r=Path(__file__).resolve().parents[1]
run_id=uuid.uuid4().hex[:8]
out=r/('migration/switch/abi-extract-'+run_id);out.mkdir(exist_ok=False)
archive=max((r/'migration/target-pc/aarch64-nx-newlib/release').rglob('lib'+crate+'*.rlib'),key=lambda p:p.stat().st_mtime_ns)
# Inspect archive members directly: native ar may reject long Windows paths.
def members(data):
    assert data[:8] == b'!<arch>\n'
    pos = 8
    while pos < len(data):
        header = data[pos:pos+60]
        assert len(header) == 60 and header[58:60] == b'`\n'
        size = int(header[48:58]); payload = data[pos+60:pos+60+size]
        if header[:16].startswith(b'#1/'):
            payload = payload[int(header[:16].split(b'/')[1].strip()):]
        if payload.startswith(b'\x7fELF'): yield payload
        pos += 60 + size + size % 2
def section(path,name):
    b=path if isinstance(path,bytes) else path.read_bytes()
    assert b[:4]==b'\x7fELF' and b[4:6]==b'\x02\x01'
    assert struct.unpack_from('<H',b,18)[0]==183
    start=struct.unpack_from('<Q',b,40)[0]
    width,count,strings=struct.unpack_from('<HHH',b,58)
    def header(n):return struct.unpack_from('<IIQQQQIIQQ',b,start+n*width)
    sh=header(strings);names=b[sh[4]:sh[4]+sh[5]]
    for i in range(count):
        sh=header(i);label=names[sh[0]:].split(b'\0',1)[0].decode()
        if label==name:return b[sh[4]:sh[4]+sh[5]]
    return None
c=section(r/('migration/switch/native-network-abi.o' if network else 'migration/switch/native-abi.o'),'.rodata')
rust=next(fact for member in members(archive.read_bytes()) if (fact:=section(member,'.rodata.NX_RUST_ABI')) is not None)
labels=[f['label'] for f in json.loads((r/('switch-port/'+probe+'/facts.json')).read_text())]
assert len(c)==len(rust)==8*len(labels)
rows=[dict(field=name,devkitA64=a,rust_libc_horizon=b,match=a==b) for name,a,b in zip(labels,struct.unpack('<'+str(len(labels))+'Q',c),struct.unpack('<'+str(len(labels))+'Q',rust))]
result={'archive':str(archive),'status':'PASS' if all(row['match'] for row in rows) else 'FAIL','scope':args.scope,'target':'aarch64-nx-newlib','executed_on_hardware':False,'facts':rows,'note':'ABI comparison, not full runner compilation or gameplay. Mismatches forbid using these bindings as a compatible libnx std layer.'}
report=r/('migration/switch/abi-comparison-'+run_id+'.json')
report.write_text(json.dumps(result,indent=2)+'\n')
print('REPORT',report)
print(json.dumps(result,indent=2))
raise SystemExit(result['status']!='PASS')
