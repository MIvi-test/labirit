# Cross-platform build test script for Windows PowerShell
# Используйте: .\build-test.ps1

$ErrorActionPreference = "Stop"

Write-Host "================================" -ForegroundColor Cyan
Write-Host "Cross-platform Build Test" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

# Detect platform (should be Windows in this case)
$PLATFORM = "windows"
$MAKE_CMD = "mingw32-make"

Write-Host "Detected platform: $PLATFORM" -ForegroundColor Yellow
Write-Host "Using: $MAKE_CMD" -ForegroundColor Yellow
Write-Host ""

# Show platform info
Write-Host "Platform info:" -ForegroundColor Green
& $MAKE_CMD show-platform
Write-Host ""

# Clean
Write-Host "Cleaning..." -ForegroundColor Green
& $MAKE_CMD clean
Write-Host ""

# Compile
Write-Host "Compiling..." -ForegroundColor Green
& $MAKE_CMD compile
if ($LASTEXITCODE -ne 0) {
    Write-Host "Compilation failed!" -ForegroundColor Red
    exit 1
}
Write-Host ""

# Check if executable was created
if (Test-Path "main.exe") {
    Write-Host "Executable created: main.exe" -ForegroundColor Green
    $fileSize = (Get-Item "main.exe").Length / 1MB
    Write-Host "Size: $([Math]::Round($fileSize, 2)) MB"
}
else {
    Write-Host "Failed to create main.exe" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Build successful!" -ForegroundColor Green

# Show available commands
Write-Host ""
Write-Host "Available commands:" -ForegroundColor Cyan
Write-Host "  mingw32-make run              - Run the main program"
Write-Host "  mingw32-make test             - Run tests"
Write-Host "  mingw32-make run-benchmarks   - Run benchmarks"
Write-Host "  mingw32-make show-platform    - Show platform info"
