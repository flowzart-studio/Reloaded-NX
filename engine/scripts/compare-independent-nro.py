"""Compare an isolated build with the frozen NRO, without executing either binary."""
import argparse,hashlib,json,struct,subprocess,zipfile
from pathlib import Path
def sha(b):return hashlib.sha256(b).hexdigest()
def elf_sections(data):
 assert data[:6]==b'\x7fELF\x02\x01'
 off=struct.unpack_from('<Q',data,0x28)[0];size,count,names_index=struct.unpack_from('<HHH',data,0x3a)
 entries=[struct.unpack_from('<IIQQQQIIQQ',data,off+i*size) for i in range(count)]
 names_entry=entries[names_index];names=data[names_entry[4]:names_entry[4]+names_entry[5]];result={}
 for e in entries:
  name=names[e[0]:].split(b'\0',1)[0].decode(errors='replace')
  if not name:continue
  payload=b'' if e[1]==8 else data[e[4]:e[4]+e[5]]
  result[name]={'size':e[5],'address':e[3],'flags':e[2],'sha256':sha(payload)}
 return result
def nro_parts(data):
 assert data[0x10:0x14]==b'NRO0'
 length=struct.unpack_from('<I',data,0x18)[0]
 result={'file_size':len(data),'module_size':length,'header_sha256':sha(data[:0x80]),'build_id':data[0x40:0x60].hex(),'bss_size':struct.unpack_from('<I',data,0x38)[0]}
 for i,name in enumerate(['text','rodata','data']):
  off,size=struct.unpack_from('<II',data,0x20+i*8);result[name]={'offset':off,'size':size,'sha256':sha(data[off:off+size])}
 if data[length:length+4]==b'ASET':
  for off,name in [(8,'icon'),(0x18,'nacp'),(0x28,'romfs')]:
   pos,size=struct.unpack_from('<QQ',data,length+off);b=data[length+pos:length+pos+size];result[name]={'size':size,'sha256':sha(b)}
 return result
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--stable-archive',type=Path,required=True);ap.add_argument('--reference-build',type=Path,required=True);ap.add_argument('--new-build',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 with zipfile.ZipFile(a.stable_archive) as z:old=z.read('switch/reloaded-nx/reloaded-nx.nro')
 new=(a.new_build/'reloaded-nx.nro').read_bytes();result={'stable_sha256':sha(old),'rebuilt_sha256':sha(new),'byte_identical':old==new,'stable_nro':nro_parts(old),'rebuilt_nro':nro_parts(new)}
 if len(old)==len(new):
  count=0;first=None;last=None
  for i,(x,y) in enumerate(zip(old,new)):
   if x!=y:
    count+=1;last=i
    if first is None:first=i
  result.update(differing_bytes=count,first_difference=first,last_difference=last)
 reference_nro=a.reference_build/'reloaded-nx.nro'
 result['reference_elf_bound_to_stable_nro']=reference_nro.is_file() and reference_nro.read_bytes()==old
 if result['reference_elf_bound_to_stable_nro']:
  before=elf_sections((a.reference_build/'reloaded-nx.elf').read_bytes());after=elf_sections((a.new_build/'reloaded-nx.elf').read_bytes())
  result['elf_sections']={n:{'before':before.get(n),'after':after.get(n),'equal':before.get(n)==after.get(n)} for n in sorted(before.keys()|after.keys())}
  changed=[n for n,v in result['elf_sections'].items() if not v['equal']];result['changed_elf_sections']=changed
 result['classification']='BYTE_IDENTICAL' if result['byte_identical'] else 'BUILD_OUTPUT_DIFFERENT; STATIC_AND_RUNTIME_EQUIVALENCE_REQUIRE_SEPARATE_EVIDENCE'
 a.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print(json.dumps({k:v for k,v in result.items() if k not in ('elf_sections','stable_nro','rebuilt_nro')},indent=2))
if __name__=='__main__':main()
