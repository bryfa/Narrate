# Narrate Build Script (PowerShell)
# Build script for Narrate project with support for different build targets

param(
    [string]$Target = "ALL",
    [switch]$Help
)

# Show help
if ($Help) {
    Write-Host "Narrate Build Script (PowerShell)"
    Write-Host "=================================="
    Write-Host ""
    Write-Host "Usage: .\build.ps1 [-Target <TARGET>] [-Help]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -Target <TARGET>    Specify what to build (default: ALL)"
    Write-Host "                      Options: ALL, PLUGIN, STANDALONE, CONSOLE"
    Write-Host "  -Help               Show this help message"
    Write-Host ""
    Write-Host "Build Targets:"
    Write-Host "  ALL          Build everything (VST3 + Standalone + Console)"
    Write-Host "  PLUGIN       Build only VST3 plugin"
    Write-Host "  STANDALONE   Build only standalone application"
    Write-Host "  CONSOLE      Build only console export tool"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\build.ps1"
    Write-Host "  .\build.ps1 -Target CONSOLE"
    Write-Host "  .\build.ps1 -Target STANDALONE"
    exit 0
}

# Validate Target
$ValidTargets = @("ALL", "PLUGIN", "STANDALONE", "CONSOLE")
if ($ValidTargets -notcontains $Target) {
    Write-Host "Error: Invalid Target: $Target" -ForegroundColor Red
    Write-Host "Valid options are: ALL, PLUGIN, STANDALONE, CONSOLE"
    exit 1
}

Write-Host "========================================" -ForegroundColor Yellow
Write-Host "  Narrate Build Script" -ForegroundColor Yellow
Write-Host "  Target: $Target" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host ""

# Get the directory where this script is located
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Create build directory if it doesn't exist
$BuildDir = "$ScriptDir\build-windows"
if (-not (Test-Path $BuildDir)) {
    Write-Host "Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Navigate to the build directory
Set-Location $BuildDir

# Configure CMake if needed (if CMakeCache.txt doesn't exist or target changed)
$Reconfigure = $false
if (-not (Test-Path "CMakeCache.txt")) {
    $Reconfigure = $true
} else {
    # Check if BUILD_TARGET has changed
    $CachedTarget = (Select-String -Path "CMakeCache.txt" -Pattern "BUILD_TARGET:").ToString() -replace ".*=", ""
    if ($CachedTarget -ne $Target) {
        Write-Host "Build target changed from $CachedTarget to $Target" -ForegroundColor Yellow
        $Reconfigure = $true
    }
}

if ($Reconfigure) {
    Write-Host "Configuring CMake with BUILD_TARGET=$Target..." -ForegroundColor Yellow
    cmake "-DBUILD_TARGET=$Target" "$ScriptDir"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake configuration failed!" -ForegroundColor Red
        Set-Location $ScriptDir
        exit 1
    }
}

# Build the project
Write-Host "Building project..."
cmake --build . --config Release -- /m

# Check if build succeeded
if ($LASTEXITCODE -eq 0) {
    Write-Host "Build successful!"
    Set-Location $ScriptDir
    exit 0
} else {
    Write-Host "Build failed!"
    Set-Location $ScriptDir
    exit 1
}
