# Reloaded-NX 1.6

Ejecutor homebrew nativo para Nintendo Switch basado en OpenGMK, desarrollado para una copia compatible de Pokémon Reloaded obtenida legítimamente y por separado.

**Edición pública documental — escenario B. Este repositorio no contiene fuentes del motor, instalador, NRO ni archivos del juego. No puede compilarse ni instalarse.** El nombre 1.6 corresponde al ejecutor estable existente; esta publicación no introduce otra versión funcional.

[English](README.md) · [Créditos](CREDITS.md) · [Preparación de publicación](PUBLICATION_READY.md) · [Licencias pendientes](docs/LEGAL_STATUS.md)

**Pedro Soto Almonacid — Flowzart Studio**

Project Lead & Maintainer

**OpenAI ChatGPT & Codex**

AI-assisted development

## Estado del proyecto

El mantenedor confirma que 1.6 funciona en Nintendo Switch. Una reconstrucción independiente previa pasó la compilación y las comprobaciones estáticas, pero difiere en bytes del NRO estable. No se han acreditado la reproducibilidad exacta ni la equivalencia física de ese binario reconstruido. Esta preparación no compila ni modifica archivos de ejecución.

La línea estable incluye controles configurables, stick derecho y SL/SR, puntero, Mega/combate, PC/mapa/Vuelo, Safari y casino. Estas funciones describen el ejecutor privado estable; no son software entregado aquí.

## Contenido disponible

- [Historia y notas de versión](RELEASE_NOTES_v1.6.md).
- [Arquitectura y evidencia de compilación](docs/TECHNICAL.md).
- [Estado de distribución e instalación](docs/USAGE.md).
- [Permisos pendientes y alternativas](docs/LEGAL_STATUS.md).
- [Borradores de solicitudes](docs/PERMISSION_REQUESTS.md).
- [Política de imágenes y capturas](docs/MEDIA.md).

La incorporación del motor requiere resolver la procedencia del atlas Arimo, los términos del código OpenGL generado y cuatro avisos históricos. Los ZIP completos de fuentes e instalación permanecen excluidos. Esta edición no ofrece instalador ni enlace de descarga. El camino para incorporar el motor está en [la lista de integración](docs/ENGINE_INTEGRATION.md).

## Créditos y derechos

Basado en el trabajo de [OpenGMK](https://github.com/OpenGMK/OpenGMK) y la adaptación de [Klisee y colaboradores](https://github.com/Klisee/pokemon-reloaded-android). Pokémon Reloaded fue creado por Fer/F3R y sus colaboradores; consultar sus [menciones oficiales](https://pokemon-reloaded.blogspot.com/2024/04/pokemon-reloaded-menciones.html). No se distribuyen aquí su código, imágenes, música ni recursos.

Proyecto no oficial, sin afiliación con Nintendo, Game Freak, The Pokémon Company ni los desarrolladores originales del juego. Los créditos no conceden permisos sobre materiales ajenos. La documentación original de esta edición usa MIT; esa licencia no se aplica al motor retenido, al juego ni al material externo enlazado. Véanse [el alcance de licencia](LICENSE_SCOPE.md) y [los avisos de terceros](THIRD_PARTY_NOTICES.md).
