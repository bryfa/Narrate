# Narrate Test Runner Script (PowerShell)
# This script builds and runs tests for the Narrate project

param(
    [string]$Target = "ALL",
    [switch]$Help
)

$ErrorActionPreference = "Stop"

# Show help
if ($Help) {
    Write-Host "Narrate Test Runner Script (PowerShell)"
    Write-Host "========================================"
    Write-Host ""
    Write-Host "Usage: .\test.ps1 [-Target <TARGET>] [-Help]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -Target <TARGET>    Specify build target for tests (default: ALL)"
    Write-Host "                      Options: ALL, PLUGIN, STANDALONE, CONSOLE"
    Write-Host "  -Help               Show this help message"
    Write-Host ""
    Write-Host "Notes:"
    Write-Host "  - Tests are built with the same BUILD_TARGET configuration"
    Write-Host "  - The BUILD_TARGET determines which code paths are tested"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\test.ps1"
    Write-Host "  .\test.ps1 -Target CONSOLE"
    Write-Host "  .\test.ps1 -Target STANDALONE"
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
Write-Host "  Narrate Test Runner" -ForegroundColor Yellow
Write-Host "  Target: $Target" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host ""

# Get the directory where this script is located
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Check if build-windows directory exists, create if needed
if (-not (Test-Path "build-windows")) {
    Write-Host "Creating build-windows directory..." -ForegroundColor Yellow
    New-Item -Path "build-windows" -ItemType Directory | Out-Null
}

Set-Location build-windows

# Configure with tests enabled and specified target
Write-Host "Configuring with tests enabled (BUILD_TARGET=$Target)..." -ForegroundColor Yellow
cmake "-DBUILD_TESTS=ON" "-DBUILD_TARGET=$Target" "$ScriptDir"

# Build tests
Write-Host "Building tests..." -ForegroundColor Yellow
cmake --build . --target NarrateTests

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Write-Host "Build successful!" -ForegroundColor Green
Write-Host ""

# Run tests
Write-Host "Running tests..." -ForegroundColor Yellow
Write-Host ""

# Run the test executable directly for better output
$testExecutable = $null
if (Test-Path ".\NarrateTests.exe") {
    $testExecutable = ".\NarrateTests.exe"
} elseif (Test-Path ".\NarrateTests") {
    $testExecutable = ".\NarrateTests"
} elseif (Test-Path ".\Debug\NarrateTests.exe") {
    $testExecutable = ".\Debug\NarrateTests.exe"
} elseif (Test-Path ".\Release\NarrateTests.exe") {
    $testExecutable = ".\Release\NarrateTests.exe"
}

if ($null -eq $testExecutable) {
    Write-Host "Test executable not found!" -ForegroundColor Red
    Set-Location ..
    exit 1
}

# Run tests with any provided arguments
& $testExecutable $args
$testResult = $LASTEXITCODE

Write-Host ""

# Report results
if ($testResult -eq 0) {
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "  All tests passed! `u{2713}" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
} else {
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "  Some tests failed `u{2717}" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Set-Location ..
