#pragma once

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>

// Happy Room Academy model rooms (dSvMdlRm_c, name from the RTTI of its nested searchCB_c): a copy of a player's or villager's room, kept in the save data
// (dSaveTown_c::mModelRoom and _0640C8.mModelRoomCandidate) together with its owner and score. Source:
// src/dol/game/d_model_room.cpp (.text 80111A2C..80112BB8). Names are inferred.

enum {
    MODEL_ROOM_OWNED = 1 << 0,    // mPlayerID or mAnimalID is set
    MODEL_ROOM_VILLAGER = 1 << 1, // the owner is mAnimalID
};

// The room itself is the dHomeRoom_c base. Note: mFlags hides dHomeRoom_c::mFlags.
struct dSvMdlRm_c : public dHomeRoom_c {
    // Predicate for getRandomRoomItem (RTTI dSvMdlRm_c::searchCB_c; the house rating's dHR::search*_c).
    struct searchCB_c {
        virtual BOOL check(const dItem::Item *item) = 0;
    };

    void clearFlags(); // 80111CEC
    void clear(); // 80111CF8
    void setOwner(const dPersonalID_c *pid); // 80111D58
    void setOwner(const dAnmPersonalID_c *aid); // 80111E44
    int getBgId(); // 80111F38: the BG of the room, by mRoomType
    BOOL isFromThisTown(); // 80111F60: the owner's town is this town
    BOOL setFromAnimal(u32 animalIdx); // 80112114: a villager's house room
    BOOL setFromHome(u32 home, int room); // 80112640: a room of a player's house
    dItem::Item getRandomRoomItem(searchCB_c &check); // 801129A8: a random item passing check
    int countRoomFtrTiles(); // 80112B08: tiles covered by furniture on layer 0

    // Inline: assigning the room directly in setFromHome swaps the copy loop's registers.
    void setRoom(const dHomeRoom_c *room) { dHomeRoom_c::operator=(*room); }

    BOOL isOwned() const { return mFlags & MODEL_ROOM_OWNED; }
    BOOL isVillager() const { return (mFlags >> 1) & 1; }
    // Has an owner with a valid ID (the house rating's checks).
    BOOL hasOwner() const { return isOwned() && (mPlayerID.isValid() || mAnimalID.isValid()); }
    BOOL isVillagerOwner() const {
        return isOwned() && (mPlayerID.isValid() || mAnimalID.isValid()) && isVillager();
    }
    BOOL isPlayerOwner() const {
        return isOwned() && (mPlayerID.isValid() || mAnimalID.isValid()) && !isVillager() && mPlayerID.isValid();
    }
    BOOL isAnimalOwner() const {
        return isOwned() && (mPlayerID.isValid() || mAnimalID.isValid()) && isVillager() && mAnimalID.isValid();
    }
    const dPersonalID_c *getPlayerID() const {
        return isPlayerOwner() ? &mPlayerID : NULL;
    }
    const dAnmPersonalID_c *getAnimalID() const {
        return isAnimalOwner() ? &mAnimalID : NULL;
    }

    /* 0x458 */ dPersonalID_c mPlayerID;
    /* 0x484 */ dAnmPersonalID_c mAnimalID;
    /* 0x544 */ s32 mScore;
    /* 0x548 */ u8 mRoomType; // 1: villager room; player rooms: 1/2 for the side rooms, else the house size
    /* 0x549 */ u8 mTheme;
    /* 0x54A */ u8 mFlags; // MODEL_ROOM_*
    /* 0x54B */ u8 _54B;
}; // size 0x54C

int getModelRoomRnd(const dTime_c &time, u32 max); // 80111A2C: 0..max-1, fixed for the date
dTime_c getModelRoomDate(); // 80111ABC: first Sunday of this (game day's) month
int getModelRoomTheme(); // 80111C40: the theme of the month, 0..5
