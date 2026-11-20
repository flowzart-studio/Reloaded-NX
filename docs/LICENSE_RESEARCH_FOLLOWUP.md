> Historical focused review of the PRIVATE complete candidate. Referenced local source/evidence paths are not shipped in this documentation edition. No runtime asset is cleared by inclusion of this report. See [current publication scope](../PUBLICATION_READY.md).

# Seguimiento documental durante la reconstrucción autorizada

No se repitió la auditoría general ni se contactó a terceros. Se consultaron los historiales completos publicados de los cuatro paquetes pendientes y el historial de licencias de gl-rs. Las copias Git de investigación y sus metadatos quedan en evidencia privada; no se incorporan al repositorio público.

| Caso | Evidencia adicional | Resultado |
|---|---|---|
| block 0.1.6 | Historial completo de SSheldon/rust-block hasta `642ea4a4a5853a21b55b05c34832a5f1bb1af61c`: sin archivos LICENSE/LICENCE/COPYING/NOTICE/COPYRIGHT. La declaración MIT del paquete permanece. | PENDIENTE de aviso o aclaración suficiente del titular; no incompatibilidad demostrada. |
| objc-foundation 0.1.1 | Historial completo de SSheldon/rust-objc-foundation hasta `15f9ebec1190889e48a4bc2d36601e61d303071f`: sin esos archivos de aviso. | PENDIENTE por el mismo motivo. |
| objc_id 0.1.1 | Historial hasta `6527cdf28fef2b155c67c1681f548ecfd794003c` sin aviso completo. La [propuesta #7](https://github.com/SSheldon/rust-objc-id/pull/7) para añadir una licencia sigue abierta; su autor indica que el texto procede de Rust. | Una propuesta no aceptada ni confirmada por el titular no resuelve el aviso de nuestra versión. Sigue PENDIENTE. |
| malloc_buf 0.0.6 | El commit oficial [d9a3e539642b](https://github.com/SSheldon/malloc_buf/commit/d9a3e539642bd90e07df458d226b19cdfa606863) añade LICENSE en 2020, pero el árbol ya declara versión 1.0.0 y tiene cambios de código frente al commit de 0.0.6. | No hay identidad completa del código/versiones que justifique sustituir automáticamente el aviso histórico. PENDIENTE; el texto posterior se conserva como investigación, no como un permiso confirmado para 0.0.6. |
| gl_generator / khronos_api | [79ecda7d65ec](https://github.com/rust-windowing/gl-rs/commit/79ecda7d65ecc44d492e5c7667617b423db6f767), de 2025, añade enlaces LICENSE hacia la licencia Apache existente para corregir el empaquetado de crates. | Corrobora la recuperación documental de sus avisos L4; no añade una excepción para outputs ni resuelve L2. |
| Antecedente de salida del generador | [5d487de216fa](https://github.com/rust-windowing/gl-rs/commit/5d487de216fa19b95064d4240a4d023f7c5a76aa), de 2013, incorporaba un encabezado Apache al código generado de aquella versión. | Antecedente relevante, sin trasladarlo automáticamente a las plantillas de 0.14.0 ni convertirlo en una conclusión jurídica sobre el NRO. No se encontró una excepción documentada aplicable a nuestras versiones. |

L1 conserva el vínculo confirmado entre nuestro DAT y el commit original de OpenGMK, pero sigue sin prueba que identifique el TTF/versión exactos que produjo ese raster. Una reconstrucción del programa vuelve a incorporar el mismo DAT y no produce por sí misma la evidencia legal que falta.

Los cuatro casos L4 siguen fuera del grafo de dependencias alcanzables del target Horizon, según la comprobación previa. Eso no autoriza redistribuirlos en el ZIP completo que sí contiene sus fuentes. Los otros 19 casos L4 conservan sus avisos originales recuperados.

## Cierres concretos posibles sin cambiar el motor

- **Arimo:** confirmación aplicable al SHA-256 `7aaebf74636999dd16429be1f77078015dcc182a5558ccf6b29e619ff81211e1`, su fuente/versión y transformación, los avisos exigidos y las condiciones de distribución del atlas existente junto al runner GPL-2.0-only.
- **OpenGL:** aclaración o permiso suficiente para las plantillas efectivamente emitidas por gl_generator 0.14.0 y los datos de khronos_api 3.1.0, distinguiendo ejecución de herramientas, declaraciones de API y bloques de implementación. Una opinión informal o una excepción de otro proyecto no basta para afirmar ese cierre.
- **Cuatro paquetes restantes:** avisos completos o confirmación del alcance de la concesión MIT para las versiones exactas. Mantener originales, autores y declaraciones existentes; no inventar años, titulares ni textos sustitutos.

No se enviaron estas solicitudes. Tampoco se reemplazaron bindings, fuentes o dependencias para forzar un resultado. Cualquier alternativa de código queda fuera de la reconstrucción sin cambios autorizada en este encargo.
