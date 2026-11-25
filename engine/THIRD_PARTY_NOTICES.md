# Candidate third-party notices — controlling update

This file's historical table below describes the original private stable
candidate and is retained for provenance. For the modified candidate,
CANDIDATE_LICENSE_SCOPE.md controls status: historical Arimo, generated gl-rs
output and all four unresolved L4 payloads are excluded. The font-derived
replacement is OFL-1.1 with its full notice; the new bindings use the bundled
MIT header and separately authored project loader. The runner is Horizon-only.
Complete dependency inventory is third_party/CARGO_DEPENDENCIES.json; original
per-file notices and the 19 recovered original supplements are retained.
Those notices do not imply that every bundled source dependency is linked.

---

# Third-party licenses and provenance

This is a multi-license source collection. Do not read the original bootstrap MIT license as permission to distribute the combined runner under MIT. Upstream notices and per-file copyright remain controlling. Legal/asset clearance is incomplete; see GITHUB_PUBLICATION_AUDIT.md.

| Component | Observed license / provenance | Redistribution obligations and status |
|---|---|---|
| Reloaded-NX independent C adapters, scripts | Existing MIT file notices; original text in `licenses/Reloaded-NX-MIT.txt` | Retain copyright, permission and disclaimer; human/AI credits do not change rights. |
| OpenGMK, gm8exe, GML parser, Klisee adaptations | GPL-2.0-only; `full-runner/UPSTREAM.json`, pinned `d0a86a8f304f89b79ee00d6ff09bcbe789c4ebb6` | Preserve GPL and notices, identify modifications, provide complete corresponding source for binary distribution. No unilateral upgrade to GPLv3. |
| Ramen | Zlib; `full-runner/vendor/ramen/LICENCE.org` | Retain notices and mark altered sources. |
| Udon / libsoundio | GPL-2.0-only / MIT; vendor notices | Corresponding source and GPL for Udon; MIT notices for libsoundio. |
| libnx | ISC; `licenses/libnx-ISC.txt` | Preserve copyright/permission/disclaimer; [upstream](https://github.com/switchbrew/libnx/blob/master/LICENSE.md). |
| newlib, GCC runtime | Mixed per-file newlib terms; GCC GPLv3 with Runtime Library Exception | Preserve `licenses/COPYING.NEWLIB`, `GCC-COPYING3`, `GCC-COPYING.RUNTIME`. Verify exception conditions and exact SDK source/notice correspondence; an installed version is not a source audit. |
| Mesa/libdrm_nouveau | Predominantly MIT/X11 and per-file terms, not a single inferred license | Pinned source archives, recipes, patches and notices in `third_party/sdk-sources`; retain individual notices, inspect component scope. No license inferred from being an SDK dependency. |
| Rust std/core, libc Horizon adapter | MIT/Apache alternatives and embedded per-file terms | Keep both original notices; choose MIT where actually offered for combination. Unicode/BSD and runtime exceptions still apply. |
| Cargo dependencies | 204 exact version declarations in `third_party/CARGO_DEPENDENCIES.json` and [matrix](DEPENDENCY_LICENSES.md) | Every vendored file checksum was checked; retain package notices. Registry manifest license declarations do not audit all embedded C/graphics material. |
| Noto Sans panel atlas | SIL OFL 1.1; original font, OFL and hashes in `switch-port/fonts` | Bundle copyright/OFL; respect reserved names and derivative-font conditions. |
| TimGM6mb.sf2 | GPL-2; Tim Brechbill (2004), David Bolton (2010) | Bank matches bundled preferred source archive; keep copyright, GPLv2 and source. [Debian copyright](https://metadata.ftp-master.debian.org/changelogs/main/t/timgm6mb-soundfont/timgm6mb-soundfont_1.3-5_copyright) distinguishes GPL-3+ Debian packaging, which is not bundled as program code. |
| `gm8emulator/data/arimo.dat` | Source comment describes Apache 2.0 Arimo exported through GM8 | BLOCKED: exact font version, original copyright/license and generated-data status not established. Current [Arimo OFL](https://github.com/google/fonts/blob/main/ofl/arimo/OFL.txt) cannot automatically relicense this historical blob. |
| `particles.dat`, upstream particle PNGs and other images | Carried from upstream source tree | Cleared for the redrawn upstream resources and source graphics specified in RESOURCE_PROVENANCE.md; third-party comparison imagery excluded. |
| gl_generator 0.14.0 / khronos_api 3.1.0 | Apache-2.0; host generator and XML | `build.rs` invokes StructGenerator and `render/opengl.rs` includes its output. Resolve generated-template/data rights and GPLv2 compatibility before approving binary; presence on the host alone neither proves nor disproves inclusion of protected output. |
| zopfli 0.8.3 / fortanix-sgx-abi 0.6.1 | Apache-2.0 / MPL-2.0 | Retain licenses for source aggregation; determine target-specific graph before any claim that these are linked into Horizon. |
| Game, characters, forwarder artwork | Rights of Pokémon Reloaded creators and respective third parties | Game, extracted scripts, custom forwarder graphics, private NSPs and console keys excluded. No redistribution permission is asserted. |

[OpenGMK](https://github.com/OpenGMK/OpenGMK) states GPL-2.0-only. The [Apache Software Foundation compatibility guidance](https://apache.org/licenses/GPL-compatibility.html) records that Apache 2.0 is not GPLv2-compatible. That rule is relevant only after establishing the rights and scope of incorporated material; this audit does not declare a proven infringement from a lockfile alone.

Build tools (PowerShell, Python, Rust toolchain, devkitPro/devkitA64, w64devkit, CMake) are external prerequisites. They are not bundled as executables. Tool licensing is separate from the license of output, except where copied runtime/template components impose their own conditions.


Actualización focalizada: L3 RESUELTO para los recursos delimitados en RESOURCE_PROVENANCE.md; L1/L2 PENDIENTES. L4: 19 avisos originales recuperados y 4 casos pendientes; ver L4_DEPENDENCY_REVIEW.md y licenses/recovered-cargo/SOURCES.json. Los suplementos se distribuyen fuera de los crates para preservar sus checksums.
