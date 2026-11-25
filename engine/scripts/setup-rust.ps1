# SPDX-License-Identifier: MIT
# Portable official components only: no installer, registry or global PATH changes.
param([string]$LockFile='tools/rust-toolchain.lock.json', [string]$PrefixDirectory='tools/rust-portable')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$lock = Get-Content (Join-Path $projectRoot $LockFile) -Raw | ConvertFrom-Json
$prefix = Join-Path $projectRoot $PrefixDirectory
$downloads = Join-Path $projectRoot 'tools/downloads'
$extractRoot = Join-Path $downloads 'rust-extracted'
New-Item -ItemType Directory -Path $prefix,$downloads,$extractRoot -Force | Out-Null
foreach ($component in $lock.components) {
    $fileName = [IO.Path]::GetFileName([Uri]$component.url)
    $archive = Join-Path $downloads $fileName
    if (-not (Test-Path -LiteralPath $archive)) {
        Write-Output "Downloading official $fileName"
        Invoke-WebRequest -Uri $component.url -OutFile $archive
    }
    if ((Get-FileHash -LiteralPath $archive).Hash.ToLowerInvariant() -ne $component.sha256) { throw "Hash mismatch: $fileName" }
    $directory = Join-Path $extractRoot $fileName.Replace('.tar.xz','')
    if (-not (Test-Path -LiteralPath $directory)) {
        & "$env:SystemRoot/System32/tar.exe" -xf $archive -C $extractRoot
        if ($LASTEXITCODE -ne 0) { throw "Extraction failed: $fileName" }
    }
    $componentsFile = Join-Path $directory 'components'
    foreach ($componentDirectory in Get-Content -LiteralPath $componentsFile) {
        $source = Join-Path $directory $componentDirectory
        foreach ($item in Get-ChildItem -LiteralPath $source | Where-Object Name -ne 'manifest.in') {
            Copy-Item -LiteralPath $item.FullName -Destination $prefix -Recurse -Force
        }
    }
}
& (Join-Path $prefix 'bin/rustc.exe') -vV
if ($LASTEXITCODE -ne 0) { throw 'Portable compiler failed' }
& (Join-Path $prefix 'bin/cargo.exe') --version
if ($LASTEXITCODE -ne 0) { throw 'Portable Cargo failed' }
