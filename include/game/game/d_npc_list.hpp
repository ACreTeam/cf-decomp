#pragma once

// The npc actor lists (DOL TU d_npc_list.cpp, .text 800F9424..800F9B88). Two file-local lists: the villagers
// (acListNml_c, 5 entries, dAcNpcNml_c) and the special npcs (acListSp_c, 4 entries, dAcNpcSp_c). Each entry is
// an actor and its npc item (the key). Class names from RTTI; all function names are inferred.

#include <types.h>
#include <game/game/d_fg_item.hpp>

class dDemoActor_c;

namespace dNpcList {

u32 getNmlNum(); // 800F97FC: entries of the villager list
u32 getSpNum();  // 800F980C: entries of the special npc list

int addNml(dDemoActor_c *actor, const dItem::Item &key); // 800F981C: the entry index, or -1
void removeNml(dDemoActor_c *actor);                     // 800F9834
int countNml();                                          // 800F9844: used entries
dDemoActor_c *getNml(u32 idx);                           // 800F9850
dDemoActor_c *getNmlAtUnit(int ux, int uz);              // 800F9860: the villager standing on unit (ux, uz)
dDemoActor_c *getNmlByKey(const dItem::Item &key);       // 800F9878

int addSp(dDemoActor_c *actor, const dItem::Item &key); // 800F9888
void removeSp(dDemoActor_c *actor);                     // 800F98A0
int countSp();                                          // 800F98B0
dDemoActor_c *getSp(u32 idx);                           // 800F98BC
dDemoActor_c *getSpAtUnit(int ux, int uz);              // 800F98CC
dDemoActor_c *getSpByKey(const dItem::Item &key);       // 800F98E4

} // namespace dNpcList
