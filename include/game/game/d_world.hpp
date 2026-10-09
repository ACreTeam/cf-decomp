#pragma once

#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>

// City Folk's field-to-render coordinate conversion, the "drum": outdoors the field is drawn
// rolled onto a cylinder around the x axis (the rolling-log horizon). Source:
// src/dol/game/d_drum.cpp (.text 8008278C..80082F24; the file name comes from the RTTI string
// "dDrum_hostIO_c"). Names are inferred.
//
// Field z maps to an angle around the x axis: one block width of field z (lbl_80750520) is
// getAngle() (50 degrees), so the drum's radius is getRadius() = 360 * width / (2 * pi * 50).
// Field y is the height above the drum surface.
namespace dWorld {
    BOOL isCurve();                                              // 8008278C
    void setCurve(bool curve);                                   // 80082794
    void clearCurve();                                           // 8008279C

    // Rolls a field position onto the drum and returns its angle around the x axis (zero, and
    // the position copied, while the curve is off).
    mAng curvePositionSimple(mVec3_c *result, const mVec3_c *position);  // 800827A8
    // The inverse: a drum position back to field coordinates.
    void uncurvePosition(mVec3_c *result, const mVec3_c *position);      // 8008288C
    f32 angleToZ(mAng angle);                                    // 80082944
    mAng getCurveAngle(const mVec3_c *position);                 // 8008299C

    // Applies field curvature (including the distant-field adjustment) and
    // returns the X rotation needed to orient a model on the curved surface.
    // With curvature disabled, copies position unchanged and returns zero.
    mAng curvePosition(mVec3_c *result, const mVec3_c *position);         // 800829D4
    void toFieldPosition(mVec3_c *result, const mVec3_c *position);       // 80082B04

    mAng getAngle();                                             // 80082B08: 50 degrees
    f32 getRadius();                                             // 80082B14

    // The point of the horizon seen from the camera at the field position's x (the player's
    // position for NULL). FALSE without a camera or player, or for a NULL result.
    BOOL getHorizonPosition(mVec3_c *result, const mVec3_c *position);    // 80082B1C
    // The horizon angle seen from the camera over the player; ok (optional) tells whether
    // there was one.
    mAng getPlayerHorizonAngle(bool *ok);                         // 80082C00
    // The angle between the eye's direction and its tangent to the drum at the ground height
    // under position (the drum's radius alone for NULL).
    mAng getHorizonAngle(const mVec3_c *eye, const mVec3_c *position);    // 80082C94

    // A far edge of the field for curvePosition (in effect unused: nothing in the DOL moves
    // mPos from zero, and the static instance copies the hostIO values before the hostIO object
    // is constructed, so mParam is zero too). Field z at or below mPos.z - mParam.x is lowered
    // by mParam.y and pulled back in z by mParam.z per unit of distance past that point.
    class edge_c {
    public:
        edge_c();                                                // 80082D74
        edge_c(const mVec3_c &pos);                              // 80082DA8

        /* 0x00 */ mVec3_c mPos;
        /* 0x0C */ mVec3_c mParam; // x: margin, y: drop rate, z: pull rate (hostIO 420, 1/6, 1/15)
    }; // size 0x18

    edge_c *getEdge();                                           // 80082D68
}
