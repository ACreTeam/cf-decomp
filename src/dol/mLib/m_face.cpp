#include <game/mLib/m_face.hpp>
#include <game/mLib/m_allocator.hpp>
#include <revolution/ARC.h>

namespace mFace {

static nw4r::ut::List l_entries;

// 802B39E4
void create() {
    nw4r::ut::List_Init(&l_entries, 0);
}

// 802B39F4
BOOL initRes(void *arc, EGG::Heap *heap, BOOL deluxTex) {
    ARCHandle handle;
    ARCFileInfo file;
    if (!ARCInitHandle(arc, &handle)) {
        return FALSE;
    }
    if (!ARCOpen(&handle, RFLGetResourceFileName(), &file)) {
        return FALSE;
    }
    void *resource = ARCGetStartAddrInMem(&file);
    u32 size = ARCGetLength(&file);
    ARCClose(&file);
    void *buffer = heap->alloc(RFLGetWorkSize(deluxTex), 4);
    if (buffer == nullptr) {
        return FALSE;
    }
    if (RFLInitRes(buffer, resource, size, 0) != RFLErrcode_Success) {
        if (buffer != nullptr) {
            heap->free(buffer);
        }
        return FALSE;
    }
    return TRUE;
}

// 802B3B10
void execute() {
    obj_c *obj = (obj_c *)nw4r::ut::List_GetNext(&l_entries, nullptr);
    if (obj != nullptr) {
        do {
            obj_c *next = (obj_c *)nw4r::ut::List_GetNext(&l_entries, obj);
            obj->init();
            obj->mIsReady = true;
            obj->leave();
            obj = next;
        } while (obj != nullptr);
    }
}

// 802B3BA8
void obj_c::entry() {
    mIsReady = false;
    nw4r::ut::List_Append(&l_entries, this);
}

// 802B3BC0
void obj_c::leave() {
    nw4r::ut::List_Remove(&l_entries, this);
}

// 802B3BD0
BOOL isValidData(RFLDataSource source, u16 index, RFLMiddleDB *middleDB) {
    RFLCharInfo info;
    if (source == RFLDataSource_Official) {
        if (!RFLIsAvailableOfficialData(index)) {
            return FALSE;
        }
    } else if (source == RFLDataSource_MiddleDB) {
        if (RFLGetAdditionalInfo(&info, source, middleDB, index) != RFLErrcode_Success) {
            return FALSE;
        }
    } else {
        return FALSE;
    }
    return TRUE;
}

// 802B3C40
void model_c::init() {
    RFLInitCharModel(this, mSource, mpMiddleDB, mIndex, mpBuffer, mResolution, mExpressionFlag);
}

// 802B3C74
bool model_c::create(RFLDataSource source, u16 index, RFLResolution resolution, u32 expressionFlag,
                     mAllocator_c *allocator, RFLMiddleDB *middleDB) {
    if (!isValidData(source, index, middleDB)) {
        return false;
    }
    mpBuffer = allocator->alloc(RFLGetModelBufferSize(resolution, expressionFlag));
    if (mpBuffer == nullptr) {
        return false;
    }
    entry(source, index, middleDB);
    mResolution = resolution;
    mExpressionFlag = expressionFlag;
    mpAllocator = allocator;
    return true;
}

// 802B3D38
void model_c::remove() {
    if (mpBuffer != nullptr) {
        mpAllocator->free(mpBuffer);
        mpBuffer = nullptr;
        mpAllocator = nullptr;
    }
}

// 802B3D8C
void model_c::entry(RFLDataSource source, u16 index, RFLMiddleDB *middleDB) {
    mSource = source;
    mIndex = index;
    mpMiddleDB = middleDB;
    obj_c::entry();
}

// 802B3D9C
bool model_c::change(RFLDataSource source, u16 index, RFLMiddleDB *middleDB) {
    if (!isValidData(source, index, middleDB)) {
        return false;
    }
    entry(source, index, middleDB);
    return true;
}

// 802B3E20
void model_c::setMtx(const mMtx_c *mtx) {
    RFLSetMtx(this, mtx->m);
}

// 802B3E30
void model_c::setExpression(RFLExpression expression) {
    RFLSetExpression(this, expression);
}

} // namespace mFace
