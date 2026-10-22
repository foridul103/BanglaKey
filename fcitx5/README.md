# BanglaKey Linux — Fcitx5 Addon Integration

This directory contains the Fcitx5 input method addon integration for BanglaKey Linux.

---

## 1. Architectural Philosophy

1. **Native Fcitx5 Citizen**:
   - Integrates directly with the official Fcitx5 C++ SDK (`Fcitx5Core`, `Fcitx5Config`, `Fcitx5Utils`).
   - Uses `fcitx::InputMethodEngineV2` and `fcitx::InputContext` event pipelines.
   - Zero synthetic keystrokes (`xdotool`, `ydotool`, `wtype`) or clipboard injection hacks.

2. **Unified Single Input Method Entry**:
   - Registers exactly one user-facing input method: `BanglaKey` (`বাংলাকি`).
   - Both **Phonetic** and **Bijoy-compatible** layouts reside inside the same unified input method.
   - The user selects `BanglaKey` in their desktop settings and seamlessly switches typing modes using `F12` or `Ctrl+Alt+M`.

3. **Decoupled Engine Core**:
   - The addon delegates all key parsing, state machines, conjunct composition, and transliteration to the shared core C++ engine (`banglakey_engine`).
   - The addon remains strictly responsible for Wayland / X11 input method protocol handling, preedit display, candidate list presentation, and text commitment.

4. **Zero User Keystroke Logging**:
   - Follows strict privacy guidelines. Only high-level lifecycle events and mode switches are logged. User keystrokes and typed Bengali strings are never logged to disk or console.

---

## 2. Directory Structure

```text
fcitx5/
├── CMakeLists.txt              # Addon build definition & installation targets
├── banglakey.conf.in           # Fcitx5 addon metadata (installed to /usr/share/fcitx5/addon/)
├── banglakey-addon.conf.in     # Input method entry config (installed to /usr/share/fcitx5/inputmethod/)
├── src/
│   ├── banglakey_addon.hpp     # BanglaKeyAddon class & context property definitions
│   └── banglakey_addon.cpp     # Event loop, candidate word selection & mode toggles
└── README.md                   # This documentation
```

---

## 3. Build & Installation

The addon is built when `BANGLAKEY_BUILD_FCITX5=ON` and `Fcitx5Core` headers are detected:

```bash
cmake --preset default
cmake --build --preset default
```

Installation paths:
- Addon module: `${CMAKE_INSTALL_LIBDIR}/fcitx5/libbanglakey-fcitx5.so`
- Addon descriptor: `${CMAKE_INSTALL_DATADIR}/fcitx5/addon/banglakey.conf`
- Input method descriptor: `${CMAKE_INSTALL_DATADIR}/fcitx5/inputmethod/banglakey.conf`
