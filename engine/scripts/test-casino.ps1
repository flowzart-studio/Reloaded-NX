# SPDX-License-Identifier: MIT
param([string]$OutputDirectory)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$configArgs=@();if(Test-Path (Join-Path $root '.cargo/config.toml')){$configArgs=@('--config',(Join-Path $root '.cargo/config.toml'))}
$rustRoot=if($env:RELOADED_RUST_TOOLCHAIN){$env:RELOADED_RUST_TOOLCHAIN}else{Join-Path $root 'tools/rust-switch-portable'}
$nativeRoot=if($env:RELOADED_NATIVE_TOOLS){$env:RELOADED_NATIVE_TOOLS}else{Join-Path $root 'migration/w64devkit'}
$portlibs=if($env:RELOADED_PORTLIBS){$env:RELOADED_PORTLIBS}else{Join-Path $root 'migration/horizon-portlibs/opt/devkitpro/portlibs/switch'}
if(!$OutputDirectory){$OutputDirectory=Join-Path $root ('migration/casino-tests-'+[guid]::NewGuid().ToString('N'))}
New-Item -ItemType Directory $OutputDirectory | Out-Null
& (Join-Path $nativeRoot 'bin/gcc.exe') -std=c11 -Wall -Wextra -Werror (Join-Path $root 'switch-port/tests/horizon-casino.c') -o (Join-Path $OutputDirectory 'casino.exe')
if($LASTEXITCODE){throw 'Casino mapper compile failed'}
& (Join-Path $OutputDirectory 'casino.exe')
if($LASTEXITCODE){throw 'Casino mapper regression failed'}
$prior=@{}
foreach($name in @('RUSTC','CARGO_HOME','CARGO_TARGET_DIR','CARGO_ENCODED_RUSTFLAGS','CC','CXX','AR','PATH')){$prior[$name]=[Environment]::GetEnvironmentVariable($name,'Process')}
try {
 $env:PATH=(Join-Path $nativeRoot 'bin')+';'+$env:PATH
 $env:RUSTC=Join-Path $rustRoot 'bin/rustc.exe'
 $env:CARGO_HOME=if($env:RELOADED_CARGO_HOME){$env:RELOADED_CARGO_HOME}else{Join-Path $root 'build/cargo-home'}
 $env:CARGO_TARGET_DIR=Join-Path $OutputDirectory 'target'
 $env:CC=Join-Path $nativeRoot 'bin/gcc.exe'
 $env:CXX=Join-Path $nativeRoot 'bin/g++.exe'
 $env:AR=Join-Path $nativeRoot 'bin/ar.exe'
 $linker=Join-Path $rustRoot 'lib/rustlib/x86_64-pc-windows-gnu/bin/gcc-ld/ld.lld.exe'
 $env:CARGO_ENCODED_RUSTFLAGS=@('-C','linker-flavor=ld','-C',("linker="+$linker),'-C','link-self-contained=yes') -join [char]31
 & (Join-Path $rustRoot 'bin/cargo.exe') test @configArgs --offline --manifest-path (Join-Path $root 'tests/casino-input/Cargo.toml') --lib
 if($LASTEXITCODE){throw 'Casino Input contracts failed'}
} finally {foreach($name in $prior.Keys){[Environment]::SetEnvironmentVariable($name,$prior[$name],'Process')}}
'PASS Casino mapper and production Input host contracts; physical gameplay pending.'
