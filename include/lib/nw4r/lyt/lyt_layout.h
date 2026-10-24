#ifndef NW4R_LYT_LAYOUT_H
#define NW4R_LYT_LAYOUT_H
#include <nw4r/types_nw4r.h>

#include <nw4r/lyt/lyt_animation.h>
#include <nw4r/lyt/lyt_types.h>

#include <nw4r/ut.h>

#include <revolution/mem.h>

namespace nw4r {
namespace lyt {

// Forward declarations
class ArcResourceAccessor;
class DrawInfo;
class GroupContainer;
class Pane;

/******************************************************************************
 *
 * OriginType
 *
 ******************************************************************************/
enum OriginType {
    ORIGINTYPE_TOPLEFT,
    ORIGINTYPE_CENTER,

    ORIGINTYPE_MAX
};

namespace res {

/******************************************************************************
 *
 * LYT1 binary layout
 *
 ******************************************************************************/
struct Layout {
    static const ulong SIGNATURE = 'lyt1';

    DataBlockHeader blockHeader; // at 0x0
    u8 originType;               // at 0x8
    u8 PADDING_0x9[0xC - 0x9];   // at 0x9
    Size layoutSize;             // at 0xC
};

} // namespace res

/******************************************************************************
 *
 * Layout
 *
 ******************************************************************************/
class Layout {
public:
    static const ulong SIGNATURE = 'RLYT';
    static const ulong SIGNATURE_ANIMATION = 'RLAN';

public:
    Layout();
    virtual ~Layout(); // at 0x8

    virtual bool Build(const void* pLytBinary,
                       ResourceAccessor* pAccessor); // at 0xC

    virtual AnimTransform* CreateAnimTransform(); // at 0x10
    virtual AnimTransform*
    CreateAnimTransform(const void* pAnmBinary,
                        ResourceAccessor* pAccessor); // at 0x14
    virtual AnimTransform*
    CreateAnimTransform(const nw4r::lyt::AnimResource&,
                        nw4r::lyt::ResourceAccessor*); // at 0x18

    virtual void BindAnimation(AnimTransform* pAnimTrans);   // at 0x1C
    virtual void UnbindAnimation(AnimTransform* pAnimTrans); // at 0x20
    virtual void UnbindAllAnimation();                       // at 0x24
    virtual bool BindAnimationAuto(const nw4r::lyt::AnimResource&,
                                   nw4r::lyt::ResourceAccessor*); // at 0x28
    virtual void SetAnimationEnable(AnimTransform* pAnimTrans,
                                    bool enable); // at 0x2C

    virtual void CalculateMtx(const DrawInfo& rInfo); // at 0x30
    virtual void Draw(const DrawInfo& rInfo);         // at 0x34
    virtual void Animate(ulong option);               // at 0x38

    virtual void SetTagProcessor(ut::WideTagProcessor* pProcessor); // at 0x3C

    ut::Rect GetLayoutRect() const;

    Pane* GetRootPane() const {
        return mpRootPane;
    }

    GroupContainer* GetGroupContainer() const {
        return mpGroupContainer;
    }

    static MEMAllocator* GetAllocator() {
        return mspAllocator;
    }
    static void SetAllocator(MEMAllocator* pAllocator) {
        mspAllocator = pAllocator;
    }

    static void* AllocMemory(ulong size) {
        return MEMAllocFromAllocator(mspAllocator, size);
    }
    static void FreeMemory(void* pBlock) {
        MEMFreeToAllocator(mspAllocator, pBlock);
    }

    template <typename T> static T* NewObj();
    template <typename T, typename P1> static T* NewObj(P1 param1);
    template <typename T, typename P1, typename P2>
    static T* NewObj(P1 param1, P2 param2);

    template <typename T> static T* NewArray(ulong num);

    template <typename T> static void DeleteObj(T* pObj);
    template <typename T> static void DeleteArray(T objAry[], ulong num);
    template <typename T> static void DeletePrimArray(T objAry[]);

protected:
    static const ulong SIGNATURE_TEXTURELIST = 'txl1';
    static const ulong SIGNATURE_FONTLIST = 'fnl1';
    static const ulong SIGNATURE_MATERIALLIST = 'mat1';

    static const ulong SIGNATURE_ANIMATIONINFO = 'pai1';

    static const ulong SIGNATURE_USERDATA = 'usd1';

    static const ulong SIGNATURE_PANESTART = 'pas1';
    static const ulong SIGNATURE_PANEEND = 'pae1';

    static const ulong SIGNATURE_GROUPSTART = 'grs1';
    static const ulong SIGNATURE_GROUPEND = 'gre1';

protected:
    static Pane* BuildPaneObj(s32 kind, const void* pBinary,
                              const ResBlockSet& rBlockSet) DECOMP_DONT_INLINE;

protected:
    AnimTransformList mAnimTransList; // at 0x4
    Pane* mpRootPane;                 // at 0x10
    GroupContainer* mpGroupContainer; // at 0x14
    Size mLayoutSize;                 // at 0x18

    static MEMAllocator* mspAllocator;
};

template <typename T> T* Layout::NewObj() {
    if (void* pMem = AllocMemory(sizeof(T))) {
        return new (pMem) T();
    }
    return NULL;
}

template <typename T, typename P1> T* Layout::NewObj(P1 param1) {
    if (void* pMem = AllocMemory(sizeof(T))) {
        return new (pMem) T(param1);
    }
    return NULL;
}

template <typename T, typename P1, typename P2>
T* Layout::NewObj(P1 param1, P2 param2) {
    if (void* pMem = AllocMemory(sizeof(T))) {
        return new (pMem) T(param1, param2);
    }
    return NULL;
}

template <typename T> T* Layout::NewArray(ulong num) {
    void* pMem = AllocMemory(sizeof(T) * num);
    if (!pMem) {
        return NULL;
    }

    T* const objAry = static_cast<T*>(pMem);
    for (ulong i = 0; i < num; i++) {
        new (&objAry[i]) T();
    }

    return objAry;
}

template <typename T> void Layout::DeleteObj(T* pObj) {
    if (pObj != NULL) {
        pObj->~T();
        FreeMemory(pObj);
    }
}

template <typename T> void Layout::DeleteArray(T objAry[], ulong num) {
    if (objAry != NULL) {
        for (ulong i = 0; i < num; i++) {
            objAry[i].~T();
        }
        FreeMemory(objAry);
    }
}

template <typename T> void Layout::DeletePrimArray(T objAry[]) {
    if (objAry != NULL) {
        FreeMemory(objAry);
    }
}

} // namespace lyt
} // namespace nw4r

#endif
