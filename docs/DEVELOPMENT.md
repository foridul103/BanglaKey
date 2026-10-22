# BanglaKey Linux — Developer Guide

This document describes how to set up the development environment, configure, compile, test, and troubleshoot **BanglaKey Linux** on Fedora Linux (KDE Plasma 6, Wayland & X11).

---

## 1. System Requirements & Development Dependencies

BanglaKey Linux targets Fedora Linux with modern C++ tooling.

### Required Packages (Fedora 44)

To install all development tools and dependencies natively:

```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    git \
    pkg-config \
    extra-cmake-modules \
    fcitx5-devel \
    fcitx5 \
    fcitx5-configtool \
    gtest-devel \
    qt6-qtbase-devel \
    qt6-qtdeclarative-devel
```

### Containerized Development (Podman)

If you prefer not to install development packages directly on the host system, a rootless development container is supported via Podman:

```bash
# Build the dev container
podman build -t banglakey-dev -f packaging/container/Dockerfile.dev .

# Run build inside container
podman run --rm -v "$PWD:/workspace:Z" -w /workspace banglakey-dev bash -c "cmake --preset default && cmake --build --preset default && ctest --test-dir build --output-on-failure"
```

---

## 2. Project Build Presets

BanglaKey Linux uses CMake presets defined in [CMakePresets.json](file:///run/media/foredulhaque/Main%20Vault/Development/BanglaKey/CMakePresets.json):

| Preset | Build Type | Components Built |
| :--- | :--- | :--- |
| `default` | Debug | All components (`engine`, `fcitx5`, `desktop`, `tests`) |
| `release` | Release | All components with optimizations |
| `engine-only` | Debug | Core C++ engine library and unit tests only |

---

## 3. Build & Test Commands

### Using Project Helper Scripts

Convenient shell scripts are provided in `scripts/`:

```bash
# Check dependencies and environment status
./scripts/check-dependencies.sh

# Configure the build directory
./scripts/configure.sh

# Build all configured targets
./scripts/build.sh

# Run all automated tests
./scripts/test.sh
```

### Using CMake and CTest Directly

```bash
# Configure with default preset (Ninja + Debug)
cmake --preset default

# Build all targets
cmake --build --preset default

# Run all test suites with verbose output on failure
ctest --test-dir build --output-on-failure
```

---

## 4. Component Build Flags

Component targets are modular and can be enabled or disabled:

- `BANGLAKEY_BUILD_ENGINE`: Builds `libbanglakey_engine.a` (default: `ON`)
- `BANGLAKEY_BUILD_FCITX5`: Builds `libbanglakey-fcitx5.so` (default: `ON`, requires `fcitx5-devel`)
- `BANGLAKEY_BUILD_DESKTOP`: Builds `banglakey-settings` (default: `ON`, requires `qt6-qtbase-devel`, `qt6-qtdeclarative-devel`)
- `BANGLAKEY_BUILD_TESTS`: Builds unit & integration test suites (default: `ON`)
- `BANGLAKEY_INSTALL`: Generates standard install targets (default: `ON`)

---

## 5. Coding Standards & Guidelines

- **C++ Standard**: C++20 standard (`-std=c++20`).
- **Memory Safety**: RAII strictly observed; raw `new`/`delete` prohibited.
- **Pure Core Engine**: The core engine (`engine/`) must never depend on Qt, GUI libraries, or Fcitx5.
- **Privacy**: No arbitrary user-typed text or keystrokes may be logged.
- **Deterministic Testing**: All unit tests must execute headless in CI environments.
