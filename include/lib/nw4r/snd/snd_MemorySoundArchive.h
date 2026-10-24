#ifndef NW4R_SND_MEMORY_SOUND_ARCHIVE_H
#define NW4R_SND_MEMORY_SOUND_ARCHIVE_H

#include <types.h>

// clang-format off
#include <nw4r/ut/ut_FileStream.h>
#include <nw4r/snd/snd_SoundArchive.h>
// clang-format on

#include <nw4r/snd/snd_SoundArchiveFile.h>

namespace nw4r {
namespace snd {
class MemorySoundArchive : public SoundArchive {
  private:
    class MemoryFileStream;

  public:
    MemorySoundArchive();
    virtual ~MemorySoundArchive();

    virtual void const *detail_GetFileAddress(u32 fileId) const;
    virtual void const *detail_GetWaveDataFileAddress(u32 fileId) const;
    virtual int detail_GetRequiredStreamBufferSize() const;
    virtual ut::FileStream *OpenStream(void *buffer, int size, u32 begin,
                                       u32 length) const;
    virtual ut::FileStream *OpenExtStream(void *buffer, int size,
                                          char const *extFilePath, u32 begin,
                                          u32 length) const;

    bool Setup(void const *soundArchiveData);
    void Shutdown();

  private:
    /* base SoundArchive */                     // size 0x108, offset 0x000
    void const *mData;                          // size 0x004, offset 0x108
    detail::SoundArchiveFileReader mFileReader; // size 0x044, offset 0x10c
}; // size 0x150

class MemorySoundArchive::MemoryFileStream : public ut::FileStream {
  public:
    MemoryFileStream(void const *buffer, u32 size);
    virtual ~MemoryFileStream() {
    }

    virtual void Close();
    virtual s32 Read(void *buf, u32 length);

    virtual void Seek(s32 offset, u32 origin);

    virtual bool CanSeek() const {
        return true;
    }
    virtual bool CanCancel() const {
        return true;
    }
    virtual bool CanAsync() const {
        return false;
    }
    virtual bool CanRead() const {
        return true;
    }
    virtual bool CanWrite() const {
        return false;
    }
    virtual u32 Tell() const {
        return mPosition;
    }
    virtual u32 GetSize() const {
        return mSize;
    }

  private:
    /* base ut::FileStream */ // size 0x14, offset 0x00
    void const *mBuffer;      // size 0x04, offset 0x14
    u32 mSize;                // size 0x04, offset 0x18
    u32 mPosition;            // size 0x04, offset 0x1c
}; // size 0x20
} // namespace snd
} // namespace nw4r

#endif
