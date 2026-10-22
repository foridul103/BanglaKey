# BanglaKey Linux — Bijoy-Compatible Keyboard Layout Specification

This document provides the formal specification, legal research, key mapping tables, composition state machine rules, and test fixtures for the **Bijoy-compatible** typing mode in **BanglaKey Linux**.

---

## 1. Legal Research & Licensing Compliance

### 1.1 Historical Context
- The Bijoy keyboard layout was conceived in 1988 by Mustafa Jabbar (Ananda Computers) for early Macintosh publishing systems and later ported to Windows in 1993 using proprietary legacy ANSI fonts (e.g., SutonnyMJ).
- In 2004, the Bangladesh Computer Council (BCC) standardized the National Bengali Keyboard (BDS 1738:2004 / Jatiya Layout), which adopted many similar ergonomic principles while differing in specific vowel and punctuation placements.
- In 2005, the Ekushey open-source project created **UniJoy**, an open, standardized Unicode keyboard mapping designed to provide high layout compatibility with classic Bijoy on modern Unicode operating systems. UniJoy was published under the **GNU Lesser General Public License (LGPL v2.1+)** and standardized in the GNU `m17n-db` (`/usr/share/m17n/bn-unijoy.mim`).

### 1.2 Intellectual Property & Clean-Room Principles
Under global copyright conventions and software jurisprudence:
1. Pure functional keyboard key assignments (mapping physical typewriter/computer keys to characters of a natural language alphabet) are functional interfaces and standard layouts.
2. Proprietary binary code, copyrighted DLLs, font files, and application assets from commercial software (such as Bijoy Ekushe or Bijoy 52) **must never be copied or disassembled**.
3. BanglaKey Linux implements its Bijoy-compatible mode cleanly from public technical standards:
   - **BDS 1738:2004** (Bangladesh Computer Council).
   - **Ekushey UniJoy Specification** (`bn-unijoy.mim`, LGPL v2.1+).
   - **Unicode 15.0/16.0 Bengali Script Standard** (`U+0980` - `U+09FF`).

---

## 2. Layout Matrix

BanglaKey provides two selectable Bijoy variants:
1. **Classic Bijoy** (Default): Preserves traditional physical placement where `r` is `র`, `p` is `প`, and `v` is `ল`.
2. **UniJoy Compatible**: Conforms to the Ekushey UniJoy standard where `v` is `র` and `r` is `প`.

### 2.1 Base & Shifted Key Mapping (Classic Bijoy)

#### Number Row
| Key | Normal | Unicode Codepoint | Shifted | Shifted Unicode | Description |
| :---: | :---: | :--- | :---: | :--- | :--- |
| `1` | `১` | `U+09E7` | `!` | `U+0021` | Bengali digit 1 / Exclamation |
| `2` | `২` | `U+09E8` | `@` | `U+0040` | Bengali digit 2 / At sign |
| `3` | `৩` | `U+09E9` | `#` | `U+0023` | Bengali digit 3 / Hash |
| `4` | `৪` | `U+09EA` | `৳` | `U+09F3` | Bengali digit 4 / Taka sign |
| `5` | `৫` | `U+09EB` | `%` | `U+0025` | Bengali digit 5 / Percent |
| `6` | `৬` | `U+09EC` | `^` | `U+005E` | Bengali digit 6 / Caret |
| `7` | `৭` | `U+09ED` | `ঁ` | `U+0981` | Bengali digit 7 / Candrabindu |
| `8` | `৮` | `U+09EE` | `ঃ` | `U+0983` | Bengali digit 8 / Visarga |
| `9` | `৯` | `U+09EF` | `(` | `U+0028` | Bengali digit 9 / Left paren |
| `0` | `০` | `U+09E6` | `)` | `U+0029` | Bengali digit 0 / Right paren |
| `-` | `-` | `U+002D` | `_` | `U+005F` | Hyphen / Underscore |
| `=` | `=` | `U+003D` | `+` | `U+002B` | Equals / Plus |

#### Top Alphabet Row
| Key | Normal | Unicode | Shifted | Shifted Unicode | Description |
| :---: | :---: | :--- | :---: | :--- | :--- |
| `q` | `ঙ` | `U+0999` | `ং` | `U+0982` | Umo / Anusvara |
| `w` | `য` | `U+09AF` | `য়` | `U+09DF` | Antostho-Zo / Antostho-Yo |
| `e` | `ড` | `U+09A1` | `ঢ` | `U+09A2` | Ddo / Ddho |
| `r` | `র` | `U+09B0` | `ড়` | `U+09DC` | Ro / Dda-bindu-ro |
| `t` | `ট` | `U+099F` | `ঠ` | `U+09A0` | Tto / Ttho |
| `y` | `চ` | `U+099A` | `ছ` | `U+099B` | Cho / Chho |
| `u` | `জ` | `U+099C` | `ঝ` | `U+099D` | Borgiyo-Jo / Jho |
| `i` | `হ` | `U+09B9` | `ঞ` | `U+099E` | Ho / Iyo |
| `o` | `গ` | `U+0997` | `ঘ` | `U+0998` | Go / Gho |
| `p` | `প` | `U+09AA` | `ফ` | `U+09AB` | Po / Pho |
| `[` | `[` | `U+005B` | `{` | `U+007B` | Bracket left |
| `]` | `]` | `U+005D` | `}` | `U+007D` | Bracket right |
| `\` | `ৎ` | `U+09CE` | `ঃ` | `U+0983` | Khanda Ta / Visarga |

#### Home Row
| Key | Normal | Unicode | Shifted | Shifted Unicode | Description |
| :---: | :---: | :--- | :---: | :--- | :--- |
| `a` | `ৃ` | `U+09C3` | `র্` | `U+09B0 U+09CD` | Ri-kar / Reph |
| `s` | `ু` | `U+09C1` | `ূ` | `U+09C2` | Hrosho-U-kar / Dirgho-U-kar |
| `d` | `ি` | `U+09BF` | `ী` | `U+09C0` | Hrosho-I-kar / Dirgho-I-kar |
| `f` | `া` | `U+09BE` | `অ` | `U+0985` | Aa-kar / Shoro-A |
| `g` | `্` | `U+09CD` | `।` | `U+0964` | Hasanta (Link Key) / Dari |
| `h` | `ব` | `U+09AC` | `ভ` | `U+09AD` | Bo / Bho |
| `j` | `ক` | `U+0995` | `খ` | `U+0996` | Ko / Kho |
| `k` | `ত` | `U+09A4` | `থ` | `U+09A5` | To / Tho |
| `l` | `দ` | `U+09A6` | `ধ` | `U+09A7` | Do / Dho |
| `;` | `;` | `U+003B` | `:` | `U+003A` | Semicolon / Colon |
| `'` | `'` | `U+0027` | `"` | `U+0022` | Quote / Double quote |

#### Bottom Row
| Key | Normal | Unicode | Shifted | Shifted Unicode | Description |
| :---: | :---: | :--- | :---: | :--- | :--- |
| `z` | `্র` | `U+09CD U+09B0` | `্য` | `U+09CD U+09AF` | Ro-phala / Ja-phala |
| `x` | `ো` | `U+09CB` | `ৌ` | `U+09CC` | O-kar / OU-kar |
| `c` | `ে` | `U+09C7` | `ৈ` | `U+09C8` | E-kar (pre-vowel) / OI-kar |
| `v` | `ল` | `U+09B2` | `ঢ়` | `U+09DD` | Lo / Ddha-bindu-ro |
| `b` | `ন` | `U+09A8` | `ণ` | `U+09A3` | Donto-No / Murdhonno-No |
| `n` | `স` | `U+09B8` | `ষ` | `U+09B7` | Donto-Sho / Murdhonno-Sho |
| `m` | `ম` | `U+09AE` | `শ` | `U+09B6` | Mo / Talobbo-Sho |
| `,` | `,` | `U+002C` | `<` | `U+003C` | Comma / Less-than |
| `.` | `.` | `U+002E` | `>` | `U+003E` | Period / Greater-than |
| `/` | `/` | `U+002F` | `?` | `U+003F` | Slash / Question mark |

---

## 3. Composition State Machine Rules

### 3.1 Link Key (`g`) Conjunct (*Juktakkhor*) Formation
- Link key `g` emits `U+09CD` (Bengali Virama / Hasanta).
- When a consonant $C_1$ is followed by `g` and another consonant $C_2$, it resolves into a conjunct:
  $$C_1 + \text{Virama} + C_2 \rightarrow C_1C_2$$
- Examples:
  - `j` (`ক`) + `g` + `j` (`ক`) $\rightarrow$ `ক্ক` (`U+0995 U+09CD U+0995`)
  - `j` (`ক`) + `g` + `N` (`ষ`) $\rightarrow$ `ক্ষ` (`U+0995 U+09CD U+09B7`)
  - `u` (`জ`) + `g` + `I` (`ঞ`) $\rightarrow$ `জ্ঞ` (`U+099C U+09CD U+099E`)
  - `b` (`ন`) + `g` + `k` (`ত`) $\rightarrow$ `ন্ত` (`U+09A8 U+09CD U+09A4`)
  - `k` (`ত`) + `g` + `k` (`ত`) $\rightarrow$ `ত্ত` (`U+09A4 U+09CD U+09A4`)

### 3.2 Pre-Vowel Sign Reordering (E-kar & OI-kar)
In Bijoy typing, E-kar `ে` (`c`) and OI-kar `ৈ` (`C`) are typed **before** the consonant.
The engine stores the sign in `pending_pre_vowel_`:
1. User presses `c` (`ে`): Preedit displays `[ে]`.
2. User presses `j` (`ক`): Emitted Unicode is `ক` (`U+0995`) + `ে` (`U+09C7`) $\rightarrow$ `কে`.
3. User presses `c` (`ে`) followed by a conjunct sequence (`j` + `g` + `j`): Emitted Unicode is `ক` + `্` + `ক` + `ে` $\rightarrow$ `ক্কে`.

### 3.3 Two-Part Vowel Consolidation
When E-kar `ে` (`c`) is followed by Aa-kar `া` (`f`), they consolidate into O-kar `ো` (`U+09CB`):
$$\text{Pending } \text{E-kar} + \text{Aa-kar} \rightarrow \text{O-kar}$$
Typing sequence: `c` + `f` + `j` $\rightarrow$ `কো`.

### 3.4 Linked Independent Vowels via `g`
Typing link key `g` followed by a Kar produces the corresponding independent vowel:
| Sequence | Produced Character | Name |
| :---: | :---: | :--- |
| `g` + `f` | `আ` | BENGALI LETTER AA (`U+0986`) |
| `g` + `d` | `ই` | BENGALI LETTER I (`U+0987`) |
| `g` + `D` | `ঈ` | BENGALI LETTER II (`U+0988`) |
| `g` + `s` | `উ` | BENGALI LETTER U (`U+0989`) |
| `g` + `S` | `ঊ` | BENGALI LETTER UU (`U+098A`) |
| `g` + `a` | `ঋ` | BENGALI LETTER VOCALIC R (`U+098B`) |
| `g` + `c` | `এ` | BENGALI LETTER E (`U+098F`) |
| `g` + `C` | `ঐ` | BENGALI LETTER AI (`U+0990`) |
| `g` + `x` | `ও` | BENGALI LETTER O (`U+0993`) |
| `g` + `X` | `ঔ` | BENGALI LETTER AU (`U+0994`) |

### 3.5 Explicit Hasanta & Punctuation
- Consecutive `g` + `g`: Emits explicit virama with ZWNJ (`U+09CD U+200C` or `U+09CD`).
- `g` followed by Space: Commits `্ ` (hasanta + space).
- `g` + `G` (Shift+G): Emits Double Dari `॥` (`U+0965`).

---

## 4. Test Fixtures & Traceable Mapping

| Input Sequence | Expected Canonical Output | Notes |
| :--- | :--- | :--- |
| `j` | `ক` | Basic consonant |
| `j` `f` | `কা` | Consonant + post-vowel |
| `c` `j` | `কে` | Pre-vowel reordering |
| `C` `l` | `দৈ` | Pre-vowel OI-kar reordering |
| `c` `f` `j` | `কো` | Dual vowel consolidation + consonant |
| `j` `g` `j` | `ক্ক` | Basic conjunct |
| `j` `g` `N` | `ক্ষ` | Kkho conjunct |
| `u` `g` `I` | `জ্ঞ` | Jnyo conjunct |
| `k` `g` `z` | `ত্র` | Consonant + Ro-phala |
| `k` `Z` | `ক্য` | Consonant + Ja-phala |
| `A` `m` | `র্ম` | Reph + consonant |
| `c` `j` `g` `j` | `ক্কে` | Pre-vowel + conjunct |
| `g` `f` | `আ` | Linked independent vowel |
| `g` `g` | `্` | Explicit hasanta |
| `\\` | `ৎ` | Khanda Ta |

---

## 5. Scope & Current Limitations

1. **Rare Quadruple Conjuncts**: Highly unusual historical Sanskrit loan conjuncts (e.g., $C_1+্+C_2+্+C_3+্+C_4$) fall back to canonical Unicode virama sequence rendering.
2. **Proprietary Macros**: Commercial Bijoy shortcut macros (such as custom numbering bullet shortcuts) are not part of the typing engine layer.
