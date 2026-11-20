> Historical focused review of the PRIVATE complete candidate. Referenced local source/evidence paths are not shipped in this documentation edition. No runtime asset is cleared by inclusion of this report. See [current publication scope](../PUBLICATION_READY.md).

# L1/L3 — procedencia de recursos

## Arimo — PENDIENTE

El archivo existente es idéntico al introducido por Floogle en [OpenGMK `da4486c3128e`](https://github.com/OpenGMK/OpenGMK/commit/da4486c3128edb83f309ddd342456e237dd2f41c), 2020-11-03. SHA-256: `7aaebf74636999dd16429be1f77078015dcc182a5558ccf6b29e619ff81211e1`. El código original describe una exportación de Arimo mediante GM8, 12 px, caracteres 0x20–0x7f, con offsets retocados. Eso identifica el blob y su transformación, pero no el TTF exacto utilizado ni su hash/versión.

Se recuperaron dos candidatos históricos originales de Google Fonts, sin atribuir ninguno al atlas por conjetura:

| Snapshot histórico | Versión en metadatos / TTF | Licencia y titular indicados |
|---|---|---|
| [5e49edbd1875 (2017-08-07)](https://github.com/google/fonts/tree/5e49edbd1875f214e0decae1e24b200066780fa8/apache/arimo) | Publicación descrita como 1.231; el name table del TTF dice Version 1.23 | Apache-2.0; datos digitalizados 2010–2012 Google Corporation |
| [7252eca1fac7 (2020-10-07)](https://github.com/google/fonts/tree/7252eca1fac7ed118f81c4ba5d520f6f40c22860/apache/arimo) | TTF variable Version 1.33 | Apache-2.0; Copyright 2020 The Arimo Project Authors |

La existencia de ambos candidatos antes de noviembre de 2020 impide elegir uno solo por fecha. El atlas no conserva un name table de TTF. No se aplica la OFL actual retrospectivamente. Los dos LICENSE.txt y metadatos históricos se conservan como evidencia privada, no como avisos confirmados de arimo.dat.

Para redistribuir el atlas existente hace falta cerrar el origen exacto o disponer de una confirmación aplicable de los titulares sobre ese blob/transformación y la suficiencia de avisos. Si se confirma un derivado sujeto a Apache 2.0 combinado con GPL-2.0-only, habrá que resolver ese supuesto; no está demostrado únicamente por la descripción del comentario. No se regeneró ni sustituyó el atlas. No se afirma que redistribuirlo esté autorizado con la evidencia actual.

## Gráficos de OpenGMK — RESUELTO para el conjunto delimitado

Las 14 máscaras de `particles.dat` son idénticas al canal rojo/intensidad de `assets/particles/1.png` a `14.png`, en ese orden (64×64 por forma). SHA-256 del DAT: `d5e648761c224401e002e74618587aa086bf89498dc21a4fbcd9128f413823c0`. El archivo integrado en [584111d3101b](https://github.com/OpenGMK/OpenGMK/commit/584111d3101b8f978afe1dcb6758357c36c3f30e) es idéntico al actual; el DAT inicial de junio de 2020 es distinto. Por tanto no se confunde el primer recurso histórico con el redibujado que utilizamos.

Los 28 recursos compartidos del commit [712f69d63ae9, Add raw graphic assets and their sources](https://github.com/OpenGMK/OpenGMK/commit/712f69d63ae94ecb5157f06ba66bf85f207cafa5) coinciden con los blobs originales. Dmitry D. Chernov (cher-nov) figura como autor de la aportación; Floogle (skyfloogle) integró las formas iniciales. Los iconos originales de proyecto se acreditan a Jabberwock-RU conforme al README upstream. No se asigna a un committer titularidad exclusiva de todo el material.

Se acepta la licencia GPLv2 declarada por el repositorio OpenGMK para sus aportaciones originales redibujadas y se conserva su licencia, atribuciones, PNG y datos preferidos disponibles. La conclusión se apoya en la entrega expresa de recursos/fuentes, la comparación de píxeles y la ausencia de una excepción de licencia para esas aportaciones, no solo en que aparezcan en internet. No constituye certificación de todos los derechos de cualquier material ajeno que figure en una imagen.

| Recurso | Decisión de empaquetado | Avisos / límites |
|---|---|---|
| particles.dat y PNG 1–14 | Distribuibles como recursos GPL del runner, conservados sin cambiar | OpenGMK, créditos de aportación, GPLv2 y PNG fuente |
| PNG alternativos 0, 9a/9b, 11a/11b, 13a | Aportación gráfica upstream bajo la misma declaración GPL; conservada como fuente | No afirmar que todas las alternativas se cargan en Horizon |
| assets/messagebox/*.png | Fuentes gráficas redibujadas de la misma aportación; conservar bajo GPL | Conservar créditos y fuentes; no confundir con la UI original del juego |
| opengmk.ico / opengmk_sqr_crop.png | Recursos de identidad upstream, conservados con GPL y crédito original | No presentarlos como marca propia de Flowzart |
| gm8dec.ico / tas.ico | Adición upstream [66f164e77dfa](https://github.com/OpenGMK/OpenGMK/commit/66f164e77dfa317a6e224e225ff571b2d7c671fb), aportada por cher-nov | Se conservan avisos; no se afirma uso en el NRO Horizon |
| Redrawn Comparison (2020-07-09).png | Excluida del candidato público; retenida en evidencia privada | Mezcla comparación con imágenes de GameMaker 7.5. No se extiende GPL a esa parte de terceros; no la requiere ningún include/ruta de ejecución auditado |
| origins.7z | Sigue excluido del candidato | Archivo de trabajo upstream no incluido en el ZIP fuente original revisado; no se añaden contenidos sin revisar |
| Arte de forwarder, juego y personajes Pokémon | Continúan excluidos | Ningún nuevo permiso se deriva de resolver las partículas |

No se retiran partículas, fuentes, iconos utilizados ni recursos de ejecución. La única exclusión nueva es la lámina comparativa, que no participa en el motor ni su compilación. L3 queda resuelto mediante atribución y delimitación del paquete; esa lámina y otros materiales excluidos no quedan autorizados para una futura publicación aparte.
