# BanglaKey Linux — Architecture Specification

This document defines the high-level architecture, module decomposition, data flows, and subsystem specifications for **BanglaKey Linux**.

---

## 1. System Topology & Decomposition

BanglaKey Linux is designed as three decoupled subsystems centered around a pure C++ engine:

```
                      +-------------------------------------------+
                      |       Qt 6 / QML Settings Application      |
                      |            (banglakey-settings)           |
                      |   - Mode & keybinding configuration       |
                      |   - Visual layout previewer               |
                      |   - User dictionary & preferences         |
                      +---------------------+---------------------+
                                            |
                      (Configuration IPC /  | QSettings /
                       XDG Config Files)   | DBus Notification
                                            v
+------------------------+      +---------------------------+
|   Wayland Compositor   | <==> |     Fcitx5 IM Addon       |
|    (KDE Plasma / KWin) |      |    (banglakey-fcitx5)     |
|           or           |      | - fcitx::InputMethodEngine|
|       X11 Server       |      | - Preedit & Candidate UI  |
+------------------------+      +-------------+-------------+
                                              |
                                              | (In-process link)
                                              v
                                +---------------------------+
                                |    BanglaKey Core Engine  |
                                |     (banglakey_engine)    |
                                | - Bijoy Layout Processor  |
                                | - Phonetic State Machine  |
                                | - Juktakkhor Assembler    |
                                | - Unicode Normalizer      |
                                +---------------------------+
```

### Key Architectural Boundaries

1. **Independent Engine**:
   `banglakey_engine` has zero GUI or windowing dependencies. It can be compiled for unit testing, embedded into command-line utilities, or linked into the Fcitx5 addon.
2. **Decoupled Settings**:
   The Qt 6 settings application is an optional management tool. The Fcitx5 addon reads configuration directly from standard XDG paths (`~/.config/banglakey/config.ini`). If the settings application is not running, typing functions normally.
3. **Pure Input Method Pipeline**:
   All text emission is handled through standard Fcitx5 client protocols (`fcitx::InputContext::commitString` and `fcitx::InputContext::inputPanel`). Never use clipboard injection or synthetic keystrokes (`xdotool`, `ydotool`, `wtype`).

---

## 2. Shared Logical Typing Engine (`banglakey_engine`)

The core engine is structured around the `TypingEngine` interface:

```cpp
namespace banglakey {

enum class TypingMode {
    Bijoy,
    Phonetic
};

struct KeyEvent {
    uint32_t keycode;
    uint32_t keysym;
    uint32_t unicode;
    uint32_t modifiers; // Shift, Ctrl, Alt, CapsLock
    bool is_release;
};

enum class EngineActionType {
    None,
    Commit,
    Preedit,
    ForwardKey,
    Reset
};

struct EngineAction {
    EngineActionType type;
    std::string commit_text;
    std::string preedit_text;
    std::vector<std::string> candidates;
    int cursor_position;
};

class TypingEngine {
public:
    virtual ~TypingEngine() = default;
    virtual void setMode(TypingMode mode) = 0;
    virtual TypingMode mode() const = 0;
    virtual EngineAction processKeyEvent(const KeyEvent& event) = 0;
    virtual void reset() = 0;
};

} // namespace banglakey
```

---

## 3. Typing Subsystems

### 3.1 Bijoy Mode Architecture
Bijoy typing uses a direct keyboard mapping combined with a sequential state machine for conjuncts (*Juktakkhor*) and pre-vowel signs (*Kar*):

1. **Layout Matrix**:
   - Maps standard US QWERTY physical keys + Shift modifiers to Bengali characters and symbols.
2. **Link Key Processing (`g`)**:
   - In Bijoy layout, `g` acts as the Hasanta (Halant, `্`, U+09CD) operator.
   - Example sequence: `k` (ক) + `g` (্) + `k` (ক) => `ক্ক` (U+0995 + U+09CD + U+0995).
3. **Vowel Sign (*Kar*) Reordering**:
   - In physical Bijoy typing, certain *Kars* (such as E-kar `ে` on key `c`, OI-kar `ৈ` on `C`) are typed *before* the consonant, but in Unicode representation they must follow the consonant:
     - Typed: `c` (E-kar `ে`) + `k` (ক)
     - Stored Preedit: `ে`
     - Transformed Unicode Commit: `কে` (U+0995 + U+09C7)
   - The Bijoy engine maintains a bounded pending state (1-2 tokens) to correctly emit canonical Unicode codepoints without requiring user re-training.

### 3.2 Phonetic Mode Architecture
Phonetic typing transliterates Latin phonetic strings to Bengali with context-sensitive rule evaluation:

1. **Token Buffer**:
   - Accumulates typed ASCII characters into a buffer (`ami`, `bangla`, `khoroch`).
2. **Rule Evaluator**:
   - Applies longest-prefix and contextual regex matching against transliteration tables (e.g., `kh` -> `খ`, `k` -> `ক`, `ng` -> `ঙ`, `kkh` -> `ক্ষ`).
3. **Preedit & Candidate Selection**:
   - Dynamic preedit string displays the active converted text directly inline.
   - Candidates are ranked based on frequency and dictionary lookup.
   - Space or Enter commits the primary candidate; numeric keys or arrow keys select alternative candidates.

---

## 4. Fcitx5 Addon Integration (`banglakey-fcitx5`)

The Fcitx5 addon interfaces directly with the Fcitx5 daemon:

```cpp
class BanglaKeyAddon : public fcitx::InputMethodEngineV2 {
public:
    BanglaKeyAddon(fcitx::Instance* instance);
    void keyEvent(const fcitx::InputMethodEntry& entry, fcitx::KeyEvent& keyEvent) override;
    void activate(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) override;
    void deactivate(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) override;
    void reset(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) override;

private:
    std::unique_ptr<banglakey::TypingEngine> engine_;
    fcitx::Instance* instance_;
};
```

### Event Handling Pipeline:
1. `keyEvent` receives physical key and modifiers from Fcitx5.
2. Event is translated into `banglakey::KeyEvent`.
3. `engine_->processKeyEvent(event)` returns an `EngineAction`.
4. Addon acts on the action:
   - **Commit**: Calls `ic->commitString(action.commit_text)`.
   - **Preedit**: Updates `ic->inputPanel().setPreedit(...)`.
   - **ForwardKey**: Does not consume the event; lets the host application handle it (e.g., navigation arrows when buffer is empty).
   - **Update UI**: Calls `ic->updatePreedit()` and `ic->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel)`.

---

## 5. Desktop Settings Application (`banglakey-settings`)

The settings application is developed using Qt 6 and QML:
- **Presentation**: Fluid, responsive QML UI conforming to KDE Plasma 6 Breeze design language.
- **Backend**: C++ `QObject` backend reading and writing standard INI configuration files.
- **Live Typing Sandbox**: An embedded typing sandbox allowing users to immediately test Bijoy and Phonetic modes without switching active system input methods.
- **Layout Cheatsheet**: Visual SVG-rendered keyboard diagram illustrating the Bijoy layout mapping and phonetic consonant/vowel shortcuts.

---

## 6. Bijoy Bayanno-Style Desktop Interaction Model

BanglaKey Linux implements the familiar, proven interaction model inspired by classic Bengali desktop utilities (such as Bijoy Bayanno), consisting of:

1. **Startup Splash Screen (`Splash.qml`)**:
   - A compact, polished startup banner displaying original BanglaKey branding, Bengali typography, and an initialization transition.
   - Automatically closes after ~1.8 seconds and smoothly transitions to the floating toolbar.

2. **Floating Toolbar / Topbar (`Toolbar.qml`)**:
   - Compact horizontal floating window with a classic blue toolbar gradient.
   - Window attributes: Frameless (`Qt::FramelessWindowHint`), stays above normal windows (`Qt::WindowStaysOnTopHint`), tool window (`Qt::Tool`).
   - Movable by dragging anywhere across the toolbar.
   - Contains:
     - BanglaKey logo emblem.
     - Live Bengali/English mode title indicator.
     - Mode selection dropdown with 4 options: **English**, **Bijoy Classic**, **Bijoy Unicode**, **Unicode**.
     - Settings button (`⚙`) to open the full preferences & diagnostics panel.
     - Minimize button (`—`) and Close button (`✕`) which hide the toolbar to the system tray without terminating background input services.
   - Automatically persists position (`x`, `y`) and visibility state across sessions in `~/.config/banglakey/config.ini`.

3. **System Tray Integration (`QSystemTrayIcon` & Context Menu)**:
   - Persistent tray icon in the system notification area.
   - **Left-Click Action**: Cycles through the 4 input modes in predictable order (`English` → `Unicode` → `Bijoy Unicode` → `Bijoy Classic` → `English`).
   - **Right-Click Context Menu**:
     1. `Unicode` (colored square indicator, checkmark when active)
     2. `Bijoy Unicode` (colored square indicator, checkmark when active)
     3. `Bijoy Classic` (colored square indicator, checkmark when active)
     4. `English` (BanglaKey application icon, checkmark when active)
     5. Separator
     6. `Show as Topbar` (green restore icon)
     7. `Close` (red X icon, exits the application cleanly)

4. **Input Modes Definition**:
   - **English**: Raw keystroke passthrough; the Fcitx5 addon forwards all keys to the host application without Bengali processing.
   - **Unicode**: Natural phonetic transliteration (Avro-compatible Roman-to-Bengali mapping).
   - **Bijoy Unicode**: Standard Bijoy keyboard layout producing modern standard Unicode Bengali characters.
   - **Bijoy Classic**: Standard Bijoy keyboard layout.

5. **Inter-Process Communication (IPC)**:
   - Lightweight, permission-conscious IPC anchored in `$XDG_RUNTIME_DIR/banglakey/`:
     - **Runtime State File**: `$XDG_RUNTIME_DIR/banglakey/active_mode` (permissions `0600`).
     - **Local Domain Socket**: `$XDG_RUNTIME_DIR/banglakey/ipc.sock` via `QLocalServer`.
   - When the user selects a mode via the Toolbar dropdown or Tray menu, the desktop application updates the state file and notifies connected listeners.
   - The Fcitx5 addon checks this state file on key events, keeping the running input engine, floating toolbar dropdown, and system tray checkmarks synchronized in real-time.

6. **Global Keyboard Shortcuts**:
   - `Ctrl+B`: Toggles **Bijoy Classic** mode. Pressing it again while in Bijoy Classic mode returns to **English**.
   - `Ctrl+V` (and `Ctrl+Alt+V` fallback): Toggles **Unicode** (Phonetic) mode. Pressing it again while in Unicode mode returns to **English**.
   - `Ctrl+Alt+B` / `Ctrl+Shift+B`: Toggles **Bijoy Unicode** mode.
   - `F12` / `Ctrl+Alt+M`: Cycles sequentially through all modes.

---

## 7. Wayland & X11 Compatibility

Under Fedora 44 with KDE Plasma 6:
- Wayland is the default session. Fcitx5 acts as the system input method server communicating via `zwp_input_method_v1` / `zwp_input_method_v2` protocols with KWin.
- X11 applications running through XWayland receive input via the XIM / Fcitx5 X11 bridge.
- By adhering strictly to Fcitx5's abstraction layer, BanglaKey automatically guarantees flawless behavior across native Wayland applications, Qt/GTK toolkits, Electron apps, terminals, and XWayland clients.

---

## 8. Packaging & Distribution

- The project provides a standard Fedora RPM specification (`packaging/rpm/banglakey.spec`).
- Target directories follow standard FHS / Fedora packaging guidelines:
  - Addon shared object: `/usr/lib64/fcitx5/libbanglakey-fcitx5.so`
  - Addon configuration: `/usr/share/fcitx5/addon/banglakey.conf`
  - Input method configuration: `/usr/share/fcitx5/inputmethod/banglakey-addon.conf`
  - Settings binary: `/usr/bin/banglakey-settings`
  - Desktop launcher: `/usr/share/applications/org.banglakey.Settings.desktop`
  - Icons: `/usr/share/icons/hicolor/...`
