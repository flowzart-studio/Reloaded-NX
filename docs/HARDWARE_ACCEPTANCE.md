# Aceptación en Switch — Reloaded-NX 1.6.1-rc.2

**Pendiente.** En hbmenu debe aparecer **Reloaded-NX**, autor **Flowzart Studio**, versión **1.6.1** e icono **RX**. `rc.2` identifica el paquete de prueba. SHA-256 del NRO: `32b52aeb13fcb873ffed0288162ff831ef7f4b79f6735f050af015187fbd8d8c`. Compruébalo en `SHA256SUMS.txt` y anótalo en el resultado.

## Preparación

1. Apaga la Switch. Respalda al PC la carpeta completa `/switch/reloaded-nx/` de tu tarjeta habitual y cualquier carpeta histórica con partidas únicas. Verifica el respaldo por SHA-256. Conserva en especial `session/game/data/partidas/` (toda la ranura), `session/game/data/audio/config.ini`, `config/`, `saves/` y `user/` si existen.
2. Desde `artifacts/Reloaded-NX-1.6.1-rc.2/microsd/switch/` copia la carpeta **reloaded-nx** a **/switch/** de la tarjeta de ensayo. Usa la opción de abajo si debes usar tu tarjeta habitual. La ruta final es `/switch/reloaded-nx/reloaded-nx.nro`; coteja el hash del NRO ya copiado.
3. Copia el EXE legítimo `Proyecto Reloaded The Last Beta 1.9.2.2 Full.exe` y su árbol `data/` limpio a `/switch/reloaded-nx/session/game/`. Empieza sin partidas personales. Para un caso avanzado, tras C4 puedes importar una copia desechable de una ranura completa. Comprueba la estructura con `python tools/check-installation.py X:/` desde el ZIP extraído, usando la letra de la tarjeta.
4. Expulsa la tarjeta y arranca hbmenu en **Application Mode** (mantén **R** al iniciar un título en tu entorno ya configurado). Selecciona **Reloaded-NX** en hbmenu.

### Si solo tienes tu microSD habitual

- Solo después de comprobar el respaldo externo y con la consola apagada, mueve **toda** la carpeta `/switch/reloaded-nx/` a `/_rnx-resguardo/reloaded-nx/` en la misma tarjeta. Si ese destino ya existe, elige otro vacío fuera de `/switch/`. Verifica que el árbol resguardado coincide con el respaldo y que la ruta canónica quedó libre.
- Ahora copia la carpeta candidata a `/switch/` y añade el juego limpio. Conserva los datos originales en el resguardo. Arranca la candidata desde hbmenu. El ejecutable usa rutas canónicas para datos y guardados, por lo que lanzar una segunda carpeta renombrada dentro de `/switch/` no aislaría los datos.
- Para revertir, apaga la consola, mueve el árbol completo de prueba fuera de `/switch/` y devuelve **la carpeta original entera** a `/switch/reloaded-nx/`. Verifica que coincide con el respaldo antes de volver a arrancar. Conserva el respaldo externo hasta comprobar que reaparecen tus partidas. Si el resguardo no coincide o no hay espacio para una instalación limpia, detén la prueba y usa otra tarjeta.

## Pruebas

| ID | Prioridad | Prueba | Resultado esperado |
| --- | --- | --- | --- |
| C1 | Crítica | Arranca, llega al menú del juego, sal y reabre desde hbmenu. | Nombre Reloaded-NX, autor Flowzart Studio, versión 1.6.1 e icono RX en hbmenu. Dos arranques y salida limpia, sin pantalla negra ni error de memoria. |
| C2 | Crítica | Empieza una partida. Recorre dos zonas, entra y sal de un combate. Escucha música y efectos. | Texturas, escala y shaders legibles. El texto por defecto aparece con espaciado, alineación y saltos correctos. Audio y MIDI siguen el juego. Las transiciones terminan sin bloqueo. |
| C3 | Crítica | Prueba movimiento, confirmar, cancelar y paneles. Configura el stick derecho y los botones SL/SR en un estilo compatible. Prueba Mega en combate con una partida que lo permita. | Acciones correctas, configurables y liberadas al soltar. Mega se activa mediante la acción asignada. |
| C4 | Crítica | Guarda progreso nuevo en una ranura de prueba. Cambia una asignación. Cierra, vuelve a abrir y carga. | La partida y su progreso reaparecen. El perfil conserva la asignación y responde como antes de cerrar. |
| C5 | Crítica | Abre PC/cajas y mapa, mueve el puntero, selecciona un destino y usa Vuelo cuando esté disponible. | Puntero visible, selección alineada con el objetivo, menús navegables. Vuelo termina en el destino elegido. |
| C6 | Crítica | Suspende y reanuda una vez durante exploración. | Imagen, sonido y controles se recuperan. No hay cierre ni entrada atascada. |
| E1 | Complementaria | Repite C2 en dock, compara con portátil y prueba alineación táctil en portátil. | La escala se ajusta al modo y el puntero coincide con el objetivo. |
| E2 | Complementaria | Safari. Usa Ball y baya, revisa el porcentaje cuando corresponda, prueba huida y salida. | Las acciones contextuales ocurren una vez por pulsación y el panel se cierra al salir. |
| E3 | Complementaria | Completa dos rondas de tragamonedas y una de póker, cobra, sal y vuelve a entrar. | Cada botón acciona su control efectivo, la siguiente ronda funciona y al salir se recupera el perfil normal. |

## Resultado

Envía `SHA-256 NRO / modelo Switch / firmware / Atmosphère / portátil o dock / fecha` y `C1 PASS | C2 FAIL al… | C3 PENDIENTE … | E1 …`. Si falla una crítica, detén la aceptación y guarda el fragmento pertinente de `/switch/reloaded-nx/logs/reloaded-nx.log` antes de relanzar. Indica el último paso que funcionó y cómo repetir el fallo. Si se modifica el NRO, la aceptación debe corresponder al nuevo hash.
