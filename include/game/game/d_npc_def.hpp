#pragma once

// Counts shared by the NPC actors and the NPC managers.
#define NPC_SPECIES_NUM 33 // villager species (per-species npc tables)
#define NPC_SPECIAL_NUM 97 // special npcs: item ids 0x8000 + 0..96, /Npc/Special/Model/0..96

// Special npc item ids (names after ac-decomp's SP_NPC_*); most are still literals.
#define SP_NPC_BROKER 0x8003 // Crazy Redd (npc_SP/NPC_tunekichi; d_a_str_town)
#define SP_NPC_MISSING_CHILD 0x8019 // Katie, the lost kitten (d_a_npc_sp_missing_child, d_a_npc_sp_gatekeeper_a)
#define SP_NPC_GATEKEEPER_A 0x8026 // Copper, the gatekeeper (d_a_npc_sp_gatekeeper_a)
#define SP_NPC_GATEKEEPER_B 0x8027 // Booker, the other gatekeeper (d_a_npc_sp_gatekeeper_a/b)
#define SP_NPC_GUIDE 0x8029 // Rover (the player select, d_a_npc_sp_player_select)
#define SP_NPC_GODDESS 0x804F // the fountain's goddess (d_a_npc_sp_goddess, d_a_str_field)
