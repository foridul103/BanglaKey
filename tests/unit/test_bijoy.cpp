#include <gtest/gtest.h>
#include <banglakey/engine.hpp>

using namespace banglakey;

class BijoyModeTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = TypingEngine::create(TypingMode::Bijoy);
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

    std::unique_ptr<TypingEngine> engine;
};

TEST_F(BijoyModeTest, NormalKeyMapping) {
    // Consonants
    EXPECT_EQ(pressKey('j').commit_text, "ক");
    EXPECT_EQ(pressKey('h').commit_text, "ব");
    EXPECT_EQ(pressKey('v').commit_text, "ল");
    EXPECT_EQ(pressKey('r').commit_text, "র");
    EXPECT_EQ(pressKey('m').commit_text, "ম");

    // Vowel signs
    EXPECT_EQ(pressKey('f').commit_text, "া");
    EXPECT_EQ(pressKey('d').commit_text, "ি");
    EXPECT_EQ(pressKey('s').commit_text, "ু");
    EXPECT_EQ(pressKey('a').commit_text, "ৃ");

    // Khanda Ta on backslash
    EXPECT_EQ(pressKey('\\').commit_text, "ৎ");

    // Digits
    EXPECT_EQ(pressKey('1').commit_text, "১");
    EXPECT_EQ(pressKey('9').commit_text, "৯");

    // Quotes
    EXPECT_EQ(pressKey('`').commit_text, "‘");
    EXPECT_EQ(pressKey('\'').commit_text, "’");
}

TEST_F(BijoyModeTest, ShiftedKeyMapping) {
    // Shifted consonants
    EXPECT_EQ(pressKey('J', Modifier::Shift).commit_text, "খ");
    EXPECT_EQ(pressKey('H', Modifier::Shift).commit_text, "ভ");
    EXPECT_EQ(pressKey('F', Modifier::Shift).commit_text, "অ");
    EXPECT_EQ(pressKey('A', Modifier::Shift).commit_text, "র্");

    // Shifted signs & punctuation
    EXPECT_EQ(pressKey('G', Modifier::Shift).commit_text, "।"); // Dari
    EXPECT_EQ(pressKey('4', Modifier::Shift).commit_text, "৳"); // Taka sign
    EXPECT_EQ(pressKey('&', Modifier::Shift).commit_text, "ঁ"); // Candrabindu
    EXPECT_EQ(pressKey('*', Modifier::Shift).commit_text, "ঃ"); // Visarga
    EXPECT_EQ(pressKey('|', Modifier::Shift).commit_text, "ঃ"); // Visarga on pipe
    EXPECT_EQ(pressKey('~', Modifier::Shift).commit_text, "“"); // Double quote left
    EXPECT_EQ(pressKey('"', Modifier::Shift).commit_text, "”"); // Double quote right
}

TEST_F(BijoyModeTest, UniJoyVariantDifferences) {
    // Verify default Classic Bijoy layout: 'r' -> "র", 'p' -> "প", 'v' -> "ল"
    EXPECT_EQ(pressKey('r').commit_text, "র");
    EXPECT_EQ(pressKey('p').commit_text, "প");
    EXPECT_EQ(pressKey('v').commit_text, "ল");

    // Switch to UniJoy variant
    EngineConfig cfg;
    cfg.default_mode = TypingMode::Bijoy;
    cfg.bijoy_variant = BijoyVariant::UniJoy;
    engine->setConfig(cfg);

    // In UniJoy: 'v' -> "র", 'r' -> "প", 'p' -> "ড়"
    EXPECT_EQ(pressKey('v').commit_text, "র");
    EXPECT_EQ(pressKey('r').commit_text, "প");
    EXPECT_EQ(pressKey('p').commit_text, "ড়");
}

TEST_F(BijoyModeTest, LinkKeyConjunctFormation) {
    // 'j' ('ক')
    auto a1 = pressKey('j');
    EXPECT_EQ(a1.type, EngineActionType::Commit);
    EXPECT_EQ(a1.commit_text, "ক");

    // 'g' (Link key -> Hasanta preedit)
    auto a2 = pressKey('g');
    EXPECT_EQ(a2.type, EngineActionType::Preedit);
    EXPECT_EQ(a2.preedit_text, "্");

    // 'j' ('ক') -> Forms conjunct 'ক্ক'
    auto a3 = pressKey('j');
    EXPECT_EQ(a3.type, EngineActionType::Commit);
    EXPECT_EQ(a3.commit_text, "্" "ক");

    // 'k' ('ত') + 'g' + 'k' ('ত') -> '্ত'
    pressKey('k');
    pressKey('g');
    auto a4 = pressKey('k');
    EXPECT_EQ(a4.commit_text, "্ত");
}

TEST_F(BijoyModeTest, TripleConjunctFormation) {
    // 'n' ('স') + 'g' + 'k' ('ত') + 'g' + 'r' ('র') -> 'স্ত্র'
    pressKey('n');
    pressKey('g');
    pressKey('k');
    pressKey('g');
    auto a = pressKey('r');
    EXPECT_EQ(a.commit_text, "্" "র");
}

TEST_F(BijoyModeTest, ConsecutiveLinkKeysExplicitHasanta) {
    // Pressing 'g' then 'g' produces explicit hasanta
    pressKey('g');
    auto a = pressKey('g');
    EXPECT_EQ(a.type, EngineActionType::Commit);
    EXPECT_EQ(a.commit_text, "্");
}

TEST_F(BijoyModeTest, LinkKeyFormsIndependentVowels) {
    // 'g' + 'f' -> 'আ'
    pressKey('g');
    EXPECT_EQ(pressKey('f').commit_text, "আ");

    // 'g' + 'd' -> 'ই'
    pressKey('g');
    EXPECT_EQ(pressKey('d').commit_text, "ই");

    // 'g' + Shift+'D' -> 'ঈ'
    pressKey('g');
    EXPECT_EQ(pressKey('D', Modifier::Shift).commit_text, "ঈ");

    // 'g' + 's' -> 'উ'
    pressKey('g');
    EXPECT_EQ(pressKey('s').commit_text, "উ");

    // 'g' + 'c' -> 'এ'
    pressKey('g');
    EXPECT_EQ(pressKey('c').commit_text, "এ");

    // 'g' + Shift+'G' -> Double Dari '॥'
    pressKey('g');
    EXPECT_EQ(pressKey('G', Modifier::Shift).commit_text, "॥");
}

TEST_F(BijoyModeTest, PreVowelSignReordering) {
    // E-kar 'c' ('ে') typed before consonant 'j' ('ক')
    auto a1 = pressKey('c');
    EXPECT_EQ(a1.type, EngineActionType::Preedit);
    EXPECT_EQ(a1.preedit_text, "ে");

    // Consonant 'j' typed -> commits "কে" in canonical Unicode order!
    auto a2 = pressKey('j');
    EXPECT_EQ(a2.type, EngineActionType::Commit);
    EXPECT_EQ(a2.commit_text, "কে");

    // OI-kar Shift+'C' ('ৈ') before consonant 'l' ('দ') -> commits "দৈ"
    auto a3 = pressKey('C', Modifier::Shift);
    EXPECT_EQ(a3.type, EngineActionType::Preedit);
    EXPECT_EQ(a3.preedit_text, "ৈ");

    auto a4 = pressKey('l');
    EXPECT_EQ(a4.type, EngineActionType::Commit);
    EXPECT_EQ(a4.commit_text, "দৈ");
}

TEST_F(BijoyModeTest, RephWithPreVowelSign) {
    // 'c' (E-kar) + 'A' (Reph 'র্') + 'm' ('ম') -> "র্মে"
    pressKey('c'); // E-kar pending
    auto a1 = pressKey('A', Modifier::Shift); // Reph emitted
    EXPECT_EQ(a1.commit_text, "র্");

    auto a2 = pressKey('m'); // Consonant 'ম' resolves E-kar -> "মে"
    EXPECT_EQ(a2.commit_text, "মে");
}

TEST_F(BijoyModeTest, TwoPartVowelConsolidation) {
    // 'c' ('ে') + 'f' ('া') -> consolidates to 'ো' (O-kar U+09CB)
    pressKey('c');
    auto a1 = pressKey('f');
    EXPECT_EQ(a1.type, EngineActionType::Preedit);
    EXPECT_EQ(a1.preedit_text, "ো");

    // Consonant 'j' -> commits "কো"
    auto a2 = pressKey('j');
    EXPECT_EQ(a2.type, EngineActionType::Commit);
    EXPECT_EQ(a2.commit_text, "কো");
}

TEST_F(BijoyModeTest, BackspaceCancelsPendingStates) {
    // Backspace during pending hasanta clears it
    pressKey('g');
    auto a1 = pressSpecialKey(0xff08 /* Backspace */);
    EXPECT_EQ(a1.type, EngineActionType::Reset);

    // Backspace during pending pre-vowel clears it
    pressKey('c');
    auto a2 = pressSpecialKey(0xff08 /* Backspace */);
    EXPECT_EQ(a2.type, EngineActionType::Reset);
}

TEST_F(BijoyModeTest, EscapeAndSpaceFlushing) {
    // Escape clears pre-vowel
    pressKey('c');
    auto a1 = pressSpecialKey(0xff1b /* Escape */);
    EXPECT_EQ(a1.type, EngineActionType::Reset);

    // Space flushes pre-vowel with a trailing space
    pressKey('c');
    auto a2 = pressSpecialKey(0x0020 /* Space */);
    EXPECT_EQ(a2.type, EngineActionType::Commit);
    EXPECT_EQ(a2.commit_text, "ে ");
}

TEST_F(BijoyModeTest, ShortcutPassthrough) {
    // Ctrl+C passes through without modification
    KeyEvent ev;
    ev.unicode = 'c';
    ev.keysym = 'c';
    ev.modifiers = Modifier::Control;
    auto a = engine->processKeyEvent(ev);
    EXPECT_EQ(a.type, EngineActionType::ForwardKey);
}
