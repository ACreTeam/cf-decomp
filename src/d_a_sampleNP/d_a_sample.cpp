// An empty sample actor (REL d_a_sampleNP). See
// include/game/game/d_a_sample.hpp.
#include <game/game/d_a_sample.hpp>
#include <game/framework/f_profile.hpp>

namespace dASample {
// .text:0x5C
BASE_PROFILE(A_SAMPLE, dAcSample_c);
} // namespace dASample

// .text:0xA8
int dAcSample_c::create() {
    return SUCCEEDED;
}

// .text:0xB0
int dAcSample_c::doDelete() {
    return SUCCEEDED;
}

// .text:0xB8
int dAcSample_c::execute() {
    return SUCCEEDED;
}

// .text:0xC0
int dAcSample_c::draw() {
    return SUCCEEDED;
}

// .text:0xC8
dAcSample_c::~dAcSample_c() {
}
