"""Extract only the new installation plus user-supplied legitimate game into an empty tree."""
import argparse,hashlib,importlib.util,json,shutil,subprocess
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from zipfile import ZipFile
spec=importlib.util.spec_from_file_location('check',Path(__file__).with_name('check-installation.py'))
check=importlib.util.module_from_spec(spec);spec.loader.exec_module(check)
def extract_member(z,member,root):
 dest=(root/member.filename).resolve()
 if not dest.is_relative_to(root.resolve()):raise ValueError('Unsafe archive path')
 if member.is_dir():dest.mkdir(parents=True,exist_ok=True)
 else:
  dest.parent.mkdir(parents=True,exist_ok=True)
  with z.open(member) as src,dest.open('wb') as dst:shutil.copyfileobj(src,dst)
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--package',type=Path,required=True);ap.add_argument('--game-zip',type=Path,required=True)
 ap.add_argument('--output',type=Path,required=True);ap.add_argument('--asset-scanner',type=Path);a=ap.parse_args()
 if a.output.exists():raise FileExistsError('A completely new test root is required')
 a.output.mkdir(parents=True);sd=a.output/'sd';sd.mkdir()
 with a.game_zip.open('rb') as f:before=hashlib.file_digest(f,'sha256').hexdigest()
 with ZipFile(a.package) as z:
  for member in z.infolist():extract_member(z,member,sd)
 assert {p.name for p in (sd/'switch').iterdir()}=={'reloaded-nx'}
 base=sd/'switch/reloaded-nx';assert not any((base/'config').iterdir())
 game=base/'session/game';count=0
 with ZipFile(a.game_zip) as z:
  selected=[m for m in z.infolist() if m.filename=='Proyecto Reloaded The Last Beta 1.9.2.2 Full.exe' or m.filename.startswith('data/')]
  assert len({m.filename for m in selected})==len(selected),'Duplicate game archive paths'
  for member in selected:
   if member.filename.lower().endswith(('.sav','.savb','.savc','.nfo')):raise ValueError('A pristine game ZIP without personal saves is required')
   if member.is_dir():extract_member(z,member,game)
  files=[m for m in selected if not m.is_dir()];count=len(files)
  with ThreadPoolExecutor(max_workers=8) as pool:list(pool.map(lambda member:extract_member(z,member,game),files))
 report=check.inspect(sd,sd/'manifest.json');assert report['status']=='PASS_STATIC_INSTALLATION',report
 report.update(game_files_extracted=count,empty_root=True,historical_folders_present=False,
   physical_tests={k:'PENDING_SWITCH' for k in ['startup','resource_load_in_game','initial_config','save_load','controls','casino']})
 if a.asset_scanner:
  with (a.output/'asset-scan.txt').open('wb') as log:
   proc=subprocess.run([str(a.asset_scanner.resolve()),'unused','--scan-assets',str((game/'data').resolve())],stdout=log,stderr=subprocess.STDOUT)
  report['desktop_decoder_scan_exit_code']=proc.returncode
  report['desktop_scan_note']='Desktop decoder check only; not execution of the Switch NRO'
 with a.game_zip.open('rb') as f:after=hashlib.file_digest(f,'sha256').hexdigest()
 assert before==after;report['pristine_game_archive_unchanged']=True
 report['game_archive_sha256']=before
 (a.output/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 print(json.dumps(report,indent=2))
if __name__=='__main__':main()
