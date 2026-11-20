# PUBLICATION_READY — Reloaded-NX 1.6

Fecha: 2026-11-20 (America/Santiago). **LISTO LOCALMENTE para revisión final y publicación del escenario B: documentación.** El conjunto completo, el instalador y la Release funcional `v1.6` siguen pendientes. No se ha subido nada.

**Publicable:** únicamente los archivos enumerados en PUBLIC_FILES.json de esta carpeta `Reloaded-NX`: README inglés/español, créditos, licencia y alcance MIT de la documentación original, historia/notas, documentación técnica, revisiones focalizadas y guías de integración/permisos. SHA256SUMS.txt cubre el inventario y todos los archivos de contenido; se excluye a sí mismo para evitar autorreferencia. No contiene motor ni es una distribución funcional.

**Excluido:** NRO estable/reconstruido, instalador y ZIP de fuentes completo, arimo.dat, bindings OpenGL, las cuatro dependencias L4 y todos los demás payloads del motor, fuentes/bancos/gráficos, juego/EXE/data, capturas de juego/forwarder, partidas, claves, historial .git original, bundle, evidencias privadas, logs y rutas personales. No subir las carpetas hermanas `repository` ni `private-evidence`, ni el directorio padre.

**Pendiente:** L1 procedencia/licencia aplicable al atlas exacto; L2 términos del output exacto de gl_generator 0.14.0/khronos_api 3.1.0; L4 avisos completos de block 0.1.6, malloc_buf 0.0.6, objc-foundation 0.1.1 y objc_id 0.1.1. No se atribuyen permisos por suposición. Véanse docs/LEGAL_STATUS.md y docs/PERMISSION_REQUESTS.md. La ausencia de capturas autorizadas se resuelve excluyéndolas, sin impedir esta edición.

**Validación:** selección por lista explícita, solo texto/documentación/configuración, revisión del alcance de licencia, créditos pedidos, enlaces locales, rastreo de credenciales/rutas privadas, integridad SHA-256 y cotejo del ZIP documental. Los resultados públicos están en VALIDATION.json; límites: un rastreo de patrones no constituye garantía universal. Sin cambios de motor, NRO, SD, partidas, ramas ni historia originales. Se distribuye un árbol sin .git para evitar arrastrar metadatos privados; el índice de prueba Git permanece fuera del candidato.

## Pasos exactos para publicar — ejecutar solo tras aprobación final

1. Revisar esta carpeta y VALIDATION.json. Aprobar expresamente publicar **solo el escenario B documental** en la cuenta personal; no aprobar por extensión los ZIP antiguos. Abrir una consola dentro de esta carpeta `Reloaded-NX`, nunca en el workspace original. Usar Git nativo y GitHub CLI autenticados en la cuenta personal.
2. Verificar los hashes antes de inicializar Git; desde esta carpeta, Python 3:

```powershell
python -c "import hashlib,pathlib; p=pathlib.Path('.'); rows=[s.split('  ',1) for s in (p/'SHA256SUMS.txt').read_text().splitlines()]; assert all(hashlib.sha256((p/n).read_bytes()).hexdigest()==h for h,n in rows); print('SHA-256 OK')"
git init -b main
git add --all
git diff --cached --check
git diff --cached --stat
```

3. Comprobar la identidad de autor configurada con `git var GIT_AUTHOR_IDENT` y elegir un correo público/noreply adecuado. No reutilizar automáticamente correos privados de la historia original. Confirmar que el listado staged coincide con PUBLIC_FILES.json más PUBLIC_FILES.json y SHA256SUMS.txt. Crear el commit local:

```powershell
git commit -m "Publish Reloaded-NX 1.6 documentation and credits"
git status --short
gh auth status
gh api user --jq .login
```

4. Verificar que el login mostrado es la cuenta personal aprobada. Solo después de la aprobación final y esa comprobación, crear el remoto público y subirlo:

```powershell
$publicationOwner = gh api user --jq .login
gh repo create "$publicationOwner/Reloaded-NX" --public --source=. --remote=origin --description "Reloaded-NX 1.6: project documentation, credits and engine publication status"
git push -u origin main
```

5. Revisar README y README.es en GitHub, autoría visible, créditos, archivos y estado documental. No crear Release/tag `v1.6` ni adjuntar binarios en esta fase. El ZIP hermano `Reloaded-NX-documentation-publication.zip` es una copia transportable del contenido de esta carpeta, no un instalador; no es necesario adjuntarlo como Release.

Flujo de referencia: [GitHub Docs](https://docs.github.com/en/migrations/importing-source-code/using-the-command-line-to-import-source-code/adding-locally-hosted-code-to-github). Integración posterior del motor: docs/ENGINE_INTEGRATION.md. No se ha ejecutado ningún comando de publicación ni enviado solicitudes a terceros.
