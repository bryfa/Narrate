# Narrate Run Script (PowerShell)
# Helper script for running different built targets

param(
    [string]$Target = "STANDALONE",
    [switch]$Help
)

$ErrorActionPreference = "Stop"

# Show help
if ($Help) {
    Write-Host "Narrate Run Script (PowerShell)"
    Write-Host "================================"
    Write-Host ""
    Write-Host "Usage: .\run.ps1 [-Target <TARGET>] [args...]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -Target <TARGET>    Specify which target to run (default: STANDALONE)"
    Write-Host "                      Options: STANDALONE, CONSOLE"
    Write-Host "  -Help               Show this help message"
    Write-Host ""
    Write-Host "Targets:"
    Write-Host "  STANDALONE   Run the standalone application (default)"
    Write-Host "  CONSOLE      Run the console export tool"
    Write-Host "               (Pass additional args for console tool)"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\run.ps1                    # Runs standalone by default"
    Write-Host "  .\run.ps1 -Target STANDALONE"
    Write-Host "  .\run.ps1 -Target CONSOLE project.narrate output.json --format json"
    Write-Host "  .\run.ps1 -Target CONSOLE --help"
    exit 0
}

# Validate Target
$ValidTargets = @("STANDALONE", "CONSOLE")
if ($ValidTargets -notcontains $Target) {
    Write-Host "Error: Invalid Target: $Target" -ForegroundColor Red
    Write-Host "Valid options are: STANDALONE, CONSOLE"
    exit 1
}

Write-Host "======================================" -ForegroundColor Yellow
Write-Host "  Narrate Run Script" -ForegroundColor Yellow
Write-Host "  Target: $Target" -ForegroundColor Yellow
Write-Host "======================================" -ForegroundColor Yellow
Write-Host ""

# Get the directory where this script is located
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = "$ScriptDir\build-windows"

# Check if build directory exists
if (-not (Test-Path $BuildDir)) {
    Write-Host "Error: Build directory not found" -ForegroundColor Red
    Write-Host "Please run .\build.ps1 first to build the project"
    exit 1
}

# Run the appropriate target
switch ($Target) {
    "STANDALONE" {
        $StandaloneExe = "$BuildDir\Narrate_artefacts\Release\Standalone\Narrate.exe"

        if (-not (Test-Path $StandaloneExe)) {
            Write-Host "Error: Standalone executable not found" -ForegroundColor Red
            Write-Host "Expected location: $StandaloneExe"
            Write-Host ""
            Write-Host "Please build the standalone target first:"
            Write-Host "  .\build.ps1 -Target STANDALONE"
            Write-Host "or"
            Write-Host "  .\build.ps1 -Target ALL"
            exit 1
        }

        Write-Host "Launching Narrate Standalone..." -ForegroundColor Green
        Start-Process -FilePath $StandaloneExe
        Write-Host "Standalone application launched" -ForegroundColor Green
    }

    "CONSOLE" {
        $ConsoleExe = "$BuildDir\Release\NarrateConsole.exe"

        # Try alternative location if not found
        if (-not (Test-Path $ConsoleExe)) {
            $ConsoleExe = "$BuildDir\NarrateConsole.exe"
        }

        if (-not (Test-Path $ConsoleExe)) {
            Write-Host "Error: Console executable not found" -ForegroundColor Red
            Write-Host "Expected location: $BuildDir\Release\NarrateConsole.exe"
            Write-Host ""
            Write-Host "Please build the console target first:"
            Write-Host "  .\build.ps1 -Target CONSOLE"
            Write-Host "or"
            Write-Host "  .\build.ps1 -Target ALL"
            exit 1
        }

        # Collect remaining arguments for console tool
        $ConsoleArgs = $args

        if ($ConsoleArgs.Count -eq 0) {
            Write-Host "Running console tool (no arguments - showing help)..." -ForegroundColor Cyan
            & $ConsoleExe --help
        } else {
            Write-Host "Running console tool with arguments..." -ForegroundColor Cyan
            & $ConsoleExe $ConsoleArgs
        }
    }
}

exit $LASTEXITCODE
