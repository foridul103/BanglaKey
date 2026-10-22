#include <banglakey/composition_state.hpp>
#include <algorithm>

namespace banglakey {

void CompositionState::reset() noexcept {
    raw_buffer_.clear();
    cursor_pos_ = 0;
    preedit_text_.clear();
    preedit_cursor_ = -1;
    candidates_.clear();
    selected_candidate_ = 0;
    pending_pre_vowel_.clear();
    pending_hasant_ = false;
    preceding_consonant_.clear();
}

bool CompositionState::isEmpty() const noexcept {
    return raw_buffer_.empty() &&
           preedit_text_.empty() &&
           pending_pre_vowel_.empty() &&
           !pending_hasant_;
}

void CompositionState::setCursorPosition(size_t pos) noexcept {
    cursor_pos_ = std::min(pos, raw_buffer_.size());
}

bool CompositionState::insertChar(char c) {
    if (cursor_pos_ >= raw_buffer_.size()) {
        raw_buffer_.push_back(c);
        cursor_pos_ = raw_buffer_.size();
    } else {
        raw_buffer_.insert(raw_buffer_.begin() + static_cast<std::ptrdiff_t>(cursor_pos_), c);
        cursor_pos_++;
    }
    return true;
}

bool CompositionState::backspace() {
    if (cursor_pos_ == 0 || raw_buffer_.empty()) {
        return false;
    }
    raw_buffer_.erase(raw_buffer_.begin() + static_cast<std::ptrdiff_t>(cursor_pos_ - 1));
    cursor_pos_--;
    return true;
}

bool CompositionState::deleteForward() {
    if (cursor_pos_ >= raw_buffer_.size() || raw_buffer_.empty()) {
        return false;
    }
    raw_buffer_.erase(raw_buffer_.begin() + static_cast<std::ptrdiff_t>(cursor_pos_));
    return true;
}

bool CompositionState::moveCursorLeft() noexcept {
    if (cursor_pos_ > 0) {
        cursor_pos_--;
        return true;
    }
    return false;
}

bool CompositionState::moveCursorRight() noexcept {
    if (cursor_pos_ < raw_buffer_.size()) {
        cursor_pos_++;
        return true;
    }
    return false;
}

void CompositionState::moveCursorHome() noexcept {
    cursor_pos_ = 0;
}

void CompositionState::moveCursorEnd() noexcept {
    cursor_pos_ = raw_buffer_.size();
}

void CompositionState::selectCandidate(size_t index) noexcept {
    if (index < candidates_.size()) {
        selected_candidate_ = index;
    }
}

void CompositionState::selectNextCandidate() noexcept {
    if (!candidates_.empty()) {
        selected_candidate_ = (selected_candidate_ + 1) % candidates_.size();
    }
}

void CompositionState::selectPrevCandidate() noexcept {
    if (!candidates_.empty()) {
        selected_candidate_ = (selected_candidate_ == 0) ? (candidates_.size() - 1) : (selected_candidate_ - 1);
    }
}

std::string CompositionState::selectedCandidate() const {
    if (candidates_.empty()) {
        return preedit_text_;
    }
    if (selected_candidate_ < candidates_.size()) {
        return candidates_[selected_candidate_];
    }
    return candidates_[0];
}

} // namespace banglakey
