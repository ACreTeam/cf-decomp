#ifndef NW4R_SND_BANK_FILE_H
#define NW4R_SND_BANK_FILE_H

#include <types.h>

#include <nw4r/snd/snd_WaveFile.h>

#include <nw4r/snd/snd_Util.h>

#include <nw4r/ut/ut_binaryFileFormat.h>

namespace nw4r {
namespace snd {
namespace detail {
struct BankFile {
    struct Header {
        ut::BinaryFileHeader fileHeader; // size 0x10, offset 0x00
        u32 dataBlockOffset;             // size 0x04, offset 0x10
        u32 dataBlockSize;               // size 0x04, offset 0x14
        u32 waveBlockOffset;             // size 0x04, offset 0x18
        u32 waveBlockSize;               // size 0x04, offset 0x1c
    }; // size 0x20

    struct InstParam {
        union {
            s32 waveIndex;                                      // size 0x04
            WaveData const *waveInfoAddress;                    // size 0x04
            WaveDataLocationCallback *waveDataLocationCallback; // size 0x04
        }; // size 0x04, offset 0x00
        u8 attack;               // size 0x01, offset 0x04
        u8 decay;                // size 0x01, offset 0x05
        u8 sustain;              // size 0x01, offset 0x06
        u8 release;              // size 0x01, offset 0x07
        u8 hold;                 // size 0x01, offset 0x08
        u8 waveDataLocationType; // size 0x01, offset 0x09
        u8 noteOffType;          // size 0x01, offset 0x0a
        u8 alternateAssign;      // size 0x01, offset 0x0b
        u8 originalKey;          // size 0x01, offset 0x0c
        u8 volume;               // size 0x01, offset 0x0d
        u8 pan;                  // size 0x01, offset 0x0e
        u8 padding2;
        f32 tune; // size 0x04, offset 0x10

        Util::DataRef<void> lfoTableRef;        // size 0x08, offset 0x14
        Util::DataRef<void> graphEnvTablevRef;  // size 0x08, offset 0x1c
        Util::DataRef<void> randomizerTableRef; // size 0x08, offset 0x24
        u32 reserved;
    }; // size 0x30

    struct RangeTable {
        u8 tableSize; // size 0x01, offset 0x00
        u8 key[];
    }; // size 0x01

    struct IndexTable {
        u8 min; // size 0x01, offset 0x00
        u8 max; // size 0x01, offset 0x01
        u16 reserved;
        byte_t ref[];
    }; // size 0x04

    typedef Util::DataRef<void, InstParam, RangeTable, IndexTable> DataRegion;

    static u32 const SIGNATURE_DATA_BLOCK = NW4R_FOUR_BYTE('D', 'A', 'T', 'A');

    struct DataBlock {
        ut::BinaryBlockHeader blockHeader; // size 0x08, offset 0x00
        Util::Table<DataRegion> instTable; // size 0x0c, offset 0x08
    }; // size 0x14

    static u32 const SIGNATURE_WAVE_BLOCK = NW4R_FOUR_BYTE('W', 'A', 'V', 'E');
    typedef Util::DataRef<WaveFile::WaveInfo> WaveRegion;

    struct WaveBlock {
        ut::BinaryBlockHeader blockHeader;     // size 0x08, offset 0x00
        Util::Table<WaveRegion> waveInfoTable; // size 0x0c, offset 0x08
    }; // size 0x14

    static u32 const SIGNATURE_FILE = NW4R_FOUR_BYTE('R', 'B', 'N', 'K');
    static int const FILE_VERSION = NW4R_FILE_VERSION(1, 2);
};

struct InstInfo {
  public:
    enum NoteOffType {
        NOTE_OFF_TYPE_RELEASE,
        NOTE_OFF_TYPE_IGNORE,
    };

  public:
    struct WaveDataLocation {
      public:
        enum WaveDataLocationType {
            WAVE_DATA_LOCATION_INDEX,
            WAVE_DATA_LOCATION_ADDRESS,
            WAVE_DATA_LOCATION_CALLBACK,
        };

      public:
        WaveDataLocationType type; // size 0x04, offset 0x00

        union {
            s32 index;                          // size 0x04
            WaveData const *address;            // size 0x04
            WaveDataLocationCallback *callback; // size 0x04
        }; // size 0x04, offset 0x04
    }; // size 0x08

  public:
    WaveDataLocation waveDataLocation; // size 0x08, offset 0x00
    u8 attack;                         // size 0x01, offset 0x08
    u8 hold;                           // size 0x01, offset 0x09
    u8 decay;                          // size 0x01, offset 0x0a
    u8 sustain;                        // size 0x01, offset 0x0b
    u8 release;                        // size 0x01, offset 0x0c
    /* 3 bytes padding */
    NoteOffType noteOffType; // size 0x04, offset 0x10
    u8 alternateAssign;      // size 0x01, offset 0x14
    u8 originalKey;          // size 0x01, offset 0x15
    u8 pan;                  // size 0x01, offset 0x16
    u8 volume;               // size 0x01, offset 0x17
    f32 tune;                // size 0x04, offset 0x18
}; // size 0x1c
} // namespace detail
} // namespace snd
} // namespace nw4r

namespace nw4r {
namespace snd {
namespace detail {
class BankFileReader {
  public:
    BankFileReader(void const *bankData);

    bool ReadInstInfo(InstInfo *instInfo, int prgNo, int key,
                      int velocity) const;
    bool ReadWaveInfo(WaveData *waveParam,
                      InstInfo::WaveDataLocation const &waveDataLocation,
                      void const *waveDataAddress,
                      WaveData const **waveInfoAddress) const;

  private:
    BankFile::InstParam const *GetInstParam(int prgNo, int key,
                                            int velocity) const;
    BankFile::DataRegion const *
    GetReferenceToSubRegion(BankFile::DataRegion const *ref,
                            int splitKey) const;

    static bool IsValidFileHeader(void const *bankData);

  public:
    static int const SUPPORTED_FILE_VERSION = NW4R_FILE_VERSION(1, 2);

  private:
    BankFile::Header const *mHeader;       // size 0x04, offset 0x00
    BankFile::DataBlock const *mDataBlock; // size 0x04, offset 0x04
    BankFile::WaveBlock const *mWaveBlock; // size 0x04, offset 0x08
}; // size 0x0c
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
