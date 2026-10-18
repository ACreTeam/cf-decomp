#pragma once

// The insect manager dInsectField_c. See d_ins_fld_base.hpp for the REL
// overview.

#include <game/game/d_ins_fld_base.hpp>

// The manager. Its instance is g_playActorMng (set in create, cleared in
// doDelete).
class dInsectField_c : public fBase_c {
  public:
    // addPlayerInsect's mode.
    enum PlayerInsectMode_e {
        PLAYER_INSECT_SHOW = 0,    // a dInsFldShow_c: only the held model
        PLAYER_INSECT_RELEASE = 1, // the insect itself, let go from the pocket
    };

    enum {
        SPAWN_INTERVAL = 30, // mSpawnTimer: frames between calcSpawn's spawns
    };

    dInsectField_c();          // 0x250
    virtual ~dInsectField_c(); // 0x4B0

    virtual int create(); // 0x25EE4
    virtual int preCreate() {
        return SUCCEEDED;
    } // 0x270B0 (weak)
    virtual int doDelete(); // 0x85C
    virtual int execute();  // 0x52C
    virtual void postExecute(MAIN_STATE_e state) {
    } // 0x270AC (weak)
    virtual int draw(); // 0x7A0
    virtual void postDraw(MAIN_STATE_e state) {
    } // 0x270A8 (weak)
    virtual void deleteReady() {
    } // 0x270A4 (weak)
    virtual bool createHeap() {
        return true;
    } // 0x2709C (weak)

    void
    addActive(dInsFldBase_c *insect); // 0x9B4: mActive list, mId = mNextId++
    void removeActive(dInsFldBase_c *insect); // 0x9EC
    void addPending(
        dInsFldBase_c *insect); // 0xA1C: mPending list (waiting for the model)
    void removePending(dInsFldBase_c *insect); // 0xA44
    void addDying(dInsFldBase_c *insect);      // 0xA74
    void removeDying(dInsFldBase_c *insect);   // 0xA9C
    // Adds a town insect of type at pos (heaps: list, default m_objHeap) and
    // fills its net slot.
    BOOL addInsect(int type, const mVec3_c &pos, const mAng3_c *angle,
                   dInsectField::heapList_c *list); // 0xB28
    dInsFldBase_c *addPlayerInsect(int type, int mode, const mVec3_c *pos,
                                   int player); // 0xF7C
    BOOL deleteInsect(
        dInsFldBase_c *insect); // 0x144C: removes a held (SPAWN_PLAYER) insect
    void addHoneycombBees(const mVec3_c *pos); // 0x1C74
    BOOL addFlea(const mVec3_c *pos);          // 0x1CDC
    dInsFldBase_c *getFlea();                  // 0x1D34

    BOOL addInsectFromNet(int type, int idx,
                          dInsectField::heapList_c *list); // 0xDB0
    BOOL createPending();                     // 0x10F8: FALSE while one waits
    BOOL destroyAllDying();                   // 0x12AC
    BOOL destroyDying(dInsFldBase_c *insect); // 0x131C
    void calcSpawn();                         // 0x1550: every 30 frames
    void addFromNet();                        // 0x1608: the slots others filled
    BOOL removeInsect(dInsFldBase_c *insect); // 0x1688: doDelete, to mDying
    BOOL isLifeOver(dInsFldBase_c *insect);   // 0x17A0
    BOOL isNetChanged(dInsFldBase_c *insect); // 0x1838: its net slot moved on
    void spawn(int num, BOOL awayFromPlayers); // 0x1898
    void initSpawn();                          // 0x1B94
    BOOL deleteAll();                          // 0x1D40
    void sendSlots();                          // 0x51AC
    void initSlots();                          // 0x5200
    // A catch request of member: TRUE if the slot still holds that insect.
    BOOL acceptCatch(int member, dInsectField::catchMsg_c *msg); // 0x525C

    // The manager instance: the DOL keeps it in g_playActorMng, typed as its
    // own view of the manager (dPlayActorMng_c, d_play_util.hpp).
    static dInsectField_c *getInstance() {
        return reinterpret_cast<dInsectField_c *>(g_playActorMng);
    }

    /* 0x000 fBase_c (vtable at 0x060) */
    /* 0x064 */ dSearchCandXZCore_c *mSearch[INSECT_FIELD_SEARCH_NUM];
    /* 0x0A8 */ dInsectField::heapList_c
        m_objHeap; // 12 x 0x1000 (one per net slot)
    /* 0x0B4 */ dInsectField::heapList_c
        m_objHeapPermanent; // 3 x 0x1400 (honeycomb bees, flea)
    /* 0x0C0 */ dInsectField::heapList_c
        m_objHeapPlayer;                    // 4 x 0x1000 (addPlayerInsect)
    /* 0x0CC */ EGG::FrmHeap *m_parentHeap; // 0x69400 in dHeap::localHeap_p
    /* 0x0D0 */ EGG::FrmHeap *m_ResHeap[INSECT_FIELD_BANK_NUM]; // 0x5800 each
    /* 0x10C */ dInsFldBank_c mBanks[INSECT_FIELD_BANK_NUM];
    /* 0x760 */ dInsFldBase_c *mPending; // created once their bank is loaded
    /* 0x764 */ dInsFldBase_c *mActive;
    /* 0x768 */ dInsFldBase_c *mDying;
    /* 0x76C */ dInsFldBase_c
        *mNetInsects[INSECT_UTIL_PLAYER_NUM]; // per player: the insect in its
                                              // net
    /* 0x77C */ int mNextId;
    /* 0x780 */ int mSpawnTimer; // SPAWN_INTERVAL frames between spawns
    /* 0x784 */ u8 mCreated;
    /* 0x785 */ u8 mDeleting;
    /* 0x786 */ u8 mWasOwner; // last frame's "this console runs the scene"
    /* 0x787 */ dInsectCatchData_c mCatchData[INSECT_NET_SLOT_NUM];
}; // size 0x7D0

namespace dInsectField {
extern fProf::fBaseProfile_c g_profile_INSECT_FIELD; // .data 0
} // namespace dInsectField

inline void dInsFldBase_c::releasePlayer() {
    if (mPlayer != INSECT_FIELD_PLAYER_NONE) {
        dFIMdlMgr_c::sInstance->release(mPlayer);
        dInsectField_c *mng = dInsectField_c::getInstance();
        if (mng->mNetInsects[mPlayer] == this) {
            mng->mNetInsects[mPlayer] = NULL;
        }
        mPlayer = INSECT_FIELD_PLAYER_NONE;
    }
}
