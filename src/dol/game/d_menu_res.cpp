// The menu modules (dMenuRes). .text 800CF77C..800CFB5C, .data 804EB6E0..804EBD78, .bss 805A24E8..805A2628,
// .sdata 8074AAF8..8074AD38, .sbss 8074E4F0. See include/game/game/d_menu_res.hpp.
#include <game/game/d_menu_res.hpp>
#include <game/cLib/c_dylink.hpp>
#include <game/game/d_heap.hpp>
#include <game/mLib/m_heap.hpp>

namespace dMenuRes {

#define MENU_RES_REF_MAX 0xFD

// The module (REL) names.
static const char *l_resName[MENU_RES_NUM] = {
    "d_menu_item",
    "d_menu_itemxx",
    "d_menu_item00",
    "d_menu_item01",
    "d_menu_item02",
    "d_menu_item03",
    "d_menu_item04",
    "d_menu_item05",
    "d_menu_item06",
    "d_menu_ltr",
    "d_menu_friend",
    "d_menu_friend01",
    "d_menu_friend02",
    "d_menu_friend03",
    "d_menu_friend04",
    "d_menu_friend05",
    "d_menu_friend06",
    "d_menu_friend07",
    "d_menu_friend08",
    "d_menu_paper",
    "d_menu_keytest",
    "d_menu_desi",
    "d_menu_desi00",
    "d_menu_edit",
    "d_menu_chat",
    "d_menu_insc",
    "d_menu_han",
    "d_menu_map",
    "d_menu_map00",
    "d_menu_map01",
    "d_menu_map02",
    "d_menu_map03",
    "d_menu_photo",
    "d_menu_photo01",
    "d_menu_catalog",
    "d_menu_number",
    "d_menu_number00",
    "d_menu_museum",
    "d_menu_time00",
    "d_paper",
    "d_menu_lcrsr",
    "d_menu_write",
    "d_menu_bank",
    "d_menu_kabu",
    "d_menu_mslct",
    "d_menu_mii",
    "d_menu_desi01",
    "d_menu_music",
    "d_menu_post00",
    "d_menu_post01",
    "d_menu_desi02",
    "d_menu_desi03",
    "d_menu_notice",
    "d_menu_notice00",
    "d_menu_notice01",
    "d_menu_melody",
    "d_menu_title",
    "d_menu_debug00",
    "d_menu_star",
    "d_menu_star00",
    "d_menu_star01",
    "d_menu_point",
    "d_menu_haniwa",
};

static u8 l_menu000[] = {MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM00, MENU_RES_END};
static u8 l_menu001[] = {MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM00, MENU_RES_END};
static u8 l_menu002[] = {MENU_RES_FRIEND, MENU_RES_END};
static u8 l_menu003[] = {MENU_RES_ITEM, MENU_RES_INSC, MENU_RES_END};
static u8 l_menu004[] = {MENU_RES_INSC, MENU_RES_END};
static u8 l_menu005[] = {MENU_RES_CHAT, MENU_RES_END};
static u8 l_menu006[] = {MENU_RES_MAP, MENU_RES_MAP00, MENU_RES_END};
static u8 l_menu007[] = {MENU_RES_D_PAPER, MENU_RES_PAPER, MENU_RES_END};
static u8 l_menu008[] = {MENU_RES_D_PAPER, MENU_RES_PAPER, MENU_RES_END};
static u8 l_menu009[] = {MENU_RES_D_PAPER, MENU_RES_PAPER, MENU_RES_END};
static u8 l_menu010[] = {MENU_RES_D_PAPER, MENU_RES_PAPER, MENU_RES_END};
static u8 l_menu011[] = {MENU_RES_D_PAPER, MENU_RES_DEBUG00, MENU_RES_END};
static u8 l_menu012[] = {MENU_RES_KEYTEST, MENU_RES_END};
static u8 l_menu013[] = {MENU_RES_DESI, MENU_RES_DESI00, MENU_RES_END};
static u8 l_menu014[] = {MENU_RES_DESI, MENU_RES_DESI00, MENU_RES_END};
static u8 l_menu015[] = {MENU_RES_EDIT, MENU_RES_END};
static u8 l_menu016[] = {MENU_RES_EDIT, MENU_RES_END};
static u8 l_menu017[] = {MENU_RES_EDIT, MENU_RES_END};
static u8 l_menu018[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu019[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu020[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu021[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu022[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu023[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu024[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu025[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu026[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu027[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu028[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu029[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu030[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu031[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu032[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu033[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu034[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu035[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu036[] = {MENU_RES_HAN, MENU_RES_END};
static u8 l_menu037[] = {MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM03, MENU_RES_END};
static u8 l_menu038[] = {MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM03, MENU_RES_END};
static u8 l_menu039[] = {MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM03, MENU_RES_END};
static u8 l_menu040[] = {MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM03, MENU_RES_END};
static u8 l_menu041[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu042[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu043[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu044[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu045[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu046[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu047[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu048[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu049[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu050[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu051[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu052[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu053[] = {MENU_RES_MAP, MENU_RES_MAP01, MENU_RES_END};
static u8 l_menu054[] = {MENU_RES_MAP, MENU_RES_MAP02, MENU_RES_END};
static u8 l_menu055[] = {MENU_RES_MAP, MENU_RES_MAP03, MENU_RES_END};
static u8 l_menu056[] = {MENU_RES_PHOTO, MENU_RES_END};
static u8 l_menu057[] = {MENU_RES_PHOTO01, MENU_RES_END};
static u8 l_menu058[] = {MENU_RES_D_PAPER, MENU_RES_CATALOG, MENU_RES_END};
static u8 l_menu059[] = {MENU_RES_NUMBER, MENU_RES_NUMBER00, MENU_RES_END};
static u8 l_menu060[] = {MENU_RES_MUSEUM, MENU_RES_END};
static u8 l_menu061[] = {MENU_RES_NUMBER, MENU_RES_TIME00, MENU_RES_END};
static u8 l_menu062[] = {MENU_RES_NUMBER, MENU_RES_TIME00, MENU_RES_END};
static u8 l_menu063[] = {MENU_RES_NUMBER, MENU_RES_TIME00, MENU_RES_END};
static u8 l_menu064[] = {MENU_RES_NUMBER, MENU_RES_TIME00, MENU_RES_END};
static u8 l_menu065[] = {MENU_RES_D_PAPER, MENU_RES_LCRSR, MENU_RES_WRITE, MENU_RES_END};
static u8 l_menu066[] = {MENU_RES_D_PAPER, MENU_RES_LCRSR, MENU_RES_WRITE, MENU_RES_END};
static u8 l_menu067[] = {MENU_RES_D_PAPER, MENU_RES_LCRSR, MENU_RES_WRITE, MENU_RES_END};
static u8 l_menu068[] = {MENU_RES_D_PAPER, MENU_RES_LCRSR, MENU_RES_WRITE, MENU_RES_END};
static u8 l_menu069[] = {MENU_RES_NUMBER, MENU_RES_KABU, MENU_RES_END};
static u8 l_menu070[] = {MENU_RES_NUMBER, MENU_RES_KABU, MENU_RES_END};
static u8 l_menu071[] = {MENU_RES_NUMBER, MENU_RES_KABU, MENU_RES_END};
static u8 l_menu072[] = {MENU_RES_NUMBER, MENU_RES_KABU, MENU_RES_END};
static u8 l_menu073[] = {MENU_RES_NUMBER, MENU_RES_KABU, MENU_RES_END};
static u8 l_menu074[] = {MENU_RES_NUMBER, MENU_RES_BANK, MENU_RES_END};
static u8 l_menu075[] = {MENU_RES_NUMBER, MENU_RES_BANK, MENU_RES_END};
static u8 l_menu076[] = {MENU_RES_NUMBER, MENU_RES_BANK, MENU_RES_END};
static u8 l_menu077[] = {MENU_RES_LTR, MENU_RES_MSLCT, MENU_RES_END};
static u8 l_menu078[] = {MENU_RES_LTR, MENU_RES_MSLCT, MENU_RES_END};
static u8 l_menu079[] = {MENU_RES_LTR, MENU_RES_MSLCT, MENU_RES_END};
static u8 l_menu080[] = {MENU_RES_MII, MENU_RES_END};
static u8 l_menu081[] = {MENU_RES_FRIEND01, MENU_RES_END};
static u8 l_menu082[] = {MENU_RES_FRIEND01, MENU_RES_END};
static u8 l_menu083[] = {MENU_RES_FRIEND02, MENU_RES_END};
static u8 l_menu084[] = {MENU_RES_FRIEND02, MENU_RES_END};
static u8 l_menu085[] = {MENU_RES_FRIEND03, MENU_RES_END};
static u8 l_menu086[] = {MENU_RES_FRIEND03, MENU_RES_END};
static u8 l_menu087[] = {MENU_RES_FRIEND04, MENU_RES_END};
static u8 l_menu088[] = {MENU_RES_FRIEND05, MENU_RES_END};
static u8 l_menu089[] = {MENU_RES_FRIEND06, MENU_RES_END};
static u8 l_menu090[] = {MENU_RES_FRIEND07, MENU_RES_END};
static u8 l_menu091[] = {MENU_RES_NUMBER, MENU_RES_FRIEND08, MENU_RES_END};
static u8 l_menu092[] = {MENU_RES_NUMBER, MENU_RES_FRIEND08, MENU_RES_END};
static u8 l_menu093[] = {MENU_RES_MUSIC, MENU_RES_END};
static u8 l_menu094[] = {MENU_RES_ITEM, MENU_RES_ITEM04, MENU_RES_END};
static u8 l_menu095[] = {MENU_RES_ITEM, MENU_RES_ITEM05, MENU_RES_END};
static u8 l_menu096[] = {MENU_RES_ITEM, MENU_RES_ITEM05, MENU_RES_END};
static u8 l_menu097[] = {MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_POST00, MENU_RES_END};
static u8 l_menu098[] = {MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_POST00, MENU_RES_END};
static u8 l_menu099[] = {MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_POST01, MENU_RES_END};
static u8 l_menu100[] = {MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_POST01, MENU_RES_END};
static u8 l_menu101[] = {MENU_RES_TITLE, MENU_RES_DESI, MENU_RES_DESI01, MENU_RES_END};
static u8 l_menu102[] = {MENU_RES_TITLE, MENU_RES_DESI, MENU_RES_DESI03, MENU_RES_END};
static u8 l_menu103[] = {MENU_RES_TITLE, MENU_RES_DESI, MENU_RES_DESI01, MENU_RES_END};
static u8 l_menu104[] = {MENU_RES_TITLE, MENU_RES_DESI, MENU_RES_DESI01, MENU_RES_END};
static u8 l_menu105[] = {MENU_RES_TITLE, MENU_RES_DESI, MENU_RES_DESI01, MENU_RES_END};
static u8 l_menu106[] = {MENU_RES_TITLE, MENU_RES_DESI, MENU_RES_DESI01, MENU_RES_END};
static u8 l_menu107[] = {MENU_RES_TITLE, MENU_RES_DESI, MENU_RES_DESI03, MENU_RES_END};
static u8 l_menu108[] = {MENU_RES_DESI, MENU_RES_DESI02, MENU_RES_END};
static u8 l_menu109[] = {MENU_RES_NOTICE, MENU_RES_END};
static u8 l_menu110[] = {MENU_RES_NOTICE00, MENU_RES_END};
static u8 l_menu111[] = {MENU_RES_NOTICE00, MENU_RES_END};
static u8 l_menu112[] = {MENU_RES_LCRSR, MENU_RES_NOTICE01, MENU_RES_END};
static u8 l_menu113[] = {MENU_RES_MELODY, MENU_RES_END};
static u8 l_menu114[] = {MENU_RES_STAR, MENU_RES_STAR00, MENU_RES_END};
static u8 l_menu115[] = {MENU_RES_STAR, MENU_RES_STAR00, MENU_RES_END};
static u8 l_menu116[] = {MENU_RES_STAR, MENU_RES_STAR01, MENU_RES_END};
static u8 l_menu117[] = {MENU_RES_POINT, MENU_RES_END};
static u8 l_menu118[] = {MENU_RES_HANIWA, MENU_RES_END};
static u8 l_menu119[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};
static u8 l_menu120[] = {MENU_RES_TITLE, MENU_RES_LTR, MENU_RES_ITEM, MENU_RES_ITEMXX, MENU_RES_ITEM02, MENU_RES_END};

// The resources of each menu (dMenuRes::addMenuRef / releaseMenuRef).
static u8 *l_menuRes[] = {
    l_menu000, // 0
    l_menu001, // 1
    l_menu002, // 2
    l_menu003, // 3
    l_menu004, // 4
    l_menu005, // 5
    l_menu006, // 6
    l_menu007, // 7
    l_menu008, // 8
    l_menu009, // 9
    l_menu010, // 10
    l_menu011, // 11
    l_menu012, // 12
    l_menu013, // 13
    l_menu014, // 14
    l_menu015, // 15
    l_menu016, // 16
    l_menu017, // 17
    l_menu018, // 18
    l_menu019, // 19
    l_menu020, // 20
    l_menu021, // 21
    l_menu022, // 22
    l_menu023, // 23
    l_menu024, // 24
    l_menu025, // 25
    l_menu026, // 26
    l_menu027, // 27
    l_menu028, // 28
    l_menu029, // 29
    l_menu030, // 30
    l_menu031, // 31
    l_menu032, // 32
    l_menu033, // 33
    l_menu034, // 34
    l_menu035, // 35
    l_menu036, // 36
    l_menu037, // 37
    l_menu038, // 38
    l_menu039, // 39
    l_menu040, // 40
    l_menu041, // 41
    l_menu042, // 42
    l_menu043, // 43
    l_menu044, // 44
    l_menu045, // 45
    l_menu046, // 46
    l_menu047, // 47
    l_menu048, // 48
    l_menu049, // 49
    l_menu050, // 50
    l_menu051, // 51
    l_menu052, // 52
    l_menu053, // 53
    l_menu054, // 54
    l_menu055, // 55
    l_menu056, // 56
    l_menu057, // 57
    l_menu058, // 58
    l_menu059, // 59
    l_menu060, // 60
    l_menu061, // 61
    l_menu062, // 62
    l_menu063, // 63
    l_menu064, // 64
    l_menu065, // 65
    l_menu066, // 66
    l_menu067, // 67
    l_menu068, // 68
    l_menu069, // 69
    l_menu070, // 70
    l_menu071, // 71
    l_menu072, // 72
    l_menu073, // 73
    l_menu074, // 74
    l_menu075, // 75
    l_menu076, // 76
    l_menu077, // 77
    l_menu078, // 78
    l_menu079, // 79
    l_menu080, // 80
    l_menu081, // 81
    l_menu082, // 82
    l_menu083, // 83
    l_menu084, // 84
    l_menu085, // 85
    l_menu086, // 86
    l_menu087, // 87
    l_menu088, // 88
    l_menu089, // 89
    l_menu090, // 90
    l_menu091, // 91
    l_menu092, // 92
    l_menu093, // 93
    l_menu094, // 94
    l_menu095, // 95
    l_menu096, // 96
    l_menu097, // 97
    l_menu098, // 98
    l_menu099, // 99
    l_menu100, // 100
    l_menu101, // 101
    l_menu102, // 102
    l_menu103, // 103
    l_menu104, // 104
    l_menu105, // 105
    l_menu106, // 106
    l_menu107, // 107
    l_menu108, // 108
    l_menu109, // 109
    l_menu110, // 110
    l_menu111, // 111
    l_menu112, // 112
    l_menu113, // 113
    l_menu114, // 114
    l_menu115, // 115
    l_menu116, // 116
    l_menu117, // 117
    l_menu118, // 118
    l_menu119, // 119
    l_menu120, // 120
    nullptr, // 121
};

static DynamicModuleControl *l_module[MENU_RES_NUM];
static u8 l_refCount[MENU_RES_NUM];
static BOOL l_dirty; // a reference count went from / to 0: update() has work

// 800CF77C
void create() {
    int i;
    EGG::ExpHeap *heap = dHeap::menuDylHeap_p;
    EGG::Heap *prevHeap = mHeap::setCurrentHeap(heap);
    for (i = 0; i < MENU_RES_NUM; i++) {
        l_module[i] = new DynamicModuleControl(l_resName[i], heap);
        l_refCount[i] = 0;
    }
    l_dirty = FALSE;
    mHeap::setCurrentHeap(prevHeap);
}

// 800CF824
BOOL destroy() {
    int i;
    for (i = 0; i < MENU_RES_NUM; i++) {
        if (l_module[i] != nullptr && l_module[i]->isLinked() && !l_module[i]->forceUnlink()) {
            return FALSE;
        }
    }
    EGG::Heap *prevHeap = mHeap::setCurrentHeap(dHeap::menuDylHeap_p);
    for (i = 0; i < MENU_RES_NUM; i++) {
        if (l_module[i] != nullptr) {
            delete l_module[i];
            l_module[i] = nullptr;
        }
    }
    mHeap::setCurrentHeap(prevHeap);
    return TRUE;
}

// 800CF908
static BOOL link(DynamicModuleControl *module) {
    if (module->load_async() && module->link()) {
        return TRUE;
    }
    return FALSE;
}

// 800CF958
BOOL update() {
    int i;
    if (!l_dirty) {
        return TRUE;
    }
    l_dirty = FALSE;
    for (i = 0; i < MENU_RES_NUM; i++) {
        if (l_refCount[i] == 0 && l_module[i]->isLinked()) {
            l_module[i]->forceUnlink();
        }
    }
    for (i = 0; i < MENU_RES_NUM; i++) {
        if (l_refCount[i] != 0 && !l_module[i]->isLinked() && !link(l_module[i])) {
            l_dirty = TRUE;
        }
    }
    return !l_dirty;
}

// 800CFA5C
void addRef(u8 res) {
    if (l_refCount[res] == 0) {
        l_dirty = TRUE;
    }
    if (l_refCount[res] < MENU_RES_REF_MAX) {
        l_refCount[res]++;
    }
}

// 800CFA90
void release(u8 res) {
    if (l_refCount[res] != 0) {
        l_refCount[res]--;
    }
    if (l_refCount[res] == 0) {
        l_dirty = TRUE;
    }
}

// 800CFAC4
void addMenuRef(u8 menu) {
    for (u8 *res = l_menuRes[menu]; *res != MENU_RES_END; res++) {
        addRef(*res);
    }
}

// 800CFB10
void releaseMenuRef(u8 menu) {
    for (u8 *res = l_menuRes[menu]; *res != MENU_RES_END; res++) {
        release(*res);
    }
}

} // namespace dMenuRes
