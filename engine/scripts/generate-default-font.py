"""Generate the 96-record default-font format using only bundled OFL Noto input."""
from pathlib import Path
import struct,hashlib,json,sys
try:
    from PIL import Image,ImageDraw,ImageFont,__version__
except ImportError:
    for parent in Path(__file__).resolve().parents:
        dep=parent/'tool-deps'
        if dep.is_dir():sys.path.insert(0,str(dep));break
    from PIL import Image,ImageDraw,ImageFont,__version__
ROOT=Path(__file__).resolve().parents[1]
fontpath=ROOT/'switch-port/fonts/NotoSans.ttf'
assert hashlib.sha256(fontpath.read_bytes()).hexdigest()=='bfb7bb691513f12e734dc346c03a03f784912432d7e3fa8e56efcf906fe86b3d'
font=ImageFont.truetype(str(fontpath),16)
# One shared top coordinate: the DAT has no per-glyph vertical bearing.
boxes=[font.getbbox(chr(cp),anchor='ls') for cp in range(32,127)]
top=min(b[1] for b in boxes);bottom=max(b[3] for b in boxes);height=bottom-top
result=bytearray();records=[]
for cp in range(32,128):
    if cp==127: left,right,advance=0,1,0
    else:
        left,_,right,_=font.getbbox(chr(cp),anchor='ls')
        advance=round(font.getlength(chr(cp)))
    width=max(1,right-left)
    im=Image.new('L',(width,height));d=ImageDraw.Draw(im)
    if cp!=127:d.text((-left,-top),chr(cp),font=font,anchor='ls',fill=255)
    assert -128<=advance<128 and -128<=left<128 and 0<width<256 and 0<height<256
    # Engine offset=advance, distance=horizontal bearing (not the reverse).
    result+=struct.pack('bbBB',advance,left,width,height)+im.tobytes()
    records.append({'codepoint':cp,'advance':advance,'bearing':left,'width':width,'height':height})
engine=ROOT/'full-runner/src/OpenGMK/gm8emulator'
(engine/'data/reloaded-default.dat').write_bytes(result)
metadata={'font_input_sha256':hashlib.sha256(fontpath.read_bytes()).hexdigest(),'license':'OFL-1.1','derived_resource_name':'Reloaded NX Default','pixel_size':16,'pillow':__version__,'freetype':ImageFont.core.freetype2_version,'record_format':'i8 advance, i8 horizontal bearing, u8 width, u8 height, width*height alpha bytes; U+0020..U+007F','sha256':hashlib.sha256(result).hexdigest(),'top':top,'height':height,'records':records,'stable_metric_equivalence':False}
(engine/'data/reloaded-default.json').write_text(json.dumps(metadata,indent=2)+'\n',encoding='utf-8')
print(json.dumps({k:v for k,v in metadata.items() if k!='records'}))
