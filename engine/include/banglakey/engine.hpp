#pragma once

#include <memory>
#include <unordered_map>
#include <banglakey/types.hpp>
#include <banglakey/config.hpp>
#include <banglakey/composition_state.hpp>
#include <banglakey/mode_handler.hpp>

namespace banglakey {

class TypingEngine {
public:
    virtual ~TypingEngine() = default;

    // Mode management
    virtual void setMode(TypingMode mode) = 0;
    virtual void setMode(ContextId ctx, TypingMode mode) = 0;
    virtual EngineAction switchMode(TypingMode mode) = 0;
    virtual EngineAction switchMode(ContextId ctx, TypingMode mode) = 0;
    [[nodiscard]] virtual TypingMode mode() const noexcept = 0;
    [[nodiscard]] virtual TypingMode mode(ContextId ctx) const noexcept = 0;

    // Configuration
    virtual bool setConfig(const EngineConfig& config) = 0;
    [[nodiscard]] virtual const EngineConfig& config() const noexcept = 0;

    // Key event processing
    virtual EngineAction processKeyEvent(const KeyEvent& event) = 0;
    virtual EngineAction processKeyEvent(ContextId ctx, const KeyEvent& event) = 0;

    // State reset and context lifecycle
    virtual void reset() = 0;
    virtual void reset(ContextId ctx) = 0;
    virtual void destroyContext(ContextId ctx) = 0;

    // Inspection
    [[nodiscard]] virtual const CompositionState& state(ContextId ctx = DEFAULT_CONTEXT_ID) const = 0;

    // Factory methods
    [[nodiscard]] static std::unique_ptr<TypingEngine> create(TypingMode initial_mode = TypingMode::Phonetic);
    [[nodiscard]] static std::unique_ptr<TypingEngine> create(const EngineConfig& config);
};

class DefaultTypingEngine : public TypingEngine {
public:
    explicit DefaultTypingEngine(TypingMode initial_mode = TypingMode::Phonetic);
    explicit DefaultTypingEngine(EngineConfig config);
    ~DefaultTypingEngine() override = default;

    void setMode(TypingMode mode) override;
    void setMode(ContextId ctx, TypingMode mode) override;
    EngineAction switchMode(TypingMode mode) override;
    EngineAction switchMode(ContextId ctx, TypingMode mode) override;
    [[nodiscard]] TypingMode mode() const noexcept override;
    [[nodiscard]] TypingMode mode(ContextId ctx) const noexcept override;

    bool setConfig(const EngineConfig& config) override;
    [[nodiscard]] const EngineConfig& config() const noexcept override;

    EngineAction processKeyEvent(const KeyEvent& event) override;
    EngineAction processKeyEvent(ContextId ctx, const KeyEvent& event) override;

    void reset() override;
    void reset(ContextId ctx) override;
    void destroyContext(ContextId ctx) override;

    [[nodiscard]] const CompositionState& state(ContextId ctx = DEFAULT_CONTEXT_ID) const override;

private:
    struct Session {
        TypingMode mode{TypingMode::Phonetic};
        CompositionState state{};
        std::string pending_mode_switch_commit{};
    };

    Session& getOrCreateSession(ContextId ctx);
    [[nodiscard]] ModeHandler* getHandler(TypingMode mode) noexcept;

    EngineConfig config_{};
    mutable std::unordered_map<ContextId, Session> sessions_{};

    std::unique_ptr<ModeHandler> bijoy_handler_{};
    std::unique_ptr<ModeHandler> phonetic_handler_{};
};

} // namespace banglakey
