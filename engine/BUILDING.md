# Compilar Reloaded-NX 1.6

El ZIP de fuentes contiene el runner activo, C/libnx, la adaptación libc, shaders/recursos propios, contratos de controles y casino, dependencias Cargo vendorizadas, fuentes de std, fuentes/recetas de Mesa/libdrm y banco con sus avisos. No contiene juegos, partidas, claves, NRO anteriores ni herramientas propietarias.

## Estado de esta auditoría

Las instrucciones siguientes proceden del flujo implementado en los scripts y de los registros previos. La reconstrucción aislada autorizada ya terminó, sin sustituir el NRO congelado; comparación byte a byte: DIFFERENT. Ver REBUILD_RESULT.md. Desde este árbol usar una carpeta de destino nueva y `-TargetDirectory` nuevo para aislar objetos. Ejecutar `New-Item -ItemType Directory -Force migration` antes del driver: el wrapper se genera allí y algunas rutas presuponen su existencia. `prepare-source.py` y `package-release.py` son herramientas históricas de preparación que requieren entradas privadas/de desarrollo adicionales; no son comandos de instalación ni un empaquetado público aprobado.

## Herramientas externas

Entorno de referencia: Windows, PowerShell 7, Python 3.13, devkitA64 GCC 16.1.0 y libnx 4.12.0-1. Rust es nightly-2026-10-30, rustc 1.101.0-nightly (92b5b8c87), Windows GNU, con rust-src y linker LLD. Los componentes oficiales y hashes están en `tools/rust-switch-toolchain.lock.json`; `scripts/setup-rust.ps1 -LockFile tools/rust-switch-toolchain.lock.json -PrefixDirectory tools/rust-switch-portable` los instala localmente sin cambiar el registro ni el PATH global.

Para compilar dependencias nativas del host se usan w64devkit con GCC 16.2.0 y CMake 4.4.5. Aportar esas herramientas por separado y seleccionar sus directorios mediante las variables siguientes. El SDK devkitPro y las herramientas de compilación son requisitos de desarrollo, no recursos que deban copiarse a la SD. No se distribuye NTON ni se necesita para esta versión.

## Fuentes limpias y Cargo offline

Extraer `Reloaded-NX-1.6-source.zip` en una carpeta nueva. `.cargo/config.toml` reemplaza crates.io por `third_party/cargo`; cada paquete conserva su licencia y checksums derivados del archivo original contrastado con Cargo.lock. Incluye las dependencias de resolución de std y de otros targets que Cargo solicita aun al compilar Horizon. Las bibliotecas de targets ajenos no se incorporan por esa razón al NRO.

Desde la raíz extraída:

```powershell
$env:DEVKITPRO = 'D:/SDK/devkitPro'
$env:RELOADED_RUST_TOOLCHAIN = 'D:/SDK/rust-switch-portable'
$env:RELOADED_NATIVE_TOOLS = 'D:/SDK/w64devkit'
$env:RELOADED_CMAKE = 'D:/SDK/cmake-4.4.5-windows-x86_64'
$env:RELOADED_CARGO_HOME = "$PWD/build/empty-cargo-home"
python scripts/fetch-horizon-mesa.py
$env:RELOADED_PORTLIBS = "$PWD/migration/horizon-portlibs/opt/devkitpro/portlibs/switch"
./scripts/build-full-runner-horizon.ps1 -DevkitRoot $env:DEVKITPRO -OutputDirectory "$PWD/build/output"
python scripts/verify-full-runner-nro.py build/output
```

El directorio de salida debe ser nuevo. El driver reconstruye Rust/std, compila C con errores de advertencia activados y verifica el resultado. `-ReuseRustLibrary` está reservado a reconstrucciones parciales de desarrollo; no se usó para acreditar la compilación independiente de 1.6. Los manifiestos de Mesa fijan versiones y SHA-256 de los paquetes del SDK. Puede utilizarse un caché verificado de esos paquetes, sin usar ninguna instalación previa del ejecutor.

La prueba de construcción separa fuentes, Cargo Home y resultados de los de 1.5; las herramientas generales del SDK se aportan externamente. No equivale a haber instalado desde cero todo el sistema operativo. El informe registra cualquier fallo y el resultado final.

## Pruebas y empaquetado

`test-casino.ps1`, `test-horizon06b.ps1` y `test-resource-epoch.py` comprueban contratos del código activo. Sus rutas de herramientas también se pueden seleccionar por variables de entorno. `test-sd-audit.py` prueba protección de datos y ausencia de escrituras. El empaquetador `package-release.py --help` toma NRO validado, árbol de fuentes y banco verificado contra su archivo fuente. No necesita juego para construir los ZIP.

El archivo `SOURCE_MANIFEST.json` identifica los fuentes distribuidos. Las licencias de dependencias se conservan por archivo; los snapshots del SDK y del banco conservan sus recetas/procedencia. Ver `THIRD_PARTY_NOTICES.md`. La prueba privada de instalación limpia usa un ZIP legítimo del juego y nunca lo añade a los paquetes públicos.

El driver admite `-TargetDirectory <directorio-nuevo>` para separar también todos los objetos Rust. El wrapper remapea las rutas de fuentes, toolchain y archivos generados para que las ubicaciones de pánico del binario no contengan rutas privadas del equipo. `setup-dependencies.ps1` puede aprovisionar fuentes fijadas al trabajar desde Git; el ZIP de fuentes ya las incorpora y no requiere ese paso de descarga.

## Regeneración opcional del atlas

El generador existente `scripts/bake-horizon-font.py` se añade a este árbol de revisión para corresponder con `switch-port/fonts/README.md`. Requiere Pillow; la referencia histórica es Pillow 13.0.0/FreeType 2.14.3. Las compilaciones normales consumen el atlas ya incluido. No ejecutar una regeneración para reemplazar recursos de la 1.6 congelada durante esta preparación.

## Prueba independiente preparada

Ver [REBUILD_VERIFICATION.md](REBUILD_VERIFICATION.md). El preflight dirigido y la reconstrucción posteriormente autorizada pasaron; el resultado medido está en REBUILD_RESULT.md. La recuperación de avisos está en L4_DEPENDENCY_REVIEW.md. Ninguno de estos documentos sustituye la evidencia de una ejecución real.

Actualización de reconstrucción autorizada: fuentes funcionales sin cambios, compilación y validación estática PASS; comparación binaria DIFFERENT. Ver REBUILD_RESULT.md y BUILD_EVIDENCE.json. No se instaló el resultado ni se publicó; los bloqueos legales pendientes permanecen.
