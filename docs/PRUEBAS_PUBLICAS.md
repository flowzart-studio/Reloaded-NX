# Probar Reloaded-NX 1.6.1-rc.2 en Nintendo Switch

Esta primera pre-release experimental invita a probar la compatibilidad en distintas consolas. La compilación y la instalación estática están verificadas. La aceptación física de este NRO sigue en curso.

Reloaded-NX es un proyecto independiente de compatibilidad para Nintendo Switch. El juego pertenece a F3R y sus colaboradores. El proyecto mantiene los créditos a OpenGMK, Klisee, libnx y demás dependencias. Dirección y trabajo propio de Pedro Soto Almonacid, Flowzart Studio. Desarrollo asistido por OpenAI Codex.

## Instalar por primera vez

1. Prepara una Nintendo Switch con un entorno homebrew compatible ya operativo, hbmenu y espacio en microSD para el ejecutor y el juego. La referencia de juego es **Pokémon Reloaded The Last Beta 1.9.2.2**. Otras ediciones, Switch 2 y rangos concretos de firmware quedan por verificar.
2. Apaga la consola. Si ya tienes una instalación, copia todo `/switch/reloaded-nx/` y las carpetas históricas con datos únicos al PC. Coteja el respaldo por SHA-256. Usa preferentemente otra microSD. Con una sola tarjeta, resguarda el árbol original fuera de `/switch/` para dejar libre la ruta canónica.
3. Descarga `Reloaded-NX-1.6.1-rc.2.zip` y coteja el hash con `SHA256SUMS.txt`. Extrae y copia la carpeta **`switch/`** del ZIP a la raíz de la microSD. El ejecutable queda en `/switch/reloaded-nx/reloaded-nx.nro`.
4. Añade por separado tu copia legítima del archivo `Proyecto Reloaded The Last Beta 1.9.2.2 Full.exe` y su árbol `data/` limpio dentro de `/switch/reloaded-nx/session/game/`. Empieza sin partidas personales. Desde el ZIP extraído, `python tools/check-installation.py X:/` permite comprobar la estructura en el PC.
5. Expulsa la tarjeta. Abre hbmenu en **Application Mode** mediante title override, manteniendo **R** al iniciar un título si ese es el atajo de tu entorno. En hbmenu elige **Reloaded-NX**, de **Flowzart Studio**, versión **1.6.1**, con icono **RX**.

El SHA-256 del NRO de esta pre-release es `32b52aeb13fcb873ffed0288162ff831ef7f4b79f6735f050af015187fbd8d8c`. En PowerShell compruébalo con `Get-FileHash -Algorithm SHA256 X:\switch\reloaded-nx\reloaded-nx.nro`.

## Acceso HOME opcional con DBI

El paquete HOME se distribuye por separado. Primero prueba el NRO con hbmenu. El NSP experimental apunta a `sdmc:/switch/reloaded-nx/reloaded-nx.nro`. Requiere un entorno de pruebas que ya admita este tipo de título homebrew, preferentemente emuMMC separada, y una instalación local de DBI. El firmware original no acepta este NSP. La instalación de títulos homebrew NSP puede conllevar sanciones de Nintendo. [NTON](https://github.com/rlaphoenix/NTON#readme)

1. Verifica el SHA-256 del NSP y el ID **`01834AD7B4EE8000`**. Comprueba en **Browse installed applications** de DBI que está libre o corresponde a tu propio acceso canónico de Reloaded-NX. Si pertenece a otro título, detén la instalación. Usar el mismo ID sobre el acceso canónico anterior lo reemplaza.
2. Copia solo el NSP verificado a una carpeta de la microSD. En DBI abre **Browse SD Card**, selecciona el NSP, elige **SD** como destino y confirma. DBI registra el título en la MMC activa incluso cuando su contenido se guarda en SD. Si se rechaza el NSP, conserva la vía hbmenu y reporta el mensaje. [DBI](https://github.com/rashevskyv/dbi#browse-sd-card--browse-usb0-drive)
3. Desde HOME abre **Reloaded-NX** y comprueba carga, salida a HOME y reapertura. Incluye la versión de DBI, el hash del NSP y si el mismo NRO funciona desde hbmenu.

## Actualizar o retirar

- Para actualizar, cierra el programa, apaga la consola y respalda el árbol completo. Reemplaza los archivos del ejecutor indicados en el ZIP y preserva `session/game/`, `data/partidas/` dentro del juego, `config/`, `saves/` y `user/` si existen. Instala un NRO nuevo en la misma ruta canónica. El acceso HOME conserva su destino y sus propios metadatos.
- Para retirar el ejecutor, mueve la carpeta canónica al PC después de verificar el respaldo. Si estabas ensayando en tu tarjeta habitual, devuelve el árbol original completo a su ruta antes de arrancar. Para retirar solo el acceso HOME, desinstala el título **`01834AD7B4EE8000`** desde Gestión de datos de la MMC correcta y conserva el árbol del juego de la microSD.

## Qué probar y cómo informar

Prueba arranque y reapertura, mapa y puntero, controles, combate, audio, guardado y carga de una ranura desechable, y suspensión y reanudación. Dock, táctil, Mega, Safari y casino amplían la cobertura si puedes acceder a esas escenas.

Abre un [Issue](https://github.com/flowzart-studio/Reloaded-NX/issues/new/choose) para informar tanto un funcionamiento correcto como un fallo. Indica modelo de Switch, firmware, versión de Atmosphère, modo de arranque, portátil o dock, versión del juego, versión de la candidata y SHA-256 del NRO. Para un fallo incluye pasos, resultado esperado y observado. Para HOME añade DBI, ID y hash del NSP.

El log queda en `/switch/reloaded-nx/logs/reloaded-nx.log`. Abre una copia en el PC y comparte únicamente las líneas pertinentes. Borra del extracto nombres personales, rutas privadas, números de serie, identificadores de cuenta o dispositivo y cualquier clave. Mantén las partidas, los archivos del juego y los logs completos fuera del Issue.
