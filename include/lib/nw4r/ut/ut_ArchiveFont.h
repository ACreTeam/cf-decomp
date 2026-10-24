#ifndef NW4R_UT_ARCHIVE_FONT_H
#define NW4R_UT_ARCHIVE_FONT_H
#include <nw4r/types_nw4r.h>

#include <nw4r/ut/ut_ArchiveFontBase.h>

#include <revolution/gx.h>
#include <revolution/os.h>

namespace nw4r {
namespace ut {

class ArchiveFont : public detail::ArchiveFontBase {
  public:
    ArchiveFont();
    virtual ~ArchiveFont(); // at 0x8

    virtual void GetGlyph(Glyph *pGlyph, u16 ch) const; // at 0x50

    static u32 GetRequireBufferSize(const void *pBrfna,
                                    const char *pGroups = LOAD_GLYPH_ALL,
                                    int numCache = 1);

    Result StreamingConstruct(ConstructContext *pContext, const void *pStream,
                              u32 size);
    bool Construct(void *pBuffer, u32 bufferSize, const void *pBrfna,
                   const char *pGroups = LOAD_GLYPH_ALL);

  private:
    struct CacheLink {
        u16 next; // at 0x0
        u16 prev; // at 0x2
    };

    static const u16 CACHE_NONE = 0xFFFF;

    static u32 GetRequireDataSize(const GlyphGroupInfo *pInfo,
                                  const char *pGroups, u32 *pNumSheets);

    u16 *InitSheetCache(void *pBuffer, u32 size, int numAllSheets,
                        int numSheets, u32 sheetSize);

    void GetGlyphFromIndex(Glyph *pGlyph, u16 index, u16 slot) const;

    static int GetSheetIndex(u16 adjusted, const FontTextureGlyph *pTexGlyph) {
        int sheet = adjusted / (pTexGlyph->sheetRow * pTexGlyph->sheetLine);
        return sheet;
    }

    void InitCacheLinks(int numSlots) {
        for (int i = 0; i < numSlots; i++) {
            CacheLink &rLink = mpCacheLinks[i];
            rLink.next = i + 1;
            rLink.prev = i <= 0 ? numSlots : i - 1;
        }
    }

    CacheLink *GetCacheLink(int slot) const {
        CacheLink *pLink = &mpCacheLinks[slot];
        return pLink;
    }

    static bool IsCacheLinked(const CacheLink *pLink) {
        return pLink->next != CACHE_NONE;
    }

    void LoadCacheSlot(int sheet, int slot, u32 sheetSize) const {
        const void *pSrc;
        u8 *pDst;
        int oldSheet = mpSlotToSheet[slot];
        pDst = mpCache + slot * sheetSize;
        pSrc = mpSheets[sheet];

        if (oldSheet != CACHE_NONE) {
            GXDrawDone();
            u16 oldSlot = mpSheetToSlot[oldSheet];
            mpSheetToSlot[oldSheet] = CACHE_NONE;
            mpSlotToSheet[oldSlot] = CACHE_NONE;
        }

        CXUncompressHuffman(pSrc, pDst);
        mpSheetToSlot[sheet] = slot;
        mpSlotToSheet[slot] = sheet;

        GXInvalidateTexAll();
        DCFlushRange(pDst, CXGetUncompressedSize(pSrc));
    }

    void UnlinkCache(CacheLink *pLink) const {
        CacheLink *pLinks = mpCacheLinks;
        u16 prev = pLink->prev;

        pLinks[pLink->next].prev = prev;
        pLinks[prev].next = pLink->next;
    }

    void LinkCacheLast(CacheLink *pLink, u16 slot) const {
        CacheLink *pLinks = mpCacheLinks;
        CacheLink *pEnd = &pLinks[mNumCacheLinks];
        u16 last = pEnd->prev;

        pLink->prev = last;
        pLink->next = pLinks[last].next;
        pEnd->prev = slot;
        pLinks[last].next = slot;
    }

    Result ConstructOpAnalyzeGLGR(ConstructContext *pContext,
                                  CachedStreamReader *pReader);
    static Result ConstructOpPrepairExpandSheet(ConstructContext *pContext,
                                                CachedStreamReader *pReader);

  private:
    CacheLink *mpCacheLinks; // at 0x20
    u16 mNumCacheLinks;      // at 0x24
    u16 mCacheLinkHead;      // at 0x26
    u16 mNumSheets;          // at 0x28
    u16 mNumCacheSlots;      // at 0x2A
    u16 *mpSheetToSlot;      // at 0x2C
    u16 *mpSlotToSheet;      // at 0x30
    const void **mpSheets;   // at 0x34
    u8 *mpCache;             // at 0x38
};

} // namespace ut
} // namespace nw4r

#endif
