<#
.SYNOPSIS
    One-time development environment setup for BloodArena.

.DESCRIPTION
    Clones, bootstraps, and integrates vcpkg, then registers the VCPKG_ROOT user
    environment variable. Idempotent — steps already completed are skipped.
    Visual Studio must be fully restarted afterwards for the environment variable
    to take effect.

.PARAMETER VcpkgRoot
    vcpkg install path. If omitted, reuses the existing VCPKG_ROOT user
    environment variable; otherwise falls back to C:\src\vcpkg.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Scripts\setup-env.ps1
#>
param(
    [string]$VcpkgRoot
)

if (-not $VcpkgRoot) {
    $existing = [Environment]::GetEnvironmentVariable("VCPKG_ROOT", "User")
    if ($existing) {
        $VcpkgRoot = $existing
        Write-Host "Reusing existing VCPKG_ROOT: $VcpkgRoot" -ForegroundColor DarkGray
    } else {
        $VcpkgRoot = "C:\src\vcpkg"
    }
}

$ErrorActionPreference = "Stop"

Write-Host "[1/4] Checking vcpkg path: $VcpkgRoot" -ForegroundColor Cyan
if (-not (Test-Path $VcpkgRoot)) {
    $parent = Split-Path $VcpkgRoot -Parent
    if (-not (Test-Path $parent)) {
        New-Item -ItemType Directory -Path $parent -Force | Out-Null
    }
    Write-Host "  -> git clone https://github.com/microsoft/vcpkg.git" -ForegroundColor Yellow
    git clone https://github.com/microsoft/vcpkg.git $VcpkgRoot
} else {
    Write-Host "  -> Already exists. Skipping clone." -ForegroundColor DarkGray
}

Write-Host "[2/4] bootstrap-vcpkg.bat" -ForegroundColor Cyan
$bootstrap = Join-Path $VcpkgRoot "bootstrap-vcpkg.bat"
$vcpkgExe  = Join-Path $VcpkgRoot "vcpkg.exe"
if (-not (Test-Path $vcpkgExe)) {
    & $bootstrap
} else {
    Write-Host "  -> vcpkg.exe already exists. Skipping bootstrap." -ForegroundColor DarkGray
}

Write-Host "[3/4] vcpkg integrate install" -ForegroundColor Cyan
& $vcpkgExe integrate install

Write-Host "[4/4] Registering VCPKG_ROOT user environment variable" -ForegroundColor Cyan
$current = [Environment]::GetEnvironmentVariable("VCPKG_ROOT", "User")
if ($current -ne $VcpkgRoot) {
    [Environment]::SetEnvironmentVariable("VCPKG_ROOT", $VcpkgRoot, "User")
    Write-Host "  -> VCPKG_ROOT=$VcpkgRoot (previous: '$current')" -ForegroundColor Yellow
} else {
    Write-Host "  -> Already set to $VcpkgRoot." -ForegroundColor DarkGray
}

Write-Host ""
Write-Host "Done. Fully restart Visual Studio, then rebuild BloodArena.sln." -ForegroundColor Green
Write-Host "The first build will take several minutes as dependencies are downloaded and compiled." -ForegroundColor Green
