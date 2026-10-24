#include <nw4r/g3d.h>
#include <nw4r/math.h>

namespace nw4r {
namespace g3d {

void ResNode::PatchChrAnmResult(ChrAnmResult* pResult) const {
    if (!IsValid()) {
        return;
    }

    const ResNodeData& n = ref();
    u32 flag = pResult->flags;

    if (flag & ChrAnmResult::FLAG_PATCH_SCALE) {
        if (n.flags & ResNodeData::FLAG_SCALE_ONE) {
            flag |=
                ChrAnmResult::FLAG_SCALE_ONE | ChrAnmResult::FLAG_SCALE_UNIFORM;
            pResult->s.x = pResult->s.y = pResult->s.z = 1.0f;
        } else {
            if (n.flags & ResNodeData::FLAG_SCALE_UNIFORM) {
                flag |= ChrAnmResult::FLAG_SCALE_UNIFORM;
            } else {
                flag &= ~ChrAnmResult::FLAG_SCALE_UNIFORM;
            }

            flag &= ~(ChrAnmResult::FLAG_SCALE_ONE |
                      ChrAnmResult::FLAG_MTX_IDENT);
            pResult->s = static_cast<const math::VEC3&>(n.scale);
        }
    }

    if (flag & ChrAnmResult::FLAG_PATCH_ROT) {
        math::VEC3 trans(pResult->rt._03, pResult->rt._13, pResult->rt._23);

        if (n.flags & ResNodeData::FLAG_ROT_ZERO) {
            math::MTX34Identity(&pResult->rt);
            flag |= ChrAnmResult::FLAG_ROT_ZERO;
        } else {
            math::MTX34RotXYZDeg(&pResult->rt, n.rot.x, n.rot.y, n.rot.z);
            flag &= ~(ChrAnmResult::FLAG_ROT_ZERO |
                      ChrAnmResult::FLAG_ROT_TRANS_ZERO |
                      ChrAnmResult::FLAG_MTX_IDENT);
        }

        flag |= ChrAnmResult::FLAG_ROT_RAW_FMT;

        pResult->rt._03 = trans.x;
        pResult->rt._13 = trans.y;
        pResult->rt._23 = trans.z;
    }

    if (flag & ChrAnmResult::FLAG_PATCH_TRANS) {
        if (n.flags & ResNodeData::FLAG_TRANS_ZERO) {
            flag |= ChrAnmResult::FLAG_TRANS_ZERO;
            pResult->rt._03 = pResult->rt._13 = pResult->rt._23 = 0.0f;
        } else {
            flag &= ~(ChrAnmResult::FLAG_TRANS_ZERO |
                      ChrAnmResult::FLAG_ROT_TRANS_ZERO |
                      ChrAnmResult::FLAG_MTX_IDENT);

            pResult->rt._03 = n.translate.x;
            pResult->rt._13 = n.translate.y;
            pResult->rt._23 = n.translate.z;
        }
    }

    if ((flag & ChrAnmResult::FLAG_ROT_ZERO) &&
        (flag & ChrAnmResult::FLAG_TRANS_ZERO)) {
        flag |= ChrAnmResult::FLAG_ROT_TRANS_ZERO;

        if (flag & ChrAnmResult::FLAG_SCALE_ONE) {
            flag |= ChrAnmResult::FLAG_MTX_IDENT;
        }
    }

    pResult->flags = flag & ~(ChrAnmResult::FLAG_PATCH_SCALE |
                              ChrAnmResult::FLAG_PATCH_ROT |
                              ChrAnmResult::FLAG_PATCH_TRANS);
}

void ResNode::CalcChrAnmResult(ChrAnmResult* pResult) const {
    if (!IsValid()) {
        return;
    }

    const ResNodeData& r = ref();
    u32 flags = 0;

    if (r.flags & ResNodeData::FLAG_SCALE_ONE) {
        flags |=
            ChrAnmResult::FLAG_SCALE_ONE | ChrAnmResult::FLAG_SCALE_UNIFORM;

        pResult->s.z = 1.0f;
        pResult->s.y = 1.0f;
        pResult->s.x = 1.0f;
    } else {
        if (r.flags & ResNodeData::FLAG_SCALE_UNIFORM) {
            flags |= ChrAnmResult::FLAG_SCALE_UNIFORM;
        }

        pResult->s = static_cast<const math::VEC3&>(r.scale);
    }

    if (r.flags & ResNodeData::FLAG_ROT_ZERO) {
        PSMTXIdentity(pResult->rt);
        flags |= ChrAnmResult::FLAG_ROT_ZERO;
    } else {
        pResult->rawR = math::VEC3(r.rot);
        math::MTX34RotXYZDeg(&pResult->rt, r.rot.x, r.rot.y, r.rot.z);
    }

    if (r.flags & ResNodeData::FLAG_TRANS_ZERO) {
        flags |= ChrAnmResult::FLAG_TRANS_ZERO;
    } else {
        pResult->rt._03 = r.translate.x;
        pResult->rt._13 = r.translate.y;
        pResult->rt._23 = r.translate.z;
    }

    if (flags & ChrAnmResult::FLAG_ROT_ZERO) {
        if (flags & ChrAnmResult::FLAG_TRANS_ZERO) {
            flags |= ChrAnmResult::FLAG_ROT_TRANS_ZERO;

            if (flags & ChrAnmResult::FLAG_SCALE_ONE) {
                flags |= ChrAnmResult::FLAG_MTX_IDENT;
            }
        }
    }

    flags |= ChrAnmResult::FLAG_ROT_RAW_FMT;
    flags |= ChrAnmResult::FLAG_ANM_EXISTS;

    if (r.flags & ResNodeData::FLAG_SSC_APPLY) {
        flags |= ChrAnmResult::FLAG_SSC_APPLY;
    }

    if (r.flags & ResNodeData::FLAG_SSC_PARENT) {
        flags |= ChrAnmResult::FLAG_SSC_PARENT;
    }

    pResult->flags = flags;
}

} // namespace g3d
} // namespace nw4r
