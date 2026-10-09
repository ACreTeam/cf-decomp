// Wild World (DS) save data: the players and the town name of a save sent from the DS.
// .text 80085388..80085E8C, .rodata 8046D5F0..8046DAD8, .data 804DE818..804DE838.
// See include/game/game/d_ds_player_data.hpp and notes/d_ds_player_data.txt.
#include <game/game/d_ds_player_data.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_personal_id.hpp>
#include <cstring>

namespace dDsPlayerData {

// Offsets of the fields of a player (from the player's start).
struct playerLayout_s {
    /* 0x00 */ u32 mOption;    // bit 0
    /* 0x04 */ u32 mFace;      // low nibble
    /* 0x08 */ u32 mHair;      // high nibble (same byte as mFace)
    /* 0x0C */ u32 mHairColor; // bits 0..2
    /* 0x10 */ u32 mTownId;    // u16
    /* 0x14 */ u32 mPlayerId;  // u16
    /* 0x18 */ u32 mName;
    /* 0x1C */ u32 mGender;
    /* 0x20 */ u32 mCatalog;
};

// Offsets of the fields of a save copy.
struct saveLayout_s {
    /* 0x00 */ u32 mTownName;
    /* 0x04 */ u32 mPlayers;
    /* 0x08 */ u32 _08;
    /* 0x0C */ u32 mState;
    /* 0x10 */ u32 mSaveCount;
};

// Wild World character codes (one byte per character) to wchar_t, Japan.
static const u16 l_charTableJp[256] = {
    0x0000, 0x3042, 0x3044, 0x3046, 0x3048, 0x304A, 0x304B, 0x304D,
    0x304F, 0x3051, 0x3053, 0x3055, 0x3057, 0x3059, 0x305B, 0x305D,
    0x305F, 0x3061, 0x3064, 0x3066, 0x3068, 0x306A, 0x306B, 0x306C,
    0x306D, 0x306E, 0x306F, 0x3072, 0x3075, 0x3078, 0x307B, 0x307E,
    0x307F, 0x3080, 0x3081, 0x3082, 0x3084, 0x3086, 0x3088, 0x3089,
    0x308A, 0x308B, 0x308C, 0x308D, 0x308F, 0x3092, 0x3093, 0x304C,
    0x304E, 0x3050, 0x3052, 0x3054, 0x3056, 0x3058, 0x305A, 0x305C,
    0x305E, 0x3060, 0x3062, 0x3065, 0x3067, 0x3059, 0x3070, 0x3073,
    0x3076, 0x3079, 0x307C, 0x3071, 0x3074, 0x3077, 0x307A, 0x307D,
    0x3041, 0x3043, 0x3045, 0x3047, 0x3049, 0x3083, 0x3085, 0x3087,
    0x3063, 0x30A2, 0x30A4, 0x30A6, 0x30A8, 0x30AA, 0x30AB, 0x30AD,
    0x30AF, 0x30B1, 0x30B3, 0x30B5, 0x30B7, 0x30B9, 0x30BB, 0x30BD,
    0x30BF, 0x30C1, 0x30C4, 0x30C6, 0x30C8, 0x30CA, 0x30CB, 0x30CC,
    0x30CD, 0x30CE, 0x30CF, 0x30D2, 0x30D5, 0x30D8, 0x30DB, 0x30DE,
    0x30DF, 0x30E0, 0x30E1, 0x30E2, 0x30E4, 0x30E6, 0x30E8, 0x30E9,
    0x30EA, 0x30EB, 0x30EC, 0x30ED, 0x30EF, 0x30F2, 0x30F3, 0x30AC,
    0x30AE, 0x30B0, 0x30B2, 0x30B4, 0x30B6, 0x30B8, 0x30BA, 0x30BC,
    0x30BE, 0x30C0, 0x30C2, 0x30C5, 0x30C7, 0x30C9, 0x30D0, 0x30D3,
    0x30D6, 0x30D9, 0x30DC, 0x30D1, 0x30D4, 0x30D7, 0x30DA, 0x30DD,
    0x30A1, 0x30A3, 0x30A5, 0x30A7, 0x30A9, 0x30E3, 0x30E5, 0x30E7,
    0x30C3, 0x30F4, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046,
    0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E,
    0x004F, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056,
    0x0057, 0x0058, 0x0059, 0x005A, 0x0061, 0x0062, 0x0063, 0x0064,
    0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C,
    0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
    0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x0030, 0x0031,
    0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039,
    0x3000, 0x000A, 0x30FC, 0xFF5E, 0x30FB, 0x3002, 0x3001, 0xFF01,
    0xFF1F, 0x002E, 0x002C, 0x300C, 0x300D, 0x0028, 0x0029, 0x003C,
    0x003E, 0x0027, 0x201D, 0x005F, 0x002B, 0x003D, 0x0026, 0x0040,
    0x003A, 0x003B, 0x00D7, 0x00F7, 0xE06D, 0xE06E, 0xE06C, 0xE06F,
};

// The same for America and Europe.
static const u16 l_charTableUs[256] = {
    0x0000, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005A, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065,
    0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D,
    0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075,
    0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x0030, 0x0031, 0x0032,
    0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x0192,
    0x0160, 0x0152, 0x017D, 0x0161, 0x0153, 0x017E, 0x0178, 0x00C0,
    0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8,
    0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF, 0x00D0,
    0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D8, 0x00D9,
    0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, 0x00E0, 0x00E1,
    0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9,
    0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 0x00F0, 0x00F1,
    0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F8, 0x00F9, 0x00FA,
    0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF, 0x0020, 0x000A, 0x0021,
    0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029,
    0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F, 0x003A, 0x003B,
    0x003C, 0x003D, 0x003E, 0x003F, 0x0040, 0x005B, 0x005C, 0x005D,
    0x005E, 0x005F, 0x0060, 0x007B, 0x007C, 0x007D, 0xFF5E, 0x20AC,
    0x201A, 0x201E, 0x2026, 0x2020, 0x2021, 0x2038, 0x2030, 0x2039,
    0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC,
    0x2122, 0x203A, 0x0020, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5,
    0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD,
    0x00AE, 0x00AF, 0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5,
    0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD,
    0x00BE, 0x00BF, 0x00D7, 0x00F7, 0xE06D, 0xE06E, 0xE06C, 0xE06F,
};

static const playerLayout_s l_playerLayoutJp = {0x17EF, 0x1CC6, 0x1CC6, 0x1CC7, 0x1CFC, 0x1D04, 0x1D06, 0x1D0C, 0x16CC};
static const playerLayout_s l_playerLayoutUs = {0x1C6B, 0x223C, 0x223C, 0x223D, 0x2276, 0x2280, 0x2282, 0x228A, 0x1B48};
static const playerLayout_s l_playerLayoutKr = {0x1D3B, 0x243C, 0x243C, 0x243D, 0x247E, 0x248C, 0x248E, 0x249A, 0x1C18};

static const saveLayout_s l_saveLayoutJp = {0x4, 0xC, 0x24444, 0x24446, 0x24447};
static const saveLayout_s l_saveLayoutUs = {0x4, 0xC, 0x2BFBC, 0x2BFBE, 0x2BFBF};
static const saveLayout_s l_saveLayoutKr = {0x4, 0x14, 0x2E7F4, 0x2E7F6, 0x2E7F7};

static const u32 l_playerSize[4] = {0x1D10, 0x228C, 0x228C, 0x249C};
static const u32 l_saveSize[4] = {0x12224, 0x15FE0, 0x15FE0, 0x173FC};
static const u32 l_nameLen[4] = {6, 8, 8, 6};

static const playerLayout_s *l_playerLayout[4] = {
    &l_playerLayoutJp,
    &l_playerLayoutUs,
    &l_playerLayoutUs,
    &l_playerLayoutKr,
};

static const saveLayout_s *l_saveLayout[4] = {
    &l_saveLayoutJp,
    &l_saveLayoutUs,
    &l_saveLayoutUs,
    &l_saveLayoutKr,
};

int getDsRegion() {
    return getRegion();
}

u16 swap16(u16 value) {
    return (value << 8) | ((value >> 8) & 0xFF);
}

u16 calcChecksum(const u16 *data, int size) {
    u16 sum = 0;
    while (size != 0) {
        sum += swap16(*data);
        data++;
        size -= 2;
    }
    return sum;
}

u16 convertKrChar(u16 c) {
    u16 ret = c;
    switch (c) {
    case '{':
        ret = 0xE06D;
        break;
    case '%':
        ret = 0xE06E;
        break;
    case '*':
        ret = 0xE06C;
        break;
    case '/':
        ret = 0xE06F;
        break;
    }
    if (c >= 0xFF10 && c <= 0xFF5A) {
        ret = c - 0xFEE0;
    }
    return ret;
}

void getPlayerName(const u8 *player, dScript::Word_c *word) {
    int region = getDsRegion();
    wchar_t name[9];
    memset(name, 0, sizeof(name));
    if (region == CONSOLE_REGION_KR) {
        const u16 *src = (const u16 *)(player + l_playerLayout[region]->mName);
        for (u32 i = 0; i < l_nameLen[region]; i++) {
            name[i] = convertKrChar(swap16(src[i]));
        }
        word->set(name, 0);
    } else {
        const u8 *src = player + l_playerLayout[region]->mName;
        for (u32 i = 0; i < l_nameLen[region]; i++) {
            if (region == CONSOLE_REGION_JP) {
                name[i] = l_charTableJp[src[i]];
            } else {
                name[i] = l_charTableUs[src[i]];
            }
        }
        word->set(name, 0);
    }
}

u8 getOption(const u8 *player) {
    return player[l_playerLayout[getDsRegion()]->mOption] & 1;
}

u8 getHairColor(const u8 *player) {
    return player[l_playerLayout[getDsRegion()]->mHairColor] & 7;
}

u8 getHair(const u8 *player) {
    int region = getDsRegion();
    u8 hairs[16] = {0, 1, 2, 3, 4, 5, 6, 7, 13, 14, 15, 16, 17, 18, 19, 20};
    return hairs[(player[l_playerLayout[region]->mHair] >> 4) & 0xF];
}

u8 getFace(const u8 *player) {
    return player[l_playerLayout[getDsRegion()]->mFace] & 0xF;
}

u16 getTownId(const u8 *player) {
    return swap16(*(const u16 *)(player + l_playerLayout[getDsRegion()]->mTownId));
}

u16 getPlayerId(const u8 *player) {
    return swap16(*(const u16 *)(player + l_playerLayout[getDsRegion()]->mPlayerId));
}

u8 getGender(const u8 *player) {
    return player[l_playerLayout[getDsRegion()]->mGender];
}

const u8 *getCatalog(const u8 *player) {
    return player + l_playerLayout[getDsRegion()]->mCatalog;
}

BOOL isValidPlayer(const u8 *player) {
    u16 townId = getTownId(player);
    u16 playerId = getPlayerId(player);
    if (townId != 0 && playerId != 0) {
        return TRUE;
    }
    return FALSE;
}

int countPlayers(const u8 *save) {
    int num = 0;
    for (int i = 0; i < 4; i++) {
        if (isValidPlayer(getPlayer(save, i))) {
            num++;
        }
    }
    return num;
}

int findPlayer(const u8 *save, u32 n) {
    int idx = -1;
    u32 count = 0;
    for (int i = 0; i < 4; i++) {
        if (isValidPlayer(getPlayer(save, i))) {
            if (n == count) {
                idx = i;
                break;
            }
            count++;
        }
    }
    return idx;
}

const u8 *getPlayer(const u8 *save, int idx) {
    int region = getDsRegion();
    return save + l_saveLayout[region]->mPlayers + idx * l_playerSize[region];
}

void getTownName(const u8 *save, dScript::Word_c *word) {
    int region = getDsRegion();
    wchar_t name[PLAYER_NAME_LEN+1];
    memset(name, 0, sizeof(name));
    if (region == CONSOLE_REGION_KR) {
        const u16 *src = (const u16 *)(save + l_saveLayout[region]->mTownName);
        for (u32 i = 0; i < l_nameLen[region]; i++) {
            name[i] = convertKrChar(swap16(src[i]));
        }
        word->set(name, 0);
    } else {
        const u8 *src = save + l_saveLayout[region]->mTownName;
        for (u32 i = 0; i < l_nameLen[region]; i++) {
            if (region == CONSOLE_REGION_JP) {
                name[i] = l_charTableJp[src[i]];
            } else {
                name[i] = l_charTableUs[src[i]];
            }
        }
        word->set(name, 0);
    }
}

u8 getState(const u8 *save) {
    return save[l_saveLayout[getDsRegion()]->mState];
}

u8 getSaveCount(const u8 *save) {
    return save[l_saveLayout[getDsRegion()]->mSaveCount];
}

BOOL isChecksumOK(const u8 *save) {
    return (u16)calcChecksum((const u16 *)save, l_saveSize[getDsRegion()]) == 0;
}

BOOL isState2(const u8 *save) {
    return getState(save) == 2;
}

u32 isUsable(const u8 *save) {
    if (getState(save) == 2 || getState(save) == 0x1C) {
        return FALSE;
    }
    return TRUE;
}

void clearBuffer(u8 *buf) {
    memset(buf, 0, 0x40000);
}

u8 *getSave(u8 *buf, int idx) {
    return buf + idx * l_saveSize[getDsRegion()];
}

u8 *selectSave(u8 *buf) {
    u8 *save0 = getSave(buf, 0);
    u8 *save1 = getSave(buf, 1);
    BOOL state0 = isState2(save0);

    // @BUG - devs checked the save copy 0 state instead of save copy 1
#ifndef BUGFIXES
    BOOL state1 = isState2(save0);
#else
    BOOL state1 = isState2(save1);
#endif
    if (!state0 && !state1) {
        u32 usable0 = isUsable(save0);

        // @BUG - devs checked the save copy 0 state instead of save copy 1
#ifndef BUGFIXES
        u32 usable1 = isUsable(save0);
#else
        u32 usable1 = isUsable(save1);
#endif
        if (usable0 == TRUE) {
            return save0;
        }
        if (usable1 == TRUE) {
            return save1;
        }
        return NULL;
    }
    if (!state0) {
        return save1;
    }
    if (!state1) {
        return save0;
    }
    BOOL ok0 = isChecksumOK(save0);
    BOOL ok1 = isChecksumOK(save1);
    if (!ok0 && !ok1) {
        return NULL;
    }
    if (!ok0) {
        return save1;
    }
    if (!ok1) {
        return save0;
    }
    u8 count0 = getSaveCount(save0);
    u8 count1 = getSaveCount(save1);
    if (count0 == count1) {
        return save1;
    }
    return save0;
}

} // namespace dDsPlayerData
