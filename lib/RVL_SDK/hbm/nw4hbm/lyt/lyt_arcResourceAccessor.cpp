#include "revolution/hbm/nw4hbm/lyt/arcResourceAccessor.h"

#include "revolution/arc.h"
#include "revolution/hbm/nw4hbm/ut.h"

#include "string.h"

#include "string.h"

namespace {

s32 FindNameResource(ARCHandle* pArcHandle, const char* resName) {
    s32 entryNum = -1;

    ARCDir dir;
    BOOL bSuccess = ARCOpenDir(pArcHandle, ".", &dir);
    NW4HBMAssert_Line(bSuccess, 48);

    ARCDirEntry dirEntry;

    while (ARCReadDir(&dir, &dirEntry)) {
        if (dirEntry.isDir != 0) {
            bSuccess = ARCChangeDir(pArcHandle, dirEntry.name);
            NW4HBMAssert_Line(bSuccess, 57);

            entryNum = FindNameResource(pArcHandle, resName);
            bSuccess = ARCChangeDir(pArcHandle, "..");
            NW4HBMAssert_Line(bSuccess, 60);

            if (entryNum != -1) {
                break;
            }
        } else if (stricmp(resName, dirEntry.name) == 0) {
            entryNum = dirEntry.entryNum;
            break;
        }
    }

    bSuccess = ARCCloseDir(&dir);
    NW4HBMAssert_Line(bSuccess, 77);
    return entryNum;
}

void* GetResourceSub(ARCHandle* pArcHandle, const char* resRootDir, u32 resType, const char* name, u32* pSize) {
    s32 entryNum = -1;

    if (ARCConvertPathToEntrynum(pArcHandle, resRootDir) != -1 && ARCChangeDir(pArcHandle, resRootDir)) {
        if (!resType) {
            entryNum = FindNameResource(pArcHandle, name);
        } else {
            char resTypeStr[5];
            resTypeStr[0] = resType >> 24;
            resTypeStr[1] = resType >> 16;
            resTypeStr[2] = resType >> 8;
            resTypeStr[3] = resType;
            resTypeStr[4] = '\0';

            if (ARCConvertPathToEntrynum(pArcHandle, resTypeStr) != -1 && ARCChangeDir(pArcHandle, resTypeStr)) {
                entryNum = ARCConvertPathToEntrynum(pArcHandle, name);
                BOOL bSuccess = ARCChangeDir(pArcHandle, "..");
                NW4HBMAssert_Line(bSuccess, 117);
            }
        }

        BOOL bSuccess = ARCChangeDir(pArcHandle, "..");
        NW4HBMAssert_Line(bSuccess, 123);
    }

    if (entryNum != -1) {
        ARCFileInfo arcFileInfo;
        BOOL bSuccess = ARCFastOpen(pArcHandle, entryNum, &arcFileInfo);
        NW4HBMAssert_Line(bSuccess, 131);

        void* resPtr = ARCGetStartAddrInMem(&arcFileInfo);

        if (pSize) {
            *pSize = ARCGetLength(&arcFileInfo);
        }

        ARCClose(&arcFileInfo);

        return resPtr;
    }

    return NULL;
}

} // namespace

namespace nw4hbm {
namespace lyt {

ut::Font* detail::FindFont(FontRefLinkList* pFontRefList, const char* name) {
    for (FontRefLinkList::Iterator it = pFontRefList->GetBeginIter(); it != pFontRefList->GetEndIter(); it++) {
        if (strcmp(name, it->GetFontName()) == 0) {
            return it->GetFont();
        }
    }
    return NULL;
}

FontRefLink::FontRefLink() :
    mpFont(NULL) {}

void FontRefLink::Set(const char* name, ut::Font* pFont) {
    NW4HBMAssert_Line(std::strlen(name) < FONTNAMEBUF_MAX, 203);

    strcpy(mFontName, name);
    mpFont = pFont;
}

ArcResourceAccessor::ArcResourceAccessor() :
    mArcBuf(NULL) {}

bool ArcResourceAccessor::Attach(void* archiveStart, const char* resourceRootDirectory) {
    NW4HBMAssert_Line(! IsAttached(), 220);
    NW4HBMAssertPointerNonnull_Line(archiveStart, 221);
    NW4HBMAssertPointerNonnull_Line(resourceRootDirectory, 222);

    BOOL bSuccess = ARCInitHandle(archiveStart, &mArcHandle);

    if (!bSuccess) {
        return false;
    }

    mArcBuf = archiveStart;

    strncpy(mResRootDir, resourceRootDirectory, ARRAY_COUNT(mResRootDir) - 1);
    mResRootDir[ARRAY_COUNT(mResRootDir) - 1] = '\0';

    return true;
}

void* ArcResourceAccessor::Detach() {
    NW4HBMAssert(IsAttached());

    void* old = mArcBuf;
    mArcBuf = NULL;
    return old;
}

void* ArcResourceAccessor::GetResource(u32 resType, const char* name, u32* pSize) {
    return GetResourceSub(&mArcHandle, mResRootDir, resType, name, pSize);
}

bool ArcResourceLink::Set(void* archiveStart, const char* resRootDirectory) {
    BOOL bSuccess = ARCInitHandle(archiveStart, &mArcHandle);

    if (!bSuccess) {
        return false;
    }

    strncpy(mResRootDir, resRootDirectory, ARRAY_COUNT(mResRootDir) - 1);
    mResRootDir[ARRAY_COUNT(mResRootDir) - 1] = '\0';

    return true;
}

void ArcResourceAccessor::RegistFont(FontRefLink* pLink) {
    NW4HBMAssertPointerNonnull(pLink);
    mFontList.PushBack(pLink);
}

void ArcResourceAccessor::UnregistFont(FontRefLink* pLink) {
    NW4HBMAssertPointerNonnull(pLink);
    mFontList.Erase(pLink);
}

ut::Font* ArcResourceAccessor::GetFont(const char* name) { return detail::FindFont(&mFontList, name); }

MultiArcResourceAccessor::MultiArcResourceAccessor() {}

MultiArcResourceAccessor::~MultiArcResourceAccessor() { DetachAll(); }

void MultiArcResourceAccessor::Attach(ArcResourceLink* pLink) { mArcList.PushBack(pLink); }

void* MultiArcResourceAccessor::GetResource(u32 resType, const char* name, u32* pSize) {
    for (ArcResourceLinkList::Iterator it = mArcList.GetBeginIter(); it != mArcList.GetEndIter(); it++) {
        ARCHandle* pArcHandle = it->GetArcHandle();
        if (void* resPtr = GetResourceSub(pArcHandle, it->GetResRootDir(), resType, name, pSize)) {
            return resPtr;
        }
    }
    return NULL;
}

ArcResourceLink& FirstArcResource(ArcResourceLinkList* pLinks) {
    return *pLinks->GetBeginIter();
}

void MultiArcResourceAccessor::RegistFont(FontRefLink* pLink) { mFontList.PushBack(pLink); }

ut::Font* MultiArcResourceAccessor::GetFont(const char* name) { return detail::FindFont(&mFontList, name); }

} // namespace lyt
} // namespace nw4hbm
