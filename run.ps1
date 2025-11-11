# Get the directory where this script is located
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Create build directory if it doesn't exist
$BuildDir = "$ScriptDir\build-windows"
if (-not (Test-Path $BuildDir)) {
    Write-Host "Creating build directory..."
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Navigate to the build directory
Set-Location $BuildDir

# Configure CMake if needed (if CMakeCache.txt doesn't exist)
if (-not (Test-Path "CMakeCache.txt")) {
    Write-Host "Configuring CMake..."
    cmake "$ScriptDir"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake configuration failed!"
        Set-Location $ScriptDir
        exit 1
    }
}

# Build the project
Write-Host "Building project..."
cmake --build . --config Release

# Run the app if build succeeded
if ($LASTEXITCODE -eq 0) {
    Write-Host "Launching Narrate..."
    & ".\Narrate_artefacts\Release\Standalone\Narrate.exe"
    # Return to the Narrate root folder
    Set-Location $ScriptDir
} else {
    Write-Host "Build failed!"
    Set-Location $ScriptDir
    exit 1
}
