// dRF: the random town field generator (see include/game/game/d_random_field.hpp).
// .text 80104C64..8010655C.
#include <game/game/d_random_field.hpp>
#include <game/game/d_save_main_field.hpp>
#include <game/cLib/c_math.hpp>
#include <game/mLib/m_heap.hpp>
#include <lib/egg/core/eggHeap.h>
#include <string.h>

// The block kind tables (unsplit BG code; C linkage keeps the target names).
extern "C" {
int fn_800812C8(int type);   // the kind of a block type
int fn_800812E8(u32 flags);  // the kind with exactly these flags, or BLOCK_KIND_NONE
u32 fn_80081348(int kind);   // a kind's BLOCK_KIND_FLAG_*
u8 fn_8008136C(int kind);
}

namespace dRF {

// The pond block types (see the header).
static const int sPondTypes[RF_POND_TYPE_NUM] = {0x33, 0x34, 0x35, 0x38, 0x3E, 0x3A, 0x3B, 0x43, 0x44};

// The river's step for each river shape.
static const direction_c sDirections[7] = {
    {BLOCK_KIND_FLAG_RIVER_S0, 0, 1},
    {BLOCK_KIND_FLAG_RIVER_E0, 1, 0},
    {BLOCK_KIND_FLAG_RIVER_W0, -1, 0},
    {BLOCK_KIND_FLAG_RIVER_E1, 1, 0},
    {BLOCK_KIND_FLAG_RIVER_S1, 0, 1},
    {BLOCK_KIND_FLAG_RIVER_S2, 0, 1},
    {BLOCK_KIND_FLAG_RIVER_W1, -1, 0},
};

static int sCandRequest = -1; // the candidate generate() asks for; -1 = random
static int sCandIdx;          // the last candidate used
loader_c l_loader;

// 80104C64
BOOL field_c::addKindFlags(cell_c *cell, u32 flags) {
    u32 kindFlags = fn_80081348(cell->mKind);
    kindFlags |= flags;
    int kind = fn_800812E8(kindFlags);
    if (kind != BLOCK_KIND_NONE) {
        cell->mKind = kind;
        return TRUE;
    }
    return FALSE;
}

// 80104CC0
int cell_c::getCount() const {
    return 0;
}

// 80104CC8
int field_c::getCount() {
    int count = 0;
    for (u32 x = 0; x < BLOCK_X_NUM; x++) {
        for (u32 z = 0; z < BLOCK_Z_NUM; z++) {
            count += getCell(x, z)->getCount();
        }
    }
    return count;
}

// 80104D48
u32 field_c::getIndex(int x, int z) {
    return x + z * BLOCK_X_NUM;
}

// 80104D58
void field_c::setLevels() {
    u8 level;
    for (int z = BLOCK_Z_NUM - 1; z >= 0; z--) {
        u32 west = fn_80081348(getCell(1, z)->mKind);
        u32 east = fn_80081348(getCell(5, z)->mKind);
        if ((west & BLOCK_KIND_FLAG_CLIFF_0) || (west & BLOCK_KIND_FLAG_CLIFF_1) || (west & BLOCK_KIND_FLAG_CLIFF_4)) {
            getCell(0, z)->mKind = BLOCK_KIND_BORDER_W_CLIFF;
        }
        if ((east & BLOCK_KIND_FLAG_CLIFF_3) || (east & BLOCK_KIND_FLAG_CLIFF_0) || (east & BLOCK_KIND_FLAG_CLIFF_6)) {
            getCell(6, z)->mKind = BLOCK_KIND_BORDER_E_CLIFF;
        }
    }
    for (u32 x = 1; x < 6; x++) {
        level = 0;
        for (int z = BLOCK_Z_NUM - 1; z >= 0; z--) {
            u32 flags = fn_80081348(getCell(x, z)->mKind);
            cell_c *cell = getCell(x, z);
            cell->mFlag = level & 1;
            if ((flags & BLOCK_KIND_FLAG_CLIFF_0) || (flags & BLOCK_KIND_FLAG_CLIFF_3) || (flags & BLOCK_KIND_FLAG_CLIFF_4)) {
                level++;
            }
        }
    }
    for (int z = BLOCK_Z_NUM - 1; z >= 0; z--) {
        u32 west = fn_80081348(getCell(1, z)->mKind);
        u32 east = fn_80081348(getCell(5, z)->mKind);
        if ((west & BLOCK_KIND_FLAG_CLIFF_2) || (west & BLOCK_KIND_FLAG_CLIFF_3)) {
            level = getCell(1, z)->mFlag & 1;
            cell_c *cell = getCell(0, z);
            cell->mFlag = (level + 1) & 1;
        } else {
            level = getCell(1, z)->mFlag & 1;
            getCell(0, z)->mFlag = level;
        }
        if ((east & BLOCK_KIND_FLAG_CLIFF_5) || (east & BLOCK_KIND_FLAG_CLIFF_4)) {
            level = getCell(5, z)->mFlag & 1;
            cell_c *cell = getCell(6, z);
            cell->mFlag = (level + 1) & 1;
        } else {
            level = getCell(5, z)->mFlag & 1;
            getCell(6, z)->mFlag = level;
        }
    }
}

// 80104FC8
BOOL field_c::generate(int *count) {
    bool ok = false;
    while (!ok) {
        setCandidate(sCandRequest);
        ok = true;
        ok &= setBorder();
        ok &= addBridges();
        ok &= addRamps();
        ok &= addRacco();
        ok &= placeFacilities();
        if (ok) {
            setLevels();
            ok &= setTypes();
            ok &= hasPond();
        }
        if (ok && count != NULL) {
            *count = getCount();
        }
    }
    return TRUE;
}

// 801050E8
void field_c::write(dFdBlockId_c *blocks) {
    for (u32 z = 0; z < BLOCK_Z_NUM; z++) {
        for (u32 x = 0; x < BLOCK_X_NUM; x++) {
            cell_c *cell = getCell(x, z);
            blocks->mId = cell->mType;
            blocks->mFlag = cell->mFlag;
            blocks++;
        }
    }
}

// 8010517C
bool field_c::setTypes() {
    u8 used[RF_TYPE_NUM / 8];
    memset(used, 0, sizeof(used));
    for (u32 z = 0; z < BLOCK_Z_NUM; z++) {
        for (u32 x = 0; x < BLOCK_X_NUM; x++) {
            getCell(x, z)->mType = pickType(used, getCell(x, z)->mKind);
        }
    }
    return true;
}

// 80105228
bool field_c::hasPond() {
    const int *type;
    for (u32 z = 1; z < 6; z++) {
        for (u32 x = 1; x < 6; x++) {
            type = sPondTypes;
            for (u32 i = 0; i < RF_POND_TYPE_NUM; i++, type++) {
                if (*type == getCell(x, z)->mType) {
                    return true;
                }
            }
        }
    }
    return false;
}

// 801052C0
cell_c *field_c::getCell(int x, int z) {
    u32 idx = getIndex(x, z);
    if (idx < RF_CELL_NUM) {
        return &mCells[idx];
    } else {
        static cell_c sDummy;
        return &sDummy;
    }
}

// 80105340
BOOL canAddRacco(int kind) {
    u32 flags = fn_80081348(kind);
    BOOL ok = FALSE;
    if (!(flags & BLOCK_KIND_FLAG_BEACH) && !(flags & BLOCK_KIND_FLAG_BRIDGE) && !(flags & BLOCK_KIND_FLAG_FALL) &&
        !(flags & BLOCK_KIND_FLAG_CLIFF) && fn_8008136C(kind) == 1) {
        ok = TRUE;
    }
    return ok;
}

// 801053B4
bool field_c::placeKind(int kind) {
    u32 num = 0;
    for (u32 z = 1; z < 6; z++) {
        for (u32 x = 1; x < 6; x++) {
            if (getCell(x, z)->mKind == BLOCK_KIND_PLAIN) {
                num++;
            }
        }
    }
    u32 pick = cM::rndInt(num);
    u32 i = 0;
    for (u32 z = 1; z < 6; z++) {
        for (u32 x = 1; x < 6; x++) {
            if (getCell(x, z)->mKind == BLOCK_KIND_PLAIN) {
                if (pick == i) {
                    getCell(x, z)->mKind = kind;
                    return true;
                }
                i++;
            }
        }
    }
    return false;
}

// 801054A4
BOOL getDirection(int *dx, int *dz, u32 kindFlags) {
    *dx = *dz = 0;
    for (const direction_c *dir = sDirections; dir < sDirections + 7; dir++) {
        if (dir->mFlags == (int)(dir->mFlags & kindFlags)) {
            *dx = dir->mDx;
            *dz = dir->mDz;
            return TRUE;
        }
    }
    return FALSE;
}

static inline bool isInterior(int x, int z) {
    return x >= 1 && (u32)x < 6 && z >= 1 && (u32)z < 6;
}

// 80105510
bool field_c::addBridges() {
    int firstX;
    u32 z;
    int firstZ;
    int lastX;
    bool ok;
    u32 x;
    int lastZ;
    int endX;
    int endZ;

    lastZ = -1;
    lastX = -1;
    firstZ = -1;
    firstX = -1;
    endZ = -1;
    endX = -1;
    z = 1;
    for (; z < 6; z++) {
        for (x = 1; x < 6; x++) {
            cell_c *cell = getCell(x, z);
            int cls = fn_8008136C(cell->mKind);
            if (cls == 2) {
                endX = x;
                endZ = z;
            } else if (cls == 1 && (fn_80081348(cell->mKind) & BLOCK_KIND_FLAG_FALL)) {
                lastX = x;
                lastZ = z;
                if (firstX == -1) {
                    firstX = x;
                    firstZ = z;
                }
            }
        }
    }

    // A bridge somewhere along the river from its first waterfall to its last.
    u32 firstFlags = fn_80081348(getCell(firstX, firstZ)->mKind);
    int dx, dz;
    getDirection(&dx, &dz, firstFlags);
    {
        int x = firstX + dx;
        int z = firstZ + dz;
        u32 num = 0;
        do {
            u32 flags = fn_80081348(getCell(x, z)->mKind);
            getDirection(&dx, &dz, flags);
            if (!(flags & BLOCK_KIND_FLAG_CLIFF)) {
                num++;
            }
            x += dx;
            z += dz;
        } while (x != lastX || z != lastZ);
        u32 pick = cM::rndInt(num);
        getDirection(&dx, &dz, firstFlags);
        u32 i = 0;
        x = firstX + dx;
        z = firstZ + dz;
        while (true) {
            cell_c *cell = getCell(x, z);
            u32 flags = fn_80081348(cell->mKind);
            if (i == pick && !(flags & BLOCK_KIND_FLAG_CLIFF)) {
                ok = addKindFlags(cell, BLOCK_KIND_FLAG_BRIDGE);
                break;
            }
            getDirection(&dx, &dz, flags);
            if (!(flags & BLOCK_KIND_FLAG_CLIFF)) {
                i++;
            }
            x += dx;
            z += dz;
        }

        // One more on each branch leaving the river's end.
        if (endX != -1) {
            x = endX;
            z = endZ;
        } else {
            x = lastX;
            z = lastZ;
        }
        u32 endFlags = fn_80081348(getCell(x, z)->mKind);
        for (u32 d = 0; d < 7; d++) {
            int mask = sDirections[d].mFlags;
            if (mask == (int)(mask & endFlags)) {
                int bdx, bdz;
                getDirection(&bdx, &bdz, mask);
                int bx = x + bdx;
                u32 branchNum = 0;
                int bz = z + bdz;
                while (isInterior(bx, bz)) {
                    u32 flags = fn_80081348(getCell(bx, bz)->mKind);
                    getDirection(&bdx, &bdz, flags);
                    if (!(flags & BLOCK_KIND_FLAG_CLIFF)) {
                        branchNum++;
                    }
                    bx += bdx;
                    bz += bdz;
                }
                u32 branchPick = cM::rndInt(branchNum);
                getDirection(&bdx, &bdz, mask);
                u32 j = 0;
                bx = x + bdx;
                bz = z + bdz;
                while (isInterior(bx, bz)) {
                    cell_c *cell = getCell(bx, bz);
                    u32 flags = fn_80081348(cell->mKind);
                    if (j == branchPick && !(flags & BLOCK_KIND_FLAG_CLIFF)) {
                        ok &= addKindFlags(cell, BLOCK_KIND_FLAG_BRIDGE);
                        break;
                    }
                    getDirection(&bdx, &bdz, flags);
                    if (!(flags & BLOCK_KIND_FLAG_CLIFF)) {
                        j++;
                    }
                    bx += bdx;
                    bz += bdz;
                }
            }
        }
        return ok;
    }
}

// 80105940
bool field_c::addRamps() {
    bool ok = false;
    u32 westNum = 0;
    u32 eastNum = 0;
    for (u32 z = 1; z < 6; z++) {
        bool west = true;
        for (u32 x = 1; x < 6; x++) {
            u32 flags = fn_80081348(getCell(x, z)->mKind);
            if (flags & BLOCK_KIND_FLAG_RIVER) {
                west = false;
            } else if (flags & BLOCK_KIND_FLAG_CLIFF) {
                if (west) {
                    westNum++;
                }
                if (!west) {
                    eastNum++;
                }
            }
        }
    }
    u32 westPick = cM::rndInt(westNum);
    u32 eastPick = cM::rndInt(eastNum);
    u32 westIdx = 0;
    u32 eastIdx = 0;
    for (u32 z = 1; z < 6; z++) {
        bool west = true;
        for (u32 x = 1; x < 6; x++) {
            cell_c *cell = getCell(x, z);
            u32 flags = fn_80081348(cell->mKind);
            if (flags & BLOCK_KIND_FLAG_RIVER) {
                west = false;
            } else if (flags & BLOCK_KIND_FLAG_CLIFF) {
                if (west) {
                    if (westIdx == westPick && addKindFlags(cell, BLOCK_KIND_FLAG_RAMP)) {
                        ok = true;
                    }
                    westIdx++;
                } else {
                    if (eastIdx == eastPick && addKindFlags(cell, BLOCK_KIND_FLAG_RAMP)) {
                        ok = true;
                    }
                    eastIdx++;
                }
            }
        }
    }
    return ok;
}

// 80105AB8
bool field_c::placeFacilities() {
    if (!placeKind(BLOCK_KIND_TAILOR)) {
        return false;
    }
    if (!placeKind(BLOCK_KIND_SHOP)) {
        return false;
    }
    if (!placeKind(BLOCK_KIND_TOWN_HALL)) {
        return false;
    }
    if (!placeKind(BLOCK_KIND_MUSEUM)) {
        return false;
    }
    return true;
}

// 80105B3C
bool field_c::addRacco() {
    u32 num = 0;
    for (u32 z = 1; z < 6; z++) {
        for (u32 x = 1; x < 6; x++) {
            if (canAddRacco(getCell(x, z)->mKind)) {
                num++;
            }
        }
    }
    if (num != 0) {
        u32 pick = cM::rndInt(num);
        u32 i = 0;
        for (u32 z = 1; z < 6; z++) {
            for (u32 x = 1; x < 6; x++) {
                if (canAddRacco(getCell(x, z)->mKind)) {
                    if (pick == i) {
                        addKindFlags(getCell(x, z), BLOCK_KIND_FLAG_RACCO);
                        return true;
                    }
                    i++;
                }
            }
        }
    }
    return false;
}

// 80105C3C
bool field_c::setBorder() {
    cell_c *cell;
    cell = getCell(0, 0);
    cell->mKind = BLOCK_KIND_BORDER_NW;
    cell = getCell(1, 0);
    cell->mKind = BLOCK_KIND_BORDER_N;
    cell = getCell(2, 0);
    cell->mKind = BLOCK_KIND_BORDER_N;
    cell = getCell(3, 0);
    cell->mKind = BLOCK_KIND_BORDER_N;
    cell = getCell(4, 0);
    cell->mKind = BLOCK_KIND_BORDER_N;
    cell = getCell(5, 0);
    cell->mKind = BLOCK_KIND_BORDER_N;
    cell = getCell(6, 0);
    cell->mKind = BLOCK_KIND_BORDER_NE;
    cell = getCell(0, 6);
    cell->mKind = BLOCK_KIND_SEA;
    cell = getCell(1, 6);
    cell->mKind = BLOCK_KIND_SEA;
    cell = getCell(2, 6);
    cell->mKind = BLOCK_KIND_SEA;
    cell = getCell(3, 6);
    cell->mKind = BLOCK_KIND_SEA;
    cell = getCell(4, 6);
    cell->mKind = BLOCK_KIND_SEA;
    cell = getCell(5, 6);
    cell->mKind = BLOCK_KIND_SEA;
    cell = getCell(6, 6);
    cell->mKind = BLOCK_KIND_SEA;
    cell = getCell(0, 1);
    cell->mKind = BLOCK_KIND_BORDER_W;
    cell = getCell(0, 2);
    cell->mKind = BLOCK_KIND_BORDER_W;
    cell = getCell(0, 3);
    cell->mKind = BLOCK_KIND_BORDER_W;
    cell = getCell(0, 4);
    cell->mKind = BLOCK_KIND_BORDER_W;
    cell = getCell(0, 5);
    cell->mKind = BLOCK_KIND_BORDER_W_BEACH;
    cell = getCell(6, 1);
    cell->mKind = BLOCK_KIND_BORDER_E;
    cell = getCell(6, 2);
    cell->mKind = BLOCK_KIND_BORDER_E;
    cell = getCell(6, 3);
    cell->mKind = BLOCK_KIND_BORDER_E;
    cell = getCell(6, 4);
    cell->mKind = BLOCK_KIND_BORDER_E;
    cell = getCell(6, 5);
    cell->mKind = BLOCK_KIND_BORDER_E_BEACH;
    for (u32 x = 1; x < 6; x++) {
        if (getCell(x, 1)->mKind == BLOCK_KIND_RIVER_FALL) {
            getCell(x, 0)->mKind = BLOCK_KIND_BORDER_N_RIVER;
        }
    }
    for (u32 x = 1; x <= 5; x++) {
        if (getCell(x, 1)->mKind == BLOCK_KIND_GATE) {
            cell = getCell(x, 0);
            cell->mKind = BLOCK_KIND_BORDER_N_GATE;
            return true;
        }
    }
    return false;
}

// 80105F04
bool field_c::setCandidate(int idx) {
    u8 *data;
    idx = idx < 0 ? cM::rndInt(RF_CAND_NUM) : (u32)idx % RF_CAND_NUM;
    data = (u8 *)l_loader.getData() + idx * RF_CAND_SIZE;
    if (data != NULL) {
        for (u32 z = 1; z < 6; z++) {
            for (u32 x = 1; x < 6; x++) {
                getCell(x, z)->setKind(*data);
                data++;
            }
        }
        sCandIdx = idx;
        return true;
    }
    return setCandidate(0);
}

// 80105FD0
BOOL isTypeUsed(const u8 *used, int type) {
    return (used[type / 8] >> (type & 7)) & 1;
}

// 80105FEC
void setTypeUsed(u8 *used, int type) {
    used[type / 8] |= (u8)(1 << (type & 7));
}

// 80106014
int countTypes(const u8 *used, int kind, BOOL allowUsed) {
    int num = 0;
    if (allowUsed) {
        for (u32 type = 0; type < RF_TYPE_NUM; type++) {
            if (kind == fn_800812C8(type)) {
                num++;
            }
        }
    } else {
        for (u32 type = 0; type < RF_TYPE_NUM; type++) {
            if (kind == fn_800812C8(type) && !isTypeUsed(used, type)) {
                num++;
            }
        }
    }
    return num;
}

// 801060C8
int getNthType(const u8 *used, int kind, BOOL allowUsed, int n) {
    u32 i = 0;
    if (allowUsed) {
        for (u32 type = 0; type < RF_TYPE_NUM; type++) {
            if (kind == fn_800812C8(type)) {
                if (i == n) {
                    return type;
                }
                i++;
            }
        }
    } else {
        for (u32 type = 0; type < RF_TYPE_NUM; type++) {
            if (kind == fn_800812C8(type) && !isTypeUsed(used, type)) {
                if (i == n) {
                    return type;
                }
                i++;
            }
        }
    }
    return 0;
}

// 80106190
int pickType(u8 *used, int kind) {
    int num = countTypes(used, kind, FALSE);
    if (num != 0) {
        int type = getNthType(used, kind, FALSE, cM::rndInt(num));
        setTypeUsed(used, type);
        return type;
    }
    num = countTypes(used, kind, TRUE);
    if (num != 0) {
        int type = getNthType(used, kind, TRUE, cM::rndInt(num));
        setTypeUsed(used, type);
        return type;
    }
    return 0;
}

// Defined here: after getCell's static guard in .sbss.
EGG::FrmHeap *l_frmHeap_p;

// 80106250
BOOL create(dSaveMainField_c *field, int *count) {
    if (l_frmHeap_p == NULL) {
        l_frmHeap_p = mHeap::createFrmHeap(0x72C0, EGG::Heap::getCurrentHeap(), "dRF::l_frmHeap_p", 0x20,
                                           (mHeap::AllocOptBit_t)0);
    }
    if (!l_loader.load(l_frmHeap_p)) {
        return FALSE;
    }
    field_c layout;
    layout.generate(count);
    layout.write(field->mFieldBlockData[0]);
    return TRUE;
}

// 8010640C
BOOL finishCreate() {
    if (l_loader.unload(FALSE)) {
        if (l_frmHeap_p != NULL) {
            mHeap::destroyFrmHeap(l_frmHeap_p);
            l_frmHeap_p = NULL;
        }
        return TRUE;
    }
    return FALSE;
}

// 80106464
BOOL isPondType(int type) {
    for (const int *p = sPondTypes; p < sPondTypes + RF_POND_TYPE_NUM; p++) {
        if (*p == type) {
            return TRUE;
        }
    }
    return FALSE;
}

} // namespace dRF
