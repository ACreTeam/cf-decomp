#pragma once

#include <types.h>

namespace EGG {

class DvdFile {
public:
    DvdFile();
    virtual ~DvdFile();

    virtual bool openFilename(const char *filename);
    virtual void close();
    virtual s32 readData(void *buffer, long size, long offset); // Bytes read, or a negative error.
    virtual void writeData(const void *, long, long);
    virtual size_t getFileSize() const { return mFileSize; }
    virtual void open(const char *);
    virtual void open(const char *, void *);

    u8 mPad1[0x6c];
    size_t mFileSize;
    u8 mPad2[0x5c];

    static void initialize();
};

} // namespace EGG
