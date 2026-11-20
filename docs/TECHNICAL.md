# Architecture and existing verification

This describes the private stable runner. Source paths and tools named below are not present in this public documentation edition.

The C/libnx layer creates the worker and provides NWindow/HID, focus, PCM audio and EGL/Mesa integration. The Rust OpenGMK-derived engine executes GM8 logic. Platform libraries are linked into the NRO. A custom AArch64/newlib target, rebuilt Rust std and a Horizon libc adaptation supply the platform ABI.

Input flows from libnx through a virtual-key bitset, Ramen/Input and GML events. Configurable mappings cover exploration, combat, Safari and pointer contexts; control profiles persist separately from game scripts. Audio uses engine decoders and RustySynth for MIDI. The intended installation uses a separate TimGM6mb bank. User-supplied game assets are separate from the engine.

## Existing build evidence

Previously verified reference environment: Windows/PowerShell 7, Python 3.13, devkitA64 GCC 16.1.0, libnx 4.12.0-1, Rust nightly-2026-10-30 (`1.101.0-nightly`, revision `92b5b8c87`), w64devkit GCC 16.2.0 and CMake 4.4.5. These are historical reference values, not a current installation recipe.

An independent build used fresh Cargo Home and target directories and source/dependency inputs checked against the archived source package. Compilation and static NRO validation passed. It did not use a prior runner library. The build produced different bytes and Rust code/function sizes; matching some native objects does not establish behavioral equivalence. No hardware test of the rebuilt binary or exact reproducibility is claimed.

| Private evidence item | SHA-256 |
| --- | --- |
| Archived source ZIP used for rebuild | `b72e149618aeb8c3b13437e130d87bf093b812d1b8b68f0e4375d10cea8baa63` |
| Stable NRO | `266dbe88fecf9a582dd141057f8e5a9fdbfc61b23435b6122a9277c7823b504f` |
| Prior independently rebuilt NRO | `7c2baa6ace2ca557825a8943de9352d0759134b3ea38753977c11a0766eaf701` |

These hashes identify excluded evidence; they are not downloadable assets or the integrity manifest of this edition. Public edition hashes are in ../SHA256SUMS.txt. Detailed raw logs, absolute paths, tool caches and original history remain private. No new compilation, SD access or save operation is required to publish this edition.
