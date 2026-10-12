// The scene data lists shared by the d_sc_data_* scenes (dScData). A data-only TU:
// .data 804F4BB0..804F4D80, .sdata 8074B5F0..8074B5F8, .sdata2 807513A8..807513B8. See include/game/game/d_sc_data.hpp.
#include <game/game/d_sc_data.hpp>

namespace dScData {

dSceneBaseData_c g_commonBase[] = {
    {0x128, 0}, {0x0A7, 0}, {0x12A, 0}, {0x00C, 0}, {0x0F3, 0}, {0x0B5, 0}, {0x007, 0},
    {0x002, 0}, {0x0F8, 0}, {0x003, 0}, {0x005, 0}, {0x127, 0}, {0x006, 0}, {0x0F7, 0},
    {0x126, 0}, {0x125, 0}, {0x004, 0}, {0x0A9, 0}, {0x0AF, 0}, {0x00A, 0}, {0x00B, 0},
};
const u16 g_commonBaseNum = ARRAY_SIZE(g_commonBase);

u16 g_commonRes[] = {0x009, 0x00C};
const u16 g_commonResNum = ARRAY_SIZE(g_commonRes);

dSceneBaseData_c g_fieldBase[] = {
    {0x0ED, 0}, {0x123, 0}, {0x0EF, 1}, {0x0F0, 0}, {0x12B, 0},
    {0x0AA, 0}, {0x0B3, 0}, {0x0B2, 0}, {0x0AE, 0}, {0x008, 0},
};
const u16 g_fieldBaseNum = ARRAY_SIZE(g_fieldBase);

u16 g_fieldRes[] = {
    0x0EF, 0x0F0, 0x0ED, 0x123, 0x0FA, 0x0FB, 0x104, 0x124, 0x10C, 0x0AA, 0x0AB,
    0x0AC, 0x0B3, 0x0B2, 0x11D, 0x11E, 0x0F4, 0x008, 0x122, 0x121, 0x120,
};
const u16 g_fieldResNum = ARRAY_SIZE(g_fieldRes);

dSceneBaseData_c g_roomBase[] = {
    {0x0EE, 0}, {0x0EF, 0}, {0x0F0, 0}, {0x12B, 1}, {0x0F1, 0}, {0x047, 0}, {0x0AE, 0},
};
const u16 g_roomBaseNum = ARRAY_SIZE(g_roomBase);

u16 g_roomRes[] = {0x0EF, 0x0F0, 0x0EE, 0x0F1, 0x047, 0x021, 0x022, 0x0F6};
const u16 g_roomResNum = ARRAY_SIZE(g_roomRes);

dSceneBaseData_c g_townBase[] = {
    {0x0EC, 0}, {0x0EF, 0}, {0x0F0, 0}, {0x123, 0}, {0x12B, 0}, {0x0AA, 0}, {0x0AE, 0}, {0x008, 0},
};
const u16 g_townBaseNum = ARRAY_SIZE(g_townBase);

u16 g_townRes[] = {
    0x0EF, 0x0F0, 0x0AA, 0x0AB, 0x0AC, 0x0EC, 0x00D, 0x123, 0x0FA, 0x10C, 0x112, 0x0F6, 0x008,
};
const u16 g_townResNum = ARRAY_SIZE(g_townRes);

} // namespace dScData
