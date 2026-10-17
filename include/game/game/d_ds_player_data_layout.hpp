#pragma once

// The per-region Wild World save layouts (d_ds_player_data.cpp). Defined in
// this header: the player select REL (d_a_npc_sp_player_selectNP) includes it
// too and keeps its own unreferenced copies of the tables.

#include <types.h>

namespace dDsPlayerData {

// Offsets of the fields of a player (from the player's start).
struct playerLayout_s {
    /* 0x00 */ u32 mOption;    // bit 0
    /* 0x04 */ u32 mFace;      // low nibble
    /* 0x08 */ u32 mHair;      // high nibble (same byte as mFace)
    /* 0x0C */ u32 mHairColor; // bits 0..2
    /* 0x10 */ u32 mTownId;    // u16
    /* 0x14 */ u32 mPlayerId;  // u16
    /* 0x18 */ u32 mName;
    /* 0x1C */ u32 mGender;
    /* 0x20 */ u32 mCatalog;
};

// Offsets of the fields of a save copy.
struct saveLayout_s {
    /* 0x00 */ u32 mTownName;
    /* 0x04 */ u32 mPlayers;
    /* 0x08 */ u32 _08;
    /* 0x0C */ u32 mState;
    /* 0x10 */ u32 mSaveCount;
};

static const playerLayout_s l_playerLayoutJp = {
    0x17EF, 0x1CC6, 0x1CC6, 0x1CC7, 0x1CFC, 0x1D04, 0x1D06, 0x1D0C, 0x16CC};
static const playerLayout_s l_playerLayoutUs = {
    0x1C6B, 0x223C, 0x223C, 0x223D, 0x2276, 0x2280, 0x2282, 0x228A, 0x1B48};
static const playerLayout_s l_playerLayoutKr = {
    0x1D3B, 0x243C, 0x243C, 0x243D, 0x247E, 0x248C, 0x248E, 0x249A, 0x1C18};

static const saveLayout_s l_saveLayoutJp = {0x4, 0xC, 0x24444, 0x24446,
                                            0x24447};
static const saveLayout_s l_saveLayoutUs = {0x4, 0xC, 0x2BFBC, 0x2BFBE,
                                            0x2BFBF};
static const saveLayout_s l_saveLayoutKr = {0x4, 0x14, 0x2E7F4, 0x2E7F6,
                                            0x2E7F7};

const playerLayout_s *l_playerLayout[4] = {
    &l_playerLayoutJp,
    &l_playerLayoutUs,
    &l_playerLayoutUs,
    &l_playerLayoutKr,
};

const saveLayout_s *l_saveLayout[4] = {
    &l_saveLayoutJp,
    &l_saveLayoutUs,
    &l_saveLayoutUs,
    &l_saveLayoutKr,
};

} // namespace dDsPlayerData
