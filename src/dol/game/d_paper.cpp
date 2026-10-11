// Stationery (letter paper) picks for villagers. .text 800FABF4..800FACC0, .rodata 80475768..804759F0,
// .data 804ED830..804ED848, .sdata2 807509C0..80750A60. See include/game/game/d_paper.hpp.
#include <game/game/d_paper.hpp>
#include <game/cLib/c_math.hpp>

namespace dPaper {

#define SEASON_LIST_NUM 5 // the all-year list, then one per season
#define LOOKS_NUM 6
#define SEASON_CHANCE 30.0f // percent

// A list of stationery item ids.
struct List_s {
    const u16 *mItems;
    u32 mNum;
};

static const u16 sPaper0_0[] = {0x9660, 0x9664, 0x966C, 0x9674, 0x967C, 0x9690, 0x9694, 0x9698, 0x96C4, 0x96C8, 0x96CC, 0x96D0, 0x96D4, 0x96D8, 0x96DC, 0x96E0, 0x96E4, 0x96E8, 0x96F8, 0x9700, 0x9704, 0x9708, 0x970C, 0x9710, 0x9718, 0x971C, 0x9720, 0x9724, 0x9728, 0x972C, 0x9730, 0x9734, 0x9738, 0x973C};
static const u16 sPaper0_1[] = {0x9640, 0x9654, 0x96AC};
static const u16 sPaper0_2[] = {0x9640, 0x9650, 0x96AC, 0x96EC};
static const u16 sPaper0_3[] = {0x965C, 0x96F4};
static const u16 sPaper0_4[] = {0x9658, 0x9678};
static const List_s sLists0[SEASON_LIST_NUM] = {
    {sPaper0_0, ARRAY_SIZE(sPaper0_0)},
    {sPaper0_1, ARRAY_SIZE(sPaper0_1)},
    {sPaper0_2, ARRAY_SIZE(sPaper0_2)},
    {sPaper0_3, ARRAY_SIZE(sPaper0_3)},
    {sPaper0_4, ARRAY_SIZE(sPaper0_4)},
};

static const u16 sPaper1_0[] = {0x9660, 0x9664, 0x967C, 0x968C, 0x9690, 0x9694, 0x9698, 0x96C4, 0x96C8, 0x96D0, 0x96D4, 0x96D8, 0x96DC, 0x96E0, 0x96E4, 0x96E8, 0x96F8, 0x9700, 0x9704, 0x9708, 0x970C, 0x9710, 0x9720, 0x9724, 0x972C, 0x9730, 0x973C};
static const u16 sPaper1_1[] = {0x9640, 0x9654, 0x96AC};
static const u16 sPaper1_2[] = {0x9640, 0x9650, 0x96AC, 0x96EC};
static const u16 sPaper1_3[] = {0x965C, 0x96F4};
static const u16 sPaper1_4[] = {0x9658, 0x9678};
static const List_s sLists1[SEASON_LIST_NUM] = {
    {sPaper1_0, ARRAY_SIZE(sPaper1_0)},
    {sPaper1_1, ARRAY_SIZE(sPaper1_1)},
    {sPaper1_2, ARRAY_SIZE(sPaper1_2)},
    {sPaper1_3, ARRAY_SIZE(sPaper1_3)},
    {sPaper1_4, ARRAY_SIZE(sPaper1_4)},
};

static const u16 sPaper2_0[] = {0x9660, 0x9664, 0x967C, 0x9688, 0x968C, 0x9690, 0x9694, 0x96C4, 0x96C8, 0x96D0, 0x96D4, 0x96D8, 0x96DC, 0x96E0, 0x96E4, 0x96E8, 0x96F8, 0x9700, 0x9704, 0x970C, 0x9710, 0x9718, 0x9724, 0x9728, 0x972C, 0x9730, 0x9734, 0x9738, 0x973C};
static const u16 sPaper2_1[] = {0x9640, 0x9654, 0x96AC};
static const u16 sPaper2_2[] = {0x9640, 0x9650, 0x96AC, 0x96EC};
static const u16 sPaper2_3[] = {0x965C, 0x96F4};
static const u16 sPaper2_4[] = {0x9658, 0x9678};
static const List_s sLists2[SEASON_LIST_NUM] = {
    {sPaper2_0, ARRAY_SIZE(sPaper2_0)},
    {sPaper2_1, ARRAY_SIZE(sPaper2_1)},
    {sPaper2_2, ARRAY_SIZE(sPaper2_2)},
    {sPaper2_3, ARRAY_SIZE(sPaper2_3)},
    {sPaper2_4, ARRAY_SIZE(sPaper2_4)},
};

static const u16 sPaper3_0[] = {0x964C, 0x9660, 0x9664, 0x9668, 0x966C, 0x9674, 0x967C, 0x9690, 0x969C, 0x96C0, 0x96C4, 0x96CC, 0x96D0, 0x96D4, 0x96D8, 0x96DC, 0x96E0, 0x96E4, 0x96E8, 0x96F0, 0x96F8, 0x96FC, 0x9700, 0x9708, 0x970C, 0x9710, 0x9714, 0x9718, 0x971C, 0x9720, 0x9728, 0x9730, 0x9734, 0x9738, 0x973C};
static const u16 sPaper3_1[] = {0x9640, 0x9654, 0x96AC};
static const u16 sPaper3_2[] = {0x9640, 0x9650, 0x96AC, 0x96EC};
static const u16 sPaper3_3[] = {0x965C, 0x96F4};
static const u16 sPaper3_4[] = {0x9658, 0x9678};
static const List_s sLists3[SEASON_LIST_NUM] = {
    {sPaper3_0, ARRAY_SIZE(sPaper3_0)},
    {sPaper3_1, ARRAY_SIZE(sPaper3_1)},
    {sPaper3_2, ARRAY_SIZE(sPaper3_2)},
    {sPaper3_3, ARRAY_SIZE(sPaper3_3)},
    {sPaper3_4, ARRAY_SIZE(sPaper3_4)},
};

static const u16 sPaper4_0[] = {0x9660, 0x9664, 0x9668, 0x966C, 0x9674, 0x967C, 0x9690, 0x969C, 0x96C0, 0x96C4, 0x96CC, 0x96D0, 0x96D4, 0x96D8, 0x96DC, 0x96E0, 0x96E4, 0x96E8, 0x96F0, 0x96F8, 0x96FC, 0x9700, 0x9704, 0x9708, 0x970C, 0x9710, 0x9714, 0x9718, 0x971C, 0x9720, 0x972C, 0x9730, 0x9734, 0x973C};
static const u16 sPaper4_1[] = {0x9640, 0x9654, 0x96AC};
static const u16 sPaper4_2[] = {0x9640, 0x9650, 0x96AC, 0x96EC};
static const u16 sPaper4_3[] = {0x965C, 0x96F4};
static const u16 sPaper4_4[] = {0x9658, 0x9678};
static const List_s sLists4[SEASON_LIST_NUM] = {
    {sPaper4_0, ARRAY_SIZE(sPaper4_0)},
    {sPaper4_1, ARRAY_SIZE(sPaper4_1)},
    {sPaper4_2, ARRAY_SIZE(sPaper4_2)},
    {sPaper4_3, ARRAY_SIZE(sPaper4_3)},
    {sPaper4_4, ARRAY_SIZE(sPaper4_4)},
};

static const u16 sPaper5_0[] = {0x964C, 0x9660, 0x9664, 0x9668, 0x9674, 0x967C, 0x9688, 0x968C, 0x969C, 0x96C0, 0x96C4, 0x96CC, 0x96D0, 0x96D4, 0x96D8, 0x96DC, 0x96E0, 0x96E4, 0x96E8, 0x96F0, 0x96F8, 0x96FC, 0x9700, 0x9704, 0x9708, 0x970C, 0x9710, 0x9714, 0x9718, 0x971C, 0x9720, 0x9728, 0x9730, 0x9734, 0x9738, 0x973C};
static const u16 sPaper5_1[] = {0x9640, 0x9654, 0x96AC};
static const u16 sPaper5_2[] = {0x9640, 0x9650, 0x96AC, 0x96EC};
static const u16 sPaper5_3[] = {0x965C, 0x96F4};
static const u16 sPaper5_4[] = {0x9658, 0x9678};
static const List_s sLists5[SEASON_LIST_NUM] = {
    {sPaper5_0, ARRAY_SIZE(sPaper5_0)},
    {sPaper5_1, ARRAY_SIZE(sPaper5_1)},
    {sPaper5_2, ARRAY_SIZE(sPaper5_2)},
    {sPaper5_3, ARRAY_SIZE(sPaper5_3)},
    {sPaper5_4, ARRAY_SIZE(sPaper5_4)},
};
static const List_s *sLists[LOOKS_NUM] = {sLists0, sLists1, sLists2, sLists3, sLists4, sLists5};

// 800FABF4
u16 getPaper(u8 looks, u32 season) {
    if (looks < LOOKS_NUM && season < 4) {
        u32 list = 0;
        if (cM::rndF(100.0f) < SEASON_CHANCE) {
            list = season + 1;
        }
        if (list > SEASON_LIST_NUM) {
            list = 0;
        }
        const List_s *lists = sLists[looks];
        return lists[list].mItems[(u32)cM::rndF(lists[list].mNum)];
    }
    return 0xFFF1;
}

// 800FACBC
void fn_800FACBC() {}

} // namespace dPaper
