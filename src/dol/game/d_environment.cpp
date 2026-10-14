// d_environment: the environment lighting (time of day, weather), the fog and the fireworks.

#include <game/game/d_environment.hpp>
#include <egg/math/eggMath.h>
#include <game/framework/f_profile.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_sky.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_weather.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/sLib/s_lib.hpp>
#include <revolution/MTX.h>

#define CLAMP_COLOR(v) ((v) < 0.0f ? 0.0f : ((v) > 255.0f ? 255.0f : (v)))
#define MIN_255(v) ((v) > 255 ? 255 : (v))

static inline f32 clampColor(f32 value) {
    return CLAMP_COLOR(value);
}

CUSTOM_BASE_PROFILE(ENVIRONMENT, dEnvironment_c, 0x12B, 0xE8);

static const u8 l_starAlpha[] = {0xFF, 0x90, 0x48, 0x20, 0x00, 0x20, 0x00};

static const GXColor l_particleColor0[24] = {
    {0xE6, 0xDC, 0xF0, 0xFF}, {0xE6, 0xDC, 0xF0, 0xFF}, {0xE6, 0xDC, 0xF0, 0xFF}, {0xDC, 0xD2, 0xF5, 0xFF},
    {0xD2, 0xC8, 0xF5, 0xFF}, {0xCD, 0xC8, 0xF5, 0xFF}, {0xCD, 0xF5, 0xF5, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFA, 0xEB, 0xFF}, {0xF5, 0xD7, 0xE1, 0xFF}, {0xFF, 0xC8, 0xE6, 0xFF}, {0xC8, 0xC8, 0xF0, 0xFF},
    {0xD7, 0xD2, 0xF0, 0xFF}, {0xD7, 0xD2, 0xF0, 0xFF}, {0xDC, 0xD2, 0xF0, 0xFF}, {0xE6, 0xDC, 0xF0, 0xFF},
};
static const GXColor l_particleColor1[24] = {
    {0xF0, 0xE6, 0xF0, 0xFF}, {0xEB, 0xE6, 0xF5, 0xFF}, {0xE1, 0xDC, 0xF0, 0xFF}, {0xE1, 0xDC, 0xFA, 0xFF},
    {0xD7, 0xD2, 0xF5, 0xFF}, {0xD7, 0xD2, 0xF5, 0xFF}, {0xDC, 0xD2, 0xF5, 0xFF}, {0xE6, 0xDC, 0xFA, 0xFF},
    {0xEB, 0xF0, 0xFA, 0xFF}, {0xEB, 0xF0, 0xFA, 0xFF}, {0xEB, 0xF0, 0xFA, 0xFF}, {0xEB, 0xF0, 0xFA, 0xFF},
    {0xEB, 0xF0, 0xFA, 0xFF}, {0xEB, 0xF0, 0xFA, 0xFF}, {0xEB, 0xF0, 0xFA, 0xFF}, {0xEB, 0xF0, 0xFA, 0xFF},
    {0xEB, 0xF0, 0xFA, 0xFF}, {0xE6, 0xE1, 0xF5, 0xFF}, {0xDC, 0xD7, 0xFA, 0xFF}, {0xD7, 0xD2, 0xFF, 0xFF},
    {0xD7, 0xD2, 0xFF, 0xFF}, {0xDC, 0xD7, 0xFA, 0xFF}, {0xE1, 0xDC, 0xF5, 0xFF}, {0xE6, 0xE1, 0xF5, 0xFF},
};
static const GXColor *l_particleColor[2] = {l_particleColor0, l_particleColor1};

static const GXColor l_sunColor0[24] = {
    {0x00, 0x00, 0x00, 0xFF}, {0x00, 0x00, 0x04, 0xFF}, {0x00, 0x04, 0x08, 0xFF}, {0x00, 0x04, 0x0C, 0xFF},
    {0x00, 0x08, 0x15, 0xFF}, {0x29, 0x42, 0x3E, 0xFF}, {0x53, 0x80, 0x67, 0xFF}, {0x6F, 0x80, 0x67, 0xFF},
    {0x80, 0x80, 0x67, 0xFF}, {0x80, 0x80, 0x67, 0xFF}, {0x80, 0x80, 0x67, 0xFF}, {0x80, 0x80, 0x67, 0xFF},
    {0x80, 0x80, 0x67, 0xFF}, {0x80, 0x80, 0x5B, 0xFF}, {0x80, 0x7C, 0x4A, 0xFF}, {0x80, 0x7C, 0x3E, 0xFF},
    {0x80, 0x7C, 0x31, 0xFF}, {0x80, 0x57, 0x19, 0xFF}, {0x80, 0x42, 0x00, 0xFF}, {0x36, 0x2D, 0x00, 0xFF},
    {0x15, 0x15, 0x00, 0xFF}, {0x0C, 0x0C, 0x00, 0xFF}, {0x08, 0x08, 0x00, 0xFF}, {0x04, 0x04, 0x00, 0xFF},
};
static const GXColor l_sunColor1[24] = {
    {0x00, 0x00, 0x00, 0xFF}, {0x04, 0x00, 0x11, 0xFF}, {0x08, 0x04, 0x21, 0xFF}, {0x0C, 0x04, 0x2D, 0xFF},
    {0x11, 0x08, 0x3E, 0xFF}, {0x21, 0x15, 0x4A, 0xFF}, {0x2D, 0x29, 0x57, 0xFF}, {0x3A, 0x3E, 0x5F, 0xFF},
    {0x3E, 0x53, 0x67, 0xFF}, {0x3E, 0x53, 0x67, 0xFF}, {0x3E, 0x53, 0x67, 0xFF}, {0x3E, 0x53, 0x67, 0xFF},
    {0x3E, 0x53, 0x67, 0xFF}, {0x3E, 0x53, 0x67, 0xFF}, {0x3E, 0x53, 0x67, 0xFF}, {0x3E, 0x53, 0x67, 0xFF},
    {0x3E, 0x53, 0x67, 0xFF}, {0x3E, 0x46, 0x5B, 0xFF}, {0x2D, 0x2D, 0x4E, 0xFF}, {0x21, 0x21, 0x46, 0xFF},
    {0x19, 0x19, 0x3E, 0xFF}, {0x0C, 0x0C, 0x2D, 0xFF}, {0x08, 0x08, 0x1D, 0xFF}, {0x04, 0x04, 0x0C, 0xFF},
};
static const GXColor *l_sunColor[2] = {l_sunColor0, l_sunColor1};

static const GXColor l_moonColor0[24] = {
    {0x67, 0x80, 0x53, 0xFF}, {0x67, 0x80, 0x53, 0xFF}, {0x67, 0x80, 0x53, 0xFF}, {0x67, 0x80, 0x53, 0xFF},
    {0x67, 0x80, 0x53, 0xFF}, {0x3E, 0x53, 0x42, 0xFF}, {0x15, 0x21, 0x2D, 0xFF}, {0x08, 0x11, 0x19, 0xFF},
    {0x00, 0x04, 0x08, 0xFF}, {0x00, 0x04, 0x04, 0xFF}, {0x00, 0x00, 0x04, 0xFF}, {0x00, 0x00, 0x00, 0xFF},
    {0x00, 0x00, 0x00, 0xFF}, {0x00, 0x00, 0x04, 0xFF}, {0x00, 0x00, 0x08, 0xFF}, {0x04, 0x00, 0x0C, 0xFF},
    {0x04, 0x00, 0x11, 0xFF}, {0x11, 0x08, 0x25, 0xFF}, {0x25, 0x15, 0x3A, 0xFF}, {0x3E, 0x53, 0x46, 0xFF},
    {0x67, 0x80, 0x53, 0xFF}, {0x67, 0x80, 0x53, 0xFF}, {0x67, 0x80, 0x53, 0xFF}, {0x67, 0x80, 0x53, 0xFF},
};
static const GXColor l_moonColor1[24] = {
    {0x5F, 0x74, 0x53, 0xFF}, {0x5F, 0x74, 0x53, 0xFF}, {0x5F, 0x74, 0x53, 0xFF}, {0x5F, 0x74, 0x53, 0xFF},
    {0x5F, 0x74, 0x53, 0xFF}, {0x2D, 0x4A, 0x3E, 0xFF}, {0x08, 0x1D, 0x25, 0xFF}, {0x00, 0x0C, 0x11, 0xFF},
    {0x00, 0x04, 0x08, 0xFF}, {0x00, 0x04, 0x04, 0xFF}, {0x00, 0x00, 0x04, 0xFF}, {0x00, 0x00, 0x00, 0xFF},
    {0x00, 0x00, 0x00, 0xFF}, {0x00, 0x00, 0x04, 0xFF}, {0x00, 0x00, 0x08, 0xFF}, {0x04, 0x00, 0x0C, 0xFF},
    {0x04, 0x00, 0x11, 0xFF}, {0x04, 0x00, 0x19, 0xFF}, {0x0C, 0x08, 0x25, 0xFF}, {0x3B, 0x3E, 0x3E, 0xFF},
    {0x5F, 0x74, 0x53, 0xFF}, {0x5F, 0x74, 0x53, 0xFF}, {0x5F, 0x74, 0x53, 0xFF}, {0x5F, 0x74, 0x53, 0xFF},
};
static const GXColor *l_moonColor[2] = {l_moonColor0, l_moonColor1};

static const GXColor l_ambColor0[24] = {
    {0x29, 0x29, 0xA5, 0xFF}, {0x29, 0x29, 0xA5, 0xFF}, {0x29, 0x29, 0xA5, 0xFF}, {0x29, 0x29, 0xA5, 0xFF},
    {0x29, 0x29, 0xA5, 0xFF}, {0x31, 0x31, 0x9C, 0xFF}, {0x39, 0x39, 0x9C, 0xFF}, {0x52, 0x52, 0x9C, 0xFF},
    {0x63, 0x63, 0x9C, 0xFF}, {0x63, 0x63, 0x9C, 0xFF}, {0x5A, 0x5A, 0x9C, 0xFF}, {0x5A, 0x5A, 0x9C, 0xFF},
    {0x52, 0x52, 0x9C, 0xFF}, {0x52, 0x52, 0x94, 0xFF}, {0x52, 0x52, 0x94, 0xFF}, {0x52, 0x52, 0x94, 0xFF},
    {0x52, 0x52, 0x94, 0xFF}, {0x4A, 0x4A, 0x94, 0xFF}, {0x39, 0x39, 0x9C, 0xFF}, {0x29, 0x29, 0x9C, 0xFF},
    {0x29, 0x29, 0xA5, 0xFF}, {0x29, 0x29, 0xA5, 0xFF}, {0x29, 0x29, 0xA5, 0xFF}, {0x29, 0x29, 0xA5, 0xFF},
};
static const GXColor l_ambColor1[24] = {
    {0x42, 0x42, 0xA5, 0xFF}, {0x42, 0x42, 0xA5, 0xFF}, {0x42, 0x42, 0xA5, 0xFF}, {0x42, 0x42, 0xA5, 0xFF},
    {0x42, 0x42, 0xA5, 0xFF}, {0x4A, 0x4A, 0xA5, 0xFF}, {0x52, 0x52, 0x9C, 0xFF}, {0x5A, 0x5A, 0x9C, 0xFF},
    {0x63, 0x63, 0x9C, 0xFF}, {0x63, 0x63, 0x9C, 0xFF}, {0x63, 0x63, 0x9C, 0xFF}, {0x63, 0x63, 0x9C, 0xFF},
    {0x63, 0x63, 0x9C, 0xFF}, {0x63, 0x63, 0x9C, 0xFF}, {0x63, 0x63, 0x9C, 0xFF}, {0x63, 0x63, 0x9C, 0xFF},
    {0x63, 0x63, 0x9C, 0xFF}, {0x5A, 0x5A, 0x9C, 0xFF}, {0x52, 0x52, 0x9C, 0xFF}, {0x4A, 0x4A, 0xA5, 0xFF},
    {0x42, 0x42, 0xA5, 0xFF}, {0x42, 0x42, 0xA5, 0xFF}, {0x42, 0x42, 0xA5, 0xFF}, {0x42, 0x42, 0xA5, 0xFF},
};
static const GXColor *l_ambColor[2] = {l_ambColor0, l_ambColor1};

static const GXColor l_skyColor0_0[24] = {
    {0x00, 0x21, 0x5A, 0xFF}, {0x00, 0x21, 0x5A, 0xFF}, {0x00, 0x21, 0x5A, 0xFF}, {0x00, 0x29, 0x5A, 0xFF},
    {0x08, 0x21, 0x7B, 0xFF}, {0x10, 0x21, 0xA5, 0xFF}, {0x18, 0x21, 0xC6, 0xFF}, {0x21, 0x21, 0xE7, 0xFF},
    {0x21, 0x21, 0xE7, 0xFF}, {0x21, 0x21, 0xE7, 0xFF}, {0x21, 0x21, 0xE7, 0xFF}, {0x21, 0x21, 0xE7, 0xFF},
    {0x21, 0x21, 0xE7, 0xFF}, {0x21, 0x21, 0xE7, 0xFF}, {0x21, 0x29, 0xE7, 0xFF}, {0x29, 0x39, 0xE7, 0xFF},
    {0x39, 0x63, 0xDE, 0xFF}, {0x4A, 0xAD, 0xD6, 0xFF}, {0x63, 0x52, 0x7B, 0xFF}, {0x4A, 0x42, 0x6B, 0xFF},
    {0x29, 0x31, 0x63, 0xFF}, {0x10, 0x29, 0x5A, 0xFF}, {0x00, 0x21, 0x5A, 0xFF}, {0x00, 0x21, 0x5A, 0xFF},
};
static const GXColor l_skyColor0_1[24] = {
    {0x10, 0x29, 0x4A, 0xFF}, {0x10, 0x29, 0x4A, 0xFF}, {0x10, 0x29, 0x4A, 0xFF}, {0x10, 0x29, 0x4A, 0xFF},
    {0x10, 0x29, 0x4A, 0xFF}, {0x29, 0x31, 0x5A, 0xFF}, {0x39, 0x42, 0x63, 0xFF}, {0x39, 0x4A, 0x73, 0xFF},
    {0x39, 0x5A, 0x84, 0xFF}, {0x39, 0x5A, 0x84, 0xFF}, {0x39, 0x5A, 0x84, 0xFF}, {0x39, 0x5A, 0x84, 0xFF},
    {0x39, 0x5A, 0x84, 0xFF}, {0x39, 0x5A, 0x84, 0xFF}, {0x39, 0x5A, 0x84, 0xFF}, {0x39, 0x5A, 0x84, 0xFF},
    {0x39, 0x5A, 0x84, 0xFF}, {0x39, 0x4A, 0x73, 0xFF}, {0x39, 0x42, 0x63, 0xFF}, {0x29, 0x31, 0x5A, 0xFF},
    {0x10, 0x29, 0x4A, 0xFF}, {0x10, 0x29, 0x4A, 0xFF}, {0x10, 0x29, 0x4A, 0xFF}, {0x10, 0x29, 0x4A, 0xFF},
};
static const GXColor *l_skyColor0[2] = {l_skyColor0_0, l_skyColor0_1};

static const GXColor l_skyColor1_0[24] = {
    {0x00, 0x39, 0x84, 0xFF}, {0x00, 0x39, 0x84, 0xFF}, {0x00, 0x39, 0x84, 0xFF}, {0x00, 0x39, 0x84, 0xFF},
    {0x08, 0x4A, 0xA5, 0xFF}, {0x10, 0x5A, 0xC6, 0xFF}, {0x18, 0x73, 0xE7, 0xFF}, {0x21, 0x84, 0xFF, 0xFF},
    {0x21, 0x84, 0xFF, 0xFF}, {0x21, 0x84, 0xFF, 0xFF}, {0x21, 0x84, 0xFF, 0xFF}, {0x21, 0x84, 0xFF, 0xFF},
    {0x21, 0x84, 0xFF, 0xFF}, {0x21, 0x84, 0xFF, 0xFF}, {0x21, 0x84, 0xF7, 0xFF}, {0x29, 0x8C, 0xEF, 0xFF},
    {0x5A, 0xC6, 0xE7, 0xFF}, {0xFF, 0xA5, 0x42, 0xFF}, {0xEF, 0x6B, 0x4A, 0xFF}, {0x73, 0x31, 0x63, 0xFF},
    {0x39, 0x31, 0x73, 0xFF}, {0x00, 0x39, 0x84, 0xFF}, {0x00, 0x39, 0x84, 0xFF}, {0x00, 0x39, 0x84, 0xFF},
};
static const GXColor l_skyColor1_1[24] = {
    {0x18, 0x31, 0x4A, 0xFF}, {0x18, 0x31, 0x4A, 0xFF}, {0x18, 0x31, 0x4A, 0xFF}, {0x18, 0x31, 0x4A, 0xFF},
    {0x18, 0x31, 0x4A, 0xFF}, {0x29, 0x39, 0x5A, 0xFF}, {0x42, 0x4A, 0x6B, 0xFF}, {0x42, 0x5A, 0x84, 0xFF},
    {0x42, 0x6B, 0x94, 0xFF}, {0x42, 0x6B, 0x94, 0xFF}, {0x42, 0x6B, 0x94, 0xFF}, {0x42, 0x6B, 0x94, 0xFF},
    {0x42, 0x6B, 0x94, 0xFF}, {0x42, 0x6B, 0x94, 0xFF}, {0x42, 0x6B, 0x94, 0xFF}, {0x42, 0x6B, 0x94, 0xFF},
    {0x42, 0x6B, 0x94, 0xFF}, {0x42, 0x5A, 0x84, 0xFF}, {0x42, 0x4A, 0x6B, 0xFF}, {0x29, 0x39, 0x5A, 0xFF},
    {0x18, 0x31, 0x4A, 0xFF}, {0x18, 0x31, 0x4A, 0xFF}, {0x18, 0x31, 0x4A, 0xFF}, {0x18, 0x31, 0x4A, 0xFF},
};
static const GXColor *l_skyColor1[2] = {l_skyColor1_0, l_skyColor1_1};

static const GXColor l_cloudColor0_0[24] = {
    {0x52, 0x73, 0x84, 0xFF}, {0x52, 0x73, 0x84, 0xFF}, {0x52, 0x73, 0x84, 0xFF}, {0x52, 0x73, 0x84, 0xFF},
    {0x7B, 0x94, 0xA5, 0xFF}, {0xAD, 0xBD, 0xC6, 0xFF}, {0xD6, 0xDE, 0xE7, 0xFF}, {0xE0, 0xF0, 0xFF, 0xFF},
    {0xE0, 0xF0, 0xFF, 0xFF}, {0xE0, 0xF0, 0xFF, 0xFF}, {0xE0, 0xF0, 0xFF, 0xFF}, {0xE0, 0xF0, 0xFF, 0xFF},
    {0xE0, 0xF0, 0xFF, 0xFF}, {0xE0, 0xF0, 0xFF, 0xFF}, {0xE0, 0xF0, 0xFF, 0xFF}, {0xE0, 0xF0, 0xFF, 0xFF},
    {0xEF, 0xEF, 0xC6, 0xFF}, {0xDE, 0xCE, 0xAD, 0xFF}, {0xCE, 0x94, 0x73, 0xFF}, {0x9C, 0x5A, 0x8C, 0xFF},
    {0x6B, 0x63, 0x8C, 0xFF}, {0x5A, 0x6B, 0x8C, 0xFF}, {0x52, 0x73, 0x84, 0xFF}, {0x52, 0x73, 0x84, 0xFF},
};
static const GXColor l_cloudColor0_1[24] = {
    {0x39, 0x52, 0x6B, 0xFF}, {0x39, 0x52, 0x6B, 0xFF}, {0x39, 0x52, 0x6B, 0xFF}, {0x39, 0x52, 0x6B, 0xFF},
    {0x39, 0x52, 0x6B, 0xFF}, {0x52, 0x63, 0x7B, 0xFF}, {0x6B, 0x73, 0x8C, 0xFF}, {0x7B, 0x84, 0x9C, 0xFF},
    {0x84, 0x94, 0xA5, 0xFF}, {0x84, 0x94, 0xA5, 0xFF}, {0x84, 0x94, 0xA5, 0xFF}, {0x84, 0x94, 0xA5, 0xFF},
    {0x84, 0x94, 0xA5, 0xFF}, {0x84, 0x94, 0xA5, 0xFF}, {0x84, 0x94, 0xA5, 0xFF}, {0x84, 0x94, 0xA5, 0xFF},
    {0x84, 0x94, 0xA5, 0xFF}, {0x7B, 0x84, 0x9C, 0xFF}, {0x6B, 0x73, 0x8C, 0xFF}, {0x52, 0x63, 0x7B, 0xFF},
    {0x39, 0x52, 0x6B, 0xFF}, {0x39, 0x52, 0x6B, 0xFF}, {0x39, 0x52, 0x6B, 0xFF}, {0x39, 0x52, 0x6B, 0xFF},
};
static const GXColor *l_cloudColor0[2] = {l_cloudColor0_0, l_cloudColor0_1};

static const GXColor l_cloudColor1_0[24] = {
    {0x29, 0x4A, 0x6B, 0xFF}, {0x29, 0x4A, 0x6B, 0xFF}, {0x29, 0x4A, 0x6B, 0xFF}, {0x29, 0x4A, 0x6B, 0xFF},
    {0x31, 0x52, 0x94, 0xFF}, {0x39, 0x63, 0xB5, 0xFF}, {0x42, 0x6B, 0xDE, 0xFF}, {0x4A, 0x7B, 0xFF, 0xFF},
    {0x4A, 0x7B, 0xFF, 0xFF}, {0x4A, 0x7B, 0xFF, 0xFF}, {0x4A, 0x7B, 0xFF, 0xFF}, {0x4A, 0x7B, 0xFF, 0xFF},
    {0x4A, 0x7B, 0xFF, 0xFF}, {0x4A, 0x7B, 0xFF, 0xFF}, {0x4A, 0x7B, 0xFF, 0xFF}, {0x4A, 0x7B, 0xFF, 0xFF},
    {0x7B, 0xA5, 0xA5, 0xFF}, {0x8C, 0x8C, 0x8C, 0xFF}, {0x94, 0x52, 0x7B, 0xFF}, {0x42, 0x21, 0x8C, 0xFF},
    {0x31, 0x31, 0x7B, 0xFF}, {0x29, 0x39, 0x73, 0xFF}, {0x29, 0x4A, 0x6B, 0xFF}, {0x29, 0x4A, 0x6B, 0xFF},
};
static const GXColor l_cloudColor1_1[24] = {
    {0x31, 0x39, 0x52, 0xFF}, {0x31, 0x39, 0x52, 0xFF}, {0x31, 0x39, 0x52, 0xFF}, {0x31, 0x39, 0x52, 0xFF},
    {0x31, 0x39, 0x52, 0xFF}, {0x42, 0x4A, 0x63, 0xFF}, {0x52, 0x52, 0x73, 0xFF}, {0x5A, 0x63, 0x84, 0xFF},
    {0x5A, 0x73, 0x94, 0xFF}, {0x5A, 0x73, 0x94, 0xFF}, {0x5A, 0x73, 0x94, 0xFF}, {0x5A, 0x73, 0x94, 0xFF},
    {0x5A, 0x73, 0x94, 0xFF}, {0x5A, 0x73, 0x94, 0xFF}, {0x5A, 0x73, 0x94, 0xFF}, {0x5A, 0x73, 0x94, 0xFF},
    {0x5A, 0x73, 0x94, 0xFF}, {0x5A, 0x63, 0x84, 0xFF}, {0x52, 0x52, 0x73, 0xFF}, {0x42, 0x4A, 0x63, 0xFF},
    {0x31, 0x39, 0x52, 0xFF}, {0x31, 0x39, 0x52, 0xFF}, {0x31, 0x39, 0x52, 0xFF}, {0x31, 0x39, 0x52, 0xFF},
};
static const GXColor *l_cloudColor1[2] = {l_cloudColor1_0, l_cloudColor1_1};

static const GXColor l_color8_0[24] = {
    {0x0C, 0x10, 0x48, 0xFF}, {0x0C, 0x10, 0x48, 0xFF}, {0x0C, 0x10, 0x48, 0xFF}, {0x0C, 0x10, 0x48, 0xFF},
    {0x0C, 0x10, 0x48, 0xFF}, {0x20, 0x38, 0x80, 0xFF}, {0x58, 0x90, 0xFF, 0xFF}, {0xA0, 0xE0, 0xFF, 0xFF},
    {0xA0, 0xE0, 0xFF, 0xFF}, {0xA0, 0xE0, 0xFF, 0xFF}, {0xA0, 0xE0, 0xFF, 0xFF}, {0xA0, 0xE0, 0xFF, 0xFF},
    {0xA0, 0xE0, 0xFF, 0xFF}, {0xA0, 0xE0, 0xFF, 0xFF}, {0x9E, 0xE0, 0xF8, 0xFF}, {0x9C, 0xE0, 0xEC, 0xFF},
    {0x90, 0xE0, 0xE0, 0xFF}, {0xB8, 0x90, 0x58, 0xFF}, {0x80, 0x40, 0x68, 0xFF}, {0x38, 0x30, 0x58, 0xFF},
    {0x18, 0x18, 0x50, 0xFF}, {0x0C, 0x10, 0x48, 0xFF}, {0x0C, 0x10, 0x48, 0xFF}, {0x0C, 0x10, 0x48, 0xFF},
};
static const GXColor l_color8_1[24] = {
    {0x00, 0x1C, 0x30, 0xFF}, {0x00, 0x1C, 0x30, 0xFF}, {0x00, 0x1C, 0x30, 0xFF}, {0x00, 0x1C, 0x30, 0xFF},
    {0x00, 0x1C, 0x30, 0xFF}, {0x10, 0x20, 0x48, 0xFF}, {0x18, 0x30, 0x58, 0xFF}, {0x28, 0x48, 0x68, 0xFF},
    {0x40, 0x60, 0x80, 0xFF}, {0x48, 0x78, 0x80, 0xFF}, {0x48, 0x78, 0x80, 0xFF}, {0x48, 0x78, 0x80, 0xFF},
    {0x48, 0x78, 0x80, 0xFF}, {0x48, 0x78, 0x80, 0xFF}, {0x48, 0x78, 0x80, 0xFF}, {0x48, 0x78, 0x80, 0xFF},
    {0x40, 0x60, 0x80, 0xFF}, {0x28, 0x48, 0x68, 0xFF}, {0x18, 0x30, 0x58, 0xFF}, {0x10, 0x20, 0x48, 0xFF},
    {0x00, 0x1C, 0x30, 0xFF}, {0x00, 0x1C, 0x30, 0xFF}, {0x00, 0x1C, 0x30, 0xFF}, {0x00, 0x1C, 0x30, 0xFF},
};
static const GXColor *l_color8[2] = {l_color8_0, l_color8_1};

static const u8 l_alpha0[24] = {
    0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x00, 0xA5, 0xA5, 0xAD, 0xAD, 0xB5,
    0xB5, 0xBD, 0xBD, 0xBD, 0xAD, 0xA5, 0x00, 0x73, 0x73, 0x73, 0x73, 0x73,
};
static const u8 l_alpha1[24] = {
    0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x00, 0xA0, 0xA0, 0xA8, 0xA8, 0xB0,
    0xB0, 0xB8, 0xB8, 0xB8, 0xB0, 0xA0, 0x00, 0x73, 0x73, 0x73, 0x73, 0x73,
};
static const u8 *l_alpha[2] = {l_alpha0, l_alpha1};

static const f32 l_fogStart0[24] = {
    625.0f, 475.0f, 362.5f, 287.5f, 250.0f, 250.0f,
    250.0f, 437.5f, 625.0f, 775.0f, 887.5f, 962.5f,
    1000.0f, 1000.0f, 1000.0f, 1000.0f, 1000.0f, 1000.0f,
    1000.0f, 1000.0f, 1000.0f, 962.5f, 887.5f, 775.0f,
};
static const f32 l_fogStart1[24] = {
    -125.0f, -225.0f, -300.0f, -350.0f, -375.0f, -375.0f,
    -375.0f, -250.0f, -125.0f, -25.0f, 50.0f, 100.0f,
    125.0f, 125.0f, 125.0f, 125.0f, 125.0f, 125.0f,
    125.0f, 125.0f, 125.0f, 100.0f, 50.0f, -25.0f,
};
static const f32 *l_fogStart[2] = {l_fogStart0, l_fogStart1};

static const GXColor **l_colorTables[dEnvironment_c::COLOR_NUM] = {
    l_sunColor,    l_moonColor,   l_sunColor,    l_ambColor, l_skyColor0,
    l_skyColor1,   l_cloudColor0, l_cloudColor1, l_color8,   l_particleColor,
};

static dLight::AbstractLight_c *(dEnvironment_c::*l_createFunc[dEnvironment_c::LIGHT_TYPE_NUM])(void *) = {
    &dEnvironment_c::createSunLight,
    &dEnvironment_c::createMoonLight,
    &dEnvironment_c::createSubLightRoom,
    &dEnvironment_c::createSubLightField,
    &dEnvironment_c::createEnvAmbLight,
    &dEnvironment_c::createFixAmbLight,
    &dEnvironment_c::createFixAmbSwitchLight,
    &dEnvironment_c::createCheckPAmbLight,
    &dEnvironment_c::createBusAmbLight,
    &dEnvironment_c::createFixFarLight,
    &dEnvironment_c::createFixFarSwitchLight,
    &dEnvironment_c::createFixFarSwitch2Light,
    &dEnvironment_c::createFixSpotLight,
    &dEnvironment_c::createFixSpotSwitchLight,
    &dEnvironment_c::createFixSpotTheaterLight,
    &dEnvironment_c::createResetCenterLight,
    &dEnvironment_c::createCheckPLight,
    &dEnvironment_c::createBusLight,
    &dEnvironment_c::createBusTunnelLight,
    &dEnvironment_c::createEffectLight,
    &dEnvironment_c::createPlayerFollowSpotLight,
    &dEnvironment_c::createSunLightRmIns,
    &dEnvironment_c::createMoonLightRmIns,
    NULL,
    &dEnvironment_c::createBookLight,
    &dEnvironment_c::createBookAmbLight,
};

static dEnvironment_c::LightReq_c l_roomLights[] = {
    {dEnvironment_c::LIGHT_ENV_AMB, NULL},  {dEnvironment_c::LIGHT_SUN, NULL},
    {dEnvironment_c::LIGHT_MOON, NULL},     {dEnvironment_c::LIGHT_SUB_ROOM, NULL},
    {dEnvironment_c::LIGHT_EFFECT, NULL},
};

static const dEnvironment_c::LightReq_c l_fieldLights[] = {
    {dEnvironment_c::LIGHT_ENV_AMB, NULL},  {dEnvironment_c::LIGHT_SUN, NULL},
    {dEnvironment_c::LIGHT_MOON, NULL},     {dEnvironment_c::LIGHT_SUB_FIELD, NULL},
    {dEnvironment_c::LIGHT_EFFECT, NULL},
};

static dFixAmbLight_c::Data_c l_bookAmbLight = {{0x96, 0x96, 0xB4, 0xFF}, 0};
static dFixFarLight_c::Data_c l_bookLight = {{0xFF, 0xFF, 0xFF, 0xFF}, DEG_TO_ANGLE(-10), DEG_TO_ANGLE(15)};

static dEnvironment_c::LightReq_c l_bookLights[] = {
    {dEnvironment_c::LIGHT_BOOK_AMB, &l_bookAmbLight},
    {dEnvironment_c::LIGHT_BOOK, &l_bookLight},
};

dEnvironment_c *dEnvironment_c::sInstance;
f32 dEnvironment_c::sFlashR;
f32 dEnvironment_c::sFlashG;
f32 dEnvironment_c::sFlashB;
f32 dEnvironment_c::sFlashA;
int dSubLightRoom_c::sLightMode;
u8 dSubLightRoom_c::sLightOn;
dFixAmbSwitchLight_c *dFixAmbSwitchLight_c::sInstance;
dCheckPAmbLight_c *dCheckPAmbLight_c::sInstance;
dBusAmbLight_c *dBusAmbLight_c::sInstance;
dFixFarSwitchLight_c *dFixFarSwitchLight_c::sInstance;
dFixFarSwitch2Light_c *dFixFarSwitch2Light_c::sInstance;
dFixSpotSwitchLight_c *dFixSpotSwitchLight_c::sInstance;
dFixSpotTheaterLight_c *dFixSpotTheaterLight_c::sInstance;
dResetCenterLight_c *dResetCenterLight_c::sInstance;
dCheckPLight_c *dCheckPLight_c::sInstance;
dBusLight_c *dBusLight_c::sInstance;
dBusTunnelLight_c *dBusTunnelLight_c::sInstance;
dEffectLight_c *dEffectLight_c::sInstance;

f32 dEnvironment_c::sFlashSpeed = 0.08f;

dSTR::light_c dSubLightRoom_c::sLight;

dLight::AbstractLight_c *dEnvironment_c::createSunLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dSunLight_c();
}

dLight::AbstractLight_c *dEnvironment_c::createMoonLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dMoonLight_c();
}

dLight::AbstractLight_c *dEnvironment_c::createSubLightRoom(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dSubLightRoom_c();
}

dLight::AbstractLight_c *dEnvironment_c::createSubLightField(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dSubLightField_c();
}

dLight::AbstractLight_c *dEnvironment_c::createEnvAmbLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dEnvAmbLight_c();
}

dLight::AbstractLight_c *dEnvironment_c::createFixAmbLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dFixAmbLight_c((dFixAmbLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createFixAmbSwitchLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dFixAmbSwitchLight_c((dFixAmbSwitchLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createCheckPAmbLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dCheckPAmbLight_c((dFixAmbLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createBusAmbLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dBusAmbLight_c((dFixAmbLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createFixFarLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dFixFarLight_c((dFixFarLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createFixFarSwitchLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dFixFarSwitchLight_c((dFixFarSwitchLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createFixFarSwitch2Light(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dFixFarSwitch2Light_c((dFixFarSwitch2Light_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createFixSpotLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dFixSpotLight_c((dFixSpotLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createFixSpotSwitchLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dFixSpotSwitchLight_c((dFixSpotSwitchLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createFixSpotTheaterLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dFixSpotTheaterLight_c((dFixSpotLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createResetCenterLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dResetCenterLight_c((dResetCenterLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createCheckPLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dCheckPLight_c((dFixFarLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createBusLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dBusLight_c((dFixFarLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createBusTunnelLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dBusTunnelLight_c((dBusTunnelLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createEffectLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dEffectLight_c();
}

dLight::AbstractLight_c *dEnvironment_c::createPlayerFollowSpotLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dPlayerFollowSpotLight_c();
}

dLight::AbstractLight_c *dEnvironment_c::createSunLightRmIns(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dSunLightRmIns_c();
}

dLight::AbstractLight_c *dEnvironment_c::createMoonLightRmIns(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dMoonLightRmIns_c();
}

dLight::AbstractLight_c *dEnvironment_c::createBookLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dBookLight_c((dFixFarLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createBookAmbLight(void *data) {
    return new (dHeap::EnvLightHeap_p, 4) dBookAmbLight_c((dFixAmbLight_c::Data_c *)data);
}

dLight::AbstractLight_c *dEnvironment_c::createLight(int type, void *data) {
    return (this->*l_createFunc[type])(data);
}

static void addLightCallback(void *req, void *env) {
    static_cast<dEnvironment_c *>(env)->addLight(static_cast<dEnvironment_c::LightReq_c *>(req));
}

int dEnvironment_c::create() {
    mFireWorks.init(!dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM) ||
                    dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_CHKP));
    fn_800CCCD8();
    if (!fn_800CCDB4(addLightCallback, this)) {
        const LightReq_c *reqs;
        u32 num;
        if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM)) {
            reqs = l_roomLights;
            num = ARRAY_SIZE(l_roomLights);
        } else {
            reqs = l_fieldLights;
            num = ARRAY_SIZE(l_fieldLights);
        }
        for (u32 i = 0; i < num; i++) {
            addLight(&reqs[i]);
        }
    }
    fn_800CCEAC(1);
    for (u32 i = 0; i < ARRAY_SIZE(l_bookLights); i++) {
        addLight(&l_bookLights[i]);
    }
    fn_800CCEBC();
    sInstance = this;
    sFlashR = 0.0f;
    sFlashG = 0.0f;
    sFlashB = 0.0f;
    sFlashA = 0.0f;
    sFlashSpeed = 0.08f;
    return SUCCEEDED;
}

int dEnvironment_c::doDelete() {
    sInstance = NULL;
    sFlashR = 0.0f;
    sFlashG = 0.0f;
    sFlashB = 0.0f;
    sFlashA = 0.0f;
    for (int i = 0; i < LIGHT_MAX; i++) {
        if (mLights[i] != NULL) {
            mLights[i]->~AbstractLight_c();
        }
    }
    dHeap::EnvLightHeap_p->free(3);
    return SUCCEEDED;
}

int dEnvironment_c::execute() {
    mFireWorks.execute();
    for (int i = 0; i < LIGHT_MAX; i++) {
        if (mLights[i] != NULL) {
            mLights[i]->calc();
        }
    }
    int hour = dTime_c::getCurrent()->hour;
    int next = hour + 1;
    if (next >= 24) {
        next -= 24;
    }
    f32 ratio = getHourRatio();
    f32 blend = calcHourBlend(ratio);

    f32 alpha = (1.0f - blend) * getAlphaTable(isCloudy(), next) + blend * getAlphaTable(isNextCloudy(), next);
    mAlpha = alpha * ratio + (1.0f - ratio) * getAlphaTable(isCloudy(), hour);

    f32 fogStart;
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_TOWN) ||
        dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_CHKP) ||
        dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_BUS) || dScene::getCurrentScene() == dScene::SCENE_TOWN) {
        fogStart = (1.0f - blend) * getFogTable(isCloudy(), next) + blend * getFogTable(isNextCloudy(), next);
        fogStart = fogStart * ratio + (1.0f - ratio) * getFogTable(isCloudy(), hour);
    } else {
        fogStart = 512.0f;
    }

    mColor color;
    mColor black(0x000000FF);
    dSky_c *sky = lbl_8074E830;
    if (sky != NULL) {
        color.lerp(sky->mSkyColor1, sky->mSkyColor0, 0.375f);
    } else {
        color = nw4r::ut::Color(0x000000FF);
    }
    mFogStart = fogStart * mLightRatio;
    mFogStart += 512.0f * (1.0f - mLightRatio);

    mFogColor.lerp(black, color, mLightRatio);

    dWeather_c *weather = dWeather_c::sInstance;
    if (weather != NULL) {
        f32 t;
        if (ratio < 50.0f / 60.0f) {
            t = 0.0f;
        } else {
            t = (ratio - 50.0f / 60.0f) / (1.0f - 50.0f / 60.0f);
        }
        mStarAlpha = (1.0f - t) * l_starAlpha[weather->mWeather] + t * l_starAlpha[weather->mNextWeather];
    } else {
        mStarAlpha = 0xFF;
    }
    dEffect::calc();
    return SUCCEEDED;
}

int dEnvironment_c::draw() {
    mFireWorks.draw();
    EGG::Fog *fog = m3d::getFogMgr()->mFog[0];
    fog->mFlags |= EGG::Fog::FLAG_ENABLE;
    fog->mColor = mFogColor;
    fog->mFogType = GX_FOG_PERSP_EXP2;
    fog->mStartZ = mFogStart;
    fog->mEndZ = 2000.0f;
    fog->mNearZ = dCamera_c::getNear();
    fog->mFarZ = dCamera_c::getFar();
    for (int i = 0; i < LIGHT_MAX; i++) {
        if (mLights[i] != NULL) {
            mLights[i]->apply();
        }
    }
    return SUCCEEDED;
}

BOOL dEnvironment_c::isCloudy() {
    BOOL cloudy = FALSE;
    dWeather_c *weather = dWeather_c::sInstance;
    if (weather != NULL && !dWeather_c::isFineKind(weather->mWeather)) {
        cloudy = TRUE;
    }
    return cloudy;
}

BOOL dEnvironment_c::isNextCloudy() {
    BOOL cloudy = FALSE;
    dWeather_c *weather = dWeather_c::sInstance;
    if (weather != NULL && !dWeather_c::isFineKind(weather->mNextWeather)) {
        cloudy = TRUE;
    }
    return cloudy;
}

mAng dEnvironment_c::getSunAngle() {
    dTime_c time = *dTime_c::getCurrent();
    return mAng::fromDegree((((time.hour - 12) * 60 + time.min) * 60 + time.sec) * (-1.0f / 480.0f));
}

mAng dEnvironment_c::getMoonAngle() {
    dTime_c time = *dTime_c::getCurrent();
    f32 deg = (((time.hour - 12) * 60 + time.min) * 60 + time.sec) * (-1.0f / 480.0f);
    if (deg > 0.0f) {
        deg -= 90.0f;
    } else {
        deg += 90.0f;
    }
    return mAng::fromDegree(deg);
}

mAng dEnvironment_c::getCurveAngle() {
    if (dScene::getSceneE848() && dWorld::isCurve()) {
        return dWorld::getCurveAngle(&dCamera_c::sCenter).mAngle - DEG_TO_ANGLE(29);
    }
    return 0;
}

f32 dEnvironment_c::getHourRatio() {
    dTime_c time = *dTime_c::getCurrent();
    return (time.sec + time.min * 60) * (1.0f / 3600.0f);
}

f32 dEnvironment_c::calcHourBlend(f32 ratio) {
    f32 blend = 0.0f;
    if (ratio > 50.0f / 60.0f) {
        blend = (ratio - 50.0f / 60.0f) / (1.0f - 50.0f / 60.0f);
    }
    return blend;
}

f32 dEnvironment_c::getHourBlend() {
    return calcHourBlend(getHourRatio());
}

void dEnvironment_c::calcColor(mColor *color, int type) {
    int hour = dTime_c::getCurrent()->hour;
    int next = hour + 1;
    if (next >= 24) {
        next -= 24;
    }
    if (type == COLOR_8) {
        mColor base;
        f32 ratio = getHourRatio();
        mColor nextColor;
        f32 blend = calcHourBlend(ratio);
        const GXColor *n = getColor(type, isNextCloudy(), next);
        nextColor.lerp(*getColor(type, isCloudy(), next), *n, blend);
        base.lerp(*getColor(type, isCloudy(), hour), nextColor, getHourRatio());
        u8 alpha = getAlpha();
        base.a = alpha;
        if (hour >= 6 && hour < 18) {
            base.a = alpha / 2;
        }
        mColor flash(sFlashR, sFlashG, sFlashB, sFlashA);
        color->Set(MIN_255(flash.r + base.r), MIN_255(flash.g + base.g), MIN_255(flash.b + base.b),
                   MIN_255(flash.a + base.a));
        f32 decay = 1.0f - sFlashSpeed;
        sFlashR *= decay;
        sFlashG *= decay;
        sFlashB *= decay;
        sFlashA *= decay;
    } else {
        f32 ratio = getHourRatio();
        mColor nextColor;
        f32 blend = calcHourBlend(ratio);
        const GXColor *n = getColor(type, isNextCloudy(), next);
        nextColor.lerp(*getColor(type, isCloudy(), next), *n, blend);
        color->lerp(*getColor(type, isCloudy(), hour), nextColor, ratio);
    }
}

void dEnvironment_c::setCheckPRatio(f32 ratio) {
    dCheckPAmbLight_c *amb = dCheckPAmbLight_c::sInstance;
    dCheckPLight_c *light = dCheckPLight_c::sInstance;
    if (ratio < 0.4f) {
        ratio = 0.0f;
    } else if (ratio > 0.8f) {
        ratio = 1.0f;
    } else {
        ratio = (ratio - 0.4f) / 0.4f;
    }
    if (amb != NULL) {
        amb->mRatio = ratio;
    }
    if (light != NULL) {
        light->mRatio = ratio;
    }
    if (sInstance != NULL) {
        sInstance->mLightRatio = ratio;
    }
    fn_80010304(ratio);
}

void dEnvironment_c::setBusRatio(f32 value) {
    dBusAmbLight_c *amb = dBusAmbLight_c::getInstance();
    dBusLight_c *light = dBusLight_c::getInstance();
    dBusTunnelLight_c *tunnel = dBusTunnelLight_c::getInstance();
    if (amb != NULL) {
        amb->mRatio = value;
    }
    if (light != NULL) {
        light->mRatio = value;
    }
    if (tunnel != NULL) {
        tunnel->mRatio = value;
    }
    if (sInstance != NULL) {
        sInstance->mLightRatio = value;
    }
}

u8 dEnvironment_c::getShadowAlpha() {
    if (sInstance == NULL) {
        return 0xFF;
    }
    if (sInstance->mUseFixAlpha) {
        return sInstance->mFixAlpha;
    }
    return getAlpha();
}

u8 dEnvironment_c::getAlpha() {
    if (sInstance == NULL) {
        return 0xFF;
    }
    return sInstance->mAlpha;
}

u8 dEnvironment_c::getStarAlpha() {
    if (sInstance == NULL) {
        return 0xFF;
    }
    return sInstance->mStarAlpha;
}

const GXColor *dEnvironment_c::getColor(int type, int weather, int hour) {
    return &l_colorTables[type][weather][hour];
}

u8 dEnvironment_c::getAlphaTable(int weather, int hour) {
    return l_alpha[weather][hour];
}

f32 dEnvironment_c::getFogTable(int weather, int hour) {
    return l_fogStart[weather][hour];
}

void dEnvLight_c::setDir(const mAng &x, const mAng &y) {
    mDir.set(0.0f, 0.0f, -1.0f);
    mDir.rotX(x);
    mDir.rotY(y);
    mDir.rotX(dEnvironment_c::getCurveAngle());
    mDir.normalize();
}

void dSunLight_c::calc() {
    s16 x;
    // Both branches use the same angle; the curve check is in the original code.
    if (dWorld::isCurve()) {
        x = mAng::fromDegree(-22.5f);
    } else {
        x = mAng::fromDegree(-22.5f);
    }
    setDir(x, dEnvironment_c::getSunAngle());
    dEnvironment_c::calcColor(&mLightColor, dEnvironment_c::COLOR_SUN);
}

void dSunLightRmIns_c::calc() {
    dSunLight_c::calc();
    s16 x = mAng::fromDegree(-45.0f);
    setDir(x, dEnvironment_c::getSunAngle());
    GXColor color = mLightColor;
    mLightColor.r = color.r * 2 > 255 ? 255 : color.r * 2;
    mLightColor.g = color.g * 2 > 255 ? 255 : color.g * 2;
    mLightColor.b = color.b * 2 > 255 ? 255 : color.b * 2;
    mLightColor.a = color.a * 2 > 255 ? 255 : color.a * 2;
}

void dMoonLight_c::calc() {
    s16 x;
    // Both branches use the same angle; the curve check is in the original code.
    if (dWorld::isCurve()) {
        x = mAng::fromDegree(-22.5f);
    } else {
        x = mAng::fromDegree(-22.5f);
    }
    setDir(x, dEnvironment_c::getMoonAngle());
    dEnvironment_c::calcColor(&mLightColor, dEnvironment_c::COLOR_MOON);
}

void dMoonLightRmIns_c::calc() {
    dMoonLight_c::calc();
    s16 x = mAng::fromDegree(-45.0f);
    setDir(x, dEnvironment_c::getMoonAngle());
}

void dSubLightField_c::calc() {
    setDir(mAng::fromDegree(-45.0f), dEnvironment_c::getSunAngle());
    dEnvironment_c::calcColor(&mLightColor, dEnvironment_c::COLOR_SUB_FIELD);
}

dSubLightRoom_c::dSubLightRoom_c() : _20(0.0f), mInit(FALSE) {
    u8 scene = dScene::getCurrentScene();
    if (dScene::isSceneAttr(scene, dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
        int idx = dScene::getSceneAttrIndex(dScene::getCurrentScene(), dScene::SCENE_ATTR_VILLAGER_HOUSE);
        dAnimal_c *animal = dSaveData_c::getAnimalBlock()->getAnimalConst(idx);
        if (animal != NULL) {
            if ((int)animal->mPlace != 1) {
                sLightOn = FALSE;
            } else {
                sLightOn = TRUE;
            }
        } else {
            sLightOn = TRUE;
        }
    } else if (scene == dScene::SCENE_RM_MM_INS) {
        sLightOn = FALSE;
    } else if (!dScene::isSceneAttr(scene, dScene::SCENE_ATTR_PLAYER_HOUSE) || getRoomFromScene(dScene::getCurrentScene()) == 3) {
        sLightOn = TRUE;
    }
}

static GXColor l_subLightColor = {0xEF, 0xEF, 0xBD, 0xFF};

void dSubLightRoom_c::calc() {
    mColor color(l_subLightColor);
    setDir(mAng::fromDegree(-45.0f), mAng::fromDegree(0.0f));
    if (!mInit) {
        mInit = TRUE;
        if (sLightOn) {
            sLight.set(TRUE);
        } else {
            sLight.set(FALSE);
        }
    }
    sLight.update();
    mLightColor.lerp(::mColor(0x000000FF), color, sLight.mValue);
}

void dEnvAmbLight_c::calc() {
    dEnvironment_c::calcColor(&mLightColor, dEnvironment_c::COLOR_AMB);
}

void dFixAmbLight_c::calc() {}

void dFixAmbLight_c::apply() {
    dLight::AmbLight_c::apply();
    dEnvironment_c *env = dEnvironment_c::sInstance;
    if (env != NULL) {
        env->mFixAlpha = mAlpha;
        env->mUseFixAlpha = TRUE;
    }
}

void dFixAmbSwitchLight_c::calc() {
    if (mOn) {
        sLib::chase(&mRatio, 1.0f, mSpeedOn);
    } else {
        sLib::chase(&mRatio, 0.0f, mSpeedOff);
    }
    mLightColor.lerp(mColorOn, mColorOff, mRatio);
}

void dCheckPAmbLight_c::calc() {
    f32 ratio = mRatio;
    f32 inv = 1.0f - ratio;
    u8 alpha = mData->mAlpha;
    dEnvironment_c::calcColor(&mEnvColor, dEnvironment_c::COLOR_AMB);
    f32 r = clampColor(mFixColor.r * inv + mEnvColor.r * ratio);
    f32 g = clampColor(mFixColor.g * inv + mEnvColor.g * ratio);
    f32 b = clampColor(mFixColor.b * inv + mEnvColor.b * ratio);
    f32 a = clampColor(mFixColor.a * inv + mEnvColor.a * ratio);
    mLightColor.r = r;
    mLightColor.g = g;
    mLightColor.b = b;
    mLightColor.a = a;
    if (dEnvironment_c::sInstance != NULL) {
        u8 envAlpha = dEnvironment_c::getAlpha();
        alpha = CLAMP_COLOR(alpha * inv + envAlpha * ratio);
    }
    mAlpha = alpha;
}

void dBusAmbLight_c::calc() {
    f32 ratio = mRatio;
    f32 inv = 1.0f - ratio;
    u8 alpha = mData->mAlpha;
    dEnvironment_c::calcColor(&mEnvColor, dEnvironment_c::COLOR_AMB);
    f32 r = clampColor(mFixColor.r * inv + mEnvColor.r * ratio);
    f32 g = clampColor(mFixColor.g * inv + mEnvColor.g * ratio);
    f32 b = clampColor(mFixColor.b * inv + mEnvColor.b * ratio);
    f32 a = clampColor(mFixColor.a * inv + mEnvColor.a * ratio);
    mLightColor.r = r;
    mLightColor.g = g;
    mLightColor.b = b;
    mLightColor.a = a;
    if (dEnvironment_c::sInstance != NULL) {
        u8 envAlpha = dEnvironment_c::getAlpha();
        alpha = CLAMP_COLOR(alpha * inv + envAlpha * ratio);
    }
    mAlpha = alpha;
}

void dFixFarLight_c::calc() {
    mDir.set(0.0f, 0.0f, -1.0f);
    mDir.rotX(mAngX);
    mDir.rotY(mAngY);
}

void dFixFarSwitchLight_c::calc() {
    mDir.set(0.0f, 0.0f, -1.0f);
    mDir.rotX(mAngX);
    mDir.rotY(mAngY);
    if (mOn) {
        sLib::chase(&mRatio, 1.0f, mSpeedOn);
    } else {
        sLib::chase(&mRatio, 0.0f, mSpeedOff);
    }
    mLightColor.lerp(mColorOn, mColorOff, mRatio);
}

void dFixFarSwitch2Light_c::calc() {
    if (mOn) {
        sLib::chase(&mRatio, 1.0f, mSpeedOn);
    } else {
        sLib::chase(&mRatio, 0.0f, mSpeedOff);
    }
    mAngX = mRatio * mAngOnX + (1.0f - mRatio) * mAngOffX;
    mAngY = mRatio * mAngOnY + (1.0f - mRatio) * mAngOffY;
    mDir.set(0.0f, 0.0f, -1.0f);
    mDir.rotX(mAngX);
    mDir.rotY(mAngY);
    mLightColor.lerp(mColorOn, mColorOff, mRatio);
}

void dFixSpotLight_c::calc() {
    mDir.set(0.0f, 0.0f, -1.0f);
    mDir.rotX(mAngX);
    mDir.rotY(mAngY);
}

void dFixSpotSwitchLight_c::calc() {
    dFixSpotLight_c::calc();
    if (mOn) {
        sLib::chase(&mRatio, 1.0f, mSpeedOn);
    } else {
        sLib::chase(&mRatio, 0.0f, mSpeedOff);
    }
    mLightColor.lerp(::mColor(0x000000FF), mColorOn, mRatio);
}

void dFixSpotTheaterLight_c::calc() {
    dFixSpotLight_c::calc();
    f32 value = mValue;
    f32 min, max;
    if (mMode) {
        min = 0.5f;
        max = 0.9f;
    } else {
        min = 0.2f;
        max = 0.8f;
    }
    if (value < min) {
        mRatio = 0.0f;
    } else if (value > max) {
        mRatio = 1.0f;
    } else {
        mRatio = (value - min) / (max - min);
    }
    mLightColor.lerp(::mColor(0x000000FF), mColorOn, mRatio);
}

void dResetCenterLight_c::calc() {
    mAngY = mWave;
    mDir.set(0.0f, 0.0f, -1.0f);
    mDir.rotX(mAngX);
    mDir.rotY(mAngY);
    if (mOn) {
        sLib::chase(&mRatio, 1.0f, mSpeedOn);
    } else {
        sLib::chase(&mRatio, 0.0f, mSpeedOff);
    }
    mWave += mWaveSpeed;
    f32 wave = nw4r::math::CosIdx(mWave);
    mColor color;
    color.lerp(mColor2, mColor1, 0.5f + 0.5f * wave);
    mLightColor.lerp(::mColor(0x000000FF), color, mRatio);
}

void dCheckPLight_c::calc() {
    mDir.set(0.0f, 0.0f, -1.0f);
    mDir.rotX(mAngX);
    mDir.rotY(mAngY);
    f32 r = 0.0f;
    f32 g = 0.0f;
    f32 b = 0.0f;
    f32 a = 0.0f;
    mColor sun;
    mColor moon;
    dEnvironment_c::calcColor(&sun, dEnvironment_c::COLOR_SUN);
    dEnvironment_c::calcColor(&moon, dEnvironment_c::COLOR_MOON);
    r += 2.0f * sun.r;
    g += 2.0f * sun.g;
    b += 2.0f * sun.b;
    a += 2.0f * sun.a;
    r += moon.r;
    g += moon.g;
    b += moon.b;
    a += moon.a;
    mEnvColor.r = CLAMP_COLOR(r);
    mEnvColor.g = CLAMP_COLOR(g);
    mEnvColor.b = CLAMP_COLOR(b);
    mEnvColor.a = CLAMP_COLOR(a);
    f32 ratio = mRatio;
    f32 inv = 1.0f - ratio;
    r = clampColor(mFixColor.r * inv + mEnvColor.r * ratio);
    g = clampColor(mFixColor.g * inv + mEnvColor.g * ratio);
    b = clampColor(mFixColor.b * inv + mEnvColor.b * ratio);
    a = clampColor(mFixColor.a * inv + mEnvColor.a * ratio);
    mLightColor.r = r;
    mLightColor.g = g;
    mLightColor.b = b;
    mLightColor.a = a;
}

void dBusLight_c::calc() {
    mDir.set(0.0f, 0.0f, -1.0f);
    mDir.rotX(mAngX);
    mDir.rotY(mAngY);
    f32 r = 0.0f;
    f32 g = 0.0f;
    f32 b = 0.0f;
    f32 a = 0.0f;
    mColor sun;
    mColor moon;
    dEnvironment_c::calcColor(&sun, dEnvironment_c::COLOR_SUN);
    dEnvironment_c::calcColor(&moon, dEnvironment_c::COLOR_MOON);
    r += 2.0f * sun.r;
    g += 2.0f * sun.g;
    b += 2.0f * sun.b;
    a += 2.0f * sun.a;
    r += moon.r;
    g += moon.g;
    b += moon.b;
    a += moon.a;
    mEnvColor.r = CLAMP_COLOR(r);
    mEnvColor.g = CLAMP_COLOR(g);
    mEnvColor.b = CLAMP_COLOR(b);
    mEnvColor.a = CLAMP_COLOR(a);
    f32 ratio = mRatio;
    f32 inv = 1.0f - ratio;
    r = clampColor(mFixColor.r * inv + mEnvColor.r * ratio);
    g = clampColor(mFixColor.g * inv + mEnvColor.g * ratio);
    b = clampColor(mFixColor.b * inv + mEnvColor.b * ratio);
    a = clampColor(mFixColor.a * inv + mEnvColor.a * ratio);
    mLightColor.r = r;
    mLightColor.g = g;
    mLightColor.b = b;
    mLightColor.a = a;

    dFixSpotSwitchLight_c *spot = dFixSpotSwitchLight_c::sInstance;
    int hour = dTime_c::getCurrent()->hour;
    if (mRatio < 1.0f) {
        mOff = TRUE;
    }
    if ((hour < 6 || hour >= 18) && !mOff) {
        spot->mOn = TRUE;
    } else {
        spot->mOn = FALSE;
    }
}

void dBusTunnelLight_c::calc() {
    dFixSpotLight_c::calc();
    mLightColor.lerp(mColorOn, ::mColor(0x000000FF), mRatio);
    mTimer++;
    s16 angMax = mAng::fromDegree(7.5f);
    s16 angMin = mAng::fromDegree(82.5f);
    mAng yaw = dCamera_c::sInstance->getYaw();
    f32 cosMin = nw4r::math::CosIdx(angMin);
    f32 cosMax = nw4r::math::CosIdx(angMax);
    f32 t = (nw4r::math::CosIdx(yaw) - cosMin) / (cosMax - cosMin);
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    f32 inv = 1.0f - t;
    int len = 68.0f * t + 40.0f * inv;
    f32 pos = ((mTimer + len) % 80) / 80.0f;
    mPos = (1.0f - pos) * mStart + pos * mEnd;
}

void dPlayerFollowSpotLight_c::calc() {
    mVec3_c *pos = dPlayerMgr::getPosition(4);
    if (pos != NULL) {
        mPos = *pos;
        mPos += mOffset;
    } else {
        mPos = mVec3_c::Zero;
        mLightColor = nw4r::ut::Color(0x000000FF);
    }
    mDir.set(0.0f, 0.0f, -1.0f);
    mDir.rotX(mAngX);
    mDir.rotY(mAngY);
    mDir.rotX(dWorld::curvePositionSimple(&mPos, &mPos));
    mDir.normalize();
}

dEffectLight_c::dEffectLight_c() {
    mLightColor = nw4r::ut::Color(0);
    mGlowNum = 0;
    mInit = FALSE;
    mCenter.set(0.0f, 0.0f, 0.0f);
    mPrevCenter.set(0.0f, 0.0f, 0.0f);
    mGlowColor = nw4r::ut::Color(0);
    mTotalSize = 0.0f;
    mBrightness = 0.0f;
    mScale = 1.0f;
    sInstance = this;
}

dEffectLight_c::~dEffectLight_c() {
    sInstance = NULL;
}

void dEffectLight_c::apply() {
    f32 r = 0.0f;
    f32 g = 0.0f;
    f32 b = 0.0f;
    f32 a = 0.0f;
    f32 weightSum = 0.0f;
    BOOL hasGlow = FALSE;
    mTotalSize = 0.0f;
    mVec3_c center(0.0f, 0.0f, 0.0f);
    mVec3_c viewPos(0.0f, 0.0f, 0.0f);
    dActor_c *player = (dActor_c *)dScene::getScenePlayer();
    if (player != NULL) {
        viewPos = *player->getPosP();
        viewPos.y += 20.0f;
    } else {
        viewPos = dCamera_c::sCenter;
    }

    if (mGlowNum > 0) {
        for (int i = 0; i < mGlowNum; i++) {
            hasGlow = TRUE;
            Glow_c *glow = &mGlows[i];
            f32 size = glow->mSize;
            f32 dist = EGG::Math<f32>::sqrt(PSVECSquareDistance(viewPos, glow->mPos));
            dist = dist < 0.75f * size ? 0.75f * size : dist;
            f32 scale;
            f32 weight;
            if (size < FLT_EPSILON) {
                scale = 1.0f;
                weight = 0.0f;
            } else if (dist < FLT_EPSILON) {
                scale = 1.0f;
                weight = scale;
            } else {
                scale = size / dist;
                weight = scale * (scale * (scale * scale));
            }
            weightSum += weight;
            mTotalSize += scale * (size * scale);
            r += weight * glow->mColor.r;
            g += weight * glow->mColor.g;
            b += weight * glow->mColor.b;
            a += weight * glow->mColor.a;
            center += weight * glow->mPos;
        }
    }

    f32 rate = 0.1f;
    if (!hasGlow || weightSum < FLT_EPSILON) {
        mBrightness *= 0.9f;
        mGlowColor.lerp(mGlowColor, ::mColor(0), 0.25f);
    } else {
        f32 inv = 1.0f / weightSum;
        mCenter.x = center.x * inv;
        mCenter.y = center.y * inv;
        mCenter.z = center.z * inv;
        if (EGG::Math<f32>::sqrt(PSVECSquareDistance(mPrevCenter, mCenter)) > 64.0f &&
            EGG::Math<f32>::sqrt(PSVECSquareDistance(mPrevCenter, viewPos)) <
                EGG::Math<f32>::sqrt(PSVECSquareDistance(mCenter, viewPos)) &&
            EGG::Math<f32>::sqrt(PSVECSquareDistance(mCenter, viewPos)) > 1.25f * mTotalSize) {
            mColor black(0x000000FF);
            mGlowColor.lerp(mGlowColor, black, 0.25f);
            mBrightness *= 0.9f;
            rate = 0.0f;
        } else {
            mBrightness = 0.05f * mTotalSize + 0.95f * mBrightness;
            mGlowColor.Set((u8)clampColor(r * inv), (u8)clampColor(g * inv), (u8)clampColor(b * inv), (u8)clampColor(a * inv));
        }
    }

    if (!mInit) {
        mInit = TRUE;
        mPrevCenter = mCenter;
        mLightColor = mGlowColor;
    } else {
        mPrevCenter = (1.0f - rate) * mPrevCenter + rate * mCenter;
        mLightColor.lerp(mLightColor, mGlowColor, 0.25f);
    }

    f32 brightness = mBrightness * mScale;
    if (brightness > 4.0f) {
        f32 attn = 1.0f / brightness;
        mDistAttn[0] = 1.0f;
        mDistAttn[1] = 0.0f;
        mDistAttn[2] = attn * attn;
        dWorld::curvePosition(&mPos, &mPrevCenter);
        dLight::PointLight_c::apply();
    } else {
        mInit = FALSE;
    }
    mGlowNum = 0;
}


BOOL dEffectLight_c::addGlow(const nw4r::ut::Color *color, const nw4r::math::VEC3 *pos, f32 size) {
    if (mGlowNum >= GLOW_MAX) {
        return FALSE;
    }
    Glow_c *glow = &mGlows[mGlowNum];
    glow->mColor = *color;
    glow->mPos = *pos;
    glow->mSize = size;
    mGlowNum++;
    return TRUE;
}

int dBookLight_c::sLightObjIdx = -1;

void dBookLight_c::regist() {
    dLight::AbstractLight_c::regist();
    sLightObjIdx = mLightIdx;
}

dBookLight_c::~dBookLight_c() {
    sLightObjIdx = -1;
}

void dBookLight_c::apply() {
    dLight::FarLight_c::apply();
}

void dBookAmbLight_c::calc() {}
