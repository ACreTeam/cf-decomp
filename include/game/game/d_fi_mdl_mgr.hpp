#pragma once

// d_fi_mdl_mgr (DOL TU d_fi_mdl_mgr.cpp, .text 8018FE10..80190BE4): dFIMdlMgr_c (named by its RTTI), the
// models of the fish and insects the players look at: the one held up after a catch (d_fish_field,
// d_hand_item) and the encyclopedia's (d_menu_insc). One slot per player: a dFIMdlBank_c loading the .brres
// into the player's own frame heap ("dFIMdlMgr_c::m_ResHeap"), and a dFIObj_c built from it. Class names are
// the game's (RTTI); everything else is inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/mLib/m_3d.hpp> // before d_dvd.hpp: sets the weak data order
#include <game/game/d_dvd.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/egg/core/eggFrmHeap.h>

namespace dItem { struct Item; }

// The .brres of one fish or insect (dFIMdlMgr_c::mBanks).
class dFIMdlBank_c : public dDvd::brresBank_c {
public:
    // What is loaded (mType).
    enum Type_e {
        TYPE_INSECT_MENU = 0, // an insect's menu model, "/Insect/m_%s.brres"
        TYPE_INSECT = 1,      // "/Insect/%s.brres"
        TYPE_FISH = 2,        // "/Fish/%s.brres" (the frog's book model is "/Fish/m_%s.brres")
    };

    // How the model is shown (mMode): picks the "book" or "get" animations.
    enum Mode_e {
        MODE_BOOK = 0, // the encyclopedia: its lights, posed from the display tables (dFIObj_c::calcBook)
        MODE_GET = 1,  // held up after the catch
    };

    // mState.
    enum State_e {
        STATE_NONE = 0,      // free
        STATE_LOADING = 1,   // requested: loads, then builds the model if mMakeMdl
        STATE_LOADED = 2,    // loaded, no model
        STATE_MDL = 3,       // loaded with its model (dFIMdlMgr_c::getObj)
        STATE_REMOVE = 4,    // released: the model is removed, then unloaded
        STATE_UNLOADING = 5, // released: unloads
    };

    dFIMdlBank_c() : mKind(-1), mType(-1), mState(STATE_NONE), mMakeMdl(false) {}
    virtual ~dFIMdlBank_c() {}

    virtual void freeData(); // 8018FFAC: frees mpResHeap's allocations

    void setHeap(EGG::FrmHeap *heap); // 8018FE40
    BOOL load();                      // 8018FE48: TRUE once loaded, FALSE while loading or for a bad type

    /* 0x58 */ EGG::FrmHeap *mpResHeap;
    /* 0x5C */ int mKind;  // dFishType_e / dInsectType_e
    /* 0x60 */ int mType;  // Type_e
    /* 0x64 */ int mState; // State_e
    /* 0x68 */ int mMode;  // Mode_e
    /* 0x6C */ u8 mMakeMdl;
}; // size 0x70

// The model of a loaded dFIMdlBank_c (dFIMdlMgr_c::mObjs).
class dFIObj_c {
public:
    enum { BOOK_LIGHT_SET = 1 }; // the light set / env map camera of the book models (dBookLight_c)

    // A display table entry (calcBook).
    struct Disp_c {
        /* 0x00 */ Vec mPos;
        /* 0x0C */ Vec mRot; // degrees
    }; // size 0x18

    dFIObj_c() {}
    virtual ~dFIObj_c() {}

    void create(dFIMdlBank_c *bank, int mode); // 8018FFD4: model and "get" / "book" animations
    void remove();                             // 801901C0
    void setAnm(const char *name);             // 8019022C: the animations of this name (d_fish_field "swim")
    void calcBook();                           // 80190304: d_menu_insc: posed from the display tables

    /* 0x04 */ dFIMdlBank_c *mpBank;
    /* 0x08 */ m3d::smdl_c mMdl;
    /* 0x14 */ m3d::anmChr_c mAnmChr;
    /* 0x4C */ m3d::anmTexSrt_c mAnmTexSrt;
    /* 0x78 */ mAllocator_c mAllocator;
}; // size 0x94

class dFIMdlMgr_c : public fBase_c {
public:
    enum {
        OBJ_NUM = 4,            // one per player
        RES_HEAP_SIZE = 0x6800, // each slot's frame heap
    };

    dFIMdlMgr_c(); // 801904F4
    virtual ~dFIMdlMgr_c(); // 8019057C
    virtual int create();   // 8019060C
    virtual int doDelete(); // 801906E8: unloads everything first
    virtual int execute();  // 801906A8
    virtual int draw();     // 801906E0

    BOOL loadObjs();   // 8019077C: steps the requested loads; FALSE while one loads
    BOOL unloadObjs(); // 80190844: steps the released slots; FALSE while one is in use or unloading
    // Requests slot idx (a player) to load the model of an item / fish or insect kind. TRUE once it is
    // loaded; while the slot is busy it changes nothing.
    BOOL request(int idx, int type, const dItem::Item &item, BOOL makeMdl, int mode); // 80190900
    BOOL request(int idx, int type, int kind, BOOL makeMdl, int mode);                // 80190970
    void release(int idx); // 801909D4

    // The model of slot idx once it is built, NULL before.
    dFIObj_c *getObj(int idx) {
        dFIMdlBank_c *bank = &mBanks[idx];
        if (bank->mState == dFIMdlBank_c::STATE_MDL && bank->mMakeMdl) {
            return &mObjs[idx];
        }
        return NULL;
    }

    /* 0x064 */ dFIMdlBank_c mBanks[OBJ_NUM];
    /* 0x224 */ EGG::FrmHeap *mpResHeaps[OBJ_NUM]; // "dFIMdlMgr_c::m_ResHeap", 0x6800 each
    /* 0x234 */ dFIObj_c mObjs[OBJ_NUM];
    /* 0x484 */ u8 mCreated;
    /* 0x485 */ u8 mDeleting;

    static dFIMdlMgr_c *sInstance; // 8074E9E8
}; // size 0x488
