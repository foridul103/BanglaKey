#!/usr/bin/env bash
set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "=========================================================="
echo " BanglaKey Linux — Dependency Inspection"
echo "=========================================================="
echo ""

echo "--- 1. Operating System & Desktop Environment ---"
if [ -f /etc/os-release ]; then
    # shellcheck disable=SC1091
    source /etc/os-release
    echo "OS:                ${PRETTY_NAME:-Linux}"
else
    echo "OS:                Linux (unknown distribution)"
fi
echo "Kernel:            $(uname -r)"
echo "Session Type:      ${XDG_SESSION_TYPE:-unknown}"
echo "Current Desktop:   ${XDG_CURRENT_DESKTOP:-unknown}"
echo "Qt QPA Platform:   ${QT_QPA_PLATFORM:-default}"
echo ""

echo "--- 2. Core Build Tools (Host System) ---"
MISSING_HOST_TOOLS=()

check_cmd() {
    local cmd="$1"
    local desc="$2"
    if command -v "${cmd}" >/dev/null 2>&1; then
        local ver
        ver=$("${cmd}" --version 2>&1 | head -n 1)
        printf "  %-14s: [FOUND] %s\n" "${cmd}" "${ver}"
    else
        printf "  %-14s: [MISSING] (%s)\n" "${cmd}" "${desc}"
        MISSING_HOST_TOOLS+=("${cmd}")
    fi
}

check_cmd "g++" "C++20 Compiler (gcc-c++)"
check_cmd "cmake" "CMake Build System (>= 3.25)"
check_cmd "ninja" "Ninja Build Accelerator"
check_cmd "git" "Version Control"
check_cmd "pkg-config" "Package Config Tool"
check_cmd "ctest" "CTest Test Runner"
check_cmd "rpmbuild" "Fedora RPM Builder"
echo ""

echo "--- 3. Qt 6 & Fcitx5 Libraries (Host System) ---"
MISSING_HOST_PKGS=()

check_pkg() {
    local pkg="$1"
    local desc="$2"
    if pkg-config --exists "${pkg}" >/dev/null 2>&1; then
        local ver
        ver=$(pkg-config --modversion "${pkg}")
        printf "  %-16s: [FOUND] v%s\n" "${pkg}" "${ver}"
    else
        printf "  %-16s: [MISSING] (%s)\n" "${pkg}" "${desc}"
        MISSING_HOST_PKGS+=("${pkg}")
    fi
}

check_pkg "Qt6Core" "Qt 6 Base (qt6-qtbase-devel)"
check_pkg "Qt6Quick" "Qt 6 Declarative (qt6-qtdeclarative-devel)"
check_pkg "Qt6Qml" "Qt 6 QML Engine"
check_pkg "Fcitx5Core" "Fcitx5 SDK Core (fcitx5-devel)"
check_pkg "Fcitx5Config" "Fcitx5 Configuration Library"
echo ""

echo "--- 4. Container Development Environment ---"
HAS_CONTAINER_ENV=false
if command -v podman >/dev/null 2>&1; then
    echo "  Podman:        [FOUND] $(podman --version)"
    if podman image exists banglakey-dev 2>/dev/null; then
        echo "  banglakey-dev: [FOUND] Ready for containerized builds"
        HAS_CONTAINER_ENV=true
    else
        echo "  banglakey-dev: [NOT BUILT] (build with: podman build -t banglakey-dev -f packaging/container/Dockerfile.dev .)"
    fi
else
    echo "  Podman:        [NOT INSTALLED]"
fi
echo ""

echo "--- 5. Summary & Recommendation ---"
if [ ${#MISSING_HOST_TOOLS[@]} -eq 0 ] && [ ${#MISSING_HOST_PKGS[@]} -eq 0 ]; then
    echo "Status: All dependencies are satisfied directly on host!"
    exit 0
elif [ "${HAS_CONTAINER_ENV}" = true ]; then
    echo "Status: Host has missing packages, but 'banglakey-dev' container is ready."
    echo "Development commands (build, configure, test) can execute seamlessly via container."
    echo ""
    echo "To install all dependencies directly onto Fedora host:"
    echo "  sudo dnf install -y gcc-c++ cmake ninja-build git pkg-config extra-cmake-modules fcitx5-devel fcitx5 fcitx5-configtool gtest-devel qt6-qtbase-devel qt6-qtdeclarative-devel"
    exit 0
else
    echo "Status: Missing dependencies detected."
    echo ""
    echo "To install on Fedora Linux:"
    echo "  sudo dnf install -y gcc-c++ cmake ninja-build git pkg-config extra-cmake-modules fcitx5-devel fcitx5 fcitx5-configtool gtest-devel qt6-qtbase-devel qt6-qtdeclarative-devel"
    echo ""
    echo "Or build the rootless dev container:"
    echo "  podman build -t banglakey-dev -f packaging/container/Dockerfile.dev ."
    exit 1
fi
