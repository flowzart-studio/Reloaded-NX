"""Private fixtures prove the inventory cannot silently approve deletion or change SD files."""
import importlib.util,json,tempfile
from pathlib import Path
spec=importlib.util.spec_from_file_location('audit',Path(__file__).with_name('audit-sd.py'))
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
with tempfile.TemporaryDirectory() as temp:
 root=Path(temp);sd=root/'sd';backup=root/'backup';sd.mkdir()
 files={'switch/reloaded-nx/reloaded-nx.nro':b'nro','switch/reloaded-nx-1.2-qol/reloaded-nx.nro':b'nro',
  'switch/reloaded-nx-1.2-qol/saves/unique.sav':b'important',
  'switch/reloaded-nx/user/identity.txt':b'private','switch/other-app/file':b'not ours'}
 for name,data in files.items():
  for target in [sd/name,backup/name]:target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
 r=m.inventory(sd,root/'report.json',backup)
 assert len(r['files'])==4 and not r['other_homebrew_inspected'] and not r['sd_modified']
 assert all(not row['cleanup_approved'] for row in r['roots'])
 old=next(row for row in r['roots'] if row['path'].endswith('-1.2-qol'))
 assert old['personal']==1 and old['unique_vs_canonical']==1 and old['backup_verified']
 assert all((sd/name).read_bytes()==data for name,data in files.items())
 try:m.inventory(sd,sd/'report.json');raise AssertionError('must reject writes to SD')
 except ValueError:pass
 assert not (sd/'report.json').exists()
print('PASS audit: personal/unique data protected, backups hashed, other apps untouched, SD writes rejected')
