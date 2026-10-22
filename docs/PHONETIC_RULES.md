# BanglaKey Linux — Phonetic Transliteration Specification

This document details the phonetic transliteration mapping rules, parsing algorithm, and acceptance criteria for Roman-to-Bengali typing in **BanglaKey Linux**.

---

## 1. Overview & Transliteration Scheme

Phonetic typing converts Roman keyboard sequences into standard Unicode Bengali characters using an intuitive phonetic correspondence.

### Key Characteristics
1. **Dynamic Multi-Character Greedy Matching**: Longest prefix matching for multi-character sequences (e.g. `kkh` -> `ক্ষ`, `ng` -> `ঙ`, `oi` -> `ঐ`).
2. **Context-Sensitive Vowel Conversion**:
   - Word-initial vowels or post-vowel positions map to **Independent Vowels** (স্বরবর্ণ, e.g. `a` -> `অ`, `aa` -> `আ`).
   - Post-consonant vowels map to **Dependent Vowel Signs** (কার, e.g. `k` + `a` -> `কা`, `k` + `i` -> `কি`).
3. **Deterministic Lexicon for Ambiguous Clusters**:
   - Common Bengali words with irregular conjuncts (e.g., `khoroch` -> `খরচ`, `ekhon` -> `এখন`, `bondhu` -> `বন্ধু`, `utsob` -> `উৎসব`) are prioritized through a deterministic lexicon table before falling back to rule-based composition.

---

## 2. Acceptance Test Fixtures

The following acceptance test cases are verified through automated unit tests in `tests/unit/test_phonetic.cpp`:

| Input Sequence | Expected Output | Category |
| :--- | :--- | :--- |
| `amar` | `আমার` | Basic word transliteration |
| `sonar` | `সোনার` | Consonant + vowel kar |
| `bangla` | `বাংলা` | Anusvara / Velar nasal |
| `khoroch` | `খরচ` | Lexicon / Aspirational consonant |
| `ekhon` | `এখন` | Independent vowel + consonant |
| `bondhu` | `বন্ধু` | Non-nasal conjunct (`ন্ধ`) |
| `utsob` | `উৎসব` | Khanda Ta (`ৎ`) + consonant |
| `kkhoma` | `ক্ষমা` | Complex conjunct (`ক্ষ`) |
| `rosto` | `রস্ত` | S-conjunct cluster |
| `hoThat` | `হঠাৎ` | Explicit Khanda Ta marker |
| `12345` | `১২৩৪৫` | Bengali numerals |

---

## 3. Keyboard Editing & Composition Actions

- **Space**: Commits the primary selected candidate with a trailing space.
- **Enter / Return**: Commits the selected candidate without adding space.
- **Backspace**: Deletes the trailing character from the raw buffer and updates preedit transliteration.
- **Left / Right Arrows**: Navigates the cursor within the active buffer.
- **Up / Down Arrows**: Cycles candidate selection.
- **Escape**: Cancels composition and clears preedit buffer without committing text.
- **Application Shortcuts (`Ctrl`, `Alt`, `Super`)**: Flushes composition and forwards the shortcut unaltered to the host application.
