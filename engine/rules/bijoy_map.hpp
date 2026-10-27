#pragma once

#include <string_view>
#include <banglakey/config.hpp>

namespace banglakey::rules {

// Lookup table for exact Bijoy keyboard layout as specified in 232436413-Bijoy-Keyboard-Layout.pdf
inline std::string_view getBijoyNormal(char c, BijoyVariant variant = BijoyVariant::Classic) {
    (void)variant; // The PDF layout is the authoritative Bijoy specification
    switch (c) {
        // Digits (Row 1)
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
        case 'a': return "ৃ"; // U+09C3 (Ri-kar) - Row 3, Key A
        case 'b': return "ন"; // U+09A8 (Donto-No) - Row 4, Key B
        case 'c': return "ে"; // U+09C7 (E-kar, pre-vowel) - Row 4, Key C
        case 'd': return "ি"; // U+09BF (Hrosho-I-kar) - Row 3, Key D
        case 'e': return "ড"; // U+09A1 (Ddo) - Row 2, Key E
        case 'f': return "া"; // U+09BE (Aa-kar) - Row 3, Key F
        case 'g': return "্"; // U+09CD (Hasanta / Link key) - Row 3, Key G
        case 'h': return "ব"; // U+09AC (Bo) - Row 3, Key H
        case 'i': return "হ"; // U+09B9 (Ho) - Row 2, Key I
        case 'j': return "ক"; // U+0995 (Ko) - Row 3, Key J
        case 'k': return "ত"; // U+09A4 (To) - Row 3, Key K
        case 'l': return "দ"; // U+09A6 (Do) - Row 3, Key L
        case 'm': return "ম"; // U+09AE (Mo) - Row 4, Key M
        case 'n': return "স"; // U+09B8 (Donto-Sho) - Row 4, Key N
        case 'o': return "গ"; // U+0997 (Go) - Row 2, Key O
        case 'p': return "ড়"; // U+09DC (Dda-bindu-ro) - Row 2, Key P
        case 'q': return "ঙ"; // U+0999 (Umo) - Row 2, Key Q
        case 'r': return "প"; // U+09AA (Po) - Row 2, Key R
        case 's': return "ু"; // U+09C1 (Hrosho-U-kar) - Row 3, Key S
        case 't': return "ট"; // U+099F (Tto) - Row 2, Key T
        case 'u': return "জ"; // U+099C (Borgiyo-Jo) - Row 2, Key U
        case 'v': return "র"; // U+09B0 (Ro) - Row 4, Key V
        case 'w': return "য"; // U+09AF (Antostho-Zo) - Row 2, Key W
        case 'x': return "ও"; // U+0993 (Independent vowel O) - Row 4, Key X
        case 'y': return "চ"; // U+099A (Cho) - Row 2, Key Y
        case 'z': return "্র"; // U+09CD U+09B0 (Ro-phala) - Row 4, Key Z

        // Symbols & Punctuation
        case '\\': return "ৎ"; // U+09CE (Khanda Ta) - Row 1, Key backslash
        case '`':  return "‘"; // U+2018 (Left single quote) - Row 1, Key backquote
        case '\'': return "’"; // U+2019 (Right single quote) - Row 3, Key single quote
        default: return "";
    }
}

inline std::string_view getBijoyShifted(char c, BijoyVariant variant = BijoyVariant::Classic) {
    (void)variant;
    switch (c) {
        // Shifted uppercase letters
        case 'A': case 'a': return "র্"; // U+09B0 U+09CD (Reph) - Key A Shift
        case 'B': case 'b': return "ণ"; // U+09A3 (Murdhonno-No) - Key B Shift
        case 'C': case 'c': return "ৈ"; // U+09C8 (OI-kar, pre-vowel) - Key C Shift
        case 'D': case 'd': return "ী"; // U+09C0 (Dirgho-I-kar) - Key D Shift
        case 'E': case 'e': return "ঢ"; // U+09A2 (Ddho) - Key E Shift
        case 'F': case 'f': return "অ"; // U+0985 (Shoro-A) - Key F Shift
        case 'G': case 'g': return "।"; // U+0964 (Dari) - Key G Shift
        case 'H': case 'h': return "ভ"; // U+09AD (Bho) - Key H Shift
        case 'I': case 'i': return "ঞ"; // U+099E (Iyo) - Key I Shift
        case 'J': case 'j': return "খ"; // U+0996 (Kho) - Key J Shift
        case 'K': case 'k': return "থ"; // U+09A5 (Tho) - Key K Shift
        case 'L': case 'l': return "ধ"; // U+09A7 (Dho) - Key L Shift
        case 'M': case 'm': return "শ"; // U+09B6 (Talobbo-Sho) - Key M Shift
        case 'N': case 'n': return "ষ"; // U+09B7 (Murdhonno-Sho) - Key N Shift
        case 'O': case 'o': return "ঘ"; // U+0998 (Gho) - Key O Shift
        case 'P': case 'p': return "ঢ়"; // U+09DD (Ddha-bindu-ro) - Key P Shift
        case 'Q': case 'q': return "ং"; // U+0982 (Anusvara) - Key Q Shift
        case 'R': case 'r': return "ফ"; // U+09AB (Pho) - Key R Shift
        case 'S': case 's': return "ূ"; // U+09C2 (Dirgho-U-kar) - Key S Shift
        case 'T': case 't': return "ঠ"; // U+09A0 (Ttho) - Key T Shift
        case 'U': case 'u': return "ঝ"; // U+099D (Jho) - Key U Shift
        case 'V': case 'v': return "ল"; // U+09B2 (Lo) - Key V Shift
        case 'W': case 'w': return "য়"; // U+09DF (Antostho-Yo) - Key W Shift
        case 'X': case 'x': return "ৌ"; // U+09CC (OU-kar) - Key X Shift
        case 'Y': case 'y': return "ছ"; // U+099B (Chho) - Key Y Shift
        case 'Z': case 'z': return "্য"; // U+09CD U+09AF (Ja-phala) - Key Z Shift

        // Shifted symbols
        case '$': case '4': return "৳"; // U+09F3 (Taka sign on Shift+4)
        case '&': case '7': return "ঁ"; // U+0981 (Candrabindu on Shift+7)
        case '|':           return "ঃ"; // U+0983 (Visarga on Shift+\ / pipe)
        case '~':           return "“"; // U+201C (Left double quote on Shift+`)
        case '"':           return "”"; // U+201D (Right double quote on Shift+')
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
            case 'x': return "ও"; // g + x (O) -> ও (U+0993)
            case 'g': return "্"; // g + g -> explicit virama
            default: break;
        }
    } else {
        switch (c) {
            case 'D': case 'd': return "ঈ"; // g + D (Dirgho-I-kar) -> ঈ (U+0988)
            case 'S': case 's': return "ঊ"; // g + S (Dirgho-U-kar) -> ঊ (U+098A)
            case 'C': case 'c': return "ঐ"; // g + C (OI-kar) -> ঐ (U+0990)
            case 'X': case 'x': return "ঔ"; // g + X (OU-kar) -> ঔ (U+0994)
            case 'F': case 'f': return "অ"; // g + F (Shoro-A) -> অ (U+0985)
            case 'G': case 'g': return "॥"; // g + G (Dari) -> Double Dari (U+0965)
            default: break;
        }
    }
    return "";
}

} // namespace banglakey::rules
