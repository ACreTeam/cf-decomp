#pragma once

#include <types.h>
#include <game/game/d_item.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>

#define MAIL_ADDRESS_RAW_LEN 95
#define MAIL_HEADER_LEN 32
#define MAIL_BODY_LEN 192
#define MAIL_FOOTER_LEN 32

// 0xC2. The first 0xBF bytes hold one of three forms, picked by mType.
struct dMailAddress_c {              // 0xC2
    union {
        dPersonalID_c mPlayer;                 // types 1, 6 (fn_80118B70); the name comes from mPlayer.mPlayer (fn_8013E8C8)
        dAnmPersonalID_c mAnimal;              // type 2 (fn_80118C54; fn_80118694 copies it from a villager in town data)
        wchar_t mText[MAIL_ADDRESS_RAW_LEN+1]; // type 7 (fn_80118D44 memsets 0xC0 and copies up to 95 chars)
    };
    u8 mType;                        // 0xC0
};

class dMail_c {                    // 0x390; ctor fn_80117400, clear fn_801178F8
public:
    dMail_c(); // 80117400
    ~dMail_c(); // 80117410

    dMailAddress_c mTo;              // 0x000
    dMailAddress_c mFrom;            // 0x0C2
    wchar_t mHeader[MAIL_HEADER_LEN + 1];             // 0x184  0x42 bytes
    wchar_t mBody[MAIL_BODY_LEN + 1];              // 0x1C6  0x182 bytes
    wchar_t mFooter[MAIL_FOOTER_LEN + 1];             // 0x348  0x42 bytes
    u8 mNamePos;                     // 0x38A  where the recipient name goes in the header ('\n' index, fn_80117AE8)
    u8 mPaper;                       // 0x38B  stationery index (seeker_c::findLike on the paper item)
    u8 m38C_hi : 2;                  // 0x38C  bits 7-6 (fn_801174AC / fn_801176B8)
    u8 m38C_b5 : 1;                  //        bit 5 (fn_80117570)
    u8 m38C_lo : 5;                  //        bits 4-0 (fn_80117484 / fn_80117490)
    u8 mFlags;                       // 0x38D  bit flags by index (fn_801174D8 set, fn_801174F4 clear, fn_80117510 test)
    dItem::Item mPresent;            // 0x38E  0xFFF1 when there's no present
};
