#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace banglakey::unicode {

// Codepoints constants for Bengali block (U+0980 - U+09FF)
inline constexpr char32_t CP_CANDRABINDU       = 0x0981; // ঁ
inline constexpr char32_t CP_ANUSVARA          = 0x0982; // ং
inline constexpr char32_t CP_VISARGA           = 0x0983; // ঃ
inline constexpr char32_t CP_LETTER_A          = 0x0985; // অ
inline constexpr char32_t CP_LETTER_AA         = 0x0986; // আ
inline constexpr char32_t CP_LETTER_I          = 0x0987; // ই
inline constexpr char32_t CP_LETTER_II         = 0x0988; // ঈ
inline constexpr char32_t CP_LETTER_U          = 0x0989; // উ
inline constexpr char32_t CP_LETTER_UU         = 0x098A; // ঊ
inline constexpr char32_t CP_LETTER_VOCALIC_R  = 0x098B; // ঋ
inline constexpr char32_t CP_LETTER_E          = 0x098F; // এ
inline constexpr char32_t CP_LETTER_AI         = 0x0990; // ঐ
inline constexpr char32_t CP_LETTER_O          = 0x0993; // ও
inline constexpr char32_t CP_LETTER_AU         = 0x0994; // ঔ
inline constexpr char32_t CP_LETTER_KA         = 0x0995; // ক
inline constexpr char32_t CP_LETTER_RA         = 0x09B0; // র
inline constexpr char32_t CP_LETTER_KHANDA_TA  = 0x09CE; // ৎ
inline constexpr char32_t CP_SIGN_VIRAMA       = 0x09CD; // ্ (Hasanta)
inline constexpr char32_t CP_SIGN_AA           = 0x09BE; // া
inline constexpr char32_t CP_SIGN_I            = 0x09BF; // ি
inline constexpr char32_t CP_SIGN_II           = 0x09C0; // ী
inline constexpr char32_t CP_SIGN_U            = 0x09C1; // ু
inline constexpr char32_t CP_SIGN_UU           = 0x09C2; // ূ
inline constexpr char32_t CP_SIGN_VOCALIC_R    = 0x09C3; // ৃ
inline constexpr char32_t CP_SIGN_E            = 0x09C7; // ে
inline constexpr char32_t CP_SIGN_AI           = 0x09C8; // ৈ
inline constexpr char32_t CP_SIGN_O            = 0x09CB; // ো
inline constexpr char32_t CP_SIGN_AU           = 0x09CC; // ৌ
inline constexpr char32_t CP_AU_LENGTH_MARK    = 0x09D7; // ৗ
inline constexpr char32_t CP_DANDA             = 0x0964; // । (Dari)
inline constexpr char32_t CP_DOUBLE_DANDA      = 0x0965; // ॥
inline constexpr char32_t CP_TAK_SIGN          = 0x09F3; // ৳
inline constexpr char32_t CP_ZWJ               = 0x200D;
inline constexpr char32_t CP_ZWNJ              = 0x200C;

// Classification helpers
[[nodiscard]] constexpr bool isBengali(char32_t cp) noexcept {
    return (cp >= 0x0980 && cp <= 0x09FF) || cp == CP_DANDA || cp == CP_DOUBLE_DANDA;
}

[[nodiscard]] constexpr bool isIndependentVowel(char32_t cp) noexcept {
    return (cp >= 0x0985 && cp <= 0x0994) || cp == 0x09E0 || cp == 0x09E1;
}

[[nodiscard]] constexpr bool isConsonant(char32_t cp) noexcept {
    return (cp >= 0x0995 && cp <= 0x09B9) ||
           (cp >= 0x09DC && cp <= 0x09DF) ||
           cp == CP_LETTER_KHANDA_TA;
}

[[nodiscard]] constexpr bool isVowelSign(char32_t cp) noexcept {
    return (cp >= 0x09BE && cp <= 0x09CC) || cp == CP_AU_LENGTH_MARK;
}

[[nodiscard]] constexpr bool isPreVowelSign(char32_t cp) noexcept {
    return cp == CP_SIGN_I || cp == CP_SIGN_E || cp == CP_SIGN_AI;
}

[[nodiscard]] constexpr bool isModifier(char32_t cp) noexcept {
    return cp == CP_CANDRABINDU || cp == CP_ANUSVARA || cp == CP_VISARGA;
}

[[nodiscard]] constexpr bool isVirama(char32_t cp) noexcept {
    return cp == CP_SIGN_VIRAMA;
}

[[nodiscard]] constexpr bool isDigit(char32_t cp) noexcept {
    return cp >= 0x09E6 && cp <= 0x09EF;
}

// UTF-8 conversion and string manipulation
[[nodiscard]] std::string toUtf8(char32_t cp);
[[nodiscard]] std::string toUtf8(const std::vector<char32_t>& cps);
[[nodiscard]] std::vector<char32_t> toCodepoints(std::string_view utf8);
[[nodiscard]] size_t codepointCount(std::string_view utf8);
[[nodiscard]] std::string toHexCodepoints(std::string_view utf8);

// Two-part vowel consolidation (e.g. ে + া -> ো)
[[nodiscard]] char32_t combineTwoPartVowel(char32_t pre, char32_t post) noexcept;

// Syllable / Conjunct formation helpers
[[nodiscard]] std::string formConjunct(std::string_view c1, std::string_view c2);
[[nodiscard]] std::string formReph(std::string_view consonant);
[[nodiscard]] std::string formExplicitHasanta(std::string_view consonant);

} // namespace banglakey::unicode
