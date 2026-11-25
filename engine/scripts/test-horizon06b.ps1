# SPDX-License-Identifier: MIT
# Host contracts. The production OpenGL shaders are exercised through a WGL context.
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$configArgs=@();if(Test-Path (Join-Path $root '.cargo/config.toml')){$configArgs=@('--config',(Join-Path $root '.cargo/config.toml'))}
$rustRoot=if($env:RELOADED_RUST_TOOLCHAIN){$env:RELOADED_RUST_TOOLCHAIN}else{Join-Path $root 'tools/rust-switch-portable'}
$nativeRoot=if($env:RELOADED_NATIVE_TOOLS){$env:RELOADED_NATIVE_TOOLS}else{Join-Path $root 'migration/w64devkit'}
$portlibs=if($env:RELOADED_PORTLIBS){$env:RELOADED_PORTLIBS}else{Join-Path $root 'migration/horizon-portlibs/opt/devkitpro/portlibs/switch'}
$out=Join-Path $root ('migration/h06d-tests-'+[Guid]::NewGuid().ToString('N').Substring(0,8))
New-Item -ItemType Directory $out | Out-Null
$cc=Join-Path $nativeRoot 'bin/gcc.exe'
$mesa=Join-Path $portlibs 'include'
$prior=@{}
foreach($name in @('RUSTC','CARGO_HOME','CARGO_ENCODED_RUSTFLAGS','TEMP','TMP')){
    $prior[$name]=[Environment]::GetEnvironmentVariable($name,'Process')
}
Push-Location $root
try {
    & $cc -std=c11 -Wall -Wextra -Werror switch-port/tests/horizon-controls.c switch-port/horizon-panel.c switch-port/horizon-actions.c -o "$out/controls.exe"
    if($LASTEXITCODE){throw 'Control test compile failed'}
    & "$out/controls.exe"
    if($LASTEXITCODE){throw 'Control test failed'}
    & $cc -std=c11 -Wall -Wextra -Werror switch-port/tests/horizon-pointer.c -o "$out/pointer.exe"
    if($LASTEXITCODE){throw 'PC pointer test compile failed'}
    & "$out/pointer.exe"
    if($LASTEXITCODE){throw 'PC pointer test failed'}
    & $cc -std=c11 -Wall -Wextra -Werror -DKHRONOS_STATIC "-I$mesa" switch-port/tests/horizon-overlay-wgl.c switch-port/horizon-party-overlay.c switch-port/horizon-panel.c switch-port/horizon-actions.c -lopengl32 -lgdi32 -o "$out/overlay.exe"
    if($LASTEXITCODE){throw 'OpenGL test compile failed'}
    & "$out/overlay.exe" "$out/panel-native.ppm"
    if($LASTEXITCODE){throw 'OpenGL test failed'}

    # Test the same production module without linking the game or accessing an SD mount.
    Copy-Item -LiteralPath 'full-runner/src/OpenGMK/gm8emulator/src/horizon_control_store.rs' -Destination "$out/store.rs"
    @'
[package]
name = "h06b-store-tests"
version = "0.1.0"
edition = "2021"
[lib]
path = "store.rs"
[dependencies]
serde = { version = "1", features = ["derive"] }
serde_json = "1"
'@ | Set-Content "$out/Cargo.toml"
    $env:RUSTC=Join-Path $rustRoot 'bin/rustc.exe'
    $env:CARGO_HOME=if($env:RELOADED_CARGO_HOME){$env:RELOADED_CARGO_HOME}else{Join-Path $root 'build/cargo-home'}
    $linker=Join-Path $rustRoot 'lib/rustlib/x86_64-pc-windows-gnu/bin/gcc-ld/ld.lld.exe'
    $env:CARGO_ENCODED_RUSTFLAGS=@('-C','linker-flavor=ld','-C',("linker="+$linker),'-C','link-self-contained=yes') -join [char]31
    $env:TEMP=$out
    $env:TMP=$out
    & (Join-Path $rustRoot 'bin/cargo.exe') test @configArgs --offline --manifest-path "$out/Cargo.toml" --lib
    if($LASTEXITCODE){throw 'Controls persistence test failed'}
    & $env:RUSTC --edition=2021 -C linker-flavor=ld -C "linker=$linker" -C link-self-contained=yes --test full-runner/src/OpenGMK/gm8emulator/src/render/horizon_geometry.rs -o "$out/pointer-geometry.exe"
    if($LASTEXITCODE){throw 'PC geometry test compile failed'}
    & "$out/pointer-geometry.exe"
    if($LASTEXITCODE){throw 'PC geometry test failed'}
} finally {
    Pop-Location
    foreach($name in $prior.Keys){[Environment]::SetEnvironmentVariable($name,$prior[$name],'Process')}
}
