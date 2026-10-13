#pragma once

// Process and persistent grid classes implemented by d_field.cpp.

#include <game/framework/f_base.hpp>
#include <game/game/d_field_info.hpp>

// The field view of dScData::SceneData_c's extra word. The scene tables
// currently store this as ExitInfo_c (a block-id list and packed dimensions).
struct dFieldSceneMap_c {
    /* 0x00 */ const dFdBlockId_c *mBlockIds;
    /* 0x04 */ u8 mBlockW;
    /* 0x05 */ u8 mBlockH;
}; // size 0x8

struct dFieldSceneData_c {
    /* 0x00 */ dScene::dSceneData_c mData;
    /* 0x0C */ dFieldSceneMap_c *mField;
}; // size 0x10

class dFdInfoSv_c : public dFdBase_c {
public:
    BOOL createTown(EGG::Heap *heap); // 8008DD70 (d_field_info.cpp)
    BOOL updateTown();               // 8008DEF4 (d_field_info.cpp)
    virtual int getKind(); // 8019145C
    virtual ~dFdInfoSv_c(); // 80191464
};

class dFdInfoSvHs_c : public dFdBase_c {
public:
    virtual int getKind(); // 80191414
    virtual ~dFdInfoSvHs_c(); // 8019141C
};

class dField_c : public fBase_c {
public:
    static fBase_c *construct(); // 80190BE4
    virtual int create();       // 80190D40
    virtual int doDelete();     // 8019102C
    virtual int execute();      // 8019111C
    virtual int draw();         // 80191124
    virtual ~dField_c();        // 801914A4
}; // size 0x64

