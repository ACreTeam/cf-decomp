// Per-player flags of the downloaded per-language villager messages (dSvLangFlag::Flag_c).
// .text 80117118..80117400. See include/game/game/d_sv_lang_flag.hpp.
#include <game/game/d_sv_lang_flag.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_region.hpp>

namespace dSvLangFlag {

// 80117118
Flag_c::Flag_c() {}

// 8011711C
void Flag_c::clear(u32 player) {
    if (player < PLAYER_NUM) {
        mFlags[player] = 0;
    }
}

// 80117130
void Flag_c::clearAll() {
    for (int i = 0; i < PLAYER_NUM; i++) {
        clear(i);
    }
}

// 8011717C
void Flag_c::update(u32 player, const dSaveDistContentList_c *list, int lang) {
    clear(player);
    if (list == NULL) {
        list = &dSaveData_c::getRawExtra()->mDistContent;
    }
    if (list != NULL) {
        if (lang == LANG_FLAG_LANGUAGE_CURRENT) {
            lang = getLanguage();
        }
        if ((u32)lang < LANGUAGE_NUM && player < PLAYER_NUM) {
            const dSaveDistContent_c *contents = list->mContents[lang];
            for (int i = 0; i < LANG_FLAG_NUM; i++) {
                u8 looks = i;
                if (contents[looks]._0000 != 0) {
                    set(player, looks);
                }
            }
        }
    }
}

// 80117244
void Flag_c::updateAll() {
    for (int i = 0; i < PLAYER_NUM; i++) {
        update(i, NULL, LANG_FLAG_LANGUAGE_CURRENT);
    }
}

// 80117298
void Flag_c::set(u32 player, u8 looks) {
    if (player < PLAYER_NUM && looks < LANG_FLAG_NUM) {
        mFlags[player] |= 1 << looks;
    }
}

// 801172C0
void Flag_c::reset(u32 player, u8 looks) {
    if (player < PLAYER_NUM && looks < LANG_FLAG_NUM) {
        mFlags[player] &= ~(1 << looks);
    }
}

// 801172E8
BOOL Flag_c::isSet(u32 player, u8 looks) const {
    if (player < PLAYER_NUM && looks < LANG_FLAG_NUM && (mFlags[player] >> looks) & 1) {
        return TRUE;
    }
    return FALSE;
}

// 80117318
const dSaveDistContent_c *Flag_c::get(u32 player, u8 looks, int lang) const {
    if (looks >= LANG_FLAG_NUM) {
        return NULL;
    }
    if (player >= PLAYER_NUM) {
        return NULL;
    }
    if (lang == LANG_FLAG_LANGUAGE_CURRENT) {
        lang = getLanguage();
    }
    if ((u32)lang >= LANGUAGE_NUM) {
        return NULL;
    }
    if (!isSet(player, looks)) {
        return NULL;
    }
    const dSaveDistContent_c *content = &dSaveData_c::getRawExtra()->mDistContent.mContents[lang][looks];
    if (content->_0000 != 0) {
        return content;
    }
    return NULL;
}

} // namespace dSvLangFlag
