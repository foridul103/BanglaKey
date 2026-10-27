#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
VERSION="0.1.0"
PACKAGE_NAME="banglakey-v${VERSION}-linux-x86_64"
OUTPUT_DIR="${ROOT_DIR}/release"
STAGING_DIR="${OUTPUT_DIR}/${PACKAGE_NAME}"

echo "=========================================================="
echo " BanglaKey Linux — Release Packaging (v${VERSION})"
echo "=========================================================="

mkdir -p "${OUTPUT_DIR}"
rm -rf "${STAGING_DIR}"
mkdir -p "${STAGING_DIR}"

BUILD_DIR="${ROOT_DIR}/build-release"

# Build inside localhost/banglakey-dev container if available (contains all fcitx5-devel & qt6-devel)
if command -v podman &>/dev/null && podman image exists localhost/banglakey-dev; then
    echo "==> Building in Fedora build container (localhost/banglakey-dev)..."
    podman run --rm -v "${ROOT_DIR}:/workspace:Z" -w /workspace localhost/banglakey-dev bash -c "
        set -euo pipefail
        cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DBANGLAKEY_INSTALL=ON
        cmake --build build-release
        ctest --test-dir build-release --output-on-failure
        DESTDIR=/workspace/release/staging cmake --install build-release --prefix /usr
    "
else
    echo "==> Building on local host..."
    cmake -B "${BUILD_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=Release -DBANGLAKEY_INSTALL=ON
    cmake --build "${BUILD_DIR}"
    ctest --test-dir "${BUILD_DIR}" --output-on-failure
    DESTDIR="${OUTPUT_DIR}/staging" cmake --install "${BUILD_DIR}" --prefix /usr
fi

echo "==> Assembling release bundle in ${STAGING_DIR}..."

# Populate staging directory
mkdir -p "${STAGING_DIR}/bin"
mkdir -p "${STAGING_DIR}/lib/fcitx5"
mkdir -p "${STAGING_DIR}/share/fcitx5/addon"
mkdir -p "${STAGING_DIR}/share/fcitx5/inputmethod"
mkdir -p "${STAGING_DIR}/share/applications"
mkdir -p "${STAGING_DIR}/share/icons/hicolor"

cp -a "${OUTPUT_DIR}/staging/usr/bin/banglakey-settings" "${STAGING_DIR}/bin/"
cp -a "${OUTPUT_DIR}/staging/usr/lib64/fcitx5/libbanglakey-fcitx5.so" "${STAGING_DIR}/lib/fcitx5/banglakey-fcitx5.so"
cp -a "${OUTPUT_DIR}/staging/usr/share/fcitx5/addon/banglakey.conf" "${STAGING_DIR}/share/fcitx5/addon/"
cp -a "${OUTPUT_DIR}/staging/usr/share/fcitx5/inputmethod/banglakey.conf" "${STAGING_DIR}/share/fcitx5/inputmethod/"
cp -a "${OUTPUT_DIR}/staging/usr/share/applications/org.banglakey.BanglaKey.desktop" "${STAGING_DIR}/share/applications/"
cp -a "${OUTPUT_DIR}/staging/usr/share/icons/hicolor/"* "${STAGING_DIR}/share/icons/hicolor/"
cp -a "${ROOT_DIR}/README.md" "${STAGING_DIR}/"
cp -a "${ROOT_DIR}/LICENSE" "${STAGING_DIR}/"

# Create installer script
cat << 'EOF' > "${STAGING_DIR}/install.sh"
#!/usr/bin/env bash
set -e

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root: sudo ./install.sh"
    exit 1
fi

PREFIX="${PREFIX:-/usr}"
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "============================================="
echo " Installing BanglaKey Linux..."
echo "============================================="

# Detect libdir (lib64 for Fedora/RHEL, lib for Debian/Ubuntu/Arch)
LIBDIR="${PREFIX}/lib"
if [ -d "${PREFIX}/lib64" ]; then
    LIBDIR="${PREFIX}/lib64"
fi

install -m 755 -d "${PREFIX}/bin"
install -m 755 -d "${LIBDIR}/fcitx5"
install -m 755 -d "${PREFIX}/share/fcitx5/addon"
install -m 755 -d "${PREFIX}/share/fcitx5/inputmethod"
install -m 755 -d "${PREFIX}/share/applications"

# Copy binary
install -m 755 "${BASE_DIR}/bin/banglakey-settings" "${PREFIX}/bin/"

# Copy fcitx5 addon
install -m 755 "${BASE_DIR}/lib/fcitx5/banglakey-fcitx5.so" "${LIBDIR}/fcitx5/"
install -m 644 "${BASE_DIR}/share/fcitx5/addon/banglakey.conf" "${PREFIX}/share/fcitx5/addon/"
install -m 644 "${BASE_DIR}/share/fcitx5/inputmethod/banglakey.conf" "${PREFIX}/share/fcitx5/inputmethod/"

# Copy desktop and icons
install -m 644 "${BASE_DIR}/share/applications/org.banglakey.BanglaKey.desktop" "${PREFIX}/share/applications/"
cp -a "${BASE_DIR}/share/icons/hicolor/"* "${PREFIX}/share/icons/hicolor/" 2>/dev/null || true

# Update desktop and icon databases
gtk-update-icon-cache "${PREFIX}/share/icons/hicolor" 2>/dev/null || true
update-desktop-database "${PREFIX}/share/applications" 2>/dev/null || true

echo "============================================="
echo " BanglaKey installed successfully!"
echo " Restarting Fcitx5..."
echo "============================================="
if command -v fcitx5-remote &>/dev/null; then
    fcitx5-remote -r || true
fi
echo "Done! You can now start 'banglakey-settings' or select BanglaKey in Fcitx5."
EOF
chmod +x "${STAGING_DIR}/install.sh"

# Create uninstaller script
cat << 'EOF' > "${STAGING_DIR}/uninstall.sh"
#!/usr/bin/env bash
set -e

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root: sudo ./uninstall.sh"
    exit 1
fi

PREFIX="${PREFIX:-/usr}"
LIBDIR="${PREFIX}/lib"
if [ -d "${PREFIX}/lib64" ]; then
    LIBDIR="${PREFIX}/lib64"
fi

echo "Uninstalling BanglaKey Linux..."
rm -f "${PREFIX}/bin/banglakey-settings"
rm -f "${LIBDIR}/fcitx5/banglakey-fcitx5.so"
rm -f "${PREFIX}/share/fcitx5/addon/banglakey.conf"
rm -f "${PREFIX}/share/fcitx5/inputmethod/banglakey.conf"
rm -f "${PREFIX}/share/applications/org.banglakey.BanglaKey.desktop"

gtk-update-icon-cache "${PREFIX}/share/icons/hicolor" 2>/dev/null || true
update-desktop-database "${PREFIX}/share/applications" 2>/dev/null || true

echo "BanglaKey uninstalled."
EOF
chmod +x "${STAGING_DIR}/uninstall.sh"

# Create archives
echo "==> Creating tarball archives..."
cd "${OUTPUT_DIR}"
tar -czvf "${PACKAGE_NAME}.tar.gz" "${PACKAGE_NAME}"
tar -cjvf "${PACKAGE_NAME}.tar.xz" "${PACKAGE_NAME}" 2>/dev/null || tar -cJvf "${PACKAGE_NAME}.tar.xz" "${PACKAGE_NAME}"

# Calculate checksums
sha256sum "${PACKAGE_NAME}.tar.gz" > "${PACKAGE_NAME}.tar.gz.sha256"
sha256sum "${PACKAGE_NAME}.tar.xz" > "${PACKAGE_NAME}.tar.xz.sha256"

# Cleanup staging
rm -rf "${OUTPUT_DIR}/staging"

echo "=========================================================="
echo " Release build complete!"
echo " Artifacts created in ${OUTPUT_DIR}:"
ls -lh "${OUTPUT_DIR}/${PACKAGE_NAME}.tar."* "${OUTPUT_DIR}/${PACKAGE_NAME}.tar."*.sha256
echo "=========================================================="
