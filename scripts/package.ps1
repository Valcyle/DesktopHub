# Builds a portable Windows package and its SHA-256 checksum.
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File scripts/package.ps1
#
# The version is read from CMakeLists.txt. The resulting files are:
#   dist/DesktopHub-<version>-win64/          (portable application folder)
#   dist/DesktopHub-<version>-win64.zip       (release asset)
#   dist/SHA256SUMS.txt                       (checksum used by the updater)

param(
    [string]$BuildPreset = "mingw-release",
    [string]$QtDir = "C:\Qt\6.12.0\mingw_64",
    [string]$CMake = "C:\Qt\Tools\CMake_64\bin\cmake.exe"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Push-Location $root

try {
    $cmakeText = Get-Content "CMakeLists.txt" -Raw
    if ($cmakeText -notmatch 'project\(DesktopHub\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)') {
        throw "Could not read the project version from CMakeLists.txt"
    }
    $version = $Matches[1]

    Write-Host "Building Desktop Hub $version ($BuildPreset)..."
    & $CMake --preset $BuildPreset
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }

    & $CMake --build --preset $BuildPreset
    if ($LASTEXITCODE -ne 0) { throw "CMake build failed." }

    $buildDir = Join-Path $root "build\$BuildPreset"
    $distDir = Join-Path $root "dist"
    $stage = Join-Path $distDir "DesktopHub-$version-win64"

    Remove-Item $stage -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path $stage | Out-Null

    Copy-Item (Join-Path $buildDir "DesktopHub.exe") $stage
    Copy-Item (Join-Path $buildDir "DesktopHubUpdater.exe") $stage
    Copy-Item "README.md" $stage
    Copy-Item "docs\third-party-licenses.md" $stage

    $licenseDir = Join-Path $stage "LICENSES\Qt"
    New-Item -ItemType Directory -Force -Path $licenseDir | Out-Null
    Copy-Item "third_party\qt\LGPL-3.0.txt" $licenseDir
    Copy-Item "third_party\qt\GPL-3.0.txt" $licenseDir
    Copy-Item "third_party\qt\NOTICE.md" $licenseDir

    Write-Host "Deploying Qt runtime..."
    & (Join-Path $QtDir "bin\windeployqt.exe") `
        --release `
        --no-translations `
        --compiler-runtime `
        --qmldir (Join-Path $root "src\gui\qml") `
        (Join-Path $stage "DesktopHub.exe")
    if ($LASTEXITCODE -ne 0) { throw "windeployqt failed." }

    $zipPath = "$stage.zip"
    Remove-Item $zipPath -Force -ErrorAction SilentlyContinue
    Compress-Archive -Path (Join-Path $stage "*") -DestinationPath $zipPath

    $hash = (Get-FileHash $zipPath -Algorithm SHA256).Hash.ToLowerInvariant()
    $checksumPath = Join-Path $distDir "SHA256SUMS.txt"
    Set-Content -Path $checksumPath -Value "$hash  $(Split-Path $zipPath -Leaf)" -Encoding ascii

    Write-Host ""
    Write-Host "Package:  $zipPath"
    Write-Host "Folder:   $stage"
    Write-Host "Checksum: $checksumPath"
    Write-Host "SHA-256:  $hash"
} finally {
    Pop-Location
}
