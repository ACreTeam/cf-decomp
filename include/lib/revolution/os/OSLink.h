

#ifndef REVOLUTION_OS_LINK_H
#define REVOLUTION_OS_LINK_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef u32 OSModuleID;
typedef struct OSModuleInfo OSModuleInfo;

typedef struct OSModuleQueue {
    OSModuleInfo* head;
    OSModuleInfo* tail;
} OSModuleQueue;

typedef struct OSModuleLink {
    OSModuleInfo* next;
    OSModuleInfo* prev;
} OSModuleLink;

struct OSModuleInfo {
    OSModuleID id;      // 0x00
    OSModuleLink link;  // 0x04

    u32 numSections;        // 0x0C
    u32 sectionInfoOffset;  // 0x10

    u32 nameOffset;  // 0x14
    u32 nameSize;    // 0x18

    u32 version;  // 0x1C
};

#define OS_MODULE_VERSION 3

typedef struct OSModuleHeader {
    OSModuleInfo info;  // 0x00

    // OS_MODULE_VERSION == 1
    u32 bssSize;           // 0x20
    u32 relOffset;         // 0x24
    u32 impOffset;         // 0x28
    u32 impSize;           // 0x2C
    u8 prologSection;      // 0x30
    u8 epilogSection;      // 0x31
    u8 unresolvedSection;  // 0x32
    u8 bssSection;         // 0x33
    u32 prolog;            // 0x34
    u32 epilog;            // 0x38
    u32 unresolved;        // 0x3C

    // OS_MODULE_VERSION == 2
    u32 align;     // 0x40
    u32 bssAlign;  // 0x44

    // OS_MODULE_VERSION == 3
    u32 fixSize;  // 0x48
} OSModuleHeader;

typedef struct OSSectionInfo {
    u32 offset;  // 0x00
    u32 size;    // 0x04
} OSSectionInfo;

#define OSGetSectionInfo(module) ((OSSectionInfo*)(((OSModuleInfo*)(module))->sectionInfoOffset))

#define OS_SECTIONINFO_EXEC 0x1
#define OS_SECTIONINFO_OFFSET(offset) ((offset) & ~0x1)

typedef struct OSImportInfo {
    OSModuleID id;  // 0x00
    u32 offset;     // 0x04
} OSImportInfo;

typedef struct OSRel {
    u16 offset;  // 0x00
    u8 type;     // 0x02
    u8 section;  // 0x03
    u32 addend;  // 0x04
} OSRel;

#define R_DOLPHIN_NOP 201
#define R_DOLPHIN_SECTION 202
#define R_DOLPHIN_END 203
#define R_DOLPHIN_MRKREF 204

void OSNotifyLink(OSModuleInfo* module);
void OSNotifyUnlink(OSModuleInfo* module);
void OSNotifyPreLink(OSModuleHeader* newModule, OSModuleHeader* module);
void OSNotifyPostLink(OSModuleHeader* newModule, OSModuleHeader* module);
void OSSetStringTable(void* stringTable);
BOOL OSLink(OSModuleInfo* newModule, void* bss);
BOOL OSLinkFixed(OSModuleInfo* newModule, void* bss);
BOOL OSUnlink(OSModuleInfo* oldModule);

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_OS_LINK_H
