#include <banglakey/engine.hpp>
#include <banglakey/bijoy_mode.hpp>
#include <banglakey/phonetic_mode.hpp>

namespace banglakey {

std::unique_ptr<TypingEngine> TypingEngine::create(TypingMode initial_mode) {
    EngineConfig cfg;
    cfg.default_mode = initial_mode;
    return std::make_unique<DefaultTypingEngine>(cfg);
}

std::unique_ptr<TypingEngine> TypingEngine::create(const EngineConfig& config) {
    return std::make_unique<DefaultTypingEngine>(config);
}

DefaultTypingEngine::DefaultTypingEngine(TypingMode initial_mode)
    : bijoy_handler_(std::make_unique<BijoyModeHandler>()),
      phonetic_handler_(std::make_unique<PhoneticModeHandler>()) {
    config_.default_mode = initial_mode;
    getOrCreateSession(DEFAULT_CONTEXT_ID);
}

DefaultTypingEngine::DefaultTypingEngine(EngineConfig config)
    : config_(std::move(config)),
      bijoy_handler_(std::make_unique<BijoyModeHandler>()),
      phonetic_handler_(std::make_unique<PhoneticModeHandler>()) {
    getOrCreateSession(DEFAULT_CONTEXT_ID);
}

void DefaultTypingEngine::setMode(TypingMode mode) {
    config_.default_mode = mode;
    setMode(DEFAULT_CONTEXT_ID, mode);
}

void DefaultTypingEngine::setMode(ContextId ctx, TypingMode mode) {
    auto& session = getOrCreateSession(ctx);
    if (session.mode != mode) {
        // Mode changes must not silently lose unfinished text!
        if (!session.state.isEmpty()) {
            std::string commit = session.state.selectedCandidate();
            if (commit.empty()) {
                commit = session.state.preeditText();
            }
            if (commit.empty()) {
                commit = session.state.pendingPreVowel();
            }
            if (commit.empty() && session.state.pendingHasant()) {
                commit = "্";
            }
            session.pending_mode_switch_commit = std::move(commit);
        }
        session.state.reset();
        session.mode = mode;
    }
}

EngineAction DefaultTypingEngine::switchMode(TypingMode mode) {
    return switchMode(DEFAULT_CONTEXT_ID, mode);
}

EngineAction DefaultTypingEngine::switchMode(ContextId ctx, TypingMode mode) {
    auto& session = getOrCreateSession(ctx);
    if (session.mode == mode) {
        return EngineAction{};
    }

    std::string commit;
    if (!session.state.isEmpty()) {
        commit = session.state.selectedCandidate();
        if (commit.empty()) {
            commit = session.state.preeditText();
        }
        if (commit.empty()) {
            commit = session.state.pendingPreVowel();
        }
        if (commit.empty() && session.state.pendingHasant()) {
            commit = "্";
        }
    }

    session.state.reset();
    session.mode = mode;
    session.pending_mode_switch_commit.clear();

    if (!commit.empty()) {
        return EngineAction::makeCommit(std::move(commit));
    }
    return EngineAction{};
}

TypingMode DefaultTypingEngine::mode() const noexcept {
    return mode(DEFAULT_CONTEXT_ID);
}

TypingMode DefaultTypingEngine::mode(ContextId ctx) const noexcept {
    auto it = sessions_.find(ctx);
    if (it != sessions_.end()) {
        return it->second.mode;
    }
    return config_.default_mode;
}

bool DefaultTypingEngine::setConfig(const EngineConfig& config) {
    if (!config.validate().is_valid) {
        return false;
    }
    config_ = config;
    return true;
}

const EngineConfig& DefaultTypingEngine::config() const noexcept {
    return config_;
}

EngineAction DefaultTypingEngine::processKeyEvent(const KeyEvent& event) {
    return processKeyEvent(DEFAULT_CONTEXT_ID, event);
}

EngineAction DefaultTypingEngine::processKeyEvent(ContextId ctx, const KeyEvent& event) {
    auto& session = getOrCreateSession(ctx);
    ModeHandler* handler = getHandler(session.mode);
    if (!handler) {
        return EngineAction::makeForward();
    }

    // If there is uncommitted text from a mode change, flush/prefix it
    if (!session.pending_mode_switch_commit.empty()) {
        std::string prefix = std::move(session.pending_mode_switch_commit);
        session.pending_mode_switch_commit.clear();

        EngineAction act = handler->processKeyEvent(event, session.state, config_);
        if (act.type == EngineActionType::Commit) {
            act.commit_text = prefix + act.commit_text;
            return act;
        } else if (act.type == EngineActionType::Preedit) {
            act.commit_text = prefix; // Commit previous unfinished text, and display new preedit!
            return act;
        } else if (act.type == EngineActionType::ForwardKey || act.type == EngineActionType::None) {
            return EngineAction::makeCommit(std::move(prefix));
        }
    }

    return handler->processKeyEvent(event, session.state, config_);
}

void DefaultTypingEngine::reset() {
    reset(DEFAULT_CONTEXT_ID);
}

void DefaultTypingEngine::reset(ContextId ctx) {
    auto it = sessions_.find(ctx);
    if (it != sessions_.end()) {
        it->second.state.reset();
        it->second.pending_mode_switch_commit.clear();
    }
}

void DefaultTypingEngine::destroyContext(ContextId ctx) {
    sessions_.erase(ctx);
}

const CompositionState& DefaultTypingEngine::state(ContextId ctx) const {
    auto it = sessions_.find(ctx);
    if (it != sessions_.end()) {
        return it->second.state;
    }
    static const CompositionState empty_state{};
    return empty_state;
}

DefaultTypingEngine::Session& DefaultTypingEngine::getOrCreateSession(ContextId ctx) {
    auto it = sessions_.find(ctx);
    if (it == sessions_.end()) {
        Session s;
        s.mode = config_.default_mode;
        auto res = sessions_.emplace(ctx, std::move(s));
        return res.first->second;
    }
    return it->second;
}

ModeHandler* DefaultTypingEngine::getHandler(TypingMode mode) noexcept {
    switch (mode) {
        case TypingMode::Bijoy:
            return bijoy_handler_.get();
        case TypingMode::Phonetic:
            return phonetic_handler_.get();
    }
    return nullptr;
}

} // namespace banglakey
