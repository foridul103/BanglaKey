#!/usr/bin/env bash
set -euo pipefail

echo "=========================================================="
echo " BanglaKey Linux — Dependency Installer for Fedora Linux"
echo "=========================================================="

PACKAGES=(
    gcc-c++
    cmake
    ninja-build
    qt6-qtbase-devel
    qt6-qtdeclarative-devel
    fcitx5-devel
    extra-cmake-modules
    gtest-devel
    rpm-build
)

echo "The following dependencies will be installed:"
for pkg in "${PACKAGES[@]}"; do
    echo "  - $pkg"
done
echo ""

echo "Running: sudo dnf install -y ${PACKAGES[*]}"
sudo dnf install -y "${PACKAGES[@]}"

echo ""
echo "All dependencies successfully installed!"
