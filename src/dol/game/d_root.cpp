// The root process (dRoot_c). .text 801B96A4..801B97B4, .data 804FC9A0..804FC9F8, .sdata 8074C330..8074C348,
// .sbss 8074EB88..8074EB90, .sdata2 807519A0..807519A8. See include/game/game/d_root.hpp.
#include <game/game/d_root.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_scene.hpp>

CUSTOM_BASE_PROFILE(ROOT, dRoot_c, 0, 0x12B);

static const ProfileName l_childProfiles[] = {0x129};

dRoot_c *dRoot_c::sInstance;

// 801B96F0
int dRoot_c::create() {
    sInstance = this;
    for (int i = 0; i < ARRAY_SIZE(l_childProfiles); i++) {
        dBase_c::createBase(l_childProfiles[i], sInstance, 0, 0);
    }
    return 1;
}

// 801B972C
int dRoot_c::doDelete() {
    sInstance = NULL;
    return 1;
}

// 801B973C
int dRoot_c::execute() {
    return 1;
}

// 801B9744
int dRoot_c::draw() {
    return 1;
}

// 801B974C
dRoot_c::~dRoot_c() {}

// 801B97A8
dSceneChange_c *dRoot_c::getSceneChange() {
    return &gSceneChange;
}
