#include <nw4r/lyt.h>
#include <nw4r/ut.h>

#include <cstring>

namespace nw4r {
namespace lyt {

/******************************************************************************
 *
 * Group
 *
 ******************************************************************************/
Group::Group(const res::Group* pResGroup, Pane* pRootPane) {
    Init();

    std::strncpy(mName, pResGroup->name, NW4R_LYT_RES_NAME_LEN);
    mName[NW4R_LYT_RES_NAME_LEN] = '\0';

    const char* const paneNameBase =
        detail::ConvertOffsToPtr<char>(pResGroup, sizeof(res::Group));

    for (int i = 0; i < pResGroup->paneNum; i++) {
        Pane* pFindPane = pRootPane->FindPaneByName(
            paneNameBase + i * NW4R_LYT_RES_NAME_LEN, true);

        if (pFindPane != NULL) {
            AppendPane(pFindPane);
        }
    }
}

void Group::Init() {
    mbUserAllocated = false;
}

Group::~Group() {
    for (detail::PaneLinkList::Iterator it = mPaneLinkList.GetBeginIter();
         it != mPaneLinkList.GetEndIter();) {
        detail::PaneLinkList::Iterator currIt = it++;
        mPaneLinkList.Erase(currIt);
        Layout::DeleteObj(&*currIt);
    }
}

void Group::AppendPane(Pane* pPane) {
    detail::PaneLink* pPaneLink = Layout::NewObj<detail::PaneLink>();

    if (pPaneLink != NULL) {
        pPaneLink->mTarget = pPane;
        mPaneLinkList.PushBack(pPaneLink);
    }
}

/******************************************************************************
 *
 * GroupContainer
 *
 ******************************************************************************/
GroupContainer::~GroupContainer() {
    for (GroupList::Iterator it = mGroupList.GetBeginIter();
         it != mGroupList.GetEndIter();) {
        GroupList::Iterator currIt = it++;
        mGroupList.Erase(currIt);

        if (!currIt->IsUserAllocated()) {
            Layout::DeleteObj(&*currIt);
        }
    }
}

void GroupContainer::AppendGroup(Group* pGroup) {
    mGroupList.PushBack(pGroup);
}

Group* GroupContainer::FindGroupByName(const char* findName) {
    for (GroupList::Iterator it = mGroupList.GetBeginIter();
         it != mGroupList.GetEndIter(); it++) {
        if (detail::EqualsResName(it->GetName(), findName)) {
            return &*it;
        }
    }

    return NULL;
}

} // namespace lyt
} // namespace nw4r
