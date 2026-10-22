#include "banglakey_addon.hpp"

#include <fstream>
#include <cstdlib>

#include <fcitx-utils/key.h>
#include <fcitx-utils/keysymgen.h>
#include <fcitx-utils/log.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontextmanager.h>

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
      engine_(banglakey::TypingEngine::create(banglakey::TypingMode::Phonetic)),
      propFactory_([this](fcitx::InputContext& ic) {
          return new BanglaKeyContextProperty(engine_.get(), reinterpret_cast<banglakey::ContextId>(&ic));
      }) {
    if (instance_) {
        instance_->inputContextManager().registerProperty("banglakey-context-prop", &propFactory_);
    }
    FCITX_INFO() << "BanglaKey: Addon initialized with Phonetic mode as default.";
}

BanglaKeyAddon::~BanglaKeyAddon() {
    if (propFactory_.registered()) {
        propFactory_.unregister();
    }
}

void BanglaKeyAddon::activate(const fcitx::InputMethodEntry& /*entry*/, fcitx::InputContextEvent& event) {
    auto* ic = event.inputContext();
    if (ic && engine_) {
        auto ctxId = reinterpret_cast<banglakey::ContextId>(ic);
        engine_->reset(ctxId);
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
    entry.setIcon("input-keyboard");
    entry.setLabel("বাং");
    entries.push_back(std::move(entry));
    return entries;
}

std::string BanglaKeyAddon::subMode(const fcitx::InputMethodEntry& /*entry*/, fcitx::InputContext& ic) {
    if (!engine_) return {};
    auto ctxId = reinterpret_cast<banglakey::ContextId>(&ic);
    return (engine_->mode(ctxId) == banglakey::TypingMode::Bijoy) ? "Bijoy" : "Phonetic";
}

std::string BanglaKeyAddon::subModeLabelImpl(const fcitx::InputMethodEntry& /*entry*/, fcitx::InputContext& ic) {
    if (!engine_) return {};
    auto ctxId = reinterpret_cast<banglakey::ContextId>(&ic);
    return (engine_->mode(ctxId) == banglakey::TypingMode::Bijoy) ? "বিজয়" : "ফোনেটিক";
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

void BanglaKeyAddon::syncActiveModeFromDisk(banglakey::ContextId ctxId) {
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
                if (engine_) {
                    if (active_mode_ == "Bijoy Classic" || active_mode_ == "Bijoy Unicode" || active_mode_ == "Bijoy") {
                        engine_->setMode(ctxId, banglakey::TypingMode::Bijoy);
                    } else if (active_mode_ == "Unicode" || active_mode_ == "Phonetic") {
                        engine_->setMode(ctxId, banglakey::TypingMode::Phonetic);
                    }
                }
            }
        }
    }
}

void BanglaKeyAddon::writeActiveModeToDisk(const std::string& targetMode) {
    std::string path = getRuntimeModeFilePath();
    std::ofstream out(path, std::ios::trunc);
    if (out.is_open()) {
        out << targetMode << "\n";
    }
}

void BanglaKeyAddon::setSpecificMode(fcitx::InputContext* ic, banglakey::ContextId ctxId, const std::string& targetMode, fcitx::KeyEvent& keyEvent) {
    if (!engine_) return;

    engine_->reset(ctxId);
    ic->inputPanel().reset();
    ic->updatePreedit();
    ic->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);

    active_mode_ = targetMode;
    if (targetMode == "Bijoy Classic" || targetMode == "Bijoy Unicode" || targetMode == "Bijoy") {
        engine_->setMode(ctxId, banglakey::TypingMode::Bijoy);
    } else if (targetMode == "Unicode" || targetMode == "Phonetic") {
        engine_->setMode(ctxId, banglakey::TypingMode::Phonetic);
    }

    writeActiveModeToDisk(targetMode);

    std::string popup = "BanglaKey: " + targetMode;
    FCITX_INFO() << "BanglaKey: activated mode " << targetMode;

    if (instance_) {
        instance_->showCustomInputMethodInformation(ic, popup.c_str());
    }

    keyEvent.filterAndAccept();
}

void BanglaKeyAddon::handleModeSwitch(fcitx::InputContext* ic, banglakey::ContextId ctxId, fcitx::KeyEvent& keyEvent) {
    auto currentMode = engine_->mode(ctxId);
    auto newMode = (currentMode == banglakey::TypingMode::Phonetic)
                       ? banglakey::TypingMode::Bijoy
                       : banglakey::TypingMode::Phonetic;

    auto action = engine_->switchMode(ctxId, newMode);
    if (action.type == banglakey::EngineActionType::Commit && !action.commit_text.empty()) {
        ic->commitString(action.commit_text);
    }
    ic->inputPanel().reset();
    ic->updatePreedit();
    ic->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);

    const char* modeName = (newMode == banglakey::TypingMode::Bijoy) ? "Bijoy" : "Phonetic";
    active_mode_ = (newMode == banglakey::TypingMode::Bijoy) ? "Bijoy Classic" : "Unicode";
    writeActiveModeToDisk(active_mode_);

    const char* popupMsg = (newMode == banglakey::TypingMode::Bijoy) ? "BanglaKey: Bijoy Mode" : "BanglaKey: Phonetic Mode";
    FCITX_INFO() << "BanglaKey: switched typing mode to " << modeName;

    if (instance_) {
        instance_->showCustomInputMethodInformation(ic, popupMsg);
    }

    keyEvent.filterAndAccept();
}

void BanglaKeyAddon::keyEvent(const fcitx::InputMethodEntry& /*entry*/, fcitx::KeyEvent& keyEvent) {
    if (!engine_ || keyEvent.isRelease()) {
        return;
    }

    auto* ic = keyEvent.inputContext();
    if (!ic) {
        return;
    }

    // Ensure property exists to manage context cleanup
    ic->propertyFor(&propFactory_);
    auto ctxId = reinterpret_cast<banglakey::ContextId>(ic);

    // Extract raw key attributes
    auto sym = keyEvent.rawKey().sym();
    auto code = keyEvent.rawKey().code();
    auto states = keyEvent.rawKey().states();

    // Convert modifiers
    banglakey::Modifier mod = banglakey::Modifier::None;
    if (states & fcitx::KeyState::Shift) {
        mod = mod | banglakey::Modifier::Shift;
    }
    if (states & fcitx::KeyState::Ctrl) {
        mod = mod | banglakey::Modifier::Control;
    }
    if (states & fcitx::KeyState::Alt) {
        mod = mod | banglakey::Modifier::Alt;
    }
    if (states & fcitx::KeyState::Super) {
        mod = mod | banglakey::Modifier::Super;
    }
    if (states & fcitx::KeyState::CapsLock) {
        mod = mod | banglakey::Modifier::CapsLock;
    }

    // Sync active mode from disk if updated by toolbar or tray
    syncActiveModeFromDisk(ctxId);

    // 1. Ctrl+B: Toggle Bijoy Classic <-> English
    if ((sym == 'b' || sym == 'B' || sym == FcitxKey_b || sym == FcitxKey_B) &&
        banglakey::hasModifier(mod, banglakey::Modifier::Control) &&
        !banglakey::hasModifier(mod, banglakey::Modifier::Alt) &&
        !banglakey::hasModifier(mod, banglakey::Modifier::Super)) {
        std::string target = (active_mode_ == "Bijoy Classic") ? "English" : "Bijoy Classic";
        setSpecificMode(ic, ctxId, target, keyEvent);
        return;
    }

    // 2. Ctrl+V: Toggle Unicode <-> English
    if ((sym == 'v' || sym == 'V' || sym == FcitxKey_v || sym == FcitxKey_V) &&
        banglakey::hasModifier(mod, banglakey::Modifier::Control) &&
        !banglakey::hasModifier(mod, banglakey::Modifier::Super)) {
        std::string target = (active_mode_ == "Unicode") ? "English" : "Unicode";
        setSpecificMode(ic, ctxId, target, keyEvent);
        return;
    }

    // 3. Ctrl+Alt+B or Ctrl+Shift+B: Toggle Bijoy Unicode <-> English
    if ((sym == 'b' || sym == 'B' || sym == FcitxKey_b || sym == FcitxKey_B) &&
        banglakey::hasModifier(mod, banglakey::Modifier::Control) &&
        (banglakey::hasModifier(mod, banglakey::Modifier::Alt) || banglakey::hasModifier(mod, banglakey::Modifier::Shift))) {
        std::string target = (active_mode_ == "Bijoy Unicode") ? "English" : "Bijoy Unicode";
        setSpecificMode(ic, ctxId, target, keyEvent);
        return;
    }

    // 4. F12 or Ctrl+Alt+M: Toggle / Cycle mode
    if (isModeSwitchKey(keyEvent, mod)) {
        handleModeSwitch(ic, ctxId, keyEvent);
        return;
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

    banglakey::EngineAction action = engine_->processKeyEvent(ctxId, bkEvent);

    switch (action.type) {
        case banglakey::EngineActionType::Commit:
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
            fcitx::Text preeditText(action.preedit_text);
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
