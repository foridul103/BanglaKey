# BanglaKey Linux

Native Bengali input system for **Fedora Linux 44** (KDE Plasma 6, Wayland & X11).

BanglaKey provides a unified, first-class Bengali typing experience on Linux, integrating **Bijoy-compatible layout** typing and **Phonetic Bengali** typing inside a single user-facing product.

---

## Highlights

- **Native Linux Architecture**: Zero Electron, zero Flutter, zero mandatory WebView runtime. Built with modern C++, Qt 6, and Fcitx5.
- **Two Typing Modes in One Product**:
  1. **Bijoy Layout**: Familiar classic layout mapping with link key (`g`) conjunct (*Juktakkhor*) processing and vowel reordering.
  2. **Phonetic Bengali**: Natural English-to-Bengali phonetic typing with smart contextual lookahead and candidate prediction.
- **Robust Fcitx5 Integration**: Integrates directly into Fedora's modern input framework via `fcitx::InputMethodEngineV2`. Works across Wayland compositors (KWin) and X11 without fake keystroke hacks or clipboard injection.
- **Decoupled Desktop Settings**: Elegant Qt 6 / QML settings center styled for KDE Plasma 6. The background typing engine is completely independent and never relies on the GUI process remaining open.
- **Independent Modular Builds**: Engine, Fcitx5 addon, Desktop app, and Test harness can be configured and built independently.
- **RPM Packaging**: Native RPM spec file tailored for Fedora 44 repository standards.

---

## Repository Structure

```
banglakey-linux/
├── AGENTS.md                 # Permanent coding and architecture guidelines
├── CMakeLists.txt            # Root build orchestration
├── CMakePresets.json         # Standardized CMake configure and build presets
├── README.md                 # Project documentation
├── docs/
│   └── ARCHITECTURE.md       # Comprehensive system architecture specification
├── engine/                   # Pure C++ core typing engine
│   ├── CMakeLists.txt
│   ├── include/banglakey/    # Public engine headers
│   │   ├── banglakey.hpp
│   │   ├── bijoy_mode.hpp
│   │   ├── engine.hpp
│   │   ├── phonetic_mode.hpp
│   │   └── types.hpp
│   ├── rules/                # Mapping rules and transliteration tables
│   │   ├── bijoy_map.hpp
│   │   └── phonetic_map.hpp
│   └── src/                  # Engine implementations
│       ├── bijoy_mode.cpp
│       ├── engine.cpp
│       └── phonetic_mode.cpp
├── fcitx5/                   # Native Fcitx5 input method addon
│   ├── CMakeLists.txt
│   ├── banglakey.conf.in     # Fcitx5 addon registration metadata
│   ├── banglakey-addon.conf.in # Fcitx5 input method entry configuration
│   └── src/
│       ├── banglakey_addon.cpp
│       └── banglakey_addon.hpp
├── desktop/                  # Qt 6 / QML settings & management application
│   ├── CMakeLists.txt
│   ├── qml/
│   │   └── Main.qml          # Modern KDE Plasma-styled UI
│   └── src/
│       ├── main.cpp
│       ├── settings_backend.cpp
│       └── settings_backend.hpp
├── tests/                    # CTest & GoogleTest test suites
│   ├── CMakeLists.txt
│   └── unit/
│       └── test_smoke.cpp    # Engine initialization and mode-switching smoke tests
├── packaging/
│   └── rpm/
│       └── banglakey.spec    # Fedora RPM package specification
└── scripts/
    ├── build.sh              # Build automation script
    └── install_deps.sh       # Dependency installer for Fedora
```

---

## Prerequisites (Fedora 44)

Install the necessary development tools and libraries:

```bash
sudo dnf install -y \
  gcc-c++ \
  cmake \
  ninja-build \
  qt6-qtbase-devel \
  qt6-qtdeclarative-devel \
  fcitx5-devel \
  extra-cmake-modules \
  gtest-devel \
  rpm-build
```

---

## Building

### Standard Build (All Components)
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Modular Build Options
- `-DBANGLAKEY_BUILD_ENGINE=ON|OFF` (default: `ON`)
- `-DBANGLAKEY_BUILD_FCITX5=ON|OFF` (default: `ON`)
- `-DBANGLAKEY_BUILD_DESKTOP=ON|OFF` (default: `ON`)
- `-DBANGLAKEY_BUILD_TESTS=ON|OFF` (default: `ON`)

Example building engine and tests only:
```bash
cmake -B build -G Ninja -DBANGLAKEY_BUILD_FCITX5=OFF -DBANGLAKEY_BUILD_DESKTOP=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## Phase Status

- [x] **Phase 1: Project Foundation**
  - Project architecture, AGENTS.md, docs/ARCHITECTURE.md.
  - Modular CMake infrastructure with independent build options.
  - Pure C++ typing engine with Bijoy and Phonetic mode foundations.
  - Fcitx5 input method addon skeleton adhering to `fcitx::InputMethodEngineV2`.
  - Modern Qt 6 / QML settings application skeleton.
  - Automated smoke test suite.
  - Fedora RPM spec file.
- [ ] **Phase 2: Core Engine & Phonetic Processing**
- [ ] **Phase 3: Bijoy Layout & Juktakkhor Parser**
- [ ] **Phase 4: Fcitx5 Integration & Wayland Preedit**
- [ ] **Phase 5: Desktop Settings App & RPM Packaging**
