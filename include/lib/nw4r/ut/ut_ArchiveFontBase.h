#ifndef NW4R_UT_ARCHIVE_FONT_BASE_H
#define NW4R_UT_ARCHIVE_FONT_BASE_H
#include <nw4r/types_nw4r.h>

#include <nw4r/ut/ut_ResFontBase.h>
#include <nw4r/ut/ut_algorithm.h>
#include <nw4r/ut/ut_binaryFileFormat.h>

#include <revolution/cx.h>

#include <cstddef>
#include <cstring>

namespace nw4r {
namespace ut {

struct FontGlyphGroupData {
    u32 sheetSize;      // at 0x0
    u16 glyphsPerSheet; // at 0x4
    u16 numSet;         // at 0x6
    u16 numSheet;       // at 0x8
    u16 numCWDH;        // at 0xA
    u16 numCMAP;        // at 0xC
    u16 nameOffsets[1]; // at 0xE
};

struct FontGlyphGroupBlock {
    BinaryBlockHeader header; // at 0x0
    FontGlyphGroupData body;  // at 0x8
};

namespace detail {

class ArchiveFontBase : public ResFontBase {
  public:
    struct ConstructContext;

    enum Result {
        RESULT_NEED_DATA,
        RESULT_DONE,
        RESULT_ERROR,
        RESULT_CONTINUE
    };

  protected:
    struct CachedStreamReader {
      public:
        void Init();
        void Attach(const void *pStream, u32 size);
        u32 GetRestSize() const;
        void Read(void *pDst, u32 size);
        void Read(ConstructContext *pContext, u32 size);
        u32 ReadTask(ConstructContext *pContext);
        bool RequestData(ConstructContext *pContext, u32 size);

        u32 GetReadSize() const {
            return (mpCur - mpBegin) + (mpCacheCur - mpCacheBegin);
        }

        void Skip(u32 size) {
            u32 cacheRest = mpCacheEnd - mpCacheCur;

            if (cacheRest > size) {
                mpCacheCur += size;
            } else {
                mpCacheCur = mpCacheEnd;
                mpCur += size - cacheRest;
            }
        }

        void Relocate(void *pDst, u32 size) {
            u32 cacheRest = mpCacheEnd - mpCacheCur;
            u8 *pDst8 = static_cast<u8 *>(pDst);

            if (cacheRest >= size) {
                std::memmove(pDst8, mpCacheCur, size);
                mpCacheCur += size;
            } else {
                u32 streamSize = size - cacheRest;
                std::memmove(pDst8, mpCacheCur, cacheRest);
                std::memmove(pDst8 + cacheRest, mpCur, streamSize);
                mpCacheCur = mpCacheEnd;
                mpCur += streamSize;
            }
        }

        const void *GetPtr(u32 size) {
            const void *p = mpCur;
            mpCur += size;
            return p;
        }

        const u8 *mpBegin; // at 0x0
        const u8 *mpCur;   // at 0x4
        const u8 *mpEnd;   // at 0x8
        u8 *mpCacheBegin;  // at 0xC
        u8 *mpCacheCur;    // at 0x10
        u8 *mpCacheEnd;    // at 0x14
        u32 mRequestSize;  // at 0x18
    };

  public:
    struct ConstructContext {
      public:
        enum Operation {
            OP_DISPATCH,
            OP_FILE_HEADER,
            OP_GLGR,
            OP_FINF,
            OP_CMAP,
            OP_CWDH,
            OP_TGLP,
            OP_COPY_SHEET,
            OP_EXPAND_SHEET,
            OP_COPY,
            OP_SKIP,
            OP_EXPAND,
            OP_FATAL_ERROR,

            OP_MAX,
            OP_INVALID
        };

        void Init(void *pBuffer, u32 bufferSize, const char *pGroups) {
            reader.Init();
            pGlyphGroups = pGroups;
            pAdjustTable = NULL;
            pBufferBegin = static_cast<u8 *>(pBuffer);
            pBufferEnd = pBufferBegin + bufferSize;
            pBufferCur = pBufferBegin;
            pUncompContext = static_cast<CXUncompContextHuffman *>(
                RoundDown(pBufferEnd - sizeof(CXUncompContextHuffman), 4));
            nextOp = OP_INVALID;
            taskSize = 0;
            numBlocks = 1;
            blockIndex = 0;
            sheetIndex = 0;
            numSheets = 0;
            glyphsPerSheet = 0;
            streamPos = 0;
            pFontInfo = NULL;
            pLastWidth = NULL;
            pLastCodeMap = NULL;
            op = OP_INVALID;
        }

        void SetGlyphGroups(u16 *pTable, u16 sheets, u16 perSheet, u32 blocks) {
            pAdjustTable = pTable;
            numSheets = sheets;
            glyphsPerSheet = perSheet;
            numBlocks = blocks;
        }

        template <typename T> T GetBuffer() {
            return reinterpret_cast<T>(pBufferCur);
        }

        u32 GetBufferRestSize() const {
            return pBufferEnd - pBufferCur;
        }

        void AlignBuffer(u32 align) {
            pBufferCur = static_cast<u8 *>(RoundUp(pBufferCur, align));
        }

        void AdvanceBuffer(u32 size) {
            pBufferCur += size;
        }

        bool IsSheetLoaded(int i) const {
            return pAdjustTable[i] != ADJUST_SHEET_NOT_LOADED;
        }

        bool IsSheetLoaded() const {
            return IsSheetLoaded(sheetIndex);
        }

        bool IsSheetRemaining() const {
            return sheetIndex < numSheets;
        }

        bool IsBlockRemaining() const {
            return blockIndex < numBlocks;
        }

        void NextBlock() {
            blockIndex++;
        }

        void NextSheet() {
            sheetIndex++;
        }

        void StartCopy(u32 size, Operation next = OP_DISPATCH) {
            taskSize = size;
            op = OP_COPY;
            nextOp = next;
        }

        void StartSkip(u32 size, Operation next = OP_DISPATCH) {
            taskSize = size;
            op = OP_SKIP;
            nextOp = next;
        }

        void StartExpand(u32 size, Operation next = OP_DISPATCH) {
            taskSize = size;
            op = OP_EXPAND;
            nextOp = next;
        }

        CachedStreamReader &GetReader() {
            return reader;
        }

        void FinishTask() {
            op = nextOp;
        }

        void ProgressTask(u32 size) {
            taskSize -= Min(size, taskSize);
        }

        FontInformation *pFontInfo;             // at 0x0
        FontWidth *pLastWidth;                  // at 0x4
        FontCodeMap *pLastCodeMap;              // at 0x8
        Operation op;                           // at 0xC
        BinaryBlockHeader blockHeader;          // at 0x10
        u32 streamPos;                          // at 0x18
        CachedStreamReader reader;              // at 0x1C
        CXUncompContextHuffman *pUncompContext; // at 0x38
        const char *pGlyphGroups;               // at 0x3C
        u16 *pAdjustTable;                      // at 0x40
        u8 *pBufferBegin;                       // at 0x44
        u8 *pBufferEnd;                         // at 0x48
        u8 *pBufferCur;                         // at 0x4C
        Operation nextOp;                       // at 0x50
        u32 taskSize;                           // at 0x54
        u32 numBlocks;                          // at 0x58
        u32 blockIndex;                         // at 0x5C
        u16 sheetIndex;                         // at 0x60
        u16 numSheets;                          // at 0x62
        u16 glyphsPerSheet;                     // at 0x64
    };

  public:
    static const char LOAD_GLYPH_ALL[1];

    static const u32 SIGNATURE = 'RFNA';
    static const u32 SIGNATURE_GLGR = 'GLGR';
    static const u32 SIGNATURE_FINF = 'FINF';
    static const u32 SIGNATURE_CMAP = 'CMAP';
    static const u32 SIGNATURE_CWDH = 'CWDH';
    static const u32 SIGNATURE_TGLP = 'TGLP';

    static const u16 SHEET_FORMAT_COMPRESSED = 0x8000;
    static const u16 SHEET_FORMAT_MASK = 0x7FFF;

    static const u16 ADJUST_SHEET_NOT_LOADED = 0xFFFF;

  public:
    ArchiveFontBase();
    virtual ~ArchiveFontBase(); // at 0x8

    virtual CharWidths GetCharWidths(u16 ch) const; // at 0x4C
    virtual bool HasGlyph(u16 ch) const;            // at 0x54

  protected:
    class GlyphGroupInfo {
      public:
        explicit GlyphGroupInfo(const void *pBrfna) {
            mpHeader = static_cast<const BinaryFileHeader *>(pBrfna);
            mpBlock =
                reinterpret_cast<const FontGlyphGroupBlock *>(mpHeader + 1);

            mSheetFlagsSize = sizeof(u32) * DivUp(mpBlock->body.numSheet, 32);
            mCWDHFlagsSize = sizeof(u32) * DivUp(mpBlock->body.numCWDH, 32);
            mCMAPFlagsSize = sizeof(u32) * DivUp(mpBlock->body.numCMAP, 32);

            u32 namesEnd =
                RoundUp(RoundUp(sizeof(BinaryFileHeader) +
                                    sizeof(BinaryBlockHeader),
                                2) +
                            offsetof(FontGlyphGroupData, nameOffsets),
                        2) +
                sizeof(u16) * mpBlock->body.numSet;

            u32 sheetSizesPos = RoundUp(namesEnd, 4);
            u32 cwdhSizesPos = RoundUp(
                sheetSizesPos + sizeof(u32) * mpBlock->body.numSheet, 4);
            u32 cmapSizesPos =
                RoundUp(cwdhSizesPos + sizeof(u32) * mpBlock->body.numCWDH, 4);
            u32 sheetUsesPos =
                RoundUp(cmapSizesPos + sizeof(u32) * mpBlock->body.numCMAP, 4);
            u32 cwdhUsesPos = RoundUp(
                sheetUsesPos + mSheetFlagsSize * mpBlock->body.numSet, 4);
            u32 cmapUsesPos =
                RoundUp(cwdhUsesPos + mCWDHFlagsSize * mpBlock->body.numSet, 4);

            mpSheetSizes = static_cast<const u32 *>(
                AddOffsetToPtr(mpHeader, sheetSizesPos));
            mpCWDHSizes = static_cast<const u32 *>(
                AddOffsetToPtr(mpHeader, cwdhSizesPos));
            mpCMAPSizes = static_cast<const u32 *>(
                AddOffsetToPtr(mpHeader, cmapSizesPos));
            mpSheetUses = static_cast<const u32 *>(
                AddOffsetToPtr(mpHeader, sheetUsesPos));
            mpCWDHUses =
                static_cast<const u32 *>(AddOffsetToPtr(mpHeader, cwdhUsesPos));
            mpCMAPUses =
                static_cast<const u32 *>(AddOffsetToPtr(mpHeader, cmapUsesPos));
        }

        u16 GetNumBlock() const {
            return mpHeader->dataBlocks;
        }

        u32 GetBlockSize() const {
            return mpBlock->header.size;
        }

        const char *GetSetName(int set) const {
            return static_cast<const char *>(
                AddOffsetToPtr(mpHeader, mpBlock->body.nameOffsets[set]));
        }

        u32 GetSheetSize(int sheet) const {
            return mpSheetSizes[sheet];
        }

        u32 GetCWDHSize(int cwdh) const {
            return mpCWDHSizes[cwdh];
        }

        u32 GetCMAPSize(int cmap) const {
            return mpCMAPSizes[cmap];
        }

        bool IsSheetUsed(int set, int sheet) const {
            return TestFlag(mpSheetUses, set * mSheetFlagsSize * 8 + sheet);
        }

        u32 GetSheetFlags(int set, int word) const {
            return mpSheetUses[set * mSheetFlagsSize / 4 + word];
        }

        u32 GetCWDHFlags(int set, int word) const {
            return mpCWDHUses[set * mCWDHFlagsSize / 4 + word];
        }

        u32 GetCMAPFlags(int set, int word) const {
            return mpCMAPUses[set * mCMAPFlagsSize / 4 + word];
        }

        u32 GetUncompSheetSize() const {
            return mpBlock->body.sheetSize;
        }

        u16 GetGlyphsPerSheet() const {
            return mpBlock->body.glyphsPerSheet;
        }

        u16 GetNumSet() const {
            return mpBlock->body.numSet;
        }

        int GetNumSheet() const {
            return mpBlock->body.numSheet;
        }

        u16 GetNumCWDH() const {
            return mpBlock->body.numCWDH;
        }

        u16 GetNumCMAP() const {
            return mpBlock->body.numCMAP;
        }

      private:
        static int DivUp(int x, int div) {
            return (x + div - 1) / div;
        }

        static bool TestFlag(const u32 *pFlags, u32 bit) {
            return ((pFlags[bit / 32] << (bit % 32)) & (1u << 31)) != 0;
        }

        const BinaryFileHeader *mpHeader;   // at 0x0
        const FontGlyphGroupBlock *mpBlock; // at 0x4
        const u16 *mpNames;                 // at 0x8
        const u32 *mpSheetSizes;            // at 0xC
        const u32 *mpCWDHSizes;             // at 0x10
        const u32 *mpCMAPSizes;             // at 0x14
        const u32 *mpSheetUses;             // at 0x18
        const u32 *mpCWDHUses;              // at 0x1C
        const u32 *mpCMAPUses;              // at 0x20
        u32 mSheetFlagsSize;                // at 0x24
        u32 mCWDHFlagsSize;                 // at 0x28
        u32 mCMAPFlagsSize;                 // at 0x2C
    };

    static const u32 HEADER_SIZE = 16 * 1024;

    static bool IsNullString(const char *pStr) {
        return *pStr == '\0';
    }

    void SetResourceBuffer(void *pBuffer, FontInformation *pInfo,
                           u16 *pAdjustTable);

    u16 AdjustIndex(u16 index) const;

    static bool IncludeName(const char *pNameList, const char *pName);
    static bool IsValidResource(const void *pBrfna, u32 size);

    static Result RequestMoreData(ConstructContext *pContext,
                                  CachedStreamReader *pReader, u32 size);

    static Result ConstructOpDispatch(ConstructContext *pContext,
                                      CachedStreamReader *pReader);
    static Result ConstructOpAnalyzeFileHeader(ConstructContext *pContext,
                                               CachedStreamReader *pReader);
    static Result ConstructOpAnalyzeFINF(ConstructContext *pContext,
                                         CachedStreamReader *pReader);
    static Result ConstructOpAnalyzeCMAP(ConstructContext *pContext,
                                         CachedStreamReader *pReader);
    static Result ConstructOpAnalyzeCWDH(ConstructContext *pContext,
                                         CachedStreamReader *pReader);
    static Result ConstructOpAnalyzeTGLP(ConstructContext *pContext,
                                         CachedStreamReader *pReader);
    static Result ConstructOpCopy(ConstructContext *pContext,
                                  CachedStreamReader *pReader);
    static Result ConstructOpSkip(ConstructContext *pContext,
                                  CachedStreamReader *pReader);
    static Result ConstructOpFatalError(ConstructContext *pContext,
                                        CachedStreamReader *pReader);

    u16 *mpAdjustTable; // at 0x1C
};

inline void
ArchiveFontBase::CachedStreamReader::Read(ConstructContext *pContext,
                                          u32 size) {
    Read(pContext->GetBuffer<void *>(), size);
    pContext->AdvanceBuffer(size);
}

inline u32
ArchiveFontBase::CachedStreamReader::ReadTask(ConstructContext *pContext) {
    u32 size = Min(static_cast<u32>(mpEnd - mpCur), pContext->taskSize);
    std::memcpy(pContext->GetBuffer<void *>(), mpCur, size);
    mpCur += size;
    pContext->AdvanceBuffer(size);
    pContext->ProgressTask(size);
    return pContext->taskSize;
}

} // namespace detail
} // namespace ut
} // namespace nw4r

#endif
