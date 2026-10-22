#pragma once

#include <types.h>

namespace EGG {

// Common interface of objects that can be saved to / loaded from a binary blob (name from Skyward Sword's
// EGG; CF's older version has no virtual destructor). T needs a BinData type describing its payload.
template <class T>
class IBinary {
public:
#pragma pack(push, 1)
    struct BinHeader {
        /* 0x00 */ char mType[4];
        /* 0x04 */ u32 mSize;
        /* 0x08 */ u8 mVersion;
        /* 0x09 */ u8 _09[7];
    };
    struct Bin {
        BinHeader mHeader;
        typename T::BinData mData;
    };
#pragma pack(pop)

protected:
    virtual void SetBinaryInner(const Bin &) = 0;                                 // 0x08
    virtual void GetBinaryInner(Bin *) const = 0;                                 // 0x0C
    virtual u32 GetBinarySize() const { return sizeof(Bin); }                     // 0x10
    virtual void SetBinaryInner(const Bin &, const Bin &, f32) {}                 // 0x14

    // Specialised per type: a 4 character ID and the current version.
    static const char *GetBinaryType();
    static int GetVersion();

public:
    void SetBinary(const void *);
    void GetBinary(void *pData) const;
    void SetBinaryBlend(const void *a, const void *b, f32 blend);
};

template <typename T>
void IBinary<T>::SetBinary(const void *a) {
    const Bin *pBinA = reinterpret_cast<const Bin *>(a);
    SetBinaryInner(*pBinA);
}

template <typename T>
void IBinary<T>::GetBinary(void *pData) const {
    Bin *pBin = reinterpret_cast<Bin *>(pData);
    BinHeader zeroedHeader = {0};
    pBin->mHeader = zeroedHeader;

    pBin->mHeader.mVersion = GetVersion();
    pBin->mHeader.mSize = GetBinarySize();
    for (int i = 0; i < 4; i++) {
        pBin->mHeader.mType[i] = GetBinaryType()[i];
    }
    typename T::BinData zeroedData = {0};
    pBin->mData = zeroedData;
    GetBinaryInner(pBin);
}

template <typename T>
void IBinary<T>::SetBinaryBlend(const void *a, const void *b, f32 blend) {
    const Bin *pBinA = reinterpret_cast<const Bin *>(a);
    const Bin *pBinB = reinterpret_cast<const Bin *>(b);
    SetBinaryInner(*pBinA, *pBinB, blend);
}

} // namespace EGG
