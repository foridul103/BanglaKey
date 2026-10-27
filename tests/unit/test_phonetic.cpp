#include <gtest/gtest.h>
#include <banglakey/engine.hpp>
#include <banglakey/unicode.hpp>

using namespace banglakey;

class PhoneticModeTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = TypingEngine::create(TypingMode::Phonetic);
        ASSERT_NE(engine, nullptr);
    }

    EngineAction pressKey(char c, Modifier mod = Modifier::None) {
        KeyEvent ev;
        ev.unicode = static_cast<uint32_t>(c);
        ev.keysym = static_cast<uint32_t>(c);
        ev.modifiers = mod;
        ev.is_release = false;
        return engine->processKeyEvent(ev);
    }

    EngineAction pressSpecialKey(uint32_t keysym, Modifier mod = Modifier::None) {
        KeyEvent ev;
        ev.unicode = 0;
        ev.keysym = keysym;
        ev.modifiers = mod;
        ev.is_release = false;
        return engine->processKeyEvent(ev);
    }

    std::string typeWord(std::string_view word) {
        std::string last_preedit;
        for (char c : word) {
            auto a = pressKey(c);
            last_preedit = a.preedit_text;
        }
        return last_preedit;
    }

    std::unique_ptr<TypingEngine> engine;
};

TEST_F(PhoneticModeTest, AcceptanceCases) {
    // 1. ami -> আমি
    EXPECT_EQ(typeWord("ami"), "আমি");
    auto a1 = pressSpecialKey(0x0020 /* Space */);
    EXPECT_EQ(a1.type, EngineActionType::Commit);
    EXPECT_EQ(a1.commit_text, "আমি ");

    // 2. bangla -> বাংলা
    EXPECT_EQ(typeWord("bangla"), "বাংলা");
    auto a2 = pressSpecialKey(0x0020 /* Space */);
    EXPECT_EQ(a2.type, EngineActionType::Commit);
    EXPECT_EQ(a2.commit_text, "বাংলা ");

    // 3. amar sonar bangla -> আমার সোনার বাংলা
    std::string phrase_output;

    typeWord("amar");
    auto p1 = pressSpecialKey(0x0020 /* Space */);
    phrase_output.append(p1.commit_text);

    typeWord("sonar");
    auto p2 = pressSpecialKey(0x0020 /* Space */);
    phrase_output.append(p2.commit_text);

    typeWord("bangla");
    auto p3 = pressSpecialKey(0xff0d /* Return */);
    phrase_output.append(p3.commit_text);

    EXPECT_EQ(phrase_output, "আমার সোনার বাংলা");
}

TEST_F(PhoneticModeTest, BasicWordTransliteration) {
    EXPECT_EQ(typeWord("ami"), "আমি");
    engine->reset();

    EXPECT_EQ(typeWord("bangla"), "বাংলা");
    engine->reset();

    EXPECT_EQ(typeWord("khoroch"), "খরচ");
    engine->reset();

    EXPECT_EQ(typeWord("ekhon"), "এখন");
    engine->reset();
}

TEST_F(PhoneticModeTest, ConsonantClustersAndJuktakkhor) {
    EXPECT_EQ(typeWord("kkhoma"), "ক্ষমা");
    engine->reset();

    EXPECT_EQ(typeWord("jnyan"), "জ্ঞান");
    engine->reset();

    EXPECT_EQ(typeWord("gnyan"), "জ্ঞান");
    engine->reset();

    EXPECT_EQ(typeWord("iccha"), "ইচ্ছা");
    engine->reset();

    EXPECT_EQ(typeWord("shanti"), "শান্তি");
    engine->reset();

    EXPECT_EQ(typeWord("bondhu"), "বন্ধু");
    engine->reset();

    EXPECT_EQ(typeWord("rosto"), "রস্ত");
    engine->reset();
}

TEST_F(PhoneticModeTest, KhandaTaAndSpecialCharacters) {
    // Backtick modifier for Khanda Ta: "t`" -> "ৎ"
    EXPECT_EQ(typeWord("ut`sob"), "উৎসব");
    engine->reset();

    EXPECT_EQ(typeWord("hoThat`"), "হঠাৎ");
    engine->reset();
}

TEST_F(PhoneticModeTest, Numerals) {
    EXPECT_EQ(typeWord("1971"), "১৯৭১");
    engine->reset();
}

TEST_F(PhoneticModeTest, SpaceAndReturnCommit) {
    typeWord("bangla");

    // Space commits with trailing space
    auto a1 = pressSpecialKey(0x0020 /* Space */);
    EXPECT_EQ(a1.type, EngineActionType::Commit);
    EXPECT_EQ(a1.commit_text, "বাংলা ");

    // Return commits without trailing space
    typeWord("desh");
    auto a2 = pressSpecialKey(0xff0d /* Return */);
    EXPECT_EQ(a2.type, EngineActionType::Commit);
    EXPECT_EQ(a2.commit_text, "দেশ");
}

TEST_F(PhoneticModeTest, BackspaceEditing) {
    typeWord("ami"); // "আমি"

    // Backspace deletes 'i', buffer becomes "am" -> "আম"
    auto a1 = pressSpecialKey(0xff08 /* Backspace */);
    EXPECT_EQ(a1.type, EngineActionType::Preedit);
    EXPECT_EQ(a1.preedit_text, "আম");

    // Backspace deletes 'm', buffer becomes "a" -> "আ"
    auto a2 = pressSpecialKey(0xff08 /* Backspace */);
    EXPECT_EQ(a2.type, EngineActionType::Preedit);
    EXPECT_EQ(a2.preedit_text, "আ");

    // Backspace deletes 'a', buffer becomes empty -> Reset action
    auto a3 = pressSpecialKey(0xff08 /* Backspace */);
    EXPECT_EQ(a3.type, EngineActionType::Reset);
}

TEST_F(PhoneticModeTest, CursorMovementAndInsertion) {
    typeWord("am"); // "আম"

    // Move cursor left: cursor moves to position 1 (between 'a' and 'm')
    auto a1 = pressSpecialKey(0xff51 /* Left Arrow */);
    EXPECT_EQ(a1.type, EngineActionType::Preedit);

    // Insert 'a' -> buffer becomes "aam" -> "আম"
    pressKey('a');
    EXPECT_EQ(engine->state().rawBuffer(), "aam");
}

TEST_F(PhoneticModeTest, CandidateNavigation) {
    typeWord("kkh"); // Candidate 0: "ক্ষ", Candidate 1: "kkh"
    EXPECT_GE(engine->state().candidates().size(), 2u);

    // Down arrow selects candidate 1
    auto a1 = pressSpecialKey(0xff54 /* Down Arrow */);
    EXPECT_EQ(a1.type, EngineActionType::Preedit);
    EXPECT_EQ(engine->state().selectedCandidateIndex(), 1u);

    // Up arrow goes back to candidate 0
    auto a2 = pressSpecialKey(0xff52 /* Up Arrow */);
    EXPECT_EQ(a2.type, EngineActionType::Preedit);
    EXPECT_EQ(engine->state().selectedCandidateIndex(), 0u);
}

TEST_F(PhoneticModeTest, PunctuationAutoCommit) {
    typeWord("sonar");

    // Comma commits the word + comma
    auto a = pressKey(',');
    EXPECT_EQ(a.type, EngineActionType::Commit);
    EXPECT_EQ(a.commit_text, "সোনার,");
}

TEST_F(PhoneticModeTest, EscapeCancellation) {
    typeWord("khoroch");

    // Escape clears preedit and cancels composition
    auto a = pressSpecialKey(0xff1b /* Escape */);
    EXPECT_EQ(a.type, EngineActionType::Reset);
    EXPECT_TRUE(engine->state().isEmpty());
}

TEST_F(PhoneticModeTest, MixedBengaliEnglishInput) {
    // Type Bengali word
    typeWord("ami");
    auto a1 = pressSpecialKey(0x0020 /* Space */);
    EXPECT_EQ(a1.commit_text, "আমি ");

    // Type English word "Linux" and choose raw ASCII candidate
    typeWord("Linux");
    // Candidate 1 is raw typed ASCII token
    engine->state(DEFAULT_CONTEXT_ID);
    auto a2 = pressSpecialKey(0xff54 /* Down Arrow */); // select candidate 1 ("Linux")
    EXPECT_EQ(engine->state().selectedCandidate(), "Linux");

    auto a3 = pressSpecialKey(0x0020 /* Space */);
    EXPECT_EQ(a3.commit_text, "Linux ");
}

TEST_F(PhoneticModeTest, ExactVowelSignCodepoints) {
    // 1. কি: 'k' + 'i' + Space -> "কি " (U+0995 U+09BF U+0020)
    typeWord("ki");
    auto s1 = pressSpecialKey(0x0020 /* Space */);
    EXPECT_EQ(s1.type, EngineActionType::Commit);
    EXPECT_EQ(s1.commit_text, "কি ");
    auto ki_cps = unicode::toCodepoints(s1.commit_text);
    ASSERT_EQ(ki_cps.size(), 3u);
    EXPECT_EQ(ki_cps[0], 0x0995u); // ক
    EXPECT_EQ(ki_cps[1], 0x09BFu); // ি
    EXPECT_EQ(ki_cps[2], 0x0020u); // Space

    // 2. কে: 'k' + 'e' + Space -> "কে " (U+0995 U+09C7 U+0020)
    typeWord("ke");
    auto s2 = pressSpecialKey(0x0020 /* Space */);
    EXPECT_EQ(s2.type, EngineActionType::Commit);
    EXPECT_EQ(s2.commit_text, "কে ");
    auto ke_cps = unicode::toCodepoints(s2.commit_text);
    ASSERT_EQ(ke_cps.size(), 3u);
    EXPECT_EQ(ke_cps[0], 0x0995u); // ক
    EXPECT_EQ(ke_cps[1], 0x09C7u); // ে
    EXPECT_EQ(ke_cps[2], 0x0020u); // Space

    // 3. কী: 'k' + 'I' + Space -> "কী " (U+0995 U+09C0 U+0020)
    typeWord("kI");
    auto s3 = pressSpecialKey(0x0020 /* Space */);
    EXPECT_EQ(s3.type, EngineActionType::Commit);
    EXPECT_EQ(s3.commit_text, "কী ");
    auto kii_cps = unicode::toCodepoints(s3.commit_text);
    ASSERT_EQ(kii_cps.size(), 3u);
    EXPECT_EQ(kii_cps[0], 0x0995u); // ক
    EXPECT_EQ(kii_cps[1], 0x09C0u); // ী
    EXPECT_EQ(kii_cps[2], 0x0020u); // Space

    // 4. কু: 'k' + 'u' + Space -> "কু " (U+0995 U+09C1 U+0020)
    typeWord("ku");
    auto s4 = pressSpecialKey(0x0020 /* Space */);
    EXPECT_EQ(s4.type, EngineActionType::Commit);
    EXPECT_EQ(s4.commit_text, "কু ");
    auto ku_cps = unicode::toCodepoints(s4.commit_text);
    ASSERT_EQ(ku_cps.size(), 3u);
    EXPECT_EQ(ku_cps[0], 0x0995u); // ক
    EXPECT_EQ(ku_cps[1], 0x09C1u); // ু
    EXPECT_EQ(ku_cps[2], 0x0020u); // Space

    // 5. Backspace during composition: "ki" -> backspace -> "ক" -> backspace -> Reset
    typeWord("ki");
    auto b1 = pressSpecialKey(0xff08 /* Backspace */);
    EXPECT_EQ(b1.type, EngineActionType::Preedit);
    EXPECT_EQ(b1.preedit_text, "ক");
    auto b2 = pressSpecialKey(0xff08 /* Backspace */);
    EXPECT_EQ(b2.type, EngineActionType::Reset);
    EXPECT_TRUE(engine->state().isEmpty());
}
