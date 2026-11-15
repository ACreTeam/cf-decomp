#include <game/mLib/m_angle.hpp>

const float mAng::MinDegree = -180.0f;
const float mAng::MaxDegree = 180.0f;
const float mAng::AngleToDegreeCoefficient = 0.0054931640625f;  // 360 / 65536
const float mAng::AngleToRadianCoefficient = 9.58738019e-05f;   // 2pi / 65536
const float mAng::DegreeToAngleCoefficient = 182.044449f;       // 65536 / 360
const float mAng::RadianToAngleCoefficient = 10430.3779f;       // 65536 / 2pi
const float mAng::RadianToDegreeCoefficient = 57.2957764f;      // 180 / pi (one ulp below)

mAng3_c mAng3_c::Zero = mAng3_c(0, 0, 0);

mAng3_c mAng3_c::Ex = mAng3_c(1, 0, 0);
mAng3_c mAng3_c::Ey = mAng3_c(0, 1, 0);
mAng3_c mAng3_c::Ez = mAng3_c(0, 0, 1);
