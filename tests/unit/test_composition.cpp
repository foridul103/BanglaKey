#include <gtest/gtest.h>
#include <banglakey/engine.hpp>

using namespace banglakey;

TEST(CompositionTest, PerContextIsolation) {
    auto engine = TypingEngine::create(TypingMode::Phonetic);
    ASSERT_NE(engine, nullptr);

    const ContextId ctx1 = 101;
    const ContextId ctx2 = 102;

    // Set Context 1 to Bijoy and Context 2 to Phonetic
    engine->setMode(ctx1, TypingMode::Bijoy);
    engine->setMode(ctx2, TypingMode::Phonetic);

    EXPECT_EQ(engine->mode(ctx1), TypingMode::Bijoy);
    EXPECT_EQ(engine->mode(ctx2), TypingMode::Phonetic);

    // Type in Context 1 (Bijoy: 'j' -> "ক")
    KeyEvent ev1;
    ev1.unicode = 'j';
    ev1.keysym = 'j';
    auto a1 = engine->processKeyEvent(ctx1, ev1);
    EXPECT_EQ(a1.type, EngineActionType::Commit);
    EXPECT_EQ(a1.commit_text, "ক");

    // Context 2 state must remain completely empty!
    EXPECT_TRUE(engine->state(ctx2).isEmpty());

    // Type in Context 2 (Phonetic: 'a' + 'm' + 'i' -> "আমি")
    KeyEvent ev2a; ev2a.unicode = 'a'; ev2a.keysym = 'a';
    KeyEvent ev2b; ev2b.unicode = 'm'; ev2b.keysym = 'm';
    KeyEvent ev2c; ev2c.unicode = 'i'; ev2c.keysym = 'i';
    engine->processKeyEvent(ctx2, ev2a);
    engine->processKeyEvent(ctx2, ev2b);
    auto a2 = engine->processKeyEvent(ctx2, ev2c);

    EXPECT_EQ(a2.type, EngineActionType::Preedit);
    EXPECT_EQ(a2.preedit_text, "আমি");
    EXPECT_EQ(engine->state(ctx2).rawBuffer(), "ami");

    // Context 1 state must remain unaffected by Context 2
    EXPECT_TRUE(engine->state(ctx1).isEmpty());

    // Reset Context 2
    engine->reset(ctx2);
    EXPECT_TRUE(engine->state(ctx2).isEmpty());

    // Destroy Context 1
    engine->destroyContext(ctx1);
    EXPECT_TRUE(engine->state(ctx1).isEmpty());
}

TEST(CompositionTest, ModeChangeDoesNotLoseUnfinishedText) {
    auto engine = TypingEngine::create(TypingMode::Phonetic);
    ASSERT_NE(engine, nullptr);

    // Type incomplete word in Phonetic mode: "bangla"
    for (char c : std::string("bangla")) {
        KeyEvent ev;
        ev.unicode = static_cast<uint32_t>(c);
        ev.keysym = static_cast<uint32_t>(c);
        engine->processKeyEvent(ev);
    }
    EXPECT_EQ(engine->state().preeditText(), "বাংলা");

    // Switch mode to Bijoy via switchMode: returns commit of unfinished text!
    auto switchAction = engine->switchMode(TypingMode::Bijoy);
    EXPECT_EQ(switchAction.type, EngineActionType::Commit);
    EXPECT_EQ(switchAction.commit_text, "বাংলা");
    EXPECT_EQ(engine->mode(), TypingMode::Bijoy);

    // Now in Bijoy mode, type 'j' -> commits "ক"
    KeyEvent evJ;
    evJ.unicode = 'j';
    evJ.keysym = 'j';
    auto a1 = engine->processKeyEvent(evJ);
    EXPECT_EQ(a1.type, EngineActionType::Commit);
    EXPECT_EQ(a1.commit_text, "ক");

    // Test setMode preserving unfinished text across next event
    // Type in Bijoy: 'c' (E-kar pending preedit)
    KeyEvent evC;
    evC.unicode = 'c';
    evC.keysym = 'c';
    engine->processKeyEvent(evC);
    EXPECT_EQ(engine->state().preeditText(), "ে");

    // Change mode to Phonetic via setMode
    engine->setMode(TypingMode::Phonetic);

    // Next key event in Phonetic mode must not lose the pending "ে"
    KeyEvent evK;
    evK.unicode = 'k';
    evK.keysym = 'k';
    auto a2 = engine->processKeyEvent(evK);
    // Unfinished "ে" was committed
    EXPECT_EQ(a2.commit_text, "ে");
    EXPECT_EQ(a2.preedit_text, "ক");
}

TEST(CompositionTest, CursorNavigationInState) {
    CompositionState state;
    EXPECT_TRUE(state.isEmpty());

    // Insert characters
    state.insertChar('a');
    state.insertChar('b');
    state.insertChar('c');
    EXPECT_EQ(state.rawBuffer(), "abc");
    EXPECT_EQ(state.cursorPosition(), 3u);

    // Move left
    EXPECT_TRUE(state.moveCursorLeft());
    EXPECT_EQ(state.cursorPosition(), 2u);

    // Insert at cursor: 'X' -> "abXc"
    state.insertChar('X');
    EXPECT_EQ(state.rawBuffer(), "abXc");
    EXPECT_EQ(state.cursorPosition(), 3u);

    // Backspace: deletes 'X' -> "abc"
    EXPECT_TRUE(state.backspace());
    EXPECT_EQ(state.rawBuffer(), "abc");
    EXPECT_EQ(state.cursorPosition(), 2u);

    // Delete forward: deletes 'c' -> "ab"
    EXPECT_TRUE(state.deleteForward());
    EXPECT_EQ(state.rawBuffer(), "ab");
    EXPECT_EQ(state.cursorPosition(), 2u);
}
