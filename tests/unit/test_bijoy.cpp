#include <gtest/gtest.h>
#include <banglakey/engine.hpp>
#include <banglakey/unicode.hpp>

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

// 1. Normal Physical Key Mappings (Exact layout from 232436413-Bijoy-Keyboard-Layout.pdf)
TEST_F(BijoyModeTest, NormalKeyMappingMatchesPdf) {
    // Row 1: Digits, Khanda Ta, Quotes
    EXPECT_EQ(pressKey('1').commit_text, "১");
    EXPECT_EQ(pressKey('2').commit_text, "২");
    EXPECT_EQ(pressKey('3').commit_text, "৩");
    EXPECT_EQ(pressKey('4').commit_text, "৪");
    EXPECT_EQ(pressKey('5').commit_text, "৫");
    EXPECT_EQ(pressKey('6').commit_text, "৬");
    EXPECT_EQ(pressKey('7').commit_text, "৭");
    EXPECT_EQ(pressKey('8').commit_text, "৮");
    EXPECT_EQ(pressKey('9').commit_text, "৯");
    EXPECT_EQ(pressKey('0').commit_text, "০");
    EXPECT_EQ(pressKey('\\').commit_text, "ৎ"); // Khanda Ta on backslash
    EXPECT_EQ(pressKey('`').commit_text, "‘");   // Left single quote on backquote

    // Row 2: QWERTY row
    EXPECT_EQ(pressKey('q').commit_text, "ঙ"); // Umo
    EXPECT_EQ(pressKey('w').commit_text, "য"); // Antostho-Zo
    EXPECT_EQ(pressKey('e').commit_text, "ড"); // Ddo
    EXPECT_EQ(pressKey('r').commit_text, "প"); // Po (Key R normal in PDF)
    EXPECT_EQ(pressKey('t').commit_text, "ট"); // Tto
    EXPECT_EQ(pressKey('y').commit_text, "চ"); // Cho
    EXPECT_EQ(pressKey('u').commit_text, "জ"); // Borgiyo-Jo
    EXPECT_EQ(pressKey('i').commit_text, "হ"); // Ho
    EXPECT_EQ(pressKey('o').commit_text, "গ"); // Go
    EXPECT_EQ(pressKey('p').commit_text, "ড়"); // Dda-bindu-ro (Key P normal in PDF)

    // Row 3: ASDF row
    EXPECT_EQ(pressKey('a').commit_text, "ৃ"); // Ri-kar
    EXPECT_EQ(pressKey('s').commit_text, "ু"); // Hrosho-U-kar

    // Hrosho-I-kar is a pre-vowel sign, so it enters preedit
    auto ikar = pressKey('d');
    EXPECT_EQ(ikar.type, EngineActionType::Preedit);
    EXPECT_EQ(ikar.preedit_text, "ি");
    pressSpecialKey(0xff08 /* Backspace */); // Clear preedit

    EXPECT_EQ(pressKey('f').commit_text, "া"); // Aa-kar
    EXPECT_EQ(pressKey('h').commit_text, "ব"); // Bo
    EXPECT_EQ(pressKey('j').commit_text, "ক"); // Ko
    EXPECT_EQ(pressKey('k').commit_text, "ত"); // To
    EXPECT_EQ(pressKey('l').commit_text, "দ"); // Do
    EXPECT_EQ(pressKey('\'').commit_text, "’"); // Right single quote

    // Row 4: ZXCV row
    EXPECT_EQ(pressKey('z').commit_text, "্র"); // Ro-phala
    EXPECT_EQ(pressKey('x').commit_text, "ও"); // Independent vowel O (Key X normal in PDF)
    EXPECT_EQ(pressKey('v').commit_text, "র"); // Ro (Key V normal in PDF)
    EXPECT_EQ(pressKey('b').commit_text, "ন"); // Donto-No
    EXPECT_EQ(pressKey('n').commit_text, "স"); // Donto-Sho
    EXPECT_EQ(pressKey('m').commit_text, "ম"); // Mo
}

// 2. Shifted Physical Key Mappings (Exact layout from 232436413-Bijoy-Keyboard-Layout.pdf)
TEST_F(BijoyModeTest, ShiftedKeyMappingMatchesPdf) {
    // Row 1: Symbols
    EXPECT_EQ(pressKey('4', Modifier::Shift).commit_text, "৳"); // Taka sign ($)
    EXPECT_EQ(pressKey('7', Modifier::Shift).commit_text, "ঁ"); // Candrabindu (&)
    EXPECT_EQ(pressKey('|', Modifier::Shift).commit_text, "ঃ"); // Visarga (|)
    EXPECT_EQ(pressKey('~', Modifier::Shift).commit_text, "“"); // Left double quote (~)
    EXPECT_EQ(pressKey('"', Modifier::Shift).commit_text, "”"); // Right double quote (")

    // Asterisk on Shift+8 passes through as *
    auto star = pressKey('*', Modifier::Shift);
    EXPECT_EQ(star.type, EngineActionType::ForwardKey);

    // Row 2: Shifted uppercase
    EXPECT_EQ(pressKey('Q', Modifier::Shift).commit_text, "ং"); // Anusvara
    EXPECT_EQ(pressKey('W', Modifier::Shift).commit_text, "য়"); // Antostho-Yo
    EXPECT_EQ(pressKey('E', Modifier::Shift).commit_text, "ঢ"); // Ddho
    EXPECT_EQ(pressKey('R', Modifier::Shift).commit_text, "ফ"); // Pho (Key R Shift in PDF)
    EXPECT_EQ(pressKey('T', Modifier::Shift).commit_text, "ঠ"); // Ttho
    EXPECT_EQ(pressKey('Y', Modifier::Shift).commit_text, "ছ"); // Chho
    EXPECT_EQ(pressKey('U', Modifier::Shift).commit_text, "ঝ"); // Jho
    EXPECT_EQ(pressKey('I', Modifier::Shift).commit_text, "ঞ"); // Iyo
    EXPECT_EQ(pressKey('O', Modifier::Shift).commit_text, "ঘ"); // Gho
    EXPECT_EQ(pressKey('P', Modifier::Shift).commit_text, "ঢ়"); // Ddha-bindu-ro (Key P Shift in PDF)

    // Row 3: Shifted uppercase
    EXPECT_EQ(pressKey('A', Modifier::Shift).commit_text, "র্"); // Reph
    EXPECT_EQ(pressKey('S', Modifier::Shift).commit_text, "ূ"); // Dirgho-U-kar
    EXPECT_EQ(pressKey('D', Modifier::Shift).commit_text, "ী"); // Dirgho-I-kar
    EXPECT_EQ(pressKey('F', Modifier::Shift).commit_text, "অ"); // Shoro-A (Key F Shift in PDF)
    EXPECT_EQ(pressKey('G', Modifier::Shift).commit_text, "।"); // Dari (Bengali full stop)
    EXPECT_EQ(pressKey('H', Modifier::Shift).commit_text, "ভ"); // Bho
    EXPECT_EQ(pressKey('J', Modifier::Shift).commit_text, "খ"); // Kho
    EXPECT_EQ(pressKey('K', Modifier::Shift).commit_text, "থ"); // Tho
    EXPECT_EQ(pressKey('L', Modifier::Shift).commit_text, "ধ"); // Dho

    // Row 4: Shifted uppercase
    EXPECT_EQ(pressKey('Z', Modifier::Shift).commit_text, "্য"); // Ja-phala
    EXPECT_EQ(pressKey('X', Modifier::Shift).commit_text, "ৌ"); // OU-kar (Key X Shift in PDF)

    // OI-kar is a pre-vowel sign, so it enters preedit
    auto oi = pressKey('C', Modifier::Shift);
    EXPECT_EQ(oi.type, EngineActionType::Preedit);
    EXPECT_EQ(oi.preedit_text, "ৈ");
    pressSpecialKey(0xff08 /* Backspace */); // Clear preedit

    EXPECT_EQ(pressKey('V', Modifier::Shift).commit_text, "ল"); // Lo (Key V Shift in PDF)
    EXPECT_EQ(pressKey('B', Modifier::Shift).commit_text, "ণ"); // Murdhonno-No
    EXPECT_EQ(pressKey('N', Modifier::Shift).commit_text, "ষ"); // Murdhonno-Sho
    EXPECT_EQ(pressKey('M', Modifier::Shift).commit_text, "শ"); // Talobbo-Sho
}

// 3. Complete Independent Vowel Formation via G / Keys (From PDF Table)
TEST_F(BijoyModeTest, SworobornoFormationMatchesPdfTable) {
    // অ: Shift + F
    EXPECT_EQ(pressKey('F', Modifier::Shift).commit_text, "অ");

    // আ: G + া ('g' + 'f')
    pressKey('g');
    EXPECT_EQ(pressKey('f').commit_text, "আ");

    // ই: G + ি ('g' + 'd')
    pressKey('g');
    EXPECT_EQ(pressKey('d').commit_text, "ই");

    // ঈ: G + ী ('g' + Shift+'D')
    pressKey('g');
    EXPECT_EQ(pressKey('D', Modifier::Shift).commit_text, "ঈ");

    // উ: G + ু ('g' + 's')
    pressKey('g');
    EXPECT_EQ(pressKey('s').commit_text, "উ");

    // ঊ: G + ূ ('g' + Shift+'S')
    pressKey('g');
    EXPECT_EQ(pressKey('S', Modifier::Shift).commit_text, "ঊ");

    // ঋ: G + ৃ ('g' + 'a')
    pressKey('g');
    EXPECT_EQ(pressKey('a').commit_text, "ঋ");

    // এ: G + ে ('g' + 'c')
    pressKey('g');
    EXPECT_EQ(pressKey('c').commit_text, "এ");

    // ঐ: G + ৈ ('g' + Shift+'C')
    pressKey('g');
    EXPECT_EQ(pressKey('C', Modifier::Shift).commit_text, "ঐ");

    // ও: X directly
    EXPECT_EQ(pressKey('x').commit_text, "ও");

    // ঔ: G + ৌ ('g' + Shift+'X')
    pressKey('g');
    EXPECT_EQ(pressKey('X', Modifier::Shift).commit_text, "ঔ");

    // Explicit Hasanta: G + G
    pressKey('g');
    auto hasant = pressKey('g');
    EXPECT_EQ(hasant.type, EngineActionType::Commit);
    EXPECT_EQ(hasant.commit_text, "্");

    // Double Dari: G + Shift+'G'
    pressKey('g');
    EXPECT_EQ(pressKey('G', Modifier::Shift).commit_text, "॥");
}

// 4. Post-Consonant Phalas & Kars Without G (From PDF Note)
TEST_F(BijoyModeTest, PostConsonantPhalasAndKarsWithoutG) {
    // ক্র = ক + ্র ('j' + 'z')
    std::string kro;
    kro.append(pressKey('j').commit_text);
    kro.append(pressKey('z').commit_text);
    EXPECT_EQ(kro, "ক্র");

    // ব্যা = ব + ্য + া ('h' + Shift+'Z' + 'f')
    std::string bya;
    bya.append(pressKey('h').commit_text);
    bya.append(pressKey('Z', Modifier::Shift).commit_text);
    bya.append(pressKey('f').commit_text);
    EXPECT_EQ(bya, "ব্যা");

    // বা = ব + া ('h' + 'f')
    std::string ba;
    ba.append(pressKey('h').commit_text);
    ba.append(pressKey('f').commit_text);
    EXPECT_EQ(ba, "বা");

    // হৃ = হ + ৃ ('i' + 'a')
    std::string hri;
    hri.append(pressKey('i').commit_text);
    hri.append(pressKey('a').commit_text);
    EXPECT_EQ(hri, "হৃ");

    // ত্র = ত + ্র ('k' + 'z')
    std::string tro;
    tro.append(pressKey('k').commit_text);
    tro.append(pressKey('z').commit_text);
    EXPECT_EQ(tro, "ত্র");

    // ত্রু = ত + ্র + ু ('k' + 'z' + 's')
    std::string tru;
    tru.append(pressKey('k').commit_text);
    tru.append(pressKey('z').commit_text);
    tru.append(pressKey('s').commit_text);
    EXPECT_EQ(tru, "ত্রু");

    // শ্রু = শ + ্র + ু (Shift+'M' + 'z' + 's')
    std::string shru;
    shru.append(pressKey('M', Modifier::Shift).commit_text);
    shru.append(pressKey('z').commit_text);
    shru.append(pressKey('s').commit_text);
    EXPECT_EQ(shru, "শ্রু");
}

// 5. Verified Conjunct Sequences from PDF Table (Using G)
TEST_F(BijoyModeTest, ConjunctSequencesFromPdfTable) {
    // ক্ষ = ক + G + ষ ('j' + 'g' + Shift+'N')
    pressKey('j');
    pressKey('g');
    EXPECT_EQ(pressKey('N', Modifier::Shift).commit_text, "্" "ষ");

    // হ্ম = হ + G + ম ('i' + 'g' + 'm')
    pressKey('i');
    pressKey('g');
    EXPECT_EQ(pressKey('m').commit_text, "্" "ম");

    // ক্ষ্ম = ক + G + ষ + G + ম
    pressKey('j');
    pressKey('g');
    pressKey('N', Modifier::Shift);
    pressKey('g');
    EXPECT_EQ(pressKey('m').commit_text, "্" "ম");

    // ঙ্গ = ঙ + G + গ ('q' + 'g' + 'o')
    pressKey('q');
    pressKey('g');
    EXPECT_EQ(pressKey('o').commit_text, "্" "গ");

    // জ্ঞ = জ + G + ঞ ('u' + 'g' + Shift+'I')
    pressKey('u');
    pressKey('g');
    EXPECT_EQ(pressKey('I', Modifier::Shift).commit_text, "্" "ঞ");

    // ঞ্চ = ঞ + G + চ (Shift+'I' + 'g' + 'y')
    pressKey('I', Modifier::Shift);
    pressKey('g');
    EXPECT_EQ(pressKey('y').commit_text, "্" "চ");

    // ঞ্ছ = ঞ + G + ছ (Shift+'I' + 'g' + Shift+'Y')
    pressKey('I', Modifier::Shift);
    pressKey('g');
    EXPECT_EQ(pressKey('Y', Modifier::Shift).commit_text, "্" "ছ");

    // ঞ্জ = ঞ + G + জ (Shift+'I' + 'g' + 'u')
    pressKey('I', Modifier::Shift);
    pressKey('g');
    EXPECT_EQ(pressKey('u').commit_text, "্" "জ");

    // ত্থ = ত + G + থ ('k' + 'g' + Shift+'K')
    pressKey('k');
    pressKey('g');
    EXPECT_EQ(pressKey('K', Modifier::Shift).commit_text, "্" "থ");

    // দ্ব = দ + G + ব ('l' + 'g' + 'h')
    pressKey('l');
    pressKey('g');
    EXPECT_EQ(pressKey('h').commit_text, "্" "ব");

    // স্থ = স + G + থ ('n' + 'g' + Shift+'K')
    pressKey('n');
    pressKey('g');
    EXPECT_EQ(pressKey('K', Modifier::Shift).commit_text, "্" "থ");

    // ষ্ণ = ষ + G + ণ (Shift+'N' + 'g' + Shift+'B')
    pressKey('N', Modifier::Shift);
    pressKey('g');
    EXPECT_EQ(pressKey('B', Modifier::Shift).commit_text, "্" "ণ");

    // ক্ত = ক + G + ত ('j' + 'g' + 'k')
    pressKey('j');
    pressKey('g');
    EXPECT_EQ(pressKey('k').commit_text, "্" "ত");

    // ট্ট = ট + G + ট ('t' + 'g' + 't')
    pressKey('t');
    pressKey('g');
    EXPECT_EQ(pressKey('t').commit_text, "্" "ট");

    // ত্ত = ত + G + ত ('k' + 'g' + 'k')
    pressKey('k');
    pressKey('g');
    EXPECT_EQ(pressKey('k').commit_text, "্" "ত");

    // দ্ম = দ + G + ম ('l' + 'g' + 'm')
    pressKey('l');
    pressKey('g');
    EXPECT_EQ(pressKey('m').commit_text, "্" "ম");

    // ন্ধ = ন + G + ধ ('b' + 'g' + Shift+'L')
    pressKey('b');
    pressKey('g');
    EXPECT_EQ(pressKey('L', Modifier::Shift).commit_text, "্" "ধ");

    // ন্থ = ন + G + থ ('b' + 'g' + Shift+'K')
    pressKey('b');
    pressKey('g');
    EXPECT_EQ(pressKey('K', Modifier::Shift).commit_text, "্" "থ");

    // ত্ম = ত + G + ম ('k' + 'g' + 'm')
    pressKey('k');
    pressKey('g');
    EXPECT_EQ(pressKey('m').commit_text, "্" "ম");

    // হ্ন = হ + G + ন ('i' + 'g' + 'b')
    pressKey('i');
    pressKey('g');
    EXPECT_EQ(pressKey('b').commit_text, "্" "ন");
}

// 6. Pre-vowel Sign Reordering and Consolidation
TEST_F(BijoyModeTest, PreVowelSignReorderingAndConsolidation) {
    // E-kar 'c' ('ে') before consonant 'j' ('ক') -> commits "কে"
    auto a1 = pressKey('c');
    EXPECT_EQ(a1.type, EngineActionType::Preedit);
    EXPECT_EQ(a1.preedit_text, "ে");

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

    // Two-part vowel: 'c' ('ে') + 'f' ('া') -> preedit 'ো', then consonant 'j' -> commits "কো"
    pressKey('c');
    auto a5 = pressKey('f');
    EXPECT_EQ(a5.type, EngineActionType::Preedit);
    EXPECT_EQ(a5.preedit_text, "ো");
    auto a6 = pressKey('j');
    EXPECT_EQ(a6.commit_text, "কো");

    // Two-part vowel: 'c' ('ে') + Shift+'X' ('ৌ') -> preedit 'ৌ', then consonant 'j' -> commits "কৌ"
    pressKey('c');
    auto a7 = pressKey('X', Modifier::Shift);
    EXPECT_EQ(a7.type, EngineActionType::Preedit);
    EXPECT_EQ(a7.preedit_text, "ৌ");
    auto a8 = pressKey('j');
    EXPECT_EQ(a8.commit_text, "কৌ");
}

// 7. Backspace and Reset Behavior
TEST_F(BijoyModeTest, BackspaceCancelsPendingStates) {
    pressKey('g');
    auto a1 = pressSpecialKey(0xff08 /* Backspace */);
    EXPECT_EQ(a1.type, EngineActionType::Reset);

    pressKey('c');
    auto a2 = pressSpecialKey(0xff08 /* Backspace */);
    EXPECT_EQ(a2.type, EngineActionType::Reset);
}
// 7.5 I-kar pre-vowel handling and regression tests
TEST_F(BijoyModeTest, IkarPreVowelHandling) {
    // 1. কি: 'd' (i-kar U+09BF) followed by consonant 'ক' ('j', U+0995) -> "কি" (U+0995 U+09BF)
    auto a1 = pressKey('d'); // i-kar
    EXPECT_EQ(a1.type, EngineActionType::Preedit);
    EXPECT_EQ(a1.preedit_text, "ি");
    auto a2 = pressKey('j'); // ক
    EXPECT_EQ(a2.type, EngineActionType::Commit);
    EXPECT_EQ(a2.commit_text, "কি");
    auto ki_cps = unicode::toCodepoints(a2.commit_text);
    ASSERT_EQ(ki_cps.size(), 2u);
    EXPECT_EQ(ki_cps[0], 0x0995u); // ক
    EXPECT_EQ(ki_cps[1], 0x09BFu); // ি

    // 2. কে: 'c' (e-kar U+09C7) followed by consonant 'ক' ('j', U+0995) -> "কে" (U+0995 U+09C7)
    auto c1 = pressKey('c');
    EXPECT_EQ(c1.type, EngineActionType::Preedit);
    EXPECT_EQ(c1.preedit_text, "ে");
    auto c2 = pressKey('j');
    EXPECT_EQ(c2.type, EngineActionType::Commit);
    EXPECT_EQ(c2.commit_text, "কে");
    auto ke_cps = unicode::toCodepoints(c2.commit_text);
    ASSERT_EQ(ke_cps.size(), 2u);
    EXPECT_EQ(ke_cps[0], 0x0995u); // ক
    EXPECT_EQ(ke_cps[1], 0x09C7u); // ে

    // 3. কী: 'j' (ক, U+0995) followed by 'D' (ii-kar U+09C0, Shift+'D') -> "কী" (U+0995 U+09C0)
    auto d1 = pressKey('j');
    EXPECT_EQ(d1.type, EngineActionType::Commit);
    EXPECT_EQ(d1.commit_text, "ক");
    auto d2 = pressKey('D', Modifier::Shift);
    EXPECT_EQ(d2.type, EngineActionType::Commit);
    EXPECT_EQ(d2.commit_text, "ী");
    std::string kii = d1.commit_text + d2.commit_text;
    auto kii_cps = unicode::toCodepoints(kii);
    ASSERT_EQ(kii_cps.size(), 2u);
    EXPECT_EQ(kii_cps[0], 0x0995u); // ক
    EXPECT_EQ(kii_cps[1], 0x09C0u); // ী

    // 4. কু: 'j' (ক, U+0995) followed by 's' (u-kar U+09C1) -> "কু" (U+0995 U+09C1)
    auto u1 = pressKey('j');
    EXPECT_EQ(u1.commit_text, "ক");
    auto u2 = pressKey('s');
    EXPECT_EQ(u2.commit_text, "ু");
    std::string ku = u1.commit_text + u2.commit_text;
    auto ku_cps = unicode::toCodepoints(ku);
    ASSERT_EQ(ku_cps.size(), 2u);
    EXPECT_EQ(ku_cps[0], 0x0995u); // ক
    EXPECT_EQ(ku_cps[1], 0x09C1u); // ু

    // 5. Successive pre-vowel replacement: pressing 'd' (ি) then 'c' (ে) commits previous "ি" and keeps "ে" in preedit
    auto p1 = pressKey('d');
    EXPECT_EQ(p1.type, EngineActionType::Preedit);
    EXPECT_EQ(p1.preedit_text, "ি");
    auto p2 = pressKey('c');
    EXPECT_EQ(p2.type, EngineActionType::Commit);
    EXPECT_EQ(p2.commit_text, "ি");
    EXPECT_EQ(engine->state().preeditText(), "ে");
    auto p3 = pressKey('j');
    EXPECT_EQ(p3.type, EngineActionType::Commit);
    EXPECT_EQ(p3.commit_text, "কে");

    // 6. Multiple different consonants with I-kar:
    // 'd' + 'k' -> "তি" (U+09A4 U+09BF)
    pressKey('d');
    auto ti = pressKey('k');
    EXPECT_EQ(ti.commit_text, "তি");
    auto ti_cps = unicode::toCodepoints(ti.commit_text);
    ASSERT_EQ(ti_cps.size(), 2u);
    EXPECT_EQ(ti_cps[0], 0x09A4u);
    EXPECT_EQ(ti_cps[1], 0x09BFu);

    // 'd' + 'l' -> "দি" (U+09A6 U+09BF)
    pressKey('d');
    auto di = pressKey('l');
    EXPECT_EQ(di.commit_text, "দি");

    // 'd' + 'b' -> "নি" (U+09A8 U+09BF)
    pressKey('d');
    auto ni = pressKey('b');
    EXPECT_EQ(ni.commit_text, "নি");

    // 'd' + 'n' -> "সি" (U+09B8 U+09BF)
    pressKey('d');
    auto si = pressKey('n');
    EXPECT_EQ(si.commit_text, "সি");

    // 7. Consecutive syllables: 'd' + 'j' + 'm' -> "কিম"
    pressKey('d');
    auto c_ki = pressKey('j');
    auto c_m = pressKey('m');
    EXPECT_EQ(c_ki.commit_text + c_m.commit_text, "কিম");

    // Consecutive identical: 'd' + 'j' + 'd' + 'j' -> "কিকি"
    pressKey('d');
    auto r1 = pressKey('j');
    pressKey('d');
    auto r2 = pressKey('j');
    EXPECT_EQ(r1.commit_text + r2.commit_text, "কিকি");

    // 8. Backspace during composition with 'd'
    pressKey('d');
    auto bs = pressSpecialKey(0xff08 /* Backspace */);
    EXPECT_EQ(bs.type, EngineActionType::Reset);
    EXPECT_TRUE(engine->state().isEmpty());

    // 9. Committing pre-vowel with Space
    pressKey('d');
    auto sp = pressSpecialKey(0x0020 /* Space */);
    EXPECT_EQ(sp.type, EngineActionType::Commit);
    EXPECT_EQ(sp.commit_text, "ি ");
}

// 8. CRITICAL: Bijoy Unicode Does Not Perform Avro Phonetic Transliteration
TEST_F(BijoyModeTest, BijoyUnicodeDoesNotPerformAvroTransliteration) {
    // Under the PDF layout:
    // 'a' -> 'ৃ'
    // 'm' -> 'ম'
    // 'a' -> 'ৃ'
    // 'r' -> 'প' (Key R in PDF is Po 'প'!)
    std::string output;
    output.append(pressKey('a').commit_text);
    output.append(pressKey('m').commit_text);
    output.append(pressKey('a').commit_text);
    output.append(pressKey('r').commit_text);

    EXPECT_NE(output, "আমার");
    EXPECT_EQ(output, "ৃমৃপ");

    // 'amav' ('v' is Ro 'র' in PDF) produces "ৃমৃর"
    std::string output_v;
    output_v.append(pressKey('a').commit_text);
    output_v.append(pressKey('m').commit_text);
    output_v.append(pressKey('a').commit_text);
    output_v.append(pressKey('v').commit_text);

    EXPECT_NE(output_v, "আমার");
    EXPECT_EQ(output_v, "ৃমৃর");
}

// 9. Bijoy Classic Mode Mappings (SutonnyMJ ANSI output)
TEST_F(BijoyModeTest, BijoyClassicMappingMatchesSutonnyMJ) {
    engine->setMode(TypingMode::BijoyClassic);

    // Consonants emit SutonnyMJ ANSI character codes
    EXPECT_EQ(pressKey('j').commit_text, "j"); // Ko
    EXPECT_EQ(pressKey('c').commit_text, "c"); // E-kar
    EXPECT_EQ(pressKey('k').commit_text, "k"); // To
    EXPECT_EQ(pressKey('f').commit_text, "f"); // Aa-kar
    EXPECT_EQ(pressKey('r').commit_text, "r"); // Po (Key R)
    EXPECT_EQ(pressKey('v').commit_text, "v"); // Ro (Key V)
    EXPECT_EQ(pressKey('p').commit_text, "p"); // Dda-bindu-ro (Key P)
    EXPECT_EQ(pressKey('x').commit_text, "x"); // Independent O (Key X)

    // Linked independent vowel via 'g'
    pressKey('g');
    auto a1 = pressKey('f'); // 'g' + 'f' -> "Av" (আ in SutonnyMJ)
    EXPECT_EQ(a1.commit_text, "Av");
}
