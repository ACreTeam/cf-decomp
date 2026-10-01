#include <game/game/d_animal_id.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_script.hpp>
#include <cstring>
#include <cstdio>

extern "C" {
void fn_80116710(dLandID_c *land); // clear
BOOL fn_80116758(const dLandID_c *land); // isValid
void fn_8011676C(dLandID_c *dst, const dLandID_c *src); // copy
}

// 804760D8
static const int lbl_804760D8[LANGUAGE_NUM] = {0, 1, 1, 1, 2, 3, 4, 5, 6, 7};

// 80476100
static const char lbl_80476100[LOOKS_TYPE_NUM][13] = {
    "npc_4_BO/BO_", "npc_5_HA/HA_", "npc_6_KO/KO_", "npc_1_FU/FU_", "npc_2_GE/GE_", "npc_3_TA/TA_",
};

// 80135E88
void dAnmPersonalID_c::clear() {
    memset(this, 0, sizeof(dAnmPersonalID_c));
    mNpcIdx = 0xFFFF;
    mLooks = LOOKS_TYPE_NUM;
    fn_80116710(&mLand);
    fn_80116710(&mLand2);
}

// 80135EE0
int dAnmPersonalID_c::getNameSlot(int language) {
    if ((u32)language < LANGUAGE_NUM) {
        return lbl_804760D8[language];
    }
    return 0;
}

// 80135F04
void dAnmPersonalID_c::setName(int slot, const wchar_t *name) {
    memcpy(mNameByRegion[slot], name, sizeof(mNameByRegion[slot]));
}

// 80135F20
void dAnmPersonalID_c::set(u16 npcIdx, u8 looks, const dLandID_c *land, const wchar_t *name0,
                           const wchar_t *name1, const wchar_t *name2, const wchar_t *name3,
                           const wchar_t *name4, const wchar_t *name5, const wchar_t *name6,
                           const wchar_t *name7) {
    mNpcIdx = npcIdx;
    mLooks = looks;
    fn_8011676C(&mLand, land);
    fn_8011676C(&mLand2, land);
    setName(0, name0);
    setName(1, name1);
    setName(2, name2);
    setName(3, name3);
    setName(4, name4);
    setName(5, name5);
    setName(6, name6);
    setName(7, name7);
}

// 80136010
BOOL dAnmPersonalID_c::isValid() const {
    return mNpcIdx != 0xFFFF && mLooks != LOOKS_TYPE_NUM && fn_80116758(&mLand2);
}

// 80136068
void dAnmPersonalID_c::copy(const dAnmPersonalID_c *other) {
    memcpy(this, other, sizeof(dAnmPersonalID_c));
}

// 80136070
u8 dAnmPersonalID_c::getLooks() {
    return mLooks;
}

// 80136078
int dAnmPersonalID_c::looksToGender(u8 looks) {
    int gender = GENDER_OTHER;
    if (looks <= LOOKS_TYPE_CRANKY) {
        gender = GENDER_MALE;
    } else if (looks <= LOOKS_TYPE_SNOOTY) {
        gender = GENDER_FEMALE;
    }
    return gender;
}

// 801360A0
void dAnmPersonalID_c::makeResName(char *buf, u32 size, const char *name, u32 looks) {
    memset(buf, 0, size);
    if (looks < LOOKS_TYPE_NUM && strlen(name) + 12 <= size) {
        sprintf(buf, "%s%s", lbl_80476100[looks], name);
    }
}

// 80136138
int dAnmPersonalID_c::getGender(int) {
    return looksToGender(getLooks());
}

// 80136160
const wchar_t *dAnmPersonalID_c::getName(int language) {
    if (language >= LANGUAGE_NUM) {
        language = fn_801068B4();
    }
    return mNameByRegion[getNameSlot(language)];
}

// 801361AC
void dAnmPersonalID_c::setWord(dScript::Word_c *word, int language) {
    word->set(getName(language), 0);
    u8 gender = getGender(0);
    dScript::Inflect_c &inflect = word->mInflect;
    switch (gender) {
    case GENDER_MALE:
        inflect.setGender(GENDER_MALE);
        break;
    case GENDER_FEMALE:
        inflect.setGender(GENDER_FEMALE);
        break;
    default:
        inflect.setGender(GENDER_OTHER);
        break;
    }
}
