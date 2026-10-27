#include <gtest/gtest.h>
#include <banglakey/unicode.hpp>

using namespace banglakey::unicode;

TEST(UnicodeTest, CharacterClassification) {
    // Independent vowels
    EXPECT_TRUE(isIndependentVowel(CP_LETTER_A));
    EXPECT_TRUE(isIndependentVowel(CP_LETTER_AA));
    EXPECT_TRUE(isIndependentVowel(CP_LETTER_I));
    EXPECT_TRUE(isIndependentVowel(CP_LETTER_E));
    EXPECT_TRUE(isIndependentVowel(CP_LETTER_O));
    EXPECT_FALSE(isIndependentVowel(CP_LETTER_KA));

    // Consonants
    EXPECT_TRUE(isConsonant(CP_LETTER_KA));
    EXPECT_TRUE(isConsonant(CP_LETTER_RA));
    EXPECT_TRUE(isConsonant(CP_LETTER_KHANDA_TA));
    EXPECT_FALSE(isConsonant(CP_LETTER_A));
    EXPECT_FALSE(isConsonant(CP_SIGN_AA));

    // Vowel Signs (Kar)
    EXPECT_TRUE(isVowelSign(CP_SIGN_AA));
    EXPECT_TRUE(isVowelSign(CP_SIGN_I));
    EXPECT_TRUE(isVowelSign(CP_SIGN_E));
    EXPECT_TRUE(isVowelSign(CP_SIGN_AI));
    EXPECT_TRUE(isVowelSign(CP_SIGN_O));
    EXPECT_FALSE(isVowelSign(CP_LETTER_A));

    // Pre-vowel signs (I-kar, E-kar, OI-kar)
    EXPECT_TRUE(isPreVowelSign(CP_SIGN_I));
    EXPECT_TRUE(isPreVowelSign(CP_SIGN_E));
    EXPECT_TRUE(isPreVowelSign(CP_SIGN_AI));
    EXPECT_FALSE(isPreVowelSign(CP_SIGN_AA));

    // Modifiers & Virama
    EXPECT_TRUE(isModifier(CP_CANDRABINDU));
    EXPECT_TRUE(isModifier(CP_ANUSVARA));
    EXPECT_TRUE(isModifier(CP_VISARGA));
    EXPECT_TRUE(isVirama(CP_SIGN_VIRAMA));

    // Digits
    EXPECT_TRUE(isDigit(0x09E6)); // ০
    EXPECT_TRUE(isDigit(0x09EF)); // ৯
    EXPECT_FALSE(isDigit(CP_LETTER_KA));
}

TEST(UnicodeTest, Utf8ConversionAndCounting) {
    // Single codepoint conversion
    EXPECT_EQ(toUtf8(CP_LETTER_KA), "ক");
    EXPECT_EQ(toUtf8(CP_LETTER_A), "অ");
    EXPECT_EQ(toUtf8(CP_DANDA), "।");

    // Codepoints to string
    std::vector<char32_t> cps = {CP_LETTER_KA, CP_SIGN_VIRAMA, CP_LETTER_KA};
    EXPECT_EQ(toUtf8(cps), "ক্ক");

    // String to codepoints
    auto decoded = toCodepoints("বাংলা");
    ASSERT_EQ(decoded.size(), 5u);
    EXPECT_EQ(decoded[0], 0x09AC); // ব
    EXPECT_EQ(decoded[1], 0x09BE); // া
    EXPECT_EQ(decoded[2], 0x0982); // ং
    EXPECT_EQ(decoded[3], 0x09B2); // ল
    EXPECT_EQ(decoded[4], 0x09BE); // া

    // Codepoint count
    EXPECT_EQ(codepointCount("বাংলা"), 5u);
    EXPECT_EQ(codepointCount("ক্ক"), 3u); // ক + ্ + ক
}

TEST(UnicodeTest, TwoPartVowelConsolidation) {
    // E-kar + Aa-kar -> O-kar (ো)
    EXPECT_EQ(combineTwoPartVowel(CP_SIGN_E, CP_SIGN_AA), CP_SIGN_O);
    EXPECT_EQ(toUtf8(combineTwoPartVowel(CP_SIGN_E, CP_SIGN_AA)), "ো");

    // E-kar + AU-length-mark -> OU-kar (ৌ)
    EXPECT_EQ(combineTwoPartVowel(CP_SIGN_E, CP_AU_LENGTH_MARK), CP_SIGN_AU);
    EXPECT_EQ(toUtf8(combineTwoPartVowel(CP_SIGN_E, CP_AU_LENGTH_MARK)), "ৌ");

    // Unrelated combination
    EXPECT_EQ(combineTwoPartVowel(CP_SIGN_AA, CP_SIGN_E), 0u);
}

TEST(UnicodeTest, SyllableHelpers) {
    // Conjunct
    EXPECT_EQ(formConjunct("ক", "ক"), "ক্ক");
    EXPECT_EQ(formConjunct("ন", "দ"), "ন্দ");

    // Reph
    EXPECT_EQ(formReph("ম"), "র্ম");

    // Explicit Hasanta
    std::string explicitH = formExplicitHasanta("হ");
    auto cps = toCodepoints(explicitH);
    ASSERT_EQ(cps.size(), 3u);
    EXPECT_EQ(cps[0], 0x09B9); // হ
    EXPECT_EQ(cps[1], CP_SIGN_VIRAMA); // ্
    EXPECT_EQ(cps[2], CP_ZWNJ);        // ZWNJ
}
