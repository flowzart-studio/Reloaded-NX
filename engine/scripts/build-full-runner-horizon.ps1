# SPDX-License-Identifier: MIT
# Native complete OpenGMK runner, isolated from all H01-H13 builds/artifacts.
param([string]$OutputDirectory,[switch]$ReuseRustLibrary,[string]$DevkitRoot,[string]$TargetDirectory)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$sdkRoot=if($DevkitRoot){$DevkitRoot.Replace('\','/')}elseif($env:DEVKITPRO){$env:DEVKITPRO.Replace('\','/')}else{'C:/devkitPro'}
if(!(Test-Path (Join-Path $sdkRoot 'devkitA64/bin/aarch64-none-elf-gcc.exe'))){throw 'Set -DevkitRoot to the native Windows devkitPro directory (not its MSYS /opt path).'}
if(!$OutputDirectory){$OutputDirectory=Join-Path $root ('migration/full-horizon-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))}
New-Item -ItemType Directory $OutputDirectory | Out-Null
$priorDevkit=$env:DEVKITPRO
try {
 $env:DEVKITPRO=$sdkRoot
 & (Join-Path $PSScriptRoot 'check-full-runner-switch.ps1') -Mode Abi *> (Join-Path $OutputDirectory 'abi.txt')
 & (Join-Path $PSScriptRoot 'check-full-runner-switch.ps1') -Mode NetworkAbi *> (Join-Path $OutputDirectory 'network-abi.txt')
 $pl=if($env:RELOADED_PORTLIBS){$env:RELOADED_PORTLIBS}else{Join-Path $root 'migration/horizon-portlibs/opt/devkitpro/portlibs/switch'}
 if(!(Test-Path (Join-Path $pl 'lib/libEGL.a'))){throw 'Fetch pinned Mesa packages first: scripts/fetch-horizon-mesa.py'}
 if(!$ReuseRustLibrary){
  & (Join-Path $PSScriptRoot 'build-full-runner-pc.ps1') -StaticLibrary -Target (Join-Path $root 'switch-port/targets/aarch64-nx-newlib.json') -BuildStd 'std,panic_abort' -TargetDirectory $TargetDirectory *> (Join-Path $OutputDirectory 'rust.txt')
 }
 $cache=& (Join-Path $PSScriptRoot 'horizon-cache.ps1') -BaseDirectory $TargetDirectory
 $rust=Join-Path $cache 'aarch64-nx-newlib/release/libgm8emulator.a'
 if(!(Test-Path $rust)){
  $candidate=Get-ChildItem (Join-Path $cache 'aarch64-nx-newlib/release/build/gm8emulator/*/out/libgm8emulator-*.a') -File | Sort-Object LastWriteTime -Descending | Select-Object -First 1
  if(!$candidate){throw 'Full runner static library missing'}
  $rust=$candidate.FullName
 }
 Get-FileHash $rust -Algorithm SHA256 | Format-List | Out-File (Join-Path $OutputDirectory 'rust-library.txt')
 $cc=(Join-Path $sdkRoot 'devkitA64/bin/aarch64-none-elf-gcc.exe')
 $arch=@('-march=armv8-a+crc+crypto','-mtune=cortex-a57','-mtp=soft','-fPIE')
 $objects=@()
 foreach($source in @('runner-main','horizon-platform','horizon-party-overlay','horizon-panel','horizon-actions','newlib-compat','horizon-perf','horizon-audio')) {
  $sourcePath=Join-Path $root ('switch-port/'+$source+'.c')

  & $cc @arch -std=gnu11 -O2 -g -Wall -Wextra -Werror -ffunction-sections -fdata-sections -D__SWITCH__ ('-I'+$sdkRoot+'/libnx/include') ('-I'+$pl+'/include') ('-I'+(Join-Path $root 'switch-port')) -c $sourcePath -o (Join-Path $OutputDirectory ($source+'.o')) *> (Join-Path $OutputDirectory ($source+'-compile.txt'))
  if($LASTEXITCODE -ne 0){throw "C compile failed: $source"}
  $objects+=Join-Path $OutputDirectory ($source+'.o')
 }
 $elf=Join-Path $OutputDirectory 'reloaded-nx.elf'
 $link=@(('-specs='+$sdkRoot+'/libnx/switch.specs'),'-Wl,--gc-sections',('-Wl,-Map,'+(Join-Path $OutputDirectory 'reloaded-nx.map')),('-L'+$sdkRoot+'/libnx/lib'),('-L'+$pl+'/lib'))
 & $cc @arch @link @objects '-Wl,--start-group' $rust -lEGL -lglapi -ldrm_nouveau -lstdc++ -lnx -lm '-Wl,--end-group' -o $elf *> (Join-Path $OutputDirectory 'link.txt')
 if($LASTEXITCODE -ne 0){throw 'Full runner link failed; see link.txt'}
 $displayName='Reloaded-NX'
 $displayVersion='1.6.1'
 & (Join-Path $sdkRoot 'tools/bin/nacptool.exe') --create $displayName 'Flowzart Studio' $displayVersion (Join-Path $OutputDirectory 'reloaded-nx.nacp')
 if($LASTEXITCODE -ne 0){throw 'nacptool failed'}
 $icon=Join-Path $root 'switch-port/branding/reloaded-nx.jpg'
 if(!(Test-Path -LiteralPath $icon -PathType Leaf)){throw 'Original launcher icon is missing'}
 & (Join-Path $sdkRoot 'tools/bin/elf2nro.exe') $elf (Join-Path $OutputDirectory 'reloaded-nx.nro') ('--nacp='+(Join-Path $OutputDirectory 'reloaded-nx.nacp')) ('--icon='+$icon)
 if($LASTEXITCODE -ne 0){throw 'elf2nro failed'}
 & python (Join-Path $PSScriptRoot 'verify-full-runner-nro.py') $OutputDirectory | Out-File (Join-Path $OutputDirectory 'static-validation.json') -Encoding utf8
 if($LASTEXITCODE -ne 0){throw 'NRO static verification failed'}
 Get-FileHash (Join-Path $OutputDirectory 'reloaded-nx.nro') -Algorithm SHA256 | Format-List
} finally {$env:DEVKITPRO=$priorDevkit}
