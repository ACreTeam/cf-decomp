#include <game/game/d_base.hpp>

int dBase_c::loadAsyncCallback() {
    return FAILED;
}

void dBase_c::unloadCallback() {}

void dBase_c::initLoader() {
    sLoadAsyncCallback = &loadAsyncCallback;
    sUnloadCallback = &unloadCallback;
}

fBase_c *dBase_c::createBase(ProfileName profile, fBase_c *parent, unsigned long param, u8 group) {
    return fBase_c::createChild(profile, parent, param, group);
}

fBase_c *dBase_c::createRoot(ProfileName profile, unsigned long param, u8 group) {
    return fBase_c::createRoot(profile, param, group);
}
