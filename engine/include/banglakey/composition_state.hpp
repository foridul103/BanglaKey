#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace banglakey {

using ContextId = uint64_t;
inline constexpr ContextId DEFAULT_CONTEXT_ID = 0;

class CompositionState {
public:
    CompositionState() = default;
    ~CompositionState() = default;

    // Buffer management
    void reset() noexcept;
    [[nodiscard]] bool isEmpty() const noexcept;
    [[nodiscard]] size_t bufferLength() const noexcept { return raw_buffer_.size(); }
    [[nodiscard]] const std::string& rawBuffer() const noexcept { return raw_buffer_; }

    // Cursor navigation within raw buffer
    [[nodiscard]] size_t cursorPosition() const noexcept { return cursor_pos_; }
    void setCursorPosition(size_t pos) noexcept;
    bool insertChar(char c);
    bool backspace();
    bool deleteForward();
    bool moveCursorLeft() noexcept;
    bool moveCursorRight() noexcept;
    void moveCursorHome() noexcept;
    void moveCursorEnd() noexcept;

    // Preedit text representation
    [[nodiscard]] const std::string& preeditText() const noexcept { return preedit_text_; }
    void setPreeditText(std::string text) { preedit_text_ = std::move(text); }

    [[nodiscard]] int preeditCursor() const noexcept { return preedit_cursor_; }
    void setPreeditCursor(int pos) noexcept { preedit_cursor_ = pos; }

    // Candidates management
    [[nodiscard]] const std::vector<std::string>& candidates() const noexcept { return candidates_; }
    void setCandidates(std::vector<std::string> cands) {
        candidates_ = std::move(cands);
        selected_candidate_ = 0;
    }
    [[nodiscard]] size_t selectedCandidateIndex() const noexcept { return selected_candidate_; }
    void selectCandidate(size_t index) noexcept;
    void selectNextCandidate() noexcept;
    void selectPrevCandidate() noexcept;
    [[nodiscard]] std::string selectedCandidate() const;

    // Bijoy state machine markers
    [[nodiscard]] const std::string& pendingPreVowel() const noexcept { return pending_pre_vowel_; }
    void setPendingPreVowel(std::string v) { pending_pre_vowel_ = std::move(v); }
    void clearPendingPreVowel() noexcept { pending_pre_vowel_.clear(); }

    [[nodiscard]] bool pendingHasant() const noexcept { return pending_hasant_; }
    void setPendingHasant(bool val) noexcept { pending_hasant_ = val; }

    [[nodiscard]] const std::string& precedingConsonant() const noexcept { return preceding_consonant_; }
    void setPrecedingConsonant(std::string c) { preceding_consonant_ = std::move(c); }
    void clearPrecedingConsonant() noexcept { preceding_consonant_.clear(); }

private:
    std::string raw_buffer_{};
    size_t cursor_pos_{0};

    std::string preedit_text_{};
    int preedit_cursor_{-1};

    std::vector<std::string> candidates_{};
    size_t selected_candidate_{0};

    // Bijoy layout specific state
    std::string pending_pre_vowel_{};
    bool pending_hasant_{false};
    std::string preceding_consonant_{};
};

} // namespace banglakey
