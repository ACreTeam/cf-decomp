// The city npc base dAcNpcTown_c. .text 80063438..800636C4 (no data: its vtable and RTTI are only in the RELs).
// See include/game/game/d_a_npc_town.hpp.
#include <game/game/d_a_npc_town.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_npc_setup.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_weather.hpp>
#include <lib/egg/core/eggHeap.h>

void dAcNpcTown_c::setAnimal() {
    if (mpAnimal != NULL) {
        return;
    }
    dAnimalSave_c *animals = &dSaveData_c::getTown()->mAnimals;
    dItem::Item *key = &mNpcItem;
    u16 npcIdx = key->mId;
    BOOL moved = FALSE;
    if (ITEM_NAME_TYPE(npcIdx) == 0xE && (npcIdx & 0x800)) {
        moved = TRUE;
    }
    if (moved) {
        // A moved-out villager (a special npc key): the save has it.
        dAnimal_c *animal = animals->getAnimalByKey(key);
        if (animal != NULL) {
            mpAnimal = animal;
        }
    } else if (m_heap_p != NULL) {
        // A guest: built from its template.
        u32 num = 0;
        dAnimalTemplate_c *list = dNpcSetup::getList(&num);
        if (list != NULL && num != 0) {
            dAnimalTemplate_c *tmpl = dNpcSetup::find(npcIdx, list, num);
            if (tmpl != NULL) {
                dAnimal_c *animal = (dAnimal_c *)m_heap_p->alloc(sizeof(dAnimal_c), 0x20);
                if (animal != NULL) {
                    dSaveData_c *save = dSaveData_c::getRaw();
                    animal->init(npcIdx, 1, &save->mLandID, tmpl);
                    BOOL rain = FALSE;
                    dWeather_c *weather = dWeather_c::sInstance;
                    if (weather != NULL) {
                        BOOL raining = dWeather_c::isRainKind(weather->_5884);
                        if (raining) {
                            rain = TRUE;
                        }
                    }
                    dItem::Item item = rain ? animal->getUmbrella() : dItem::Item();
                    animal->setHeldItem(&item);
                    mpAnimal = animal;
                }
            }
        }
    }
}

int dAcNpcTown_c::getSoundId() {
    dAnimal_c *animal;
    int id = 0xFFFF;
    if (!mSoundIdDone) {
        animal = mpAnimal;
        if (animal != NULL && animal->mID.isValid()) {
            id = (u16)animal->getSoundId();
            mSoundIdDone = TRUE;
        }
    }
    return id;
}

u32 dAcNpcTown_c::getHeapSize() {
    return calcHeapSize(FALSE);
}

int dAcNpcTown_c::preCreate() {
    clearNetSync();
    if (!dAcNpcNml_c::preCreate()) {
        return 0;
    }
    mSoundIdDone = FALSE;
    return 1;
}

u32 dAcNpcTown_c::calcHeapSize(BOOL withFrm) {
    return dAcNpcNml_c::calcHeapSize(withFrm) + sizeof(dAnimal_c);
}
