#pragma once

// dNpcMng_c: the npc manager process (RTTI "dNpcMng_c"). Source: src/dol/game/d_npc_mng.cpp
// (.text 801A6468..801AD6E0). Created with each scene; it spawns the scene's villagers and special npcs:
//   town            the villagers that are out (createVillagers), hide-and-seek (updateHideAndSeek), and the
//                   special npcs of the three spawn tables (event npcs, holiday npcs, visitors)
//   villager house  its villager (createHouseVillager)
//   player house    a villager on a visit (createPlayerHouseVisitor)
//   city            the moved-out villagers (createCityAnimals), Gracie's and the theater's guests
//   museum cafe     the villager on a home stay (createCafeNpc)
// It also owns the npc stage heap (createStageHeap, the parent of every npc actor's heap) and the town's
// special npc model heap (m_halloweenModelHeap_p, the model file /Npc/Special/Model/<n>.brres).
// The class name is the game's (RTTI); the rest is inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_event.hpp>

namespace EGG {
class FrmHeap;
}
namespace dItem {
struct Item;
}
class mVec3_c;
class mAng3_c;
class dTime_c;
class dFdBase_c;
struct dAnimalTemplate_c;

// The city's square, where its guests stand (pickCitySpots) and the moved-out villagers stroll
// (d_a_npc_town_out).
extern const f32 cCityMinX; // 512.0f
extern const f32 cCityMinZ; // 288.0f
extern const f32 cCityMaxX; // 1024.0f
extern const f32 cCityMaxZ; // 672.0f

class dNpcMng_c : public fBase_c {
public:
    // A special npc of the spawn tables (l_eventNpcs, l_holidayNpcs, l_visitorNpcs): it comes when mCheck
    // (scene, previous scene) says so (and mEvent is active, when set); mSetPos places it.
    struct Spawn_c {
        /* 0x00 */ BOOL (*mCheck)(u8 scene, u8 prevScene);
        /* 0x04 */ void (dNpcMng_c::*mSetPos)(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc);
        /* 0x10 */ BOOL (*mIsHere)(dQuestEvent_e event); // talk checks (isEventNpcHere...): NULL = always
        /* 0x14 */ u16 mEvent;   // dQuestEvent_e; 0xFFFF = none
        /* 0x16 */ u16 mProfile; // the actor's profile
        /* 0x18 */ u16 mNpc;     // the npc's item id (0x80xx)
    }; // size 0x1C

    // Hide-and-seek (dAcNpcNml_c's quest 10; sHideAndSeekState).
    enum HideAndSeekState_e {
        HIDE_AND_SEEK_START, // the hiders gather at the town hall
        HIDE_AND_SEEK_HIDE,  // the hiders hide
        HIDE_AND_SEEK_END,   // the hiders gather at the town hall again
        HIDE_AND_SEEK_NONE,
    };

    dNpcMng_c();          // 801A6498
    virtual ~dNpcMng_c(); // 801A64EC

    virtual int create();    // 801AA56C
    virtual int preCreate(); // 801AA4EC: the model heap (town)
    virtual int doDelete();  // 801AA70C
    virtual int execute();   // 801AA774
    virtual int draw();      // 801AA8DC

    static void reset();                         // 801A6564: (d_reset) soft reset
    static void createStageHeap();               // 801A6584: (d_scene) sized for the scene's npcs
    static void destroyStageHeap();              // 801A670C: (d_scene)
    static void *getSpecialModel();              // 801A6740: the loaded model file
    static BOOL setHideAndSeekState(int state);  // 801A6748: FALSE while online or out of range

    // Hide-and-seek.
    void placeHidersAtTownHall();                // 801A679C
    BOOL pickHideSpotBuilding(mVec3_c *pos, const s8 *usedBlocks, u32 usedNum);  // 801A6898
    BOOL pickHideSpotTree(mVec3_c *pos, const s8 *usedBlocks, u32 usedNum);      // 801A6C2C
    BOOL pickHideSpotBuildSite(mVec3_c *pos, const s8 *usedBlocks, u32 usedNum); // 801A6FD8
    void hideHiders();                           // 801A72A4
    void updateHideAndSeek();                    // 801A74C4: runs the state's step once

    // Villagers.
    BOOL createVillagers();                      // 801A753C: the villagers that are out (up to 5)
    BOOL pickHouseVillagerPos(mVec3_c *pos);     // 801A7834: a free unit of the villager house
    void createHouseVillager();                  // 801A7AC0
    BOOL createPlayerHouseGuest();               // 801A7D34: the event 11 villager
    BOOL createAppointmentGuest();               // 801A7EE0: the appointment's villager
    void createPlayerHouseVisitor();             // 801A8008
    BOOL createAnimals();                        // 801A8048

    // Spots of the special npcs (Spawn_c::mSetPos).
    static BOOL getTownHallPos(mVec3_c *pos, int unitX, int unitZ); // 801A8134: a unit from the town hall's
    void setPosTownHallA(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc);  // 801A81E0
    void setPosTownHallRight(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801A8250
    void setPosTownHallB(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc);  // 801A82C0
    void setPosTownHallC(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc);  // 801A8330
    void setPosTownHallLeft(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801A83A0
    void setPosRandomSpot(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801A8410
    void setPosRandomPlace(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801A8488
    void setPosHarvest(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc);  // 801A84F0
    // 801A85C8: the spot the npc's daub saved for this scene; checkPut: also moves it when blocked.
    static BOOL getDaubPos(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc, BOOL checkPut);
    static BOOL getDaubPos(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801A87B8

    // Spawn checks (Spawn_c::mCheck).
    static BOOL isTownScene(u8 scene, u8 prevScene);       // 801A87C0: the town (not its demo / check field)
    static BOOL isTownPlay(u8 scene, u8 prevScene);        // 801A882C: and a player without private flag 0xD
    static const Spawn_c *findSpawn(const Spawn_c *list, u32 num, u8 scene, u8 prevScene); // 801A8880
    static int findSpawnIdx(const dItem::Item *npc, const Spawn_c *list, u32 num); // 801A8954: -1 if none
    void createSpecialNpcs();                    // 801A89F4

    // The city's guests.
    static void clearIdList(u16 *list, u32 num);         // 801A8AFC: 0xFFFF
    static void addId(u16 *list, u32 num, u16 id);       // 801A8B5C: into the first free slot, once
    static int findId(u16 id, const u16 *list, u32 num); // 801A8BDC: -1 if none
    static u32 getLooksToday(u8 *looks, const dTime_c *time); // 801A8C3C: looks bits (0..5) allowed today
    static dAnimalTemplate_c *pickTemplate(u8 looks, const u16 *exclude, u32 num); // 801A8CE0
    void setupCityGuests();                      // 801A8E2C: Gracie's and the theater's guests
    static BOOL pickUnit(mVec3_c *pos, const mVec3_c *used, u32 usedNum, BOOL inArea, f32 minX, f32 minZ,
                         f32 maxX, f32 maxZ); // 801A9098
    static BOOL pickCitySpots(mVec3_c *pos, u32 num); // 801A933C
    void setupCityAnimalItems();                 // 801A93F0: umbrellas in the rain
    BOOL createCityAnimals();                    // 801A94E8
    BOOL pickRoomSpots(mVec3_c *pos, u32 num);        // 801A9970
    BOOL createGraceGuest();                     // 801A9A24
    static BOOL pickTheaterSpot(mVec3_c *pos, const mVec3_c *used, u32 usedNum); // 801A9B14
    static BOOL pickTheaterSpots(mVec3_c *pos, u32 num); // 801A9D40
    BOOL createTheaterGuests();                  // 801A9DE0
    BOOL createCafeGuest();                      // 801A9EEC
    void updateAnimalPlaces();                   // 801AA020
    static void applyHouseVillagerItems();       // 801AA424: (d_field)

    // Event npcs (l_eventNpcs).
    BOOL createEventNpc();                       // 801AA8E4
    static BOOL isFishingTourneyTime(dQuestEvent_e event); // 801AA9CC
    static BOOL isHarvestTime(dQuestEvent_e event);        // 801AAA64
    static BOOL isToyDayTime(dQuestEvent_e event);         // 801AAA98
    static BOOL isBugOffTime(dQuestEvent_e event);         // 801AAACC
    static BOOL isVisitorBeforeTortimer(u8 scene, u8 prevScene); // 801AAB64
    static BOOL isPlayerSelectVisitor();                   // 801AAC38: (d_sv_proc)
    static BOOL isHolidayNpcToday();                       // 801AAD1C

    // Holiday npcs (l_holidayNpcs).
    BOOL createHolidayNpc();                     // 801AAD5C
    void setPosConstruction(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801AAE44
    static BOOL isConstructionTime(u8 scene, u8 prevScene); // 801AB124
    static BOOL isCatalogTime(u8 scene, u8 prevScene);      // 801AB1D0
    void setPosPlayerHouseA(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801AB2C8
    static BOOL isHolidayTime(dQuestEvent_e event);         // 801AB394
    static BOOL isHolidayDay(dQuestEvent_e event);          // 801AB400
    static BOOL isEventActive(dQuestEvent_e event);         // 801AB460
    static BOOL isFireworksTime(dQuestEvent_e event);       // 801AB494

    // Visitors (l_visitorNpcs).
    BOOL createVisitorNpc();                     // 801AB52C
    static BOOL isHouseWarmingTime(u8 scene, u8 prevScene); // 801AB62C
    void setPosPrevHouse(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801AB760
    static BOOL isDistMailVisitTime(u8 scene, u8 prevScene); // 801AB830: Pete brings a distribution letter
    void setPosPlayerHouseB(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801AB940
    static BOOL isLetterVisitTime(u8 scene, u8 prevScene);  // 801ABA0C
    void setPosPlayerHouseDoor(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801ABAE4
    static BOOL isLetterVisitFirstTime(u8 scene, u8 prevScene); // 801ABBA0
    static BOOL isLetterVisitKind1(u8 scene, u8 prevScene); // 801ABC1C
    static BOOL isPascalDay(u8 scene, u8 prevScene);      // 801ABC60
    void setPosRaccoSpot(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801ABCC8
    static BOOL isGuestVisitTime(u8 scene, u8 prevScene);   // 801ABD7C
    void setPosGuestVisit(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801ABDC0
    static BOOL isHostVisitTime(u8 scene, u8 prevScene);    // 801ABDC4
    void setPosHostVisit(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801ABE08
    void setPosFountain(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801ABE0C
    static BOOL hasFountain(u8 scene, u8 prevScene);        // 801ABEC8
    static BOOL isRelayDesignVisitTime(u8 scene, u8 prevScene); // 801ABF44
    static BOOL isJoanDay(u8 scene, u8 prevScene);      // 801ABF88
    static BOOL isWispDay(u8 scene, u8 prevScene);      // 801ABFCC
    static BOOL isFlag13Visit(u8 scene, u8 prevScene);      // 801AC010
    void setPosJoan(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801AC150
    static BOOL isResetVisitTime(u8 scene, u8 prevScene);   // 801AC154
    void setPosResetVisit(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801AC278
    static BOOL isWendellDay(u8 scene, u8 prevScene);      // 801AC374
    void setPosGateA(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801AC438
    static BOOL isNoVisitorDay(u8 scene, u8 prevScene);      // 801AC520
    void setPosGateB(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801AC588
    static BOOL isSaharahDay(u8 scene, u8 prevScene);      // 801AC670
    void setPosSaharah(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801AC6B4
    void setPosWisp(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801AC6B8
    BOOL pickEmptyBlockSpot(mVec3_c *pos);       // 801AC7BC: a block of kind 0xB / 0x2D without houses
    BOOL pickBlockSpot(mVec3_c *pos, int blockX, int blockZ); // 801ACA4C
    BOOL canStandAt(dFdBase_c *field, int unitX, int unitZ); // 801ACB54: an fg item with free units behind
    void setPosBeach(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801ACC58
    void setPosCarnival(mVec3_c *pos, mAng3_c *angle, const dItem::Item *npc); // 801ACDC8
    static BOOL isJoanHere(dQuestEvent_e event);        // 801ACE8C
    static BOOL isSaharahHere(dQuestEvent_e event);        // 801ACEBC
    static BOOL isNotTownFlag5(dQuestEvent_e event);        // 801ACEEC
    static BOOL isWendellHere(dQuestEvent_e event);        // 801ACF1C
    static BOOL isPeteGateTime(dQuestEvent_e event);        // 801ACF94
    static void getTodayEventNpc(dItem::Item *npc);         // 801ACFFC

    static void setShowTelop();                  // 801AD168: (q10) the telop at the next execute

    // Today's special npcs in town (d_npc_talk_free FreeE_Snpc).
    static BOOL isEventNpcHere(const dItem::Item &npc);   // 801AD174: an event npc (l_eventNpcs)
    static BOOL isHolidayNpcHere(const dItem::Item &npc); // 801AD228: a holiday npc (l_holidayNpcs; 0xFFF1: any)
    static BOOL isVisitorNpcHere(const dItem::Item &npc); // 801AD304: a visitor (l_visitorNpcs), or 0x8007 with visitor 3

    static BOOL isWeekVisitDay();                // 801AD3F0: 9:00..19:00 on a day of the l_weekDays bits
    static BOOL isWeekVisitWeek();               // 801AD560

    static const Spawn_c l_eventNpcs[7];         // 8047DCCC
    static const Spawn_c l_holidayNpcs[25];      // 8047DD90
    static const Spawn_c l_visitorNpcs[15];      // 8047E04C

    static dNpcMng_c *sInstance;                 // 8074EAB8: (d_camera)
    static EGG::FrmHeap *m_stageHeap_p;          // 8074EABC: the npc stage heap (parent of dAcNpc_c::m_heap_p)
    static u8 sResetGraceSpots;                  // 8074EAC0
    static u8 sResetTheaterSpots;                // 8074EAC1
    static void *sSpecialModel;                  // 8074EAC4
    static int sTheaterMsg;                      // 8074EAC8: theater message code (1..0x1D), set by a REL
                                                 // (d_npc_talk_town "Town_Theater")
    static u8 sShowTelop;                        // 8074EACC
    static u8 _8074EACD;                         // cleared by d_scene
    static u8 sResetVisited;                     // 8074EACE
    static u8 _8074EACF;                         // cleared on the title scene (d_sv_proc)
    static u8 sGulliverVisitorMet;               // 8074EAD0: Gulliver introduced himself to a visitor (d_a_npc_sp_jonny;
                                                 // PRIVATE_FLAG1_GULLIVER_MET for town players); cleared on the title
                                                 // scene and by dSaveData_c::refreshTown
    static int sHideAndSeekState;                // 8074C180: HideAndSeekState_e
    static u16 sGraceGuest;                      // 8074C184: Gracie's guest (npc index, 0xFFFF none)
    static u16 sTheaterGuests[2];                // 8074C188

    /* 0x64 */ EGG::FrmHeap *m_halloweenModelHeap_p;
    /* 0x68 */ mAllocator_c mAllocator;
    /* 0x84 */ dDvd::loader_c mLoader;
    /* 0x98 */ u8 mShowTelop;
}; // size 0x9C
