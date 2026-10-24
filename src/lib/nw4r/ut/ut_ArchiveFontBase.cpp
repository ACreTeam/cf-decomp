#include <nw4r/ut.h>

#include <cstring>

namespace nw4r {
namespace ut {
namespace detail {

const char ArchiveFontBase::LOAD_GLYPH_ALL[1] = "";

ArchiveFontBase::ArchiveFontBase() : mpAdjustTable(NULL) {
}

ArchiveFontBase::~ArchiveFontBase() {
}

CharWidths ArchiveFontBase::GetCharWidths(u16 ch) const {
    u16 index = GetGlyphIndex(ch);

    if (AdjustIndex(index) == GLYPH_INDEX_NOT_FOUND) {
        index = GetFontInfo()->alterCharIndex;
    }

    return GetCharWidthsFromIndex(index);
}

bool ArchiveFontBase::HasGlyph(u16 ch) const {
    u16 index = FindGlyphIndex(ch);

    if (index != GLYPH_INDEX_NOT_FOUND) {
        return GLYPH_INDEX_NOT_FOUND != AdjustIndex(index);
    }

    return false;
}

void ArchiveFontBase::SetResourceBuffer(void *pBuffer, FontInformation *pInfo,
                                        u16 *pAdjustTable) {
    ResFontBase::SetResourceBuffer(pBuffer, pInfo);
    mpAdjustTable = pAdjustTable;
}

u16 ArchiveFontBase::AdjustIndex(u16 index) const {
    const FontTextureGlyph &rGlyph = *GetFontInfo()->pGlyph;
    int perSheet = rGlyph.sheetRow * rGlyph.sheetLine;
    int sheet = index / perSheet;
    u16 adjust = mpAdjustTable[sheet];

    return adjust == ADJUST_SHEET_NOT_LOADED ? GLYPH_INDEX_NOT_FOUND
                                             : static_cast<u16>(index - adjust);
}

bool ArchiveFontBase::IncludeName(const char *pNameList, const char *pName) {
    u32 nameLen = std::strlen(pName);
    const char *pFound = pNameList - 1;

    while (true) {
        pFound = std::strstr(pFound + 1, pName);
        if (pFound == NULL) {
            return false;
        }

        if (pFound != pNameList) {
            const char *pPrev = pFound - 1;
            while (pNameList < pPrev && *pPrev == ' ') {
                pPrev--;
            }

            if (*pPrev != ',') {
                continue;
            }
        }

        const char *pComma = std::strchr(pFound, ',');
        u32 foundLen;

        if (pComma != NULL) {
            foundLen = pComma - pFound;
        } else {
            foundLen = std::strlen(pFound);
        }

        const char *pPos = pFound + nameLen;
        const char *pEnd = pFound + foundLen;
        while (pPos < pEnd && *pPos == ' ') {
            pPos++;
        }

        if (pPos == pEnd) {
            return true;
        }
    }

    return false;
}

bool ArchiveFontBase::IsValidResource(const void *pBrfna, u32 size) {
    const BinaryFileHeader *pHeader =
        static_cast<const BinaryFileHeader *>(pBrfna);

    if (!IsValidBinaryFile(pHeader, SIGNATURE, NW4R_VERSION(1, 4), 2)) {
        return false;
    }

    const BinaryBlockHeader *pBlock =
        reinterpret_cast<const BinaryBlockHeader *>(pHeader + 1);

    if (pBlock->kind != SIGNATURE_GLGR) {
        return false;
    }

    if (sizeof(BinaryFileHeader) + pBlock->size > size) {
        return false;
    }

    return true;
}

ArchiveFontBase::Result
ArchiveFontBase::RequestMoreData(ConstructContext *pContext,
                                 CachedStreamReader *pReader, u32 size) {
    pContext->streamPos += pReader->GetReadSize();
    return pReader->RequestData(pContext, size) ? RESULT_NEED_DATA
                                                : RESULT_ERROR;
}

ArchiveFontBase::Result
ArchiveFontBase::ConstructOpDispatch(ConstructContext *pContext,
                                     CachedStreamReader *pReader) {
    if (!pContext->IsBlockRemaining()) {
        return RESULT_DONE;
    }

    if (pReader->GetRestSize() < sizeof(BinaryBlockHeader)) {
        return RequestMoreData(pContext, pReader, sizeof(BinaryBlockHeader));
    }

    pReader->Read(&pContext->blockHeader, sizeof(pContext->blockHeader));

    switch (pContext->blockHeader.kind) {
    case SIGNATURE_GLGR: {
        pContext->op = ConstructContext::OP_GLGR;
        break;
    }

    case SIGNATURE_FINF: {
        pContext->op = ConstructContext::OP_FINF;
        break;
    }

    case SIGNATURE_CMAP: {
        pContext->op = ConstructContext::OP_CMAP;
        break;
    }

    case SIGNATURE_CWDH: {
        pContext->op = ConstructContext::OP_CWDH;
        break;
    }

    case SIGNATURE_TGLP: {
        pContext->op = ConstructContext::OP_TGLP;
        break;
    }

    default: {
        pContext->op = ConstructContext::OP_FATAL_ERROR;
        return RESULT_ERROR;
    }
    }

    pContext->NextBlock();
    pContext->AlignBuffer(4);
    return RESULT_CONTINUE;
}

ArchiveFontBase::Result
ArchiveFontBase::ConstructOpAnalyzeFileHeader(ConstructContext *pContext,
                                              CachedStreamReader *pReader) {
    if (pReader->GetRestSize() < sizeof(BinaryFileHeader)) {
        return RequestMoreData(pContext, pReader, sizeof(BinaryFileHeader));
    }

    if (pContext->GetBufferRestSize() < sizeof(BinaryFileHeader)) {
        return RESULT_ERROR;
    }

    pReader->Read(pContext->GetBuffer<void *>(), sizeof(BinaryFileHeader));
    pContext->op = ConstructContext::OP_DISPATCH;
    return RESULT_CONTINUE;
}

ArchiveFontBase::Result
ArchiveFontBase::ConstructOpAnalyzeFINF(ConstructContext *pContext,
                                        CachedStreamReader *pReader) {
#pragma unused(pReader)
    u32 size = pContext->blockHeader.size - sizeof(BinaryBlockHeader);

    if (pContext->GetBufferRestSize() < size) {
        return RESULT_ERROR;
    }

    pContext->pFontInfo = pContext->GetBuffer<FontInformation *>();
    pContext->StartCopy(size);
    return RESULT_CONTINUE;
}

ArchiveFontBase::Result
ArchiveFontBase::ConstructOpAnalyzeCMAP(ConstructContext *pContext,
                                        CachedStreamReader *pReader) {
#pragma unused(pReader)
    FontCodeMap *pMap = pContext->GetBuffer<FontCodeMap *>();

    if (pContext->pLastCodeMap != NULL) {
        pContext->pLastCodeMap->pNext = pMap;
    } else {
        pContext->pFontInfo->pMap = pMap;
    }

    pContext->pLastCodeMap = pMap;

    u32 size = pContext->blockHeader.size - sizeof(BinaryBlockHeader);

    if (pContext->GetBufferRestSize() < size) {
        return RESULT_ERROR;
    }

    pContext->StartCopy(size);
    return RESULT_CONTINUE;
}

ArchiveFontBase::Result
ArchiveFontBase::ConstructOpAnalyzeCWDH(ConstructContext *pContext,
                                        CachedStreamReader *pReader) {
#pragma unused(pReader)
    FontWidth *pWidth = pContext->GetBuffer<FontWidth *>();

    if (pContext->pLastWidth != NULL) {
        pContext->pLastWidth->pNext = pWidth;
    } else {
        pContext->pFontInfo->pWidth = pWidth;
    }

    pContext->pLastWidth = pWidth;

    u32 size = pContext->blockHeader.size - sizeof(BinaryBlockHeader);

    if (pContext->GetBufferRestSize() < size) {
        return RESULT_ERROR;
    }

    pContext->StartCopy(size);
    return RESULT_CONTINUE;
}

ArchiveFontBase::Result
ArchiveFontBase::ConstructOpAnalyzeTGLP(ConstructContext *pContext,
                                        CachedStreamReader *pReader) {
    if (pReader->GetRestSize() < sizeof(FontTextureGlyph)) {
        return RequestMoreData(pContext, pReader, sizeof(FontTextureGlyph));
    }

    if (pContext->GetBufferRestSize() < sizeof(FontTextureGlyph)) {
        return RESULT_ERROR;
    }

    pContext->pFontInfo->pGlyph = pContext->GetBuffer<FontTextureGlyph *>();
    pReader->Read(pContext, sizeof(FontTextureGlyph));

    bool compressed = (pContext->pFontInfo->pGlyph->sheetFormat &
                       SHEET_FORMAT_COMPRESSED) != 0;
    pContext->pFontInfo->pGlyph->sheetFormat &= SHEET_FORMAT_MASK;

    u16 numLoaded = 0;
    for (int i = 0; i < pContext->numSheets; i++) {
        if (pContext->IsSheetLoaded(i)) {
            numLoaded++;
        }
    }
    pContext->pFontInfo->pGlyph->sheetNum = numLoaded;

    u32 sheetPos =
        reinterpret_cast<u32>(pContext->pFontInfo->pGlyph->sheetImage);

    pContext->AlignBuffer(32);
    pContext->pFontInfo->pGlyph->sheetImage = pContext->GetBuffer<u8 *>();

    ConstructContext::Operation next = compressed
                                           ? ConstructContext::OP_EXPAND_SHEET
                                           : ConstructContext::OP_COPY_SHEET;

    u32 streamPos = pContext->streamPos + pReader->GetReadSize();
    pContext->StartSkip(sheetPos - streamPos, next);
    return RESULT_CONTINUE;
}

ArchiveFontBase::Result
ArchiveFontBase::ConstructOpCopy(ConstructContext *pContext,
                                 CachedStreamReader *pReader) {
    u32 rest = pReader->ReadTask(pContext);

    if (rest == 0) {
        pContext->FinishTask();
    } else {
        return RequestMoreData(pContext, pReader, rest);
    }

    return RESULT_CONTINUE;
}

ArchiveFontBase::Result
ArchiveFontBase::ConstructOpSkip(ConstructContext *pContext,
                                 CachedStreamReader *pReader) {
    u32 size = Min(pContext->taskSize, pReader->GetRestSize());
    pReader->Skip(size);
    pContext->ProgressTask(size);

    if (pContext->taskSize == 0) {
        pContext->FinishTask();
    } else {
        return RequestMoreData(pContext, pReader, pContext->taskSize);
    }

    return RESULT_CONTINUE;
}

ArchiveFontBase::Result
ArchiveFontBase::ConstructOpFatalError(ConstructContext *pContext,
                                       CachedStreamReader *pReader) {
#pragma unused(pReader)
    pContext->op = ConstructContext::OP_FATAL_ERROR;
    return RESULT_ERROR;
}

void ArchiveFontBase::CachedStreamReader::Init() {
    mpBegin = NULL;
    mpCur = NULL;
    mpEnd = NULL;
    mpCacheBegin = NULL;
    mpCacheCur = NULL;
    mpCacheEnd = NULL;
    mRequestSize = 0;
}

void ArchiveFontBase::CachedStreamReader::Attach(const void *pStream,
                                                 u32 size) {
    mpBegin = static_cast<const u8 *>(pStream);
    mpCur = mpBegin;
    mpEnd = mpBegin + size;
}

u32 ArchiveFontBase::CachedStreamReader::GetRestSize() const {
    return (mpEnd - mpCur) + (mpCacheEnd - mpCacheCur);
}

void ArchiveFontBase::CachedStreamReader::Read(void *pDst, u32 size) {
    u32 cacheRest = mpCacheEnd - mpCacheCur;
    u8 *pDst8 = static_cast<u8 *>(pDst);

    if (cacheRest >= size) {
        std::memcpy(pDst8, mpCacheCur, size);
        mpCacheCur += size;
    } else {
        u32 streamSize = size - cacheRest;
        std::memcpy(pDst8, mpCacheCur, cacheRest);
        std::memcpy(pDst8 + cacheRest, mpCur, streamSize);
        mpCacheCur = mpCacheEnd;
        mpCur += streamSize;
    }
}

bool ArchiveFontBase::CachedStreamReader::RequestData(
    ConstructContext *pContext, u32 size) {
    u32 rest = GetRestSize();

    if (rest == 0) {
        mpCacheBegin = NULL;
        mpCacheCur = NULL;
        mpCacheEnd = NULL;
        mRequestSize = 0;
    } else {
        if (pContext->GetBufferRestSize() < size * 2) {
            return false;
        }

        u8 *pCache = pContext->GetBuffer<u8 *>() + size;
        Relocate(pCache, rest);

        mpCacheBegin = pCache;
        mpCacheCur = mpCacheBegin;
        mpCacheEnd = mpCacheBegin + rest;
        mRequestSize = size;
    }

    return true;
}

} // namespace detail
} // namespace ut
} // namespace nw4r
