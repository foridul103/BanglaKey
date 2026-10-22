#!/usr/bin/env bash
set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${ROOT_DIR}/build"
BUILD_TYPE="${1:-Debug}"

echo "=========================================================="
echo " BanglaKey Linux — Configure"
echo " Build Type: ${BUILD_TYPE}"
echo " Source Dir: ${ROOT_DIR}"
echo " Build Dir:  ${BUILD_DIR}"
echo "=========================================================="

mkdir -p "${BUILD_DIR}"

if command -v cmake >/dev/null 2>&1; then
    GENERATOR="Unix Makefiles"
    if command -v ninja >/dev/null 2>&1; then
        GENERATOR="Ninja"
    fi
    echo "Configuring natively using host CMake (${GENERATOR})..."
    cmake -B "${BUILD_DIR}" \
        -S "${ROOT_DIR}" \
        -G "${GENERATOR}" \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DBANGLAKEY_BUILD_ENGINE=ON \
        -DBANGLAKEY_BUILD_FCITX5=ON \
        -DBANGLAKEY_BUILD_DESKTOP=ON \
        -DBANGLAKEY_BUILD_TESTS=ON
elif command -v podman >/dev/null 2>&1 && podman image exists banglakey-dev 2>/dev/null; then
    echo "Host CMake missing. Configuring inside 'banglakey-dev' Podman container (Ninja)..."
    podman run --rm \
        -v "${ROOT_DIR}:/workspace:Z" \
        -w /workspace \
        banglakey-dev \
        cmake -B /workspace/build \
            -S /workspace \
            -G Ninja \
            -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DBANGLAKEY_BUILD_ENGINE=ON \
            -DBANGLAKEY_BUILD_FCITX5=ON \
            -DBANGLAKEY_BUILD_DESKTOP=ON \
            -DBANGLAKEY_BUILD_TESTS=ON
else
    echo "ERROR: Neither host CMake nor 'banglakey-dev' Podman image is available!"
    echo "Install tools natively (sudo dnf install -y cmake ninja-build gcc-c++)"
    echo "Or build the dev container (podman build -t banglakey-dev -f packaging/container/Dockerfile.dev .)"
    exit 1
fi

echo "Configuration completed successfully!"
