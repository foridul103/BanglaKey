#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace banglakey {

enum class TypingMode {
    Bijoy,
    Phonetic
};

enum class Modifier : uint32_t {
    None    = 0,
    Shift   = 1 << 0,
    Control = 1 << 1,
    Alt     = 1 << 2,
    Super   = 1 << 3,
    CapsLock= 1 << 4
};

inline Modifier operator|(Modifier a, Modifier b) {
    return static_cast<Modifier>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline Modifier operator&(Modifier a, Modifier b) {
    return static_cast<Modifier>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool hasModifier(Modifier mask, Modifier flag) {
    return (static_cast<uint32_t>(mask) & static_cast<uint32_t>(flag)) != 0;
}

struct KeyEvent {
    uint32_t keycode{0};
    uint32_t keysym{0};
    uint32_t unicode{0};
    Modifier modifiers{Modifier::None};
    bool is_release{false};
};

enum class EngineActionType {
    None,
    Commit,
    Preedit,
    ForwardKey,
    Reset
};

struct EngineAction {
    EngineActionType type{EngineActionType::None};
    std::string commit_text{};
    std::string preedit_text{};
    std::vector<std::string> candidates{};
    int cursor_position{0};

    static EngineAction makeCommit(std::string text) {
        EngineAction action;
        action.type = EngineActionType::Commit;
        action.commit_text = std::move(text);
        return action;
    }

    static EngineAction makePreedit(std::string preedit, std::vector<std::string> cand = {}, int cursor = -1) {
        EngineAction action;
        action.type = EngineActionType::Preedit;
        action.preedit_text = std::move(preedit);
        action.candidates = std::move(cand);
        action.cursor_position = (cursor >= 0) ? cursor : static_cast<int>(action.preedit_text.size());
        return action;
    }

    static EngineAction makeForward() {
        EngineAction action;
        action.type = EngineActionType::ForwardKey;
        return action;
    }

    static EngineAction makeReset() {
        EngineAction action;
        action.type = EngineActionType::Reset;
        return action;
    }
};

} // namespace banglakey
