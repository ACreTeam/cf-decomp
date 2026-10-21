/******
GameSpy NAT Negotiation SDK - internal types

Layouts from the TFP II DWARF (RVLDWC RELEASE_2_1_3 natneg.c / NATify.c). Packet
structs are byte packed (InitPacket.localip at 0x3). The packet type, version
and retry constants below are not in the DWARF; they were taken from CF's code.
******/

#pragma once

#include "natneg.h"

#ifdef __cplusplus
extern "C" {
#endif

// Protocol version
#define NN_PROTVER 3

// Packet types
#define NN_INIT 0
#define NN_INITACK 1
#define NN_ERTTEST 2
#define NN_ERTACK 3
#define NN_STATEUPDATE 4
#define NN_CONNECT 5
#define NN_CONNECT_ACK 6
#define NN_CONNECT_PING 7
#define NN_BACKUP_TEST 8
#define NN_BACKUP_ACK 9
#define NN_ADDRESS_CHECK 10
#define NN_ADDRESS_REPLY 11
#define NN_NATIFY_REQUEST 12
#define NN_REPORT 13
#define NN_REPORT_ACK 14

// Port types (InitPacket.porttype)
#define NN_PT_GP 0
#define NN_PT_NN1 1
#define NN_PT_NN2 2
#define NN_PT_NN3 3

// ConnectPacket.finished error values
#define NN_CONNECT_DEADBEAT 1
#define NN_CONNECT_INITTIMEOUT 2

// Packet sizes (BASEPACKET_SIZE is the header up to the Packet union)
#define BASEPACKET_SIZE 12
#define INITPACKET_SIZE (BASEPACKET_SIZE + 9)
#define CONNECTPACKET_SIZE (BASEPACKET_SIZE + 8)
#define REPORTPACKET_SIZE (BASEPACKET_SIZE + 61)

// NAT negotiation server port
#define MATCHUP_PORT 27901

// Retry timing (milliseconds / counts)
#define INIT_RETRY_TIME 500
#define INIT_RETRY_COUNT 10
#define PING_RETRY_TIME 700
#define PING_RETRY_COUNT 7
#define REPORT_RETRY_TIME 1000
#define REPORT_RETRY_COUNT 5
#define INITACK_WAIT_TIME 60000
#define FINISH_WAIT_TIME 5000

#pragma pack(push, 1)

typedef struct _InitPacket {
    unsigned char porttype;    // 0x0
    unsigned char clientindex; // 0x1
    unsigned char usegameport; // 0x2
    unsigned int localip;      // 0x3
    unsigned short localport;  // 0x7
} InitPacket;                  // 0x9

typedef struct _ConnectPacket {
    unsigned int remoteIP;     // 0x0
    unsigned short remotePort; // 0x4
    unsigned char gotyourdata; // 0x6
    unsigned char finished;    // 0x7
} ConnectPacket;               // 0x8

typedef struct _ReportPacket {
    unsigned char porttype;            // 0x0
    unsigned char clientindex;         // 0x1
    unsigned char negResult;           // 0x2
    NatType natType;                   // 0x3
    NatMappingScheme natMappingScheme; // 0x7
    char gamename[50];                 // 0xB
} ReportPacket;                        // 0x3D

typedef struct _NatNegPacket {
    unsigned char magic[NATNEG_MAGIC_LEN]; // 0x0
    unsigned char version;                 // 0x6
    unsigned char packettype;              // 0x7
    int cookie;                            // 0x8
    union {
        InitPacket Init;
        ConnectPacket Connect;
        ReportPacket Report;
    } Packet;   // 0xC
} NatNegPacket; // 0x49

#pragma pack(pop)

typedef struct _NATNegotiator {
    SOCKET negotiateSock;                     // 0x00
    SOCKET gameSock;                          // 0x04
    int cookie;                               // 0x08
    int clientindex;                          // 0x0C
    NegotiateState state;                     // 0x10
    int initAckRecv[4];                       // 0x14
    int retryCount;                           // 0x24
    int maxRetryCount;                        // 0x28
    unsigned long retryTime;                  // 0x2C
    unsigned int guessedIP;                   // 0x30
    unsigned short guessedPort;               // 0x34
    unsigned char gotRemoteData;              // 0x36
    unsigned char sendGotRemoteData;          // 0x37
    NegotiateProgressFunc progressCallback;   // 0x38
    NegotiateCompletedFunc completedCallback; // 0x3C
    void *userdata;                           // 0x40
    NegotiateResult result;                   // 0x44
    SOCKET connectedSocket;                   // 0x48
    struct sockaddr_in remoteaddr;            // 0x4C
} NATNegotiator;                              // 0x54

#ifdef __cplusplus
}
#endif
