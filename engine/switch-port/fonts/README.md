# Font for the Horizon controls overlay

The font is [Noto Sans](https://github.com/google/fonts/tree/main/ofl/notosans)
from Google Fonts, licensed under SIL OFL 1.1. The original face and license
are included here. The installer distributes the license with the embedded
coverage atlas.

| Input | SHA-256 |
|---|---|
| `NotoSans.ttf` (upstream `NotoSans[wdth,wght].ttf`) | `bfb7bb691513f12e734dc346c03a03f784912432d7e3fa8e56efcf906fe86b3d` |
| `OFL.txt` (download) | `cee9892f9f0cc8fe882c9e9537ee6a89621d86ee7ceaf70b02e2b2b1c25c061a` |

The bundled license trims a trailing space from the upstream text.

Run `python scripts/bake-horizon-font.py` from the project to regenerate
`noto-atlas.inc`. The checked-in atlas was generated with Pillow 13.0.0 and
FreeType 2.14.3 at 36 px using the default face coordinates. The C renderer
uploads it once as an R8 coverage texture and uses linear sampling.
Normal builds consume the generated atlas directly.
