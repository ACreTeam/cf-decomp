#pragma once

// Counts shared by the NPC actors and the NPC managers.
#define NPC_SPECIES_NUM 33 // villager species (per-species npc tables)
#define NPC_SPECIAL_NUM 97 // special npcs: item ids 0x8000 + 0..96, /Npc/Special/Model/0..96

// Special npc item ids (names after ac-decomp's SP_NPC_*); most are still literals.
#define SP_NPC_KABUPEDDLER 0x8001 // Joan, the turnip seller (d_a_npc_sp_kaburiba; d_a_npc_sp_gatekeeper_b)
#define SP_NPC_SHOP_MASTER 0x8002 // Tom Nook (d_a_npc_sp_op_tanukichi; the nameplate of an empty house)
#define SP_NPC_BROKER 0x8003 // Crazy Redd (npc_SP/NPC_tunekichi; d_a_str_town)
#define SP_NPC_TOTAKEKE 0x8007 // K.K. Slider (d_a_npc_sp_totakeke; d_a_npc_sp_gatekeeper_b)
#define SP_NPC_SABLE 0x8008 // Sable (d_a_npc_sp_kinuyo: Mabel's talk partner)
#define SP_NPC_MABEL 0x8009 // Mabel (d_a_npc_sp_asami: Sable's talk partner)
#define SP_NPC_POST_GIRL 0x800A // Pelly (d_a_npc_sp_periko: dAcNpcSpPeriko_c)
#define SP_NPC_POST_GIRL2 0x800B // Phyllis (d_a_npc_sp_periko: dAcNpcSpPerimi_c)
#define SP_NPC_MAMEDANUKI0 0x800F // Timmy (d_a_npc_sp_tanukichi: dAcNpcSpMamekichi_c; d_sc_data_rm_shop3_f2)
#define SP_NPC_MAMEDANUKI1 0x8010 // Tommy (d_a_npc_sp_tanukichi: dAcNpcSpTubukichi_c)
#define SP_NPC_RESETSAN 0x8011 // Mr. Resetti after a reset (d_a_npc_sp_resetsan)
#define SP_NPC_RACKETSAN 0x8012 // Don Resetti after the fifth reset (d_a_npc_sp_resetsan)
#define SP_NPC_MASK_CAT 0x8013 // the mysterious cat, Blanca (d_a_npc_sp_mysterycat; d_a_npc_sp_gatekeeper_b)
#define SP_NPC_KAPPN 0x8014 // Kapp'n, the bus driver (d_a_npc_sp_player_create: the stop announcements)
#define SP_NPC_ARTIST 0x8016 // Wendell, the artist (d_a_npc_sp_seiichi; d_a_npc_sp_gatekeeper_b)
#define SP_NPC_MISSING_CHILD 0x8019 // Katie, the lost kitten (d_a_npc_sp_missing_child, d_a_npc_sp_gatekeeper_a)
#define SP_NPC_MISSING_MOTHER 0x801A // Katie's mother (d_a_npc_sp_missing_mother, d_a_npc_sp_missing_child)
#define SP_NPC_GHOST 0x801B // Wisp (d_a_npc_sp_yuutaro, d_a_npc_sp_yuutaro_loft; d_menu_item00 rubbing the lamp)
#define SP_NPC_CARPETPEDDLER 0x801D // Saharah (d_a_npc_sp_roran; d_a_npc_sp_gatekeeper_b)
#define SP_NPC_SISYO 0x8020 // Dr. Shrunk at the theater (d_a_npc_sp_sisyo)
#define SP_NPC_KYOSYO 0x8021 // Master Frillard at the theater (d_a_npc_sp_sisyo on his programs)
#define SP_NPC_FISHING_MASTER 0x8025 // Chip at the fishing tourney (d_a_npc_sp_uomasa; d_a_npc_sp_gatekeeper_b)
#define SP_NPC_GATEKEEPER_A 0x8026 // Copper, the gatekeeper (d_a_npc_sp_gatekeeper_a)
#define SP_NPC_GATEKEEPER_B 0x8027 // Booker, the other gatekeeper (d_a_npc_sp_gatekeeper_a/b)
#define SP_NPC_GUIDE 0x8029 // Rover (the player select, d_a_npc_sp_player_select)
#define SP_NPC_CAFE_GUEST 0x802A // the Roost's guest seat in the scene data (d_a_npc_sp_cafe_guest: no guest today)
#define SP_NPC_SONCHO_EVENT 0x802C // Tortimer at the countdown (d_npc_mng l_holidayNpcs); d_a_npc_sp_soncho awake
#define SP_NPC_CONV_MASTER 0x8044 // Tom Nook in Nook 'n' Go (d_a_npc_sp_tanukichi preCreate)
#define SP_NPC_SUPER_MASTER 0x8045 // Tom Nook in Nookway
#define SP_NPC_DEPART_MASTER 0x8046 // Tom Nook in Nookington's
#define SP_NPC_EASTER_BUNNY 0x804A // Zipper T. Bunny (d_a_npc_sp_pyontaro; d_a_npc_sp_gatekeeper_b)
#define SP_NPC_BUG_MASTER 0x804C // Nat at the Bug-Off (d_a_npc_sp_bug; d_a_npc_sp_gatekeeper_b)
#define SP_NPC_DANCER 0x804D // Pave at the Festivale (d_a_npc_sp_dancer; d_a_npc_sp_gatekeeper_b)
#define SP_NPC_GODDESS 0x804F // the fountain's goddess (d_a_npc_sp_goddess, d_a_str_field)
#define SP_NPC_RESETTI 0x8050 // Mr. Resetti at the Reset Surveillance Center (d_a_npc_sp_demo_resetsan, RM_RESET)
#define SP_NPC_DON 0x8051 // Don Resetti at the Reset Surveillance Center (d_a_npc_sp_demo_resetsan, RM_RESET)
#define SP_NPC_RESETTI_OFF 0x805C // Mr. Resetti off duty, model "mof" (the cafe guest; the center's first visit)
#define SP_NPC_DON_OFF 0x805D // Don Resetti off duty, model "mod" (the cafe guest, d_a_npc_sp_cafe_guest)
