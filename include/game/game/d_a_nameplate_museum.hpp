#ifndef GAME_D_A_NAMEPLATE_MUSEUM_HPP
#define GAME_D_A_NAMEPLATE_MUSEUM_HPP

// A museum exhibit's plaque (REL d_a_nameplate_museumNP, profile
// A_NAMEPLATE_MUSEUM), placed by the museum rooms' scene data with a
// dScData::PanelData_c as mParam: the kind of exhibit, its item indices and the
// room node it stands at. Reading it shows sys_MESS/SYS_Museum: the donated
// exhibits' names (insects and fish, up to four per page, "Read more."), a
// painting's or fossil's donor, a fossil's description pages, or "Thank you for
// your patronage." while nothing is donated. Class name from the RTTI
// ("daNameplateMuseum_c"); everything else is inferred.

#include <game/game/d_demo_actor.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_museum.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_sc_data.hpp>

class daNameplateMuseum_c : public dDemoActor_c, private dMsg::Rcpt_c {
  public:
    // PanelData_c::mKind: the exhibits' item kind.
    enum KIND_e {
        KIND_INSECT,
        KIND_FISH,
        KIND_PICTURE,
        KIND_FOSSIL,
    };

    // sys_MESS/SYS_Museum message codes, named from the English text.
    enum {
        MSG_THANK_YOU = 0x01,  // "Thank you for your patronage."
        MSG_DONATED_BY = 0x02, // "<item> / Donated by <player>."
        MSG_DONATED_LONG_AGO =
            0x03, // "... Donated by a kind village resident long, long ago."
        MSG_COCKROACH_DONATED =
            0x04, // "That cockroach was donated to the museum by <player>."
        MSG_COCKROACH_WARD = 0x05, // "That cockroach is a ward of the museum."
        MSG_ITEMS_1 = 0x06,        // one exhibit's name
        MSG_ITEMS_2 = 0x07,
        MSG_ITEMS_3 = 0x08,
        MSG_ITEMS_4 = 0x09,
    };

    // sys_SELECT/SYS_Select choices.
    enum {
        SEL_MSG_READ_MORE = 3, // "Read more."
        SEL_MSG_ENOUGH = 4,    // "Enough!"
    };

    enum {
        ITEMS_PER_PAGE = 4,
    };

    enum MODE_e {
        MODE_WAIT,     // nobody reads it
        MODE_TALK,     // waits for the message window to open
        MODE_TALK_END, // waits for the message window to close
        MODE_NUM,
    };

    typedef BOOL (daNameplateMuseum_c::*InitFunc)();
    typedef void (daNameplateMuseum_c::*ExecFunc)();

    daNameplateMuseum_c() : mCreated(FALSE) {
    }

    virtual int create();                          // .text:0xF8
    virtual int execute();                         // .text:0x29C
    virtual int draw();                            // .text:0x2D4
    virtual int doDelete();                        // .text:0x2DC
    virtual bool canInteract(dDemoActor_c *actor); // .text:0x300
    virtual void onDemoCmd(int cmd, u8 member);    // .text:0x438

    int getItemBase() const; // .text:0x460: the kind's first dItem::ITEM_IDX_*
    BOOL isAnyDonated() const; // .text:0x4B4
    BOOL isAllDonated() const; // .text:0x554
    void findFirstDonated(); // .text:0x5F4: mIndex = the first donated exhibit
                             // (or mCount)
    void findNextDonated();  // .text:0x69C: mIndex = the next donated exhibit
                             // (or mCount)
    u32
    countDonated() const; // .text:0x740: the donated exhibits from mIndex on

    virtual void onMessageStart(int arg); // .text:0x7E8
    virtual void onMessageEnd(int kind);  // .text:0x960
    virtual void onAnswer(int arg);       // .text:0xA44

    static const InitFunc sInitFuncs[MODE_NUM];    // .rodata:0x14
    static const ExecFunc sExecuteFuncs[MODE_NUM]; // .rodata:0x38

    BOOL setMode(int mode); // .text:0xB14
    void executeMode();     // .text:0xB80

    BOOL initWait();       // .text:0xBC0
    void executeWait();    // .text:0xBC8
    BOOL initTalk();       // .text:0xBCC
    void executeTalk();    // .text:0xD28
    BOOL initTalkEnd();    // .text:0xD9C
    void executeTalkEnd(); // .text:0xDA4

    BOOL isDonated(u32 i) const {
        dItem::Item item(getItemBase(), mIds[i], FALSE);
        return getMuseum()->isDonated(item);
    }
    const dScData::PanelData_c *getPanel() const {
        return (const dScData::PanelData_c *)mParam;
    }
    static dMuseum_c *getMuseum() {
        return &dSaveData_c::getTown()->mMuseum;
    }

    static u16 sNum; // .bss:0x8: the plaques created (the demo ID's low byte)

    /* 0x0BC dMsg::Rcpt_c */
    /* 0x120 */ int mMode;
    /* 0x124 */ int mKind; // KIND_e
    /* 0x128 */ u32 *mIds; // item indices from getItemBase()
    /* 0x12C */ int mCount;
    /* 0x130 */ int mIndex;   // the exhibit shown next
    /* 0x134 */ int mMsgCode; // a fossil's description (mCount > 1)
    /* 0x138 */ dObjc::ac_c mObjc;
    /* 0x17C */ u8 mSolid; // the player bumps into it
    /* 0x17D */ u8 mCreated;
}; // size 0x180

#endif
