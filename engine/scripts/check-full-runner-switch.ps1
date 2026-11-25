# SPDX-License-Identifier: MIT
# Bounded compile/ABI checks only. Never launches the game or produces a candidate NRO.
param([ValidateSet('Core','Std','Abi','NetworkAbi')][string]$Mode='Core')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$sdkRoot=if($env:DEVKITPRO){$env:DEVKITPRO.Replace('\','/')}else{'C:/devkitPro'}
$build=Join-Path $PSScriptRoot 'build-full-runner-pc.ps1'
& python (Join-Path $PSScriptRoot 'generate-horizon-abi.py') --check
if($LASTEXITCODE -ne 0){throw 'Generated ABI probes are stale'}
if($Mode -eq 'Core'){
 & $build -Check -Target aarch64-nintendo-switch-freestanding -BuildStd 'core,alloc'
}elseif($Mode -eq 'Std'){
 & $build -Check -Target (Join-Path $root 'switch-port/targets/aarch64-nx-newlib.json') -BuildStd 'std,panic_abort' -Manifest (Join-Path $root 'switch-port/std-probe/Cargo.toml') -Package reloaded-nx-std-probe
}else{
 $network=$Mode -eq 'NetworkAbi'
 $probe=if($network){'network-abi-probe'}else{'abi-probe'}
 $obj=if($network){'native-network-abi.o'}else{'native-abi.o'}
 New-Item -ItemType Directory -Force (Join-Path $root 'migration/switch') | Out-Null
 & (Join-Path $sdkRoot 'devkitA64/bin/aarch64-none-elf-gcc.exe') -std=c11 -D__SWITCH__ ('-I'+$sdkRoot+'/libnx/include') -fPIC -mtp=soft -c (Join-Path $root ('switch-port/'+$probe+'/native.c')) -o (Join-Path $root ('migration/switch/'+$obj))
 if($LASTEXITCODE -ne 0){throw 'Native ABI compile failed'}
 & $build -Target (Join-Path $root 'switch-port/targets/aarch64-nx-newlib.json') -BuildStd 'core,alloc' -Manifest (Join-Path $root ('switch-port/'+$probe+'/Cargo.toml')) -Package ("reloaded-nx-"+$probe)
 $scope=if($network){'network'}else{'core'}
 & python (Join-Path $PSScriptRoot 'check-switch-abi.py') --scope $scope
 if($LASTEXITCODE -ne 0){throw 'ABI mismatch: do not package an NRO using these bindings'}
}
