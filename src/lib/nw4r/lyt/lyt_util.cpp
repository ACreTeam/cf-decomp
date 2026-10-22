#include <nw4r/lyt.h>
#include <nw4r/lyt/lyt_utils.h>

namespace nw4r {
namespace lyt {

void BindAnimation(Group* pGroup, AnimTransform* pAnimTrans, bool bRecursive,
                   bool bDisable) {
    detail::PaneLinkList& paneList = pGroup->GetPaneList();

    for (detail::PaneLinkList::Iterator it = paneList.GetBeginIter();
         it != paneList.GetEndIter(); it++) {
        it->mTarget->BindAnimation(pAnimTrans, bRecursive, bDisable);
    }
}

} // namespace lyt
} // namespace nw4r
