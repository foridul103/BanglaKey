#pragma once

#include <string_view>
#include <vector>

namespace banglakey::rules {

struct PhoneticClusterRule {
    std::string_view pattern;
    std::string_view replacement;
};

struct PhoneticVowelRule {
    std::string_view pattern;
    std::string_view independent;
    std::string_view dependent; // Kar
};

// Deterministic lexicon for common words containing inherent vowels without kar
inline std::string_view lookupPhoneticLexicon(std::string_view word) {
    if (word == "khoroch")   return "খরচ";
    if (word == "ekhon")     return "এখন";
    if (word == "bondhu")    return "বন্ধু";
    if (word == "utsob")     return "উৎসব";
    if (word == "ut`sob")    return "উৎসব";
    if (word == "kkhoma")    return "ক্ষমা";
    if (word == "hoThat`")   return "হঠাৎ";
    if (word == "rosto")     return "রস্ত";
    if (word == "dhonnobad") return "ধন্যবাদ";
    if (word == "onno")      return "অন্য";
    if (word == "shob")      return "সব";
    if (word == "moti")      return "মতি";
    if (word == "kotha")     return "কথা";
    return "";
}

// Validated multi-character consonant clusters
inline const std::vector<PhoneticClusterRule>& getPhoneticConjuncts() {
    static const std::vector<PhoneticClusterRule> conjuncts = {
        // Quad-character clusters
        {"kkhm", "ক্ষ্ম"},
        {"ksho", "ক্ষ"},
        {"ngg`", "ঙ্গ"},

        // Triple-character clusters
        {"kkh", "ক্ষ"},
        {"cch", "চ্ছ"},
        {"jny", "জ্ঞ"},
        {"gny", "জ্ঞ"},
        {"ngk", "ঙ্ক"},
        {"ngg", "ঙ্গ"},
        {"nch", "ঞ্চ"},
        {"njh", "ঞ্ছ"},
        {"ntr", "ন্ত্র"},
        {"ndr", "ন্দ্র"},
        {"ndh", "ন্ধ"},
        {"mbh", "ম্ভ"},
        {"mph", "ম্ফ"},
        {"str", "স্ত্র"},
        {"spl", "স্প্ল"},
        {"shk", "ষ্ক"},
        {"sht", "ষ্ট"},
        {"shm", "ষ্ম"},
        {"skh", "স্খ"},
        {"sth", "স্থ"},
        {"sph", "স্ফ"},
        {"bdh", "ব্ধ"},

        // Dual-character conjuncts & doubled consonants
        {"kk", "ক্ক"},
        {"kt", "ক্ত"},
        {"cc", "চ্চ"},
        {"jj", "জ্জ"},
        {"tt", "ত্ত"},
        {"dd", "দ্দ"},
        {"nn", "ন্ন"},
        {"nt", "ন্ত"},
        {"nd", "ন্দ"},
        {"pp", "প্প"},
        {"pt", "প্ত"},
        {"bb", "ব্ব"},
        {"bd", "ব্দ"},
        {"mm", "ম্ম"},
        {"ll", "ল্ল"},
        {"st", "স্ত"},
        {"sp", "স্প"},
        {"sn", "স্ন"},
        {"sm", "স্ম"},
        {"sl", "স্ল"},

        // Digraph consonants
        {"kh", "খ"},
        {"gh", "ঘ"},
        {"Ng", "ঙ"}, // Capitalized Ng for Umo
        {"ng", "ং"}, // Lowercase ng for Anusvara (e.g. bangla -> বাংলা)
        {"ch", "চ"},
        {"Ch", "ছ"},
        {"jh", "ঝ"},
        {"th", "থ"},
        {"Th", "ঠ"},
        {"dh", "ধ"},
        {"Dh", "ঢ"},
        {"ph", "ফ"},
        {"bh", "ভ"},
        {"sh", "শ"},
        {"Sh", "ষ"},
        {"rh", "ঢ়"},
        {"Rh", "ঢ়"},
        {"t`", "ৎ"}, // Khanda Ta via backtick modifier
        {"r`", "র্"}  // Explicit Reph via backtick modifier
    };
    return conjuncts;
}

// Single-character consonant mappings
inline std::string_view getPhoneticSingleConsonant(char c) {
    switch (c) {
        case 'k': return "ক";
        case 'K': return "খ";
        case 'g': return "গ";
        case 'G': return "ঘ";
        case 'c': return "চ";
        case 'C': return "ছ";
        case 'j': return "জ";
        case 'J': return "ঝ";
        case 't': return "ত";
        case 'T': return "ট";
        case 'd': return "দ";
        case 'D': return "ড";
        case 'n': return "ন";
        case 'N': return "ণ";
        case 'p': return "প";
        case 'P': return "ফ";
        case 'f': return "ফ";
        case 'F': return "ফ";
        case 'b': return "ব";
        case 'B': return "ভ";
        case 'v': return "ভ";
        case 'V': return "ভ";
        case 'm': return "ম";
        case 'M': return "ম";
        case 'z': return "য";
        case 'Z': return "্য";
        case 'r': return "র";
        case 'R': return "ড়";
        case 'l': return "ল";
        case 'L': return "ল";
        case 's': return "স";
        case 'S': return "ষ";
        case 'h': return "হ";
        case 'H': return "হ";
        case 'y': return "য়";
        case 'Y': return "্য";
        case 'w': return "ও";
        default: return "";
    }
}

// Vowel rules: provides both independent form and dependent Kar form
inline const std::vector<PhoneticVowelRule>& getPhoneticVowels() {
    static const std::vector<PhoneticVowelRule> vowels = {
        {"rri", "ঋ", "ৃ"},
        {"aa",  "আ", "া"},
        {"ee",  "ঈ", "ী"},
        {"oo",  "ঊ", "ূ"},
        {"oi",  "ঐ", "ৈ"},
        {"ou",  "ঔ", "ৌ"},
        {"OI",  "ঐ", "ৈ"},
        {"OU",  "ঔ", "ৌ"},
        {"a",   "আ", "া"},  // Initial 'a' -> 'আ' (e.g. ami -> আমি, amar -> আমার)
        {"A",   "আ", "া"},
        {"i",   "ই", "ি"},
        {"I",   "ঈ", "ী"},
        {"u",   "উ", "ু"},
        {"U",   "ঊ", "ূ"},
        {"e",   "এ", "ে"},
        {"E",   "এ", "ে"},
        {"O",   "ও", "ো"},
        {"o",   "অ", "ো"}   // Initial 'o' -> 'অ' (e.g. onek -> অনেক), after consonant O-kar 'ো' (e.g. sonar -> সোনার, bhalo -> ভালো)
    };
    return vowels;
}

} // namespace banglakey::rules
