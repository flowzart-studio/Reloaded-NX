> Historical focused review of the PRIVATE complete candidate. Referenced local source/evidence paths are not shipped in this documentation edition. No runtime asset is cleared by inclusion of this report. See [current publication scope](../PUBLICATION_READY.md).

# L2 — herramientas de OpenGL y código emitido

Estado: **PENDIENTE**, no incompatibilidad demostrada del NRO por el mero listado de herramientas.

Las versiones exactas son gl_generator 0.14.0, revisión `ea503e8d5fb6d73c6030e6191ce738cd3bf3433e`, y khronos_api 3.1.0, revisión `f150967b1c44ae888e6676f93f639ebc82771bdc`. Sus .crate coinciden con los checksums fijados y se recuperó la licencia Apache 2.0 original del repositorio correspondiente. Los XML mantienen sus copyright/avisos propios.

Hay dos cuestiones diferentes:

1. Ejecutar un generador Apache como herramienta host no cambia por sí solo la licencia de todo programa generado. Sus ejecutables no se enlazan automáticamente en el runner. Su redistribución separada como herramienta conserva sus propios términos.
2. Aquí el resultado no se limita a nombres y valores de API: StructGenerator emite bloques literales con FnPtr, comprobación de carga y do_metaloadfn. Se observaron esos bloques en el gl_bindings.rs ya existente del build Horizon, SHA-256 `60a288248c9e492d5d02fce144a42ffc075f5ee131291121f6479ccdd6719b0e`. `render/opengl.rs` incluye el resultado y el build.rs solicita ese generador. Esto acredita la incorporación de plantillas al fuente emitido; no decide por sí solo su originalidad protegible ni qué instrucciones conserva el enlazador.

El [issue gl-rs #516](https://github.com/rust-windowing/gl-rs/issues/516) pregunta por la licencia del output; la respuesta observada es una suposición de un participante, no una excepción de licencia otorgada por los titulares. No se usa como permiso.

En [Khronos OpenGL-Registry #376](https://github.com/KhronosGroup/OpenGL-Registry/issues/376), el interlocutor explica que su opinión favorable al uso de otras licencias en outputs es informal y no representa una respuesta jurídica vinculante de Khronos. No se convierte esa opinión en autorización. El gl.xml vendorizado conserva Apache-2.0 sin la excepción de salida citada para otros registros como vk.xml. Una excepción de Vulkan o de glad no se aplica automáticamente a estas versiones de gl-rs.

La [ASF](https://apache.org/licenses/GPL-compatibility.html) documenta la incompatibilidad de Apache 2.0 con GPLv2 cuando se combina material derivado sujeto a ambas. Lo que falta cerrar aquí es si, y en qué alcance, las plantillas/definiciones emitidas requieren esos términos o una excepción. No se etiqueta como infracción demostrada a partir de suposiciones.

## Soluciones concretas

**Vía A, conservar el NRO:** obtener una confirmación/permiso de alcance suficiente para las versiones de plantillas y datos realmente incorporadas, o una determinación fundada de que el output utilizado no está sujeto a términos incompatibles. Hay que conservar copyright, licencia Apache y los demás avisos exigibles; añadir solo un texto de LICENSE no soluciona por sí mismo una incompatibilidad si existe. No se enviaron mensajes a terceros.

**Vía B, requiere aprobación previa de código y reconstrucción:** reemplazar la emisión de plantillas por bindings escritos de forma independiente bajo términos compatibles con GPLv2, manteniendo exactamente las funciones GL, tipos/anchos, convenciones de llamada, constantes y conducta de carga requeridas por el backend. Habría que cambiar build.rs/dependencias y las inclusiones, comprobar ABI y controles/renderer, reconstruir en un árbol aislado y contrastar características. No se ha escrito ni aplicado ese reemplazo. Preservaría el NRO instalado y exigiría decidir por separado cómo identificar cualquier binario resultante; no se publicaría silenciosamente como el mismo binario estable.

No se propone pasar unilateralmente OpenGMK a GPLv3: su licencia es GPL-2.0-only. Tampoco se afirma que cambiar de generador resuelva L1 u otros derechos. La propuesta completa de reconstrucción sin cambios de código está en REBUILD_VERIFICATION.md.
