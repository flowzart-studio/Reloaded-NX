"""Read-only, scoped SD inventory. No deletion, moves, game execution or SD writes."""
import argparse, hashlib, json, os
from pathlib import Path

NAMES=('reloaded-nx','reloaded-nx-1.1-diagnostic','reloaded-nx-1.1-rc1','reloaded-nx-1.2-qol',
       'reloaded-nx-1.3-rc1','switchreloaded-nx-1.2.1-safari-rc2')
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def classify(path):
 parts=Path(path).parts; tail='/'.join(parts[2:]) if len(parts)>2 else ''
 if not tail:return 'directory'
 if any(s.lower() in {'saves','user','partidas','config'} for s in parts):return 'PERSONAL_PROTECT'
 if '/data/audio/config.ini' in path:return 'PERSONAL_PROTECT'
 if parts[1]!='reloaded-nx':return 'HISTORICAL_CONTENT_REVIEW_REQUIRED'
 if tail in {'reloaded-nx.nro','engine/TimGM6mb.sf2'}:return 'REQUIRED_EXECUTOR'
 if tail=='session/game/Proyecto Reloaded The Last Beta 1.9.2.2 Full.exe' or tail.startswith('session/game/data/'):
  return 'REQUIRED_USER_GAME'
 if tail.startswith('session/temp/'):return 'GENERATED_TEMP_REVIEW'
 if tail.startswith(('logs/','diagnostics/')):return 'DIAGNOSTIC_PRIVATE'
 if tail.startswith(('data/','engine/')):return 'UNKNOWN_PROTECT'
 return 'UNKNOWN_PROTECT'
def inventory(sd_root,output,backup=None):
 sd_root=sd_root.resolve();output=output.resolve()
 if output.is_relative_to(sd_root):raise ValueError('Write the report outside the audited SD root')
 if output.exists():raise FileExistsError(output)
 entries=[];errors=[];found=[]
 for name in NAMES:
  base=sd_root/'switch'/name
  if not base.exists():continue
  found.append('switch/'+name)
  if base.is_symlink() or base.is_junction():errors.append(dict(path='switch/'+name,error='reparse point skipped'));continue
  for current,dirs,files in os.walk(base,followlinks=False):
   current=Path(current)
   keep=[]
   for d in dirs:
    child=current/d
    if child.is_symlink() or child.is_junction():errors.append(dict(path=child.relative_to(sd_root).as_posix(),error='reparse point skipped'))
    else:keep.append(d)
   dirs[:]=keep
   for filename in sorted(files):
    p=current/filename;rel=p.relative_to(sd_root).as_posix()
    try:
     if p.is_symlink():raise ValueError('link skipped')
     before=p.stat();digest=sha(p);after=p.stat()
     if (before.st_size,before.st_mtime_ns)!=(after.st_size,after.st_mtime_ns):raise ValueError('changed during inventory')
     record=dict(path=rel,size=after.st_size,sha256=digest,classification=classify(rel))
     if backup:
      other=backup/rel
      record['backup_verified']=other.is_file() and sha(other)==digest
     entries.append(record)
    except (OSError,ValueError) as error:errors.append(dict(path=rel,error=type(error).__name__+': '+str(error)))
 canonical={e['sha256'] for e in entries if e['path'].startswith('switch/reloaded-nx/')}
 summaries=[]
 for folder in found:
  rows=[e for e in entries if e['path'].startswith(folder+'/')]
  old=folder!='switch/reloaded-nx'
  summaries.append(dict(path=folder,files=len(rows),bytes=sum(e['size'] for e in rows),
   personal=sum(e['classification']=='PERSONAL_PROTECT' for e in rows),
   unique_vs_canonical=sum(e['sha256'] not in canonical for e in rows) if old else 0,
   backup_verified=bool(rows) and all(e.get('backup_verified',False) for e in rows),
   cleanup_approved=False,reason='Physical 1.6 approval, installed forwarder path review and manual file review required'))
 result=dict(status='INVENTORY_COMPLETE' if not errors else 'INVENTORY_INCOMPLETE',scope=list(NAMES),
  other_homebrew_inspected=False,sd_modified=False,physical_installation_validated=False,
  roots=summaries,files=entries,errors=errors,automatic_deletion_supported=False)
 output.parent.mkdir(parents=True,exist_ok=True)
 output.write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
 return result
if __name__=='__main__':
 ap=argparse.ArgumentParser();ap.add_argument('sd_root',type=Path);ap.add_argument('--output',required=True,type=Path)
 ap.add_argument('--backup-root',type=Path);a=ap.parse_args();r=inventory(a.sd_root,a.output,a.backup_root)
 print(json.dumps(dict(status=r['status'],folders=len(r['roots']),files=len(r['files']),errors=len(r['errors']),sd_modified=False)))
 raise SystemExit(0 if r['status']=='INVENTORY_COMPLETE' else 1)
