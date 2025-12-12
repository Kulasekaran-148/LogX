#!/bin/bash

# make_package.sh
# Builds the project using CMake and generates a .deb package with CPack.

set -euo pipefail  # Exit on error, unset variables are errors, fail on pipe errors

PROJECT_NAME="LogX"
BUILD_TYPE="${1:-Debug}"   # Default=Debug
BUILD_DIR="build"

echo "📦 Configuring project with CMake (BUILD_TYPE = $BUILD_TYPE)"
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

echo "🔨 Building project..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

echo "🗜️  Generating Debian package..."
cd "$BUILD_DIR"
cpack -G DEB

echo "✅ Package generation completed successfully!"

