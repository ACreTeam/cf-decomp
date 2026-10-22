#include "revolution/hbm/nw4hbm/ut/binaryFileFormat.h"

#include "revolution/hbm/nw4hbm/db.h"
#include "revolution/hbm/nw4hbm/ut/inlines.h"

namespace nw4hbm {
namespace ut {

bool IsValidBinaryFile(const BinaryFileHeader* header, byte4_t signature, u16 version, u16 minBlocks) {
    NW4HBMAssertPointerValid_Line(header, 48);

    if (header->signature != signature) {
        s8 signature1 = (header->signature >> 24) & 0xFF;
        s8 signature2 = (header->signature >> 16) & 0xFF;
        s8 signature3 = (header->signature >> 8) & 0xFF;
        s8 signature4 = (header->signature & 0xFF);

        s8 signature5 = (signature >> 24) & 0xFF;
        s8 signature6 = (signature >> 16) & 0xFF;
        s8 signature7 = (signature >> 8) & 0xFF;
        s8 signature8 = (signature & 0xFF);

        NW4HBMWarningMessage_Line(60, "Signature check failed ('%c%c%c%c' must be '%c%c%c%c').", signature1, signature2,
                                  signature3, signature4, signature5, signature6, signature7, signature8);
        return false;
    }

    // U+FEFF * BYTE ORDER MARK
    if (header->byteOrder != 0xFEFF) {
        NW4HBMWarningMessage_Line(65, "Unsupported byte order.");
        return false;
    }

    if (header->version != version) {
        NW4HBMWarningMessage_Line(75, "Version check faild ('%d.%d' must be '%d.%d').", (header->version >> 8) & 0xFF,
                                  header->version & 0xFF, (version >> 8) & 0xFF, version & 0xFF);
        return false;
    }

    if (header->fileSize < sizeof(*header) + sizeof(BinaryBlockHeader) * minBlocks) {
        NW4HBMWarningMessage_Line(80, "Too small file size(=%d).", header->fileSize);
        return false;
    }

    if (header->dataBlocks < minBlocks) {
        NW4HBMWarningMessage_Line(85, "Too small number of data blocks(=%d).", header->dataBlocks);
        return false;
    }

    return true;
}

bool IsReverseEndianBinaryFile(const BinaryFileHeader* fileHeader) {
    NW4HBMAssertPointerValid(fileHeader);

    // U+FEFF * BYTE ORDER MARK
    return fileHeader->byteOrder != 0xFEFF;
}

BinaryBlockHeader* GetNextBinaryBlockHeader(BinaryFileHeader* fileHeader, BinaryBlockHeader* blockHeader) {
    NW4HBMAssertPointerValid(fileHeader);

    void* ptr;

    if (!IsReverseEndianBinaryFile(fileHeader)) {
        if (blockHeader == NULL) {
            if (fileHeader->dataBlocks == 0) {
                return NULL;
            }

            ptr = AddOffsetToPtr(fileHeader, fileHeader->headerSize);
        } else {
            ptr = AddOffsetToPtr(blockHeader, blockHeader->size);
        }

        if (ptr >= AddOffsetToPtr(fileHeader, fileHeader->fileSize)) {
            return NULL;
        }
    } else {
        if (blockHeader == NULL) {
            if (fileHeader->dataBlocks == 0) {
                return NULL;
            }

            ptr = AddOffsetToPtr(fileHeader, ReverseEndian(fileHeader->headerSize));
        } else {
            ptr = AddOffsetToPtr(blockHeader, ReverseEndian(blockHeader->size));
        }

        if (ptr >= AddOffsetToPtr(fileHeader, ReverseEndian(fileHeader->fileSize))) {
            return NULL;
        }
    }

    return static_cast<BinaryBlockHeader*>(ptr);
}

} // namespace ut
} // namespace nw4hbm
