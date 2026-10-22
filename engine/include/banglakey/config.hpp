#pragma once

#include <string>
#include <banglakey/types.hpp>

namespace banglakey {

enum class BijoyVariant {
    Classic,
    UniJoy
};

struct ValidationResult {
    bool is_valid{true};
    std::string error_message{};

    [[nodiscard]] explicit operator bool() const noexcept { return is_valid; }

    static ValidationResult success() {
        return {true, ""};
    }

    static ValidationResult failure(std::string message) {
        return {false, std::move(message)};
    }
};

struct EngineConfig {
    TypingMode default_mode{TypingMode::Phonetic};
    BijoyVariant bijoy_variant{BijoyVariant::Classic};
    bool commit_on_space{true};
    bool commit_on_punctuation{true};
    bool auto_reorder_bijoy_vowels{true};
    bool enable_dari_on_period{false};
    size_t max_buffer_size{128};
    size_t max_candidates{5};

    [[nodiscard]] ValidationResult validate() const;
    [[nodiscard]] bool isValid() const noexcept {
        return validate().is_valid;
    }
};

} // namespace banglakey
