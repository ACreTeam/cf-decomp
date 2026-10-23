#include <lib/textinput/tiLayout.h>
#include <new>
#include <string.h>

namespace textinput {
void AnimTransformPane::SetResource(const nw4r::lyt::res::AnimationBlock *pRes,
                                    nw4r::lyt::ResourceAccessor *pResAccessor) {
    mpRes = pRes;
    mpFileResAry = NULL;

    if (pRes->fileNum != 0) {
        mpFileResAry = static_cast<void **>(nw4r::lyt::Layout::AllocMemory(
            sizeof(*mpFileResAry) * pRes->fileNum));

        if (mpFileResAry != NULL) {
            const u32 *fileNameOffsets =
                nw4r::lyt::detail::ConvertOffsToPtr<u32>(mpRes, sizeof(*mpRes));

            for (int i = 0; i < mpRes->fileNum; i++) {
                mpFileResAry[i] = pResAccessor->GetResource(
                    nw4r::lyt::ArcResourceAccessor::RES_TYPE_TEXTURE,
                    nw4r::lyt::detail::GetStrTableStr(fileNameOffsets, i),
                    NULL);
            }
        }
    }
}

u32 AnimTransformPane::CountAnimForPane_(const char *name) {
    if (mpRes == NULL) {
        return 0;
    }

    u32 count = 0;

    const u32 *animContOffsets = nw4r::lyt::detail::ConvertOffsToPtr<u32>(
        mpRes, mpRes->animContOffsetsOffset);
    for (u16 i = 0; i < mpRes->animContNum; i++) {
        const nw4r::lyt::res::AnimationContent *cnt =
            nw4r::lyt::detail::ConvertOffsToPtr<
                nw4r::lyt::res::AnimationContent>(mpRes, animContOffsets[i]);

        if (cnt->type == nw4r::lyt::res::AnimationContent::ANIMTYPE_PANE) {
            if (strncmp(cnt->name, name, 17) == 0) {
                count++;
            }
        } else {
            if (strncmp(cnt->name, name, 17) == 0) {
                count++;
            }
        }
    }

    return count;
}

u32 AnimTransformPane::CountAnimForPane_(nw4r::lyt::Pane *pane,
                                         bool bRecursive) {
    u32 count = CountAnimForPane_(pane->GetName());

    if (bRecursive) {
        nw4r::lyt::PaneList &paneList = pane->GetChildList();
        for (nw4r::lyt::PaneList::Iterator it = paneList.GetBeginIter();
             it != paneList.GetEndIter(); ++it) {
            count += CountAnimForPane_(&*it, bRecursive);
        }
    }

    return count;
}

void AnimTransformPane::Bind(nw4r::lyt::Pane *pane, bool bRecursive,
                             bool bDisable) {
    u32 count = 0;

    if (!bRecursive) {
        count = CountAnimForPane_(pane->GetName());
    } else {
        count = CountAnimForPane_(pane, true);
    }

    u32 linkSize = sizeof(nw4r::lyt::AnimationLink) * count;
    void *linkBuffer = nw4r::lyt::Layout::AllocMemory(linkSize);

    mAnimLinkAry = static_cast<nw4r::lyt::AnimationLink *>(linkBuffer);
    mAnimLinkNum = count;

    memset(mAnimLinkAry, 0, linkSize);
    for (u16 i = 0; i < count; i++) {
        new (&mAnimLinkAry[i]) nw4r::lyt::AnimationLink();
    }

    const u32 *animContOffsets = nw4r::lyt::detail::ConvertOffsToPtr<u32>(
        mpRes, mpRes->animContOffsetsOffset);

    int linkIdx = 0;
    for (u16 i = 0; i < mpRes->animContNum; i++) {
        const nw4r::lyt::res::AnimationContent *animCont =
            nw4r::lyt::detail::ConvertOffsToPtr<
                nw4r::lyt::res::AnimationContent>(mpRes, animContOffsets[i]);

        if (animCont->type == nw4r::lyt::res::AnimationContent::ANIMTYPE_PANE) {
            nw4r::lyt::Pane *pFindPane =
                pane->FindPaneByName(animCont->name, bRecursive);

            if (pFindPane != NULL) {
                mAnimLinkAry[linkIdx].SetAnimTransform(this, i);
                pFindPane->AddAnimationLink(&mAnimLinkAry[linkIdx]);
                linkIdx++;
            }
        } else {
            nw4r::lyt::Material *pFindMat =
                pane->FindMaterialByName(animCont->name, bRecursive);

            if (pFindMat != NULL) {
                mAnimLinkAry[linkIdx].SetAnimTransform(this, i);
                pFindMat->AddAnimationLink(&mAnimLinkAry[linkIdx]);
                linkIdx++;
            }
        }
    }
}

void AnimTransformPane::Bind(nw4r::lyt::Material *material) {
}

void AnimTransformPane::ForceBind(nw4r::lyt::Pane *pane, const char *name) {
    u32 count = CountAnimForPane_(name);

    u32 linkSize = sizeof(nw4r::lyt::AnimationLink) * count;
    void *linkBuffer = nw4r::lyt::Layout::AllocMemory(linkSize);

    mAnimLinkAry = static_cast<nw4r::lyt::AnimationLink *>(linkBuffer);
    mAnimLinkNum = count;

    memset(mAnimLinkAry, 0, linkSize);
    for (u16 i = 0; i < count; i++) {
        for (u16 i = 0; i < count; i++) {
            new (&mAnimLinkAry[i]) nw4r::lyt::AnimationLink();
        }
    }

    const u32 *animContOffsets = nw4r::lyt::detail::ConvertOffsToPtr<u32>(
        mpRes, mpRes->animContOffsetsOffset);

    int linkIdx = 0;
    for (u16 i = 0; i < mpRes->animContNum; i++) {
        const nw4r::lyt::res::AnimationContent *animCont =
            nw4r::lyt::detail::ConvertOffsToPtr<
                nw4r::lyt::res::AnimationContent>(mpRes, animContOffsets[i]);

        if (strncmp(animCont->name, name, 17) != 0) {
            continue;
        }

        mAnimLinkAry[linkIdx].SetAnimTransform(this, i);
        if (animCont->type == nw4r::lyt::res::AnimationContent::ANIMTYPE_PANE) {
            pane->AddAnimationLink(&mAnimLinkAry[linkIdx]);
        } else {
            pane->GetMaterial()->AddAnimationLink(&mAnimLinkAry[linkIdx]);
        }

        linkIdx++;
    }
}

static inline bool
IsValidVersion(const nw4r::lyt::res::BinaryFileHeader &header) {
    u8 major = (header.version >> 8) & 0xFF;
    u8 minor = header.version;
    return major == 0 && minor >= 8 && minor <= 10;
}

nw4r::lyt::AnimTransform *
TiLayout::CreateAnimTransform(const void *anmResBuf,
                              nw4r::lyt::ResourceAccessor *pResAcsr) {
    const nw4r::lyt::res::BinaryFileHeader *header =
        static_cast<const nw4r::lyt::res::BinaryFileHeader *>(anmResBuf);

    if (!IsValidVersion(*header)) {
        return NULL;
    }

    const nw4r::lyt::res::AnimationBlock *animBlock = NULL;
    const nw4r::lyt::res::DataBlockHeader *blockHeader =
        nw4r::lyt::detail::ConvertOffsToPtr<nw4r::lyt::res::DataBlockHeader>(
            header, header->headerSize);

    nw4r::lyt::AnimTransform *ret = NULL;

    for (int i = 0; i < header->dataBlocks; i++) {
        switch (nw4r::lyt::detail::GetSignatureInt(blockHeader->kind)) {
        case SIGNATURE_ANIMATIONINFO: {
            switch (nw4r::lyt::detail::GetSignatureInt(header->signature)) {
            case nw4r::lyt::Layout::SIGNATURE_ANIMATION:
            case nw4r::lyt::res::AnimationInfo::SIGNATURE_ANMPANESRT:
            case nw4r::lyt::res::AnimationInfo::SIGNATURE_ANMVTXCLR:
            case nw4r::lyt::res::AnimationInfo::SIGNATURE_ANMPANEVIS:
            case nw4r::lyt::res::AnimationInfo::SIGNATURE_ANMMATCLR:
            case nw4r::lyt::res::AnimationInfo::SIGNATURE_ANMTEXPAT:
            case nw4r::lyt::res::AnimationInfo::SIGNATURE_ANMTEXSRT: {
                void *mem =
                    nw4r::lyt::Layout::AllocMemory(sizeof(AnimTransformPane));
                AnimTransformPane *animTrans = new (mem) AnimTransformPane();

                if (animTrans != NULL) {
                    animBlock = reinterpret_cast<
                        const nw4r::lyt::res::AnimationBlock *>(blockHeader);
                    animTrans->SetResource(animBlock, pResAcsr);

                    ret = animTrans;
                }
            }
            }

            if (ret) {
                mAnimTransList.PushBack(ret);
            }
        }
        }

        blockHeader = nw4r::lyt::detail::ConvertOffsToPtr<
            nw4r::lyt::res::DataBlockHeader>(blockHeader, blockHeader->size);
    }

    return ret;
}
} // namespace textinput