#ifdef USE_GTEST
#include <gtest/gtest.h>
#else
#include <iostream>
#include <cassert>
#endif

#include <banglakey/banglakey.hpp>

#ifdef USE_GTEST

TEST(BanglaKeySmokeTest, EngineCreationAndModeSwitch) {
    auto engine = banglakey::TypingEngine::create(banglakey::TypingMode::Phonetic);
    ASSERT_NE(engine, nullptr);
    EXPECT_EQ(engine->mode(), banglakey::TypingMode::Phonetic);

    engine->setMode(banglakey::TypingMode::Bijoy);
    EXPECT_EQ(engine->mode(), banglakey::TypingMode::Bijoy);
}

TEST(BanglaKeySmokeTest, BijoyDirectMapping) {
    auto engine = banglakey::TypingEngine::create(banglakey::TypingMode::Bijoy);
    ASSERT_NE(engine, nullptr);

    // 'j' should map to Bengali Ko ("ক")
    banglakey::KeyEvent ev;
    ev.unicode = 'j';
    ev.keysym = 'j';
    ev.modifiers = banglakey::Modifier::None;

    auto action = engine->processKeyEvent(ev);
    EXPECT_EQ(action.type, banglakey::EngineActionType::Commit);
    EXPECT_EQ(action.commit_text, "ক");
}

TEST(BanglaKeySmokeTest, BijoyConjunctProcessing) {
    auto engine = banglakey::TypingEngine::create(banglakey::TypingMode::Bijoy);
    ASSERT_NE(engine, nullptr);

    // 'j' -> "ক"
    banglakey::KeyEvent ev1;
    ev1.unicode = 'j';
    ev1.keysym = 'j';
    auto a1 = engine->processKeyEvent(ev1);
    EXPECT_EQ(a1.type, banglakey::EngineActionType::Commit);
    EXPECT_EQ(a1.commit_text, "ক");

    // 'g' -> Hasanta preedit "্"
    banglakey::KeyEvent ev2;
    ev2.unicode = 'g';
    ev2.keysym = 'g';
    auto a2 = engine->processKeyEvent(ev2);
    EXPECT_EQ(a2.type, banglakey::EngineActionType::Preedit);
    EXPECT_EQ(a2.preedit_text, "্");

    // 'j' -> Consonant resolves conjunct -> "্ক"
    banglakey::KeyEvent ev3;
    ev3.unicode = 'j';
    ev3.keysym = 'j';
    auto a3 = engine->processKeyEvent(ev3);
    EXPECT_EQ(a3.type, banglakey::EngineActionType::Commit);
    EXPECT_EQ(a3.commit_text, "্ক");
}

TEST(BanglaKeySmokeTest, PhoneticTransliteration) {
    auto engine = banglakey::TypingEngine::create(banglakey::TypingMode::Phonetic);
    ASSERT_NE(engine, nullptr);

    // 'k' -> preedit "ক"
    banglakey::KeyEvent ev1;
    ev1.unicode = 'k';
    ev1.keysym = 'k';
    auto a1 = engine->processKeyEvent(ev1);
    EXPECT_EQ(a1.type, banglakey::EngineActionType::Preedit);
    EXPECT_EQ(a1.preedit_text, "ক");

    // 'h' -> preedit updates to "খ"
    banglakey::KeyEvent ev2;
    ev2.unicode = 'h';
    ev2.keysym = 'h';
    auto a2 = engine->processKeyEvent(ev2);
    EXPECT_EQ(a2.type, banglakey::EngineActionType::Preedit);
    EXPECT_EQ(a2.preedit_text, "খ");

    // Space commits "খ "
    banglakey::KeyEvent ev3;
    ev3.keysym = 0x0020;
    auto a3 = engine->processKeyEvent(ev3);
    EXPECT_EQ(a3.type, banglakey::EngineActionType::Commit);
    EXPECT_EQ(a3.commit_text, "খ ");
}

#else

int main() {
    std::cout << "[SmokeTest] Running BanglaKey engine standalone smoke tests..." << std::endl;

    auto engine = banglakey::TypingEngine::create(banglakey::TypingMode::Phonetic);
    assert(engine != nullptr);
    assert(engine->mode() == banglakey::TypingMode::Phonetic);

    engine->setMode(banglakey::TypingMode::Bijoy);
    assert(engine->mode() == banglakey::TypingMode::Bijoy);
    std::cout << "[SmokeTest] PASS: Mode switching" << std::endl;

    // Bijoy direct key mapping ('j' -> "ক")
    banglakey::KeyEvent ev1;
    ev1.unicode = 'j';
    ev1.keysym = 'j';
    auto a1 = engine->processKeyEvent(ev1);
    assert(a1.type == banglakey::EngineActionType::Commit);
    assert(a1.commit_text == "ক");
    std::cout << "[SmokeTest] PASS: Bijoy direct mapping 'j' -> 'ক'" << std::endl;

    // Bijoy conjunct: 'g' (link) + 'j' -> "্ক"
    banglakey::KeyEvent ev2;
    ev2.unicode = 'g';
    ev2.keysym = 'g';
    auto a2 = engine->processKeyEvent(ev2);
    assert(a2.type == banglakey::EngineActionType::Preedit);
    assert(a2.preedit_text == "্");

    banglakey::KeyEvent ev3;
    ev3.unicode = 'j';
    ev3.keysym = 'j';
    auto a3 = engine->processKeyEvent(ev3);
    assert(a3.type == banglakey::EngineActionType::Commit);
    assert(a3.commit_text == "্ক");
    std::cout << "[SmokeTest] PASS: Bijoy conjunct 'g' + 'j' -> '্ক'" << std::endl;

    // Phonetic transliteration: 'k' + 'h' -> "খ", Space -> Commit "খ "
    engine->setMode(banglakey::TypingMode::Phonetic);
    banglakey::KeyEvent p1;
    p1.unicode = 'k';
    p1.keysym = 'k';
    auto pa1 = engine->processKeyEvent(p1);
    assert(pa1.type == banglakey::EngineActionType::Preedit);
    assert(pa1.preedit_text == "ক");

    banglakey::KeyEvent p2;
    p2.unicode = 'h';
    p2.keysym = 'h';
    auto pa2 = engine->processKeyEvent(p2);
    assert(pa2.type == banglakey::EngineActionType::Preedit);
    assert(pa2.preedit_text == "খ");

    banglakey::KeyEvent p3;
    p3.keysym = 0x0020;
    auto pa3 = engine->processKeyEvent(p3);
    assert(pa3.type == banglakey::EngineActionType::Commit);
    assert(pa3.commit_text == "খ ");
    std::cout << "[SmokeTest] PASS: Phonetic transliteration 'kh' + Space -> 'খ '" << std::endl;

    std::cout << "[SmokeTest] All smoke tests passed successfully!" << std::endl;
    return 0;
}

#endif
