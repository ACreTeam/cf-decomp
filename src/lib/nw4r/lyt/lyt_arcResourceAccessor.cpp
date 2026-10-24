#include <nw4r/lyt.h>

#include <revolution/arc.h>

#include <cstring>

/******************************************************************************
 *
 * Utility functions
 *
 ******************************************************************************/
namespace {

s32 FindNameResource(ARCHandle* pArcHandle, const char* resName) {
    s32 entryNum = -1;

    ARCDir dir;
    ARCOpenDir(pArcHandle, ".", &dir);

    ARCDirEntry dirEntry;
    while (ARCReadDir(&dir, &dirEntry)) {
        if (dirEntry.isDir) {
            ARCChangeDir(pArcHandle, dirEntry.name);
            entryNum = FindNameResource(pArcHandle, resName);
            ARCChangeDir(pArcHandle, "..");

            if (entryNum != -1) {
                break;
            }
        } else {
            if (std::stricmp(resName, dirEntry.name) == 0) {
                entryNum = dirEntry.entryNum;
                break;
            }
        }
    }

    ARCCloseDir(&dir);
    return entryNum;
}

void* GetResourceSub(ARCHandle* pArcHandle, const char* resRootDir, ulong resType,
                     const char* name, ulong* pSize) {

    s32 entryNum = -1;

    if (ARCConvertPathToEntrynum(pArcHandle, resRootDir) != -1) {
        if (ARCChangeDir(pArcHandle, resRootDir)) {
            if (resType == 0) {
                entryNum = FindNameResource(pArcHandle, name);
            } else {
                char resTypeStr[4 + 1];
                resTypeStr[0] = resType >> 24;
                resTypeStr[1] = resType >> 16;
                resTypeStr[2] = resType >> 8;
                resTypeStr[3] = resType >> 0;
                resTypeStr[4] = '\0';

                if (ARCConvertPathToEntrynum(pArcHandle, resTypeStr) != -1 &&
                    ARCChangeDir(pArcHandle, resTypeStr)) {

                    entryNum = ARCConvertPathToEntrynum(pArcHandle, name);
                    ARCChangeDir(pArcHandle, "..");
                }
            }

            ARCChangeDir(pArcHandle, "..");
        }
    }

    if (entryNum != -1) {
        ARCFileInfo arcFileInfo;
        ARCFastOpen(pArcHandle, entryNum, &arcFileInfo);

        void* resPtr = ARCGetStartAddrInMem(&arcFileInfo);

        if (pSize != NULL) {
            *pSize = ARCGetLength(&arcFileInfo);
        }

        ARCClose(&arcFileInfo);
        return resPtr;
    }

    return NULL;
}

} // namespace

namespace nw4r {
namespace lyt {

/******************************************************************************
 *
 * FontRefLink
 *
 ******************************************************************************/
FontRefLink::FontRefLink() : mpFont(NULL) {}

void FontRefLink::Set(const char* name, ut::Font* pFont) {
    std::strcpy(mFontName, name);
    mpFont = pFont;
}

namespace detail {

ut::Font* FindFont(FontRefLinkList* pFontRefList, const char* name) {
    for (FontRefLinkList::Iterator it = pFontRefList->GetBeginIter();
         it != pFontRefList->GetEndIter(); it++) {
        if (std::strcmp(name, it->GetFontName()) == 0) {
            return it->GetFont();
        }
    }

    return NULL;
}

} // namespace detail

/******************************************************************************
 *
 * ArcResourceAccessor
 *
 ******************************************************************************/
ArcResourceAccessor::ArcResourceAccessor() : mArcBuf(NULL) {}

bool ArcResourceAccessor::Attach(void* archiveStart,
                                 const char* resourceRootDirectory) {
    BOOL bSuccess = ARCInitHandle(archiveStart, &mArcHandle);
    if (!bSuccess) {
        return false;
    }

    mArcBuf = archiveStart;

    std::strncpy(mResRootDir, resourceRootDirectory, ROOTPATH_MAX - 1);
    mResRootDir[ROOTPATH_MAX - 1] = '\0';

    return true;
}

void* ArcResourceAccessor::Detach() {
    void* ret = mArcBuf;
    mArcBuf = NULL;
    return ret;
}

void* ArcResourceAccessor::GetResource(ulong resType, const char* name,
                                       ulong* pSize) {
    return GetResourceSub(&mArcHandle, mResRootDir, resType, name, pSize);
}

/******************************************************************************
 *
 * ArcResourceLink
 *
 ******************************************************************************/
bool ArcResourceLink::Set(void* archiveStart,
                          const char* resourceRootDirectory) {
    BOOL bSuccess = ARCInitHandle(archiveStart, &mArcHandle);
    if (!bSuccess) {
        return false;
    }

    std::strncpy(mResRootDir, resourceRootDirectory, ROOTPATH_MAX - 1);
    mResRootDir[ROOTPATH_MAX - 1] = '\0';

    return true;
}

void ArcResourceAccessor::RegistFont(FontRefLink* pLink) {
    mFontList.PushBack(pLink);
}

void ArcResourceAccessor::UnregistFont(FontRefLink* pLink) {
    mFontList.Erase(pLink);
}

ut::Font* ArcResourceAccessor::GetFont(const char* name) {
    return detail::FindFont(&mFontList, name);
}

/******************************************************************************
 *
 * MultiArcResourceAccessor
 *
 ******************************************************************************/
MultiArcResourceAccessor::MultiArcResourceAccessor() {}

MultiArcResourceAccessor::~MultiArcResourceAccessor() {
    DetachAll();
}

void MultiArcResourceAccessor::Attach(ArcResourceLink* pLink) {
    mArcList.PushBack(pLink);
}

void MultiArcResourceAccessor::Detach(ArcResourceLink* pLink) {
    mArcList.Erase(pLink);
}

void MultiArcResourceAccessor::DetachAll() {
    mArcList.Clear();
}

void* MultiArcResourceAccessor::GetResource(ulong resType, const char* name,
                                            ulong* pSize) {
    for (ArcResourceLinkList::Iterator it = mArcList.GetBeginIter();
         it != mArcList.GetEndIter(); it++) {
        ARCHandle* pArcHandle = it->GetArcHandle();
        void* resPtr = GetResourceSub(pArcHandle, it->GetResRootDir(),
                                      resType, name, pSize);

        if (resPtr != NULL) {
            return resPtr;
        }
    }

    return NULL;
}

void MultiArcResourceAccessor::RegistFont(FontRefLink* pLink) {
    mFontList.PushBack(pLink);
}

void MultiArcResourceAccessor::UnregistFont(FontRefLink* pLink) {
    mFontList.Erase(pLink);
}

ut::Font* MultiArcResourceAccessor::GetFont(const char* name) {
    return detail::FindFont(&mFontList, name);
}

} // namespace lyt
} // namespace nw4r
