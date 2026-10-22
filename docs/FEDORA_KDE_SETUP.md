# BanglaKey Linux — Fedora KDE Plasma Setup & Troubleshooting Guide

This guide describes how to configure, run, and troubleshoot **BanglaKey Linux** with **Fcitx5** on **Fedora Linux (KDE Plasma 6, Wayland & X11)**.

---

## 1. System Requirements & Prerequisites

On Fedora Linux, ensure the following core Fcitx5 runtime packages are installed:

```bash
sudo dnf install -y fcitx5 fcitx5-autostart fcitx5-configtool kcm-fcitx5
```

- `fcitx5`: The core input method daemon and event engine.
- `fcitx5-autostart`: Systemd user service / desktop autostart configuration.
- `fcitx5-configtool`: Graphical and CLI configuration utilities.
- `kcm-fcitx5`: KDE Plasma 6 System Settings integration module.

> [!NOTE]
> If `ibus` was previously installed and active, disable or remove it to avoid daemon conflicts:
> ```bash
> gsettings set org.gnome.desktop.interface default-input-sources "[]" 2>/dev/null || true
> ```

---

## 2. Environment Configuration (Wayland & X11)

Fedora KDE Plasma 6 runs on Wayland by default. For seamless text-input protocol support across all client toolkits (Qt 6, GTK 3/4, Chromium/Electron), configure your user environment.

Create or update `~/.config/environment.d/im.conf`:

```ini
GTK_IM_MODULE=fcitx
QT_IM_MODULE=fcitx
XMODIFIERS=@im=fcitx
```

Apply immediately or log out and log back in for changes to take effect across systemd user sessions.

### Wayland Browser & Electron Considerations
For Chromium-based browsers or Electron applications running on Wayland, enable native Wayland text-input integration:
```bash
--enable-features=UseOzonePlatform --ozone-platform=wayland --enable-wayland-ime
```

---

## 3. Building & Installing BanglaKey

### System-Wide Installation (RPM / Direct Install)
From the project root:

```bash
# Configure and build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build

# Install (requires root privileges)
sudo cmake --install build
```

This installs:
- Engine Library: `/usr/lib64/libbanglakey_engine.a`
- Fcitx5 Addon: `/usr/lib64/fcitx5/libbanglakey-fcitx5.so`
- Addon Metadata: `/usr/share/fcitx5/addon/banglakey.conf`
- Input Method Config: `/usr/share/fcitx5/inputmethod/banglakey.conf`

### User-Local Installation (No Root Required)
For development and user-local testing:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build
cmake --install build
```

Ensure Fcitx5 searches user directories:
```bash
export XDG_DATA_DIRS="$HOME/.local/share:$XDG_DATA_DIRS"
export FCITX_ADDON_DIRS="$HOME/.local/lib/fcitx5:$HOME/.local/lib64/fcitx5"
```

---

## 4. Enabling BanglaKey in KDE Plasma 6

1. Open **System Settings** (`systemsettings`).
2. Navigate to **Input Devices** → **Virtual Keyboard** (or **Input Method**).
3. Verify that **Fcitx 5** is selected as the active input method system.
4. Click **Configure...** or launch `fcitx5-configtool`.
5. Under **Current Input Method**, click **Add...**.
6. Deselect "Only Show Current Language".
7. Search for **BanglaKey** (বাংলাকি) and click **Add**.
8. Apply settings.

---

## 5. Usage & Keybindings

| Function | Default Key | Description |
| :--- | :--- | :--- |
| **Activate BanglaKey** | `Ctrl + Space` / `Super + Space` | Standard Fcitx5 toggle between English and Bengali |
| **Switch Typing Mode** | `F12` or `Ctrl + Alt + M` | Toggles between **Phonetic** and **Bijoy-compatible** modes |
| **Select Candidate** | `1` .. `9` or Mouse Click | Commits the candidate from the suggestion popup |
| **Navigate Candidates** | `Up` / `Down` | Cycles candidate selection |
| **Commit Selection** | `Enter` / `Return` | Commits selected word without trailing space |
| **Word Commit** | `Space` | Commits current word with space |
| **Cancel Composition** | `Escape` | Cancels current preedit without committing |
| **Bypass / Shortcuts** | `Ctrl+C`, `Ctrl+V`, `Alt+F4`, etc. | Flushes preedit and forwards shortcut unaltered to application |

### Visual Feedback
- **Tray & Panel Indicator**: Displays `বাং (ফোনেটিক)` in Phonetic mode and `বাং (বিজয়)` in Bijoy mode.
- **On-Screen Popup**: Displays a temporary notification ("BanglaKey: Phonetic Mode" / "BanglaKey: Bijoy Mode") when toggling modes.

---

## 6. Diagnostic Logging & Privacy Guarantee

BanglaKey adheres strictly to privacy standards:
- **No Keystroke Logging**: Keystrokes, raw keycodes, and user-typed Bengali text are NEVER written to logs or disk.
- **Diagnostic Events**: Only mode switches, addon registration, and input context lifecycles are logged at `FCITX_INFO` / `FCITX_DEBUG` levels.

To inspect Fcitx5 diagnostic logs in real time:
```bash
# Restart fcitx5 in debug mode
fcitx5 -r --verbose *=5
```

Expected startup messages confirming proper discovery:
```
I[...] banglakey_addon.cpp:44] BanglaKey: Addon initialized with Phonetic mode as default.
I[...] addonmanager.cpp:208] Loaded addon banglakey
I[...] inputmethodmanager.cpp:213] Found 1 input method(s) in addon banglakey
```

---

## 7. Troubleshooting

### Issue 1: BanglaKey does not appear in Fcitx5 Configuration Tool
**Cause**: Metadata files are not in `$XDG_DATA_DIRS/fcitx5/`.
**Fix**:
1. Check that `/usr/share/fcitx5/addon/banglakey.conf` and `/usr/share/fcitx5/inputmethod/banglakey.conf` exist.
2. Restart Fcitx5: `fcitx5 -r`.

### Issue 2: Preedit does not appear inline in Wayland applications
**Cause**: The application is running via XWayland or lacks text-input protocol support.
**Fix**:
1. Ensure `kcm-fcitx5` and Wayland text-input-v2/v3 are enabled in KDE Plasma.
2. Verify `QT_IM_MODULE=fcitx` and `GTK_IM_MODULE=fcitx` in `~/.config/environment.d/im.conf`.
3. BanglaKey simultaneously updates both client inline preedit and the floating input panel so text is visible even if the client application does not support inline preedit.

### Issue 3: Mode switch shortcut does not toggle mode
**Cause**: `F12` may be captured by a drop-down terminal (e.g. Yakuake).
**Fix**:
- Use `Ctrl + Alt + M` as the alternative mode switch hotkey.
- Or reconfigure Yakuake's toggle shortcut in **System Settings** → **Shortcuts**.

### Issue 4: Text lost when switching applications
**Cause**: Input context deactivation.
**Fix**: BanglaKey automatically isolates composition state per window context. When focus leaves a window, active compositions are safely preserved or committed according to context reset policies.
