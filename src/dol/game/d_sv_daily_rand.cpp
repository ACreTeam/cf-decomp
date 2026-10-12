// The daily random value (dSvDailyRand::Rand_c). .text 80153818..801538F8, .ctors 80465744, .sbss 8074E7F0..8074E7F8.
// See include/game/game/d_sv_daily_rand.hpp.
#include <game/game/d_sv_daily_rand.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_net.hpp>

namespace dSvDailyRand {

// 80153818
void Rand_c::init() {
    mValue = cM::rndInt(DAILY_RAND_NUM);
}

// 8015384C
void Rand_c::reset() {
    mValue = DAILY_RAND_NUM;
    change();
}

// 80153858
void Rand_c::change() {
    if (mValue == DAILY_RAND_NUM) {
        mValue = cM::rndInt(DAILY_RAND_NUM);
    } else {
        u8 value = cM::rndInt(DAILY_RAND_NUM - 1);
        if (value < mValue) {
            mValue = value;
        } else {
            mValue = value + 1;
        }
    }
    if (dAcNpc_c::isMultiPlay()) {
        dNet::beginPacket();
        dNet::addPacket(this, sizeof(mValue));
        dNet::sendPacket(NET_PACKET_DAILY_RAND, NET_MEMBER_NONE);
    }
}

static Rand_c sRand(DAILY_RAND_NUM); // 8074E7F0

} // namespace dSvDailyRand
