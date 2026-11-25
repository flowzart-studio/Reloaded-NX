# Forwarder HOME experimental — Reloaded-NX 1.6.1-rc.2

Paquete opcional para un entorno de pruebas que ya admite títulos NSP homebrew. Identidad HOME **Reloaded-NX / Flowzart Studio / 1.6.1**, icono **RX**, ID **`01834AD7B4EE8000`** y destino **`sdmc:/switch/reloaded-nx/reloaded-nx.nro`**. Estado **verificado estáticamente, pendiente de pruebas físicas del NSP nuevo**. El ejecutor se instala con el ZIP principal y funciona con hbmenu en Application Mode.

## Instalar mediante DBI

1. Respalda la microSD y verifica que estás en el entorno de pruebas previsto, preferentemente emuMMC separada. Arranca primero el NRO desde hbmenu. Comprueba los SHA-256 publicados.
2. Usa **Browse installed applications** de DBI para verificar el ID. Si está ocupado por tu forwarder canónico de Reloaded-NX, instalar este NSP lo reemplaza. Si corresponde a otro título, detente. El catálogo de NTON no refleja los títulos privados instalados en tu consola.
3. Copia el NSP local a la microSD. En DBI abre **Browse SD Card**, selecciona solo ese archivo, marca destino **SD** y confirma. La MMC activa registra el título. Si lo rechaza, usa hbmenu y anota el error. [Guía de DBI](https://github.com/rashevskyv/dbi#browse-sd-card--browse-usb0-drive)
4. En HOME abre **Reloaded-NX**, carga el juego, sal a HOME y vuelve a abrir. Registra Switch, firmware, Atmosphère, DBI, ID, hash del NSP y hash del NRO.

El firmware original no admite este NSP homebrew. NTON documenta requisitos adicionales y riesgo de sanción de consola o cuenta. La compatibilidad de un entorno previo con este NSP requiere comprobarse en hardware. [NTON](https://github.com/rlaphoenix/NTON#readme)

## Actualizar o retirar

El acceso lee el NRO desde la ruta canónica cada vez que arranca. Puedes actualizar el NRO sin reinstalar el acceso, y HOME conservará sus metadatos hasta que reemplaces el propio NSP. El NSP reserva cero espacio de guardado propio. Los datos del juego y la configuración viven en la microSD. Para retirar el acceso, elimina únicamente el título con ese ID desde Gestión de datos y conserva el árbol `/switch/reloaded-nx/`.

## Generar localmente con NTON

El kit aporta `build-1.6.1.py`, el icono original y su fuente, y referencias verificables del loader. Descarga **NTON v3.0.1** de su proyecto. Usa Python 3.10 o posterior en Windows, la distribución fijada de NTON ya configurada localmente, el NRO exacto de rc.2 y un directorio de salida nuevo. Mantén el material local de NTON fuera de la salida y de cualquier repositorio o reporte. [NTON v3.0.1](https://github.com/rlaphoenix/NTON/releases/tag/v3.0.1)

```powershell
python build-1.6.1.py --nro <ruta-NRO> --nton <ruta-NTON> --keys <archivo-local> --output <carpeta-nueva>
```

El programa comprueba hashes de las herramientas y del ExeFS, omite el logo de arranque, reabre las tres NCA y coteja destino, identidad, permisos NPDM, ID, icono y ausencia de particiones extra. Genera un manifiesto con la suma de la salida local. Si necesitas otro ID, usa `--title-id` solo después de verificar que el nuevo ID está libre en tu consola. Un build local puede producir un NSP con otra suma.
