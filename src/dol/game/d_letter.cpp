// d_letter: letter text building and the keyboard key sound effects. .text 800CB0CC..800CCCD8.
// See include/game/game/d_letter.hpp.
#include <game/game/d_letter.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <cstdio>
#include <cstring>

// Key SE tables, per SE set (getKeySeSet). Digits: three rows of ten (playDigitKeySe); letters: A..Z.
// 80472E50
static const int sDigitSeUs[] = {
    0x1E8F, 0x1E90, 0x1E91, 0x1E92, 0x1E93, 0x1E94, 0x1E95, 0x1E96, 0x1E97, 0x1E98,
    0x1E51, 0x1E52, 0x1E53, 0x1E54, 0x1E55, 0x1E56, 0x1E57, 0x1E58, 0x1E59, 0x1E5A,
    0x1E99, 0x1E9A, 0x1E9B, 0x1E9C, 0x1E9D, 0x1E9E, 0x1E9F, 0x1EA0, 0x1EA1, 0x1EA2,
};

static const int sUpperSeUs[] = {
    0x1E5B, 0x1E5C, 0x1E5D, 0x1E5E, 0x1E5F, 0x1E60, 0x1E61, 0x1E62, 0x1E63, 0x1E64,
    0x1E65, 0x1E66, 0x1E67, 0x1E68, 0x1E69, 0x1E6A, 0x1E6B, 0x1E6C, 0x1E6D, 0x1E6E,
    0x1E6F, 0x1E70, 0x1E71, 0x1E72, 0x1E73, 0x1E74,
};

static const int sLowerSeUs[] = {
    0x1E75, 0x1E76, 0x1E77, 0x1E78, 0x1E79, 0x1E7A, 0x1E7B, 0x1E7C, 0x1E7D, 0x1E7E,
    0x1E7F, 0x1E80, 0x1E81, 0x1E82, 0x1E83, 0x1E84, 0x1E85, 0x1E86, 0x1E87, 0x1E88,
    0x1E89, 0x1E8A, 0x1E8B, 0x1E8C, 0x1E8D, 0x1E8E,
};

static const int sAltSeUs[] = {
    0x1EA3, 0x1EA4, 0x1EA5, 0x1EA6, 0x1EA7, 0x1EA8, 0x1EA9, 0x1EAA, 0x1EAB, 0x1EAC,
    0x1EAD, 0x1EAE, 0x1EAF, 0x1EB0, 0x1EB1, 0x1EB2, 0x1EB3, 0x1EB4, 0x1EB5, 0x1EB6,
    0x1EB7, 0x1EB8, 0x1EB9, 0x1EBA, 0x1EBB, 0x1EBC,
};

// 80473000
static const int sDigitSeUsF[] = {
    0x1EFB, 0x1EFC, 0x1EFD, 0x1EFE, 0x1EFF, 0x1F00, 0x1F01, 0x1F02, 0x1F03, 0x1F04,
    0x1EBD, 0x1EBE, 0x1EBF, 0x1EC0, 0x1EC1, 0x1EC2, 0x1EC3, 0x1EC4, 0x1EC5, 0x1EC6,
    0x1F05, 0x1F06, 0x1F07, 0x1F08, 0x1F09, 0x1F0A, 0x1F0B, 0x1F0C, 0x1F0D, 0x1F0E,
};

static const int sUpperSeUsF[] = {
    0x1EC7, 0x1EC8, 0x1EC9, 0x1ECA, 0x1ECB, 0x1ECC, 0x1ECD, 0x1ECE, 0x1ECF, 0x1ED0,
    0x1ED1, 0x1ED2, 0x1ED3, 0x1ED4, 0x1ED5, 0x1ED6, 0x1ED7, 0x1ED8, 0x1ED9, 0x1EDA,
    0x1EDB, 0x1EDC, 0x1EDD, 0x1EDE, 0x1EDF, 0x1EE0,
};

static const int sAltSeUsF[] = {
    0x1F0F, 0x1F10, 0x1F11, 0x1F12, 0x1F13, 0x1F14, 0x1F15, 0x1F16, 0x1F17, 0x1F18,
    0x1F19, 0x1F1A, 0x1F1B, 0x1F1C, 0x1F1D, 0x1F1E, 0x1F1F, 0x1F20, 0x1F21, 0x1F22,
    0x1F23, 0x1F24, 0x1F25, 0x1F26, 0x1F27, 0x1F28,
};

static const int sLowerSeUsF[] = {
    0x1EE1, 0x1EE2, 0x1EE3, 0x1EE4, 0x1EE5, 0x1EE6, 0x1EE7, 0x1EE8, 0x1EE9, 0x1EEA,
    0x1EEB, 0x1EEC, 0x1EED, 0x1EEE, 0x1EEF, 0x1EF0, 0x1EF1, 0x1EF2, 0x1EF3, 0x1EF4,
    0x1EF5, 0x1EF6, 0x1EF7, 0x1EF8, 0x1EF9, 0x1EFA,
};

// 804731B0
static const int sDigitSeQc[] = {
    0x1F29, 0x1F2A, 0x1F2B, 0x1F2C, 0x1F2D, 0x1F2E, 0x1F2F, 0x1F30, 0x1F31, 0x1F32,
    0x1F29, 0x1F2A, 0x1F2B, 0x1F2C, 0x1F2D, 0x1F2E, 0x1F2F, 0x1F30, 0x1F31, 0x1F32,
    0x1F71, 0x1F72, 0x1F73, 0x1F74, 0x1F75, 0x1F76, 0x1F77, 0x1F78, 0x1F79, 0x1F7A,
};

static const int sUpperSeQc[] = {
    0x1F34, 0x1F36, 0x1F38, 0x1F3A, 0x1F3C, 0x1F3E, 0x1F40, 0x1F42, 0x1F44, 0x1F46,
    0x1F48, 0x1F4A, 0x1F4C, 0x1F4E, 0x1F50, 0x1F52, 0x1F54, 0x1F56, 0x1F58, 0x1F5A,
    0x1F5C, 0x1F5E, 0x1F60, 0x1F62, 0x1F64, 0x1F66,
};

static const int sAltSeQc[] = {
    0x1F7B, 0x1F7C, 0x1F7D, 0x1F7E, 0x1F7F, 0x1F80, 0x1F81, 0x1F82, 0x1F83, 0x1F84,
    0x1F85, 0x1F86, 0x1F87, 0x1F88, 0x1F89, 0x1F8A, 0x1F8B, 0x1F8C, 0x1F8D, 0x1F8E,
    0x1F8F, 0x1F90, 0x1F91, 0x1F92, 0x1F93, 0x1F94,
};

static const int sLowerSeQc[] = {
    0x1F33, 0x1F35, 0x1F37, 0x1F39, 0x1F3B, 0x1F3D, 0x1F3F, 0x1F41, 0x1F43, 0x1F45,
    0x1F47, 0x1F49, 0x1F4B, 0x1F4D, 0x1F4F, 0x1F51, 0x1F53, 0x1F55, 0x1F57, 0x1F59,
    0x1F5B, 0x1F5D, 0x1F5F, 0x1F61, 0x1F63, 0x1F65,
};

// 80473360
static const int sDigitSeQcF[] = {
    0x1F95, 0x1F96, 0x1F97, 0x1F98, 0x1F99, 0x1F9A, 0x1F9B, 0x1F9C, 0x1F9D, 0x1F9E,
    0x1F95, 0x1F96, 0x1F97, 0x1F98, 0x1F99, 0x1F9A, 0x1F9B, 0x1F9C, 0x1F9D, 0x1F9E,
    0x1FDD, 0x1FDE, 0x1FDF, 0x1FE0, 0x1FE1, 0x1FE2, 0x1FE3, 0x1FE4, 0x1FE5, 0x1FE6,
};

static const int sUpperSeQcF[] = {
    0x1F9F, 0x1FA0, 0x1FA1, 0x1FA2, 0x1FA3, 0x1FA4, 0x1FA5, 0x1FA6, 0x1FA7, 0x1FA8,
    0x1FA9, 0x1FAA, 0x1FAB, 0x1FAC, 0x1FAD, 0x1FAE, 0x1FAF, 0x1FB0, 0x1FB1, 0x1FB2,
    0x1FB3, 0x1FB4, 0x1FB5, 0x1FB6, 0x1FB7, 0x1FB8,
};

static const int sAltSeQcF[] = {
    0x1FE7, 0x1FE8, 0x1FE9, 0x1FEA, 0x1FEB, 0x1FEC, 0x1FED, 0x1FEE, 0x1FEF, 0x1FF0,
    0x1FF1, 0x1FF2, 0x1FF3, 0x1FF4, 0x1FF5, 0x1FF6, 0x1FF7, 0x1FF8, 0x1FF9, 0x1FFA,
    0x1FFB, 0x1FFC, 0x1FFD, 0x1FFE, 0x1FFF, 0x2000,
};

static const int sLowerSeQcF[] = {
    0x1FB9, 0x1FBA, 0x1FBB, 0x1FBC, 0x1FBD, 0x1FBE, 0x1FBF, 0x1FC0, 0x1FC1, 0x1FC2,
    0x1FC3, 0x1FC4, 0x1FC5, 0x1FC6, 0x1FC7, 0x1FC8, 0x1FC9, 0x1FCA, 0x1FCB, 0x1FCC,
    0x1FCD, 0x1FCE, 0x1FCF, 0x1FD0, 0x1FD1, 0x1FD2,
};

// 80473510
static const int sDigitSeMx[] = {
    0x2205, 0x2206, 0x2207, 0x2208, 0x2209, 0x220A, 0x220B, 0x220C, 0x220D, 0x220E,
    0x21C7, 0x21C8, 0x21C9, 0x21CA, 0x21CB, 0x21CC, 0x21CD, 0x21CE, 0x21CF, 0x21D0,
    0x220F, 0x2210, 0x2211, 0x2212, 0x2213, 0x2214, 0x2215, 0x2216, 0x2217, 0x2218,
};

static const int sUpperSeMx[] = {
    0x21D1, 0x21D2, 0x21D3, 0x21D4, 0x21D5, 0x21D6, 0x21D7, 0x21D8, 0x21D9, 0x21DA,
    0x21DB, 0x21DC, 0x21DD, 0x21DE, 0x21DF, 0x21E0, 0x21E1, 0x21E2, 0x21E3, 0x21E4,
    0x21E5, 0x21E6, 0x21E7, 0x21E8, 0x21E9, 0x21EA,
};

static const int sAltSeMx[] = {
    0x2219, 0x221A, 0x221B, 0x221C, 0x221D, 0x221E, 0x221F, 0x2220, 0x2221, 0x2222,
    0x2223, 0x2224, 0x2225, 0x2226, 0x2227, 0x2228, 0x2229, 0x222A, 0x222B, 0x222C,
    0x222D, 0x222E, 0x222F, 0x2230, 0x2231, 0x2232,
};

static const int sLowerSeMx[] = {
    0x21EB, 0x21EC, 0x21ED, 0x21EE, 0x21EF, 0x21F0, 0x21F1, 0x21F2, 0x21F3, 0x21F4,
    0x21F5, 0x21F6, 0x21F7, 0x21F8, 0x21F9, 0x21FA, 0x21FB, 0x21FC, 0x21FD, 0x21FE,
    0x21FF, 0x2200, 0x2201, 0x2202, 0x2203, 0x2204,
};

// 804736C0
static const int sDigitSeMxF[] = {
    0x2271, 0x2272, 0x2273, 0x2274, 0x2275, 0x2276, 0x2277, 0x2278, 0x2279, 0x227A,
    0x2233, 0x2234, 0x2235, 0x2236, 0x2237, 0x2238, 0x2239, 0x223A, 0x223B, 0x223C,
    0x227B, 0x227C, 0x227D, 0x227E, 0x227F, 0x2280, 0x2281, 0x2282, 0x2283, 0x2284,
};

static const int sUpperSeMxF[] = {
    0x223D, 0x223E, 0x223F, 0x2240, 0x2241, 0x2242, 0x2243, 0x2244, 0x2245, 0x2246,
    0x2247, 0x2248, 0x2249, 0x224A, 0x224B, 0x224C, 0x224D, 0x224E, 0x224F, 0x2250,
    0x2251, 0x2252, 0x2253, 0x2254, 0x2255, 0x2256,
};

static const int sAltSeMxF[] = {
    0x2285, 0x2286, 0x2287, 0x2288, 0x2289, 0x228A, 0x228B, 0x228C, 0x228D, 0x228E,
    0x228F, 0x2290, 0x2291, 0x2292, 0x2293, 0x2294, 0x2295, 0x2296, 0x2297, 0x2298,
    0x2299, 0x229A, 0x229B, 0x229C, 0x229D, 0x229E,
};

static const int sLowerSeMxF[] = {
    0x2257, 0x2258, 0x2259, 0x225A, 0x225B, 0x225C, 0x225D, 0x225E, 0x225F, 0x2260,
    0x2261, 0x2262, 0x2263, 0x2264, 0x2265, 0x2266, 0x2267, 0x2268, 0x2269, 0x226A,
    0x226B, 0x226C, 0x226D, 0x226E, 0x226F, 0x2270,
};

static const int *sDigitSe[] = {sDigitSeUs, sDigitSeQc, sDigitSeMx, sDigitSeUsF, sDigitSeQcF, sDigitSeMxF};
static const int *sUpperSe[] = {sUpperSeUs, sUpperSeQc, sUpperSeMx, sUpperSeUsF, sUpperSeQcF, sUpperSeMxF};
static const int *sAltSe[] = {sAltSeUs, sAltSeQc, sAltSeMx, sAltSeUsF, sAltSeQcF, sAltSeMxF};
static const int *sLowerSe[] = {sLowerSeUs, sLowerSeQc, sLowerSeMx, sLowerSeUsF, sLowerSeQcF, sLowerSeMxF};

// The mail BMGs, by MailKind_e.
static const char *sMailDirs[dLetter::MAIL_KIND_NUM] = {
    "sys_MAIL1/TO/",     "sys_MAIL1/MSG/",    "sys_MAIL1/FROM/",   "sys_BBS/",         "sys_MAIL2/TO2/",
    "sys_MAIL2/MSG2_A/", "sys_MAIL2/MSG2_B/", "sys_MAIL2/MSG2_C/", "sys_MAIL2/FROM2/",
};

namespace dLetter {

// 805A02D0 (static initializer 800CCBBC)
Comp_c g_comp;

// 800CB0CC
BOOL isMalePlayer() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
    if (player == NULL) {
        return TRUE;
    }
    return player->mPID.player.mGender != GENDER_FEMALE;
}

// 800CB110
int getKeySeSet() {
    int set = 0;
    switch (getLanguage()) {
    case LANGUAGE_US:
        set = 0;
        break;
    case LANGUAGE_QC:
        set = 1;
        break;
    case LANGUAGE_MX:
        set = 2;
        break;
    }
    if (!isMalePlayer()) {
        set += 3;
    }
    return set;
}

// 800CB188
void playDigitKeySe(int idx, BOOL a, BOOL b) {
    if (b) {
        idx += 20;
    } else if (a) {
        idx += 10;
    }
    playSe(sDigitSe[getKeySeSet()][idx]);
}

// 800CB1EC
void playUpperKeySe(int idx) {
    playSe(sUpperSe[getKeySeSet()][idx]);
}

// 800CB234
void playAltKeySe(int idx) {
    playSe(sAltSe[getKeySeSet()][idx]);
}

// 800CB27C
void playLowerKeySe(int idx) {
    if (dSaveOption::getKeySe()) {
        playUpperKeySe(idx);
    } else {
        playSe(sLowerSe[getKeySeSet()][idx]);
    }
}

// 800CB2DC
void playKeySe(wchar_t c, BOOL a, BOOL b) {
    if (c >= L'0' && c <= L'9') {
        playDigitKeySe(c - L'0', a, b);
    } else if (c >= L'A' && c <= L'Z') {
        if (b) {
            playAltKeySe(c - L'A');
        } else {
            playUpperKeySe(c - L'A');
        }
    } else if (c >= L'a' && c <= L'z') {
        if (b) {
            playAltKeySe(c - L'a');
        } else {
            playLowerKeySe(c - L'a');
        }
    } else {
        playSe(0x16DA);
    }
}

// 800CB34C
int getEntryNum(int kind, const char *label) {
    char path[0x40];
    sprintf(path, "%s%s", sMailDirs[kind], label);
    dScript::Res_c res(dScript::getBmgFile(dScript::getBank(), path));
    int num = res.getCount();
    return num;
}

// 800CB3C8
BOOL loadEntry(dScript::Word_c *word, int kind, u16 idx, const char *label) {
    u32 max = word->getCapacity() - 1;
    char path[0x40];
    sprintf(path, "%s%s", sMailDirs[kind], label);
    void *bmg = dScript::getBmgFile(dScript::getBank(), path);
    if (bmg == NULL) {
        return FALSE;
    }
    dScript::Res_c res(bmg);
    if (idx >= res.getCount()) {
        return FALSE;
    }
    const wchar_t *msg = res.getMessage(idx);
    if (dScript::getStringLength(msg, 0, 0) > max) {
        return FALSE;
    }
    dLetter::Word_c text;
    if (!text.set(msg, 0)) {
        return FALSE;
    }
    if (text.getLength(0) > max) {
        return FALSE;
    }
    if (!word->set(text.getText(), 0)) {
        return FALSE;
    }
    return TRUE;
}

// 800CB594
void reset() {
    g_comp.mTo.clear();
    g_comp.mFrom.clear();
    for (int i = 0; i < LETTER_WORD_NUM; i++) {
        g_comp.mWords[i].clear();
    }
    for (int i = 0; i < LETTER_NUM_WORD_NUM; i++) {
        g_comp.mNumWords[i].clear();
    }
    memset(g_comp.mRandomWords, 0, sizeof(g_comp.mRandomWords));
}

// 800CB638
BOOL loadText(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 kind, const char *label) {
    return loadTextParts(header, body, footer, kind, kind, kind, label, label, label);
}

// 800CB66C
BOOL loadTextParts(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 headerIdx,
                 u16 bodyIdx, u16 footerIdx, const char *headerLabel, const char *bodyLabel, const char *footerLabel) {
    if (!loadEntry(header, MAIL_KIND_TO, headerIdx, headerLabel)) {
        return FALSE;
    }
    if (!loadEntry(body, MAIL_KIND_MSG, bodyIdx, bodyLabel)) {
        return FALSE;
    }
    if (!loadEntry(footer, MAIL_KIND_FROM, footerIdx, footerLabel)) {
        return FALSE;
    }
    reset();
    return TRUE;
}

// 800CB720
BOOL loadBbsText(dScript::Word_c *word, u16 msgId, const char *group) {
    BOOL ok = loadEntry(word, MAIL_KIND_BBS, msgId, group);
    reset();
    return ok;
}

// 800CB760
BOOL loadText2(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 headerIdx, u16 bodyA,
                 u16 bodyB, u16 bodyC, u16 footerIdx, const char *bodyLabel, const char *label) {
    if (!loadEntry(header, MAIL_KIND_TO2, headerIdx, label)) {
        return FALSE;
    }
    if (!loadEntry(footer, MAIL_KIND_FROM2, footerIdx, label)) {
        return FALSE;
    }
    dLetter::Word_c a;
    dLetter::Word_c b;
    dLetter::Word_c c;
    if (!loadEntry(&a, MAIL_KIND_MSG2_A, bodyA, bodyLabel)) {
        return FALSE;
    }
    if (!loadEntry(&b, MAIL_KIND_MSG2_B, bodyB, bodyLabel)) {
        return FALSE;
    }
    if (!loadEntry(&c, MAIL_KIND_MSG2_C, bodyC, bodyLabel)) {
        return FALSE;
    }
    if (!body->set(a.getText(), 0)) {
        return FALSE;
    }
    if (!body->appendText(b.getText(), 0)) {
        return FALSE;
    }
    if (!body->appendText(c.getText(), 0)) {
        return FALSE;
    }
    reset();
    return TRUE;
}

// 800CBA14
u16 getToNum(const char *label) {
    return getEntryNum(MAIL_KIND_TO, label);
}

// 800CBA40
u16 getMsgNum(const char *label) {
    return getEntryNum(MAIL_KIND_MSG, label);
}

// 800CBA6C
u16 getFromNum(const char *label) {
    return getEntryNum(MAIL_KIND_FROM, label);
}

// 800CBA98
u16 getMsg2ANum(const char *label) {
    return getEntryNum(MAIL_KIND_MSG2_A, label);
}

// 800CBAC4
u16 getTo2Num(const char *label) {
    return getEntryNum(MAIL_KIND_TO2, label);
}

// 800CBAF0
void setMonthName(int slot, int month) {
    dString::Word_c word;
    dScript::setMonthName(&word, month);
    dLetter::setWord(slot, &word);
}

// 800CBB50
void setDayName(int slot, u8 day) {
    dString::Word_c word;
    dScript::setDayName(&word, day);
    dLetter::setWord(slot, &word);
}

// 800CBBB0
void setPersonalName(int slot, const dPersonalID_c *pid) {
    dHmnName::Word_c name;
    pid->setWord(&name);
    dLetter::setWord(slot, &name);
}

// 800CBC10
void setLandName(int slot, const dLandID_c *land) {
    dLandNameWord_c name;
    // dLandID_c::setWord is not const (symbol setWord__9dLandID_cFPQ27dScript6Word_c).
    const_cast<dLandID_c *>(land)->setWord(&name);
    dLetter::setWord(slot, &name);
}

// 800CBC70
void setAnmPersonalName(int slot, const dAnmPersonalID_c *animal) {
    dHmnName::Word_c name;
    const_cast<dAnmPersonalID_c *>(animal)->setWord(&name, LANGUAGE_NUM);
    dLetter::setWord(slot, &name);
}

// 800CBCD4
void setWord(int slot, dScript::Word_c *word) {
    dScript::Word_c *dst = &g_comp.mWords[slot];
    dst->clear();
    dst->copy(word, 0);
}

// 800CBD30
void setStringWord(int slot, u16 index, const char *group) {
    dString::Word_c word;
    dString::loadString(&word, index, group);
    dLetter::setWord(slot, &word);
}

// 800CBDA0
void setItemName(int slot, const dItem::Item *item) {
    dItem::name_c name(*item);
    dLetter::setWord(slot, &name);
}

// 800CBDF4
dScript::Word_c *getWord(u8 slot) {
    return &g_comp.mWords[slot];
}

// 800CBE0C
void setNumber(int slot, int value, int digits, dScript::NumberFormat_e format) {
    if (value >= 0) {
        dString::Word_c word;
        dScript::setNumber(&word, value, digits, format);
        dScript::Word_c *dst = &g_comp.mNumWords[slot];
        dst->clear();
        dst->copy(&word, 0);
    }
}

// 800CBEB4
void setNumber4(int slot, int value) {
    dString::Word_c word;
    dScript::setNumber4(&word, value);
    dScript::Word_c *dst = &g_comp.mNumWords[slot];
    dst->clear();
    dst->copy(&word, 0);
}

// 800CBF34
void formatNumber(int slot, int value, int digits, dScript::NumberFormat_e format) {
    if (value >= 0) {
        dString::Word_c word;
        dScript::formatNumber(&word, value, digits, format, 0);
        dScript::Word_c *dst = &g_comp.mNumWords[slot];
        dst->clear();
        dst->copy(&word, 0);
    }
}

// 800CBFE0
void setHour(u8 hour, int slot, int numSlot) {
    int hour12 = dScript::to12Hour(hour);
    dString::Word_c word;
    dScript::setAmPm(&word, hour);
    dLetter::setWord(slot, &word);
    formatNumber(numSlot, hour12, 2, dScript::NUM_FORMAT_REGION);
}

// 800CC070
dScript::Word_c *getNumWord(u8 slot) {
    return &g_comp.mNumWords[slot];
}

// 800CC088
u16 getRandomWord(int kind, int slot) {
    return g_comp.mRandomWords[kind][slot];
}

// 800CC0A8
void setRandomWord(int kind, int slot, u16 msgId) {
    g_comp.mRandomWords[kind][slot] = msgId;
}

// 800CC0C8
void setTo(const dPersonalID_c *to) {
    g_comp.mTo = *to;
}

// 800CC1AC
void setFrom(const dAnmPersonalID_c *sender) {
    g_comp.mFrom = *sender;
}

// 800CC2A4
dPersonalID_c *getTo() {
    return &g_comp.mTo;
}

// 800CC2B4
dAnmPersonalID_c *getFrom() {
    return &g_comp.mFrom;
}

// 800CC2C4
Word_c::Word_c() {
    clear();
}

// 800CC308
Word_c::~Word_c() {}

// 800CC360
u32 Word_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 800CC368
wchar_t *Word_c::getBuffer() {
    return mBuffer;
}

// 800CC370
int Word_c::procNumber(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    dScript::getTagId(tag);
    const u8 *params = dScript::getTagParams(tag);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    dScript::Word_c *word = getNumWord(slot);
    _18 = 1;
    return setTagWord(pos, word);
}

// 800CC418
int Word_c::procWord(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    dScript::getTagId(tag);
    const u8 *params = dScript::getTagParams(tag);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    dScript::Word_c *word = getWord(slot);
    _18 = 0;
    return setTagWord(pos, word);
}

// 800CC4BC
int Word_c::procRandomWordTag(int pos, int kind) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    dAnmPersonalID_c *id = getFrom();
    const char *group = getGenderedGroup(kind, id);
    u16 msgId;
    u16 used[LETTER_WORD_NUM];
    u16 *list = used;
    for (int i = 0; i < LETTER_WORD_NUM; i++) {
        list[i] = getRandomWord(kind, i);
    }
    int n = procRandomWord(pos, kind, id, list);
    msgId = used[slot];
    setRandomWord(kind, slot, msgId);
    setStringWord(slot, msgId, group);
    return n;
}

// 800CC5BC
int Word_c::procDrink(int pos) {
    return procRandomWordTag(pos, 0);
}

// 800CC5C4
int Word_c::procObject(int pos) {
    return procRandomWordTag(pos, 1);
}

// 800CC5CC
int Word_c::procSweets(int pos) {
    return procRandomWordTag(pos, 2);
}

// 800CC5D4
int Word_c::procSports(int pos) {
    return procRandomWordTag(pos, 3);
}

// 800CC5DC
int Word_c::procMusic(int pos) {
    return procRandomWordTag(pos, 4);
}

// 800CC5E4
int Word_c::procFood(int pos) {
    return procRandomWordTag(pos, 5);
}

// 800CC5EC
int Word_c::procHobby(int pos) {
    return procRandomWordTag(pos, 6);
}

// 800CC5F4
int Word_c::procUnit(int pos) {
    dAnmPersonalID_c *from = getFrom();
    return procAnimalUnit(pos, getTo(), from);
}

// 800CC650
int Word_c::procPlayerChar(int pos, u32 index) {
    dHmnName::Word_c name;
    getTo()->setWord(&name);
    wchar_t c = 0;
    if (!name.getChar(&c, index)) {
        return removeTag(pos);
    }
    return insertWord(pos, &c, 1, FALSE);
}

// 800CC714
int Word_c::procPlayerChar0(int pos) {
    return procPlayerChar(pos, 0);
}

// 800CC71C
int Word_c::procPlayerChar1(int pos) {
    return procPlayerChar(pos, 1);
}

// 800CC724
int Word_c::procPlayerChar2(int pos) {
    return procPlayerChar(pos, 2);
}

// 800CC72C
int Word_c::procPlayer(int pos) {
    return procPlayerName(pos, getTo());
}

// 800CC774
int Word_c::procAnimal(int pos) {
    return procAnimalName(pos, getFrom());
}

// 800CC7BC
int Word_c::procAnimalA(int pos) {
    return removeTag(pos);
}

// 800CC7C0
int Word_c::procAnimalB(int pos) {
    return removeTag(pos);
}

// 800CC7C4
int Word_c::procAnimalParam(int pos) {
    return removeTag(pos);
}

// 800CC7C8
int Word_c::procCatchphrase(int pos) {
    return removeTag(pos);
}

// 800CC7CC
int Word_c::procPlayerNickname(int pos) {
    return removeTag(pos);
}

// 800CC7D0
int Word_c::procImpression(int pos) {
    return removeTag(pos);
}

// 800CC7D4
int Word_c::procPlayerStarSign(int pos) {
    return removeTag(pos);
}

// 800CC7D8
int Word_c::procAnimalStarSign(int pos) {
    return removeTag(pos);
}

// 800CC7DC
int Word_c::procPlayerGender(int pos) {
    return procByPlayerGender(pos, getTo());
}

// 800CC824
int Word_c::procAnimalGender(int pos) {
    return procByAnimalGender(pos, getFrom());
}

// 800CC86C
int Word_c::procWordGender(int pos) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    return procByWordGender(pos, getWord(slot));
}

// 800CC8EC
int Word_c::procPlayerElision(int pos) {
    return procElisionPlayer(pos, getTo());
}

// 800CC934
int Word_c::procAnimalElision(int pos) {
    return procElisionAnimal(pos, getFrom());
}

// 800CC97C
int Word_c::procAnimalAElision(int pos) {
    return removeTag(pos);
}

// 800CC980
int Word_c::procAnimalBElision(int pos) {
    return removeTag(pos);
}

// 800CC984
int Word_c::procAnimalParamElision(int pos) {
    return removeTag(pos);
}

// 800CC988
int Word_c::procPlayerNicknameElision(int pos) {
    return removeTag(pos);
}

// 800CC98C
int Word_c::procWordElision(int pos) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    if (slot >= LETTER_WORD_NUM) {
        return removeTag(pos);
    }
    return procElisionWord(pos, 1, getWord(slot));
}

// 800CCA28
int Word_c::procNumberPlural(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    const u8 *params = dScript::getTagParams(tag);
    dScript::getTagLength(tag);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    if (slot >= LETTER_NUM_WORD_NUM) {
        return removeTag(pos);
    }
    return procPlural(pos, getNumWord(slot));
}

// 800CCAE0: Japanese only: the ruby text replaces the tag and its base text
int Word_c::procRubyTag(int pos) {
    if (getLanguage() == LANGUAGE_JP) {
        const wchar_t *tag = getBuffer() + pos;
        const u8 *params = dScript::getTagParams(tag);
        u8 len = dScript::getTagLength(tag);
        u8 base = 0;
        memcpy(&base, params, 1);
        const wchar_t *reading = (const wchar_t *)(params + 1);
        int n = len - 4;
        insert(reading, n, len + pos + base);
        erase(len + base, pos);
        return n;
    }
    return removeTag(pos);
}

} // namespace dLetter

#include "d_letter_comp.inc"
