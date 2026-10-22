#pragma once

#include <memory>
#include <string>
#include <vector>

#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>

#include <banglakey/engine.hpp>

namespace banglakey::fcitx5 {

class BanglaKeyAddon;

/**
 * Tracks InputContext lifecycle and ensures isolated per-context engine state
 * is cleanly destroyed when an InputContext is finalized by Fcitx5.
 */
class BanglaKeyContextProperty : public fcitx::InputContextProperty {
public:
    BanglaKeyContextProperty(banglakey::TypingEngine* engine, banglakey::ContextId ctxId) noexcept;
    ~BanglaKeyContextProperty() override;

private:
    banglakey::TypingEngine* engine_{nullptr};
    banglakey::ContextId ctxId_{0};
};

/**
 * Candidate word representation for Fcitx5 UI selection.
 */
class BanglaKeyCandidateWord : public fcitx::CandidateWord {
public:
    BanglaKeyCandidateWord(BanglaKeyAddon* addon, std::string text, size_t index);
    ~BanglaKeyCandidateWord() override = default;

    void select(fcitx::InputContext* inputContext) const override;

private:
    BanglaKeyAddon* addon_{nullptr};
    std::string text_{};
    size_t index_{0};
};

/**
 * BanglaKey input method addon implementation for Fcitx5.
 */
class BanglaKeyAddon : public fcitx::InputMethodEngineV2 {
public:
    explicit BanglaKeyAddon(fcitx::Instance* instance);
    ~BanglaKeyAddon() override;

    // Fcitx5 engine event callbacks
    void keyEvent(const fcitx::InputMethodEntry& entry, fcitx::KeyEvent& keyEvent) override;
    void activate(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) override;
    void deactivate(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) override;
    void reset(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) override;

    // Discovery and mode metadata
    std::vector<fcitx::InputMethodEntry> listInputMethods() override;
    std::string subMode(const fcitx::InputMethodEntry& entry, fcitx::InputContext& inputContext) override;
    std::string subModeLabelImpl(const fcitx::InputMethodEntry& entry, fcitx::InputContext& inputContext) override;

    // Callback invoked when a candidate is chosen via mouse click or hotkey
    void selectCandidate(fcitx::InputContext* ic, const std::string& candidateText, size_t index);

    [[nodiscard]] fcitx::Instance* instance() const noexcept { return instance_; }
    [[nodiscard]] banglakey::TypingEngine* engine() const noexcept { return engine_.get(); }

private:
    [[nodiscard]] bool isModeSwitchKey(const fcitx::KeyEvent& keyEvent, banglakey::Modifier mod) const noexcept;
    void handleModeSwitch(fcitx::InputContext* ic, banglakey::ContextId ctxId, fcitx::KeyEvent& keyEvent);

    fcitx::Instance* instance_{nullptr};
    std::unique_ptr<banglakey::TypingEngine> engine_{nullptr};
    fcitx::FactoryFor<BanglaKeyContextProperty> propFactory_;
};

class BanglaKeyAddonFactory : public fcitx::AddonFactory {
public:
    fcitx::AddonInstance* create(fcitx::AddonManager* manager) override;
};

} // namespace banglakey::fcitx5
