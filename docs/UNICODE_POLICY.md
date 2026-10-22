# BanglaKey Linux — Bengali Unicode & Orthography Policy

This document defines the strict Bengali Unicode standard compliance, codepoint ordering policies, grapheme composition rules, and privacy safeguards for the **BanglaKey** typing engine.

---

## 1. Unicode Standard Reference

BanglaKey complies with **The Unicode Standard (Bengali Script Block: `U+0980` - `U+09FF`)**.

All text output generated and committed by BanglaKey is strictly valid UTF-8. Legacy ANSI or non-standard encodings (such as SutonnyMJ glyph fonts using ASCII code page 1252) are **not** emitted.

---

## 2. Character Classification & Codepoint Map

### 2.1 Independent Vowels (*Shoroborno*)
| Codepoint | Glyph | Name |
| :--- | :--- | :--- |
| `U+0985` | অ | BENGALI LETTER A |
| `U+0986` | আ | BENGALI LETTER AA |
| `U+0987` | ই | BENGALI LETTER I |
| `U+0988` | ঈ | BENGALI LETTER II |
| `U+0989` | উ | BENGALI LETTER U |
| `U+098A` | ঊ | BENGALI LETTER UU |
| `U+098B` | ঋ | BENGALI LETTER VOCALIC R |
| `U+098F` | এ | BENGALI LETTER E |
| `U+0990` | ঐ | BENGALI LETTER AI |
| `U+0993` | ও | BENGALI LETTER O |
| `U+0994` | ঔ | BENGALI LETTER AU |

### 2.2 Dependent Vowel Signs (*Kar*)
Dependent vowel signs attach directly to a preceding consonant base or consonant conjunct:
| Codepoint | Glyph | Attached Example | Name |
| :--- | :--- | :--- | :--- |
| `U+09BE` | া | কা | BENGALI VOWEL SIGN AA |
| `U+09BF` | ি | কি | BENGALI VOWEL SIGN I |
| `U+09C0` | ী | কী | BENGALI VOWEL SIGN II |
| `U+09C1` | ু | কু | BENGALI VOWEL SIGN U |
| `U+09C2` | ূ | কূ | BENGALI VOWEL SIGN UU |
| `U+09C3` | ৃ | কৃ | BENGALI VOWEL SIGN VOCALIC R |
| `U+09C7` | ে | কে | BENGALI VOWEL SIGN E |
| `U+09C8` | ৈ | কৈ | BENGALI VOWEL SIGN AI |
| `U+09CB` | ো | কো | BENGALI VOWEL SIGN O |
| `U+09CC` | ৌ | কৌ | BENGALI VOWEL SIGN AU |

### 2.3 Consonants (*Banjonborno*)
Standard 35 base consonants: `ক` (`U+0995`) through `হ` (`U+09B9`), plus `ড়` (`U+09DC`), `ঢ়` (`U+09DD`), and `য়` (`U+09DF`).

### 2.4 Signs, Modifiers & Controls
| Codepoint | Glyph | Name | Role |
| :--- | :--- | :--- | :--- |
| `U+09CD` | ্ | BENGALI SIGN VIRAMA (Hasanta / Halant) | Conjunct connector / vowel suppressor |
| `U+0981` | ঁ | BENGALI SIGN CANDRABINDU | Nasalization modifier |
| `U+0982` | ং | BENGALI SIGN ANUSVARA | Pure nasal consonant sound |
| `U+0983` | ঃ | BENGALI SIGN VISARGA | Unvoiced glottal aspirate |
| `U+09CE` | ৎ | BENGALI LETTER KHANDA TA | Distinct historic final consonant |
| `U+09E7`-`U+09EF` | ১-৯ | BENGALI DIGITS | Bengali decimal numerals |
| `U+0964` | । | DEVANAGARI DANDA (Dari) | Bengali standard sentence terminator |
| `U+0965` | ॥ | DEVANAGARI DOUBLE DANDA | Double Dari (verse terminator) |
| `U+09F3` | ৳ | BENGALI RUPEE SIGN | Bangladesh Taka currency sign |
| `U+200D` | ZWJ | ZERO WIDTH JOINER | Controls ligature formation |
| `U+200C` | ZWNJ| ZERO WIDTH NON-JOINER | Suppresses conjunct formation (explicit hasanta) |

---

## 3. Strict Code-Point Ordering Rules

In accordance with the Unicode Standard 15.0/16.0 for Indic scripts:

### 3.1 Logical vs Visual Order
In physical writing or legacy typewriters, certain vowel signs (such as E-kar `ে` `U+09C7` and OI-kar `ৈ` `U+09C8`) appear visually to the *left* of the consonant. Two-part vowels (such as O-kar `ো` `U+09CB` and OU-kar `ৌ` `U+09CC`) visually surround the consonant.

**Canonical Rule**: In digital Unicode text:
- The consonant (or consonant conjunct) **MUST ALWAYS PRECEDE** the dependent vowel sign in the memory stream:
  $$\text{Consonant} + \text{Vowel Sign}$$
  Example: `ক` (`U+0995`) followed by `ে` (`U+09C7`) $\rightarrow$ `কে`.
- **Under no circumstances** may a vowel sign be placed prior to the consonant in output text.
- When typing in Bijoy mode where `c` (E-kar `ে`) is pressed before `j` (`ক`), the typing engine holds `ে` in an internal pending composition state and emits `ক` + `ে` upon receiving the consonant.

### 3.2 Canonical Syllable Sequence
A canonical Bengali orthographic syllable consists of:
```
[Reph] + Base Consonant + [Virama + Consonant]* + [Virama + ZWJ + Ya/Ra] + [Vowel Sign] + [Anusvara / Visarga] + [Candrabindu]
```
Where:
1. **Reph**: Formed by `র` (`U+09B0`) + `্` (`U+09CD`). When followed by a consonant $C$, it renders as Reph over $C$ (e.g. `র্` + `ম` $\rightarrow$ `র্ম`).
2. **Conjuncts (*Juktakkhor*)**: Formed by $C_1$ + `্` (`U+09CD`) + $C_2$ (e.g. `ক` + `্` + `ক` $\rightarrow$ `ক্ক`).
3. **Khanda Ta**: Stored strictly as single codepoint `ৎ` (`U+09CE`). Never decompose into `ত` + `্` + `ZWJ` in modern output.
4. **Explicit Hasanta**: If the user desires an explicit visible hasanta without forming a conjunct, `ZWNJ` (`U+200C`) is inserted after Hasanta:
   $$C_1 + \text{Virama} + \text{ZWNJ} + C_2$$

---

## 4. Normalization Policy

1. **No Automatic Destructive Normalization**:
   Automatic NFC/NFD normalization can alter orthographic distinctions in Indic scripts (e.g., splitting `ৎ` or modifying ZWJ/ZWNJ clusters). BanglaKey does **not** run unvalidated global Unicode normalization filters over emitted strings.
2. **Two-part Vowels**:
   The engine emits single unified codepoints for multi-part vowels:
   - O-kar is emitted as `U+09CB` (`ো`), never as split `U+09C7 U+09BE` (`ে` + `া`).
   - OU-kar is emitted as `U+09CC` (`ৌ`), never as split `U+09C7 U+09D7` (`ে` + `ৗ`).
   If a user types E-kar + Aa-kar in Bijoy mode, the engine merges them into `U+09CB`.

---

## 5. Scope & Compatibility Boundaries

- **Bijoy Compatibility**:
  The Bijoy layout provided in Phase 2 implements standard core character mappings and conjunct chaining via link key `g`. Unsupported edge cases (e.g. complex triple conjuncts not yet in the rule table) fall back safely to standard Unicode sequence emission ($C_1 + \text{Virama} + C_2 + \text{Virama} + C_3$) which modern OpenType shaping engines render accurately.
- **Phonetic Compatibility**:
  Phonetic transliteration implements deterministic phonetic rule mapping with longest-prefix lookahead. It does **not** claim complete parity with proprietary Avro rule engines or heuristics until full grammar test suites are validated.

---

## 6. Privacy & Telemetry Standards

1. **Zero Keystroke Telemetry**: Keystrokes are processed strictly in-memory within the local engine process.
2. **No User Content in Logs**: Debug or error logging must never write arbitrary user-typed text, preedit strings, or buffers to disk, journald, or system consoles.
