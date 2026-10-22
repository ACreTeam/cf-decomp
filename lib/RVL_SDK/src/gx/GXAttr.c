#include <private/gx.h>
#include <revolution/gx.h>

static void SETVCDATTR(GXAttr name, GXAttrType type);
static void SETVAT(u32* vatA, u32* vatB, u32* vatC, GXAttr attr,
                   GXCompCnt compCnt, GXCompType compType, u8 shift);
static u8 GetFracForNrm(GXCompType compType);

static void __GXXfVtxSpecs(void) {
    unsigned int cmd;
    u32 nrm;
    u32 clr;
    u32 txc;

    nrm = __GXData->hasBiNrms ? 2 : (__GXData->hasNrms ? 1 : 0);

    // Both fields in one access
    clr = 32 - __cntlzw(GX_GET_REG_FIELD(__GXData->vcdLo, 4, 13)) + 1;

    // All 16 bits in VCD_Hi
    txc = 32 - __cntlzw(GX_GET_REG_FIELD(__GXData->vcdHi, 16, 0)) + 1;

    GX_WRITE_U8(GX_LOAD_XF_REG);
    GX_WRITE_U32(0x1008);
    cmd = 0;
    cmd = cmd | clr >> 1;
    cmd = (txc << 3) & ~0xF | cmd;
    GX_WRITE_U32(cmd | nrm << 2);

    __GXData->bpSentNot = TRUE;
}

void GXSetVtxDesc(GXAttr name, GXAttrType type) {
    SETVCDATTR(name, type);

    if (__GXData->hasNrms || __GXData->hasBiNrms) {
        GX_SET_REG_FIELD(__GXData->vcdLo, 2, 11, __GXData->nrmType);
    } else {
        GX_SET_REG_FIELD(__GXData->vcdLo, 2, 11, GX_NONE);
    }

    __GXData->dirtyState |= 8;
}

static inline void SETVCDATTR(GXAttr name, GXAttrType type) {
    switch (name) {
    case GX_VA_PNMTXIDX: {
        GX_SET_REG_FIELD(__GXData->vcdLo, 1, 0, type);
        break;
    }

    case GX_VA_TEX0MTXIDX: {
        GX_SET_REG_FIELD(__GXData->vcdLo, 1, 1, type);
        break;
    }

    case GX_VA_TEX1MTXIDX: {
        GX_SET_REG_FIELD(__GXData->vcdLo, 1, 2, type);
        break;
    }

    case GX_VA_TEX2MTXIDX: {
        GX_SET_REG_FIELD(__GXData->vcdLo, 1, 3, type);
        break;
    }

    case GX_VA_TEX3MTXIDX: {
        GX_SET_REG_FIELD(__GXData->vcdLo, 1, 4, type);
        break;
    }

    case GX_VA_TEX4MTXIDX: {
        GX_SET_REG_FIELD(__GXData->vcdLo, 1, 5, type);
        break;
    }

    case GX_VA_TEX5MTXIDX: {
        GX_SET_REG_FIELD(__GXData->vcdLo, 1, 6, type);
        break;
    }

    case GX_VA_TEX6MTXIDX: {
        GX_SET_REG_FIELD(__GXData->vcdLo, 1, 7, type);
        break;
    }

    case GX_VA_TEX7MTXIDX: {
        GX_SET_REG_FIELD(__GXData->vcdLo, 1, 8, type);
        break;
    }

    case GX_VA_POS: {
        GX_SET_REG_FIELD(__GXData->vcdLo, 2, 9, type);
        break;
    }

    case GX_VA_NRM: {
        if (type != GX_NONE) {
            __GXData->hasNrms = TRUE;
            // GX_VA_NRM and GX_VA_NBT should not be enabled at the same time
            __GXData->hasBiNrms = FALSE;
            __GXData->nrmType = type;
        } else {
            __GXData->hasNrms = FALSE;
        }
        break;
    }

    case GX_VA_NBT: {
        if (type != GX_NONE) {
            __GXData->hasBiNrms = TRUE;
            // GX_VA_NRM and GX_VA_NBT should not be enabled at the same time
            __GXData->hasNrms = FALSE;
            __GXData->nrmType = type;
        } else {
            __GXData->hasBiNrms = FALSE;
        }
        break;
    }

    case GX_VA_CLR0: {
        GX_SET_REG_FIELD(__GXData->vcdLo, 2, 13, type);
        break;
    }

    case GX_VA_CLR1: {
        GX_SET_REG_FIELD(__GXData->vcdLo, 2, 15, type);
        break;
    }

    case GX_VA_TEX0: {
        GX_SET_REG_FIELD(__GXData->vcdHi, 2, 0, type);
        break;
    }

    case GX_VA_TEX1: {
        GX_SET_REG_FIELD(__GXData->vcdHi, 2, 2, type);
        break;
    }

    case GX_VA_TEX2: {
        GX_SET_REG_FIELD(__GXData->vcdHi, 2, 4, type);
        break;
    }

    case GX_VA_TEX3: {
        GX_SET_REG_FIELD(__GXData->vcdHi, 2, 6, type);
        break;
    }

    case GX_VA_TEX4: {
        GX_SET_REG_FIELD(__GXData->vcdHi, 2, 8, type);
        break;
    }

    case GX_VA_TEX5: {
        GX_SET_REG_FIELD(__GXData->vcdHi, 2, 10, type);
        break;
    }

    case GX_VA_TEX6: {
        GX_SET_REG_FIELD(__GXData->vcdHi, 2, 12, type);
        break;
    }

    case GX_VA_TEX7: {
        GX_SET_REG_FIELD(__GXData->vcdHi, 2, 14, type);
        break;
    }

    default: {
        break;
    }
    }
}

void GXSetVtxDescv(const GXVtxDescList* list) {
    for (; list->attr != GX_VA_NULL; list++) {
        SETVCDATTR(list->attr, list->type);
    }

    if (__GXData->hasNrms || __GXData->hasBiNrms) {
        GX_SET_REG_FIELD(__GXData->vcdLo, 2, 11, __GXData->nrmType);
    } else {
        GX_SET_REG_FIELD(__GXData->vcdLo, 2, 11, GX_NONE);
    }

    __GXData->dirtyState |= 8;
}

void __GXSetVCD(void) {
    GX_WRITE_U8(GX_LOAD_CP_REG);
    GX_WRITE_U8(0x50);
    GX_WRITE_U32(__GXData->vcdLo);
    GX_WRITE_U8(GX_LOAD_CP_REG);
    GX_WRITE_U8(0x60);
    GX_WRITE_U32(__GXData->vcdHi);
    __GXXfVtxSpecs();
}

void __GXCalculateVLim(void) {
    static u8 tbl1[] = {0, 4, 1, 2};
    static u8 tbl2[] = {0, 8, 1, 2};
    static u8 tbl3[] = {0, 12, 1, 2};

    u32 vlim;
    u32 vcdLo;
    u32 vcdHi;
    s32 compCnt;

    if (__GXData->vNum == 0) {
        return;
    }

    vcdLo = __GXData->vcdLo;
    vcdHi = __GXData->vcdHi;

    // GXCompCnt bit of normal parameters
    compCnt = __GXData->vatA[GX_VTXFMT0];
    compCnt = (compCnt & 0x200) >> 9;

    vlim = GX_GET_REG_FIELD(vcdLo, 1, 0);
    vlim += GX_GET_REG_FIELD(vcdLo, 1, 1);
    vlim += GX_GET_REG_FIELD(vcdLo, 1, 2);
    vlim += GX_GET_REG_FIELD(vcdLo, 1, 3);
    vlim += GX_GET_REG_FIELD(vcdLo, 1, 4);
    vlim += GX_GET_REG_FIELD(vcdLo, 1, 5);
    vlim += GX_GET_REG_FIELD(vcdLo, 1, 6);
    vlim += GX_GET_REG_FIELD(vcdLo, 1, 7);
    vlim += GX_GET_REG_FIELD(vcdLo, 1, 8);

    vlim += tbl3[GX_GET_REG_FIELD(vcdLo, 2, 9)];
    vlim +=
        tbl3[GX_GET_REG_FIELD(vcdLo, 2, 11)] * (compCnt == GX_NRM_NBT ? 3 : 1);
    vlim += tbl1[GX_GET_REG_FIELD(vcdLo, 2, 13)];
    vlim += tbl1[GX_GET_REG_FIELD(vcdLo, 2, 15)];

    vlim += tbl2[GX_GET_REG_FIELD(vcdHi, 2, 0)];
    vlim += tbl2[GX_GET_REG_FIELD(vcdHi, 2, 2)];
    vlim += tbl2[GX_GET_REG_FIELD(vcdHi, 2, 4)];
    vlim += tbl2[GX_GET_REG_FIELD(vcdHi, 2, 6)];
    vlim += tbl2[GX_GET_REG_FIELD(vcdHi, 2, 8)];
    vlim += tbl2[GX_GET_REG_FIELD(vcdHi, 2, 10)];
    vlim += tbl2[GX_GET_REG_FIELD(vcdHi, 2, 12)];
    vlim += tbl2[GX_GET_REG_FIELD(vcdHi, 2, 14)];

    __GXData->vLim = vlim;
}

void GXGetVtxDesc(GXAttr name, GXAttrType* type) {
    GXAttrType result;

    switch (name) {
    case GX_VA_PNMTXIDX: {
        result = GX_GET_REG_FIELD(__GXData->vcdLo, 1, 0);
        break;
    }

    case GX_VA_TEX0MTXIDX: {
        result = GX_GET_REG_FIELD(__GXData->vcdLo, 1, 1);
        break;
    }

    case GX_VA_TEX1MTXIDX: {
        result = GX_GET_REG_FIELD(__GXData->vcdLo, 1, 2);
        break;
    }

    case GX_VA_TEX2MTXIDX: {
        result = GX_GET_REG_FIELD(__GXData->vcdLo, 1, 3);
        break;
    }

    case GX_VA_TEX3MTXIDX: {
        result = GX_GET_REG_FIELD(__GXData->vcdLo, 1, 4);
        break;
    }

    case GX_VA_TEX4MTXIDX: {
        result = GX_GET_REG_FIELD(__GXData->vcdLo, 1, 5);
        break;
    }

    case GX_VA_TEX5MTXIDX: {
        result = GX_GET_REG_FIELD(__GXData->vcdLo, 1, 6);
        break;
    }

    case GX_VA_TEX6MTXIDX: {
        result = GX_GET_REG_FIELD(__GXData->vcdLo, 1, 7);
        break;
    }

    case GX_VA_TEX7MTXIDX: {
        result = GX_GET_REG_FIELD(__GXData->vcdLo, 1, 8);
        break;
    }

    case GX_VA_POS: {
        result = GX_GET_REG_FIELD(__GXData->vcdLo, 2, 9);
        break;
    }

    case GX_VA_NRM: {
        result =
            __GXData->hasNrms ? GX_GET_REG_FIELD(__GXData->vcdLo, 2, 11) : GX_NONE;
        break;
    }

    case GX_VA_NBT: {
        result =
            __GXData->hasBiNrms ? GX_GET_REG_FIELD(__GXData->vcdLo, 2, 11) : GX_NONE;
        break;
    }

    case GX_VA_CLR0: {
        result = GX_GET_REG_FIELD(__GXData->vcdLo, 2, 13);
        break;
    }

    case GX_VA_CLR1: {
        result = GX_GET_REG_FIELD(__GXData->vcdLo, 2, 15);
        break;
    }

    case GX_VA_TEX0: {
        result = GX_GET_REG_FIELD(__GXData->vcdHi, 2, 0);
        break;
    }

    case GX_VA_TEX1: {
        result = GX_GET_REG_FIELD(__GXData->vcdHi, 2, 2);
        break;
    }

    case GX_VA_TEX2: {
        result = GX_GET_REG_FIELD(__GXData->vcdHi, 2, 4);
        break;
    }

    case GX_VA_TEX3: {
        result = GX_GET_REG_FIELD(__GXData->vcdHi, 2, 6);
        break;
    }

    case GX_VA_TEX4: {
        result = GX_GET_REG_FIELD(__GXData->vcdHi, 2, 8);
        break;
    }

    case GX_VA_TEX5: {
        result = GX_GET_REG_FIELD(__GXData->vcdHi, 2, 10);
        break;
    }

    case GX_VA_TEX6: {
        result = GX_GET_REG_FIELD(__GXData->vcdHi, 2, 12);
        break;
    }

    case GX_VA_TEX7: {
        result = GX_GET_REG_FIELD(__GXData->vcdHi, 2, 14);
        break;
    }

    default: {
        result = GX_NONE;
        break;
    }
    }

    *type = result;
}

void GXGetVtxDescv(GXVtxDescList* list) {
    int i;
    GXAttr attr;
    int* new_var2;

    for (i = 0; i <= GX_VA_TEX7; i++) {
        attr = (GXAttr)i;
        list[i].attr = attr;
        GXGetVtxDesc(attr, &list[i].type);
    }

    list[i].attr = GX_VA_NBT;
    GXGetVtxDesc(GX_VA_NBT, &list[i].type);

    // TODO(kiwi) Fakematch
    attr = 1;
    i = (*(new_var2 = &i)) + attr;
    list[i].attr = GX_VA_NULL;
}

void GXClearVtxDesc(void) {
    __GXData->vcdLo = 0;
    // GX_VA_POS is required for every vertex descriptor
    GX_SET_REG_FIELD(__GXData->vcdLo, 2, 9, GX_DIRECT);
    __GXData->vcdHi = 0;
    __GXData->hasNrms = FALSE;
    __GXData->hasBiNrms = FALSE;
    __GXData->dirtyState |= 8;
}

void GXSetVtxAttrFmt(GXVtxFmt fmt, GXAttr attr, GXCompCnt compCnt,
                     GXCompType compType, u8 shift) {
    u32* vatA;
    u32* vatB;
    u32* vatC;

    vatA = &__GXData->vatA[fmt];
    vatB = &__GXData->vatB[fmt];
    vatC = &__GXData->vatC[fmt];

    SETVAT(vatA, vatB, vatC, attr, compCnt, compType, shift);
    __GXData->dirtyState |= 0x10;
    __GXData->dirtyVAT |= (u8)(1 << (u8)fmt);
}

static inline void SETVAT(u32* vatA, u32* vatB, u32* vatC, GXAttr attr,
                                 GXCompCnt compCnt, GXCompType compType,
                                 u8 shift) {
    switch (attr) {
    case GX_VA_POS: {
        GX_SET_REG_FIELD(*vatA, 1, 0, compCnt);
        GX_SET_REG_FIELD(*vatA, 3, 1, compType);
        GX_SET_REG_FIELD(*vatA, 5, 4, shift);
        break;
    }

    case GX_VA_NRM:
    case GX_VA_NBT: {
        GX_SET_REG_FIELD(*vatA, 3, 10, compType);
        if (compCnt == GX_NRM_NBT3) {
            // Probably because the compCnt can only be one bit?
            GX_SET_REG_FIELD(*vatA, 1, 9, GX_NRM_NBT);
            GX_SET_REG_FIELD(*vatA, 1, 31, TRUE);
        } else {
            GX_SET_REG_FIELD(*vatA, 1, 9, compCnt);
            GX_SET_REG_FIELD(*vatA, 1, 31, FALSE);
        }
        break;
    }

    case GX_VA_CLR0: {
        GX_SET_REG_FIELD(*vatA, 1, 13, compCnt);
        GX_SET_REG_FIELD(*vatA, 3, 14, compType);
        break;
    }

    case GX_VA_CLR1: {
        GX_SET_REG_FIELD(*vatA, 1, 17, compCnt);
        GX_SET_REG_FIELD(*vatA, 3, 18, compType);
        break;
    }

    case GX_VA_TEX0: {
        GX_SET_REG_FIELD(*vatA, 1, 21, compCnt);
        GX_SET_REG_FIELD(*vatA, 3, 22, compType);
        GX_SET_REG_FIELD(*vatA, 5, 25, shift);
        break;
    }

    case GX_VA_TEX1: {
        GX_SET_REG_FIELD(*vatB, 1, 0, compCnt);
        GX_SET_REG_FIELD(*vatB, 3, 1, compType);
        GX_SET_REG_FIELD(*vatB, 5, 4, shift);
        break;
    }

    case GX_VA_TEX2: {
        GX_SET_REG_FIELD(*vatB, 1, 9, compCnt);
        GX_SET_REG_FIELD(*vatB, 3, 10, compType);
        GX_SET_REG_FIELD(*vatB, 5, 13, shift);
        break;
    }

    case GX_VA_TEX3: {
        GX_SET_REG_FIELD(*vatB, 1, 18, compCnt);
        GX_SET_REG_FIELD(*vatB, 3, 19, compType);
        GX_SET_REG_FIELD(*vatB, 5, 22, shift);
        break;
    }

    case GX_VA_TEX4: {
        GX_SET_REG_FIELD(*vatB, 1, 27, compCnt);
        GX_SET_REG_FIELD(*vatB, 3, 28, compType);
        GX_SET_REG_FIELD(*vatC, 5, 0, shift);
        break;
    }

    case GX_VA_TEX5: {
        GX_SET_REG_FIELD(*vatC, 1, 5, compCnt);
        GX_SET_REG_FIELD(*vatC, 3, 6, compType);
        GX_SET_REG_FIELD(*vatC, 5, 9, shift);
        break;
    }

    case GX_VA_TEX6: {
        GX_SET_REG_FIELD(*vatC, 1, 14, compCnt);
        GX_SET_REG_FIELD(*vatC, 3, 15, compType);
        GX_SET_REG_FIELD(*vatC, 5, 18, shift);
        break;
    }

    case GX_VA_TEX7: {
        GX_SET_REG_FIELD(*vatC, 1, 23, compCnt);
        GX_SET_REG_FIELD(*vatC, 3, 24, compType);
        GX_SET_REG_FIELD(*vatC, 5, 27, shift);
        break;
    }

    default: {
        break;
    }
    }
}

void GXSetVtxAttrFmtv(GXVtxFmt fmt, const GXVtxAttrFmtList* list) {
    u32* vatA;
    u32* vatB;
    u32* vatC;

    vatA = &__GXData->vatA[fmt];
    vatB = &__GXData->vatB[fmt];
    vatC = &__GXData->vatC[fmt];

    for (; list->attr != GX_VA_NULL; list++) {
        SETVAT(vatA, vatB, vatC, list->attr, list->cnt, list->type,
               list->frac);
    }

    __GXData->dirtyState |= 0x10;
    __GXData->dirtyVAT |= (u8)(1 << (u8)fmt);
}

void __GXSetVAT(void) {
    int i = 0;
    u32 flags = __GXData->dirtyVAT;

    do {
        if (flags & 1) {
            GX_WRITE_U8(GX_LOAD_CP_REG);
    GX_WRITE_U8(0x70 | i);
    GX_WRITE_U32(__GXData->vatA[i]);
            GX_WRITE_U8(GX_LOAD_CP_REG);
    GX_WRITE_U8(0x80 | i);
    GX_WRITE_U32(__GXData->vatB[i]);
            GX_WRITE_U8(GX_LOAD_CP_REG);
    GX_WRITE_U8(0x90 | i);
    GX_WRITE_U32(__GXData->vatC[i]);
        }

        i++;
        flags >>= 1;
    } while (flags != 0);

    GX_WRITE_U8(GX_NOP);
    __GXData->dirtyVAT = 0;
}

void GXGetVtxAttrFmt(GXVtxFmt fmt, GXAttr attr, GXCompCnt* compCnt,
                     GXCompType* compType, u8* shift) {
    u32* vatA;
    u32* vatB;
    u32* vatC;

    vatA = &__GXData->vatA[fmt];
    vatB = &__GXData->vatB[fmt];
    vatC = &__GXData->vatC[fmt];

    switch (attr) {
    case GX_VA_POS: {
        *compCnt = GX_GET_REG_FIELD(*vatA, 1, 0);
        *compType = GX_GET_REG_FIELD(*vatA, 3, 1);
        *shift = GX_GET_REG_FIELD(*vatA, 5, 4);
        break;
    }

    case GX_VA_NRM:
    case GX_VA_NBT: {
        *compCnt = GX_GET_REG_FIELD(*vatA, 1, 9);

        // Undoing what SETVAT did
        if (*compCnt == GX_NRM_NBT &&
            GX_GET_REG_FIELD(*vatA, 1, 31)) {
            *compCnt = GX_NRM_NBT3;
        }

        *compType = GX_GET_REG_FIELD(*vatA, 3, 10);
        *shift = GetFracForNrm(*compType);
        break;
    }

    case GX_VA_CLR0: {
        *compCnt = GX_GET_REG_FIELD(*vatA, 1, 13);
        *compType = GX_GET_REG_FIELD(*vatA, 3, 14);
        *shift = 0;
        break;
    }

    case GX_VA_CLR1: {
        *compCnt = GX_GET_REG_FIELD(*vatA, 1, 17);
        *compType = GX_GET_REG_FIELD(*vatA, 3, 18);
        *shift = 0;
        break;
    }

    case GX_VA_TEX0: {
        *compCnt = GX_GET_REG_FIELD(*vatA, 1, 21);
        *compType = GX_GET_REG_FIELD(*vatA, 3, 22);
        *shift = GX_GET_REG_FIELD(*vatA, 5, 25);
        break;
    }

    case GX_VA_TEX1: {
        *compCnt = GX_GET_REG_FIELD(*vatB, 1, 0);
        *compType = GX_GET_REG_FIELD(*vatB, 3, 1);
        *shift = GX_GET_REG_FIELD(*vatB, 5, 4);
        break;
    }

    case GX_VA_TEX2: {
        *compCnt = GX_GET_REG_FIELD(*vatB, 1, 9);
        *compType = GX_GET_REG_FIELD(*vatB, 3, 10);
        *shift = GX_GET_REG_FIELD(*vatB, 5, 13);
        break;
    }

    case GX_VA_TEX3: {
        *compCnt = GX_GET_REG_FIELD(*vatB, 1, 18);
        *compType = GX_GET_REG_FIELD(*vatB, 3, 19);
        *shift = GX_GET_REG_FIELD(*vatB, 5, 22);
        break;
    }

    case GX_VA_TEX4: {
        *compCnt = GX_GET_REG_FIELD(*vatB, 1, 27);
        *compType = GX_GET_REG_FIELD(*vatB, 3, 28);
        *shift = GX_GET_REG_FIELD(*vatC, 5, 0);
        break;
    }

    case GX_VA_TEX5: {
        *compCnt = GX_GET_REG_FIELD(*vatC, 1, 5);
        *compType = GX_GET_REG_FIELD(*vatC, 3, 6);
        *shift = GX_GET_REG_FIELD(*vatC, 5, 9);
        break;
    }

    case GX_VA_TEX6: {
        *compCnt = GX_GET_REG_FIELD(*vatC, 1, 14);
        *compType = GX_GET_REG_FIELD(*vatC, 3, 15);
        *shift = GX_GET_REG_FIELD(*vatC, 5, 18);
        break;
    }

    case GX_VA_TEX7: {
        *compCnt = GX_GET_REG_FIELD(*vatC, 1, 23);
        *compType = GX_GET_REG_FIELD(*vatC, 3, 24);
        *shift = GX_GET_REG_FIELD(*vatC, 5, 27);
        break;
    }

    default: {
        *compCnt = GX_POS_XYZ;
        *compType = GX_U8;
        *shift = 0;
        break;
    }
    }
}

static u8 GetFracForNrm(GXCompType compType) {
    switch (compType) {
    case GX_S8: {
        return 6;
    }

    case GX_S16: {
        return 14;
    }

    case GX_U16:
    default: {
        return 0;
    }
    }
}

void GXGetVtxAttrFmtv(GXVtxFmt fmt, GXVtxAttrFmtList* list) {
    int i;
    GXAttr attr;

    for (i = GX_VA_POS; i <= GX_VA_TEX7; i++, list++) {
        attr = (GXAttr)i;
        list->attr = attr;
        GXGetVtxAttrFmt(fmt, attr, &list->cnt, &list->type,
                        &list->frac);
    }

    list->attr = GX_VA_NULL;
}

void GXSetArray(GXAttr attr, void* base_ptr, u8 stride) {
    u32 idx;

    if (attr == GX_VA_NBT) {
        attr = GX_VA_NRM;
    }

    idx = attr - GX_VA_POS;

    GX_WRITE_U8(GX_LOAD_CP_REG);
    GX_WRITE_U8(0xA0 | idx);
    // Address -> offset?
    GX_WRITE_U32((u32)base_ptr & ~0xC0000000);

    GX_WRITE_U8(GX_LOAD_CP_REG);
    GX_WRITE_U8(0xB0 | idx);
    GX_WRITE_U32(stride);
}

void GXInvalidateVtxCache(void) {
    GX_WRITE_U8(0x48);
}

void GXSetTexCoordGen2(GXTexCoordID id, GXTexGenType type, GXTexGenSrc src,
                       u32 texMtxIdx, GXBool normalize, u32 dualTexMtxIdx) {
    u32 reg;
    u32 inputRow;
    u32 inputForm;

    reg = 0;
    inputForm = 0;
    inputRow = 5;

    switch (src) {
    case GX_TG_POS: {
        inputRow = 0;
        inputForm = 1;
        break;
    }

    case GX_TG_NRM: {
        inputRow = 1;
        inputForm = 1;
        break;
    }

    case GX_TG_BINRM: {
        inputRow = 3;
        inputForm = 1;
        break;
    }

    case GX_TG_TANGENT: {
        inputRow = 4;
        inputForm = 1;
        break;
    }

    case GX_TG_COLOR0: {
        inputRow = 2;
        break;
    }

    case GX_TG_COLOR1: {
        inputRow = 2;
        break;
    }

    case GX_TG_TEX0: {
        inputRow = 5;
        break;
    }

    case GX_TG_TEX1: {
        inputRow = 6;
        break;
    }

    case GX_TG_TEX2: {
        inputRow = 7;
        break;
    }

    case GX_TG_TEX3: {
        inputRow = 8;
        break;
    }

    case GX_TG_TEX4: {
        inputRow = 9;
        break;
    }

    case GX_TG_TEX5: {
        inputRow = 10;
        break;
    }

    case GX_TG_TEX6: {
        inputRow = 11;
        break;
    }

    case GX_TG_TEX7: {
        inputRow = 12;
        break;
    }

    default: {
        break;
    }
    }

    switch (type) {
    case GX_TG_MTX2x4: {
        reg = 0;
        GX_SET_REG_FIELD(reg, 1, 1, 0); // 2x4 projection
        GX_SET_REG_FIELD(reg, 1, 2, inputForm);
        GX_SET_REG_FIELD(reg, 5, 7, inputRow);
        break;
    }

    case GX_TG_MTX3x4: {
        reg = 0;
        GX_SET_REG_FIELD(reg, 1, 1, 1); // 3x4 projection
        GX_SET_REG_FIELD(reg, 1, 2, inputForm);
        GX_SET_REG_FIELD(reg, 5, 7, inputRow);
        break;
    }

    case GX_TG_BUMP0:
    case GX_TG_BUMP1:
    case GX_TG_BUMP2:
    case GX_TG_BUMP3:
    case GX_TG_BUMP4:
    case GX_TG_BUMP5:
    case GX_TG_BUMP6:
    case GX_TG_BUMP7: {
        reg = 0;
        GX_SET_REG_FIELD(reg, 1, 2, inputForm);
        GX_SET_REG_FIELD(reg, 3, 4, 1);
        GX_SET_REG_FIELD(reg, 5, 7, inputRow);
        GX_SET_REG_FIELD(reg, 3, 12, src - GX_TG_TEXCOORD0);
        GX_SET_REG_FIELD(reg, 3, 15, type - GX_TG_BUMP0);
        break;
    }

    case GX_TG_SRTG: {
        reg = 0;
        GX_SET_REG_FIELD(reg, 1, 2, inputForm);

        if (src == GX_TG_COLOR0) {
            GX_SET_REG_FIELD(reg, 3, 4, 2);
        } else {
            GX_SET_REG_FIELD(reg, 3, 4, 3);
        }

        GX_SET_REG_FIELD(reg, 5, 7, 2);
        break;
    }

    default: {
        break;
    }
    }

    __GXData->texGenCtrl[id] = reg;
    __GXData->dirtyState |= 0x10000 << id;

    reg = 0;
    GX_SET_REG_FIELD(reg, 6, 0, dualTexMtxIdx - GX_PTTEXMTX0);
    GX_SET_REG_FIELD(reg, 1, 8, normalize);
    __GXData->texGenCtrl2[id] = reg;

    switch (id) {
    case GX_TEXCOORD0: {
        GX_SET_REG_FIELD(__GXData->matIdxA, 6, 6, texMtxIdx);
        break;
    }

    case GX_TEXCOORD1: {
        GX_SET_REG_FIELD(__GXData->matIdxA, 6, 12, texMtxIdx);
        break;
    }

    case GX_TEXCOORD2: {
        GX_SET_REG_FIELD(__GXData->matIdxA, 6, 18, texMtxIdx);
        break;
    }

    case GX_TEXCOORD3: {
        GX_SET_REG_FIELD(__GXData->matIdxA, 6, 24, texMtxIdx);
        break;
    }

    case GX_TEXCOORD4: {
        GX_SET_REG_FIELD(__GXData->matIdxB, 6, 0, texMtxIdx);
        break;
    }

    case GX_TEXCOORD5: {
        GX_SET_REG_FIELD(__GXData->matIdxB, 6, 6, texMtxIdx);
        break;
    }

    case GX_TEXCOORD6: {
        GX_SET_REG_FIELD(__GXData->matIdxB, 6, 12, texMtxIdx);
        break;
    }

    default: {
        GX_SET_REG_FIELD(__GXData->matIdxB, 6, 18, texMtxIdx);
        break;
    }
    }

    __GXData->dirtyState |= 0x4000000;
}

void GXSetNumTexGens(u8 num) {
    GX_SET_REG_FIELD(__GXData->genMode, 4, 0, num);
    __GXData->dirtyState |= 0x2000000;
    __GXData->dirtyState |= 4;
}
