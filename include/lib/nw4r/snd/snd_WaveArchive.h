#ifndef NW4R_SND_WAVE_ARCHIVE_H
#define NW4R_SND_WAVE_ARCHIVE_H

#include <types.h>

#include <nw4r/snd/snd_Util.h>
#include <nw4r/snd/snd_WaveFile.h>

#include <nw4r/ut/ut_binaryFileFormat.h>

namespace nw4r {
namespace snd {
namespace detail {
struct WaveArchive {
    struct FileHeader {
        ut::BinaryFileHeader fileHeader; // size 0x10, offset 0x00
        u32 tableChunkOffset;            // size 0x04, offset 0x10
        u32 tableChunkSize;              // size 0x04, offset 0x14
        u32 dataChunkOffset;             // size 0x04, offset 0x18
        u32 dataChunkSize;               // size 0x04, offset 0x1c
    }; // size 0x20

    struct TableItem {
        Util::DataRef<WaveFile::FileHeader>
            waveFileRef;  // size 0x08, offset 0x00
        u32 waveFileSize; // size 0x04, offset 0x08
    }; // size 0x0c

    static u32 const SIGNATURE_TABLE_BLOCK = NW4R_FOUR_BYTE('T', 'A', 'B', 'L');

    struct TableBlock {
        ut::BinaryBlockHeader blockHeader; // size 0x08, offset 0x00
        Util::Table<TableItem> fileTable;  // size 0x10, offset 0x08
    }; // size 0x18

    static u32 const SIGNATURE_DATA_BLOCK = NW4R_FOUR_BYTE('D', 'A', 'T', 'A');

    struct DataBlock {
        ut::BinaryBlockHeader blockHeader; // size 0x08, offset 0x00
        byte_t data[1];                    // size 0x01, offset 0x08
                                           /* 3 bytes padding */
    }; // size 0x0c

    static u32 const SIGNATURE_FILE = NW4R_FOUR_BYTE('R', 'W', 'A', 'R');
    static int const FILE_VERSION = NW4R_FILE_VERSION(1, 0);
};
} // namespace detail
} // namespace snd
} // namespace nw4r

namespace nw4r {
namespace snd {
namespace detail {
class WaveArchiveReader {
  public:
    WaveArchiveReader(void const *waveArc);

    WaveFile::FileHeader const *GetWaveFile(int index) const;

  private:
    bool VerifyFileHeader(void const *waveArc);

  public:
    static u16 const SUPPORTED_FILE_VERSION_MAX = NW4R_FILE_VERSION(1, 0);
    static u16 const SUPPORTED_FILE_VERSION_MIN = NW4R_FILE_VERSION(1, 0);

  private:
    WaveArchive::TableBlock const *mTableBlock; // size 0x04, offset 0x00
    WaveArchive::DataBlock const *mDataBlock;   // size 0x04, offset 0x04
}; // size 0x08
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
