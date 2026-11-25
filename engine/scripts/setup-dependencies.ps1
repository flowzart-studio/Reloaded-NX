# SPDX-License-Identifier: MIT. Populate sources from locked, checksum-verified archives.
param([string]$RustToolchainDirectory)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
if(!$RustToolchainDirectory){$RustToolchainDirectory=Join-Path $root 'tools/rust-switch-portable'}
if(!(Test-Path (Join-Path $RustToolchainDirectory 'bin/rustc.exe'))){
 & (Join-Path $PSScriptRoot 'setup-rust.ps1') -LockFile 'tools/rust-switch-toolchain.lock.json' -PrefixDirectory 'tools/rust-switch-portable'
 if($LASTEXITCODE){throw 'Rust toolchain setup failed'}
}
$vendor=Join-Path $root 'third_party/cargo'
New-Item -ItemType Directory -Force $vendor | Out-Null
& python (Join-Path $PSScriptRoot 'vendor-cargo.py') $vendor --cache (Join-Path $root 'tools/downloads/crates') --lock (Join-Path $root 'full-runner/src/OpenGMK/Cargo.lock') --lock (Join-Path $RustToolchainDirectory 'lib/rustlib/src/rust/library/Cargo.lock') --lock (Join-Path $root 'tests/casino-input/Cargo.lock') --fetch-missing
if($LASTEXITCODE){throw 'Locked dependency sources could not be prepared'}
$config=Join-Path $root '.cargo/config.toml'
New-Item -ItemType Directory -Force (Split-Path $config -Parent) | Out-Null
if(!(Test-Path $config)){
 @'
[source.crates-io]
replace-with = "vendored-sources"
[source.vendored-sources]
directory = "third_party/cargo"
'@ | Set-Content -LiteralPath $config -Encoding utf8
}
& python (Join-Path $PSScriptRoot 'fetch-horizon-mesa.py')
if($LASTEXITCODE){throw 'Pinned platform SDK packages could not be prepared'}
'Sources ready. Select DEVKITPRO and host tools as documented in BUILDING.md.'
