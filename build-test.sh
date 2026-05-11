#!/bin/bash
# Cross-platform build test script
# Этот скрипт можно использовать на Linux/macOS для проверки сборки

set -e  # Exit on error

echo "================================"
echo "Cross-platform Build Test"
echo "================================"
echo ""

# Detect platform
if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    PLATFORM="windows"
    MAKE_CMD="mingw32-make"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
    MAKE_CMD="make"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
    MAKE_CMD="make"
else
    PLATFORM="unknown"
    MAKE_CMD="make"
fi

echo "Detected platform: $PLATFORM"
echo "Using: $MAKE_CMD"
echo ""

# Show platform info
echo "Platform info:"
$MAKE_CMD show-platform
echo ""

# Clean
echo "Cleaning..."
$MAKE_CMD clean
echo ""

# Compile
echo "Compiling..."
$MAKE_CMD compile
echo ""

# Check if executable was created
if [[ "$PLATFORM" == "windows" ]]; then
    if [ -f "main.exe" ]; then
        echo "✓ Executable created: main.exe"
        ls -lh main.exe
    else
        echo "✗ Failed to create main.exe"
        exit 1
    fi
else
    if [ -f "main" ]; then
        echo "✓ Executable created: main"
        ls -lh main
    else
        echo "✗ Failed to create main"
        exit 1
    fi
fi

echo ""
echo "Build successful!"
