"""Read-only installation check. It never launches executables or writes saves."""
import argparse, hashlib, json, struct
from pathlib import Path
def inspect(root, manifest=None):
    base=root/'switch/reloaded-nx'
    names=['reloaded-nx.nro','engine/TimGM6mb.sf2',
           'session/game/Proyecto Reloaded The Last Beta 1.9.2.2 Full.exe']
    files={name:(base/name).is_file() for name in names}
    data=(base/'session/game/data').is_dir()
    magic=False; title=None; version=None; errors=[]
    if files['reloaded-nx.nro']:
        n=(base/'reloaded-nx.nro').read_bytes()
        try:
            magic=n[0x10:0x14]==b'NRO0'
            asset=struct.unpack_from('<I',n,0x18)[0];assert n[asset:asset+4]==b'ASET'
            offset,size=struct.unpack_from('<QQ',n,asset+0x18);nacp=n[asset+offset:asset+offset+size]
            assert len(nacp)==0x4000
            title=nacp[:0x200].split(b'\0',1)[0].decode();version=nacp[0x3060:0x3070].split(b'\0',1)[0].decode()
            assert title=='Reloaded-NX 1.6' and version=='1.6'
        except (AssertionError,ValueError,struct.error):errors.append('Invalid NRO/NACP 1.6')
    if files['engine/TimGM6mb.sf2']:
        with (base/'engine/TimGM6mb.sf2').open('rb') as stream:header=stream.read(12)
        if header[:4]!=b'RIFF' or header[8:12]!=b'sfbk':errors.append('Invalid soundfont')
    if files[names[2]]:
        with (base/names[2]).open('rb') as stream:
            if stream.read(2)!=b'MZ':errors.append('Invalid game EXE header')
    for name in ('session/game/data/pkmnbattle','session/game/data/audio'):
        if not (base/name).is_dir():errors.append('Missing '+name)
    checked=0
    if manifest:
        for item in json.loads(manifest.read_text(encoding='utf-8'))['files']:
            p=(root/item['path']).resolve()
            if not p.is_relative_to(root.resolve()):raise ValueError('Unsafe manifest path')
            if not p.is_file():errors.append('Missing packaged '+item['path']);continue
            with p.open('rb') as stream:digest=hashlib.file_digest(stream,'sha256').hexdigest()
            if digest!=item['sha256']:errors.append('Hash mismatch '+item['path'])
            checked+=1
    return dict(status='PASS_STATIC_INSTALLATION' if all(files.values()) and data and magic and not errors else 'INCOMPLETE',
        files=files,data_directory=data,nro_magic=magic,
        saves_present=(base/'session/game/data/partidas').is_dir(),
        config_present=(base/'config').is_dir(),title=title,version=version,errors=errors,
        manifest_files_checked=checked,sd_modified=False,gameplay_validated=False)
if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('sd_root',type=Path);ap.add_argument('--manifest',type=Path);args=ap.parse_args()
    result=inspect(args.sd_root,args.manifest)
    print(json.dumps(result,indent=2));raise SystemExit(0 if result['status']=='PASS_STATIC_INSTALLATION' else 1)
