#ifndef NW4R_SND_BANK_H
#define NW4R_SND_BANK_H

#include <nw4r/snd/snd_BankFile.h>

#include <nw4r/NW4RAssert.hpp>

namespace nw4r {
namespace snd {
namespace detail {
class Channel;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
namespace detail {
struct NoteOnInfo;
}
} // namespace snd
} // namespace nw4r

namespace nw4r {
namespace snd {
namespace detail {
class Bank {
  public:
    Bank(void const *bankData);
    ~Bank();

    void SetWaveDataAddress(void const *waveData) {
        NW4RAssertPointerNonnull_Line(50, waveData);

        mWaveDataAddress = waveData;
    }

    Channel *NoteOn(NoteOnInfo const &noteOnInfo) const;

  private:
    BankFileReader mBankReader;   // size 0x0c, offset 0x00
    void const *mWaveDataAddress; // size 0x04, offset 0x0c
}; // size 0x10
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
