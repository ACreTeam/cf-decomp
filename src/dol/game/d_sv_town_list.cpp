// The list of other towns and their jinxes (dSvTownList::List_c). .text 80150140..801503E4,
// .sdata2 80750CD0..80750CD8. See include/game/game/d_sv_town_list.hpp.
#include <game/game/d_sv_town_list.hpp>
#include <game/cLib/c_math.hpp>

namespace dSvTownList {

// 80150140
List_c::List_c() {
    for (u32 i = 0; i < TOWN_LIST_NUM; i++) {
        mTowns[i].mLand.clear();
    }
}

// 801501F0
void List_c::set(const dLandID_c *land, dSaveJinx_c jinx) {
    Entry_c *last = &mTowns[TOWN_LIST_NUM - 1];
    Entry_c *entry;
    for (entry = mTowns; entry <= last; entry++) {
        if (entry->mLand.mId == land->mId) {
            entry->mLand.copy(land);
            entry->mJinx = jinx;
            return;
        }
    }
    for (entry = mTowns; entry <= last; entry++) {
        if (!entry->mLand.isValid()) {
            entry->mLand.copy(land);
            entry->mJinx = jinx;
            return;
        }
    }
    if (cM::rndF(100.0f) < 50.0f) {
        return;
    }
    int idx = cM::rndInt(TOWN_LIST_NUM);
    mTowns[idx].mLand.copy(land);
    mTowns[idx].mJinx = jinx;
}

// 80150308
Entry_c *List_c::get(int idx) {
    Entry_c *entry = &mTowns[idx];
    if (entry->mLand.isValid() == FALSE) {
        return NULL;
    }
    return entry;
}

// 80150350
const Entry_c *List_c::get(int idx) const {
    const Entry_c *entry = &mTowns[idx];
    if (entry->mLand.isValid() == FALSE) {
        return NULL;
    }
    return entry;
}

// 80150398
void List_c::clear() {
    for (u32 i = 0; i < TOWN_LIST_NUM; i++) {
        mTowns[i].mLand.clear();
    }
}

} // namespace dSvTownList
