#!/usr/bin/env bash
set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${ROOT_DIR}/build"

echo "=========================================================="
echo " BanglaKey Linux — Test Runner"
echo " Build Dir: ${BUILD_DIR}"
echo "=========================================================="

if [ ! -d "${BUILD_DIR}" ]; then
    echo "ERROR: Build directory '${BUILD_DIR}' does not exist!"
    echo "Run ./scripts/build.sh first."
    exit 1
fi

if command -v ctest >/dev/null 2>&1; then
    echo "Running CTest natively..."
    ctest --test-dir "${BUILD_DIR}" --output-on-failure
elif command -v podman >/dev/null 2>&1 && podman image exists banglakey-dev 2>/dev/null; then
    echo "Running CTest inside 'banglakey-dev' Podman container..."
    podman run --rm \
        -v "${ROOT_DIR}:/workspace:Z" \
        -w /workspace \
        banglakey-dev \
        ctest --test-dir /workspace/build --output-on-failure
else
    echo "ERROR: Neither host CTest nor 'banglakey-dev' Podman image is available!"
    exit 1
fi

echo ""
echo "All tests passed successfully!"
