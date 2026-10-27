#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <fcitx-utils/key.h>
#include <fcitx-utils/keysymgen.h>
#include <fcitx-utils/log.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontextmanager.h>

#include <banglakey/unicode.hpp>

#include "banglakey_addon.hpp"

namespace banglakey::fcitx5 {

BanglaKeyContextProperty::BanglaKeyContextProperty(banglakey::TypingEngine* engine, banglakey::ContextId ctxId) noexcept
    : engine_(engine), ctxId_(ctxId) {
}

BanglaKeyContextProperty::~BanglaKeyContextProperty() {
    if (engine_) {
        engine_->destroyContext(ctxId_);
    }
}

BanglaKeyCandidateWord::BanglaKeyCandidateWord(BanglaKeyAddon* addon, std::string text, size_t index)
    : fcitx::CandidateWord(fcitx::Text(text)),
      addon_(addon),
      text_(std::move(text)),
      index_(index) {
}

void BanglaKeyCandidateWord::select(fcitx::InputContext* inputContext) const {
    if (!inputContext || !addon_) {
        return;
    }
    addon_->selectCandidate(inputContext, text_, index_);
}

BanglaKeyAddon::BanglaKeyAddon(fcitx::Instance* instance)
    : instance_(instance),
      engine_(banglakey::TypingEngine::create(banglakey::TypingMode::BijoyUnicode)),
      propFactory_([this](fcitx::InputContext& ic) {
          return new BanglaKeyContextProperty(engine_.get(), reinterpret_cast<banglakey::ContextId>(&ic));
      }) {
    if (instance_) {
        instance_->inputContextManager().registerProperty("banglakey-context-prop", &propFactory_);
        eventWatcher_ = instance_->watchEvent(
            fcitx::EventType::InputContextKeyEvent,
            fcitx::EventWatcherPhase::PreInputMethod,
            [this](fcitx::Event& event) {
                auto& keyEvent = static_cast<fcitx::KeyEvent&>(event);
                processShortcut(keyEvent);
            });
    }
    FCITX_INFO() << "BanglaKey: Addon initialized with Bijoy Unicode mode as default.";
}

BanglaKeyAddon::~BanglaKeyAddon() {
    eventWatcher_.reset();
    if (propFactory_.registered()) {
        propFactory_.unregister();
    }
}

void BanglaKeyAddon::activate(const fcitx::InputMethodEntry& /*entry*/, fcitx::InputContextEvent& event) {
    auto* ic = event.inputContext();
    if (ic && engine_) {
        auto ctxId = reinterpret_cast<banglakey::ContextId>(ic);
        engine_->reset(ctxId);
        syncActiveModeFromDisk(ic, ctxId);
    }
}

void BanglaKeyAddon::deactivate(const fcitx::InputMethodEntry& /*entry*/, fcitx::InputContextEvent& event) {
    auto* ic = event.inputContext();
    if (ic && engine_) {
        auto ctxId = reinterpret_cast<banglakey::ContextId>(ic);
        engine_->reset(ctxId);
        ic->inputPanel().reset();
    }
}

void BanglaKeyAddon::reset(const fcitx::InputMethodEntry& /*entry*/, fcitx::InputContextEvent& event) {
    auto* ic = event.inputContext();
    if (ic && engine_) {
        auto ctxId = reinterpret_cast<banglakey::ContextId>(ic);
        engine_->reset(ctxId);
        ic->inputPanel().reset();
    }
}

std::vector<fcitx::InputMethodEntry> BanglaKeyAddon::listInputMethods() {
    std::vector<fcitx::InputMethodEntry> entries;
    fcitx::InputMethodEntry entry("banglakey", "BanglaKey", "bn", "banglakey");
    entry.setNativeName("বাংলাকি");
    entry.setIcon("banglakey");
    entry.setLabel("বাং");
    entries.push_back(std::move(entry));
    return entries;
}

std::string BanglaKeyAddon::subMode(const fcitx::InputMethodEntry& /*entry*/, fcitx::InputContext& ic) {
    if (!engine_) return {};
    auto ctxId = reinterpret_cast<banglakey::ContextId>(&ic);
    if (active_mode_ == "English") return "English";
    auto m = engine_->mode(ctxId);
    if (m == banglakey::TypingMode::BijoyClassic) return "Bijoy Classic";
    if (m == banglakey::TypingMode::BijoyUnicode) return "Bijoy Unicode";
    return "Avro Phonetic";
}

std::string BanglaKeyAddon::subModeLabelImpl(const fcitx::InputMethodEntry& /*entry*/, fcitx::InputContext& ic) {
    if (!engine_) return {};
    auto ctxId = reinterpret_cast<banglakey::ContextId>(&ic);
    if (active_mode_ == "English") return "EN";
    auto m = engine_->mode(ctxId);
    if (m == banglakey::TypingMode::BijoyClassic) return "ক্লাসিক";
    if (m == banglakey::TypingMode::BijoyUnicode) return "বি-ইউ";
    return "অভ্রো";
}

void BanglaKeyAddon::selectCandidate(fcitx::InputContext* ic, const std::string& candidateText, size_t index) {
    if (!ic || !engine_) return;
    auto ctxId = reinterpret_cast<banglakey::ContextId>(ic);
    ic->commitString(candidateText);
    engine_->reset(ctxId);
    ic->inputPanel().reset();
    ic->updatePreedit();
    ic->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
    FCITX_DEBUG() << "BanglaKey: candidate selected by UI at index " << index;
}

bool BanglaKeyAddon::isModeSwitchKey(const fcitx::KeyEvent& keyEvent, banglakey::Modifier mod) const noexcept {
    auto sym = keyEvent.rawKey().sym();
    // F12 key (XK_F12 = 0xffc9) without modifiers
    if (sym == FcitxKey_F12 &&
        !banglakey::hasModifier(mod, banglakey::Modifier::Control) &&
        !banglakey::hasModifier(mod, banglakey::Modifier::Alt) &&
        !banglakey::hasModifier(mod, banglakey::Modifier::Super)) {
        return true;
    }
    // Ctrl + Alt + M
    if ((sym == 'm' || sym == 'M' || sym == FcitxKey_m || sym == FcitxKey_M) &&
        banglakey::hasModifier(mod, banglakey::Modifier::Control) &&
        banglakey::hasModifier(mod, banglakey::Modifier::Alt) &&
        !banglakey::hasModifier(mod, banglakey::Modifier::Super)) {
        return true;
    }
    return false;
}

namespace {

std::string getRuntimeModeFilePath() {
    const char* xdgRuntime = getenv("XDG_RUNTIME_DIR");
    if (xdgRuntime && *xdgRuntime) {
        return std::string(xdgRuntime) + "/banglakey/active_mode";
    }
    const char* user = getenv("USER");
    return "/tmp/banglakey-" + std::string(user ? user : "default") + "/active_mode";
}

} // namespace

void BanglaKeyAddon::syncActiveModeFromDisk(fcitx::InputContext* ic, banglakey::ContextId ctxId) {
    std::string path = getRuntimeModeFilePath();
    std::ifstream in(path);
    if (in.is_open()) {
        std::string line;
        if (std::getline(in, line)) {
            while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) {
                line.pop_back();
            }
            if (!line.empty() && line != active_mode_) {
                active_mode_ = line;
                if (active_mode_ != "English") {
                    last_bengali_mode_ = active_mode_;
                }
                if (engine_) {
                    banglakey::TypingMode targetMode = banglakey::TypingMode::AvroPhonetic;
                    if (active_mode_ == "Bijoy Classic") {
                        targetMode = banglakey::TypingMode::BijoyClassic;
                    } else if (active_mode_ == "Bijoy Unicode" || active_mode_ == "Unicode") {
                        targetMode = banglakey::TypingMode::BijoyUnicode;
                    } else if (active_mode_ == "Avro Phonetic" || active_mode_ == "Phonetic") {
                        targetMode = banglakey::TypingMode::AvroPhonetic;
                    }

                    if (engine_->mode(ctxId) != targetMode) {
                        if (ic) {
                            auto action = engine_->switchMode(ctxId, targetMode);
                            if (action.type == banglakey::EngineActionType::Commit && !action.commit_text.empty()) {
                                ic->commitString(action.commit_text);
                            }
                            ic->inputPanel().reset();
                            ic->updatePreedit();
                            ic->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
                        } else {
                            engine_->setMode(ctxId, targetMode);
                        }
                    }
                }
            }
        }
    }
}

void BanglaKeyAddon::writeActiveModeToDisk(const std::string& targetMode) {
    std::string path = getRuntimeModeFilePath();
    try {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    } catch (...) {}
    std::ofstream out(path, std::ios::trunc);
    if (out.is_open()) {
        out << targetMode << "\n";
        out.flush();
    }

    // Direct IPC socket notification to running desktop app
    std::string sockPath;
    const char* xdg = getenv("XDG_RUNTIME_DIR");
    if (xdg && *xdg) {
        sockPath = std::string(xdg) + "/banglakey/ipc.sock";
    } else {
        const char* u = getenv("USER");
        sockPath = "/tmp/banglakey-" + std::string(u ? u : "default") + "/ipc.sock";
    }

    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd >= 0) {
        struct sockaddr_un addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, sockPath.c_str(), sizeof(addr.sun_path) - 1);
        if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
            std::string msg = "MODE:" + targetMode + "\n";
            [[maybe_unused]] auto ignored = ::write(fd, msg.data(), msg.size());
        }
        ::close(fd);
    }
}

void BanglaKeyAddon::setSpecificMode(fcitx::InputContext* ic, banglakey::ContextId ctxId, const std::string& targetMode, fcitx::KeyEvent& keyEvent) {
    if (!engine_) return;

    banglakey::TypingMode targetTypingMode = banglakey::TypingMode::AvroPhonetic;
    if (targetMode == "Bijoy Classic") {
        targetTypingMode = banglakey::TypingMode::BijoyClassic;
    } else if (targetMode == "Bijoy Unicode" || targetMode == "Unicode") {
        targetTypingMode = banglakey::TypingMode::BijoyUnicode;
    } else if (targetMode == "Avro Phonetic" || targetMode == "Phonetic") {
        targetTypingMode = banglakey::TypingMode::AvroPhonetic;
    }

    auto action = engine_->switchMode(ctxId, targetTypingMode);
    if (action.type == banglakey::EngineActionType::Commit && !action.commit_text.empty()) {
        ic->commitString(action.commit_text);
    }
    ic->inputPanel().reset();
    ic->updatePreedit();
    ic->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);

    bool modeChanged = (active_mode_ != targetMode);
    active_mode_ = targetMode;
    if (targetMode != "English") {
        last_bengali_mode_ = targetMode;
    }
    writeActiveModeToDisk(targetMode);

    std::string popup = "BanglaKey: " + targetMode;
    FCITX_INFO() << "BanglaKey: activated mode " << targetMode;

    if (instance_ && modeChanged) {
        instance_->showCustomInputMethodInformation(ic, popup.c_str());
    }

    keyEvent.filterAndAccept();
}

void BanglaKeyAddon::toggleModeWithEnglish(fcitx::InputContext* ic, banglakey::ContextId ctxId, const std::string& assignedBengaliMode, fcitx::KeyEvent& keyEvent) {
    std::string nextMode;
    if (active_mode_ == assignedBengaliMode) {
        nextMode = "English";
    } else {
        nextMode = assignedBengaliMode;
    }
    setSpecificMode(ic, ctxId, nextMode, keyEvent);
}

void BanglaKeyAddon::handleModeSwitch(fcitx::InputContext* ic, banglakey::ContextId ctxId, fcitx::KeyEvent& keyEvent) {
    // Cycle English -> Bijoy Unicode -> Bijoy Classic -> Avro Phonetic -> English
    std::string nextMode = "English";
    if (active_mode_ == "English") {
        nextMode = "Bijoy Unicode";
    } else if (active_mode_ == "Bijoy Unicode" || active_mode_ == "Unicode") {
        nextMode = "Bijoy Classic";
    } else if (active_mode_ == "Bijoy Classic") {
        nextMode = "Avro Phonetic";
    } else if (active_mode_ == "Avro Phonetic" || active_mode_ == "Phonetic") {
        nextMode = "English";
    } else {
        nextMode = "English";
    }

    setSpecificMode(ic, ctxId, nextMode, keyEvent);
}

bool BanglaKeyAddon::processShortcut(fcitx::KeyEvent& keyEvent) {
    if (!engine_) {
        return false;
    }

    if (keyEvent.isRelease()) {
        last_shortcut_id_ = 0;
        return false;
    }

    if (keyEvent.filtered()) {
        return false;
    }

    auto* ic = keyEvent.inputContext();
    if (!ic) {
        return false;
    }

    ic->propertyFor(&propFactory_);
    auto ctxId = reinterpret_cast<banglakey::ContextId>(ic);
    syncActiveModeFromDisk(ic, ctxId);

    auto sym = keyEvent.rawKey().sym();
    auto rawStates = keyEvent.rawKey().states();
    auto normSym = keyEvent.key().sym();
    auto normStates = keyEvent.key().states();

    bool hasCtrl = (normStates & fcitx::KeyState::Ctrl) || (rawStates & fcitx::KeyState::Ctrl);
    bool hasAlt = (normStates & fcitx::KeyState::Alt) || (rawStates & fcitx::KeyState::Alt) ||
                  (normStates & fcitx::KeyState::Meta) || (rawStates & fcitx::KeyState::Meta);
    bool hasShift = (normStates & fcitx::KeyState::Shift) || (rawStates & fcitx::KeyState::Shift);
    bool hasSuper = (normStates & fcitx::KeyState::Super) || (rawStates & fcitx::KeyState::Super) ||
                    (normStates & fcitx::KeyState::Super2) || (rawStates & fcitx::KeyState::Super2);

    bool isKeyB = (normSym == FcitxKey_b || normSym == FcitxKey_B ||
                   sym == FcitxKey_b || sym == FcitxKey_B ||
                   sym == 'b' || sym == 'B');
    bool isKeyV = (normSym == FcitxKey_v || normSym == FcitxKey_V ||
                   sym == FcitxKey_v || sym == FcitxKey_V ||
                   sym == 'v' || sym == 'V');
    bool isKeyA = (normSym == FcitxKey_a || normSym == FcitxKey_A ||
                   sym == FcitxKey_a || sym == FcitxKey_A ||
                   sym == 'a' || sym == 'A');
    bool isKeyO = (normSym == FcitxKey_o || normSym == FcitxKey_O ||
                   sym == FcitxKey_o || sym == FcitxKey_O ||
                   sym == 'o' || sym == 'O');
    bool isF12 = (normSym == FcitxKey_F12 || sym == FcitxKey_F12);

    bool isCtrlAlt = hasCtrl && hasAlt && !hasSuper && !hasShift;

    uint32_t matchedShortcut = 0;

    // 1. Ctrl+Alt+B -> Toggle Bijoy Classic <-> English
    if (isCtrlAlt && isKeyB) {
        matchedShortcut = 1;
    }
    // 2. Ctrl+Alt+A -> Toggle Bijoy Unicode <-> English
    else if (isCtrlAlt && isKeyA) {
        matchedShortcut = 2;
    }
    // 3. Ctrl+Alt+O -> Toggle Avro Phonetic <-> English
    else if (isCtrlAlt && isKeyO) {
        matchedShortcut = 3;
    }
    // 4. Ctrl+Alt+V -> Toggle Bijoy Unicode <-> English
    else if (isCtrlAlt && isKeyV) {
        matchedShortcut = 4;
    }
    // 5. F12 or Ctrl+Alt+M -> Cycle all modes
    else if (isF12 || isModeSwitchKey(keyEvent, banglakey::Modifier::None) ||
             (isCtrlAlt && (normSym == FcitxKey_m || normSym == FcitxKey_M || sym == 'm' || sym == 'M'))) {
        matchedShortcut = 5;
    }

    if (matchedShortcut == 0) {
        return false;
    }

    // Auto-repeat event from physical key hold: swallow without re-toggling
    if (last_shortcut_id_ == matchedShortcut) {
        keyEvent.filterAndAccept();
        return true;
    }
    last_shortcut_id_ = matchedShortcut;

    // Ensure BanglaKey is active for this input context if another IM was focused
    if (instance_ && instance_->inputMethod(ic) != "banglakey") {
        instance_->setCurrentInputMethod(ic, "banglakey", false);
    }

    switch (matchedShortcut) {
        case 1:
            FCITX_INFO() << "BanglaKey: shortcut Ctrl+Alt+B (Bijoy Classic toggle). Current: " << active_mode_;
            toggleModeWithEnglish(ic, ctxId, "Bijoy Classic", keyEvent);
            break;
        case 2:
            FCITX_INFO() << "BanglaKey: shortcut Ctrl+Alt+A (Bijoy Unicode toggle). Current: " << active_mode_;
            toggleModeWithEnglish(ic, ctxId, "Bijoy Unicode", keyEvent);
            break;
        case 3:
            FCITX_INFO() << "BanglaKey: shortcut Ctrl+Alt+O (Avro Phonetic toggle). Current: " << active_mode_;
            toggleModeWithEnglish(ic, ctxId, "Avro Phonetic", keyEvent);
            break;
        case 4:
            FCITX_INFO() << "BanglaKey: shortcut Ctrl+Alt+V (Bijoy Unicode toggle). Current: " << active_mode_;
            toggleModeWithEnglish(ic, ctxId, "Bijoy Unicode", keyEvent);
            break;
        case 5:
            FCITX_INFO() << "BanglaKey: cycle mode shortcut (F12). Current: " << active_mode_;
            handleModeSwitch(ic, ctxId, keyEvent);
            break;
    }

    return true;
}

void BanglaKeyAddon::keyEvent(const fcitx::InputMethodEntry& /*entry*/, fcitx::KeyEvent& keyEvent) {
    if (!engine_) {
        return;
    }

    if (keyEvent.isRelease()) {
        last_shortcut_id_ = 0;
        return;
    }

    if (keyEvent.filtered()) {
        return;
    }

    // Process shortcuts with repeat deduplication and toggle semantics
    if (processShortcut(keyEvent)) {
        return;
    }

    auto* ic = keyEvent.inputContext();
    if (!ic) {
        return;
    }

    // Ensure property exists to manage context cleanup
    ic->propertyFor(&propFactory_);
    auto ctxId = reinterpret_cast<banglakey::ContextId>(ic);

    // Sync active mode from disk if updated by toolbar or tray
    syncActiveModeFromDisk(ic, ctxId);

    // Extract raw and normalized key attributes
    auto sym = keyEvent.rawKey().sym();
    auto code = keyEvent.rawKey().code();
    auto rawStates = keyEvent.rawKey().states();
    auto normStates = keyEvent.key().states();

    banglakey::Modifier mod = banglakey::Modifier::None;
    if (rawStates & fcitx::KeyState::Shift || normStates & fcitx::KeyState::Shift) {
        mod = mod | banglakey::Modifier::Shift;
    }
    if (rawStates & fcitx::KeyState::Ctrl || normStates & fcitx::KeyState::Ctrl) {
        mod = mod | banglakey::Modifier::Control;
    }
    if (rawStates & fcitx::KeyState::Alt || normStates & fcitx::KeyState::Alt) {
        mod = mod | banglakey::Modifier::Alt;
    }
    if (rawStates & fcitx::KeyState::Super || normStates & fcitx::KeyState::Super) {
        mod = mod | banglakey::Modifier::Super;
    }
    if (rawStates & fcitx::KeyState::CapsLock || normStates & fcitx::KeyState::CapsLock) {
        mod = mod | banglakey::Modifier::CapsLock;
    }

    // If English mode is active, pass key through to applications untouched!
    if (active_mode_ == "English") {
        return;
    }

    // Handle candidate selection with number keys 1-9 if candidate list is active
    auto candList = ic->inputPanel().candidateList();
    if (candList && candList->size() > 0 &&
        !banglakey::hasModifier(mod, banglakey::Modifier::Control) &&
        !banglakey::hasModifier(mod, banglakey::Modifier::Alt) &&
        !banglakey::hasModifier(mod, banglakey::Modifier::Super)) {
        if (sym >= FcitxKey_1 && sym <= FcitxKey_9) {
            int index = static_cast<int>(sym - FcitxKey_1);
            if (index < candList->size()) {
                candList->candidate(index).select(ic);
                keyEvent.filterAndAccept();
                return;
            }
        }
    }

    // Convert to engine event
    banglakey::KeyEvent bkEvent;
    bkEvent.keycode = code;
    bkEvent.keysym = sym;
    bkEvent.unicode = fcitx::Key::keySymToUnicode(sym);
    bkEvent.modifiers = mod;
    bkEvent.is_release = false;

    FCITX_INFO() << "[DIAG 1. KEY] sym=" << sym << " (0x" << std::hex << sym << std::dec << ") code=" << code
                 << " states=0x" << std::hex << rawStates << std::dec << " mode=" << active_mode_;

    banglakey::EngineAction action = engine_->processKeyEvent(ctxId, bkEvent);

    switch (action.type) {
        case banglakey::EngineActionType::Commit:
            FCITX_INFO() << "[DIAG 5. COMMIT] text='" << action.commit_text << "' [DIAG 6. CODEPOINTS] ("
                         << banglakey::unicode::toHexCodepoints(action.commit_text) << ")";
            if (!action.commit_text.empty()) {
                ic->commitString(action.commit_text);
            }
            ic->inputPanel().reset();
            ic->updatePreedit();
            ic->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
            // Forward system shortcuts so application receives them after commit
            if (!banglakey::hasModifier(mod, banglakey::Modifier::Control) &&
                !banglakey::hasModifier(mod, banglakey::Modifier::Alt) &&
                !banglakey::hasModifier(mod, banglakey::Modifier::Super)) {
                keyEvent.filterAndAccept();
            }
            break;

        case banglakey::EngineActionType::Preedit: {
            FCITX_INFO() << "[DIAG 4. PREEDIT] text='" << action.preedit_text << "' ("
                         << banglakey::unicode::toHexCodepoints(action.preedit_text)
                         << ") cursor=" << action.cursor_position;
            fcitx::Text preeditText(action.preedit_text);
            if (action.cursor_position >= 0) {
                preeditText.setCursor(action.cursor_position);
            }
            ic->inputPanel().setClientPreedit(preeditText);
            ic->inputPanel().setPreedit(preeditText);

            if (!action.candidates.empty()) {
                auto cList = std::make_unique<fcitx::CommonCandidateList>();
                cList->setLayoutHint(fcitx::CandidateLayoutHint::Vertical);
                std::vector<std::string> labels;
                for (size_t i = 0; i < action.candidates.size(); ++i) {
                    if (i < 9) {
                        labels.push_back(std::to_string(i + 1));
                    }
                    cList->append(std::make_unique<BanglaKeyCandidateWord>(
                        this, action.candidates[i], i
                    ));
                }
                if (!labels.empty()) {
                    cList->setLabels(labels);
                }
                const auto& state = engine_->state(ctxId);
                cList->setGlobalCursorIndex(static_cast<int>(state.selectedCandidateIndex()));
                ic->inputPanel().setCandidateList(std::move(cList));
            } else {
                ic->inputPanel().setCandidateList(nullptr);
            }

            ic->updatePreedit();
            ic->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
            keyEvent.filterAndAccept();
            break;
        }

        case banglakey::EngineActionType::Reset:
            ic->inputPanel().reset();
            ic->updatePreedit();
            ic->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
            keyEvent.filterAndAccept();
            break;

        case banglakey::EngineActionType::ForwardKey:
        case banglakey::EngineActionType::None:
            // Let host application receive key event
            break;
    }
}

fcitx::AddonInstance* BanglaKeyAddonFactory::create(fcitx::AddonManager* manager) {
    return new BanglaKeyAddon(manager->instance());
}

} // namespace banglakey::fcitx5

FCITX_ADDON_FACTORY(banglakey::fcitx5::BanglaKeyAddonFactory);
