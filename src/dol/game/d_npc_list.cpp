// d_npc_list.cpp: the npc actor lists. .text 800F9424..800F9B88, .ctors 804656E4, .data 804ED130..804ED190,
// .bss 805CE9D0..805CEA48, .sdata 8074AEF0..8074AF08. See include/game/game/d_npc_list.hpp.
// Class names from RTTI (global); member names are inferred.
#include <game/game/d_npc_list.hpp>
#include <game/game/d_demo_actor.hpp>

#define NPC_LIST_NML_NUM 5
#define NPC_LIST_SP_NUM 4

// A fixed-size list of actors and their keys (the derived classes own the arrays).
class acList_c {
public:
    acList_c(dDemoActor_c **actors, dItem::Item *keys, int num) : mpActors(actors), mpKeys(keys), mNum(num) {
        for (int i = 0; i < mNum; i++) {
            mpActors[i] = NULL;
            mpKeys[i] = dItem::ITEM_ID_NONE;
        }
    }
    virtual ~acList_c() {}

    int find(const dDemoActor_c *actor) const;
    int findKey(const dItem::Item &key) const;
    dDemoActor_c *get(u32 idx) const;
    dDemoActor_c *getAtUnit(int ux, int uz) const;
    dDemoActor_c *getByKey(const dItem::Item &key) const;
    int findEmpty() const;
    int add(dDemoActor_c *actor, const dItem::Item &key);
    void clear(u32 idx);
    void remove(const dDemoActor_c *actor);
    int count() const;
    u32 getNum() const { return mNum; }

    /* 0x04 */ dDemoActor_c **mpActors;
    /* 0x08 */ dItem::Item *mpKeys;
    /* 0x0C */ int mNum;
}; // size 0x10

class acListNml_c : public acList_c {
public:
    acListNml_c() : acList_c(mActors, mKeys, NPC_LIST_NML_NUM) {}
    virtual ~acListNml_c() {}

    /* 0x10 */ dDemoActor_c *mActors[NPC_LIST_NML_NUM];
    /* 0x24 */ dItem::Item mKeys[NPC_LIST_NML_NUM];
}; // size 0x30

class acListSp_c : public acList_c {
public:
    acListSp_c() : acList_c(mActors, mKeys, NPC_LIST_SP_NUM) {}
    virtual ~acListSp_c() {}

    /* 0x10 */ dDemoActor_c *mActors[NPC_LIST_SP_NUM];
    /* 0x20 */ dItem::Item mKeys[NPC_LIST_SP_NUM];
}; // size 0x28

static acListNml_c l_nmlList;
static acListSp_c l_spList;

// 800F9424
int acList_c::find(const dDemoActor_c *actor) const {
    if (mpActors != NULL && actor != NULL) {
        for (int i = 0; i < mNum; i++) {
            if (mpActors[i] == actor) {
                return i;
            }
        }
    }
    return -1;
}

// 800F947C
int acList_c::findKey(const dItem::Item &key) const {
    if (mpActors != NULL && mpKeys != NULL && key.isValid()) {
        for (int i = 0; i < mNum; i++) {
            if (mpKeys[i].isSame(key)) {
                return i;
            }
        }
    }
    return -1;
}

// 800F9528
dDemoActor_c *acList_c::get(u32 idx) const {
    if (idx < mNum && mpActors != NULL) {
        return mpActors[idx];
    }
    return NULL;
}

// 800F9554
dDemoActor_c *acList_c::getAtUnit(int ux, int uz) const {
    for (int i = 0; i < mNum; i++) {
        dDemoActor_c *actor = get(i);
        if (actor != NULL) {
            f32 fx = actor->getPosP()->x;
            f32 fz = actor->getPosP()->z;
            int x = (int)fx >> 5;
            int z = (int)fz >> 5;
            if (ux == x && uz == z) {
                return actor;
            }
        }
    }
    return NULL;
}

// 800F9608
dDemoActor_c *acList_c::getByKey(const dItem::Item &key) const {
    return get(findKey(key));
}

// 800F9640
int acList_c::findEmpty() const {
    dDemoActor_c **actors = mpActors;
    if (actors != NULL && mpKeys != NULL) {
        for (int i = 0; i < mNum; i++) {
            if (actors[i] == NULL) {
                return i;
            }
        }
    }
    return -1;
}

// 800F968C
int acList_c::add(dDemoActor_c *actor, const dItem::Item &key) {
    if (actor == NULL || !key.isValid() || mpActors == NULL || mpKeys == NULL) {
        return -1;
    }
    int idx = find(actor);
    if (idx == -1) {
        idx = findEmpty();
        if (idx != -1) {
            mpActors[idx] = actor;
            mpKeys[idx] = key;
        }
    }
    return idx;
}

// 800F9738
void acList_c::clear(u32 idx) {
    if (idx < mNum && mpActors != NULL && mpKeys != NULL) {
        mpActors[idx] = NULL;
        mpKeys[idx] = dItem::ITEM_ID_NONE;
    }
}

// 800F9780
void acList_c::remove(const dDemoActor_c *actor) {
    clear(find(actor));
}

// 800F97B8
int acList_c::count() const {
    dDemoActor_c **actors = mpActors;
    if (actors != NULL) {
        int n = 0;
        for (int i = 0; i < mNum; i++) {
            if (actors[i] != NULL) {
                n++;
            }
        }
        return n;
    }
    return 0;
}

namespace dNpcList {

// 800F97FC
u32 getNmlNum() {
    return l_nmlList.getNum();
}

// 800F980C
u32 getSpNum() {
    return l_spList.getNum();
}

// 800F981C
int addNml(dDemoActor_c *actor, const dItem::Item &key) {
    return l_nmlList.add(actor, key);
}

// 800F9834
void removeNml(dDemoActor_c *actor) {
    l_nmlList.remove(actor);
}

// 800F9844
int countNml() {
    return l_nmlList.count();
}

// 800F9850
dDemoActor_c *getNml(u32 idx) {
    return l_nmlList.get(idx);
}

// 800F9860
dDemoActor_c *getNmlAtUnit(int ux, int uz) {
    return l_nmlList.getAtUnit(ux, uz);
}

// 800F9878
dDemoActor_c *getNmlByKey(const dItem::Item &key) {
    return l_nmlList.getByKey(key);
}

// 800F9888
int addSp(dDemoActor_c *actor, const dItem::Item &key) {
    return l_spList.add(actor, key);
}

// 800F98A0
void removeSp(dDemoActor_c *actor) {
    l_spList.remove(actor);
}

// 800F98B0
int countSp() {
    return l_spList.count();
}

// 800F98BC
dDemoActor_c *getSp(u32 idx) {
    return l_spList.get(idx);
}

// 800F98CC
dDemoActor_c *getSpAtUnit(int ux, int uz) {
    return l_spList.getAtUnit(ux, uz);
}

// 800F98E4
dDemoActor_c *getSpByKey(const dItem::Item &key) {
    return l_spList.getByKey(key);
}

} // namespace dNpcList
