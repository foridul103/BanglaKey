#pragma once

#include <banglakey/types.hpp>
#include <banglakey/composition_state.hpp>
#include <banglakey/config.hpp>

namespace banglakey {

class ModeHandler {
public:
    virtual ~ModeHandler() = default;

    [[nodiscard]] virtual TypingMode mode() const noexcept = 0;
    virtual EngineAction processKeyEvent(const KeyEvent& event, CompositionState& state, const EngineConfig& config) = 0;
    virtual void reset(CompositionState& state) = 0;
};

} // namespace banglakey
