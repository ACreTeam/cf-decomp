#include <private/gx.h>
#include <revolution/gx.h>
#include <revolution/gx/GXRegs.h>
#include <revolution/mtx.h>

// clang-format off
inline void LoadProjPS(register f32* dst) {
    register f32 ps_0, ps_1, ps_2;
    register GXData* src;

    asm volatile {
        lwz src, __GXData
        psq_l  ps_0,  0  + GXData.projMtx(src), 0, 0
        psq_l  ps_1,  8  + GXData.projMtx(src), 0, 0
        psq_l  ps_2,  16 + GXData.projMtx(src), 0, 0
        psq_st ps_0,  0(dst),                0, 0
        psq_st ps_1,  8(dst),                0, 0
        psq_st ps_2, 16(dst),                0, 0
    }
}
// clang-format on

// clang-format off
inline void WriteProjPS(register volatile void* dst, register const f32* src) {
    register f32 ps_0, ps_1, ps_2;

    asm volatile {
        psq_l  ps_0,  0(src), 0, 0
        psq_l  ps_1,  8(src), 0, 0
        psq_l  ps_2, 16(src), 0, 0
        psq_st ps_0,  0(dst), 0, 0
        psq_st ps_1,  0(dst), 0, 0
        psq_st ps_2,  0(dst), 0, 0
    }
}
// clang-format on

// clang-format off
inline void Copy6Floats(register f32* dst, register const f32* src) {
    register f32 ps_0, ps_1, ps_2;

    asm volatile {
        psq_l  ps_0,  0(src), 0, 0
        psq_l  ps_1,  8(src), 0, 0
        psq_l  ps_2, 16(src), 0, 0
        psq_st ps_0,  0(dst), 0, 0
        psq_st ps_1,  8(dst), 0, 0
        psq_st ps_2, 16(dst), 0, 0
    }
}
// clang-format on

void __GXSetProjection(void) {
    // The write-gather pipe is memory mapped at 0xCC008000; holding its
    // address once keeps the projection store and the type store on the
    // same pointer.
    volatile void* wgpipe = &GXWGFifo;

    {
        u32 cmd = 0;
        cmd |= (0x1020);
        cmd |= (GX_PROJECTION_SZ - 1) << 16;
        GX_WRITE_U8(0x10);
        GX_WRITE_U32(cmd);
    }
    WriteProjPS(wgpipe, __GXData->projMtx);
    GX_WRITE_U32(__GXData->projType);
}

void GXSetProjection(const Mtx44 proj, GXProjectionType type) {
    __GXData->projType = type;

    __GXData->projMtx[0] = proj[0][0];
    __GXData->projMtx[2] = proj[1][1];
    __GXData->projMtx[4] = proj[2][2];
    __GXData->projMtx[5] = proj[2][3];

    if (type == GX_ORTHOGRAPHIC) {
        __GXData->projMtx[1] = proj[0][3];
        __GXData->projMtx[3] = proj[1][3];
    } else {
        __GXData->projMtx[1] = proj[0][2];
        __GXData->projMtx[3] = proj[1][2];
    }

    __GXData->dirtyState |= 0x08000000;
}

void GXSetProjectionv(const f32 proj[GX_PROJECTION_SZ]) {
    __GXData->projType = proj[0] == 0.0f ? GX_PERSPECTIVE : GX_ORTHOGRAPHIC;
    Copy6Floats(__GXData->projMtx, proj + 1);
    __GXData->dirtyState |= 0x08000000;
}

void GXGetProjectionv(f32 proj[GX_PROJECTION_SZ]) {
    proj[0] = __GXData->projType != GX_PERSPECTIVE ? 1.0f : 0.0f;
    LoadProjPS(proj + 1);
}

// clang-format off
inline void WriteMTXPS4x3(register volatile void* dst, register const Mtx src) {
    register f32 ps_0, ps_1, ps_2, ps_3, ps_4, ps_5;

    asm volatile {
        psq_l  ps_0,  0(src), 0, 0
        psq_l  ps_1,  8(src), 0, 0
        psq_l  ps_2, 16(src), 0, 0
        psq_l  ps_3, 24(src), 0, 0
        psq_l  ps_4, 32(src), 0, 0
        psq_l  ps_5, 40(src), 0, 0

        psq_st ps_0, 0(dst),  0, 0
        psq_st ps_1, 0(dst),  0, 0
        psq_st ps_2, 0(dst),  0, 0
        psq_st ps_3, 0(dst),  0, 0
        psq_st ps_4, 0(dst),  0, 0
        psq_st ps_5, 0(dst),  0, 0
    }
}
// clang-format on

// clang-format off
inline void WriteMTXPS3x3(register volatile void* dst, register const Mtx src) {
    register f32 ps_0, ps_1, ps_2, ps_3, ps_4, ps_5;

    asm volatile {
        psq_l  ps_0,  0(src), 0, 0
        lfs    ps_1,  8(src)
        psq_l  ps_2, 16(src), 0, 0
        lfs    ps_3, 24(src)
        psq_l  ps_4, 32(src), 0, 0
        lfs    ps_5, 40(src)

        psq_st ps_0, 0(dst),  0, 0
        stfs   ps_1, 0(dst)
        psq_st ps_2, 0(dst),  0, 0
        stfs   ps_3, 0(dst)
        psq_st ps_4, 0(dst),  0, 0
        stfs   ps_5, 0(dst)
    }
}
// clang-format on

// clang-format off
inline void WriteMTXPS4x2(register volatile void* dst, register const Mtx src) {
    register f32 ps_0, ps_1, ps_2, ps_3;

    asm volatile {
        psq_l  ps_0,  0(src), 0, 0
        psq_l  ps_1,  8(src), 0, 0
        psq_l  ps_2, 16(src), 0, 0
        psq_l  ps_3, 24(src), 0, 0

        psq_st ps_0, 0(dst),  0, 0
        psq_st ps_1, 0(dst),  0, 0
        psq_st ps_2, 0(dst),  0, 0
        psq_st ps_3, 0(dst),  0, 0
    }
}
// clang-format on

void GXLoadPosMtxImm(const Mtx mtx, u32 id) {
    // Position matrices are 4x3
    {
        u32 cmd = 0;
        cmd |= (id * 4);
        cmd |= (4 * 3 - 1) << 16;
        GX_WRITE_U8(0x10);
        GX_WRITE_U32(cmd);
    }
    WriteMTXPS4x3(&GXWGFifo, mtx);
}

void GXLoadPosMtxIndx(u16 index, u32 id) {
    // Position matrices are 4x3
    u32 cmd = 0;
    GX_SET_REG_FIELD(cmd, 12, 0, id * 4);
    GX_SET_REG_FIELD(cmd, 4, 12, 4 * 3 - 1);
    GX_SET_REG_FIELD(cmd, 16, 16, index);
    GX_WRITE_U8(0x20);
    GX_WRITE_U32(cmd);
}

void GXLoadNrmMtxImm(const Mtx mtx, u32 id) {
    // Normal matrices are 3x3
    {
        u32 cmd = 0;
        cmd |= (id * 3 + 0x400);
        cmd |= (3 * 3 - 1) << 16;
        GX_WRITE_U8(0x10);
        GX_WRITE_U32(cmd);
    }
    WriteMTXPS3x3(&GXWGFifo, mtx);
}

void GXLoadNrmMtxIndx3x3(u16 index, u32 id) {
    // Normal matrices are 3x3
    u32 cmd = 0;
    GX_SET_REG_FIELD(cmd, 12, 0, id * 3 + 0x400);
    GX_SET_REG_FIELD(cmd, 4, 12, 3 * 3 - 1);
    GX_SET_REG_FIELD(cmd, 16, 16, index);
    GX_WRITE_U8(0x28);
    GX_WRITE_U32(cmd);
}

void GXSetCurrentMtx(u32 id) {
    GX_SET_REG_FIELD(__GXData->matIdxA, 6, 0, id);
    __GXData->dirtyState |= 0x04000000;
}

void GXLoadTexMtxImm(const Mtx mtx, u32 id, GXTexMtxType type) {
    u32 addr;
    u32 num;
    u32 cmd;

    // Base row address in XF memory
    addr = id >= GX_PTTEXMTX0
               ? (id - GX_PTTEXMTX0) * sizeof(f32) + 0x500
               : id * 4 + (u64)0;

    // Number of elements in matrix, as an XF load-register size field
    num = type == GX_MTX2x4 ? (u64)(2 * 4) : 3 * 4;
    num = (num - 1) << 16;

    cmd = addr | num;
    GX_WRITE_U8(0x10);
    GX_WRITE_U32(cmd);

    if (type == GX_MTX3x4) {
        WriteMTXPS4x3(&GXWGFifo, mtx);
    } else {
        WriteMTXPS4x2(&GXWGFifo, mtx);
    }
}

void __GXSetViewport(void) {
    f32 a, b, c, d, e, f;
    f32 near, far;

    a = __GXData->vpWd / 2.0f;
    b = -__GXData->vpHt / 2.0f;
    d = __GXData->vpLeft + (__GXData->vpWd / 2.0f) + 342.0f;
    e = __GXData->vpTop + (__GXData->vpHt / 2.0f) + 342.0f;

    near = __GXData->vpNearz * __GXData->zScale;
    far = __GXData->vpFarz * __GXData->zScale;

    c = far - near;
    f = far + __GXData->zOffset;

    {
        u32 cmd = 0;
        cmd |= (0x101A);
        cmd |= (6 - 1) << 16;
        GX_WRITE_U8(0x10);
        GX_WRITE_U32(cmd);
    }
    GX_WRITE_F32(a);
    GX_WRITE_F32(b);
    GX_WRITE_F32(c);
    GX_WRITE_F32(d);
    GX_WRITE_F32(e);
    GX_WRITE_F32(f);
}

void GXSetViewportJitter(f32 ox, f32 oy, f32 sx, f32 sy, f32 near, f32 far,
                         u32 nextField) {
    if (nextField == GX_FIELD_EVEN) {
        oy -= 0.5f;
    }

    __GXData->vpLeft = ox;
    __GXData->vpTop = oy;
    __GXData->vpWd = sx;
    __GXData->vpHt = sy;
    __GXData->vpNearz = near;
    __GXData->vpFarz = far;
    __GXData->dirtyState |= 0x10000000;
}

void GXSetViewport(f32 ox, f32 oy, f32 sx, f32 sy, f32 near, f32 far) {
    __GXData->vpLeft = ox;
    __GXData->vpTop = oy;
    __GXData->vpWd = sx;
    __GXData->vpHt = sy;
    __GXData->vpNearz = near;
    __GXData->vpFarz = far;
    __GXData->dirtyState |= 0x10000000;
}

void GXGetViewportv(f32 view[GX_VIEWPORT_SZ]) {
    Copy6Floats(view, &__GXData->vpLeft);
}

void GXSetZScaleOffset(f32 scale, f32 offset) {
    __GXData->zOffset = (f32)0xFFFFFF * offset;      // ???
    __GXData->zScale = 1.0f + (f32)0xFFFFFF * scale; // ???
    __GXData->dirtyState |= 0x10000000;
}

void GXSetScissor(u32 x, u32 y, u32 w, u32 h) {
    u32 x1, y1, x2, y2;
    u32 reg;

    x1 = x + 342;
    y1 = y + 342;
    x2 = x1 + w - 1;
    y2 = y1 + h - 1;

    reg = __GXData->suScis0;
    SC_SU_SCIS0_SET_SY0(reg, y1);
    SC_SU_SCIS0_SET_SX0(reg, x1);
    __GXData->suScis0 = reg;

    reg = __GXData->suScis1;
    SC_SU_SCIS1_SET_SY1(reg, y2);
    SC_SU_SCIS1_SET_SX1(reg, x2);
    __GXData->suScis1 = reg;

    GX_WRITE_RAS_REG(__GXData->suScis0);
    GX_WRITE_RAS_REG(__GXData->suScis1);
    __GXData->bpSentNot = FALSE;
}

void GXGetScissor(u32* x, u32* y, u32* w, u32* h) {
    u32 y2, y1;
    u32 x2, x1;

    x1 = SU_SCIS0_GET_SX0(__GXData->suScis0);
    y1 = SU_SCIS0_GET_SY0(__GXData->suScis0);
    x2 = SU_SCIS1_GET_SX1(__GXData->suScis1);
    y2 = SU_SCIS1_GET_SY1(__GXData->suScis1);

    *x = x1 - 342;
    *y = y1 - 342;
    *w = x2 - x1 + 1;
    *h = y2 - y1 + 1;
}

void GXSetScissorBoxOffset(u32 ox, u32 oy) {
    u32 cmd = 0;
    GX_SET_REG_FIELD(cmd, 10, 0, (ox + 342) / 2);
    GX_SET_REG_FIELD(cmd, 10, 10, (oy + 342) / 2);
    GX_SET_REG_FIELD(cmd, 8, 24, 0x59);

    GX_WRITE_RAS_REG(cmd);
    __GXData->bpSentNot = FALSE;
}

void GXSetClipMode(GXClipMode mode) {
    GX_WRITE_XF_REG(0x1005, mode);
    __GXData->bpSentNot = TRUE;
}

void __GXSetMatrixIndex(GXAttr index) {
    // Tex4 and after is stored in XF MatrixIndex1
    if (index < GX_VA_TEX4MTXIDX) {
        GX_WRITE_SOME_REG2(8, 0x30, __GXData->matIdxA, 0);
        GX_WRITE_XF_REG(0x1018, __GXData->matIdxA);
    } else {
        GX_WRITE_SOME_REG2(8, 0x40, __GXData->matIdxB, 0);
        GX_WRITE_XF_REG(0x1019, __GXData->matIdxB);
    }

    __GXData->bpSentNot = TRUE;
}
