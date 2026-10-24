#ifndef NW4R_SND_SEQ_FILE_H
#define NW4R_SND_SEQ_FILE_H

#include <types.h>

#include <nw4r/snd/snd_Util.h>

#include <nw4r/ut/ut_binaryFileFormat.h>

namespace nw4r {
namespace snd {
namespace detail {
struct SeqFile {
    struct Header {
        ut::BinaryFileHeader fileHeader; // size 0x10, offset 0x00
        u32 dataBlockOffset;             // size 0x04, offset 0x10
        u32 dataBlockSize;               // size 0x04, offset 0x14
        u32 labelBlockOffset;            // size 0x04, offset 0x18
        u32 labelBlockSize;              // size 0x04, offset 0x1c
    }; // size 0x20

    struct LabelInfo {
        u32 offset;   // size 0x04, offset 0x00
        u32 nameLen;  // size 0x04, offset 0x04
        char name[1]; // size 0x01, offset 0x08
                      /* 3 bytes padding */
    }; // size 0x0c

    static u32 const SIGNATURE_LABEL_BLOCK;

    struct LabelBlock {
        ut::BinaryBlockHeader blockHeader; // size 0x08, offset 0x00
        Util::Table<u32> labelInfoTable;   // size 0x08, offset 0x08
    }; // size 0x10

    static u32 const SIGNATURE_DATA_BLOCK = NW4R_FOUR_BYTE('D', 'A', 'T', 'A');

    struct DataBlock {
        ut::BinaryBlockHeader blockHeader; // size 0x08, offset 0x00
        u32 baseOffset;                    // size 0x04, offset 0x08
    }; // size 0x0c

    static u32 const SIGNATURE_FILE = NW4R_FOUR_BYTE('R', 'S', 'E', 'Q');
    static int const FILE_VERSION = NW4R_FILE_VERSION(1, 2);
};
} // namespace detail
} // namespace snd
} // namespace nw4r

namespace nw4r {
namespace snd {
namespace detail {
class SeqFileReader {
  public:
    SeqFileReader(void const *seqData);

    void const *GetBaseAddress() const;

    bool IsValidFileHeader(void const *seqData);

    bool ReadOffsetByLabel(char const *labelName, u32 *offsetPtr) const;

  public:
    static u16 const SUPPORTED_FILE_VERSION = NW4R_FILE_VERSION(1, 0);

  private:
    SeqFile::Header const *mHeader;       // size 0x04, offset 0x00
    SeqFile::DataBlock const *mDataBlock; // size 0x04, offset 0x04
}; // size 0x08
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
