// Katie's save data (dSaveCatherine_c). .text 80147A48..80147ECC.
#include <game/game/d_save_catherine.hpp>
#include <string.h>

// 80147A48
void dSaveCatherine_c::clear() {
    memset(this, 0, sizeof(dSaveCatherine_c));
    mCounted = 0;
    for (int i = 0; i < 2; i++) {
        mPersons[i].clear();
        mCategory[i] = -1;
        mSub[i] = -1;
    }
}

// 80147ACC
BOOL dSaveCatherine_c::isValid(int slot) {
    BOOL valid = FALSE;
    if (mPersons[slot].isValid() && mCounted) {
        valid = TRUE;
    }
    return valid;
}

// 80147B2C
void dSaveCatherine_c::set(const dPersonalID_c *person, u8 category, u8 sub) {
    if (person->isValid() && category < CATHERINE_CATEGORY_NUM && category != 12 && category != 25 &&
        sub < CATHERINE_SUB_NUM) {
        if (category < 13) {
            mCategory[0] = category;
            mPersons[0] = *person;
            mSub[0] = sub;
        } else if (category < CATHERINE_CATEGORY_NUM) {
            mCategory[1] = category;
            mPersons[1] = *person;
            mSub[1] = sub;
        }
    }
}

// 80147D18
void dSaveCatherine_c::count(u8 category, u8 sub) {
    if (category < CATHERINE_CATEGORY_NUM && category != 12 && category != 25) {
        if (mCount[category] == 0xFF) {
            mCount[category] >>= 1;
        } else {
            mCount[category]++;
        }
    }
    if (sub < CATHERINE_SUB_NUM) {
        if (mCountSub[sub] == 0xFF) {
            mCountSub[sub] >>= 1;
        } else {
            mCountSub[sub]++;
        }
    }
    mCounted = 1;
}

// 80147D88
u8 dSaveCatherine_c::getTopCategory(int group) {
    int start;
    int end;
    int top;
    switch (group) {
    case 0:
        start = 0;
        end = 13;
        top = 0;
        break;
    case 1:
        start = 13;
        end = CATHERINE_CATEGORY_NUM;
        top = 13;
        break;
    }

    for (int i = start; i < end; i++) {
        if (i == 12 || i == 25) {
            continue;
        }
        if (mCount[i] > mCount[top]) {
            top = i;
        }
    }
    return top;
}

// 80147E08
int dSaveCatherine_c::getTopSub() {
    int top = 0;
    for (int i = 0; i < CATHERINE_SUB_NUM; i++) {
        if (mCountSub[i] > mCountSub[top]) {
            top = i;
        }
    }
    return top;
}
