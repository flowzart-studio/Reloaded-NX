# Outstanding rights — focused publication decision

Decision: **scenario B, documentation only**, 2026-11-20 (America/Santiago). Prior focused reviews are reused below; the general audit was not repeated. Fresh checks of primary manifests, issues and historical metadata did not supply the missing grants. Absence of sufficient evidence is not proof of infringement. No author was contacted.

## L1 — arimo.dat

The exact atlas SHA-256 is `7aaebf74636999dd16429be1f77078015dcc182a5558ccf6b29e619ff81211e1`, traced by prior blob comparison to [OpenGMK da4486c3128e](https://github.com/OpenGMK/OpenGMK/commit/da4486c3128edb83f309ddd342456e237dd2f41c), 2020-11-03. The source describes a GM8 export of Arimo at 12 px with adjusted offsets. It does not identify the original TTF/hash/version.

Two pre-existing Google Fonts snapshots were already examined: [2017 metadata](https://github.com/google/fonts/blob/5e49edbd1875f214e0decae1e24b200066780fa8/apache/arimo/METADATA.pb) (release 1.231, TTF name-table 1.23) and [2020 snapshot](https://github.com/google/fonts/tree/7252eca1fac7ed118f81c4ba5d520f6f40c22860/apache/arimo) (variable TTF 1.33). Both historical candidates declare Apache-2.0, with different notices. The fresh 2017 metadata check confirms its historical declaration; online fetching of the 2020 metadata and OpenGMK commit failed in this pass, so their detailed findings rely on preserved prior evidence. No current OFL declaration is applied retrospectively. Neither candidate is proven to be the atlas input.

**Unresolved:** exact input or sufficiently scoped confirmation covering the existing raster, transformation and notices. If Apache-derived protected material is combined with GPL-2.0-only code, the applicable combination terms also require resolution. This edition excludes the DAT, font files and all executable/source packages containing it. See [the focused provenance report](RESOURCE_PROVENANCE.md).

## L2 — generated OpenGL bindings

Exact versions: gl_generator **0.14.0**, revision `ea503e8d5fb6d73c6030e6191ce738cd3bf3433e`; khronos_api **3.1.0**, revision `f150967b1c44ae888e6676f93f639ebc82771bdc`. Original Apache-2.0 notices were recovered previously. Running a host generator alone does not license all output; here the prior review identified literal loading/FnPtr implementation templates in the emitted bindings as well as API declarations.

[gl-rs #516](https://github.com/rust-windowing/gl-rs/issues/516) remains open in the retrieved page; it provides no applicable formal output exception. [Khronos #376](https://github.com/KhronosGroup/OpenGL-Registry/issues/376) contains explicitly informal commentary, not a binding grant for our output. An exception in Vulkan or another generator is not automatically applicable. The [ASF compatibility explanation](https://apache.org/licenses/GPL-compatibility.html) concerns combining Apache-2.0 material with GPLv2; use of a build tool alone is a different question.

**Unresolved:** terms applicable to the protected portions, if any, of the actual emitted code. Retain the original notices; obtain a sufficient permission or documented determination for the exact output. This edition excludes bindings, generator payloads and combined engine/binaries. See [the focused OpenGL review](OPENGL_LICENSE_REVIEW.md).

## L4 — four historical notices

Each exact upstream Cargo.toml identifies Steven Sheldon and declares MIT. That confirms the declared license; it does not supply the missing full historical notice. The prior audit compared package checksums and complete histories. Fresh manifest checks reconfirm the versions below.

| Exact package | Primary historical manifest | Outstanding evidence |
| --- | --- | --- |
| block 0.1.6 | [47178790cfc9](https://github.com/SSheldon/rust-block/blob/47178790cfc9d4a8b092051d8b413b78bd31254a/Cargo.toml) | Applicable full MIT notice/confirmation |
| malloc_buf 0.0.6 | [a7811e5f4c6f](https://github.com/SSheldon/malloc_buf/blob/a7811e5f4c6f9685dd57ad9bfe34e9bf6b0ba4f9/Cargo.toml) | Same; later notice not presumed retroactive |
| objc-foundation 0.1.1 | [0c157a59b793](https://github.com/SSheldon/rust-objc-foundation/blob/0c157a59b7931430ed5ac7839381470b0c516d3c/Cargo.toml) | Applicable full MIT notice/confirmation |
| objc_id 0.1.1 | [6527cdf28fef](https://github.com/SSheldon/rust-objc-id/blob/6527cdf28fef2b155c67c1681f548ecfd794003c/Cargo.toml) | Same; proposed notice is not accepted |

The fresh [objc_id PR #7](https://github.com/SSheldon/rust-objc-id/pull/7) check still shows an open proposal with no reviews. The [malloc_buf notice added in 2020](https://github.com/SSheldon/malloc_buf/blob/d9a3e539642bd90e07df458d226b19cdfa606863/LICENSE) exists; the earlier review established changes between that tree and 0.0.6. It is not treated as confirmed permission for the historical package. No copyright dates or replacement notices are invented.

All four were outside the previously verified Horizon reachable graph, yet were inside the complete source ZIP. Thus this issue does not automatically establish a defect in the NRO, but it matters to redistribution of that ZIP. Nineteen other examined notice cases were resolved; those findings stand. This edition redistributes none of the packages. See [individual dependency findings](L4_DEPENDENCY_REVIEW.md) and [prior follow-up](LICENSE_RESEARCH_FOLLOWUP.md).

## Viable routes

1. Publish the present documentation edition while retaining blocked materials privately.
2. Request scoped confirmations for the atlas/output and complete notices for the exact crate revisions. Drafts are in PERMISSION_REQUESTS.md; sending requires the maintainer's decision.
3. For a later source package, evaluate excluding unused platform dependencies without claiming a complete offline build unless the resulting corresponding-source/build path is verified. No source package was altered here.
4. Consider independently licensed font/binding replacements in a future explicitly authorized functional release, with ABI/rendering/build/hardware checks. These changes cannot silently alter the stable 1.6 binary.

Existing L3 findings delimit certain OpenGMK redrawn graphics as distributable under their upstream terms. This minimal edition includes none, avoiding unnecessary asset and source obligations. Comparative GameMaker imagery, origins.7z, forwarder/game art and gameplay captures stay excluded.
