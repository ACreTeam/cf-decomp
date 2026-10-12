#pragma once

// d_demo_sync (DOL TU d_demo_sync.cpp, .text 8008205C..8008278C, .bss 805839D8..80583A08, .sdata 80749E18): keeps
// demos (dDemoActor_c events, by demo ID) in step between the consoles of an online session over
// NET_PACKET_DEMO_SYNC (0x2C). The host holds one demo slot per member (no two members may run the same demo);
// guests ask the host for a slot (request), and the scene owner runs the demo actor's start check / commands.
// Packets: [0] command (dDemoSyncCmd_e), [1..4] demo ID (little endian), [5] argument. No RTTI; the namespace
// comes from the file name. All names are inferred.

#include <types.h>

// Packet commands; dDemoSync::getState() returns the last one received or sent, or DEMO_SYNC_IDLE.
enum dDemoSyncCmd_e {
    DEMO_SYNC_REQUEST, // guest -> host: reserve a demo slot
    DEMO_SYNC_ACCEPT,  // answer: accepted
    DEMO_SYNC_REJECT,  // answer: rejected
    DEMO_SYNC_RELEASE, // guest -> host: free the slot
    DEMO_SYNC_START,   // -> scene owner: run the demo actor's start check (argument: the dDemoCmd_e talk kind)
    DEMO_SYNC_COMMAND, // -> scene owner: pass a command (dDemoCmd_e) to the demo actor (dDemoActor_c::onDemoCmd)
    DEMO_SYNC_IDLE,
};

// dDemoSync::checkRequest results.
enum dDemoSyncCheck_e {
    DEMO_SYNC_CHECK_BUSY,    // host: our slot is taken or another member runs the demo
    DEMO_SYNC_CHECK_REQUEST, // guest: ask the host (request)
    DEMO_SYNC_CHECK_OK,      // offline, or host with a free slot
};

#define DEMO_SYNC_MEMBER_NUM 4
#define DEMO_SYNC_PACKET_SIZE 6
#define DEMO_SYNC_MEMBER_SELF 4 // onDemoCmd member argument: this console

namespace dDemoSync {

// A command packet kept until its demo actor exists (DEMO_SYNC_COMMAND for an actor not created yet).
class Pending_c {
public:
    void clear();            // 80082670
    void set(const u8 *data); // 8008267C
    BOOL get(u8 *out) const; // 8008268C: FALSE when empty

    /* 0x0 */ u8 mData[DEMO_SYNC_PACKET_SIZE];
    /* 0x6 */ u8 mValid;
}; // size 0x7

void init();                                        // 8008205C: d_net session start / end
int checkRequest(u32 demoId);                       // 800820DC: dDemoSyncCheck_e
void request(u32 demoId);                           // 80082148
void release();                                     // 800821D8
u8 getState();                                      // 80082224: dDemoSyncCmd_e
BOOL canStart(u8 member, u32 demoId);               // 8008222C: host: the member's slot is free, no slot runs demoId
void releaseSlot(u8 member);                        // 800822A8
BOOL isRemote();                                    // 800822C0: online and not the scene owner
void sendStart(u32 demoId, u8 check);               // 800822FC: check: dDemoCmd_e talk kind
void sendCommand(u32 demoId, u8 cmd);               // 80082304
void send(u32 demoId, u8 arg, u8 cmd);              // 8008230C
void receive(int member, const u8 *data, u32 size); // 8008239C: NET_PACKET_DEMO_SYNC
u32 readDemoId(const u8 *data);                     // 80082600
void sendCmd(u8 cmd, int member);                   // 80082624: a 1-byte packet
void reply(u8 cmd, int member);                     // 8008266C
void storePending(int member, const u8 *data);      // 800826D4
void update();                                      // 800826EC: dDemo_c::execute: forwards / runs the pending commands

} // namespace dDemoSync
