#include "revolution/hbm/nw4hbm/lyt/drawInfo.h"

#include "revolution/hbm/nw4hbm/math.h"

#include <string.h>

namespace nw4hbm {
namespace lyt {

DrawInfo::DrawInfo() :
    mLocationAdjustScale(1.0f, 1.0f),
    mGlobalAlpha(1.0f) {
    memset(&mFlag, 0, sizeof(mFlag));

    math::MTX34Identity(&mViewMtx);
}

DrawInfo::~DrawInfo() {}

} // namespace lyt
} // namespace nw4hbm
