#pragma once

#include <string_view>
#include <banglakey/config.hpp>

namespace banglakey::rules {

// Lookup table for verified standard Bijoy-compatible layout mapping
inline std::string_view getBijoyNormal(char c, BijoyVariant variant = BijoyVariant::Classic) {
    switch (c) {
        // Digits
        case '0': return "০";
        case '1': return "১";
        case '2': return "২";
        case '3': return "৩";
        case '4': return "৪";
        case '5': return "৫";
        case '6': return "৬";
        case '7': return "৭";
        case '8': return "৮";
        case '9': return "৯";

        // Lowercase alphabet keys
        case 'a': return "ৃ"; // U+09C3 (Ri-kar)
        case 'b': return "ন"; // U+09A8 (Donto-No)
        case 'c': return "ে"; // U+09C7 (E-kar, pre-vowel)
        case 'd': return "ি"; // U+09BF (Hrosho-I-kar)
        case 'e': return "ড"; // U+09A1 (Ddo)
        case 'f': return "া"; // U+09BE (Aa-kar)
        case 'g': return "্"; // U+09CD (Hasanta / Link key)
        case 'h': return "ব"; // U+09AC (Bo)
        case 'i': return "হ"; // U+09B9 (Ho)
        case 'j': return "ক"; // U+0995 (Ko)
        case 'k': return "ত"; // U+09A4 (To)
        case 'l': return "দ"; // U+09A6 (Do)
        case 'm': return "ম"; // U+09AE (Mo)
        case 'n': return "স"; // U+09B8 (Donto-Sho)
        case 'o': return "গ"; // U+0997 (Go)
        case 'p': return (variant == BijoyVariant::UniJoy) ? "ড়" : "প";
        case 'q': return "ঙ"; // U+0999 (Umo)
        case 'r': return (variant == BijoyVariant::UniJoy) ? "প" : "র";
        case 's': return "ু"; // U+09C1 (Hrosho-U-kar)
        case 't': return "ট"; // U+099F (Tto)
        case 'u': return "জ"; // U+099C (Borgiyo-Jo)
        case 'v': return (variant == BijoyVariant::UniJoy) ? "র" : "ল";
        case 'w': return "য"; // U+09AF (Antostho-Zo)
        case 'x': return "ো"; // U+09CB (O-kar)
        case 'y': return "চ"; // U+099A (Cho)
        case 'z': return "্র"; // U+09CD U+09B0 (Ro-phala)

        // Symbols & Punctuation
        case '\\': return "ৎ"; // U+09CE (Khanda Ta)
        case '`':  return "‘"; // U+2018 (Left single quote)
        case '\'': return "’"; // U+2019 (Right single quote)
        default: return "";
    }
}

inline std::string_view getBijoyShifted(char c, BijoyVariant variant = BijoyVariant::Classic) {
    switch (c) {
        // Shifted uppercase letters
        case 'A': case 'a': return "র্"; // U+09B0 U+09CD (Reph)
        case 'B': case 'b': return "ণ"; // U+09A3 (Murdhonno-No)
        case 'C': case 'c': return "ৈ"; // U+09C8 (OI-kar, pre-vowel)
        case 'D': case 'd': return "ী"; // U+09C0 (Dirgho-I-kar)
        case 'E': case 'e': return "ঢ"; // U+09A2 (Ddho)
        case 'F': case 'f': return "অ"; // U+0985 (Shoro-A)
        case 'G': case 'g': return "।"; // U+0964 (Dari)
        case 'H': case 'h': return "ভ"; // U+09AD (Bho)
        case 'I': case 'i': return "ঞ"; // U+099E (Iyo)
        case 'J': case 'j': return "খ"; // U+0996 (Kho)
        case 'K': case 'k': return "থ"; // U+09A5 (Tho)
        case 'L': case 'l': return "ধ"; // U+09A7 (Dho)
        case 'M': case 'm': return "শ"; // U+09B6 (Talobbo-Sho)
        case 'N': case 'n': return "ষ"; // U+09B7 (Murdhonno-Sho)
        case 'O': case 'o': return "ঘ"; // U+0998 (Gho)
        case 'P': case 'p': return (variant == BijoyVariant::UniJoy) ? "ঢ়" : "ফ";
        case 'Q': case 'q': return "ং"; // U+0982 (Anusvara)
        case 'R': case 'r': return (variant == BijoyVariant::UniJoy) ? "ফ" : "ড়";
        case 'S': case 's': return "ূ"; // U+09C2 (Dirgho-U-kar)
        case 'T': case 't': return "ঠ"; // U+09A0 (Ttho)
        case 'U': case 'u': return "ঝ"; // U+099D (Jho)
        case 'V': case 'v': return (variant == BijoyVariant::UniJoy) ? "ল" : "ঢ়";
        case 'W': case 'w': return "য়"; // U+09DF (Antostho-Yo)
        case 'X': case 'x': return "ৌ"; // U+09CC (OU-kar)
        case 'Y': case 'y': return "ছ"; // U+099B (Chho)
        case 'Z': case 'z': return "্য"; // U+09CD U+09AF (Ja-phala)

        // Shifted symbols
        case '$': case '4': return "৳"; // U+09F3 (Taka sign)
        case '&': case '7': return "ঁ"; // U+0981 (Candrabindu)
        case '*': case '8': return "ঃ"; // U+0983 (Visarga)
        case '|':           return "ঃ"; // U+0983 (Visarga on pipe)
        case '~':           return "“"; // U+201C (Left double quote)
        case '"':           return "”"; // U+201D (Right double quote)
        default: return "";
    }
}

// In Bijoy layout, typing link key 'g' followed by a Kar produces the corresponding independent vowel
inline std::string_view getBijoyLinkedVowel(char c, bool shifted) {
    if (!shifted) {
        switch (c) {
            case 'f': return "আ"; // g + f (Aa-kar) -> আ (U+0986)
            case 'd': return "ই"; // g + d (Hrosho-I-kar) -> ই (U+0987)
            case 's': return "উ"; // g + s (Hrosho-U-kar) -> উ (U+0989)
            case 'a': return "ঋ"; // g + a (Ri-kar) -> ঋ (U+098B)
            case 'c': return "এ"; // g + c (E-kar) -> এ (U+098F)
            case 'x': return "ও"; // g + x (O-kar) -> ও (U+0993)
            case 'g': return "্"; // g + g -> explicit virama
            default: break;
        }
    } else {
        switch (c) {
            case 'D': case 'd': return "ঈ"; // g + D (Dirgho-I-kar) -> ঈ (U+0988)
            case 'S': case 's': return "ঊ"; // g + S (Dirgho-U-kar) -> ঊ (U+098A)
            case 'C': case 'c': return "ঐ"; // g + C (OI-kar) -> ঐ (U+0990)
            case 'X': case 'x': return "ঔ"; // g + X (OU-kar) -> ঔ (U+0994)
            case 'G': case 'g': return "॥"; // g + G (Dari) -> Double Dari (U+0965)
            default: break;
        }
    }
    return "";
}

} // namespace banglakey::rules
