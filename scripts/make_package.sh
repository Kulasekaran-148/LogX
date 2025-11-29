#!/bin/bash

# make_package.sh
# Builds the project using CMake and generates a .deb package with CPack.

set -euo pipefail  # Exit on error, unset variables are errors, fail on pipe errors

PROJECT_NAME="LogX"
BUILD_DIR="../build"

echo "🧹 Cleaning previous build..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "📦 Configuring project with CMake..."
cmake -S . -B "$BUILD_DIR"

echo "🔨 Building project..."
cmake --build "$BUILD_DIR"

echo "🗜️  Generating Debian package..."
cd "$BUILD_DIR"
cpack -G DEB

echo "✅ Package generation completed successfully!"

