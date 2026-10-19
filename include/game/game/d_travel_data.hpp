#pragma once

#include <game/game/d_save_dl_item.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_private_data.hpp>
#include <cstring>

// Travel snapshot. Names are inferred from the save/restore operations.
namespace dTravelData {
template <int N> class ItemBlocks_c {
public:
    ItemBlocks_c() { memset(mItems, 0, sizeof(mItems)); }
    dSaveDLItem_c mItems[N];
};

class Data_c {
public:
    // Placement new for a heap block (d_a_npc_sp_gatekeeper_a create).
    static void *operator new(size_t, void *p) { return p; }
    Data_c();               // 80084840
    ~Data_c();              // 80084910
    void prepareDeparture(); // 80084990
    void backupDeparture(); // 80084B88
    void cancelDeparture(); // 80084CA4
    void prepareVisit();    // 80084D24
    void backupVisit();     // 80084F18
    void cancelVisit();     // 80084FA0
    void finishReturn();    // 80085018
    void finishVisit();     // 800851A0
    void clear();           // 80085334

    /* 0x000000 */ u32 mPocketMaskLE;
    /* 0x000004 */ ItemBlocks_c<15> mTownItems;
    /* 0x01E004 */ ItemBlocks_c<28> mPlayerItems;
    /* 0x056004 */ u8 _056004[0x1C];
    /* 0x056020 */ dAnimal_c mIncomingAnimal;
    /* 0x059060 */ dAnimal_c mOutgoingAnimal;
    /* 0x05C0A0 */ dPrivateData_c mPlayer;
    /* 0x064760 */ int mJinxOutcome;
    /* 0x064764 */ u8 mMii[0x4C];
    /* 0x0647B0 */ u8 mHasMii;
}; // size 0x647C0 (alignment inherited from the save types)
} // namespace dTravelData
