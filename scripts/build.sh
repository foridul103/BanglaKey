#!/usr/bin/env bash
set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${ROOT_DIR}/build"
BUILD_TYPE="${1:-Debug}"

echo "=========================================================="
echo " BanglaKey Linux — Build"
echo " Build Type: ${BUILD_TYPE}"
echo " Source Dir: ${ROOT_DIR}"
echo " Build Dir:  ${BUILD_DIR}"
echo "=========================================================="

# 1. Ensure configured
if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
    echo "Build directory not configured. Running configure first..."
    "${SCRIPT_DIR}/configure.sh" "${BUILD_TYPE}"
fi

# 2. Build available targets
if command -v cmake >/dev/null 2>&1; then
    echo "Building natively using host CMake..."
    cmake --build "${BUILD_DIR}" --parallel
elif command -v podman >/dev/null 2>&1 && podman image exists banglakey-dev 2>/dev/null; then
    echo "Building inside 'banglakey-dev' Podman container..."
    podman run --rm \
        -v "${ROOT_DIR}:/workspace:Z" \
        -w /workspace \
        banglakey-dev \
        cmake --build /workspace/build --parallel
else
    echo "ERROR: Neither host CMake nor 'banglakey-dev' Podman image is available!"
    exit 1
fi

echo "Build completed successfully!"
