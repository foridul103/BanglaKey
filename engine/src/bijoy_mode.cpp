#include <banglakey/bijoy_mode.hpp>
#include <banglakey/unicode.hpp>
#include "../rules/bijoy_map.hpp"

namespace banglakey {

void BijoyModeHandler::reset(CompositionState& state) {
    state.reset();
}

EngineAction BijoyModeHandler::processKeyEvent(const KeyEvent& event, CompositionState& state, const EngineConfig& config) {
    if (event.is_release) {
        return EngineAction::makeForward();
    }

    // Bypass system shortcuts (Ctrl, Alt, Super)
    if (hasModifier(event.modifiers, Modifier::Control) ||
        hasModifier(event.modifiers, Modifier::Alt) ||
        hasModifier(event.modifiers, Modifier::Super)) {
        if (!state.pendingPreVowel().empty()) {
            std::string commit = state.pendingPreVowel();
            reset(state);
            return EngineAction::makeCommit(std::move(commit));
        }
        reset(state);
        return EngineAction::makeForward();
    }

    // Handle Backspace
    if (event.keysym == 0xff08 /* XK_BackSpace */ || event.keycode == 22) {
        if (state.pendingHasant()) {
            state.setPendingHasant(false);
            state.setPreeditText("");
            return EngineAction::makeReset();
        }
        if (!state.pendingPreVowel().empty()) {
            state.clearPendingPreVowel();
            state.setPreeditText("");
            return EngineAction::makeReset();
        }
        return EngineAction::makeForward();
    }

    // Handle Escape: cancel composition
    if (event.keysym == 0xff1b /* XK_Escape */) {
        if (state.pendingHasant() || !state.pendingPreVowel().empty()) {
            reset(state);
            return EngineAction::makeReset();
        }
        return EngineAction::makeForward();
    }

    // Handle Enter / Return
    if (event.keysym == 0xff0d /* XK_Return */ || event.keysym == 0xff8d /* XK_KP_Enter */) {
        if (!state.pendingPreVowel().empty()) {
            std::string commit = state.pendingPreVowel();
            reset(state);
            return EngineAction::makeCommit(std::move(commit));
        }
        if (state.pendingHasant()) {
            reset(state);
            return EngineAction::makeCommit("্");
        }
        reset(state);
        return EngineAction::makeForward();
    }

    // Handle Space
    if (event.keysym == 0x0020 /* XK_space */) {
        if (!state.pendingPreVowel().empty()) {
            std::string commit = state.pendingPreVowel() + " ";
            reset(state);
            return EngineAction::makeCommit(std::move(commit));
        }
        if (state.pendingHasant()) {
            reset(state);
            return EngineAction::makeCommit("্ ");
        }
        reset(state);
        return EngineAction::makeForward();
    }

    // Extract ASCII char representation
    char c = 0;
    if (event.unicode >= 32 && event.unicode <= 126) {
        c = static_cast<char>(event.unicode);
    } else if (event.keysym >= 32 && event.keysym <= 126) {
        c = static_cast<char>(event.keysym);
    }

    if (c == 0) {
        return EngineAction::makeForward();
    }

    bool shifted = hasModifier(event.modifiers, Modifier::Shift);

    // Case 1: Pending Hasanta from 'g' (Link key)
    if (state.pendingHasant()) {
        state.setPendingHasant(false);

        // Check if followed by a vowel kar to form an independent vowel
        std::string_view linkedVowel = rules::getBijoyLinkedVowel(c, shifted);
        if (!linkedVowel.empty()) {
            std::string commit;
            if (!state.pendingPreVowel().empty()) {
                commit = state.pendingPreVowel();
                commit.append(linkedVowel);
                state.clearPendingPreVowel();
            } else {
                commit = std::string(linkedVowel);
            }
            state.setPreeditText("");
            return EngineAction::makeCommit(std::move(commit));
        }

        // Consecutive 'g' -> emit explicit visible hasanta
        if (c == 'g' && !shifted) {
            state.setPreeditText("");
            return EngineAction::makeCommit("্");
        }

        // Resolving conjunct with a consonant
        std::string_view mapped_view = shifted ? rules::getBijoyShifted(c, config.bijoy_variant)
                                               : rules::getBijoyNormal(c, config.bijoy_variant);
        if (!mapped_view.empty()) {
            std::string commit;
            // Prevent duplicate virama if mapped character already begins with virama (e.g. Ro-phala "্র")
            if (mapped_view.rfind("্", 0) == 0) {
                commit = std::string(mapped_view);
            } else {
                commit = "্" + std::string(mapped_view);
            }

            if (!state.pendingPreVowel().empty()) {
                commit.append(state.pendingPreVowel());
                state.clearPendingPreVowel();
            }
            state.setPreeditText("");
            return EngineAction::makeCommit(std::move(commit));
        }

        // If unmapped, commit hasanta and forward character
        state.setPreeditText("");
        return EngineAction::makeCommit("্");
    }

    // Case 2: Normal mapping lookup
    std::string_view mapped_view = shifted ? rules::getBijoyShifted(c, config.bijoy_variant)
                                           : rules::getBijoyNormal(c, config.bijoy_variant);
    if (mapped_view.empty()) {
        if (!state.pendingPreVowel().empty()) {
            std::string prev = state.pendingPreVowel();
            state.clearPendingPreVowel();
            state.setPreeditText("");
            return EngineAction::makeCommit(std::move(prev));
        }
        return EngineAction::makeForward();
    }

    std::string mapped(mapped_view);

    // A. Hasanta link key ('g' -> "্")
    if (mapped == "্") {
        state.setPendingHasant(true);
        state.setPreeditText("্");
        return EngineAction::makePreedit("্");
    }

    // B. Reph ('A' -> "র্"): If pre-vowel is pending, emit Reph and keep pre-vowel pending for base consonant
    if (mapped == "র্") {
        return EngineAction::makeCommit("র্");
    }

    // C. Pre-vowel sign reordering: E-kar "ে" or OI-kar "ৈ"
    if (mapped == "ে" || mapped == "ৈ") {
        if (config.auto_reorder_bijoy_vowels) {
            if (!state.pendingPreVowel().empty()) {
                std::string prev = state.pendingPreVowel();
                state.setPendingPreVowel(mapped);
                state.setPreeditText(mapped);
                return EngineAction::makeCommit(std::move(prev));
            }
            state.setPendingPreVowel(mapped);
            state.setPreeditText(mapped);
            return EngineAction::makePreedit(mapped);
        }
        return EngineAction::makeCommit(mapped);
    }

    // D. Two-part vowel consolidation: "ে" followed by "া" -> "ো" (O-kar U+09CB)
    if (state.pendingPreVowel() == "ে" && mapped == "া") {
        state.setPendingPreVowel("ো");
        state.setPreeditText("ো");
        return EngineAction::makePreedit("ো");
    }

    // E. Resolving pending pre-vowel (e.g. "ে" + Consonant -> Consonant + "ে")
    if (!state.pendingPreVowel().empty()) {
        std::string commit = mapped + state.pendingPreVowel();
        state.clearPendingPreVowel();
        state.setPreeditText("");
        return EngineAction::makeCommit(std::move(commit));
    }

    // F. Direct character commit
    state.setPreeditText("");
    return EngineAction::makeCommit(std::move(mapped));
}

} // namespace banglakey
