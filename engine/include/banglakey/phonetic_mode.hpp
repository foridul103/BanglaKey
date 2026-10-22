#pragma once

#include <banglakey/mode_handler.hpp>
#include <string_view>
#include <string>
#include <vector>

namespace banglakey {

class PhoneticModeHandler : public ModeHandler {
public:
    PhoneticModeHandler() = default;
    ~PhoneticModeHandler() override = default;

    [[nodiscard]] TypingMode mode() const noexcept override {
        return TypingMode::Phonetic;
    }

    EngineAction processKeyEvent(const KeyEvent& event, CompositionState& state, const EngineConfig& config) override;
    void reset(CompositionState& state) override;

    [[nodiscard]] std::string transliterate(std::string_view input) const;
    [[nodiscard]] std::vector<std::string> generateCandidates(std::string_view input) const;
};

} // namespace banglakey
