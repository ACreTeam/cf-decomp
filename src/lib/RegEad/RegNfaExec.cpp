#include <lib/RegEad/RegNfa.hpp>

namespace RegEad {

// 801ECD90
void RegNfa::buildPredefinedClass(CharSet &out, u32 classId) {
    switch (classId) {
    case CLASS_ANY: // . : anything but CR, LF, NEL, LS and PS
        out.add(CharRange(0x0, 0xFFFF));
        out.remove(0xD);
        out.remove(0xA);
        out.remove(0x85);
        out.remove(0x2028);
        out.remove(0x2029);
        break;
    case CLASS_DIGIT: // \d, \p{Digit}: [0-9]
        out.add(CharRange(0x30, 0x39));
        break;
    case CLASS_NOT_DIGIT: // \D: not \d
        buildPredefinedClass(out, CLASS_DIGIT);
        out = out.complement();
        break;
    case CLASS_SPACE: // \s, \p{Space}: [ \t\n\x0B\f\r]
        out.addChar(0x20);
        out.addChar(0x9);
        out.addChar(0xA);
        out.addChar(0xB);
        out.addChar(0xC);
        out.addChar(0xD);
        break;
    case CLASS_NOT_SPACE: // \S: not \s
        buildPredefinedClass(out, CLASS_SPACE);
        out = out.complement();
        break;
    case CLASS_WORD: // \w: [a-zA-Z0-9_]
        out.add(CharRange(0x61, 0x7A));
        out.add(CharRange(0x41, 0x5A));
        out.add(CharRange(0x30, 0x39));
        out.addChar(0x5F);
        break;
    case CLASS_NOT_WORD: // \W: not \w
        buildPredefinedClass(out, CLASS_WORD);
        out = out.complement();
        break;
    case CLASS_LOWER: // \p{Lower}: [a-z]
        out.add(CharRange(0x61, 0x7A));
        break;
    case CLASS_UPPER: // \p{Upper}: [A-Z]
        out.add(CharRange(0x41, 0x5A));
        break;
    case CLASS_ASCII: // \p{ASCII}, \p{InBasicLatin}: U+0000..U+007F
        out.add(CharRange(0x0, 0x7F));
        break;
    case CLASS_ALPHA: // \p{Alpha}: [a-zA-Z]
        out.add(CharRange(0x61, 0x7A));
        out.add(CharRange(0x41, 0x5A));
        break;
    case CLASS_ALNUM: // \p{Alnum}: [a-zA-Z0-9]
        out.add(CharRange(0x61, 0x7A));
        out.add(CharRange(0x41, 0x5A));
        out.add(CharRange(0x30, 0x39));
        break;
    case CLASS_PUNCT: // \p{Punct}: ASCII punctuation (!..~ minus Alnum)
        out.add(CharRange(0x21, 0x2F));
        out.add(CharRange(0x3A, 0x40));
        out.add(CharRange(0x5B, 0x60));
        out.add(CharRange(0x7B, 0x7E));
        break;
    case CLASS_GRAPH: // \p{Graph}: Alnum + Punct (visible ASCII)
        buildPredefinedClass(out, CLASS_ALNUM);
        buildPredefinedClass(out, CLASS_PUNCT);
        break;
    case CLASS_PRINT: // \p{Print}: Graph + space
        buildPredefinedClass(out, CLASS_GRAPH);
        out.addChar(0x20);
        break;
    case CLASS_BLANK: // \p{Blank}: space or tab
        out.addChar(0x9);
        out.addChar(0x20);
        break;
    case CLASS_CNTRL: // \p{Cntrl}: U+0000..U+001F, DEL
        out.add(CharRange(0x0, 0x1F));
        out.addChar(0x7F);
        break;
    case CLASS_XDIGIT: // \p{XDigit}: [0-9A-Fa-f]
        out.add(CharRange(0x30, 0x39));
        out.add(CharRange(0x41, 0x46));
        out.add(CharRange(0x61, 0x66));
        break;
    case CLASS_IN_LATIN_1_SUPPLEMENT: // U+0080..U+00FF
        out.add(CharRange(0x80, 0xFF));
        break;
    case CLASS_IN_LATIN_EXTENDED_A: // U+0100..U+017F
        out.add(CharRange(0x100, 0x17F));
        break;
    case CLASS_IN_IPA_EXTENSIONS: // U+0250..U+02AF
        out.add(CharRange(0x250, 0x2AF));
        break;
    case CLASS_IN_SPACING_MODIFIER_LETTERS: // U+02B0..U+02FF
        out.add(CharRange(0x2B0, 0x2FF));
        break;
    case CLASS_IN_COMBINING_DIACRITICAL_MARKS: // U+0300..U+036F
        out.add(CharRange(0x300, 0x36F));
        break;
    case CLASS_IN_GREEK: // U+0370..U+03FF
        out.add(CharRange(0x370, 0x3FF));
        break;
    case CLASS_IN_CYRILLIC: // U+0400..U+04FF
        out.add(CharRange(0x400, 0x4FF));
        break;
    case CLASS_IN_ARMENIAN: // U+0530..U+058F
        out.add(CharRange(0x530, 0x58F));
        break;
    case CLASS_IN_HEBREW: // U+0590..U+05FF
        out.add(CharRange(0x590, 0x5FF));
        break;
    case CLASS_IN_ARABIC: // U+0600..U+06FF
        out.add(CharRange(0x600, 0x6FF));
        break;
    case CLASS_IN_SYRIAC: // U+0700..U+074F
        out.add(CharRange(0x700, 0x74F));
        break;
    case CLASS_IN_THAANA: // U+0780..U+07BF
        out.add(CharRange(0x780, 0x7BF));
        break;
    case CLASS_IN_DEVANAGARI: // U+0900..U+097F
        out.add(CharRange(0x900, 0x97F));
        break;
    case CLASS_IN_BENGALI: // U+0980..U+09FF
        out.add(CharRange(0x980, 0x9FF));
        break;
    case CLASS_IN_GURMUKHI: // U+0A00..U+0A7F
        out.add(CharRange(0xA00, 0xA7F));
        break;
    case CLASS_IN_GUJARATI: // U+0A80..U+0AFF
        out.add(CharRange(0xA80, 0xAFF));
        break;
    case CLASS_IN_ORIYA: // U+0B00..U+0B7F
        out.add(CharRange(0xB00, 0xB7F));
        break;
    case CLASS_IN_TAMIL: // U+0B80..U+0BFF
        out.add(CharRange(0xB80, 0xBFF));
        break;
    case CLASS_IN_TELUGU: // U+0C00..U+0C7F
        out.add(CharRange(0xC00, 0xC7F));
        break;
    case CLASS_IN_KANNADA: // U+0C80..U+0CFF
        out.add(CharRange(0xC80, 0xCFF));
        break;
    case CLASS_IN_MALAYALAM: // U+0D00..U+0D7F
        out.add(CharRange(0xD00, 0xD7F));
        break;
    case CLASS_IN_SINHALA: // U+0D80..U+0DFF
        out.add(CharRange(0xD80, 0xDFF));
        break;
    case CLASS_IN_THAI: // U+0E00..U+0E7F
        out.add(CharRange(0xE00, 0xE7F));
        break;
    case CLASS_IN_LAO: // U+0E80..U+0EFF
        out.add(CharRange(0xE80, 0xEFF));
        break;
    case CLASS_IN_TIBETAN: // U+0F00..U+0FFF
        out.add(CharRange(0xF00, 0xFFF));
        break;
    case CLASS_IN_MYANMAR: // U+1000..U+109F
        out.add(CharRange(0x1000, 0x109F));
        break;
    case CLASS_IN_GEORGIAN: // U+10A0..U+10FF
        out.add(CharRange(0x10A0, 0x10FF));
        break;
    case CLASS_IN_HANGUL_JAMO: // U+1100..U+11FF
        out.add(CharRange(0x1100, 0x11FF));
        break;
    case CLASS_IN_ETHIOPIC: // U+1200..U+137F
        out.add(CharRange(0x1200, 0x137F));
        break;
    case CLASS_IN_CHEROKEE: // U+13A0..U+13FF
        out.add(CharRange(0x13A0, 0x13FF));
        break;
    case CLASS_IN_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS: // U+1400..U+167F
        out.add(CharRange(0x1400, 0x167F));
        break;
    case CLASS_IN_OGHAM: // U+1680..U+169F
        out.add(CharRange(0x1680, 0x169F));
        break;
    case CLASS_IN_RUNIC: // U+16A0..U+16FF
        out.add(CharRange(0x16A0, 0x16FF));
        break;
    case CLASS_IN_KHMER: // U+1780..U+17FF
        out.add(CharRange(0x1780, 0x17FF));
        break;
    case CLASS_IN_MONGOLIAN: // U+1800..U+18AF
        out.add(CharRange(0x1800, 0x18AF));
        break;
    case CLASS_IN_LATIN_EXTENDED_ADDITIONAL: // U+1E00..U+1EFF
        out.add(CharRange(0x1E00, 0x1EFF));
        break;
    case CLASS_IN_GREEK_EXTENDED: // U+1F00..U+1FFF
        out.add(CharRange(0x1F00, 0x1FFF));
        break;
    case CLASS_IN_GENERAL_PUNCTUATION: // U+2000..U+206F
        out.add(CharRange(0x2000, 0x206F));
        break;
    case CLASS_IN_SUPERSCRIPTS_AND_SUBSCRIPTS: // U+2070..U+209F
        out.add(CharRange(0x2070, 0x209F));
        break;
    case CLASS_IN_CURRENCY_SYMBOLS: // U+20A0..U+20CF
        out.add(CharRange(0x20A0, 0x20CF));
        break;
    case CLASS_IN_LETTERLIKE_SYMBOLS: // U+2100..U+214F
        out.add(CharRange(0x2100, 0x214F));
        break;
    case CLASS_IN_NUMBER_FORMS: // U+2150..U+218F
        out.add(CharRange(0x2150, 0x218F));
        break;
    case CLASS_IN_ARROWS: // U+2190..U+21FF
        out.add(CharRange(0x2190, 0x21FF));
        break;
    case CLASS_IN_MATHEMATICAL_OPERATORS: // U+2200..U+22FF
        out.add(CharRange(0x2200, 0x22FF));
        break;
    case CLASS_IN_MISCELLANEOUS_TECHNICAL: // U+2300..U+23FF
        out.add(CharRange(0x2300, 0x23FF));
        break;
    case CLASS_IN_CONTROL_PICTURES: // U+2400..U+243F
        out.add(CharRange(0x2400, 0x243F));
        break;
    case CLASS_IN_OPTICAL_CHARACTER_RECOGNITION: // U+2440..U+245F
        out.add(CharRange(0x2440, 0x245F));
        break;
    case CLASS_IN_ENCLOSED_ALPHANUMERICS: // U+2460..U+24FF
        out.add(CharRange(0x2460, 0x24FF));
        break;
    case CLASS_IN_BOX_DRAWING: // U+2500..U+257F
        out.add(CharRange(0x2500, 0x257F));
        break;
    case CLASS_IN_BLOCK_ELEMENTS: // U+2580..U+259F
        out.add(CharRange(0x2580, 0x259F));
        break;
    case CLASS_IN_GEOMETRIC_SHAPES: // U+25A0..U+25FF
        out.add(CharRange(0x25A0, 0x25FF));
        break;
    case CLASS_IN_MISCELLANEOUS_SYMBOLS: // U+2600..U+26FF
        out.add(CharRange(0x2600, 0x26FF));
        break;
    case CLASS_IN_DINGBATS: // U+2700..U+27BF
        out.add(CharRange(0x2700, 0x27BF));
        break;
    case CLASS_IN_BRAILLE_PATTERNS: // U+2800..U+28FF
        out.add(CharRange(0x2800, 0x28FF));
        break;
    case CLASS_IN_CJK_RADICALS_SUPPLEMENT: // U+2E80..U+2EFF
        out.add(CharRange(0x2E80, 0x2EFF));
        break;
    case CLASS_IN_KANGXI_RADICALS: // U+2F00..U+2FDF
        out.add(CharRange(0x2F00, 0x2FDF));
        break;
    case CLASS_IN_IDEOGRAPHIC_DESCRIPTION_CHARACTERS: // U+2FF0..U+2FFF
        out.add(CharRange(0x2FF0, 0x2FFF));
        break;
    case CLASS_IN_CJK_SYMBOLS_AND_PUNCTUATION: // U+3000..U+303F
        out.add(CharRange(0x3000, 0x303F));
        break;
    case CLASS_IN_HIRAGANA: // U+3040..U+309F
        out.add(CharRange(0x3040, 0x309F));
        break;
    case CLASS_IN_KATAKANA: // U+30A0..U+30FF
        out.add(CharRange(0x30A0, 0x30FF));
        break;
    case CLASS_IN_BOPOMOFO: // U+3100..U+312F
        out.add(CharRange(0x3100, 0x312F));
        break;
    case CLASS_IN_HANGUL_COMPATIBILITY_JAMO: // U+3130..U+318F
        out.add(CharRange(0x3130, 0x318F));
        break;
    case CLASS_IN_KANBUN: // U+3190..U+319F
        out.add(CharRange(0x3190, 0x319F));
        break;
    case CLASS_IN_BOPOMOFO_EXTENDED: // U+31A0..U+31BF
        out.add(CharRange(0x31A0, 0x31BF));
        break;
    case CLASS_IN_ENCLOSED_CJK_LETTERS_AND_MONTHS: // U+3200..U+32FF
        out.add(CharRange(0x3200, 0x32FF));
        break;
    case CLASS_IN_CJK_COMPATIBILITY: // U+3300..U+33FF
        out.add(CharRange(0x3300, 0x33FF));
        break;
    case CLASS_IN_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A: // U+3400..U+4DB5
        out.add(CharRange(0x3400, 0x4DB5));
        break;
    case CLASS_IN_CJK_UNIFIED_IDEOGRAPHS: // U+4E00..U+9FFF
        out.add(CharRange(0x4E00, 0x9FFF));
        break;
    case CLASS_IN_YI_SYLLABLES: // U+A000..U+A48F
        out.add(CharRange(0xA000, 0xA48F));
        break;
    case CLASS_IN_YI_RADICALS: // U+A490..U+A4CF
        out.add(CharRange(0xA490, 0xA4CF));
        break;
    case CLASS_IN_HANGUL_SYLLABLES: // U+AC00..U+D7A3
        out.add(CharRange(0xAC00, 0xD7A3));
        break;
    case CLASS_IN_HIGH_SURROGATES: // U+D800..U+DB7F
        out.add(CharRange(0xD800, 0xDB7F));
        break;
    case CLASS_IN_HIGH_PRIVATE_USE_SURROGATES: // U+DB80..U+DBFF
        out.add(CharRange(0xDB80, 0xDBFF));
        break;
    case CLASS_IN_LOW_SURROGATES: // U+DC00..U+DFFF
        out.add(CharRange(0xDC00, 0xDFFF));
        break;
    case CLASS_IN_CJK_COMPATIBILITY_IDEOGRAPHS: // U+F900..U+FAFF
        out.add(CharRange(0xF900, 0xFAFF));
        break;
    case CLASS_IN_ALPHABETIC_PRESENTATION_FORMS: // U+FB00..U+FB4F
        out.add(CharRange(0xFB00, 0xFB4F));
        break;
    case CLASS_IN_ARABIC_PRESENTATION_FORMS_A: // U+FB50..U+FDFF
        out.add(CharRange(0xFB50, 0xFDFF));
        break;
    case CLASS_IN_COMBINING_HALF_MARKS: // U+FE20..U+FE2F
        out.add(CharRange(0xFE20, 0xFE2F));
        break;
    case CLASS_IN_CJK_COMPATIBILITY_FORMS: // U+FE30..U+FE4F
        out.add(CharRange(0xFE30, 0xFE4F));
        break;
    case CLASS_IN_SMALL_FORM_VARIANTS: // U+FE50..U+FE6F
        out.add(CharRange(0xFE50, 0xFE6F));
        break;
    case CLASS_IN_HALFWIDTH_AND_FULLWIDTH_FORMS: // U+FF00..U+FFEF
        out.add(CharRange(0xFF00, 0xFFEF));
        break;
    case CLASS_IN_SPECIALS: // U+FEFF, U+FFF0..U+FFFD
        out.addChar(0xFEFF);
        out.add(CharRange(0xFFF0, 0xFFFD));
        break;
    case CLASS_L: // \p{L}: letters (Ll + Lu + Lt + Lm + Lo)
        buildPredefinedClass(out, CLASS_LL);
        buildPredefinedClass(out, CLASS_LU);
        buildPredefinedClass(out, CLASS_LT);
        buildPredefinedClass(out, CLASS_LM);
        buildPredefinedClass(out, CLASS_LO);
        break;
    case CLASS_LL: // \p{Ll}: lowercase letters
        out.add(CharRange(0x61, 0x7A));
        out.addChar(0xAA);
        out.addChar(0xB5);
        out.addChar(0xBA);
        out.add(CharRange(0xDF, 0xF6));
        out.add(CharRange(0xF8, 0xFF));
        out.addChar(0x101);
        out.addChar(0x103);
        out.addChar(0x105);
        out.addChar(0x107);
        out.addChar(0x109);
        out.addChar(0x10B);
        out.addChar(0x10D);
        out.addChar(0x10F);
        out.addChar(0x111);
        out.addChar(0x113);
        out.addChar(0x115);
        out.addChar(0x117);
        out.addChar(0x119);
        out.addChar(0x11B);
        out.addChar(0x11D);
        out.addChar(0x11F);
        out.addChar(0x121);
        out.addChar(0x123);
        out.addChar(0x125);
        out.addChar(0x127);
        out.addChar(0x129);
        out.addChar(0x12B);
        out.addChar(0x12D);
        out.addChar(0x12F);
        out.addChar(0x131);
        out.addChar(0x133);
        out.addChar(0x135);
        out.add(CharRange(0x137, 0x138));
        out.addChar(0x13A);
        out.addChar(0x13C);
        out.addChar(0x13E);
        out.addChar(0x140);
        out.addChar(0x142);
        out.addChar(0x144);
        out.addChar(0x146);
        out.add(CharRange(0x148, 0x149));
        out.addChar(0x14B);
        out.addChar(0x14D);
        out.addChar(0x14F);
        out.addChar(0x151);
        out.addChar(0x153);
        out.addChar(0x155);
        out.addChar(0x157);
        out.addChar(0x159);
        out.addChar(0x15B);
        out.addChar(0x15D);
        out.addChar(0x15F);
        out.addChar(0x161);
        out.addChar(0x163);
        out.addChar(0x165);
        out.addChar(0x167);
        out.addChar(0x169);
        out.addChar(0x16B);
        out.addChar(0x16D);
        out.addChar(0x16F);
        out.addChar(0x171);
        out.addChar(0x173);
        out.addChar(0x175);
        out.addChar(0x177);
        out.addChar(0x17A);
        out.addChar(0x17C);
        out.add(CharRange(0x17E, 0x180));
        out.addChar(0x183);
        out.addChar(0x185);
        out.addChar(0x188);
        out.add(CharRange(0x18C, 0x18D));
        out.addChar(0x192);
        out.addChar(0x195);
        out.add(CharRange(0x199, 0x19B));
        out.addChar(0x19E);
        out.addChar(0x1A1);
        out.addChar(0x1A3);
        out.addChar(0x1A5);
        out.addChar(0x1A8);
        out.add(CharRange(0x1AA, 0x1AB));
        out.addChar(0x1AD);
        out.addChar(0x1B0);
        out.addChar(0x1B4);
        out.addChar(0x1B6);
        out.add(CharRange(0x1B9, 0x1BA));
        out.add(CharRange(0x1BD, 0x1BF));
        out.addChar(0x1C6);
        out.addChar(0x1C9);
        out.addChar(0x1CC);
        out.addChar(0x1CE);
        out.addChar(0x1D0);
        out.addChar(0x1D2);
        out.addChar(0x1D4);
        out.addChar(0x1D6);
        out.addChar(0x1D8);
        out.addChar(0x1DA);
        out.add(CharRange(0x1DC, 0x1DD));
        out.addChar(0x1DF);
        out.addChar(0x1E1);
        out.addChar(0x1E3);
        out.addChar(0x1E5);
        out.addChar(0x1E7);
        out.addChar(0x1E9);
        out.addChar(0x1EB);
        out.addChar(0x1ED);
        out.add(CharRange(0x1EF, 0x1F0));
        out.addChar(0x1F3);
        out.addChar(0x1F5);
        out.addChar(0x1F9);
        out.addChar(0x1FB);
        out.addChar(0x1FD);
        out.addChar(0x1FF);
        out.addChar(0x201);
        out.addChar(0x203);
        out.addChar(0x205);
        out.addChar(0x207);
        out.addChar(0x209);
        out.addChar(0x20B);
        out.addChar(0x20D);
        out.addChar(0x20F);
        out.addChar(0x211);
        out.addChar(0x213);
        out.addChar(0x215);
        out.addChar(0x217);
        out.addChar(0x219);
        out.addChar(0x21B);
        out.addChar(0x21D);
        out.addChar(0x21F);
        out.addChar(0x223);
        out.addChar(0x225);
        out.addChar(0x227);
        out.addChar(0x229);
        out.addChar(0x22B);
        out.addChar(0x22D);
        out.addChar(0x22F);
        out.addChar(0x231);
        out.addChar(0x233);
        out.add(CharRange(0x250, 0x2AD));
        out.addChar(0x390);
        out.add(CharRange(0x3AC, 0x3CE));
        out.add(CharRange(0x3D0, 0x3D1));
        out.add(CharRange(0x3D5, 0x3D7));
        out.addChar(0x3DB);
        out.addChar(0x3DD);
        out.addChar(0x3DF);
        out.addChar(0x3E1);
        out.addChar(0x3E3);
        out.addChar(0x3E5);
        out.addChar(0x3E7);
        out.addChar(0x3E9);
        out.addChar(0x3EB);
        out.addChar(0x3ED);
        out.add(CharRange(0x3EF, 0x3F3));
        out.add(CharRange(0x430, 0x45F));
        out.addChar(0x461);
        out.addChar(0x463);
        out.addChar(0x465);
        out.addChar(0x467);
        out.addChar(0x469);
        out.addChar(0x46B);
        out.addChar(0x46D);
        out.addChar(0x46F);
        out.addChar(0x471);
        out.addChar(0x473);
        out.addChar(0x475);
        out.addChar(0x477);
        out.addChar(0x479);
        out.addChar(0x47B);
        out.addChar(0x47D);
        out.addChar(0x47F);
        out.addChar(0x481);
        out.addChar(0x48D);
        out.addChar(0x48F);
        out.addChar(0x491);
        out.addChar(0x493);
        out.addChar(0x495);
        out.addChar(0x497);
        out.addChar(0x499);
        out.addChar(0x49B);
        out.addChar(0x49D);
        out.addChar(0x49F);
        out.addChar(0x4A1);
        out.addChar(0x4A3);
        out.addChar(0x4A5);
        out.addChar(0x4A7);
        out.addChar(0x4A9);
        out.addChar(0x4AB);
        out.addChar(0x4AD);
        out.addChar(0x4AF);
        out.addChar(0x4B1);
        out.addChar(0x4B3);
        out.addChar(0x4B5);
        out.addChar(0x4B7);
        out.addChar(0x4B9);
        out.addChar(0x4BB);
        out.addChar(0x4BD);
        out.addChar(0x4BF);
        out.addChar(0x4C2);
        out.addChar(0x4C4);
        out.addChar(0x4C8);
        out.addChar(0x4CC);
        out.addChar(0x4D1);
        out.addChar(0x4D3);
        out.addChar(0x4D5);
        out.addChar(0x4D7);
        out.addChar(0x4D9);
        out.addChar(0x4DB);
        out.addChar(0x4DD);
        out.addChar(0x4DF);
        out.addChar(0x4E1);
        out.addChar(0x4E3);
        out.addChar(0x4E5);
        out.addChar(0x4E7);
        out.addChar(0x4E9);
        out.addChar(0x4EB);
        out.addChar(0x4ED);
        out.addChar(0x4EF);
        out.addChar(0x4F1);
        out.addChar(0x4F3);
        out.addChar(0x4F5);
        out.addChar(0x4F9);
        out.add(CharRange(0x561, 0x587));
        out.addChar(0x1E01);
        out.addChar(0x1E03);
        out.addChar(0x1E05);
        out.addChar(0x1E07);
        out.addChar(0x1E09);
        out.addChar(0x1E0B);
        out.addChar(0x1E0D);
        out.addChar(0x1E0F);
        out.addChar(0x1E11);
        out.addChar(0x1E13);
        out.addChar(0x1E15);
        out.addChar(0x1E17);
        out.addChar(0x1E19);
        out.addChar(0x1E1B);
        out.addChar(0x1E1D);
        out.addChar(0x1E1F);
        out.addChar(0x1E21);
        out.addChar(0x1E23);
        out.addChar(0x1E25);
        out.addChar(0x1E27);
        out.addChar(0x1E29);
        out.addChar(0x1E2B);
        out.addChar(0x1E2D);
        out.addChar(0x1E2F);
        out.addChar(0x1E31);
        out.addChar(0x1E33);
        out.addChar(0x1E35);
        out.addChar(0x1E37);
        out.addChar(0x1E39);
        out.addChar(0x1E3B);
        out.addChar(0x1E3D);
        out.addChar(0x1E3F);
        out.addChar(0x1E41);
        out.addChar(0x1E43);
        out.addChar(0x1E45);
        out.addChar(0x1E47);
        out.addChar(0x1E49);
        out.addChar(0x1E4B);
        out.addChar(0x1E4D);
        out.addChar(0x1E4F);
        out.addChar(0x1E51);
        out.addChar(0x1E53);
        out.addChar(0x1E55);
        out.addChar(0x1E57);
        out.addChar(0x1E59);
        out.addChar(0x1E5B);
        out.addChar(0x1E5D);
        out.addChar(0x1E5F);
        out.addChar(0x1E61);
        out.addChar(0x1E63);
        out.addChar(0x1E65);
        out.addChar(0x1E67);
        out.addChar(0x1E69);
        out.addChar(0x1E6B);
        out.addChar(0x1E6D);
        out.addChar(0x1E6F);
        out.addChar(0x1E71);
        out.addChar(0x1E73);
        out.addChar(0x1E75);
        out.addChar(0x1E77);
        out.addChar(0x1E79);
        out.addChar(0x1E7B);
        out.addChar(0x1E7D);
        out.addChar(0x1E7F);
        out.addChar(0x1E81);
        out.addChar(0x1E83);
        out.addChar(0x1E85);
        out.addChar(0x1E87);
        out.addChar(0x1E89);
        out.addChar(0x1E8B);
        out.addChar(0x1E8D);
        out.addChar(0x1E8F);
        out.addChar(0x1E91);
        out.addChar(0x1E93);
        out.add(CharRange(0x1E95, 0x1E9B));
        out.addChar(0x1EA1);
        out.addChar(0x1EA3);
        out.addChar(0x1EA5);
        out.addChar(0x1EA7);
        out.addChar(0x1EA9);
        out.addChar(0x1EAB);
        out.addChar(0x1EAD);
        out.addChar(0x1EAF);
        out.addChar(0x1EB1);
        out.addChar(0x1EB3);
        out.addChar(0x1EB5);
        out.addChar(0x1EB7);
        out.addChar(0x1EB9);
        out.addChar(0x1EBB);
        out.addChar(0x1EBD);
        out.addChar(0x1EBF);
        out.addChar(0x1EC1);
        out.addChar(0x1EC3);
        out.addChar(0x1EC5);
        out.addChar(0x1EC7);
        out.addChar(0x1EC9);
        out.addChar(0x1ECB);
        out.addChar(0x1ECD);
        out.addChar(0x1ECF);
        out.addChar(0x1ED1);
        out.addChar(0x1ED3);
        out.addChar(0x1ED5);
        out.addChar(0x1ED7);
        out.addChar(0x1ED9);
        out.addChar(0x1EDB);
        out.addChar(0x1EDD);
        out.addChar(0x1EDF);
        out.addChar(0x1EE1);
        out.addChar(0x1EE3);
        out.addChar(0x1EE5);
        out.addChar(0x1EE7);
        out.addChar(0x1EE9);
        out.addChar(0x1EEB);
        out.addChar(0x1EED);
        out.addChar(0x1EEF);
        out.addChar(0x1EF1);
        out.addChar(0x1EF3);
        out.addChar(0x1EF5);
        out.addChar(0x1EF7);
        out.addChar(0x1EF9);
        out.add(CharRange(0x1F00, 0x1F07));
        out.add(CharRange(0x1F10, 0x1F15));
        out.add(CharRange(0x1F20, 0x1F27));
        out.add(CharRange(0x1F30, 0x1F37));
        out.add(CharRange(0x1F40, 0x1F45));
        out.add(CharRange(0x1F50, 0x1F57));
        out.add(CharRange(0x1F60, 0x1F67));
        out.add(CharRange(0x1F70, 0x1F7D));
        out.add(CharRange(0x1F80, 0x1F87));
        out.add(CharRange(0x1F90, 0x1F97));
        out.add(CharRange(0x1FA0, 0x1FA7));
        out.add(CharRange(0x1FB0, 0x1FB4));
        out.add(CharRange(0x1FB6, 0x1FB7));
        out.addChar(0x1FBE);
        out.add(CharRange(0x1FC2, 0x1FC4));
        out.add(CharRange(0x1FC6, 0x1FC7));
        out.add(CharRange(0x1FD0, 0x1FD3));
        out.add(CharRange(0x1FD6, 0x1FD7));
        out.add(CharRange(0x1FE0, 0x1FE7));
        out.add(CharRange(0x1FF2, 0x1FF4));
        out.add(CharRange(0x1FF6, 0x1FF7));
        out.addChar(0x207F);
        out.addChar(0x210A);
        out.add(CharRange(0x210E, 0x210F));
        out.addChar(0x2113);
        out.addChar(0x212F);
        out.addChar(0x2134);
        out.addChar(0x2139);
        out.add(CharRange(0xFB00, 0xFB06));
        out.add(CharRange(0xFB13, 0xFB17));
        out.add(CharRange(0xFF41, 0xFF5A));
        break;
    case CLASS_LU: // \p{Lu}: uppercase letters
        out.add(CharRange(0x41, 0x5A));
        out.add(CharRange(0xC0, 0xD6));
        out.add(CharRange(0xD8, 0xDE));
        out.addChar(0x100);
        out.addChar(0x102);
        out.addChar(0x104);
        out.addChar(0x106);
        out.addChar(0x108);
        out.addChar(0x10A);
        out.addChar(0x10C);
        out.addChar(0x10E);
        out.addChar(0x110);
        out.addChar(0x112);
        out.addChar(0x114);
        out.addChar(0x116);
        out.addChar(0x118);
        out.addChar(0x11A);
        out.addChar(0x11C);
        out.addChar(0x11E);
        out.addChar(0x120);
        out.addChar(0x122);
        out.addChar(0x124);
        out.addChar(0x126);
        out.addChar(0x128);
        out.addChar(0x12A);
        out.addChar(0x12C);
        out.addChar(0x12E);
        out.addChar(0x130);
        out.addChar(0x132);
        out.addChar(0x134);
        out.addChar(0x136);
        out.addChar(0x139);
        out.addChar(0x13B);
        out.addChar(0x13D);
        out.addChar(0x13F);
        out.addChar(0x141);
        out.addChar(0x143);
        out.addChar(0x145);
        out.addChar(0x147);
        out.addChar(0x14A);
        out.addChar(0x14C);
        out.addChar(0x14E);
        out.addChar(0x150);
        out.addChar(0x152);
        out.addChar(0x154);
        out.addChar(0x156);
        out.addChar(0x158);
        out.addChar(0x15A);
        out.addChar(0x15C);
        out.addChar(0x15E);
        out.addChar(0x160);
        out.addChar(0x162);
        out.addChar(0x164);
        out.addChar(0x166);
        out.addChar(0x168);
        out.addChar(0x16A);
        out.addChar(0x16C);
        out.addChar(0x16E);
        out.addChar(0x170);
        out.addChar(0x172);
        out.addChar(0x174);
        out.addChar(0x176);
        out.add(CharRange(0x178, 0x179));
        out.addChar(0x17B);
        out.addChar(0x17D);
        out.add(CharRange(0x181, 0x182));
        out.addChar(0x184);
        out.add(CharRange(0x186, 0x187));
        out.add(CharRange(0x189, 0x18B));
        out.add(CharRange(0x18E, 0x191));
        out.add(CharRange(0x193, 0x194));
        out.add(CharRange(0x196, 0x198));
        out.add(CharRange(0x19C, 0x19D));
        out.add(CharRange(0x19F, 0x1A0));
        out.addChar(0x1A2);
        out.addChar(0x1A4);
        out.add(CharRange(0x1A6, 0x1A7));
        out.addChar(0x1A9);
        out.addChar(0x1AC);
        out.add(CharRange(0x1AE, 0x1AF));
        out.add(CharRange(0x1B1, 0x1B3));
        out.addChar(0x1B5);
        out.add(CharRange(0x1B7, 0x1B8));
        out.addChar(0x1BC);
        out.addChar(0x1C4);
        out.addChar(0x1C7);
        out.addChar(0x1CA);
        out.addChar(0x1CD);
        out.addChar(0x1CF);
        out.addChar(0x1D1);
        out.addChar(0x1D3);
        out.addChar(0x1D5);
        out.addChar(0x1D7);
        out.addChar(0x1D9);
        out.addChar(0x1DB);
        out.addChar(0x1DE);
        out.addChar(0x1E0);
        out.addChar(0x1E2);
        out.addChar(0x1E4);
        out.addChar(0x1E6);
        out.addChar(0x1E8);
        out.addChar(0x1EA);
        out.addChar(0x1EC);
        out.addChar(0x1EE);
        out.addChar(0x1F1);
        out.addChar(0x1F4);
        out.add(CharRange(0x1F6, 0x1F8));
        out.addChar(0x1FA);
        out.addChar(0x1FC);
        out.addChar(0x1FE);
        out.addChar(0x200);
        out.addChar(0x202);
        out.addChar(0x204);
        out.addChar(0x206);
        out.addChar(0x208);
        out.addChar(0x20A);
        out.addChar(0x20C);
        out.addChar(0x20E);
        out.addChar(0x210);
        out.addChar(0x212);
        out.addChar(0x214);
        out.addChar(0x216);
        out.addChar(0x218);
        out.addChar(0x21A);
        out.addChar(0x21C);
        out.addChar(0x21E);
        out.addChar(0x222);
        out.addChar(0x224);
        out.addChar(0x226);
        out.addChar(0x228);
        out.addChar(0x22A);
        out.addChar(0x22C);
        out.addChar(0x22E);
        out.addChar(0x230);
        out.addChar(0x232);
        out.addChar(0x386);
        out.add(CharRange(0x388, 0x38A));
        out.addChar(0x38C);
        out.add(CharRange(0x38E, 0x38F));
        out.add(CharRange(0x391, 0x3A1));
        out.add(CharRange(0x3A3, 0x3AB));
        out.add(CharRange(0x3D2, 0x3D4));
        out.addChar(0x3DA);
        out.addChar(0x3DC);
        out.addChar(0x3DE);
        out.addChar(0x3E0);
        out.addChar(0x3E2);
        out.addChar(0x3E4);
        out.addChar(0x3E6);
        out.addChar(0x3E8);
        out.addChar(0x3EA);
        out.addChar(0x3EC);
        out.addChar(0x3EE);
        out.add(CharRange(0x400, 0x42F));
        out.addChar(0x460);
        out.addChar(0x462);
        out.addChar(0x464);
        out.addChar(0x466);
        out.addChar(0x468);
        out.addChar(0x46A);
        out.addChar(0x46C);
        out.addChar(0x46E);
        out.addChar(0x470);
        out.addChar(0x472);
        out.addChar(0x474);
        out.addChar(0x476);
        out.addChar(0x478);
        out.addChar(0x47A);
        out.addChar(0x47C);
        out.addChar(0x47E);
        out.addChar(0x480);
        out.addChar(0x48C);
        out.addChar(0x48E);
        out.addChar(0x490);
        out.addChar(0x492);
        out.addChar(0x494);
        out.addChar(0x496);
        out.addChar(0x498);
        out.addChar(0x49A);
        out.addChar(0x49C);
        out.addChar(0x49E);
        out.addChar(0x4A0);
        out.addChar(0x4A2);
        out.addChar(0x4A4);
        out.addChar(0x4A6);
        out.addChar(0x4A8);
        out.addChar(0x4AA);
        out.addChar(0x4AC);
        out.addChar(0x4AE);
        out.addChar(0x4B0);
        out.addChar(0x4B2);
        out.addChar(0x4B4);
        out.addChar(0x4B6);
        out.addChar(0x4B8);
        out.addChar(0x4BA);
        out.addChar(0x4BC);
        out.addChar(0x4BE);
        out.add(CharRange(0x4C0, 0x4C1));
        out.addChar(0x4C3);
        out.addChar(0x4C7);
        out.addChar(0x4CB);
        out.addChar(0x4D0);
        out.addChar(0x4D2);
        out.addChar(0x4D4);
        out.addChar(0x4D6);
        out.addChar(0x4D8);
        out.addChar(0x4DA);
        out.addChar(0x4DC);
        out.addChar(0x4DE);
        out.addChar(0x4E0);
        out.addChar(0x4E2);
        out.addChar(0x4E4);
        out.addChar(0x4E6);
        out.addChar(0x4E8);
        out.addChar(0x4EA);
        out.addChar(0x4EC);
        out.addChar(0x4EE);
        out.addChar(0x4F0);
        out.addChar(0x4F2);
        out.addChar(0x4F4);
        out.addChar(0x4F8);
        out.add(CharRange(0x531, 0x556));
        out.add(CharRange(0x10A0, 0x10C5));
        out.addChar(0x1E00);
        out.addChar(0x1E02);
        out.addChar(0x1E04);
        out.addChar(0x1E06);
        out.addChar(0x1E08);
        out.addChar(0x1E0A);
        out.addChar(0x1E0C);
        out.addChar(0x1E0E);
        out.addChar(0x1E10);
        out.addChar(0x1E12);
        out.addChar(0x1E14);
        out.addChar(0x1E16);
        out.addChar(0x1E18);
        out.addChar(0x1E1A);
        out.addChar(0x1E1C);
        out.addChar(0x1E1E);
        out.addChar(0x1E20);
        out.addChar(0x1E22);
        out.addChar(0x1E24);
        out.addChar(0x1E26);
        out.addChar(0x1E28);
        out.addChar(0x1E2A);
        out.addChar(0x1E2C);
        out.addChar(0x1E2E);
        out.addChar(0x1E30);
        out.addChar(0x1E32);
        out.addChar(0x1E34);
        out.addChar(0x1E36);
        out.addChar(0x1E38);
        out.addChar(0x1E3A);
        out.addChar(0x1E3C);
        out.addChar(0x1E3E);
        out.addChar(0x1E40);
        out.addChar(0x1E42);
        out.addChar(0x1E44);
        out.addChar(0x1E46);
        out.addChar(0x1E48);
        out.addChar(0x1E4A);
        out.addChar(0x1E4C);
        out.addChar(0x1E4E);
        out.addChar(0x1E50);
        out.addChar(0x1E52);
        out.addChar(0x1E54);
        out.addChar(0x1E56);
        out.addChar(0x1E58);
        out.addChar(0x1E5A);
        out.addChar(0x1E5C);
        out.addChar(0x1E5E);
        out.addChar(0x1E60);
        out.addChar(0x1E62);
        out.addChar(0x1E64);
        out.addChar(0x1E66);
        out.addChar(0x1E68);
        out.addChar(0x1E6A);
        out.addChar(0x1E6C);
        out.addChar(0x1E6E);
        out.addChar(0x1E70);
        out.addChar(0x1E72);
        out.addChar(0x1E74);
        out.addChar(0x1E76);
        out.addChar(0x1E78);
        out.addChar(0x1E7A);
        out.addChar(0x1E7C);
        out.addChar(0x1E7E);
        out.addChar(0x1E80);
        out.addChar(0x1E82);
        out.addChar(0x1E84);
        out.addChar(0x1E86);
        out.addChar(0x1E88);
        out.addChar(0x1E8A);
        out.addChar(0x1E8C);
        out.addChar(0x1E8E);
        out.addChar(0x1E90);
        out.addChar(0x1E92);
        out.addChar(0x1E94);
        out.addChar(0x1EA0);
        out.addChar(0x1EA2);
        out.addChar(0x1EA4);
        out.addChar(0x1EA6);
        out.addChar(0x1EA8);
        out.addChar(0x1EAA);
        out.addChar(0x1EAC);
        out.addChar(0x1EAE);
        out.addChar(0x1EB0);
        out.addChar(0x1EB2);
        out.addChar(0x1EB4);
        out.addChar(0x1EB6);
        out.addChar(0x1EB8);
        out.addChar(0x1EBA);
        out.addChar(0x1EBC);
        out.addChar(0x1EBE);
        out.addChar(0x1EC0);
        out.addChar(0x1EC2);
        out.addChar(0x1EC4);
        out.addChar(0x1EC6);
        out.addChar(0x1EC8);
        out.addChar(0x1ECA);
        out.addChar(0x1ECC);
        out.addChar(0x1ECE);
        out.addChar(0x1ED0);
        out.addChar(0x1ED2);
        out.addChar(0x1ED4);
        out.addChar(0x1ED6);
        out.addChar(0x1ED8);
        out.addChar(0x1EDA);
        out.addChar(0x1EDC);
        out.addChar(0x1EDE);
        out.addChar(0x1EE0);
        out.addChar(0x1EE2);
        out.addChar(0x1EE4);
        out.addChar(0x1EE6);
        out.addChar(0x1EE8);
        out.addChar(0x1EEA);
        out.addChar(0x1EEC);
        out.addChar(0x1EEE);
        out.addChar(0x1EF0);
        out.addChar(0x1EF2);
        out.addChar(0x1EF4);
        out.addChar(0x1EF6);
        out.addChar(0x1EF8);
        out.add(CharRange(0x1F08, 0x1F0F));
        out.add(CharRange(0x1F18, 0x1F1D));
        out.add(CharRange(0x1F28, 0x1F2F));
        out.add(CharRange(0x1F38, 0x1F3F));
        out.add(CharRange(0x1F48, 0x1F4D));
        out.addChar(0x1F59);
        out.addChar(0x1F5B);
        out.addChar(0x1F5D);
        out.addChar(0x1F5F);
        out.add(CharRange(0x1F68, 0x1F6F));
        out.add(CharRange(0x1FB8, 0x1FBB));
        out.add(CharRange(0x1FC8, 0x1FCB));
        out.add(CharRange(0x1FD8, 0x1FDB));
        out.add(CharRange(0x1FE8, 0x1FEC));
        out.add(CharRange(0x1FF8, 0x1FFB));
        out.addChar(0x2102);
        out.addChar(0x2107);
        out.add(CharRange(0x210B, 0x210D));
        out.add(CharRange(0x2110, 0x2112));
        out.addChar(0x2115);
        out.add(CharRange(0x2119, 0x211D));
        out.addChar(0x2124);
        out.addChar(0x2126);
        out.addChar(0x2128);
        out.add(CharRange(0x212A, 0x212D));
        out.add(CharRange(0x2130, 0x2131));
        out.addChar(0x2133);
        out.add(CharRange(0xFF21, 0xFF3A));
        break;
    case CLASS_LT: // \p{Lt}: titlecase letters
        out.addChar(0x1C5);
        out.addChar(0x1C8);
        out.addChar(0x1CB);
        out.addChar(0x1F2);
        out.add(CharRange(0x1F88, 0x1F8F));
        out.add(CharRange(0x1F98, 0x1F9F));
        out.add(CharRange(0x1FA8, 0x1FAF));
        out.addChar(0x1FBC);
        out.addChar(0x1FCC);
        out.addChar(0x1FFC);
        break;
    case CLASS_LM: // \p{Lm}: modifier letters
        out.add(CharRange(0x2B0, 0x2B8));
        out.add(CharRange(0x2BB, 0x2C1));
        out.add(CharRange(0x2D0, 0x2D1));
        out.add(CharRange(0x2E0, 0x2E4));
        out.addChar(0x2EE);
        out.addChar(0x37A);
        out.addChar(0x559);
        out.addChar(0x640);
        out.add(CharRange(0x6E5, 0x6E6));
        out.addChar(0xE46);
        out.addChar(0xEC6);
        out.addChar(0x1843);
        out.addChar(0x3005);
        out.add(CharRange(0x3031, 0x3035));
        out.add(CharRange(0x309D, 0x309E));
        out.add(CharRange(0x30FC, 0x30FE));
        out.addChar(0xFF70);
        out.add(CharRange(0xFF9E, 0xFF9F));
        break;
    case CLASS_LO: // \p{Lo}: other letters (kana, CJK ideographs, ...)
        out.addChar(0x1BB);
        out.add(CharRange(0x1C0, 0x1C3));
        out.add(CharRange(0x5D0, 0x5EA));
        out.add(CharRange(0x5F0, 0x5F2));
        out.add(CharRange(0x621, 0x63A));
        out.add(CharRange(0x641, 0x64A));
        out.add(CharRange(0x671, 0x6D3));
        out.addChar(0x6D5);
        out.add(CharRange(0x6FA, 0x6FC));
        out.addChar(0x710);
        out.add(CharRange(0x712, 0x72C));
        out.add(CharRange(0x780, 0x7A5));
        out.add(CharRange(0x905, 0x939));
        out.addChar(0x93D);
        out.addChar(0x950);
        out.add(CharRange(0x958, 0x961));
        out.add(CharRange(0x985, 0x98C));
        out.add(CharRange(0x98F, 0x990));
        out.add(CharRange(0x993, 0x9A8));
        out.add(CharRange(0x9AA, 0x9B0));
        out.addChar(0x9B2);
        out.add(CharRange(0x9B6, 0x9B9));
        out.add(CharRange(0x9DC, 0x9DD));
        out.add(CharRange(0x9DF, 0x9E1));
        out.add(CharRange(0x9F0, 0x9F1));
        out.add(CharRange(0xA05, 0xA0A));
        out.add(CharRange(0xA0F, 0xA10));
        out.add(CharRange(0xA13, 0xA28));
        out.add(CharRange(0xA2A, 0xA30));
        out.add(CharRange(0xA32, 0xA33));
        out.add(CharRange(0xA35, 0xA36));
        out.add(CharRange(0xA38, 0xA39));
        out.add(CharRange(0xA59, 0xA5C));
        out.addChar(0xA5E);
        out.add(CharRange(0xA72, 0xA74));
        out.add(CharRange(0xA85, 0xA8B));
        out.addChar(0xA8D);
        out.add(CharRange(0xA8F, 0xA91));
        out.add(CharRange(0xA93, 0xAA8));
        out.add(CharRange(0xAAA, 0xAB0));
        out.add(CharRange(0xAB2, 0xAB3));
        out.add(CharRange(0xAB5, 0xAB9));
        out.addChar(0xABD);
        out.addChar(0xAD0);
        out.addChar(0xAE0);
        out.add(CharRange(0xB05, 0xB0C));
        out.add(CharRange(0xB0F, 0xB10));
        out.add(CharRange(0xB13, 0xB28));
        out.add(CharRange(0xB2A, 0xB30));
        out.add(CharRange(0xB32, 0xB33));
        out.add(CharRange(0xB36, 0xB39));
        out.addChar(0xB3D);
        out.add(CharRange(0xB5C, 0xB5D));
        out.add(CharRange(0xB5F, 0xB61));
        out.add(CharRange(0xB85, 0xB8A));
        out.add(CharRange(0xB8E, 0xB90));
        out.add(CharRange(0xB92, 0xB95));
        out.add(CharRange(0xB99, 0xB9A));
        out.addChar(0xB9C);
        out.add(CharRange(0xB9E, 0xB9F));
        out.add(CharRange(0xBA3, 0xBA4));
        out.add(CharRange(0xBA8, 0xBAA));
        out.add(CharRange(0xBAE, 0xBB5));
        out.add(CharRange(0xBB7, 0xBB9));
        out.add(CharRange(0xC05, 0xC0C));
        out.add(CharRange(0xC0E, 0xC10));
        out.add(CharRange(0xC12, 0xC28));
        out.add(CharRange(0xC2A, 0xC33));
        out.add(CharRange(0xC35, 0xC39));
        out.add(CharRange(0xC60, 0xC61));
        out.add(CharRange(0xC85, 0xC8C));
        out.add(CharRange(0xC8E, 0xC90));
        out.add(CharRange(0xC92, 0xCA8));
        out.add(CharRange(0xCAA, 0xCB3));
        out.add(CharRange(0xCB5, 0xCB9));
        out.addChar(0xCDE);
        out.add(CharRange(0xCE0, 0xCE1));
        out.add(CharRange(0xD05, 0xD0C));
        out.add(CharRange(0xD0E, 0xD10));
        out.add(CharRange(0xD12, 0xD28));
        out.add(CharRange(0xD2A, 0xD39));
        out.add(CharRange(0xD60, 0xD61));
        out.add(CharRange(0xD85, 0xD96));
        out.add(CharRange(0xD9A, 0xDB1));
        out.add(CharRange(0xDB3, 0xDBB));
        out.addChar(0xDBD);
        out.add(CharRange(0xDC0, 0xDC6));
        out.add(CharRange(0xE01, 0xE30));
        out.add(CharRange(0xE32, 0xE33));
        out.add(CharRange(0xE40, 0xE45));
        out.add(CharRange(0xE81, 0xE82));
        out.addChar(0xE84);
        out.add(CharRange(0xE87, 0xE88));
        out.addChar(0xE8A);
        out.addChar(0xE8D);
        out.add(CharRange(0xE94, 0xE97));
        out.add(CharRange(0xE99, 0xE9F));
        out.add(CharRange(0xEA1, 0xEA3));
        out.addChar(0xEA5);
        out.addChar(0xEA7);
        out.add(CharRange(0xEAA, 0xEAB));
        out.add(CharRange(0xEAD, 0xEB0));
        out.add(CharRange(0xEB2, 0xEB3));
        out.addChar(0xEBD);
        out.add(CharRange(0xEC0, 0xEC4));
        out.add(CharRange(0xEDC, 0xEDD));
        out.addChar(0xF00);
        out.add(CharRange(0xF40, 0xF47));
        out.add(CharRange(0xF49, 0xF6A));
        out.add(CharRange(0xF88, 0xF8B));
        out.add(CharRange(0x1000, 0x1021));
        out.add(CharRange(0x1023, 0x1027));
        out.add(CharRange(0x1029, 0x102A));
        out.add(CharRange(0x1050, 0x1055));
        out.add(CharRange(0x10D0, 0x10F6));
        out.add(CharRange(0x1100, 0x1159));
        out.add(CharRange(0x115F, 0x11A2));
        out.add(CharRange(0x11A8, 0x11F9));
        out.add(CharRange(0x1200, 0x1206));
        out.add(CharRange(0x1208, 0x1246));
        out.addChar(0x1248);
        out.add(CharRange(0x124A, 0x124D));
        out.add(CharRange(0x1250, 0x1256));
        out.addChar(0x1258);
        out.add(CharRange(0x125A, 0x125D));
        out.add(CharRange(0x1260, 0x1286));
        out.addChar(0x1288);
        out.add(CharRange(0x128A, 0x128D));
        out.add(CharRange(0x1290, 0x12AE));
        out.addChar(0x12B0);
        out.add(CharRange(0x12B2, 0x12B5));
        out.add(CharRange(0x12B8, 0x12BE));
        out.addChar(0x12C0);
        out.add(CharRange(0x12C2, 0x12C5));
        out.add(CharRange(0x12C8, 0x12CE));
        out.add(CharRange(0x12D0, 0x12D6));
        out.add(CharRange(0x12D8, 0x12EE));
        out.add(CharRange(0x12F0, 0x130E));
        out.addChar(0x1310);
        out.add(CharRange(0x1312, 0x1315));
        out.add(CharRange(0x1318, 0x131E));
        out.add(CharRange(0x1320, 0x1346));
        out.add(CharRange(0x1348, 0x135A));
        out.add(CharRange(0x13A0, 0x13F4));
        out.add(CharRange(0x1401, 0x166C));
        out.add(CharRange(0x166F, 0x1676));
        out.add(CharRange(0x1681, 0x169A));
        out.add(CharRange(0x16A0, 0x16EA));
        out.add(CharRange(0x1780, 0x17B3));
        out.add(CharRange(0x1820, 0x1842));
        out.add(CharRange(0x1844, 0x1877));
        out.add(CharRange(0x1880, 0x18A8));
        out.add(CharRange(0x2135, 0x2138));
        out.addChar(0x3006);
        out.add(CharRange(0x3041, 0x3094));
        out.add(CharRange(0x30A1, 0x30FA));
        out.add(CharRange(0x3105, 0x312C));
        out.add(CharRange(0x3131, 0x318E));
        out.add(CharRange(0x31A0, 0x31B7));
        out.add(CharRange(0x3400, 0x4DB5));
        out.add(CharRange(0x4E00, 0x9FA5));
        out.add(CharRange(0xA000, 0xA48C));
        out.add(CharRange(0xAC00, 0xD7A3));
        out.add(CharRange(0xF900, 0xFA2D));
        out.addChar(0xFB1D);
        out.add(CharRange(0xFB1F, 0xFB28));
        out.add(CharRange(0xFB2A, 0xFB36));
        out.add(CharRange(0xFB38, 0xFB3C));
        out.addChar(0xFB3E);
        out.add(CharRange(0xFB40, 0xFB41));
        out.add(CharRange(0xFB43, 0xFB44));
        out.add(CharRange(0xFB46, 0xFBB1));
        out.add(CharRange(0xFBD3, 0xFD3D));
        out.add(CharRange(0xFD50, 0xFD8F));
        out.add(CharRange(0xFD92, 0xFDC7));
        out.add(CharRange(0xFDF0, 0xFDFB));
        out.add(CharRange(0xFE70, 0xFE72));
        out.addChar(0xFE74);
        out.add(CharRange(0xFE76, 0xFEFC));
        out.add(CharRange(0xFF66, 0xFF6F));
        out.add(CharRange(0xFF71, 0xFF9D));
        out.add(CharRange(0xFFA0, 0xFFBE));
        out.add(CharRange(0xFFC2, 0xFFC7));
        out.add(CharRange(0xFFCA, 0xFFCF));
        out.add(CharRange(0xFFD2, 0xFFD7));
        out.add(CharRange(0xFFDA, 0xFFDC));
        break;
    case CLASS_M: // \p{M}: marks (Mn + Mc + Me)
        buildPredefinedClass(out, CLASS_MN);
        buildPredefinedClass(out, CLASS_MC);
        buildPredefinedClass(out, CLASS_ME);
        break;
    case CLASS_MN: // \p{Mn}: non-spacing marks
        out.add(CharRange(0x300, 0x34E));
        out.add(CharRange(0x360, 0x362));
        out.add(CharRange(0x483, 0x486));
        out.add(CharRange(0x591, 0x5A1));
        out.add(CharRange(0x5A3, 0x5B9));
        out.add(CharRange(0x5BB, 0x5BD));
        out.addChar(0x5BF);
        out.add(CharRange(0x5C1, 0x5C2));
        out.addChar(0x5C4);
        out.add(CharRange(0x64B, 0x655));
        out.addChar(0x670);
        out.add(CharRange(0x6D6, 0x6DC));
        out.add(CharRange(0x6DF, 0x6E4));
        out.add(CharRange(0x6E7, 0x6E8));
        out.add(CharRange(0x6EA, 0x6ED));
        out.addChar(0x711);
        out.add(CharRange(0x730, 0x74A));
        out.add(CharRange(0x7A6, 0x7B0));
        out.add(CharRange(0x901, 0x902));
        out.addChar(0x93C);
        out.add(CharRange(0x941, 0x948));
        out.addChar(0x94D);
        out.add(CharRange(0x951, 0x954));
        out.add(CharRange(0x962, 0x963));
        out.addChar(0x981);
        out.addChar(0x9BC);
        out.add(CharRange(0x9C1, 0x9C4));
        out.addChar(0x9CD);
        out.add(CharRange(0x9E2, 0x9E3));
        out.addChar(0xA02);
        out.addChar(0xA3C);
        out.add(CharRange(0xA41, 0xA42));
        out.add(CharRange(0xA47, 0xA48));
        out.add(CharRange(0xA4B, 0xA4D));
        out.add(CharRange(0xA70, 0xA71));
        out.add(CharRange(0xA81, 0xA82));
        out.addChar(0xABC);
        out.add(CharRange(0xAC1, 0xAC5));
        out.add(CharRange(0xAC7, 0xAC8));
        out.addChar(0xACD);
        out.addChar(0xB01);
        out.addChar(0xB3C);
        out.addChar(0xB3F);
        out.add(CharRange(0xB41, 0xB43));
        out.addChar(0xB4D);
        out.addChar(0xB56);
        out.addChar(0xB82);
        out.addChar(0xBC0);
        out.addChar(0xBCD);
        out.add(CharRange(0xC3E, 0xC40));
        out.add(CharRange(0xC46, 0xC48));
        out.add(CharRange(0xC4A, 0xC4D));
        out.add(CharRange(0xC55, 0xC56));
        out.addChar(0xCBF);
        out.addChar(0xCC6);
        out.add(CharRange(0xCCC, 0xCCD));
        out.add(CharRange(0xD41, 0xD43));
        out.addChar(0xD4D);
        out.addChar(0xDCA);
        out.add(CharRange(0xDD2, 0xDD4));
        out.addChar(0xDD6);
        out.addChar(0xE31);
        out.add(CharRange(0xE34, 0xE3A));
        out.add(CharRange(0xE47, 0xE4E));
        out.addChar(0xEB1);
        out.add(CharRange(0xEB4, 0xEB9));
        out.add(CharRange(0xEBB, 0xEBC));
        out.add(CharRange(0xEC8, 0xECD));
        out.add(CharRange(0xF18, 0xF19));
        out.addChar(0xF35);
        out.addChar(0xF37);
        out.addChar(0xF39);
        out.add(CharRange(0xF71, 0xF7E));
        out.add(CharRange(0xF80, 0xF84));
        out.add(CharRange(0xF86, 0xF87));
        out.add(CharRange(0xF90, 0xF97));
        out.add(CharRange(0xF99, 0xFBC));
        out.addChar(0xFC6);
        out.add(CharRange(0x102D, 0x1030));
        out.addChar(0x1032);
        out.add(CharRange(0x1036, 0x1037));
        out.addChar(0x1039);
        out.add(CharRange(0x1058, 0x1059));
        out.add(CharRange(0x17B7, 0x17BD));
        out.addChar(0x17C6);
        out.add(CharRange(0x17C9, 0x17D3));
        out.addChar(0x18A9);
        out.add(CharRange(0x20D0, 0x20DC));
        out.addChar(0x20E1);
        out.add(CharRange(0x302A, 0x302F));
        out.add(CharRange(0x3099, 0x309A));
        out.addChar(0xFB1E);
        out.add(CharRange(0xFE20, 0xFE23));
        break;
    case CLASS_MC: // \p{Mc}: spacing combining marks
        out.addChar(0x903);
        out.add(CharRange(0x93E, 0x940));
        out.add(CharRange(0x949, 0x94C));
        out.add(CharRange(0x982, 0x983));
        out.add(CharRange(0x9BE, 0x9C0));
        out.add(CharRange(0x9C7, 0x9C8));
        out.add(CharRange(0x9CB, 0x9CC));
        out.addChar(0x9D7);
        out.add(CharRange(0xA3E, 0xA40));
        out.addChar(0xA83);
        out.add(CharRange(0xABE, 0xAC0));
        out.addChar(0xAC9);
        out.add(CharRange(0xACB, 0xACC));
        out.add(CharRange(0xB02, 0xB03));
        out.addChar(0xB3E);
        out.addChar(0xB40);
        out.add(CharRange(0xB47, 0xB48));
        out.add(CharRange(0xB4B, 0xB4C));
        out.addChar(0xB57);
        out.addChar(0xB83);
        out.add(CharRange(0xBBE, 0xBBF));
        out.add(CharRange(0xBC1, 0xBC2));
        out.add(CharRange(0xBC6, 0xBC8));
        out.add(CharRange(0xBCA, 0xBCC));
        out.addChar(0xBD7);
        out.add(CharRange(0xC01, 0xC03));
        out.add(CharRange(0xC41, 0xC44));
        out.add(CharRange(0xC82, 0xC83));
        out.addChar(0xCBE);
        out.add(CharRange(0xCC0, 0xCC4));
        out.add(CharRange(0xCC7, 0xCC8));
        out.add(CharRange(0xCCA, 0xCCB));
        out.add(CharRange(0xCD5, 0xCD6));
        out.add(CharRange(0xD02, 0xD03));
        out.add(CharRange(0xD3E, 0xD40));
        out.add(CharRange(0xD46, 0xD48));
        out.add(CharRange(0xD4A, 0xD4C));
        out.addChar(0xD57);
        out.add(CharRange(0xD82, 0xD83));
        out.add(CharRange(0xDCF, 0xDD1));
        out.add(CharRange(0xDD8, 0xDDF));
        out.add(CharRange(0xDF2, 0xDF3));
        out.add(CharRange(0xF3E, 0xF3F));
        out.addChar(0xF7F);
        out.addChar(0x102C);
        out.addChar(0x1031);
        out.addChar(0x1038);
        out.add(CharRange(0x1056, 0x1057));
        out.add(CharRange(0x17B4, 0x17B6));
        out.add(CharRange(0x17BE, 0x17C5));
        out.add(CharRange(0x17C7, 0x17C8));
        break;
    case CLASS_ME: // \p{Me}: enclosing marks
        out.add(CharRange(0x488, 0x489));
        out.add(CharRange(0x6DD, 0x6DE));
        out.add(CharRange(0x20DD, 0x20E0));
        out.add(CharRange(0x20E2, 0x20E3));
        break;
    case CLASS_Z: // \p{Z}: separators (Zs + Zl + Zp)
        buildPredefinedClass(out, CLASS_ZS);
        buildPredefinedClass(out, CLASS_ZL);
        buildPredefinedClass(out, CLASS_ZP);
        break;
    case CLASS_ZS: // \p{Zs}: space separators
        out.addChar(0x20);
        out.addChar(0xA0);
        out.addChar(0x1680);
        out.add(CharRange(0x2000, 0x200B));
        out.addChar(0x202F);
        out.addChar(0x3000);
        break;
    case CLASS_ZL: // \p{Zl}: line separator (U+2028)
        out.addChar(0x2028);
        break;
    case CLASS_ZP: // \p{Zp}: paragraph separator (U+2029)
        out.addChar(0x2029);
        break;
    case CLASS_S: // \p{S}: symbols (Sm + Sc + Sk + So)
        buildPredefinedClass(out, CLASS_SM);
        buildPredefinedClass(out, CLASS_SC);
        buildPredefinedClass(out, CLASS_SK);
        buildPredefinedClass(out, CLASS_SO);
        break;
    case CLASS_SM: // \p{Sm}: math symbols
        out.addChar(0x2B);
        out.add(CharRange(0x3C, 0x3E));
        out.addChar(0x7C);
        out.addChar(0x7E);
        out.addChar(0xAC);
        out.addChar(0xB1);
        out.addChar(0xD7);
        out.addChar(0xF7);
        out.addChar(0x2044);
        out.add(CharRange(0x207A, 0x207C));
        out.add(CharRange(0x208A, 0x208C));
        out.add(CharRange(0x2190, 0x2194));
        out.add(CharRange(0x219A, 0x219B));
        out.addChar(0x21A0);
        out.addChar(0x21A3);
        out.addChar(0x21A6);
        out.addChar(0x21AE);
        out.add(CharRange(0x21CE, 0x21CF));
        out.addChar(0x21D2);
        out.addChar(0x21D4);
        out.add(CharRange(0x2200, 0x22F1));
        out.add(CharRange(0x2308, 0x230B));
        out.add(CharRange(0x2320, 0x2321));
        out.addChar(0x25B7);
        out.addChar(0x25C1);
        out.addChar(0x266F);
        out.addChar(0xFB29);
        out.addChar(0xFE62);
        out.add(CharRange(0xFE64, 0xFE66));
        out.addChar(0xFF0B);
        out.add(CharRange(0xFF1C, 0xFF1E));
        out.addChar(0xFF5C);
        out.addChar(0xFF5E);
        out.addChar(0xFFE2);
        out.add(CharRange(0xFFE9, 0xFFEC));
        break;
    case CLASS_SC: // \p{Sc}: currency symbols
        out.addChar(0x24);
        out.add(CharRange(0xA2, 0xA5));
        out.add(CharRange(0x9F2, 0x9F3));
        out.addChar(0xE3F);
        out.addChar(0x17DB);
        out.add(CharRange(0x20A0, 0x20AF));
        out.addChar(0xFE69);
        out.addChar(0xFF04);
        out.add(CharRange(0xFFE0, 0xFFE1));
        out.add(CharRange(0xFFE5, 0xFFE6));
        break;
    case CLASS_SK: // \p{Sk}: modifier symbols
        out.addChar(0x5E);
        out.addChar(0x60);
        out.addChar(0xA8);
        out.addChar(0xAF);
        out.addChar(0xB4);
        out.addChar(0xB8);
        out.add(CharRange(0x2B9, 0x2BA));
        out.add(CharRange(0x2C2, 0x2CF));
        out.add(CharRange(0x2D2, 0x2DF));
        out.add(CharRange(0x2E5, 0x2ED));
        out.add(CharRange(0x374, 0x375));
        out.add(CharRange(0x384, 0x385));
        out.addChar(0x1FBD);
        out.add(CharRange(0x1FBF, 0x1FC1));
        out.add(CharRange(0x1FCD, 0x1FCF));
        out.add(CharRange(0x1FDD, 0x1FDF));
        out.add(CharRange(0x1FED, 0x1FEF));
        out.add(CharRange(0x1FFD, 0x1FFE));
        out.add(CharRange(0x309B, 0x309C));
        out.addChar(0xFF3E);
        out.addChar(0xFF40);
        out.addChar(0xFFE3);
        break;
    case CLASS_SO: // \p{So}: other symbols
        out.add(CharRange(0xA6, 0xA7));
        out.addChar(0xA9);
        out.addChar(0xAE);
        out.addChar(0xB0);
        out.addChar(0xB6);
        out.addChar(0x482);
        out.addChar(0x6E9);
        out.add(CharRange(0x6FD, 0x6FE));
        out.addChar(0x9FA);
        out.addChar(0xB70);
        out.add(CharRange(0xF01, 0xF03));
        out.add(CharRange(0xF13, 0xF17));
        out.add(CharRange(0xF1A, 0xF1F));
        out.addChar(0xF34);
        out.addChar(0xF36);
        out.addChar(0xF38);
        out.add(CharRange(0xFBE, 0xFC5));
        out.add(CharRange(0xFC7, 0xFCC));
        out.addChar(0xFCF);
        out.add(CharRange(0x2100, 0x2101));
        out.add(CharRange(0x2103, 0x2106));
        out.add(CharRange(0x2108, 0x2109));
        out.addChar(0x2114);
        out.add(CharRange(0x2116, 0x2118));
        out.add(CharRange(0x211E, 0x2123));
        out.addChar(0x2125);
        out.addChar(0x2127);
        out.addChar(0x2129);
        out.addChar(0x212E);
        out.addChar(0x2132);
        out.addChar(0x213A);
        out.add(CharRange(0x2195, 0x2199));
        out.add(CharRange(0x219C, 0x219F));
        out.add(CharRange(0x21A1, 0x21A2));
        out.add(CharRange(0x21A4, 0x21A5));
        out.add(CharRange(0x21A7, 0x21AD));
        out.add(CharRange(0x21AF, 0x21CD));
        out.add(CharRange(0x21D0, 0x21D1));
        out.addChar(0x21D3);
        out.add(CharRange(0x21D5, 0x21F3));
        out.add(CharRange(0x2300, 0x2307));
        out.add(CharRange(0x230C, 0x231F));
        out.add(CharRange(0x2322, 0x2328));
        out.add(CharRange(0x232B, 0x237B));
        out.add(CharRange(0x237D, 0x239A));
        out.add(CharRange(0x2400, 0x2426));
        out.add(CharRange(0x2440, 0x244A));
        out.add(CharRange(0x249C, 0x24E9));
        out.add(CharRange(0x2500, 0x2595));
        out.add(CharRange(0x25A0, 0x25B6));
        out.add(CharRange(0x25B8, 0x25C0));
        out.add(CharRange(0x25C2, 0x25F7));
        out.add(CharRange(0x2600, 0x2613));
        out.add(CharRange(0x2619, 0x266E));
        out.add(CharRange(0x2670, 0x2671));
        out.add(CharRange(0x2701, 0x2704));
        out.add(CharRange(0x2706, 0x2709));
        out.add(CharRange(0x270C, 0x2727));
        out.add(CharRange(0x2729, 0x274B));
        out.addChar(0x274D);
        out.add(CharRange(0x274F, 0x2752));
        out.addChar(0x2756);
        out.add(CharRange(0x2758, 0x275E));
        out.add(CharRange(0x2761, 0x2767));
        out.addChar(0x2794);
        out.add(CharRange(0x2798, 0x27AF));
        out.add(CharRange(0x27B1, 0x27BE));
        out.add(CharRange(0x2800, 0x28FF));
        out.add(CharRange(0x2E80, 0x2E99));
        out.add(CharRange(0x2E9B, 0x2EF3));
        out.add(CharRange(0x2F00, 0x2FD5));
        out.add(CharRange(0x2FF0, 0x2FFB));
        out.addChar(0x3004);
        out.add(CharRange(0x3012, 0x3013));
        out.addChar(0x3020);
        out.add(CharRange(0x3036, 0x3037));
        out.add(CharRange(0x303E, 0x303F));
        out.add(CharRange(0x3190, 0x3191));
        out.add(CharRange(0x3196, 0x319F));
        out.add(CharRange(0x3200, 0x321C));
        out.add(CharRange(0x322A, 0x3243));
        out.add(CharRange(0x3260, 0x327B));
        out.addChar(0x327F);
        out.add(CharRange(0x328A, 0x32B0));
        out.add(CharRange(0x32C0, 0x32CB));
        out.add(CharRange(0x32D0, 0x32FE));
        out.add(CharRange(0x3300, 0x3376));
        out.add(CharRange(0x337B, 0x33DD));
        out.add(CharRange(0x33E0, 0x33FE));
        out.add(CharRange(0xA490, 0xA4A1));
        out.add(CharRange(0xA4A4, 0xA4B3));
        out.add(CharRange(0xA4B5, 0xA4C0));
        out.add(CharRange(0xA4C2, 0xA4C4));
        out.addChar(0xA4C6);
        out.addChar(0xFFE4);
        out.addChar(0xFFE8);
        out.add(CharRange(0xFFED, 0xFFEE));
        out.add(CharRange(0xFFFC, 0xFFFD));
        break;
    case CLASS_N: // \p{N}: numbers (Nd + Nl + No)
        buildPredefinedClass(out, CLASS_ND);
        buildPredefinedClass(out, CLASS_NL);
        buildPredefinedClass(out, CLASS_NO);
        break;
    case CLASS_ND: // \p{Nd}: decimal digits
        out.add(CharRange(0x30, 0x39));
        out.add(CharRange(0x660, 0x669));
        out.add(CharRange(0x6F0, 0x6F9));
        out.add(CharRange(0x966, 0x96F));
        out.add(CharRange(0x9E6, 0x9EF));
        out.add(CharRange(0xA66, 0xA6F));
        out.add(CharRange(0xAE6, 0xAEF));
        out.add(CharRange(0xB66, 0xB6F));
        out.add(CharRange(0xBE7, 0xBEF));
        out.add(CharRange(0xC66, 0xC6F));
        out.add(CharRange(0xCE6, 0xCEF));
        out.add(CharRange(0xD66, 0xD6F));
        out.add(CharRange(0xE50, 0xE59));
        out.add(CharRange(0xED0, 0xED9));
        out.add(CharRange(0xF20, 0xF29));
        out.add(CharRange(0x1040, 0x1049));
        out.add(CharRange(0x1369, 0x1371));
        out.add(CharRange(0x17E0, 0x17E9));
        out.add(CharRange(0x1810, 0x1819));
        out.add(CharRange(0xFF10, 0xFF19));
        break;
    case CLASS_NL: // \p{Nl}: letter numbers
        out.add(CharRange(0x2160, 0x2183));
        out.addChar(0x3007);
        out.add(CharRange(0x3021, 0x3029));
        out.add(CharRange(0x3038, 0x303A));
        break;
    case CLASS_NO: // \p{No}: other numbers
        out.add(CharRange(0xB2, 0xB3));
        out.addChar(0xB9);
        out.add(CharRange(0xBC, 0xBE));
        out.add(CharRange(0x9F4, 0x9F9));
        out.add(CharRange(0xBF0, 0xBF2));
        out.add(CharRange(0xF2A, 0xF33));
        out.add(CharRange(0x1372, 0x137C));
        out.add(CharRange(0x16EE, 0x16F0));
        out.addChar(0x2070);
        out.add(CharRange(0x2074, 0x2079));
        out.add(CharRange(0x2080, 0x2089));
        out.add(CharRange(0x2153, 0x215F));
        out.add(CharRange(0x2460, 0x249B));
        out.addChar(0x24EA);
        out.add(CharRange(0x2776, 0x2793));
        out.add(CharRange(0x3192, 0x3195));
        out.add(CharRange(0x3220, 0x3229));
        out.add(CharRange(0x3280, 0x3289));
        break;
    case CLASS_P: // \p{P}: punctuation (Pd + Ps + Pe + Pc + Po)
        buildPredefinedClass(out, CLASS_PD);
        buildPredefinedClass(out, CLASS_PS);
        buildPredefinedClass(out, CLASS_PE);
        buildPredefinedClass(out, CLASS_PC);
        buildPredefinedClass(out, CLASS_PO);
        break;
    case CLASS_PD: // \p{Pd}: dash punctuation
        out.addChar(0xAD);
        out.addChar(0x2D);
        out.addChar(0x58A);
        out.addChar(0x1806);
        out.add(CharRange(0x2010, 0x2015));
        out.addChar(0x301C);
        out.addChar(0x3030);
        out.add(CharRange(0xFE31, 0xFE32));
        out.addChar(0xFE58);
        out.addChar(0xFE63);
        out.addChar(0xFF0D);
        break;
    case CLASS_PS: // \p{Ps}: opening punctuation
        out.addChar(0x28);
        out.addChar(0x5B);
        out.addChar(0x7B);
        out.addChar(0xF3A);
        out.addChar(0xF3C);
        out.addChar(0x169B);
        out.addChar(0x201A);
        out.addChar(0x201E);
        out.addChar(0x2045);
        out.addChar(0x207D);
        out.addChar(0x208D);
        out.addChar(0x2329);
        out.addChar(0x3008);
        out.addChar(0x300A);
        out.addChar(0x300C);
        out.addChar(0x300E);
        out.addChar(0x3010);
        out.addChar(0x3014);
        out.addChar(0x3016);
        out.addChar(0x3018);
        out.addChar(0x301A);
        out.addChar(0x301D);
        out.addChar(0xFD3E);
        out.addChar(0xFE35);
        out.addChar(0xFE37);
        out.addChar(0xFE39);
        out.addChar(0xFE3B);
        out.addChar(0xFE3D);
        out.addChar(0xFE3F);
        out.addChar(0xFE41);
        out.addChar(0xFE43);
        out.addChar(0xFE59);
        out.addChar(0xFE5B);
        out.addChar(0xFE5D);
        out.addChar(0xFF08);
        out.addChar(0xFF3B);
        out.addChar(0xFF5B);
        out.addChar(0xFF62);
        break;
    case CLASS_PE: // \p{Pe}: closing punctuation
        out.addChar(0x29);
        out.addChar(0x5D);
        out.addChar(0x7D);
        out.addChar(0xF3B);
        out.addChar(0xF3D);
        out.addChar(0x169C);
        out.addChar(0x2046);
        out.addChar(0x207E);
        out.addChar(0x208E);
        out.addChar(0x232A);
        out.addChar(0x3009);
        out.addChar(0x300B);
        out.addChar(0x300D);
        out.addChar(0x300F);
        out.addChar(0x3011);
        out.addChar(0x3015);
        out.addChar(0x3017);
        out.addChar(0x3019);
        out.addChar(0x301B);
        out.add(CharRange(0x301E, 0x301F));
        out.addChar(0xFD3F);
        out.addChar(0xFE36);
        out.addChar(0xFE38);
        out.addChar(0xFE3A);
        out.addChar(0xFE3C);
        out.addChar(0xFE3E);
        out.addChar(0xFE40);
        out.addChar(0xFE42);
        out.addChar(0xFE44);
        out.addChar(0xFE5A);
        out.addChar(0xFE5C);
        out.addChar(0xFE5E);
        out.addChar(0xFF09);
        out.addChar(0xFF3D);
        out.addChar(0xFF5D);
        out.addChar(0xFF63);
        break;
    case CLASS_PC: // \p{Pc}: connector punctuation
        out.addChar(0x5F);
        out.add(CharRange(0x203F, 0x2040));
        out.addChar(0x30FB);
        out.add(CharRange(0xFE33, 0xFE34));
        out.add(CharRange(0xFE4D, 0xFE4F));
        out.addChar(0xFF3F);
        out.addChar(0xFF65);
        break;
    case CLASS_PO: // \p{Po}: other punctuation
        out.add(CharRange(0x21, 0x23));
        out.add(CharRange(0x25, 0x27));
        out.addChar(0x2A);
        out.addChar(0x2C);
        out.add(CharRange(0x2E, 0x2F));
        out.add(CharRange(0x3A, 0x3B));
        out.add(CharRange(0x3F, 0x40));
        out.addChar(0x5C);
        out.addChar(0xA1);
        out.addChar(0xB7);
        out.addChar(0xBF);
        out.addChar(0x37E);
        out.addChar(0x387);
        out.add(CharRange(0x55A, 0x55F));
        out.addChar(0x589);
        out.addChar(0x5BE);
        out.addChar(0x5C0);
        out.addChar(0x5C3);
        out.add(CharRange(0x5F3, 0x5F4));
        out.addChar(0x60C);
        out.addChar(0x61B);
        out.addChar(0x61F);
        out.add(CharRange(0x66A, 0x66D));
        out.addChar(0x6D4);
        out.add(CharRange(0x700, 0x70D));
        out.add(CharRange(0x964, 0x965));
        out.addChar(0x970);
        out.addChar(0xDF4);
        out.addChar(0xE4F);
        out.add(CharRange(0xE5A, 0xE5B));
        out.add(CharRange(0xF04, 0xF12));
        out.addChar(0xF85);
        out.add(CharRange(0x104A, 0x104F));
        out.addChar(0x10FB);
        out.add(CharRange(0x1361, 0x1368));
        out.add(CharRange(0x166D, 0x166E));
        out.add(CharRange(0x16EB, 0x16ED));
        out.add(CharRange(0x17D4, 0x17DA));
        out.addChar(0x17DC);
        out.add(CharRange(0x1800, 0x1805));
        out.add(CharRange(0x1807, 0x180A));
        out.add(CharRange(0x2016, 0x2017));
        out.add(CharRange(0x2020, 0x2027));
        out.add(CharRange(0x2030, 0x2038));
        out.add(CharRange(0x203B, 0x203E));
        out.add(CharRange(0x2041, 0x2043));
        out.add(CharRange(0x2048, 0x204D));
        out.add(CharRange(0x3001, 0x3003));
        out.addChar(0xFE30);
        out.add(CharRange(0xFE49, 0xFE4C));
        out.add(CharRange(0xFE50, 0xFE52));
        out.add(CharRange(0xFE54, 0xFE57));
        out.add(CharRange(0xFE5F, 0xFE61));
        out.addChar(0xFE68);
        out.add(CharRange(0xFE6A, 0xFE6B));
        out.add(CharRange(0xFF01, 0xFF03));
        out.add(CharRange(0xFF05, 0xFF07));
        out.addChar(0xFF0A);
        out.addChar(0xFF0C);
        out.add(CharRange(0xFF0E, 0xFF0F));
        out.add(CharRange(0xFF1A, 0xFF1B));
        out.add(CharRange(0xFF1F, 0xFF20));
        out.addChar(0xFF3C);
        out.addChar(0xFF61);
        out.addChar(0xFF64);
        break;
    case CLASS_C: // \p{C}: other (Cc + Cf + Co + Cs + Cn)
        buildPredefinedClass(out, CLASS_CC);
        buildPredefinedClass(out, CLASS_CF);
        buildPredefinedClass(out, CLASS_CO);
        buildPredefinedClass(out, CLASS_CS);
        break;
    case CLASS_CC: // \p{Cc}: control characters (C0, DEL, C1)
        out.add(CharRange(0x0, 0x1F));
        out.add(CharRange(0x7F, 0x9F));
        break;
    case CLASS_CF: // \p{Cf}: format characters
        out.addChar(0x70F);
        out.add(CharRange(0x180B, 0x180E));
        out.add(CharRange(0x200C, 0x200F));
        out.add(CharRange(0x202A, 0x202E));
        out.add(CharRange(0x206A, 0x206F));
        out.addChar(0xFEFF);
        out.add(CharRange(0xFFF9, 0xFFFB));
        break;
    case CLASS_CO: // \p{Co}: private use (U+E000..U+F8FF)
        out.add(CharRange(0xE000, 0xF8FF));
        break;
    case CLASS_CS: // \p{Cs}: surrogates (U+D800..U+DFFF)
        out.add(CharRange(0xD800, 0xDFFF));
        break;
    case CLASS_CN: // \p{Cn}: unassigned code points
        out.add(CharRange(0x220, 0x221));
        out.add(CharRange(0x234, 0x24F));
        out.add(CharRange(0x2AE, 0x2AF));
        out.add(CharRange(0x2EF, 0x2FF));
        out.add(CharRange(0x34F, 0x35F));
        out.add(CharRange(0x363, 0x373));
        out.add(CharRange(0x376, 0x379));
        out.add(CharRange(0x37B, 0x37D));
        out.add(CharRange(0x37F, 0x383));
        out.addChar(0x38B);
        out.addChar(0x38D);
        out.addChar(0x3A2);
        out.addChar(0x3CF);
        out.add(CharRange(0x3D8, 0x3D9));
        out.add(CharRange(0x3F4, 0x3FF));
        out.addChar(0x487);
        out.add(CharRange(0x48A, 0x48B));
        out.add(CharRange(0x4C5, 0x4C6));
        out.add(CharRange(0x4C9, 0x4CA));
        out.add(CharRange(0x4CD, 0x4CF));
        out.add(CharRange(0x4F6, 0x4F7));
        out.add(CharRange(0x4FA, 0x530));
        out.add(CharRange(0x557, 0x558));
        out.addChar(0x560);
        out.addChar(0x588);
        out.add(CharRange(0x58B, 0x590));
        out.addChar(0x5A2);
        out.addChar(0x5BA);
        out.add(CharRange(0x5C5, 0x5CF));
        out.add(CharRange(0x5EB, 0x5EF));
        out.add(CharRange(0x5F5, 0x60B));
        out.add(CharRange(0x60D, 0x61A));
        out.add(CharRange(0x61C, 0x61E));
        out.addChar(0x620);
        out.add(CharRange(0x63B, 0x63F));
        out.add(CharRange(0x656, 0x65F));
        out.add(CharRange(0x66E, 0x66F));
        out.add(CharRange(0x6EE, 0x6EF));
        out.addChar(0x6FF);
        out.addChar(0x70E);
        out.add(CharRange(0x72D, 0x72F));
        out.add(CharRange(0x74B, 0x77F));
        out.add(CharRange(0x7B1, 0x900));
        out.addChar(0x904);
        out.add(CharRange(0x93A, 0x93B));
        out.add(CharRange(0x94E, 0x94F));
        out.add(CharRange(0x955, 0x957));
        out.add(CharRange(0x971, 0x980));
        out.addChar(0x984);
        out.add(CharRange(0x98D, 0x98E));
        out.add(CharRange(0x991, 0x992));
        out.addChar(0x9A9);
        out.addChar(0x9B1);
        out.add(CharRange(0x9B3, 0x9B5));
        out.add(CharRange(0x9BA, 0x9BB));
        out.addChar(0x9BD);
        out.add(CharRange(0x9C5, 0x9C6));
        out.add(CharRange(0x9C9, 0x9CA));
        out.add(CharRange(0x9CE, 0x9D6));
        out.add(CharRange(0x9D8, 0x9DB));
        out.addChar(0x9DE);
        out.add(CharRange(0x9E4, 0x9E5));
        out.add(CharRange(0x9FB, 0xA01));
        out.add(CharRange(0xA03, 0xA04));
        out.add(CharRange(0xA0B, 0xA0E));
        out.add(CharRange(0xA11, 0xA12));
        out.addChar(0xA29);
        out.addChar(0xA31);
        out.addChar(0xA34);
        out.addChar(0xA37);
        out.add(CharRange(0xA3A, 0xA3B));
        out.addChar(0xA3D);
        out.add(CharRange(0xA43, 0xA46));
        out.add(CharRange(0xA49, 0xA4A));
        out.add(CharRange(0xA4E, 0xA58));
        out.addChar(0xA5D);
        out.add(CharRange(0xA5F, 0xA65));
        out.add(CharRange(0xA75, 0xA80));
        out.addChar(0xA84);
        out.addChar(0xA8C);
        out.addChar(0xA8E);
        out.addChar(0xA92);
        out.addChar(0xAA9);
        out.addChar(0xAB1);
        out.addChar(0xAB4);
        out.add(CharRange(0xABA, 0xABB));
        out.addChar(0xAC6);
        out.addChar(0xACA);
        out.add(CharRange(0xACE, 0xACF));
        out.add(CharRange(0xAD1, 0xADF));
        out.add(CharRange(0xAE1, 0xAE5));
        out.add(CharRange(0xAF0, 0xB00));
        out.addChar(0xB04);
        out.add(CharRange(0xB0D, 0xB0E));
        out.add(CharRange(0xB11, 0xB12));
        out.addChar(0xB29);
        out.addChar(0xB31);
        out.add(CharRange(0xB34, 0xB35));
        out.add(CharRange(0xB3A, 0xB3B));
        out.add(CharRange(0xB44, 0xB46));
        out.add(CharRange(0xB49, 0xB4A));
        out.add(CharRange(0xB4E, 0xB55));
        out.add(CharRange(0xB58, 0xB5B));
        out.addChar(0xB5E);
        out.add(CharRange(0xB62, 0xB65));
        out.add(CharRange(0xB71, 0xB81));
        out.addChar(0xB84);
        out.add(CharRange(0xB8B, 0xB8D));
        out.addChar(0xB91);
        out.add(CharRange(0xB96, 0xB98));
        out.addChar(0xB9B);
        out.addChar(0xB9D);
        out.add(CharRange(0xBA0, 0xBA2));
        out.add(CharRange(0xBA5, 0xBA7));
        out.add(CharRange(0xBAB, 0xBAD));
        out.addChar(0xBB6);
        out.add(CharRange(0xBBA, 0xBBD));
        out.add(CharRange(0xBC3, 0xBC5));
        out.addChar(0xBC9);
        out.add(CharRange(0xBCE, 0xBD6));
        out.add(CharRange(0xBD8, 0xBE6));
        out.add(CharRange(0xBF3, 0xC00));
        out.addChar(0xC04);
        out.addChar(0xC0D);
        out.addChar(0xC11);
        out.addChar(0xC29);
        out.addChar(0xC34);
        out.add(CharRange(0xC3A, 0xC3D));
        out.addChar(0xC45);
        out.addChar(0xC49);
        out.add(CharRange(0xC4E, 0xC54));
        out.add(CharRange(0xC57, 0xC5F));
        out.add(CharRange(0xC62, 0xC65));
        out.add(CharRange(0xC70, 0xC81));
        out.addChar(0xC84);
        out.addChar(0xC8D);
        out.addChar(0xC91);
        out.addChar(0xCA9);
        out.addChar(0xCB4);
        out.add(CharRange(0xCBA, 0xCBD));
        out.addChar(0xCC5);
        out.addChar(0xCC9);
        out.add(CharRange(0xCCE, 0xCD4));
        out.add(CharRange(0xCD7, 0xCDD));
        out.addChar(0xCDF);
        out.add(CharRange(0xCE2, 0xCE5));
        out.add(CharRange(0xCF0, 0xD01));
        out.addChar(0xD04);
        out.addChar(0xD0D);
        out.addChar(0xD11);
        out.addChar(0xD29);
        out.add(CharRange(0xD3A, 0xD3D));
        out.add(CharRange(0xD44, 0xD45));
        out.addChar(0xD49);
        out.add(CharRange(0xD4E, 0xD56));
        out.add(CharRange(0xD58, 0xD5F));
        out.add(CharRange(0xD62, 0xD65));
        out.add(CharRange(0xD70, 0xD81));
        out.addChar(0xD84);
        out.add(CharRange(0xD97, 0xD99));
        out.addChar(0xDB2);
        out.addChar(0xDBC);
        out.add(CharRange(0xDBE, 0xDBF));
        out.add(CharRange(0xDC7, 0xDC9));
        out.add(CharRange(0xDCB, 0xDCE));
        out.addChar(0xDD5);
        out.addChar(0xDD7);
        out.add(CharRange(0xDE0, 0xDF1));
        out.add(CharRange(0xDF5, 0xE00));
        out.add(CharRange(0xE3B, 0xE3E));
        out.add(CharRange(0xE5C, 0xE80));
        out.addChar(0xE83);
        out.add(CharRange(0xE85, 0xE86));
        out.addChar(0xE89);
        out.add(CharRange(0xE8B, 0xE8C));
        out.add(CharRange(0xE8E, 0xE93));
        out.addChar(0xE98);
        out.addChar(0xEA0);
        out.addChar(0xEA4);
        out.addChar(0xEA6);
        out.add(CharRange(0xEA8, 0xEA9));
        out.addChar(0xEAC);
        out.addChar(0xEBA);
        out.add(CharRange(0xEBE, 0xEBF));
        out.addChar(0xEC5);
        out.addChar(0xEC7);
        out.add(CharRange(0xECE, 0xECF));
        out.add(CharRange(0xEDA, 0xEDB));
        out.add(CharRange(0xEDE, 0xEFF));
        out.addChar(0xF48);
        out.add(CharRange(0xF6B, 0xF70));
        out.add(CharRange(0xF8C, 0xF8F));
        out.addChar(0xF98);
        out.addChar(0xFBD);
        out.add(CharRange(0xFCD, 0xFCE));
        out.add(CharRange(0xFD0, 0xFFF));
        out.addChar(0x1022);
        out.addChar(0x1028);
        out.addChar(0x102B);
        out.add(CharRange(0x1033, 0x1035));
        out.add(CharRange(0x103A, 0x103F));
        out.add(CharRange(0x105A, 0x109F));
        out.add(CharRange(0x10C6, 0x10CF));
        out.add(CharRange(0x10F7, 0x10FA));
        out.add(CharRange(0x10FC, 0x10FF));
        out.add(CharRange(0x115A, 0x115E));
        out.add(CharRange(0x11A3, 0x11A7));
        out.add(CharRange(0x11FA, 0x11FF));
        out.addChar(0x1207);
        out.addChar(0x1247);
        out.addChar(0x1249);
        out.add(CharRange(0x124E, 0x124F));
        out.addChar(0x1257);
        out.addChar(0x1259);
        out.add(CharRange(0x125E, 0x125F));
        out.addChar(0x1287);
        out.addChar(0x1289);
        out.add(CharRange(0x128E, 0x128F));
        out.addChar(0x12AF);
        out.addChar(0x12B1);
        out.add(CharRange(0x12B6, 0x12B7));
        out.addChar(0x12BF);
        out.addChar(0x12C1);
        out.add(CharRange(0x12C6, 0x12C7));
        out.addChar(0x12CF);
        out.addChar(0x12D7);
        out.addChar(0x12EF);
        out.addChar(0x130F);
        out.addChar(0x1311);
        out.add(CharRange(0x1316, 0x1317));
        out.addChar(0x131F);
        out.addChar(0x1347);
        out.add(CharRange(0x135B, 0x1360));
        out.add(CharRange(0x137D, 0x139F));
        out.add(CharRange(0x13F5, 0x1400));
        out.add(CharRange(0x1677, 0x167F));
        out.add(CharRange(0x169D, 0x169F));
        out.add(CharRange(0x16F1, 0x177F));
        out.add(CharRange(0x17DD, 0x17DF));
        out.add(CharRange(0x17EA, 0x17FF));
        out.addChar(0x180F);
        out.add(CharRange(0x181A, 0x181F));
        out.add(CharRange(0x1878, 0x187F));
        out.add(CharRange(0x18AA, 0x1DFF));
        out.add(CharRange(0x1E9C, 0x1E9F));
        out.add(CharRange(0x1EFA, 0x1EFF));
        out.add(CharRange(0x1F16, 0x1F17));
        out.add(CharRange(0x1F1E, 0x1F1F));
        out.add(CharRange(0x1F46, 0x1F47));
        out.add(CharRange(0x1F4E, 0x1F4F));
        out.addChar(0x1F58);
        out.addChar(0x1F5A);
        out.addChar(0x1F5C);
        out.addChar(0x1F5E);
        out.add(CharRange(0x1F7E, 0x1F7F));
        out.addChar(0x1FB5);
        out.addChar(0x1FC5);
        out.add(CharRange(0x1FD4, 0x1FD5));
        out.addChar(0x1FDC);
        out.add(CharRange(0x1FF0, 0x1FF1));
        out.addChar(0x1FF5);
        out.addChar(0x1FFF);
        out.addChar(0x2047);
        out.add(CharRange(0x204E, 0x2069));
        out.add(CharRange(0x2071, 0x2073));
        out.add(CharRange(0x208F, 0x209F));
        out.add(CharRange(0x20B0, 0x20CF));
        out.add(CharRange(0x20E4, 0x20FF));
        out.add(CharRange(0x213B, 0x2152));
        out.add(CharRange(0x2184, 0x218F));
        out.add(CharRange(0x21F4, 0x21FF));
        out.add(CharRange(0x22F2, 0x22FF));
        out.addChar(0x237C);
        out.add(CharRange(0x239B, 0x23FF));
        out.add(CharRange(0x2427, 0x243F));
        out.add(CharRange(0x244B, 0x245F));
        out.add(CharRange(0x24EB, 0x24FF));
        out.add(CharRange(0x2596, 0x259F));
        out.add(CharRange(0x25F8, 0x25FF));
        out.add(CharRange(0x2614, 0x2618));
        out.add(CharRange(0x2672, 0x2700));
        out.addChar(0x2705);
        out.add(CharRange(0x270A, 0x270B));
        out.addChar(0x2728);
        out.addChar(0x274C);
        out.addChar(0x274E);
        out.add(CharRange(0x2753, 0x2755));
        out.addChar(0x2757);
        out.add(CharRange(0x275F, 0x2760));
        out.add(CharRange(0x2768, 0x2775));
        out.add(CharRange(0x2795, 0x2797));
        out.addChar(0x27B0);
        out.add(CharRange(0x27BF, 0x27FF));
        out.add(CharRange(0x2900, 0x2E7F));
        out.addChar(0x2E9A);
        out.add(CharRange(0x2EF4, 0x2EFF));
        out.add(CharRange(0x2FD6, 0x2FEF));
        out.add(CharRange(0x2FFC, 0x2FFF));
        out.add(CharRange(0x303B, 0x303D));
        out.addChar(0x3040);
        out.add(CharRange(0x3095, 0x3098));
        out.add(CharRange(0x309F, 0x30A0));
        out.add(CharRange(0x30FF, 0x3104));
        out.add(CharRange(0x312D, 0x3130));
        out.addChar(0x318F);
        out.add(CharRange(0x31B8, 0x31FF));
        out.add(CharRange(0x321D, 0x321F));
        out.add(CharRange(0x3244, 0x325F));
        out.add(CharRange(0x327C, 0x327E));
        out.add(CharRange(0x32B1, 0x32BF));
        out.add(CharRange(0x32CC, 0x32CF));
        out.addChar(0x32FF);
        out.add(CharRange(0x3377, 0x337A));
        out.add(CharRange(0x33DE, 0x33DF));
        out.addChar(0x33FF);
        out.add(CharRange(0x4DB6, 0x4DFF));
        out.add(CharRange(0x9FA6, 0x9FFF));
        out.add(CharRange(0xA48D, 0xA48F));
        out.add(CharRange(0xA4A2, 0xA4A3));
        out.addChar(0xA4B4);
        out.addChar(0xA4C1);
        out.addChar(0xA4C5);
        out.add(CharRange(0xA4C7, 0xABFF));
        out.add(CharRange(0xD7A4, 0xD7FF));
        out.add(CharRange(0xFA2E, 0xFAFF));
        out.add(CharRange(0xFB07, 0xFB12));
        out.add(CharRange(0xFB18, 0xFB1C));
        out.addChar(0xFB37);
        out.addChar(0xFB3D);
        out.addChar(0xFB3F);
        out.addChar(0xFB42);
        out.addChar(0xFB45);
        out.add(CharRange(0xFBB2, 0xFBD2));
        out.add(CharRange(0xFD40, 0xFD4F));
        out.add(CharRange(0xFD90, 0xFD91));
        out.add(CharRange(0xFDC8, 0xFDEF));
        out.add(CharRange(0xFDFC, 0xFE1F));
        out.add(CharRange(0xFE24, 0xFE2F));
        out.add(CharRange(0xFE45, 0xFE48));
        out.addChar(0xFE53);
        out.addChar(0xFE67);
        out.add(CharRange(0xFE6C, 0xFE6F));
        out.addChar(0xFE73);
        out.addChar(0xFE75);
        out.add(CharRange(0xFEFD, 0xFEFE));
        out.addChar(0xFF00);
        out.add(CharRange(0xFF5F, 0xFF60));
        out.add(CharRange(0xFFBF, 0xFFC1));
        out.add(CharRange(0xFFC8, 0xFFC9));
        out.add(CharRange(0xFFD0, 0xFFD1));
        out.add(CharRange(0xFFD8, 0xFFD9));
        out.add(CharRange(0xFFDD, 0xFFDF));
        out.addChar(0xFFE7);
        out.add(CharRange(0xFFEF, 0xFFF8));
        out.add(CharRange(0xFFFE, 0xFFFF));
        break;
    default:
        break;
    }
}

// 801F6A48
int RegNfa::lookupClassName(const u16 *name, int len, bool shortForm) {
    if (!shortForm) {
        if (equalsName(name, (const u16 *)L"Lower", len)) {
            return CLASS_LOWER;
        }
        if (equalsName(name, (const u16 *)L"Upper", len)) {
            return CLASS_UPPER;
        }
        if (equalsName(name, (const u16 *)L"ASCII", len)) {
            return CLASS_ASCII;
        }
        if (equalsName(name, (const u16 *)L"Alpha", len)) {
            return CLASS_ALPHA;
        }
        if (equalsName(name, (const u16 *)L"Digit", len)) {
            return CLASS_DIGIT;
        }
        if (equalsName(name, (const u16 *)L"Alnum", len)) {
            return CLASS_ALNUM;
        }
        if (equalsName(name, (const u16 *)L"Punct", len)) {
            return CLASS_PUNCT;
        }
        if (equalsName(name, (const u16 *)L"Graph", len)) {
            return CLASS_GRAPH;
        }
        if (equalsName(name, (const u16 *)L"Print", len)) {
            return CLASS_PRINT;
        }
        if (equalsName(name, (const u16 *)L"Blank", len)) {
            return CLASS_BLANK;
        }
        if (equalsName(name, (const u16 *)L"Cntrl", len)) {
            return CLASS_CNTRL;
        }
        if (equalsName(name, (const u16 *)L"XDigit", len)) {
            return CLASS_XDIGIT;
        }
        if (equalsName(name, (const u16 *)L"Space", len)) {
            return CLASS_SPACE;
        }
        if (equalsName(name, (const u16 *)L"InBasicLatin", len)) {
            return CLASS_ASCII;
        }
        if (equalsName(name, (const u16 *)L"InLatin-1Supplement", len)) {
            return CLASS_IN_LATIN_1_SUPPLEMENT;
        }
        if (equalsName(name, (const u16 *)L"InLatinExtended-A", len)) {
            return CLASS_IN_LATIN_EXTENDED_A;
        }
        if (equalsName(name, (const u16 *)L"InIPAExtensions", len)) {
            return CLASS_IN_IPA_EXTENSIONS;
        }
        if (equalsName(name, (const u16 *)L"InSpacingModifierLetters", len)) {
            return CLASS_IN_SPACING_MODIFIER_LETTERS;
        }
        if (equalsName(name, (const u16 *)L"InCombiningDiacriticalMarks", len)) {
            return CLASS_IN_COMBINING_DIACRITICAL_MARKS;
        }
        if (equalsName(name, (const u16 *)L"InGreek", len)) {
            return CLASS_IN_GREEK;
        }
        if (equalsName(name, (const u16 *)L"InCyrillic", len)) {
            return CLASS_IN_CYRILLIC;
        }
        if (equalsName(name, (const u16 *)L"InArmenian", len)) {
            return CLASS_IN_ARMENIAN;
        }
        if (equalsName(name, (const u16 *)L"InHebrew", len)) {
            return CLASS_IN_HEBREW;
        }
        if (equalsName(name, (const u16 *)L"InArabic", len)) {
            return CLASS_IN_ARABIC;
        }
        if (equalsName(name, (const u16 *)L"InSyriac", len)) {
            return CLASS_IN_SYRIAC;
        }
        if (equalsName(name, (const u16 *)L"InThaana", len)) {
            return CLASS_IN_THAANA;
        }
        if (equalsName(name, (const u16 *)L"InDevanagari", len)) {
            return CLASS_IN_DEVANAGARI;
        }
        if (equalsName(name, (const u16 *)L"InBengali", len)) {
            return CLASS_IN_BENGALI;
        }
        if (equalsName(name, (const u16 *)L"InGurmukhi", len)) {
            return CLASS_IN_GURMUKHI;
        }
        if (equalsName(name, (const u16 *)L"InGujarati", len)) {
            return CLASS_IN_GUJARATI;
        }
        if (equalsName(name, (const u16 *)L"InOriya", len)) {
            return CLASS_IN_ORIYA;
        }
        if (equalsName(name, (const u16 *)L"InTamil", len)) {
            return CLASS_IN_TAMIL;
        }
        if (equalsName(name, (const u16 *)L"InTelugu", len)) {
            return CLASS_IN_TELUGU;
        }
        if (equalsName(name, (const u16 *)L"InKannada", len)) {
            return CLASS_IN_KANNADA;
        }
        if (equalsName(name, (const u16 *)L"InMalayalam", len)) {
            return CLASS_IN_MALAYALAM;
        }
        if (equalsName(name, (const u16 *)L"InSinhala", len)) {
            return CLASS_IN_SINHALA;
        }
        if (equalsName(name, (const u16 *)L"InThai", len)) {
            return CLASS_IN_THAI;
        }
        if (equalsName(name, (const u16 *)L"InLao", len)) {
            return CLASS_IN_LAO;
        }
        if (equalsName(name, (const u16 *)L"InTibetan", len)) {
            return CLASS_IN_TIBETAN;
        }
        if (equalsName(name, (const u16 *)L"InMyanmar", len)) {
            return CLASS_IN_MYANMAR;
        }
        if (equalsName(name, (const u16 *)L"InGeorgian", len)) {
            return CLASS_IN_GEORGIAN;
        }
        if (equalsName(name, (const u16 *)L"InHangulJamo", len)) {
            return CLASS_IN_HANGUL_JAMO;
        }
        if (equalsName(name, (const u16 *)L"InEthiopic", len)) {
            return CLASS_IN_ETHIOPIC;
        }
        if (equalsName(name, (const u16 *)L"InCherokee", len)) {
            return CLASS_IN_CHEROKEE;
        }
        if (equalsName(name, (const u16 *)L"InUnifiedCanadianAboriginalSyllabics", len)) {
            return CLASS_IN_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS;
        }
        if (equalsName(name, (const u16 *)L"InOgham", len)) {
            return CLASS_IN_OGHAM;
        }
        if (equalsName(name, (const u16 *)L"InRunic", len)) {
            return CLASS_IN_RUNIC;
        }
        if (equalsName(name, (const u16 *)L"InKhmer", len)) {
            return CLASS_IN_KHMER;
        }
        if (equalsName(name, (const u16 *)L"InMongolian", len)) {
            return CLASS_IN_MONGOLIAN;
        }
        if (equalsName(name, (const u16 *)L"InLatinExtendedAdditional", len)) {
            return CLASS_IN_LATIN_EXTENDED_ADDITIONAL;
        }
        if (equalsName(name, (const u16 *)L"InGreekExtended", len)) {
            return CLASS_IN_GREEK_EXTENDED;
        }
        if (equalsName(name, (const u16 *)L"InGeneralPunctuation", len)) {
            return CLASS_IN_GENERAL_PUNCTUATION;
        }
        if (equalsName(name, (const u16 *)L"InSuperscriptsandSubscripts", len)) {
            return CLASS_IN_SUPERSCRIPTS_AND_SUBSCRIPTS;
        }
        if (equalsName(name, (const u16 *)L"InCurrencySymbols", len)) {
            return CLASS_IN_CURRENCY_SYMBOLS;
        }
        if (equalsName(name, (const u16 *)L"InLetterlikeSymbols", len)) {
            return CLASS_IN_LETTERLIKE_SYMBOLS;
        }
        if (equalsName(name, (const u16 *)L"InNumberForms", len)) {
            return CLASS_IN_NUMBER_FORMS;
        }
        if (equalsName(name, (const u16 *)L"InArrows", len)) {
            return CLASS_IN_ARROWS;
        }
        if (equalsName(name, (const u16 *)L"InMathematicalOperators", len)) {
            return CLASS_IN_MATHEMATICAL_OPERATORS;
        }
        if (equalsName(name, (const u16 *)L"InMiscellaneousTechnical", len)) {
            return CLASS_IN_MISCELLANEOUS_TECHNICAL;
        }
        if (equalsName(name, (const u16 *)L"InControlPictures", len)) {
            return CLASS_IN_CONTROL_PICTURES;
        }
        if (equalsName(name, (const u16 *)L"InOpticalCharacterRecognition", len)) {
            return CLASS_IN_OPTICAL_CHARACTER_RECOGNITION;
        }
        if (equalsName(name, (const u16 *)L"InEnclosedAlphanumerics", len)) {
            return CLASS_IN_ENCLOSED_ALPHANUMERICS;
        }
        if (equalsName(name, (const u16 *)L"InBoxDrawing", len)) {
            return CLASS_IN_BOX_DRAWING;
        }
        if (equalsName(name, (const u16 *)L"InBlockElements", len)) {
            return CLASS_IN_BLOCK_ELEMENTS;
        }
        if (equalsName(name, (const u16 *)L"InGeometricShapes", len)) {
            return CLASS_IN_GEOMETRIC_SHAPES;
        }
        if (equalsName(name, (const u16 *)L"InMiscellaneousSymbols", len)) {
            return CLASS_IN_MISCELLANEOUS_SYMBOLS;
        }
        if (equalsName(name, (const u16 *)L"InDingbats", len)) {
            return CLASS_IN_DINGBATS;
        }
        if (equalsName(name, (const u16 *)L"InBraillePatterns", len)) {
            return CLASS_IN_BRAILLE_PATTERNS;
        }
        if (equalsName(name, (const u16 *)L"InCJKRadicalsSupplement", len)) {
            return CLASS_IN_CJK_RADICALS_SUPPLEMENT;
        }
        if (equalsName(name, (const u16 *)L"InKangxiRadicals", len)) {
            return CLASS_IN_KANGXI_RADICALS;
        }
        if (equalsName(name, (const u16 *)L"InIdeographicDescriptionCharacters", len)) {
            return CLASS_IN_IDEOGRAPHIC_DESCRIPTION_CHARACTERS;
        }
        if (equalsName(name, (const u16 *)L"InCJKSymbolsandPunctuation", len)) {
            return CLASS_IN_CJK_SYMBOLS_AND_PUNCTUATION;
        }
        if (equalsName(name, (const u16 *)L"InHiragana", len)) {
            return CLASS_IN_HIRAGANA;
        }
        if (equalsName(name, (const u16 *)L"InKatakana", len)) {
            return CLASS_IN_KATAKANA;
        }
        if (equalsName(name, (const u16 *)L"InBopomofo", len)) {
            return CLASS_IN_BOPOMOFO;
        }
        if (equalsName(name, (const u16 *)L"InHangulCompatibilityJamo", len)) {
            return CLASS_IN_HANGUL_COMPATIBILITY_JAMO;
        }
        if (equalsName(name, (const u16 *)L"InKanbun", len)) {
            return CLASS_IN_KANBUN;
        }
        if (equalsName(name, (const u16 *)L"InBopomofoExtended", len)) {
            return CLASS_IN_BOPOMOFO_EXTENDED;
        }
        if (equalsName(name, (const u16 *)L"InEnclosedCJKLettersandMonths", len)) {
            return CLASS_IN_ENCLOSED_CJK_LETTERS_AND_MONTHS;
        }
        if (equalsName(name, (const u16 *)L"InCJKCompatibility", len)) {
            return CLASS_IN_CJK_COMPATIBILITY;
        }
        if (equalsName(name, (const u16 *)L"InCJKUnifiedIdeographsExtensionA", len)) {
            return CLASS_IN_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A;
        }
        if (equalsName(name, (const u16 *)L"InCJKUnifiedIdeographs", len)) {
            return CLASS_IN_CJK_UNIFIED_IDEOGRAPHS;
        }
        if (equalsName(name, (const u16 *)L"InYiSyllables", len)) {
            return CLASS_IN_YI_SYLLABLES;
        }
        if (equalsName(name, (const u16 *)L"InYiRadicals", len)) {
            return CLASS_IN_YI_RADICALS;
        }
        if (equalsName(name, (const u16 *)L"InHangulSyllables", len)) {
            return CLASS_IN_HANGUL_SYLLABLES;
        }
        if (equalsName(name, (const u16 *)L"InHighSurrogates", len)) {
            return CLASS_IN_HIGH_SURROGATES;
        }
        if (equalsName(name, (const u16 *)L"InHighPrivateUseSurrogates", len)) {
            return CLASS_IN_HIGH_PRIVATE_USE_SURROGATES;
        }
        if (equalsName(name, (const u16 *)L"InLowSurrogates", len)) {
            return CLASS_IN_LOW_SURROGATES;
        }
        if (equalsName(name, (const u16 *)L"InCJKCompatibilityIdeographs", len)) {
            return CLASS_IN_CJK_COMPATIBILITY_IDEOGRAPHS;
        }
        if (equalsName(name, (const u16 *)L"InAlphabeticPresentationForms", len)) {
            return CLASS_IN_ALPHABETIC_PRESENTATION_FORMS;
        }
        if (equalsName(name, (const u16 *)L"InArabicPresentationForms-A", len)) {
            return CLASS_IN_ARABIC_PRESENTATION_FORMS_A;
        }
        if (equalsName(name, (const u16 *)L"InCombiningHalfMarks", len)) {
            return CLASS_IN_COMBINING_HALF_MARKS;
        }
        if (equalsName(name, (const u16 *)L"InCJKCompatibilityForms", len)) {
            return CLASS_IN_CJK_COMPATIBILITY_FORMS;
        }
        if (equalsName(name, (const u16 *)L"InSmallFormVariants", len)) {
            return CLASS_IN_SMALL_FORM_VARIANTS;
        }
        if (equalsName(name, (const u16 *)L"InHalfwidthandFullwidthForms", len)) {
            return CLASS_IN_HALFWIDTH_AND_FULLWIDTH_FORMS;
        }
        if (equalsName(name, (const u16 *)L"InSpecials", len)) {
            return CLASS_IN_SPECIALS;
        }
        if (equalsName(name, (const u16 *)L"IsL", len)) {
            return CLASS_L;
        }
        if (equalsName(name, (const u16 *)L"IsLl", len)) {
            return CLASS_LL;
        }
        if (equalsName(name, (const u16 *)L"IsLu", len)) {
            return CLASS_LU;
        }
        if (equalsName(name, (const u16 *)L"IsLt", len)) {
            return CLASS_LT;
        }
        if (equalsName(name, (const u16 *)L"IsLm", len)) {
            return CLASS_LM;
        }
        if (equalsName(name, (const u16 *)L"IsLo", len)) {
            return CLASS_LO;
        }
        if (equalsName(name, (const u16 *)L"IsM", len)) {
            return CLASS_M;
        }
        if (equalsName(name, (const u16 *)L"IsMn", len)) {
            return CLASS_MN;
        }
        if (equalsName(name, (const u16 *)L"IsMc", len)) {
            return CLASS_MC;
        }
        if (equalsName(name, (const u16 *)L"IsMe", len)) {
            return CLASS_ME;
        }
        if (equalsName(name, (const u16 *)L"IsZ", len)) {
            return CLASS_Z;
        }
        if (equalsName(name, (const u16 *)L"IsZs", len)) {
            return CLASS_ZS;
        }
        if (equalsName(name, (const u16 *)L"IsZl", len)) {
            return CLASS_ZL;
        }
        if (equalsName(name, (const u16 *)L"IsZp", len)) {
            return CLASS_ZP;
        }
        if (equalsName(name, (const u16 *)L"IsS", len)) {
            return CLASS_S;
        }
        if (equalsName(name, (const u16 *)L"IsSm", len)) {
            return CLASS_SM;
        }
        if (equalsName(name, (const u16 *)L"IsSc", len)) {
            return CLASS_SC;
        }
        if (equalsName(name, (const u16 *)L"IsSk", len)) {
            return CLASS_SK;
        }
        if (equalsName(name, (const u16 *)L"IsSo", len)) {
            return CLASS_SO;
        }
        if (equalsName(name, (const u16 *)L"IsN", len)) {
            return CLASS_N;
        }
        if (equalsName(name, (const u16 *)L"IsNd", len)) {
            return CLASS_ND;
        }
        if (equalsName(name, (const u16 *)L"IsNl", len)) {
            return CLASS_NL;
        }
        if (equalsName(name, (const u16 *)L"IsNo", len)) {
            return CLASS_NO;
        }
        if (equalsName(name, (const u16 *)L"IsP", len)) {
            return CLASS_P;
        }
        if (equalsName(name, (const u16 *)L"IsPd", len)) {
            return CLASS_PD;
        }
        if (equalsName(name, (const u16 *)L"IsPs", len)) {
            return CLASS_PS;
        }
        if (equalsName(name, (const u16 *)L"IsPe", len)) {
            return CLASS_PE;
        }
        if (equalsName(name, (const u16 *)L"IsPc", len)) {
            return CLASS_PC;
        }
        if (equalsName(name, (const u16 *)L"IsPo", len)) {
            return CLASS_PO;
        }
        if (equalsName(name, (const u16 *)L"IsC", len)) {
            return CLASS_C;
        }
        if (equalsName(name, (const u16 *)L"IsCc", len)) {
            return CLASS_CC;
        }
        if (equalsName(name, (const u16 *)L"IsCf", len)) {
            return CLASS_CF;
        }
        if (equalsName(name, (const u16 *)L"IsCo", len)) {
            return CLASS_CO;
        }
        if (equalsName(name, (const u16 *)L"IsCn", len)) {
            return CLASS_CN;
        }
        if (equalsName(name, (const u16 *)L"IsCs", len)) {
            return CLASS_CS;
        }
    }
    if (equalsName(name, (const u16 *)L"L", len)) {
        return CLASS_L;
    }
    if (equalsName(name, (const u16 *)L"Ll", len)) {
        return CLASS_LL;
    }
    if (equalsName(name, (const u16 *)L"Lu", len)) {
        return CLASS_LU;
    }
    if (equalsName(name, (const u16 *)L"Lt", len)) {
        return CLASS_LT;
    }
    if (equalsName(name, (const u16 *)L"Lm", len)) {
        return CLASS_LM;
    }
    if (equalsName(name, (const u16 *)L"Lo", len)) {
        return CLASS_LO;
    }
    if (equalsName(name, (const u16 *)L"M", len)) {
        return CLASS_M;
    }
    if (equalsName(name, (const u16 *)L"Mn", len)) {
        return CLASS_MN;
    }
    if (equalsName(name, (const u16 *)L"Mc", len)) {
        return CLASS_MC;
    }
    if (equalsName(name, (const u16 *)L"Me", len)) {
        return CLASS_ME;
    }
    if (equalsName(name, (const u16 *)L"Z", len)) {
        return CLASS_Z;
    }
    if (equalsName(name, (const u16 *)L"Zs", len)) {
        return CLASS_ZS;
    }
    if (equalsName(name, (const u16 *)L"Zl", len)) {
        return CLASS_ZL;
    }
    if (equalsName(name, (const u16 *)L"Zp", len)) {
        return CLASS_ZP;
    }
    if (equalsName(name, (const u16 *)L"S", len)) {
        return CLASS_S;
    }
    if (equalsName(name, (const u16 *)L"Sm", len)) {
        return CLASS_SM;
    }
    if (equalsName(name, (const u16 *)L"Sc", len)) {
        return CLASS_SC;
    }
    if (equalsName(name, (const u16 *)L"Sk", len)) {
        return CLASS_SK;
    }
    if (equalsName(name, (const u16 *)L"So", len)) {
        return CLASS_SO;
    }
    if (equalsName(name, (const u16 *)L"N", len)) {
        return CLASS_N;
    }
    if (equalsName(name, (const u16 *)L"Nd", len)) {
        return CLASS_ND;
    }
    if (equalsName(name, (const u16 *)L"Nl", len)) {
        return CLASS_NL;
    }
    if (equalsName(name, (const u16 *)L"No", len)) {
        return CLASS_NO;
    }
    if (equalsName(name, (const u16 *)L"P", len)) {
        return CLASS_P;
    }
    if (equalsName(name, (const u16 *)L"Pd", len)) {
        return CLASS_PD;
    }
    if (equalsName(name, (const u16 *)L"Ps", len)) {
        return CLASS_PS;
    }
    if (equalsName(name, (const u16 *)L"Pe", len)) {
        return CLASS_PE;
    }
    if (equalsName(name, (const u16 *)L"Pc", len)) {
        return CLASS_PC;
    }
    if (equalsName(name, (const u16 *)L"Po", len)) {
        return CLASS_PO;
    }
    if (equalsName(name, (const u16 *)L"C", len)) {
        return CLASS_C;
    }
    if (equalsName(name, (const u16 *)L"Cc", len)) {
        return CLASS_CC;
    }
    if (equalsName(name, (const u16 *)L"Cf", len)) {
        return CLASS_CF;
    }
    if (equalsName(name, (const u16 *)L"Co", len)) {
        return CLASS_CO;
    }
    if (equalsName(name, (const u16 *)L"Cn", len)) {
        return CLASS_CN;
    }
    return equalsName(name, (const u16 *)L"Cs", len) ? CLASS_CS : CLASS_NONE;
}

// 801F7F50
RegToken::RegToken() : mChar(0), mMin(0), mMax(0), mType(0) {}

// 801F7F68
RegToken::RegToken(int type) {
    mType = type;
    mChar = 0;
    mMin = 0;
    mMax = 0;
}

// 801F7F80
RegToken &RegToken::operator=(const RegToken &other) {
    mChar = other.mChar;
    mMin = other.mMin;
    mMax = other.mMax;
    mType = other.mType;
    return *this;
}

// 801F7FA4
u16 RegToken::getChar() const {
    return mChar;
}

// 801F7FAC
int RegToken::getMin() const {
    return mMin;
}

// 801F7FB4
int RegToken::getMax() const {
    return mMax;
}

// 801F7FBC
int RegToken::getClassId() const {
    return mMin;
}

} // namespace RegEad
