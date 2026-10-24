#ifndef NW4R_SND_STRM_CHANNEL_H
#define NW4R_SND_STRM_CHANNEL_H

#include <climits>

#include <types.h>

#include <nw4r/snd/snd_adpcm.h>

namespace nw4r {
namespace snd {
namespace detail {
struct StrmChannel {
    void *bufferAddress;           // size 0x04, offset 0x00
    AdpcmParam adpcmParam;         // size 0x28, offset 0x04
    AdpcmLoopParam adpcmLoopParam; // size 0x06, offset 0x2c
    u16 adpcmPredScale;            // size 0x02, offset 0x32
}; // size 0x34
} // namespace detail
} // namespace snd
} // namespace nw4r

namespace nw4r {
namespace snd {
namespace detail {
class StrmBufferPool {
  public:
    void Setup(void *buffer, u32 size, int blockCount);
    void Shutdown();

    u32 GetBlockSize() const {
        return mBlockSize;
    }

    void *Alloc();
    void Free(void *p);

  private:
    static int const BIT_PER_BYTE = CHAR_BIT;
    static int const BLOCK_MAX = 32;

  private:
    void *mBuffer;   // size 0x04, offset 0x00
    u32 mBufferSize; // size 0x04, offset 0x04
    u32 mBlockSize;  // size 0x04, offset 0x08
    int mBlockCount; // size 0x04, offset 0x0c
    int mAllocCount; // size 0x04, offset 0x10
    byte_t mAllocFlags[ROUND_UP(BLOCK_MAX, BIT_PER_BYTE) /
                       BIT_PER_BYTE]; // size 0x04, offset 0x14
}; // size 0x18
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
