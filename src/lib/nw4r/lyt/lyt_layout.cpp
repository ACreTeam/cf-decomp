#include <nw4r/lyt.h>
#include <nw4r/ut.h>

/******************************************************************************
 *
 * Utility functions
 *
 ******************************************************************************/
namespace {

using namespace nw4r;
using namespace nw4r::lyt;

void SetTagProcessorImpl(Pane* pPane, ut::WideTagProcessor* pTagProcessor) {
    TextBox* pTextBox = ut::DynamicCast<TextBox*>(pPane);
    if (pTextBox != NULL) {
        pTextBox->SetTagProcessor(pTagProcessor);
    }

    for (PaneList::Iterator it = pPane->GetChildList().GetBeginIter();
         it != pPane->GetChildList().GetEndIter(); it++) {
        SetTagProcessorImpl(&*it, pTagProcessor);
    }
}

bool IsIncludeAnimationGroupRef(GroupContainer* pGroupContainer,
                                const AnimationGroupRef* const groupRefs,
                                u16 bindGroupNum, bool bDescendingBind,
                                Pane* pTargetPane) {
    for (u16 grpIdx = 0; grpIdx < bindGroupNum; grpIdx++) {
        Group* const pGroup =
            pGroupContainer->FindGroupByName(groupRefs[grpIdx].GetName());
        detail::PaneLinkList& paneList = pGroup->GetPaneList();

        for (detail::PaneLinkList::Iterator it = paneList.GetBeginIter();
             it != paneList.GetEndIter(); it++) {
            if (it->mTarget == pTargetPane) {
                return true;
            }

            if (bDescendingBind) {
                for (Pane* pParentPane = pTargetPane->GetParent();
                     pParentPane != NULL;
                     pParentPane = pParentPane->GetParent()) {
                    if (it->mTarget == pParentPane) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

} // namespace

namespace nw4r {
namespace lyt {

MEMAllocator* Layout::mspAllocator = NULL;

/******************************************************************************
 *
 * Layout
 *
 ******************************************************************************/
Layout::Layout() : mpRootPane(NULL), mpGroupContainer(NULL) {}

Layout::~Layout() {
    DeleteObj(mpGroupContainer);

    if (mpRootPane != NULL && !mpRootPane->IsUserAllocated()) {
        DeleteObj(mpRootPane);
    }

    for (AnimTransformList::Iterator it = mAnimTransList.GetBeginIter();
         it != mAnimTransList.GetEndIter();) {
        AnimTransformList::Iterator currIt = it++;
        mAnimTransList.Erase(currIt);
        DeleteObj(&*currIt);
    }
}

bool Layout::Build(const void* lytResBuf, ResourceAccessor* pResAcsr) {
    const res::BinaryFileHeader* const pFileHead =
        static_cast<const res::BinaryFileHeader*>(lytResBuf);

    if (!detail::TestFileHeader(*pFileHead, SIGNATURE)) {
        return false;
    }

    if (!detail::TestFileVersion(*pFileHead)) {
        return false;
    }

    ResBlockSet resBlockSet = {NULL, NULL, NULL, pResAcsr};

    Pane* pParentPane = NULL;
    Pane* pLastPane = NULL;

    bool bReadRootGroup = false;
    int groupNestLevel = 0;

    const void* dataPtr =
        static_cast<const u8*>(lytResBuf) + pFileHead->headerSize;

    for (int i = 0; i < pFileHead->dataBlocks; i++) {
        const res::DataBlockHeader* pDataBlockHead =
            static_cast<const res::DataBlockHeader*>(dataPtr);

        switch (detail::GetSignatureInt(pDataBlockHead->kind)) {
        case res::Layout::SIGNATURE: {
            const res::Layout* pResLyt =
                static_cast<const res::Layout*>(dataPtr);

            mLayoutSize = pResLyt->layoutSize;
            break;
        }

        case SIGNATURE_TEXTURELIST: {
            resBlockSet.pTextureList =
                static_cast<const res::TextureList*>(dataPtr);
            break;
        }

        case SIGNATURE_FONTLIST: {
            resBlockSet.pFontList = static_cast<const res::FontList*>(dataPtr);
            break;
        }

        case SIGNATURE_MATERIALLIST: {
            resBlockSet.pMaterialList =
                static_cast<const res::MaterialList*>(dataPtr);
            break;
        }

        case res::Window::SIGNATURE:
        case res::Pane::SIGNATURE:
        case res::Picture::SIGNATURE:
        case res::TextBox::SIGNATURE:
        case res::Bounding::SIGNATURE: {
            Pane* pPane =
                BuildPaneObj(detail::GetSignatureInt(pDataBlockHead->kind),
                             dataPtr, resBlockSet);

            if (pPane != NULL) {
                if (mpRootPane == NULL) {
                    mpRootPane = pPane;
                }

                if (pParentPane != NULL) {
                    pParentPane->AppendChild(pPane);
                }

                pLastPane = pPane;
            }

            break;
        }

        case SIGNATURE_USERDATA: {
            pLastPane->SetExtUserDataList(
                static_cast<const res::ExtUserDataList*>(dataPtr));
            break;
        }

        case SIGNATURE_PANESTART: {
            pParentPane = pLastPane;
            break;
        }

        case SIGNATURE_PANEEND: {
            pLastPane = pParentPane;
            pParentPane = pLastPane->GetParent();
            break;
        }

        case res::Group::SIGNATURE: {
            if (!bReadRootGroup) {
                bReadRootGroup = true;
                mpGroupContainer = NewObj<GroupContainer>();
            } else if (mpGroupContainer != NULL && groupNestLevel == 1) {
                Group* pGroup = NewObj<Group>(
                    static_cast<const res::Group*>(dataPtr), mpRootPane);

                if (pGroup != NULL) {
                    mpGroupContainer->AppendGroup(pGroup);
                }
            }

            break;
        }

        case SIGNATURE_GROUPSTART: {
            groupNestLevel++;
            break;
        }

        case SIGNATURE_GROUPEND: {
            groupNestLevel--;
            break;
        }
        }

        dataPtr = static_cast<const u8*>(dataPtr) + pDataBlockHead->size;
    }

    return true;
}

AnimTransform* Layout::CreateAnimTransform() {
    AnimTransformBasic* const pAnimTrans = NewObj<AnimTransformBasic>();

    if (pAnimTrans != NULL) {
        mAnimTransList.PushBack(pAnimTrans);
    }

    return pAnimTrans;
}

AnimTransform* Layout::CreateAnimTransform(const void* animResBuf,
                                           ResourceAccessor* pResAcsr) {
    return CreateAnimTransform(AnimResource(animResBuf), pResAcsr);
}

AnimTransform* Layout::CreateAnimTransform(const AnimResource& animRes,
                                           ResourceAccessor* pResAcsr) {
    const res::AnimationBlock* const pAnimBlock = animRes.GetResourceBlock();

    if (pAnimBlock == NULL) {
        return NULL;
    }

    AnimTransform* const pAnimTrans = CreateAnimTransform();

    if (pAnimTrans != NULL) {
        pAnimTrans->SetResource(pAnimBlock, pResAcsr);
    }

    return pAnimTrans;
}

void Layout::BindAnimation(AnimTransform* pAnimTrans) {
    if (mpRootPane != NULL) {
        mpRootPane->BindAnimation(pAnimTrans, true, false);
    }
}

void Layout::UnbindAnimation(AnimTransform* pAnimTrans) {
    if (mpRootPane != NULL) {
        mpRootPane->UnbindAnimation(pAnimTrans, true);
    }
}

void Layout::UnbindAllAnimation() {
    UnbindAnimation(NULL);
}

bool Layout::BindAnimationAuto(const AnimResource& animRes,
                               ResourceAccessor* pResAcsr) {
    if (mpRootPane == NULL) {
        return false;
    }

    if (animRes.GetResourceBlock() == NULL) {
        return false;
    }

    AnimTransform* const pAnimTrans = CreateAnimTransform();

    u16 bindGroupNum = animRes.GetGroupNum();
    u16 animNum = 0;

    if (bindGroupNum == 0) {
        pAnimTrans->SetResource(animRes.GetResourceBlock(), pResAcsr,
                                animRes.GetResourceBlock()->animContNum);
        mpRootPane->BindAnimation(pAnimTrans, true, true);
    } else {
        const AnimationGroupRef* const groupRefs = animRes.GetGroupArray();

        for (int grpIdx = 0; grpIdx < bindGroupNum; grpIdx++) {
            Group* const pGroup =
                mpGroupContainer->FindGroupByName(groupRefs[grpIdx].GetName());

            animNum +=
                animRes.CalcAnimationNum(pGroup, animRes.IsDescendingBind());
        }

        pAnimTrans->SetResource(animRes.GetResourceBlock(), pResAcsr, animNum);

        for (int grpIdx = 0; grpIdx < bindGroupNum; grpIdx++) {
            Group* const pGroup =
                mpGroupContainer->FindGroupByName(groupRefs[grpIdx].GetName());

            lyt::BindAnimation(pGroup, pAnimTrans, animRes.IsDescendingBind(),
                               true);
        }
    }

    u16 animShareInfoNum = animRes.GetAnimationShareInfoNum();

    if (animShareInfoNum != 0) {
        const AnimationShareInfo* const animShareInfoAry =
            animRes.GetAnimationShareInfoArray();

        for (int i = 0; i < animShareInfoNum; i++) {
            Pane* const pSrcPane = mpRootPane->FindPaneByName(
                animShareInfoAry[i].GetSrcPaneName(), true);

            detail::AnimPaneTree animPaneTree(pSrcPane, animRes);

            if (animPaneTree.IsEnabled()) {
                Group* const pGroup = mpGroupContainer->FindGroupByName(
                    animShareInfoAry[i].GetTargetGroupName());

                detail::PaneLinkList& paneList = pGroup->GetPaneList();

                for (detail::PaneLinkList::Iterator it =
                         paneList.GetBeginIter();
                     it != paneList.GetEndIter(); it++) {
                    if (it->mTarget != pSrcPane) {
                        if (bindGroupNum != 0) {
                            bool bInclude = IsIncludeAnimationGroupRef(
                                mpGroupContainer, animRes.GetGroupArray(),
                                bindGroupNum, animRes.IsDescendingBind(),
                                it->mTarget);

                            if (!bInclude) {
                                continue;
                            }
                        }

                        animPaneTree.Bind(this, it->mTarget, pResAcsr);
                    }
                }
            }
        }
    }

    return true;
}

void Layout::SetAnimationEnable(AnimTransform* pAnimTrans, bool bEnable) {
    if (mpRootPane != NULL) {
        mpRootPane->SetAnimationEnable(pAnimTrans, bEnable, true);
    }
}

void Layout::CalculateMtx(const DrawInfo& drawInfo) {
    if (mpRootPane != NULL) {
        mpRootPane->CalculateMtx(drawInfo);
    }
}

void Layout::Draw(const DrawInfo& drawInfo) {
    if (mpRootPane != NULL) {
        mpRootPane->Draw(drawInfo);
    }
}

void Layout::Animate(ulong option) {
    if (mpRootPane != NULL) {
        mpRootPane->Animate(option);
    }
}

ut::Rect Layout::GetLayoutRect() const {
    return ut::Rect(-mLayoutSize.width / 2, mLayoutSize.height / 2,
                    mLayoutSize.width / 2, -mLayoutSize.height / 2);
}

void Layout::SetTagProcessor(ut::WideTagProcessor* pTagProcessor) {
    SetTagProcessorImpl(mpRootPane, pTagProcessor);
}

Pane* Layout::BuildPaneObj(s32 kind, const void* dataPtr,
                           const ResBlockSet& resBlockSet) {
    switch (kind) {
    case res::Pane::SIGNATURE: {
        const res::Pane* pResPane = static_cast<const res::Pane*>(dataPtr);
        return NewObj<Pane>(pResPane);
    }

    case res::Picture::SIGNATURE: {
        const res::Picture* pResPic = static_cast<const res::Picture*>(dataPtr);
        return NewObj<Picture>(pResPic, resBlockSet);
    }

    case res::TextBox::SIGNATURE: {
        const res::TextBox* pBlock = static_cast<const res::TextBox*>(dataPtr);
        return NewObj<TextBox>(pBlock, resBlockSet);
    }

    case res::Window::SIGNATURE: {
        const res::Window* pBlock = static_cast<const res::Window*>(dataPtr);
        return NewObj<Window>(pBlock, resBlockSet);
    }

    case res::Bounding::SIGNATURE: {
        const res::Bounding* pResBounding =
            static_cast<const res::Bounding*>(dataPtr);
        return NewObj<Bounding>(pResBounding, resBlockSet);
    }

    default: {
        return NULL;
    }
    }
}

} // namespace lyt
} // namespace nw4r
