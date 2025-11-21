# Clean Build Directory Script (PowerShell)
# Use this script to completely clean the build directory

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Yellow
Write-Host "  Cleaning Build Directory" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host ""

if (Test-Path "build-windows") {
    Write-Host "Removing build-windows directory..." -ForegroundColor Yellow
    Remove-Item -Path "build-windows" -Recurse -Force -ErrorAction SilentlyContinue
    Write-Host "Build-windows directory removed!" -ForegroundColor Green
} else {
    Write-Host "Build-windows directory doesn't exist. Nothing to clean." -ForegroundColor Green
}

Write-Host ""
Write-Host "Creating fresh build-windows directory..." -ForegroundColor Yellow
New-Item -Path "build-windows" -ItemType Directory | Out-Null
Write-Host "Done!" -ForegroundColor Green
Write-Host ""
Write-Host "You can now run .\build.ps1 or .\test.ps1" -ForegroundColor Cyan
