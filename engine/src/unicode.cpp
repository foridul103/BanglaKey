#include <banglakey/unicode.hpp>

namespace banglakey::unicode {

std::string toUtf8(char32_t cp) {
    std::string out;
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0x10FFFF) {
        out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return out;
}

std::string toUtf8(const std::vector<char32_t>& cps) {
    std::string out;
    for (char32_t cp : cps) {
        out.append(toUtf8(cp));
    }
    return out;
}

std::vector<char32_t> toCodepoints(std::string_view utf8) {
    std::vector<char32_t> result;
    size_t i = 0;
    const size_t len = utf8.size();

    while (i < len) {
        unsigned char c = static_cast<unsigned char>(utf8[i]);
        if (c <= 0x7F) {
            result.push_back(c);
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 < len) {
                char32_t cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(utf8[i + 1]) & 0x3F);
                result.push_back(cp);
                i += 2;
            } else {
                break;
            }
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 < len) {
                char32_t cp = ((c & 0x0F) << 12) |
                              ((static_cast<unsigned char>(utf8[i + 1]) & 0x3F) << 6) |
                              (static_cast<unsigned char>(utf8[i + 2]) & 0x3F);
                result.push_back(cp);
                i += 3;
            } else {
                break;
            }
        } else if ((c & 0xF8) == 0xF0) {
            if (i + 3 < len) {
                char32_t cp = ((c & 0x07) << 18) |
                              ((static_cast<unsigned char>(utf8[i + 1]) & 0x3F) << 12) |
                              ((static_cast<unsigned char>(utf8[i + 2]) & 0x3F) << 6) |
                              (static_cast<unsigned char>(utf8[i + 3]) & 0x3F);
                result.push_back(cp);
                i += 4;
            } else {
                break;
            }
        } else {
            // Invalid byte, advance 1
            i += 1;
        }
    }
    return result;
}

size_t codepointCount(std::string_view utf8) {
    size_t count = 0;
    for (char c : utf8) {
        // Count bytes that are not continuation bytes (i.e. not 10xxxxxx)
        if ((static_cast<unsigned char>(c) & 0xC0) != 0x80) {
            ++count;
        }
    }
    return count;
}

char32_t combineTwoPartVowel(char32_t pre, char32_t post) noexcept {
    if (pre == CP_SIGN_E && post == CP_SIGN_AA) {
        return CP_SIGN_O; // ে + া -> ো (U+09CB)
    }
    if (pre == CP_SIGN_E && post == CP_AU_LENGTH_MARK) {
        return CP_SIGN_AU; // ে + ৗ -> ৌ (U+09CC)
    }
    return 0;
}

std::string formConjunct(std::string_view c1, std::string_view c2) {
    std::string result(c1);
    result.append(toUtf8(CP_SIGN_VIRAMA));
    result.append(c2);
    return result;
}

std::string formReph(std::string_view consonant) {
    std::string result = toUtf8(CP_LETTER_RA);
    result.append(toUtf8(CP_SIGN_VIRAMA));
    result.append(consonant);
    return result;
}

std::string formExplicitHasanta(std::string_view consonant) {
    std::string result(consonant);
    result.append(toUtf8(CP_SIGN_VIRAMA));
    result.append(toUtf8(CP_ZWNJ));
    return result;
}

} // namespace banglakey::unicode
