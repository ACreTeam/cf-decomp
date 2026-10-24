#ifndef NW4R_G3D_RES_RES_VTX_H
#define NW4R_G3D_RES_RES_VTX_H
#include <nw4r/types_nw4r.h>

#include <nw4r/g3d/res/g3d_rescommon.h>

#include <nw4r/math.h>

#include <revolution/gx.h>

namespace nw4r {
namespace g3d {

/******************************************************************************
 *
 * ResVtxPos
 *
 ******************************************************************************/
struct ResVtxPosData {
    ulong size;          // at 0x0
    s32 toResMdlData;  // at 0x4
    s32 toVtxPosArray; // at 0x8
    s32 name;          // at 0xC
    ulong id;            // at 0x10
    GXCompCnt cmpcnt;  // at 0x14
    GXCompType tp;     // at 0x18
    u8 frac;           // at 0x1C
    u8 stride;         // at 0x1D
    u16 numPos;        // at 0x1E
    math::_VEC3 min;   // at 0x20
    math::_VEC3 max;   // at 0x2C
};

class ResVtxPos : public ResCommon<ResVtxPosData> {
public:
    NW4R_G3D_RESOURCE_FUNC_DEF(ResVtxPos);

    void Init() {
        DCStore(false);
    }

    void SetArray();
    void GetArray(const void** ppBase, u8* pStride) const;

    void CopyTo(void* pDst) const;
    void DCStore(bool sync);

    ulong GetSize() const {
        return ref().size;
    }

    void* GetData() {
        ResVtxPosData &r = ref();
        return r.toVtxPosArray != 0
                   ? reinterpret_cast<u8 *>(&r) + r.toVtxPosArray
                   : NULL;
    }
    const void* GetData() const {
        const ResVtxPosData &r = ref();
        return r.toVtxPosArray != 0
                   ? reinterpret_cast<const u8 *>(&r) + r.toVtxPosArray
                   : NULL;
    }

    ulong GetID() const {
        return ref().id;
    }

    u16 GetNumVtxPos() const {
        return ref().numPos;
    }
};

/******************************************************************************
 *
 * ResVtxNrm
 *
 ******************************************************************************/
struct ResVtxNrmData {
    ulong size;          // at 0x0
    s32 toResMdlData;  // at 0x4
    s32 toVtxNrmArray; // at 0x8
    s32 name;          // at 0xC
    ulong id;            // at 0x10
    GXCompCnt cmpcnt;  // at 0x14
    GXCompType tp;     // at 0x18
    u8 frac;           // at 0x1C
    u8 stride;         // at 0x1D
    u16 numNrm;        // at 0x1E
};

class ResVtxNrm : public ResCommon<ResVtxNrmData> {
public:
    NW4R_G3D_RESOURCE_FUNC_DEF(ResVtxNrm);

    void Init() {
        DCStore(false);
    }

    void SetArray();
    void GetArray(const void** ppBase, u8* pStride) const;

    void CopyTo(void* pDst) const;
    void DCStore(bool);

    ulong GetSize() const {
        return ref().size;
    }

    void* GetData() {
        return ofs_to_ptr<void>(ref().toVtxNrmArray);
    }
    const void* GetData() const {
        return ofs_to_ptr<void>(ref().toVtxNrmArray);
    }

    ulong GetID() const {
        return ref().id;
    }

    u16 GetNumVtxNrm() const {
        return ref().numNrm;
    }
};

/******************************************************************************
 *
 * ResVtxClr
 *
 ******************************************************************************/
struct ResVtxClrData {
    ulong size;          // at 0x0
    s32 toResMdlData;  // at 0x4
    s32 toVtxClrArray; // at 0x8
    s32 name;          // at 0xC
    ulong id;            // at 0x10
    GXCompCnt cmpcnt;  // at 0x14
    GXCompType tp;     // at 0x18
    u8 stride;         // at 0x1C
    u8 PADDING_0x1D;   // at 0x1D
    u16 numClr;        // at 0x1E
};

class ResVtxClr : public ResCommon<ResVtxClrData> {
public:
    NW4R_G3D_RESOURCE_FUNC_DEF(ResVtxClr);

    void Init() {
        DCStore(false);
    }

    void SetArray(GXAttr attr);
    void GetArray(const void** ppBase, u8* pStride) const;

    void CopyTo(void* pDst) const;
    void DCStore(bool sync);

    ulong GetSize() const {
        return ref().size;
    }

    void* GetData() {
        return ofs_to_ptr<void>(ref().toVtxClrArray);
    }
    const void* GetData() const {
        return ofs_to_ptr<void>(ref().toVtxClrArray);
    }

    ulong GetID() const {
        return ref().id;
    }

    u16 GetNumVtxClr() const {
        return ref().numClr;
    }
};

/******************************************************************************
 *
 * ResVtxTexCoord
 *
 ******************************************************************************/
struct ResVtxTexCoordData {
    ulong size;            // at 0x0
    s32 toResMdlData;    // at 0x4
    s32 toTexCoordArray; // at 0x8
    s32 name;            // at 0xC
    ulong id;              // at 0x10
    GXCompCnt cmpcnt;    // at 0x14
    GXCompType tp;       // at 0x18
    u8 frac;             // at 0x1C
    u8 stride;           // at 0x1D
    u16 numTexCoord;     // at 0x1E
    math::_VEC2 min;     // at 0x20
    math::_VEC2 max;     // at 0x2C
};

class ResVtxTexCoord : public ResCommon<ResVtxTexCoordData> {
public:
    NW4R_G3D_RESOURCE_FUNC_DEF(ResVtxTexCoord);

    void Init() {
        DCStore(false);
    }

    void GetArray(const void** ppBase, u8* pStride) const;
    void DCStore(bool sync);

    ulong GetSize() const {
        return ref().size;
    }

    void* GetData() {
        return ofs_to_ptr<void>(ref().toTexCoordArray);
    }
    const void* GetData() const {
        return ofs_to_ptr<void>(ref().toTexCoordArray);
    }

    ulong GetID() const {
        return ref().id;
    }

    u16 GetNumTexCoord() const {
        return ref().numTexCoord;
    }
};

struct ResVtxFurVecData {
    ulong size;          // at 0x0
    s32 toResMdlData;    // at 0x4
    s32 toFurVecArray;   // at 0x8
    s32 name;            // at 0xC
    ulong id;            // at 0x10
    u16 numFurVec;       // at 0x14
};

class ResVtxFurVec : public ResCommon<ResVtxFurVecData> {
public:
    NW4R_G3D_RESOURCE_FUNC_DEF(ResVtxFurVec);

    void Init() {
        DCStore(false);
    }

    void DCStore(bool sync);

    ulong GetSize() const {
        return ref().size;
    }
};

struct ResVtxFurPosData {
    ulong size;          // at 0x0
    s32 toResMdlData;    // at 0x4
    s32 toFurPosArray;   // at 0x8
    s32 name;            // at 0xC
    ulong id;            // at 0x10
    GXCompCnt cmpcnt;    // at 0x14
    GXCompType tp;       // at 0x18
    u8 frac;             // at 0x1C
    u8 stride;           // at 0x1D
    u16 numFurPos;       // at 0x1E
    ulong numLayer;      // at 0x20
    ulong ofsLayer;      // at 0x24
};

class ResVtxFurPos : public ResCommon<ResVtxFurPosData> {
public:
    NW4R_G3D_RESOURCE_FUNC_DEF(ResVtxFurPos);

    void Init() {
        DCStore(false);
    }

    void SetArray(u16 idx);
    void DCStore(bool sync);

    ulong GetSize() const {
        return ref().size;
    }

    ulong GetNumLayer() const {
        return ref().numLayer;
    }

    void* GetData(int idx) {
        ResVtxFurPosData& r = ref();

        return (r.toFurPosArray != 0 &&
                idx < static_cast<int>(GetNumLayer()) && idx >= 0)
                   ? reinterpret_cast<u8*>(&r) + r.toFurPosArray +
                         idx * r.ofsLayer
                   : NULL;
    }
};

} // namespace g3d
} // namespace nw4r

#endif
