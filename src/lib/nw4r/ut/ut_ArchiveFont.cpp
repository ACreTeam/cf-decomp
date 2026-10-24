#include <nw4r/ut.h>

#include <revolution/gx.h>
#include <revolution/os.h>

#include <cstring>

namespace nw4r {
namespace ut {

ArchiveFont::ArchiveFont()
    : mNumSheets(0), mNumCacheSlots(0), mpSheetToSlot(NULL),
      mpSlotToSheet(NULL), mpSheets(NULL), mpCache(NULL) {
}

ArchiveFont::~ArchiveFont() {
}

u32 ArchiveFont::GetRequireBufferSize(const void *pBrfna, const char *pGroups,
                                      int numCache) {
    if (!IsValidResource(pBrfna, HEADER_SIZE)) {
        return 0;
    }

    GlyphGroupInfo info(pBrfna);
    u32 dataSize = GetRequireDataSize(&info, pGroups, NULL);

    int numSheet = info.GetNumSheet();
    int sheetSize = info.GetUncompSheetSize();

    int numSlots;
    if (numCache > numSheet) {
        numSlots = numSheet;
    } else if (numCache < 1) {
        numSlots = 1;
    } else {
        numSlots = numCache;
    }

    u32 cacheSize = RoundUp((numSlots + 1) * sizeof(CacheLink), 4);
    cacheSize = RoundUp(numSheet * sizeof(u32) + cacheSize, 2);
    cacheSize = RoundUp(numSheet * sizeof(u32) + cacheSize, 2);
    cacheSize = RoundUp(numSlots * sizeof(u16) + cacheSize, 32);
    cacheSize = RoundUp(sheetSize * numSlots + cacheSize, 4);

    return dataSize + cacheSize;
}

ArchiveFont::Result ArchiveFont::StreamingConstruct(ConstructContext *pContext,
                                                    const void *pStream,
                                                    u32 size) {
    if (GetFontInfo() != NULL) {
        return pContext->IsBlockRemaining() ? RESULT_ERROR : RESULT_DONE;
    }

    Result result = RESULT_CONTINUE;
    CachedStreamReader &rReader = pContext->GetReader();
    rReader.Attach(pStream, size);

    while (result == RESULT_CONTINUE) {
        switch (pContext->op) {
        case ConstructContext::OP_DISPATCH: {
            result = ConstructOpDispatch(pContext, &rReader);
            break;
        }

        case ConstructContext::OP_FILE_HEADER: {
            result = ConstructOpAnalyzeFileHeader(pContext, &rReader);
            break;
        }

        case ConstructContext::OP_GLGR: {
            result = ConstructOpAnalyzeGLGR(pContext, &rReader);
            break;
        }

        case ConstructContext::OP_FINF: {
            result = ConstructOpAnalyzeFINF(pContext, &rReader);
            break;
        }

        case ConstructContext::OP_CMAP: {
            result = ConstructOpAnalyzeCMAP(pContext, &rReader);
            break;
        }

        case ConstructContext::OP_CWDH: {
            result = ConstructOpAnalyzeCWDH(pContext, &rReader);
            break;
        }

        case ConstructContext::OP_COPY: {
            result = ConstructOpCopy(pContext, &rReader);
            break;
        }

        case ConstructContext::OP_SKIP: {
            result = ConstructOpSkip(pContext, &rReader);
            break;
        }

        case ConstructContext::OP_FATAL_ERROR: {
            result = ConstructOpFatalError(pContext, &rReader);
            break;
        }

        case ConstructContext::OP_TGLP: {
            result = ConstructOpAnalyzeTGLP(pContext, &rReader);
            break;
        }

        case ConstructContext::OP_EXPAND_SHEET: {
            result = ConstructOpPrepairExpandSheet(pContext, &rReader);

            if (pContext->op == ConstructContext::OP_COPY) {
                mpSheets[mNumSheets] = pContext->GetBuffer<void *>();
                mNumSheets++;
            }
            break;
        }

        case ConstructContext::OP_COPY_SHEET: {
            pContext->op = ConstructContext::OP_FATAL_ERROR;
            return RESULT_ERROR;
        }

        default: {
            pContext->op = ConstructContext::OP_FATAL_ERROR;
            return RESULT_ERROR;
        }
        }
    }

    if (result == RESULT_DONE && GetFontInfo() == NULL) {
        DCFlushRange(pContext->pBufferBegin,
                     pContext->pBufferEnd - pContext->pBufferBegin);

        SetResourceBuffer(pContext->pBufferBegin, pContext->pFontInfo,
                          pContext->pAdjustTable);

        if (AdjustIndex(GetFontInfo()->alterCharIndex) ==
            GLYPH_INDEX_NOT_FOUND) {
            GetFontInfo()->alterCharIndex = 0;
        }

        for (int i = 0; i < mNumSheets; i++) {
            mpSheetToSlot[i] = CACHE_NONE;
        }

        for (int i = 0; i < mNumCacheSlots; i++) {
            mpSlotToSheet[i] = CACHE_NONE;
        }

        InitReaderFunc(GetEncoding());
    }

    return result;
}

bool ArchiveFont::Construct(void *pBuffer, u32 bufferSize, const void *pBrfna,
                            const char *pGroups) {
    ConstructContext context;
    u32 fileSize = static_cast<const BinaryFileHeader *>(pBrfna)->fileSize;

    context.Init(pBuffer, bufferSize, pGroups);
    context.op = ConstructContext::OP_FILE_HEADER;

    return StreamingConstruct(&context, pBrfna, fileSize) == RESULT_DONE;
}

void ArchiveFont::GetGlyph(Glyph *pGlyph, u16 ch) const {
    u16 index = GetGlyphIndex(ch);
    u16 adjusted = AdjustIndex(index);

    if (adjusted == GLYPH_INDEX_NOT_FOUND) {
        index = GetFontInfo()->alterCharIndex;
        adjusted = AdjustIndex(index);
    }

    const FontTextureGlyph *pTexGlyph = GetFontInfo()->pGlyph;
    int sheet = GetSheetIndex(adjusted, pTexGlyph);
    int slot = mpSheetToSlot[sheet];

    if (slot == CACHE_NONE) {
        slot = mpCacheLinks[mNumCacheLinks].next;

        LoadCacheSlot(sheet, slot, pTexGlyph->sheetSize);
    }

    CacheLink *pLink = GetCacheLink(slot);

    if (IsCacheLinked(pLink)) {
        UnlinkCache(pLink);
        LinkCacheLast(pLink, slot);
    }

    GetGlyphFromIndex(pGlyph, index, slot);
}

u32 ArchiveFont::GetRequireDataSize(const GlyphGroupInfo *pInfo,
                                    const char *pGroups, u32 *pNumSheets) {
    u32 sheetsSize = 0;
    u32 cwdhSize = 0;
    u32 cmapSize = 0;
    u32 numSheets = 0;

    for (int word = 0; word * 32 < pInfo->GetNumSheet(); word++) {
        u32 flags = 0;

        for (int set = 0; set < pInfo->GetNumSet(); set++) {
            const char *pName = pInfo->GetSetName(set);

            if (IsNullString(pGroups) || IncludeName(pGroups, pName)) {
                flags |= pInfo->GetSheetFlags(set, word);
            }
        }

        for (int bit = 0; bit < 32; bit++) {
            if (((flags << bit) & (1u << 31)) != 0) {
                numSheets++;
                sheetsSize += pInfo->GetSheetSize(word * 32 + bit);
            }
        }
    }

    for (int word = 0; word * 32 < pInfo->GetNumCWDH(); word++) {
        u32 flags = 0;

        for (int set = 0; set < pInfo->GetNumSet(); set++) {
            const char *pName = pInfo->GetSetName(set);

            if (IsNullString(pGroups) || IncludeName(pGroups, pName)) {
                flags |= pInfo->GetCWDHFlags(set, word);
            }
        }

        for (int bit = 0; bit < 32; bit++) {
            if (((flags << bit) & (1u << 31)) != 0) {
                cwdhSize += pInfo->GetCWDHSize(word * 32 + bit) -
                            sizeof(BinaryBlockHeader);
            }
        }
    }

    for (int word = 0; word * 32 < pInfo->GetNumCMAP(); word++) {
        u32 flags = 0;

        for (int set = 0; set < pInfo->GetNumSet(); set++) {
            const char *pName = pInfo->GetSetName(set);

            if (IsNullString(pGroups) || IncludeName(pGroups, pName)) {
                flags |= pInfo->GetCMAPFlags(set, word);
            }
        }

        for (int bit = 0; bit < 32; bit++) {
            if (((flags << bit) & (1u << 31)) != 0) {
                cmapSize += pInfo->GetCMAPSize(word * 32 + bit) -
                            sizeof(BinaryBlockHeader);
            }
        }
    }

    if (pNumSheets != NULL) {
        *pNumSheets = numSheets;
    }

    u32 dataSize = cwdhSize + sheetsSize +
                   (sizeof(BinaryFileHeader) + sizeof(FontInformation) +
                    sizeof(FontTextureGlyph)) +
                   cmapSize;
    u32 tempSize = (sizeof(BinaryFileHeader) + pInfo->GetBlockSize()) * 2;

    return Max(dataSize, tempSize);
}

u16 *ArchiveFont::InitSheetCache(void *pBuffer, u32 size, int numAllSheets,
                                 int numSheets, u32 sheetSize) {
    u8 *pBuffer8 = static_cast<u8 *>(pBuffer);

    u32 adjustSize = RoundUp(numAllSheets * sizeof(u16), 2);
    u32 sheetToSlotSize = RoundUp(numSheets * sizeof(u16), 2);
    u32 sheetsSize = RoundUp(numSheets * sizeof(void *), 4);

    u32 rest =
        size - adjustSize - sheetToSlotSize - sheetsSize - sizeof(CacheLink);
    if (rest > size) {
        return NULL;
    }

    int numSlots = rest / (sheetSize + sizeof(CacheLink) + sizeof(u16));

    mCacheLinkHead = 0;
    mpCacheLinks = reinterpret_cast<CacheLink *>(pBuffer8);
    mNumCacheLinks = numSlots;

    u32 adjustPos = RoundUp((numSlots + 1) * sizeof(CacheLink), 2);
    u32 sheetToSlotPos = RoundUp(adjustPos + adjustSize, 2);
    u32 slotToSheetPos = RoundUp(sheetToSlotPos + sheetToSlotSize, 2);
    u32 sheetsPos = RoundUp(slotToSheetPos + numSlots * sizeof(u16), 4);
    u32 cachePos = RoundUp(sheetsPos + sheetsSize, 32);

    InitCacheLinks(numSlots);

    CacheLink &rEnd = mpCacheLinks[mNumCacheLinks];
    rEnd.next = 0;
    rEnd.prev = numSlots - 1;

    mNumSheets = 0;
    mNumCacheSlots = numSlots;
    mpSheetToSlot = reinterpret_cast<u16 *>(pBuffer8 + sheetToSlotPos);
    mpSlotToSheet = reinterpret_cast<u16 *>(pBuffer8 + slotToSheetPos);
    mpSheets = reinterpret_cast<const void **>(pBuffer8 + sheetsPos);
    mpCache = pBuffer8 + cachePos;

    return reinterpret_cast<u16 *>(pBuffer8 + adjustPos);
}

void ArchiveFont::GetGlyphFromIndex(Glyph *pGlyph, u16 index, u16 slot) const {
    const FontTextureGlyph &rTexGlyph = *GetFontInfo()->pGlyph;

    u32 cellsInASheet = rTexGlyph.sheetRow * rTexGlyph.sheetLine;
    u32 cell = index % cellsInASheet;

    u32 unitX = cell % rTexGlyph.sheetRow;
    u32 unitY = cell / rTexGlyph.sheetRow;

    u32 pixelX = unitX * (rTexGlyph.cellWidth + 1);
    u32 pixelY = unitY * (rTexGlyph.cellHeight + 1);

    pGlyph->pTexture = mpCache + slot * rTexGlyph.sheetSize;

    pGlyph->widths = GetCharWidthsFromIndex(index);
    pGlyph->height = rTexGlyph.cellHeight;

    pGlyph->texFormat = static_cast<GXTexFmt>(rTexGlyph.sheetFormat);

    pGlyph->texWidth = rTexGlyph.sheetWidth;
    pGlyph->texHeight = rTexGlyph.sheetHeight;

    pGlyph->cellX = pixelX + 1;
    pGlyph->cellY = pixelY + 1;
}

ArchiveFont::Result
ArchiveFont::ConstructOpAnalyzeGLGR(ConstructContext *pContext,
                                    CachedStreamReader *pReader) {
    u8 *pTempHead;
    u8 *pTempTail;

    {
        const BinaryFileHeader &rHeader =
            *pContext->GetBuffer<const BinaryFileHeader *>();
        u32 bodySize = pContext->blockHeader.size - sizeof(BinaryBlockHeader);
        u32 infoSize = sizeof(BinaryFileHeader) + pContext->blockHeader.size;

        if (rHeader.signature != SIGNATURE) {
            return RESULT_ERROR;
        }

        if (pReader->GetRestSize() < bodySize) {
            return RequestMoreData(pContext, pReader, bodySize);
        }

        if (pContext->GetBufferRestSize() < infoSize) {
            return RESULT_ERROR;
        }

        pTempHead = pContext->GetBuffer<u8 *>();
        u8 *pBlockHead = pTempHead + sizeof(BinaryFileHeader);
        u8 *pBlockBody = pBlockHead + sizeof(BinaryBlockHeader);
        pTempTail = pBlockHead + pContext->blockHeader.size;

        std::memcpy(pBlockHead, &pContext->blockHeader,
                    sizeof(BinaryBlockHeader));
        pReader->Read(pBlockBody, bodySize);

        if (!IsValidResource(pTempHead, infoSize)) {
            return RESULT_ERROR;
        }
    }

    u32 rest = pContext->GetBufferRestSize();
    int numLoaded = 0;

    GlyphGroupInfo info(pTempHead);

    u16 numSheet = info.GetNumSheet();
    u16 glyphsPerSheet = info.GetGlyphsPerSheet();
    u32 numBlock = info.GetNumBlock();
    u32 sheetSize = info.GetUncompSheetSize();

    u32 adjustSize = RoundUp(numSheet * sizeof(u16), 4);
    u16 *pTempAdjust = reinterpret_cast<u16 *>(
        RoundDown(GetIntPtr(pTempHead + rest - adjustSize), 2));

    if (rest < (pTempTail - pTempHead) + adjustSize) {
        return RESULT_ERROR;
    }

    for (int sheet = 0; sheet < numSheet; sheet++) {
        pTempAdjust[sheet] = 0;
    }

    for (int set = 0; set < info.GetNumSet(); set++) {
        const char *pName = info.GetSetName(set);

        if (IsNullString(pContext->pGlyphGroups) ||
            IncludeName(pContext->pGlyphGroups, pName)) {

            for (int sheet = 0; sheet < numSheet; sheet++) {
                if (info.IsSheetUsed(set, sheet)) {
                    pTempAdjust[sheet] = 1;
                }
            }
        }
    }

    u16 adjust = 0;
    for (int sheet = 0; sheet < numSheet; sheet++) {
        u16 &rEntry = pTempAdjust[sheet];

        if (rEntry == 1) {
            rEntry = adjust;
            numLoaded++;
        } else {
            rEntry = ADJUST_SHEET_NOT_LOADED;
            adjust += info.GetGlyphsPerSheet();
        }
    }

    u32 dataSize = GetRequireDataSize(&info, pContext->pGlyphGroups, NULL);
    u32 cacheSize = RoundDown(rest - dataSize, 4);

    if (dataSize >= rest) {
        return RESULT_ERROR;
    }

    u16 *pAdjust =
        InitSheetCache(pTempHead, cacheSize, numSheet, numLoaded, sheetSize);
    if (pAdjust == NULL) {
        return RESULT_ERROR;
    }

    u16 *pTempAdjustEnd = pTempAdjust + numSheet;
    std::memmove(pAdjust, pTempAdjust,
                 (pTempAdjustEnd - pTempAdjust) * sizeof(u16));

    pContext->SetGlyphGroups(pAdjust, numSheet, glyphsPerSheet, numBlock);
    pContext->AdvanceBuffer(cacheSize);
    pContext->op = ConstructContext::OP_DISPATCH;
    return RESULT_CONTINUE;
}

ArchiveFont::Result
ArchiveFont::ConstructOpPrepairExpandSheet(ConstructContext *pContext,
                                           CachedStreamReader *pReader) {
    if (!pContext->IsSheetRemaining()) {
        pContext->op = ConstructContext::OP_DISPATCH;
        return RESULT_CONTINUE;
    }

    u32 compSize;
    u32 requireSize = sizeof(compSize);

    if (pReader->GetRestSize() < requireSize) {
        return RequestMoreData(pContext, pReader, requireSize);
    }

    pReader->Read(&compSize, sizeof(compSize));
    const u32 srcSize = compSize;

    if (pContext->IsSheetLoaded()) {
        if (pContext->GetBufferRestSize() < srcSize) {
            return RESULT_ERROR;
        }

        pContext->StartCopy(srcSize, ConstructContext::OP_EXPAND_SHEET);
    } else {
        pContext->StartSkip(srcSize, ConstructContext::OP_EXPAND_SHEET);
    }

    pContext->NextSheet();
    return RESULT_CONTINUE;
}

} // namespace ut
} // namespace nw4r
