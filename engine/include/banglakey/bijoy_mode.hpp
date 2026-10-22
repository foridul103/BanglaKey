#pragma once

#include <banglakey/mode_handler.hpp>

namespace banglakey {

class BijoyModeHandler : public ModeHandler {
public:
    BijoyModeHandler() = default;
    ~BijoyModeHandler() override = default;

    [[nodiscard]] TypingMode mode() const noexcept override {
        return TypingMode::Bijoy;
    }

    EngineAction processKeyEvent(const KeyEvent& event, CompositionState& state, const EngineConfig& config) override;
    void reset(CompositionState& state) override;
};

} // namespace banglakey
