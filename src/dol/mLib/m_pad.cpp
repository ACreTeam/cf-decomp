// Based on the Skyward Sword decompilation (zeldaret/ss), m/m_pad.cpp
#include <game/mLib/m_pad.hpp>

namespace mPad {

EGG::CoreControllerMgr *g_padMg;
int g_currentCoreId;
EGG::CoreController *g_currentCore;
EGG::CoreController *g_core[4];

static bool g_IsConnected[4];

struct PadAdditionalData_t {
    PadAdditionalData_t() {}
    ~PadAdditionalData_t() {}

    EGG::Vector2f v1;
    EGG::Vector2f v2;
    EGG::Vector2f v3;
};

static PadAdditionalData_t g_PadAdditionalData[4];

void create() {
    g_padMg = EGG::CoreControllerMgr::instance();
    beginPad();
    endPad();
}

void beginPad() {
    g_padMg->beginFrame();

    for (int i = 0; i < 4; i++) {
        EGG::CoreController *ctl = g_padMg->getNthController(i);
        g_core[i] = ctl;
        PadAdditionalData_t *dat = &g_PadAdditionalData[i];
        bool *connected = &g_IsConnected[i];
        if (ctl->isConnected()) {
            f32 y = ctl->getCoreStatus()->acc_vertical.y;
            f32 x = ctl->getCoreStatus()->acc_vertical.x;
            EGG::Vector2f pos(x, y);
            EGG::Vector2f v = pos - dat->v1;
            dat->v1 = pos;
            dat->v3 = v - dat->v2;
            dat->v2 = v;

            if (!*connected) {
                *connected = true;
            }
        } else if (*connected) {
            ctl->getCoreStatus()->init();
            ctl->sceneReset();
            dat->v1.x = 0.0f;
            dat->v1.y = 0.0f;
            dat->v3.x = 0.0f;
            dat->v3.y = 0.0f;
            dat->v2.x = 0.0f;
            dat->v2.y = 0.0f;
            *connected = false;
        }

    }

    g_currentCore = g_core[g_currentCoreId];
}

void endPad() {
    g_padMg->endFrame();
}

} // namespace mPad
