#pragma once

#include <types.h>
#include <revolution/arc.h>
#include <nw4r/g3d/res/g3d_resfile.h>
#include <game/mLib/m_dvd.hpp>

// Archive/resource loaders (d_dvd, src/dol/game/d_dvd.cpp, .text 80085E8C..80086558). Class names from RTTI;
// member names are inferred. MWCC places the vtable pointer where the first virtual is declared, which is
// after mStatus here.
namespace dDvd {

class loader_c {
protected:
    s32 mStatus; // 0x00 (declared before the first virtual)

public:
    loader_c(); // 80085E8C
    virtual ~loader_c(); // 80085EB4
    virtual void freeData(); // 80086000

    void *request(const char *path, u8 mountDirection, void *heap); // 80085EF4: the data once loaded, else NULL
    s32 getSize() const; // 8008611C
    void *getData() const { return mpData; }
    s32 getStatus() const { return mStatus; } // the file size once loaded
    BOOL release(); // 80085FAC: finishes the command, forgets data and heap, status -1; returns TRUE
    BOOL cancelRequest(); // 8008605C: FALSE while the command runs; frees what it loaded

protected:
    mDvd_toMainRam_c *mpCommand; // 0x08
    void *mpHeap; // 0x0C
    void *mpData; // 0x10
}; // sizeof = 0x14

class bank_c : public loader_c {
public:
    bank_c(); // 800860D0
    virtual ~bank_c() {} // 8006546C
    virtual void onLoaded(); // 80086554

    BOOL load(const char *path, void *heap, u8 entry); // 80086134
    BOOL unload(BOOL wait); // 8008620C
    BOOL checkLoaded(); // 800862C8: retries a pending load (TRUE once loaded)

protected:
    u8 mLoading; // 0x14
    u8 mEntry; // 0x15
    char mPath[0x40]; // 0x16
}; // sizeof = 0x58 (aligned)

// brresBank_c comes before arcBank_c: MWCC emits weak vtables/RTTI in reverse class-definition
// order, and d_item.o has arcBank_c's RTTI before brresBank_c's.
class brresBank_c : public bank_c {
public:
    // Implicit virtual destructor; its weak copy belongs to the generated-method section.
    virtual void onLoaded() {
        nw4r::g3d::ResFile res(mpData);
        res.Init();
        res.Release();
        res.Bind(res);
    }
}; // sizeof = 0x58

class arcBank_c : public bank_c {
public:
    arcBank_c() : mArcReady(0) {}
    // Inline: d_bg's onLoaded overrides inline it; the kept weak copy is 80065430 (in d_banner).
    virtual void onLoaded() {
        ARCInitHandle(mpData, &mHandle);
        mArcReady = 1;
    }
    virtual u32 getFileSize(const char *name); // 80086398: getArcFileSize(name, mpData)
    virtual BOOL copyFile(void *dst, const char *name); // 80086474
    virtual BOOL bindFile(void *dst, const char *name); // 800864D8: copyFile, then binds it as a .brres

    s32 getArcFileSize(const char *name, void *arc); // 80086330: -1 without data, -2 if not found
    void *getFile(const char *name, u32 *size); // 800863A0
    BOOL closeArc(); // 80086544: mArcReady = 0, then unload(FALSE)

protected:
    u8 mArcReady; // 0x56
    ARCHandle mHandle; // 0x58
}; // sizeof = 0x74

} // namespace dDvd
