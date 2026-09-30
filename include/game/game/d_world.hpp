#pragma once

#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>

// City Folk's field-to-render coordinate conversion. Names are inferred.
namespace dWorld {
    // Applies field curvature (including the distant-field adjustment) and
    // returns the X rotation needed to orient a model on the curved surface.
    // With curvature disabled, copies position unchanged and returns zero.
    mAng curvePosition(mVec3_c *result, const mVec3_c *position);
}
