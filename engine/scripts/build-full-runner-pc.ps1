# SPDX-License-Identifier: MIT
param([switch]$Check,[string]$Target='x86_64-pc-windows-gnu',[ValidateSet('','core,alloc','std,panic_abort')][string]$BuildStd='',[string]$Manifest,[string]$Package='gm8emulator',[switch]$Online,[string]$TargetDirectory,[switch]$StaticLibrary,[string]$Example)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$sdkRoot=if($env:DEVKITPRO){$env:DEVKITPRO.Replace('\','/')}else{'C:/devkitPro'}
$names=@('PATH','RUSTC','CARGO_HOME','CARGO_TARGET_DIR','CARGO_ENCODED_RUSTFLAGS','CARGO_TARGET_X86_64_PC_WINDOWS_GNU_LINKER','CC','CXX','AR','CMAKE','CMAKE_GENERATOR','RELOADED_REAL_RUSTC','RELOADED_RUSTC_LINKER','RELOADED_NATIVE_IMPORTS','RELOADED_SOURCE_ROOT')
$prior=@{};foreach($n in $names){$prior[$n]=[Environment]::GetEnvironmentVariable($n,'Process')}
$crossNames=@('CC_aarch64_nintendo_switch_freestanding','CXX_aarch64_nintendo_switch_freestanding','AR_aarch64_nintendo_switch_freestanding','CFLAGS_aarch64_nintendo_switch_freestanding','CXXFLAGS_aarch64_nintendo_switch_freestanding','CC_aarch64_nx_newlib','CXX_aarch64_nx_newlib','AR_aarch64_nx_newlib','CFLAGS_aarch64_nx_newlib','CXXFLAGS_aarch64_nx_newlib')
foreach($n in $crossNames){$prior[$n]=[Environment]::GetEnvironmentVariable($n,'Process')}
try {
 $nativeRoot=if($env:RELOADED_NATIVE_TOOLS){$env:RELOADED_NATIVE_TOOLS}else{Join-Path $root 'migration/w64devkit'}
 $native=Join-Path $nativeRoot 'bin'
 $rustRoot=if($env:RELOADED_RUST_TOOLCHAIN){$env:RELOADED_RUST_TOOLCHAIN}else{Join-Path $root 'tools/rust-switch-portable'}
 $cmakeRoot=if($env:RELOADED_CMAKE){$env:RELOADED_CMAKE}else{Join-Path $root 'migration/cmake-4.4.5-windows-x86_64'}
 $env:PATH=$native+';'+(Join-Path $cmakeRoot 'bin')+';'+$env:PATH
 $realRustc=Join-Path $rustRoot 'bin/rustc.exe'
 $env:CARGO_HOME=if($env:RELOADED_CARGO_HOME){$env:RELOADED_CARGO_HOME}else{Join-Path $root 'build/cargo-home'}
 $env:CARGO_TARGET_DIR=if($TargetDirectory){$TargetDirectory}else{Join-Path $root 'migration/target-pc'}
 if($Target.EndsWith('aarch64-nx-newlib.json') -and $BuildStd -eq 'std,panic_abort') {
  $env:CARGO_TARGET_DIR=& (Join-Path $PSScriptRoot 'horizon-cache.ps1') -BaseDirectory $TargetDirectory
 }
 $env:CC=Join-Path $native 'gcc.exe';$env:CXX=Join-Path $native 'g++.exe';$env:AR=Join-Path $native 'ar.exe'
 $env:CMAKE=Join-Path $cmakeRoot 'bin/cmake.exe'
 $env:CMAKE_GENERATOR='MinGW Makefiles'
 if($Target -ne 'x86_64-pc-windows-gnu') {
  $env:CC_aarch64_nintendo_switch_freestanding=(Join-Path $sdkRoot 'devkitA64/bin/aarch64-none-elf-gcc.exe')
  $env:CXX_aarch64_nintendo_switch_freestanding=(Join-Path $sdkRoot 'devkitA64/bin/aarch64-none-elf-g++.exe')
  $env:AR_aarch64_nintendo_switch_freestanding=(Join-Path $sdkRoot 'devkitA64/bin/aarch64-none-elf-ar.exe')
  $env:CFLAGS_aarch64_nintendo_switch_freestanding=('-fPIC -mtp=soft -D__SWITCH__ -I'+$sdkRoot+'/libnx/include')
  $env:CXXFLAGS_aarch64_nintendo_switch_freestanding=$env:CFLAGS_aarch64_nintendo_switch_freestanding
  foreach($prefix in @('CC','CXX','AR','CFLAGS','CXXFLAGS')){[Environment]::SetEnvironmentVariable(($prefix+'_aarch64_nx_newlib'),[Environment]::GetEnvironmentVariable(($prefix+'_aarch64_nintendo_switch_freestanding'),'Process'),'Process')}
 }
 $linker=Join-Path $rustRoot 'lib/rustlib/x86_64-pc-windows-gnu/bin/gcc-ld/ld.lld.exe'
 # Apply the same linker flags to HOST build scripts/proc macros and target crates.
 $wrapper=Join-Path $root 'migration/rustc-full-wrapper.exe'
 & $realRustc (Join-Path $PSScriptRoot 'rustc-full-wrapper.rs') -o $wrapper -C opt-level=1 -C linker-flavor=ld -C "linker=$linker" -C link-self-contained=yes
 if($LASTEXITCODE -ne 0){throw 'Native compiler wrapper failed'}
 $env:RELOADED_REAL_RUSTC=$realRustc;$env:RELOADED_RUSTC_LINKER=$linker;$env:RELOADED_SOURCE_ROOT=$root
 $imports=Join-Path $root 'migration/native-imports';New-Item -ItemType Directory -Force $imports | Out-Null
 Copy-Item -LiteralPath (Join-Path $nativeRoot 'lib/libavrt.a') -Destination $imports -Force
 $env:RELOADED_NATIVE_IMPORTS=$imports
 $env:RUSTC=$wrapper
 $env:CARGO_TARGET_X86_64_PC_WINDOWS_GNU_LINKER=$null
 $env:CARGO_ENCODED_RUSTFLAGS=$null
 $action=if($StaticLibrary){'rustc'}elseif($Check){'check'}else{'build'}
 $arguments=@($action)
 if(Test-Path (Join-Path $root '.cargo/config.toml')){$arguments+=@('--config',(Join-Path $root '.cargo/config.toml'))}
 if($Target.EndsWith('.json')){$arguments+=@('-Z','json-target-spec')}
 if($BuildStd){$arguments+=@('-Z',('build-std='+$BuildStd))}
 if($Target.EndsWith('aarch64-nx-newlib.json')) {
  $libc=(Join-Path $root 'switch-port/vendor/libc-horizon').Replace('\','/')
  $arguments+=@('--config',('patch.crates-io.libc.path="'+$libc+'"'))
 }
 if(!$Manifest){$Manifest=Join-Path $root 'full-runner/src/OpenGMK/Cargo.toml'}
 $arguments+=@('--manifest-path',$Manifest,'--locked','--release','--target',$Target,'-p',$Package)
 if(!$Online){$arguments+='--offline'}
 if($StaticLibrary){$arguments+=@('--crate-type','staticlib')}
 if($Example){$arguments+=@('--example',$Example)}elseif($Target -eq 'x86_64-pc-windows-gnu'){$arguments+=@('--bin','gm8emulator')}else{$arguments+='--lib'}
 & (Join-Path $rustRoot 'bin/cargo.exe') @arguments
 $code=$LASTEXITCODE
} finally {foreach($n in $prior.Keys){[Environment]::SetEnvironmentVariable($n,$prior[$n],'Process')}}
if($code -ne 0){throw "Full runner build failed: $code"}
