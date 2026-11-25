# Instalar Reloaded-NX 1.6.1-rc.2

Esta compilación de validación para Nintendo Switch se identifica como **Reloaded-NX / Flowzart Studio / 1.6.1** en hbmenu. La aceptación física está pendiente. Los [créditos](../CREDITS.md) y avisos están en el paquete, y el archivo de fuentes correspondiente figura en `manifest.json`. SHA-256 acredita integridad.

## Requisitos e instalación

- Nintendo Switch con un entorno Horizon homebrew operativo y hbmenu en **Application Mode**. El modo applet del Álbum dispone de menos memoria. El rango de firmware y la compatibilidad con Switch 2 siguen sin verificar.
- Tu copia legítima de **Pokémon Reloaded The Last Beta 1.9.2.2**. Se requieren el archivo `Proyecto Reloaded The Last Beta 1.9.2.2 Full.exe` y su árbol `data/` completo. Otras ediciones quedan fuera del alcance validado.
- El ZIP contiene NRO, motor integrado, banco TimGM6mb y avisos. El lanzador usa sus valores iniciales cuando encuentra `config/` vacío. Los archivos del juego, las partidas, Atmosphère, hbmenu y el forwarder NSP se obtienen por separado cuando corresponda.

1. Comprueba el SHA-256 externo y extrae en una carpeta vacía. Sigue el [protocolo de Switch](HARDWARE_ACCEPTANCE.md) para respaldar la instalación y preparar la tarjeta de ensayo. Puedes usar la carpeta `microsd/` preparada en la entrega local, o extraer el ZIP. La guía también explica cómo aislar y revertir el ensayo con tu microSD habitual.
2. Copia `switch/` a la raíz de la tarjeta de ensayo. Coloca el EXE y `data/` en `/switch/reloaded-nx/session/game/`. El NRO y el banco deben quedar en `/switch/reloaded-nx/reloaded-nx.nro` y `/switch/reloaded-nx/engine/TimGM6mb.sf2`.
3. Desde la carpeta extraída, ejecuta `python tools/check-installation.py X:/` con la letra de la tarjeta. La comprobación lee la estructura. Puedes añadir `--manifest manifest.json` cuando el directorio comprobado contiene el paquete completo, incluidos sus documentos.
4. Expulsa la tarjeta y abre **Reloaded-NX** en hbmenu mediante Application Mode. Si falla, revisa el log en `/switch/reloaded-nx/logs/reloaded-nx.log`.

## Actualización y vuelta a la instalación anterior

Para esta prueba usa una segunda tarjeta si dispones de ella. En una única microSD, mueve el árbol antiguo fuera de la ruta canónica tras verificar el respaldo externo, según el protocolo. Para una futura actualización aprobada, apaga la consola, respalda la carpeta canónica y actualiza los binarios y avisos desde el ZIP. Conserva `session/game/`, `config/` y cualquier directorio personal. El perfil mantiene esquema v6 y el nombre `controls-1.4.json`. Restaura cada ranura con el conjunto de archivos de `session/game/data/partidas/`. Para volver a la instalación habitual tras el ensayo, apaga la consola y reinstala la tarjeta original o restaura el árbol íntegro desde el resguardo, según el protocolo.

## Diagnóstico

Ante un fallo de arranque, verifica primero Application Mode, el SHA-256 del NRO en la tarjeta, las rutas del EXE y `data/`, y el banco de sonido. Si una escena bloquea el juego, registra el último paso que funcionó, el modelo de Switch, firmware, reproducción y tramo pertinente del log. Para el casino puedes activar la traza creando `enable-casino-trace.flag` en el directorio del NRO y retirarla al terminar. Las métricas del atlas de texto cambiaron respecto del 1.6 y forman parte de la prueba visual.

Para el ensayo privado, entrega el resultado mediante el formato del [protocolo](HARDWARE_ACCEPTANCE.md). Para reportes tras publicación usa Issues de [Reloaded-NX](https://github.com/flowzart-studio/Reloaded-NX/issues). Omite datos personales y archivos del juego.
