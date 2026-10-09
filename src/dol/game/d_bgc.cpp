// Bg collision geometry primitives (dBGC, d_bgc.hpp). .text 80069D2C..8006C620, .ctors 8046563C,
// .data 804A5508..804A5588, .sdata 80749C40..80749C60, .sbss 8074E1E0..8074E1E8,
// .sdata2 807503F8..80750418. See notes/d_bgc.txt. Function and class member names are inferred.
#include <game/game/d_bgc.hpp>
#include <game/game/d_bgc_sphere.hpp>
#include <game/cLib/c_math.hpp>

namespace dBGC {

// 80069D2C
bool circle_c::checkInsideXZ(const mVec3_c &p) const {
    f32 dx = p.x - mCenter.x;
    f32 dz = p.z - mCenter.z;
    if (EGG::Mathf::abs(dx) > mRadius) {
        return false;
    }
    if (EGG::Mathf::abs(dz) > mRadius) {
        return false;
    }
    return cM::square(dx) + cM::square(dz) <= cM::square(mRadius);
}

// 80069DC0
BOOL column_c::correctSide(mVec3_c *pos, f32 r) const {
    f32 dist = (*pos - mCenter).xzLen();
    if (dist < r + mRadius) {
        if (pos->y < mCenter.y + getHeight()) {
            mVec3_c dir(pos->x - mCenter.x, 0.0f, pos->z - mCenter.z);
            if (dir.normalizeRS()) {
                f32 push = (r + mRadius) - dist;
                pos->x += dir.x * push;
                pos->z += dir.z * push;
                return TRUE;
            }
            pos->z += r + mRadius;
        }
    } else if (dist <= r + mRadius + 0.125f) {
        if (pos->y < mCenter.y + getHeight()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80069F2C
BOOL column_c::correctTop(mVec3_c *pos, const mVec3_c &old) const {
    f32 top = mCenter.y + getHeight();
    if (old.y >= top && pos->y < top && checkInsideXZ(*pos)) {
        pos->y = top;
        return TRUE;
    }
    return FALSE;
}

// 80069FA8
BOOL column_c::crossTop(mVec3_c *pos, const mVec3_c &old) const {
    f32 top = mCenter.y + getHeight();
    mVec3_c start(old.x, old.y, old.z);
    if (start.y > top) {
        mVec3_c end(pos->x, pos->y, pos->z);
        if (end.y < top) {
            mVec3_c dir = end - start;
            if (dir.normalizeRS() && !isZero(dir.y)) {
                f32 t = (top - start.y) / dir.y;
                mVec3_c cross(start.x + dir.x * t, start.y + dir.y * t, start.z + dir.z * t);
                if (checkInsideXZ(cross)) {
                    *pos = cross;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// 8006A0FC
BOOL column_c::crossSide(mVec3_c *pos, const mVec3_c &old) const {
    if (!checkInsideXZ(old)) {
        mVec3_c start(old.x, old.y, old.z);
        f32 r = mRadius;
        f32 h = mHeight;
        mVec3_c end(pos->x, pos->y, pos->z);
        mVec3_c center(mCenter.x, mCenter.y, mCenter.z);
        mVec3_c dir = end - start;
        f32 a = cM::square(dir.x) + cM::square(dir.z);
        if (isZero(a)) {
            return FALSE;
        }
        f32 b = 2.0f * ((dir.x * (start.x - center.x) + dir.z * (start.z - center.z)) / a);
        f32 c = ((cM::square(start.x - center.x) + cM::square(start.z - center.z)) - cM::square(r)) / a;
        f32 disc = b * b - 4.0f * c;
        if (disc < 0.0f) {
            return FALSE;
        }
        f32 s = EGG::Mathf::abs(EGG::Mathf::sqrt(disc));
        f32 top = center.y + h;
        f32 t0 = 0.5f * (-b - s);
        f32 t1 = 0.5f * (-b + s);
        if (isZero(t0) || (t0 >= 0.0f && t0 <= 1.0f)) {
            mVec3_c cross(start.x + t0 * dir.x, start.y + t0 * dir.y, start.z + t0 * dir.z);
            if (cross.y <= top) {
                *pos = cross;
                return TRUE;
            }
        }
        if (isZero(t1) || (t1 >= 0.0f && t1 <= 1.0f)) {
            mVec3_c cross(old.x + t1 * dir.x, old.y + t1 * dir.y, old.z + t1 * dir.z);
            if (cross.y <= top) {
                *pos = cross;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8006A410
BOOL column_c::crossTopBottom(mVec3_c *pos, const mVec3_c &old) const {
    mVec3_c start(old.x, old.y, old.z);
    mVec3_c end(pos->x, pos->y, pos->z);
    mVec3_c dir = end - start;
    if (dir.normalizeRS() && !isZero(dir.y)) {
        f32 top = mCenter.y + getHeight();
        if (start.y > top && end.y < top) {
            f32 t = (top - start.y) / dir.y;
            mVec3_c cross(start.x + dir.x * t, start.y + dir.y * t, start.z + dir.z * t);
            if (checkInsideXZ(cross)) {
                *pos = cross;
                return TRUE;
            }
        } else if (start.y < 0.0f && end.y > 0.0f) {
            f32 t = -start.y / dir.y;
            mVec3_c cross(start.x + dir.x * t, start.y + dir.y * t, start.z + dir.z * t);
            if (checkInsideXZ(cross)) {
                *pos = cross;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8006A5E8
BOOL column_c::crossSideInf(mVec3_c *pos, const mVec3_c &old) const {
    if (!checkInsideXZ(old)) {
        mVec3_c start(old.x, old.y, old.z);
        f32 r = mRadius;
        mVec3_c end(pos->x, pos->y, pos->z);
        mVec3_c center(mCenter.x, mCenter.y, mCenter.z);
        mVec3_c dir = end - start;
        f32 a = cM::square(dir.x) + cM::square(dir.z);
        if (isZero(a)) {
            return FALSE;
        }
        f32 b = 2.0f * ((dir.x * (start.x - center.x) + dir.z * (start.z - center.z)) / a);
        f32 c = ((cM::square(start.x - center.x) + cM::square(start.z - center.z)) - cM::square(r)) / a;
        f32 disc = b * b - 4.0f * c;
        if (disc < 0.0f) {
            return FALSE;
        }
        f32 s = EGG::Mathf::abs(EGG::Mathf::sqrt(disc));
        f32 bottom = mCenter.y;
        f32 top = mCenter.y + getHeight();
        f32 t0 = 0.5f * (-b - s);
        f32 t1 = 0.5f * (-b + s);
        if (isZero(t0) || (t0 >= 0.0f && t0 <= 1.0f)) {
            mVec3_c cross(start.x + t0 * dir.x, start.y + t0 * dir.y, start.z + t0 * dir.z);
            if (cross.y >= bottom && cross.y <= top) {
                *pos = cross;
                return TRUE;
            }
        }
        if (isZero(t1) || (t1 >= 0.0f && t1 <= 1.0f)) {
            mVec3_c cross(start.x + t1 * dir.x, start.y + t1 * dir.y, start.z + t1 * dir.z);
            if (cross.y >= bottom && cross.y <= top) {
                *pos = cross;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8006A904
void line_c::calcDir(mVec3_c *dir) const {
    *dir = mEnd - mStart;
    dir->normalizeRS();
}

// 8006A95C
f32 line_c::calcDistance(const mVec3_c &p) const {
    mVec3_c nearest;
    calcNearest(&nearest, p);
    return nearest.distTo(p);
}

// 8006A9A0
void line_c::set(const mVec3_c &start, const mVec3_c &end) {
    mStart = start;
    mEnd = end;
    calcDir(&mDir);
}

// 8006A9D8
f32 line_c::calcNearestDistance(mVec3_c *out, const mVec3_c &p) const {
    mVec3_c nearest;
    calcNearest(&nearest, p);
    *out = nearest;
    return nearest.distTo(p);
}

// 8006AA3C
void line_c::calcNearest(mVec3_c *out, const mVec3_c &p) const {
    f32 d = -getDir().dot(p);
    f32 t = -(d + getDir().dot(getStart()));
    out->set(t * getDir().x + getStart().x, t * getDir().y + getStart().y,
             t * getDir().z + getStart().z);
}

// 8006AABC
bool line_c::checkBetween(const mVec3_c &p) const {
    f32 dStart = -mDir.dot(mStart);
    f32 dp = mDir.dot(p);
    f32 dEnd = -mDir.dot(mEnd);
    return (dStart + dp) * (dEnd + dp) <= 0.0f;
}

// 8006AB54
poly_c::poly_c() {
    mD = 0.0f;
    mNormal.set(0.0f, 0.0f, 0.0f);
}

// 8006AB78
f32 poly_c::calcY(const mVec3_c &p) const {
    if (isZero(mNormal.x) && isZero(mNormal.z)) {
        return mPos0.y;
    }
    f32 d = mD;
    if (!isZero(mNormal.y)) {
        return -((p.x * mNormal.x + p.z * mNormal.z + d) / mNormal.y);
    }
    return 0.0f;
}

// 8006AC34
BOOL poly_c::correct(mVec3_c *pos, const mVec3_c &old, f32 r) const {
    BOOL hit = FALSE;
    if (correctCross(pos, old, r)) {
        hit = TRUE;
    }
    if (correctFace(pos, old, r)) {
        hit = TRUE;
    }
    if (correctEdge(pos, old, r)) {
        hit = TRUE;
    }
    return hit;
}

// 8006AD04
BOOL poly_c::correctCross(mVec3_c *pos, const mVec3_c &old, f32 r) const {
    BOOL hit = FALSE;
    if (calcDist(*pos) <= 0.0f && calcDist(old) > 0.0f) {
        mVec3_c cross;
        if (crossSeg(&cross, *pos, old)) {
            hit = TRUE;
            pos->x = cross.x + r * mNormal.x;
            pos->y = cross.y + r * mNormal.y;
            pos->z = cross.z + r * mNormal.z;
        }
    }
    return hit;
}

// 8006ADF0
BOOL poly_c::correctFace(mVec3_c *pos, const mVec3_c &old, f32 r) const {
    BOOL hit = FALSE;
    f32 dist = calcDist(*pos);
    if (dist >= 0.0f && dist <= r + 0.125f) {
        if (dist < r) {
            mVec3_c end = *pos + mNormal;
            mVec3_c cross;
            if (crossLine(&cross, *pos, end)) {
                if (dist < r) {
                    f32 push = r - dist;
                    pos->x += push * mNormal.x;
                    pos->y += push * mNormal.y;
                    pos->z += push * mNormal.z;
                }
                hit = TRUE;
            }
        } else {
            hit = TRUE;
        }
    }
    return hit;
}

// 8006AF0C
BOOL poly_c::correctEdge(mVec3_c *pos, const mVec3_c &old, f32 r) const {
    if (calcDist(old) < 0.0f) {
        return FALSE;
    }
    f32 dist = calcDist(*pos);
    if (dist < 0.0f) {
        return FALSE;
    }
    BOOL hit = FALSE;
    if (dist < r) {
        line_c lines[3] = {
            line_c(mPos0, mPos1),
            line_c(mPos1, mPos2),
            line_c(mPos2, mPos0),
        };
        for (line_c *line = lines; line < &lines[3]; line++) {
            mVec3_c nearest;
            f32 lineDist = line->calcNearestDistance(&nearest, *pos);
            if (lineDist < r && line->checkBetween(*pos)) {
                mVec3_c push = *pos - nearest;
                if (!push.normalizeRS()) {
                    push.x = mNormal.x * r;
                    push.y = mNormal.y * r;
                    push.z = mNormal.z * r;
                } else {
                    push *= r - lineDist;
                }
                *pos += push;
                hit = TRUE;
                break;
            }
        }
    }
    return hit;
}

// 8006B0E4
void poly_c::calcNormal(mVec3_c *normal, const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2) {
    mVec3_c e0 = p1 - p0;
    mVec3_c e1 = p2 - p0;
    *normal = e0.cross(e1);
    normal->normalizeRS();
}

// 8006B174
BOOL poly_c::set(const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2) {
    mPos0 = p0;
    mPos1 = p1;
    mPos2 = p2;
    calcNormal(&mNormal, mPos0, mPos1, mPos2);
    mD = calcD();
    return TRUE;
}

// 8006B208
BOOL poly_c::set(const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2, const mVec3_c &normal) {
    mPos0 = p0;
    mPos1 = p1;
    mPos2 = p2;
    mNormal = normal;
    mD = calcD();
    return TRUE;
}

// 8006B29C
BOOL poly_c::checkInsideXZ(const mVec3_c &p) const {
    mVec3_c a = mPos0 - p;
    mVec3_c b = mPos1 - p;
    mVec3_c c = mPos2 - p;
    f32 ab = a.z * b.x - a.x * b.z;
    f32 bc = b.z * c.x - b.x * c.z;
    f32 ca = c.z * a.x - c.x * a.z;
    if ((ab >= 0.0f && bc >= 0.0f && ca >= 0.0f) || (ab <= 0.0f && bc <= 0.0f && ca <= 0.0f)) {
        return TRUE;
    }
    return FALSE;
}

// 8006B398
f32 poly_c::calcD() const {
    return -(mPos0.x * mNormal.x + mPos0.y * mNormal.y + mPos0.z * mNormal.z);
}

// 8006B3CC
f32 poly_c::calcDist(const mVec3_c &p) const {
    f32 d = mD;
    return p.x * mNormal.x + p.y * mNormal.y + p.z * mNormal.z + d;
}

// 8006B404
BOOL poly_c::checkInsideXY(const mVec3_c &p) const {
    mVec3_c a = mPos0 - p;
    mVec3_c b = mPos1 - p;
    mVec3_c c = mPos2 - p;
    f32 ab = a.x * b.y - a.y * b.x;
    f32 bc = b.x * c.y - b.y * c.x;
    f32 ca = c.x * a.y - c.y * a.x;
    if ((ab >= 0.0f && bc >= 0.0f && ca >= 0.0f) || (ab <= 0.0f && bc <= 0.0f && ca <= 0.0f)) {
        return TRUE;
    }
    return FALSE;
}

// 8006B500
BOOL poly_c::checkInsideYZ(const mVec3_c &p) const {
    mVec3_c a = mPos0 - p;
    mVec3_c b = mPos1 - p;
    mVec3_c c = mPos2 - p;
    f32 ab = a.y * b.z - a.z * b.y;
    f32 bc = b.y * c.z - b.z * c.y;
    f32 ca = c.y * a.z - c.z * a.y;
    if ((ab >= 0.0f && bc >= 0.0f && ca >= 0.0f) || (ab <= 0.0f && bc <= 0.0f && ca <= 0.0f)) {
        return TRUE;
    }
    return FALSE;
}

// 8006B5FC
BOOL poly_c::crossSeg(mVec3_c *out, const mVec3_c &a, const mVec3_c &b) const {
    if (calcDist(a) * calcDist(b) < 0.0f) {
        return crossLine(out, a, b);
    }
    return FALSE;
}

// 8006B69C
BOOL poly_c::crossLine(mVec3_c *out, const mVec3_c &a, const mVec3_c &b) const {
    f32 da = calcDist(a);
    f32 db = calcDist(b);
    f32 diff = da - db;
    if (!isZero(diff)) {
        mVec3_c dir = b - a;
        f32 t = da / diff;
        out->set(a.x + t * dir.x, a.y + t * dir.y, a.z + t * dir.z);
        if (!isZero(mNormal.y) && checkInsideXZ(*out)) {
            return TRUE;
        }
        if (!isZero(mNormal.x) && checkInsideYZ(*out)) {
            return TRUE;
        }
        if (!isZero(mNormal.z) && checkInsideXY(*out)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8006B870
void vecXZ_c::set(const vecXZ_c &v) {
    set(v.x, v.z);
}

// 8006B884
vecXZ_c vecXZ_c::operator+(const vecXZ_c &v) const {
    return vecXZ_c(x + v.x, z + v.z);
}

// 8006B8B8
void vecXZ_c::operator+=(const vecXZ_c &v) {
    x += v.x;
    z += v.z;
}

// 8006B8DC
vecXZ_c vecXZ_c::operator-(const vecXZ_c &v) const {
    return vecXZ_c(x - v.x, z - v.z);
}

// 8006B910
void vecXZ_c::operator*=(f32 f) {
    x *= f;
    z *= f;
}

// 8006B92C
f32 vecXZ_c::getSquareDistance(const vecXZ_c &v) const {
    vecXZ_c diff(x - v.x, z - v.z);
    return diff.x * diff.x + diff.z * diff.z;
}

// 8006B964
BOOL vecXZ_c::normalize() {
    f32 len = EGG::Mathf::sqrt(getSquareDistance(Zero));
    if (isZero(len)) {
        return FALSE;
    }
    x /= len;
    z /= len;
    return TRUE;
}

// 8006B9E4
void vecXZ_c::rotY(s16 angle) {
    mVec3_c v(x, 0.0f, z);
    v.rotY(angle);
    x = v.x;
    z = v.z;
}

// 8006BA44
void vecXZ_c::setNormal(const vecXZ_c &a, const vecXZ_c &b) {
    vecXZ_c diff = b - a;
    x = -diff.z;
    z = diff.x;
    normalize();
}

// 8006BAA0
void lineXZ_c::set(const vecXZ_c &start, const vecXZ_c &end, const vecXZ_c &normal) {
    mStart.set(start);
    mEnd.set(end);
    mNormal.set(normal);
    mD = calcD();
}

// 8006BB08
f32 lineXZ_c::calcD() const {
    return -(mNormal.x * mStart.x + mNormal.z * mStart.z);
}

// 8006BB2C
f32 lineXZ_c::calcDist(const vecXZ_c &p) const {
    f32 d = mD;
    return p.x * mNormal.x + p.z * mNormal.z + d;
}

// 8006BB54
BOOL lineXZ_c::cross(vecXZ_c *out, const vecXZ_c &a, const vecXZ_c &b) const {
    lineXZ_c line;
    vecXZ_c normal;
    normal.setNormal(a, b);
    line.set(a, b, normal);
    f32 det = mNormal.x * line.mNormal.z - line.mNormal.x * mNormal.z;
    if (!isZero(det)) {
        out->z = (line.mNormal.x * getD() - mNormal.x * line.getD()) / det;
        if (!isZero(mNormal.x)) {
            out->x = -(out->z * mNormal.z + getD()) / mNormal.x;
            return TRUE;
        }
        if (!isZero(line.mNormal.x)) {
            out->x = -(out->z * line.mNormal.z + line.getD()) / line.mNormal.x;
            return TRUE;
        }
    }
    return FALSE;
}

// 8006BD08
BOOL lineXZ_c::crossSeg(vecXZ_c *out, const vecXZ_c &a, const vecXZ_c &b) const {
    if (calcDist(a) * calcDist(b) <= 0.0f) {
        lineXZ_c line;
        vecXZ_c normal;
        normal.setNormal(a, b);
        line.set(a, b, normal);
        if (line.calcDist(mStart) * line.calcDist(mEnd) <= 0.0f && cross(out, a, b)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8006BE38
BOOL lineXZ_c::checkInside(const vecXZ_c &p) const {
    vecXZ_c dir = mStart - mEnd;
    if (dir.normalize()) {
        vecXZ_c back(-dir.x, -dir.z);
        lineXZ_c startSide(mStart, mStart + mNormal, dir);
        lineXZ_c endSide(mEnd, mEnd + mNormal, back);
        f32 startDist = startSide.calcDist(p);
        f32 endDist = endSide.calcDist(p);
        if (startDist >= 0.0f && endDist >= 0.0f) {
            return TRUE;
        }
        if (startDist <= 0.0f && endDist <= 0.0f) {
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// 8006BFD4
BOOL lineXZ_c::correctFace(vecXZ_c *pos, const vecXZ_c &old, f32 r) const {
    f32 dist = calcDist(*pos);
    if (calcDist(old) >= 0.0f && dist >= 0.0f) {
        if (dist <= r) {
            if (checkInside(*pos)) {
                vecXZ_c push(mNormal);
                push *= r - dist;
                *pos += push;
                return TRUE;
            }
        } else if (dist < r + 0.125f) {
            if (checkInside(*pos)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8006C0E8
BOOL lineXZ_c::correctEdge(vecXZ_c *pos, const vecXZ_c &old, f32 r) const {
    if (!isValid()) {
        return FALSE;
    }
    f32 dist = calcDist(*pos);
    if (dist >= 0.0f && dist < r + 0.125f && calcDist(old) > 0.0f) {
        if (dist < r) {
            if (calcDist(old) <= 0.0f) {
                return FALSE;
            }
            vecXZ_c ends[2] = {mStart, mEnd};
            for (vecXZ_c *end = ends; end < &ends[2]; end++) {
                f32 endDist = EGG::Mathf::sqrt(end->getSquareDistance(*pos));
                if (endDist < r) {
                    vecXZ_c push = *pos - *end;
                    if (!push.normalize()) {
                        push.set(getNormalX(), getNormalZ());
                    } else {
                        r -= endDist;
                    }
                    push.x *= r;
                    push.z *= r;
                    pos->x += push.x;
                    pos->z += push.z;
                    return TRUE;
                }
            }
        } else {
            return TRUE;
        }
    }
    return FALSE;
}

// 8006C2D4
BOOL lineXZ_c::correctCross(vecXZ_c *pos, const vecXZ_c &old, f32 r, BOOL) const {
    if (calcDist(old) > 0.0f) {
        vecXZ_c cross;
        if (crossSeg(&cross, *pos, old)) {
            vecXZ_c prev = *pos; // unused: the target keeps its dead stores (pos->x to sp20, pos->z to spC)
            vecXZ_c push(mNormal);
            push *= r;
            pos->set(cross + push);
            return TRUE;
        }
    }
    return FALSE;
}

// 8006C3B8
sphere_c::sphere_c() {
    mRadius = 0.0f;
}

// 8006C3D0
BOOL sphere_c::crossLine(mVec3_c *out, const line_c &line) const {
    f32 r = mRadius;
    if (line.calcDistance(mCenter) <= r) {
        // mRadius is read again here (not r): the reload is CSE'd into a copy of r (fmr f6, f31).
        mVec3_c far(mCenter.x + line.mDir.x * mRadius, mCenter.y + line.mDir.y * mRadius,
                    mCenter.z + line.mDir.z * mRadius);
        // As in the original: center - far, i.e. -mDir * r, not the point on the other side.
        mVec3_c near(mCenter.x - far.x, mCenter.y - far.y, mCenter.z - far.z);
        if (PSVECSquareDistance(line.mStart, far) < PSVECSquareDistance(line.mStart, near)) {
            *out = far;
        } else {
            *out = near;
        }
        return TRUE;
    }
    return FALSE;
}

// 8074E1E0 (initialized by __sinit 8006C4F8)
vecXZ_c vecXZ_c::Zero;

} // namespace dBGC
