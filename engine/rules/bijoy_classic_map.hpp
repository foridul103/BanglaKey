#pragma once

#include <string_view>

namespace banglakey::rules {

// Traditional Bijoy Classic mapping table for legacy SutonnyMJ font compatibility.
// In SutonnyMJ, ASCII/ANSI characters correspond to specific Bengali glyph positions.
inline std::string_view getBijoyClassicNormal(char c) {
    switch (c) {
        // Digits
        case '0': return "0";
        case '1': return "1";
        case '2': return "2";
        case '3': return "3";
        case '4': return "4";
        case '5': return "5";
        case '6': return "6";
        case '7': return "7";
        case '8': return "8";
        case '9': return "9";

        // Lowercase letters (SutonnyMJ glyphs)
        case 'a': return "…"; // Ri-kar (0x85 in CP1252)
        case 'b': return "b"; // Donto-No
        case 'c': return "c"; // E-kar (pre-positioned visual glyph)
        case 'd': return "d"; // Hrosho-I-kar
        case 'e': return "e"; // Ddo
        case 'f': return "f"; // Aa-kar
        case 'g': return "g"; // Link key (hasanta)
        case 'h': return "h"; // Bo
        case 'i': return "i"; // Ho
        case 'j': return "j"; // Ko
        case 'k': return "k"; // To
        case 'l': return "l"; // Do
        case 'm': return "m"; // Mo
        case 'n': return "n"; // Donto-Sho
        case 'o': return "o"; // Go
        case 'p': return "p"; // Po
        case 'q': return "q"; // Umo
        case 'r': return "r"; // Ro
        case 's': return "s"; // Hrosho-U-kar
        case 't': return "t"; // Tto
        case 'u': return "u"; // Borgiyo-Jo
        case 'v': return "v"; // Lo
        case 'w': return "w"; // Antostho-Zo
        case 'x': return "x"; // O-kar
        case 'y': return "y"; // Cho
        case 'z': return "z"; // Ro-phala

        // Punctuation
        case '\\': return "ৎ"; // Khanda Ta
        case '`':  return "‘";
        case '\'': return "’";
        default: return "";
    }
}

inline std::string_view getBijoyClassicShifted(char c) {
    switch (c) {
        case 'A': case 'a': return "©"; // Reph in SutonnyMJ (0xA9)
        case 'B': case 'b': return "B"; // Murdhonno-No
        case 'C': case 'c': return "C"; // OI-kar
        case 'D': case 'd': return "D"; // Dirgho-I-kar
        case 'E': case 'e': return "E"; // Ddho
        case 'F': case 'f': return "F"; // Shoro-A
        case 'G': case 'g': return "G"; // Dari
        case 'H': case 'h': return "H"; // Bho
        case 'I': case 'i': return "I"; // Iyo
        case 'J': case 'j': return "J"; // Kho
        case 'K': case 'k': return "K"; // Tho
        case 'L': case 'l': return "L"; // Dho
        case 'M': case 'm': return "M"; // Talobbo-Sho
        case 'N': case 'n': return "N"; // Murdhonno-Sho
        case 'O': case 'o': return "O"; // Gho
        case 'P': case 'p': return "P"; // Pho
        case 'Q': case 'q': return "Q"; // Anusvara
        case 'R': case 'r': return "R"; // Dda-bindu-ro
        case 'S': case 's': return "S"; // Dirgho-U-kar
        case 'T': case 't': return "T"; // Ttho
        case 'U': case 'u': return "U"; // Jho
        case 'V': case 'v': return "V"; // Ddha-bindu-ro
        case 'W': case 'w': return "W"; // Antostho-Yo
        case 'X': case 'x': return "X"; // OU-kar
        case 'Y': case 'y': return "Y"; // Chho
        case 'Z': case 'z': return "Z"; // Ja-phala

        // Symbols
        case '$': case '4': return "T"; // Taka sign in SutonnyMJ (Shift+4)
        case '&': case '7': return "u"; // Candrabindu in SutonnyMJ (Shift+7)
        case '|':           return ":"; // Visarga in SutonnyMJ (Shift+\)
        case '~':           return "“";
        case '"':           return "”";
        default: return "";
    }
}

// In Bijoy Classic, 'g' + vowel key produces SutonnyMJ independent vowel glyphs
inline std::string_view getBijoyClassicLinkedVowel(char c, bool shifted) {
    if (!shifted) {
        switch (c) {
            case 'f': return "Av"; // আ (Shoro-A 'A' + Aa-kar 'v' in SutonnyMJ)
            case 'd': return "B";  // ই
            case 's': return "D";  // উ
            case 'a': return "F";  // ঋ
            case 'c': return "G";  // এ
            case 'x': return "I";  // ও
            default: break;
        }
    } else {
        switch (c) {
            case 'D': case 'd': return "C"; // ঈ
            case 'S': case 's': return "E"; // ঊ
            case 'C': case 'c': return "H"; // ঐ
            case 'X': case 'x': return "J"; // ঔ
            case 'F': case 'f': return "F"; // অ
            default: break;
        }
    }
    return "";
}

// In Bijoy Classic, 'g' + consonant produces SutonnyMJ conjunct ligatures
inline std::string_view getBijoyClassicConjunct(char first, char second, bool shiftedSecond) {
    if (first == 'j' && second == 'j' && !shiftedSecond) return "¬";    // ক্ক
    if (first == 'j' && second == 'n' && shiftedSecond)  return "¶";    // ক্ষ (j + N)
    if (first == 'u' && second == 'i' && shiftedSecond)  return "Á";    // জ্ঞ (u + I)
    if (first == 'k' && second == 'k' && !shiftedSecond) return "Ë";    // ত্ত
    if (first == 'b' && second == 'k' && !shiftedSecond) return "šÍ";   // ন্ত
    if (first == 'k' && second == 'm' && !shiftedSecond) return "Z¥";   // ত্ম
    if (first == 'o' && second == 'o' && !shiftedSecond) return "»";    // গ্গ
    if (first == 'y' && second == 'y' && !shiftedSecond) return "”";    // চ্চ
    if (first == 'y' && second == 'y' && shiftedSecond)  return "”Q";   // চ্ছ (y + Y)
    if (first == 't' && second == 't' && !shiftedSecond) return "Æ";    // ট্ট
    if (first == 'd' && second == 'd' && !shiftedSecond) return "Ç";    // ড্ড
    if (first == 'l' && second == 'l' && !shiftedSecond) return "Ï";    // দ্দ
    if (first == 'l' && second == 'l' && shiftedSecond)  return "×";    // দ্ধ (l + L)
    if (first == 'v' && second == 'v' && shiftedSecond)  return "j−";   // ল্ল (V + V)
    if (first == 'n' && second == 'k' && !shiftedSecond) return "¯Í";   // স্ত
    if (first == 'n' && second == 'r' && !shiftedSecond) return "¯ú";   // স্প (স + প)
    return "";
}

} // namespace banglakey::rules
