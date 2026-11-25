# Licensing remediation — scope and evidence

The stable 1.6 binary remains NOT CLEARED for redistribution. This candidate
does not retroactively resolve its permissions. Original L3 and the 19
recovered dependency-notice results are reused; no general audit was repeated.

| Issue | Original stable 1.6 | Implemented isolated candidate |
| --- | --- | --- |
| L1 | Exact historical Arimo input/permission remains unestablished | Historical DAT removed. 96-record raster generated from the already bundled, hash-verified Noto Sans font, OFL-1.1. Resource named Reloaded NX Default, still OFL; original font and full copyright/OFL included in source and notices shipped with binary. Glyph advances/bearings follow the consumer format; metrics differ and require physical review. |
| L2 | No sufficient output exception established for gl_generator 0.14.0/khronos_api 3.1.0 | Generator dependency/emission removed. 60 consumed entry points and 75 constants use the SDK's actual self-licensed MIT glcorearb.h. Original loader uses typed optional function pointers. Existing ABI and alias ordering checked for every consumed function. No gl-rs implementation templates/XML copied into this loader. |
| L4 | Complete historical notices for four exact releases remain unavailable | Remove non-Horizon clipboard dependency path; Cargo resolves without block 0.1.6, malloc_buf 0.0.6, objc-foundation 0.1.1, objc_id 0.1.1. Their payloads are absent from the new source archive. No stubs, invented notices, retroactive licenses or package checksum edits. |

The MIT header is taken from switch-mesa 20.1.0-5, archive SHA-256
`78328415d06bff75af07091243844ded5a63dc4d4520d2c708787b8d80ad386d`.
It contains the full 2013–2018 Khronos permission and disclaimer. Its own
license is the basis for translating these declarations, not an inferred
exception to the Apache registry XML license. The independent loader is
project MIT code; OpenGMK-derived code remains GPL-2.0-only. This is not a
claim of formal clean-room development or a retroactive clearance of old
bindings. API alias names/order are compatibility facts, not copied loaders.

The unmodified font input SHA-256 is
`bfb7bb691513f12e734dc346c03a03f784912432d7e3fa8e56efcf906fe86b3d`.
The raster uses Pillow 11.3.0/FreeType 2.13.3 at 16 px, U+0020–007F. Only
the font-derived portions use OFL; no claim is made that the runner is OFL.
Full OFL and source font are retained, including the original 2022 Noto
Project Authors notice. No font-author endorsement is implied.

The four L4 exact revisions and prior full-history work remain documented
in the already published focused reports. Their MIT manifest declarations
are not substituted for missing notices. Exclusion resolves their presence
in THIS Horizon package, not their historical notice requests. Non-Horizon
clipboard builds are not supported by this candidate. Remaining runner
dependency versions were retained; an initial lock-generation attempt that
selected newer versions was discarded before the final build.

Primary references:

- [Official OpenGL header](https://github.com/KhronosGroup/OpenGL-Registry/blob/9cb90ca4902d588bef3c830fbb1da484893bd5fb/api/GL/glcorearb.h) corroborates an explicitly MIT header distribution. The actual older SDK header and its full notice are bundled with their hashes; the 2026 notice is not applied retrospectively.
- [OFL license/FAQ](https://openfontlicense.org/ofl-faq/) explains bundling, modified font portions and retaining font licensing; it does not relicense the program.
- [gl-rs output question](https://github.com/rust-windowing/gl-rs/issues/516) and [Khronos discussion](https://github.com/KhronosGroup/OpenGL-Registry/issues/376) do not supply the missing grant for the OLD output.
- [objc_id proposed notice](https://github.com/SSheldon/rust-objc-id/pull/7) is not accepted historical permission.

No correspondence was sent. No conclusion here grants rights to the game,
characters, private forwarder art or screenshots. Those remain excluded.
Other license/provenance findings are inherited from the existing reviewed
package with original per-file notices and recovered supplements preserved.
