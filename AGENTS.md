# BanglaKey Linux — Agent & Engineering Guidelines

This document establishes permanent architectural rules, design standards, and testing policies for all human and AI contributors to the **BanglaKey Linux** project.

---

## 1. Project Philosophy & Architecture Constraints

1. **Native Linux Citizen**:
   - Built natively for Fedora Linux (KDE Plasma 6, Wayland & X11).
   - No Electron, no Flutter, no mandatory WebView runtimes.
   - Built using modern C++ (C++17 minimum, C++20 preferred where SDK allows), Qt 6 / QML, and Fcitx5.

2. **Single Shared Core Engine**:
   - The core typing engine (`banglakey_engine`) is an independent, pure C++ library with zero GUI or Fcitx5 dependencies.
   - Both **Bijoy-compatible** and **Phonetic** modes reside inside this single engine as selectable operational modes.
   - All state tracking, conjunct (*Juktakkhor*) resolution, and character mapping happen strictly within `banglakey_engine`.

3. **Decoupled Architecture**:
   - The **Fcitx5 Addon** (`banglakey-fcitx5`) interacts exclusively with `banglakey_engine` and the Fcitx5 SDK.
   - The **Desktop Settings Application** (`banglakey-settings`) is a Qt 6 / QML application for configuring preferences, keybindings, and dictionary settings.
   - **Crucial Rule**: The Qt application MUST NEVER be required for typing to function. If the desktop app is closed, crashed, or uninstalled, typing via Fcitx5 MUST continue uninterrupted.

4. **No Synthetic Keystroke / Clipboard Hacks**:
   - Never implement fake system-wide typing using `xdotool`, `ydotool`, `wtype`, or clipboard injection.
   - All system-wide input handling MUST go through the Fcitx5 input method event pipeline (`fcitx::InputMethodEngineV2` and `fcitx::InputContext`).
   - Use documented Fcitx5 C++ APIs. Never invent API signatures or rely on deprecated Fcitx 4 conventions.

5. **Wayland First, X11 Compatible**:
   - All input method logic must operate cleanly under Wayland compositors (KWin on KDE Plasma 6) via standard input-method protocols handled by Fcitx5.
   - Client windows, popups, and preedit text rendering must adhere to Wayland constraints.

---

## 2. Coding & Implementation Standards

### Modern C++ Standards
- Target **C++17** or **C++20**.
- Follow RAII strictly. Do not manage raw heap pointers (`new`/`delete`); use `std::unique_ptr` and `std::shared_ptr`.
- Explicitly declare ownership and move semantics.
- Mark single-argument constructors `explicit`.
- Mark non-throwing functions `noexcept`.
- Use `const` correctness rigorously across methods and parameters.
- Standard headers must precede third-party and local headers:
  ```cpp
  #include <string>
  #include <vector>
  #include <memory>

  #include <fcitx/inputmethodengine.h>

  #include <banglakey/engine.hpp>
  ```

### Unicode & Bengali Processing
- All internal strings are UTF-8 encoded `std::string` or `std::string_view`.
- Handle Bengali Unicode specifics accurately:
  - Halant / Hasanta (`্`, U+09CD) for conjunct formation.
  - Zero Width Joiner (ZWJ, U+200D) and Zero Width Non-Joiner (ZWNJ, U+200C) where required for Khanda Ta (`ৎ`), reph, or explicit hasanta display.
  - Vowel signs (*Kar*): Post-consonant in Unicode standard representation, correctly mapped from pre-consonant typing sequences where applicable (e.g., E-kar `ে` typed before consonant in Bijoy layout).

### Qt 6 & QML
- Use modern QML declarative syntax.
- Follow KDE Plasma 6 design aesthetics (Breeze Dark / Light harmonious palettes, fluid micro-interactions, responsive card layouts).
- Business logic belongs in C++ (`QObject` subclasses); QML is strictly for presentation and user interaction.
- Expose properties using `Q_PROPERTY` with explicit `READ`, `WRITE`, and `NOTIFY` signals.

---

## 3. Build & Packaging Rules

- **Build Systems**: CMake (>= 3.25) and Ninja.
- **Modularity**:
  - `BANGLAKEY_BUILD_ENGINE`: Builds the core C++ engine.
  - `BANGLAKEY_BUILD_FCITX5`: Builds the Fcitx5 input method addon.
  - `BANGLAKEY_BUILD_DESKTOP`: Builds the Qt6/QML desktop settings GUI.
  - `BANGLAKEY_BUILD_TESTS`: Builds unit and integration tests.
  - Each component must be capable of building independently when required dependencies are satisfied.
- **Packaging**: Standard Fedora RPM spec file in `packaging/rpm/banglakey.spec`.

---

## 4. Testing & Verification Standards

1. **Every Core Feature Requires Unit Tests**:
   - New transliteration rules, key mapping entries, and state transitions in `banglakey_engine` must have automated unit tests in `tests/unit/`.
2. **Smoke & Regression Tests**:
   - CTest and GoogleTest are the primary test harnesses.
   - Tests must execute deterministically without requiring a display server (headless CI friendly).
3. **Phase-Gate Discipline**:
   - Never proceed to next development phases before verifying current targets compile, pass tests, and satisfy architectural boundaries.
