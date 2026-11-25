# Build Reloaded-NX 1.6.1 for Horizon

This is a Horizon-only candidate, not an unchanged stable 1.6 source export.
Use the reference external SDK/tools from BUILDING.md: Rust nightly-2026-10-30,
devkitA64 GCC 16.1.0, libnx 4.12.0-1, native GCC 16.2.0 and CMake 4.4.5.
Those historical instructions are superseded by this file where identity,
source package or gl_generator is concerned. The entire frozen working
repository is not required. Source archives include vendored Cargo/std,
pinned SDK sources/recipes and font/soundfont inputs, not tool executables.

From the extracted source root, configure external tool directories:

```powershell
$env:DEVKITPRO = 'D:/SDK/devkitPro'
$env:RELOADED_RUST_TOOLCHAIN = 'D:/SDK/rust-switch-portable'
$env:RELOADED_NATIVE_TOOLS = 'D:/SDK/w64devkit'
$env:RELOADED_CMAKE = 'D:/SDK/cmake-4.4.5-windows-x86_64'
$env:RELOADED_CARGO_HOME = "$PWD/build/cargo-home"
New-Item -ItemType Directory -Force migration
python scripts/fetch-horizon-mesa.py
$env:RELOADED_PORTLIBS = "$PWD/migration/horizon-portlibs/opt/devkitpro/portlibs/switch"
./scripts/build-full-runner-horizon.ps1 -DevkitRoot $env:DEVKITPRO -OutputDirectory "$PWD/build/output" -TargetDirectory "$PWD/build/target"
```

The output directory must be new. Cargo is offline/locked. The toolchain's
rust-src must match the shipped Rust snapshot, as in the existing independent
build evidence. Source/SHA-256 inventories bind the candidate inputs.
Do not run the old stable `package-release.py`: its identity checks and docs
target stable 1.6, not this candidate. No package command grants publication.

Normal builds consume checked-in generated resources. To regenerate only
the candidate font, install Pillow 11.3.0 into an isolated environment and run
`python scripts/generate-default-font.py` (reference FreeType 2.13.3). To
regenerate bindings, run `python scripts/generate-horizon-mit-bindings.py`.
The latter requires no gl_generator or XML registry. Both generated outputs
were checked for determinism in the recorded environment.

## Launcher identity and icon

The build sets NACP to Reloaded-NX, Flowzart Studio and 1.6.1, and embeds the
included original 256 × 256 icon. To regenerate only the icon, use Pillow
11.3.0 and run `python scripts/generate-launcher-icon.py`. Original drawing
source and rights are documented in switch-port/branding/README.md.
This is the identity built for the 1.6.1-rc.2 acceptance package.
