#pragma once

#include <game/mLib/m_dvd.hpp>
#include <lib/egg/core/eggExpHeap.h>
#include <lib/egg/core/eggDvdFile.h>
#include <lib/revolution/os/OSLink.h>

class DynamicModuleControlBase {
protected:
    /* 0x00 */ u16 mUsageCount;
    /* 0x02 */ u16 mLinkCount; // Saturating count of successful links, not active users.
    /* 0x04 */ DynamicModuleControlBase *mpPrev;
    /* 0x08 */ DynamicModuleControlBase *mpNext;

public:
    DynamicModuleControlBase(); // 802AA1D8
    virtual ~DynamicModuleControlBase(); // 802AA11C

    virtual const char *getModuleName() const { return nullptr; } // 80087054
    virtual size_t getModuleSize() const { return 0; } // 802AACE4
    virtual const char *getModuleTypeString() const { return "Base"; } // 802AACF8
    virtual void dumpAll(); // 802AA3A8: visits every used module in the list.
    virtual void dump() {} // 802AACEC
    virtual bool do_load() { return true; } // 802AACBC
    virtual bool do_load_async() { return true; } // 802AACDC
    virtual bool do_unload() { return true; } // 802AACD4
    virtual bool do_link() { return true; } // 802AACC4
    virtual bool do_unlink() { return true; } // 802AACCC

    bool link(); // 802AA220
    bool unlink(); // 802AA2B4
    bool load_async(); // 802AA320
    bool forceUnlink(); // 802AA344

    bool isLinked() const { return mUsageCount != 0; }

    static DynamicModuleControlBase *mFirst;
    static DynamicModuleControlBase *mLast;
}; // size 0x10, vptr at 0x0C

class DynamicModuleControl : public DynamicModuleControlBase {
public:
    enum ModuleType_e {
        MODULE_TYPE_UNKNOWN,
        MODULE_TYPE_MEM,
        MODULE_TYPE_ARAM,
        MODULE_TYPE_DVD,
    };

    DynamicModuleControl(const char *name, EGG::ExpHeap *heap); // 802AA434
    virtual ~DynamicModuleControl(); // 802AA4B4

    virtual const char *getModuleName() const override { return mModuleName; } // 802AACF0
    virtual size_t getModuleSize() const override; // 802AAB64
    virtual const char *getModuleTypeString() const override; // 802AABC0
    virtual void dump() override; // 802AA88C
    virtual bool do_load() override; // 802AA58C
    virtual bool do_load_async() override; // 802AA7AC
    virtual bool do_unload() override; // 802AA838
    virtual bool do_link() override; // 802AA890
    virtual bool do_unlink() override; // 802AAAC8

    static void initialize(EGG::ExpHeap *heap); // 802AA50C
    static void *callback(void *self); // 802AA554

    /* 0x10 */ OSModuleHeader *mpModule;
    /* 0x14 */ void *mpBss;
    /* 0x18 */ u32 mPrologReturn;
    /* 0x1C */ const char *mModuleName;
    /* 0x20 */ u8 mType; // Low two bits: ModuleType_e; bit 3: direct DVD read.
    /* 0x21 */ u8 mLoadCount;
    /* 0x22 */ u16 mChecksum;
    /* 0x24 */ int mModuleSize; // Size of data to checksum; zero for DVD loads.
    /* 0x28 */ mDvd_callback_c *mpDvdCallback;
    /* 0x2C */ EGG::ExpHeap *mpHeap;
    /* 0x30 */ u8 _30[0x14]; // Reserved; debug map-file storage in related games.

    static EGG::ExpHeap *sDylinkHeap;
    static EGG::DvdFile *sDvdFile;
    static size_t sAllocBytes;
    static const char *sModulesDir;
}; // size 0x44
