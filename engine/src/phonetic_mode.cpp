#include <banglakey/phonetic_mode.hpp>
#include <banglakey/unicode.hpp>
#include "../rules/phonetic_map.hpp"

#include <algorithm>

namespace banglakey {

void PhoneticModeHandler::reset(CompositionState& state) {
    state.reset();
}

std::string PhoneticModeHandler::transliterate(std::string_view input) const {
    if (input.empty()) {
        return "";
    }

    // Check deterministic lexicon first
    std::string_view lex = rules::lookupPhoneticLexicon(input);
    if (!lex.empty()) {
        return std::string(lex);
    }

    std::string result;
    size_t i = 0;
    const size_t len = input.size();
    bool last_was_consonant = false;

    const auto& conjuncts = rules::getPhoneticConjuncts();
    const auto& vowels = rules::getPhoneticVowels();

    while (i < len) {
        char c = input[i];

        // 1. Backtick '`' acts as an explicit joiner break or escape
        if (c == '`') {
            last_was_consonant = false;
            ++i;
            continue;
        }

        // 2. Check for multi-character consonant clusters
        bool matched_conjunct = false;
        for (const auto& rule : conjuncts) {
            size_t plen = rule.pattern.size();
            if (i + plen <= len && input.substr(i, plen) == rule.pattern) {
                result.append(rule.replacement);
                i += plen;
                last_was_consonant = true;
                matched_conjunct = true;
                break;
            }
        }
        if (matched_conjunct) {
            continue;
        }

        // 3. Check for vowels (handling contextual dependent Kar vs independent forms)
        bool matched_vowel = false;
        for (const auto& vrule : vowels) {
            size_t plen = vrule.pattern.size();
            if (i + plen <= len && input.substr(i, plen) == vrule.pattern) {
                if (last_was_consonant) {
                    result.append(vrule.dependent);
                } else {
                    result.append(vrule.independent);
                }
                i += plen;
                last_was_consonant = false;
                matched_vowel = true;
                break;
            }
        }
        if (matched_vowel) {
            continue;
        }

        // 4. Check for single consonants
        std::string_view scons = rules::getPhoneticSingleConsonant(c);
        if (!scons.empty()) {
            result.append(scons);
            i += 1;
            last_was_consonant = true;
            continue;
        }

        // 5. Bengali numerals (0-9)
        if (c >= '0' && c <= '9') {
            char32_t bnum = 0x09E6 + static_cast<char32_t>(c - '0');
            result.append(unicode::toUtf8(bnum));
            i += 1;
            last_was_consonant = false;
            continue;
        }

        // 6. Unmatched characters copied directly
        result.push_back(c);
        i += 1;
        last_was_consonant = false;
    }

    return result;
}

std::vector<std::string> PhoneticModeHandler::generateCandidates(std::string_view input) const {
    std::vector<std::string> cands;
    if (input.empty()) {
        return cands;
    }

    // Primary candidate
    std::string primary = transliterate(input);
    cands.push_back(primary);

    // If input is in lexicon, add algorithmic rule transliteration as alternate
    std::string_view lex = rules::lookupPhoneticLexicon(input);
    if (!lex.empty()) {
        // Compute algorithmic transliteration without lexicon
        std::string alt;
        size_t i = 0;
        const size_t len = input.size();
        bool last_c = false;
        const auto& conjuncts = rules::getPhoneticConjuncts();
        const auto& vowels = rules::getPhoneticVowels();

        while (i < len) {
            bool matched = false;
            for (const auto& r : conjuncts) {
                if (i + r.pattern.size() <= len && input.substr(i, r.pattern.size()) == r.pattern) {
                    alt.append(r.replacement);
                    i += r.pattern.size();
                    last_c = true;
                    matched = true;
                    break;
                }
            }
            if (matched) continue;
            for (const auto& v : vowels) {
                if (i + v.pattern.size() <= len && input.substr(i, v.pattern.size()) == v.pattern) {
                    alt.append(last_c ? v.dependent : v.independent);
                    i += v.pattern.size();
                    last_c = false;
                    matched = true;
                    break;
                }
            }
            if (matched) continue;
            std::string_view sc = rules::getPhoneticSingleConsonant(input[i]);
            if (!sc.empty()) {
                alt.append(sc);
                i += 1;
                last_c = true;
                continue;
            }
            alt.push_back(input[i]);
            i += 1;
            last_c = false;
        }

        if (!alt.empty() && alt != primary) {
            cands.push_back(std::move(alt));
        }
    }

    // Raw typed ASCII token as fallback candidate
    std::string raw(input);
    if (raw != primary) {
        cands.push_back(std::move(raw));
    }

    return cands;
}

EngineAction PhoneticModeHandler::processKeyEvent(const KeyEvent& event, CompositionState& state, const EngineConfig& config) {
    if (event.is_release) {
        return EngineAction::makeForward();
    }

    // Bypass system shortcuts (Ctrl, Alt, Super)
    if (hasModifier(event.modifiers, Modifier::Control) ||
        hasModifier(event.modifiers, Modifier::Alt) ||
        hasModifier(event.modifiers, Modifier::Super)) {
        if (!state.rawBuffer().empty()) {
            std::string commit = state.selectedCandidate();
            reset(state);
            return EngineAction::makeCommit(std::move(commit));
        }
        return EngineAction::makeForward();
    }

    // Handle Escape: cancel composition without committing
    if (event.keysym == 0xff1b /* XK_Escape */) {
        if (!state.rawBuffer().empty()) {
            reset(state);
            return EngineAction::makeReset();
        }
        return EngineAction::makeForward();
    }

    // Handle Enter / Return
    if (event.keysym == 0xff0d /* XK_Return */ || event.keysym == 0xff8d /* XK_KP_Enter */) {
        if (!state.rawBuffer().empty()) {
            std::string commit = state.selectedCandidate();
            reset(state);
            return EngineAction::makeCommit(std::move(commit));
        }
        return EngineAction::makeForward();
    }

    // Handle Space
    if (event.keysym == 0x0020 /* XK_space */) {
        if (!state.rawBuffer().empty()) {
            std::string commit = state.selectedCandidate();
            if (config.commit_on_space) {
                commit.push_back(' ');
            }
            reset(state);
            return EngineAction::makeCommit(std::move(commit));
        }
        return EngineAction::makeForward();
    }

    // Handle Backspace
    if (event.keysym == 0xff08 /* XK_BackSpace */ || event.keycode == 22) {
        if (!state.rawBuffer().empty()) {
            state.backspace();
            if (state.bufferLength() == 0) {
                reset(state);
                return EngineAction::makeReset();
            }
            std::string text = transliterate(state.rawBuffer());
            state.setPreeditText(text);
            state.setCandidates(generateCandidates(state.rawBuffer()));
            state.setPreeditCursor(static_cast<int>(text.size()));
            return EngineAction::makePreedit(text, state.candidates(), state.preeditCursor());
        }
        return EngineAction::makeForward();
    }

    // Handle Delete
    if (event.keysym == 0xffff /* XK_Delete */) {
        if (!state.rawBuffer().empty()) {
            state.deleteForward();
            if (state.bufferLength() == 0) {
                reset(state);
                return EngineAction::makeReset();
            }
            std::string text = transliterate(state.rawBuffer());
            state.setPreeditText(text);
            state.setCandidates(generateCandidates(state.rawBuffer()));
            return EngineAction::makePreedit(text, state.candidates());
        }
        return EngineAction::makeForward();
    }

    // Handle Left Arrow (cursor navigation within buffer)
    if (event.keysym == 0xff51 /* XK_Left */) {
        if (!state.rawBuffer().empty()) {
            state.moveCursorLeft();
            return EngineAction::makePreedit(state.preeditText(), state.candidates(), static_cast<int>(state.cursorPosition()));
        }
        return EngineAction::makeForward();
    }

    // Handle Right Arrow
    if (event.keysym == 0xff53 /* XK_Right */) {
        if (!state.rawBuffer().empty()) {
            state.moveCursorRight();
            return EngineAction::makePreedit(state.preeditText(), state.candidates(), static_cast<int>(state.cursorPosition()));
        }
        return EngineAction::makeForward();
    }

    // Handle Down Arrow (select next candidate)
    if (event.keysym == 0xff54 /* XK_Down */) {
        if (!state.candidates().empty() && !state.rawBuffer().empty()) {
            state.selectNextCandidate();
            return EngineAction::makePreedit(state.selectedCandidate(), state.candidates());
        }
        return EngineAction::makeForward();
    }

    // Handle Up Arrow (select previous candidate)
    if (event.keysym == 0xff52 /* XK_Up */) {
        if (!state.candidates().empty() && !state.rawBuffer().empty()) {
            state.selectPrevCandidate();
            return EngineAction::makePreedit(state.selectedCandidate(), state.candidates());
        }
        return EngineAction::makeForward();
    }

    // Extract ASCII char
    char c = 0;
    if (event.unicode >= 32 && event.unicode <= 126) {
        c = static_cast<char>(event.unicode);
    } else if (event.keysym >= 32 && event.keysym <= 126) {
        c = static_cast<char>(event.keysym);
    }

    if (c == 0) {
        return EngineAction::makeForward();
    }

    // Punctuation handling
    if (c == '.' || c == ',' || c == '?' || c == '!' || c == ';' || c == ':' || c == '"' || c == '\'') {
        if (config.commit_on_punctuation && !state.rawBuffer().empty()) {
            std::string commit = state.selectedCandidate();
            if (c == '.' && config.enable_dari_on_period) {
                commit.append("।");
            } else {
                commit.push_back(c);
            }
            reset(state);
            return EngineAction::makeCommit(std::move(commit));
        }
        if (state.rawBuffer().empty()) {
            return EngineAction::makeForward();
        }
    }

    // Buffer length limit check
    if (state.bufferLength() >= config.max_buffer_size) {
        return EngineAction::makePreedit(state.preeditText(), state.candidates(), state.preeditCursor());
    }

    // Insert character into buffer
    state.insertChar(c);
    std::string text = transliterate(state.rawBuffer());
    state.setPreeditText(text);
    state.setCandidates(generateCandidates(state.rawBuffer()));
    state.setPreeditCursor(static_cast<int>(text.size()));

    return EngineAction::makePreedit(text, state.candidates(), state.preeditCursor());
}

} // namespace banglakey
