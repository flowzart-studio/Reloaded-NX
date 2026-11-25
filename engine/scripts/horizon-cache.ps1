# SPDX-License-Identifier: MIT
# Cargo build-std can reuse sysroot artifacts across local binding edits.
# Keep one cache per target/binding source revision, shared by all std consumers.
param([string]$BaseDirectory)
$root=Split-Path -Parent $PSScriptRoot
if(!$BaseDirectory){$BaseDirectory=Join-Path $root 'migration/target-horizon-std'}
$files=@('switch-port/targets/aarch64-nx-newlib.json','switch-port/vendor/libc-horizon/src/unix/newlib/mod.rs','switch-port/vendor/libc-horizon/src/unix/newlib/horizon/mod.rs')
$hashes=($files | ForEach-Object {(Get-FileHash (Join-Path $root $_) -Algorithm SHA256).Hash}) -join ''
$sha=[System.Security.Cryptography.SHA256]::Create()
try {$key=([BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::UTF8.GetBytes($hashes)))).Replace('-','').Substring(0,12).ToLower()} finally {$sha.Dispose()}
return $BaseDirectory+'-'+$key
