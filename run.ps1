# Get the directory where this script is located
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Build the project using build.ps1
& "$ScriptDir\build.ps1"

# Run the app if build succeeded
if ($LASTEXITCODE -eq 0) {
    Write-Host "Launching Narrate..."
    & "$ScriptDir\build-windows\Narrate_artefacts\Release\Standalone\Narrate.exe"
} else {
    Write-Host "Build failed!"
    exit 1
}
