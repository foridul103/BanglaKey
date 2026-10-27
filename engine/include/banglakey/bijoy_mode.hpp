#pragma once

#include <banglakey/mode_handler.hpp>

namespace banglakey {

class BijoyModeHandler : public ModeHandler {
public:
    explicit BijoyModeHandler(TypingMode mode = TypingMode::BijoyUnicode) : mode_(mode) {}
    ~BijoyModeHandler() override = default;

    [[nodiscard]] TypingMode mode() const noexcept override {
        return mode_;
    }

    EngineAction processKeyEvent(const KeyEvent& event, CompositionState& state, const EngineConfig& config) override;
    void reset(CompositionState& state) override;

private:
    TypingMode mode_{TypingMode::BijoyUnicode};
};

class BijoyUnicodeHandler : public BijoyModeHandler {
public:
    BijoyUnicodeHandler() : BijoyModeHandler(TypingMode::BijoyUnicode) {}
};

class BijoyClassicHandler : public BijoyModeHandler {
public:
    BijoyClassicHandler() : BijoyModeHandler(TypingMode::BijoyClassic) {}
};

} // namespace banglakey
