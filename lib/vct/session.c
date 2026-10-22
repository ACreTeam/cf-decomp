#include <dwc/dwc_transport.h>
#include <revolution/os.h>
#include <string.h>

#include "nitro_compat.h"
#include "ssp.h"
#include "vct.h"
#include "vct_main.h"

typedef struct SSPHeader {
    u32 magic;
    u8 method;
    u8 version;
    u8 code;
    u8 reserved;
    u8 from;
    u8 to;
    u8 info;
    u8 talking;
    u32 aidBitmap;
} SSPHeader; // size 0x10

#define VCT_SSP_HEADER_VERSION 0x10

enum VCTMethod {
    VCT_METHOD_REQUEST = 0xFF,
    VCT_METHOD_RESPONSE = 0,
};

typedef enum {
    VCT_REQUEST_INVITE = 0,
    VCT_REQUEST_BYE,
    VCT_REQUEST_CANCEL,
    VCT_REQUEST_CONTACT,
    VCT_REQUEST_NOTIFY,
    VCT_REQUEST_END
} VCTRequestCode;

typedef enum {
    VCT_RESPONSE_OK,
    VCT_RESPONSE_BAD_REQUEST,
    VCT_RESPONSE_NOT_ACCEPTABLE,
    VCT_RESPONSE_BUSY_HERE,
    VCT_RESPONSE_TERMINATED,
    VCT_RESPONSE_DECLINE,
    VCT_RESPONSE_END
} VCTResponseCode;

typedef enum {
    VCT_NOTIFY_FREE = 0,
    VCT_NOTIFY_BUSY,
} VCTNotify;

typedef struct SessionInfo {
    VCTSession* free;
    VCTSession* use;
    MATHRandContext16 rand;
    VCTSession tmp;
    VCTSession trans;
} SessionInfo; // size 0x44

#define VCT_NO_CHANGE -1
#define VCT_STATE_ERROR -2
#define VCT_CONTACT_FAIL -3

// OS_SecondsToTicks(30) with the 60.75MHz timer
#define VCT_TRANSCEIVER_LIMIT_TICKS 1822500000LL

static SessionInfo sSession;
static u32 sNotifyAID;
static u32 sNumOfNotifyAID;
static BOOL sSkipSend;
static u64 sTransTalkTime;

static u64 sTransceiverLimit = VCT_TRANSCEIVER_LIMIT_TICKS;

static const s8 sSendState[6][11] = {
    {VCT_STATE_OUTGOING, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_INIT, VCT_STATE_INIT, VCT_STATE_INIT, VCT_STATE_ERROR, VCT_STATE_ERROR},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_DISCONNECTING, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_INIT, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR},
    {VCT_STATE_ERROR, VCT_STATE_DISCONNECTING, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_INIT, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_TALKING, VCT_STATE_INIT, VCT_STATE_INIT, VCT_STATE_INIT, VCT_STATE_ERROR, VCT_STATE_INIT},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_INIT, VCT_STATE_INIT, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_INIT, VCT_STATE_ERROR},
};

static const s8 sTransSendState[6][11] = {
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_OUTGOING, VCT_NO_CHANGE, VCT_STATE_ERROR, VCT_STATE_INIT, VCT_STATE_INIT, VCT_STATE_INIT, VCT_STATE_ERROR, VCT_STATE_ERROR},
    {VCT_STATE_ERROR, VCT_STATE_DISCONNECTING, VCT_STATE_DISCONNECTING, VCT_STATE_ERROR, VCT_NO_CHANGE, VCT_STATE_ERROR, VCT_STATE_INIT, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR},
    {VCT_STATE_ERROR, VCT_STATE_DISCONNECTING, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_NO_CHANGE, VCT_STATE_ERROR, VCT_STATE_INIT, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_NO_CHANGE, VCT_STATE_ERROR, VCT_STATE_INIT, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_NO_CHANGE, VCT_STATE_CONNECTED, VCT_STATE_INIT, VCT_STATE_INIT, VCT_STATE_INIT, VCT_STATE_ERROR, VCT_STATE_INIT},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_NO_CHANGE, VCT_STATE_INIT, VCT_STATE_INIT, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_INIT, VCT_STATE_ERROR},
};

static const s8 sRecvState[6][11] = {
    {VCT_STATE_INCOMING, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_NO_CHANGE, VCT_STATE_TALKING, VCT_STATE_INIT, VCT_STATE_INIT, VCT_STATE_INIT, VCT_NO_CHANGE, VCT_STATE_INIT},
    {VCT_STATE_ERROR, VCT_STATE_DISCONNECTING, VCT_STATE_DISCONNECTING, VCT_STATE_ERROR, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_DISCONNECTING, VCT_STATE_ERROR, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_NO_CHANGE, VCT_STATE_INIT, VCT_STATE_INIT, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_STATE_INIT, VCT_NO_CHANGE},
};

static const s8 sTransRecvState[6][11] = {
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_CONNECTED, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_STATE_INIT, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_CONTACT_FAIL, VCT_NO_CHANGE, VCT_STATE_TALKING, VCT_STATE_INIT, VCT_STATE_INIT, VCT_STATE_INIT, VCT_NO_CHANGE, VCT_STATE_INIT},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_CONTACT_FAIL, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE},
    {VCT_STATE_ERROR, VCT_STATE_INIT, VCT_STATE_ERROR, VCT_CONTACT_FAIL, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_CONTACT_FAIL, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_NO_CHANGE},
    {VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_STATE_ERROR, VCT_CONTACT_FAIL, VCT_NO_CHANGE, VCT_STATE_INIT, VCT_STATE_INIT, VCT_NO_CHANGE, VCT_NO_CHANGE, VCT_STATE_INIT, VCT_NO_CHANGE},
};

static const u8 sRequestEvent[] = {
    VCT_EVENT_INCOMING, VCT_EVENT_RESPONDBYE, VCT_EVENT_CANCEL, VCT_EVENT_CONTACT, VCT_EVENT_NONE,
};

static const u8 sResponseEvent[] = {
    VCT_EVENT_CONNECTED, VCT_EVENT_ABORT, VCT_EVENT_REJECT, VCT_EVENT_BUSY, VCT_EVENT_DISCONNECTED, VCT_EVENT_REJECT,
};

BOOL vct_response_reject(SSPHeader* header, VCTResult* result);
static VCTSession* vct_create_session_impl(u8 aid);
static BOOL vct_handle_ssp_client(SSPHeader* header, VCTResult* result);
static BOOL vct_handle_ssp_server(SSPHeader* header, VCTResult* result);
static BOOL vct_handle_notify(SSPHeader* header, VCTResult* result);
static int vct_send_notify_impl(VCTSession* session, SSPHeader* header);
static int vct_dispatch_session(SSPHeader* header, VCTSession* session);
static int vct_check_recv_state(VCTSession* session, SSPHeader* header);
static inline void vct_init_session(VCTSession* session, u8 aid);

static inline void vct_set_remote_aid(VCTSession* session, u32 aidBitmap) {
    session->aidBitmap = aidBitmap;
}

// Reliable send of an SSP header with the HMAC appended.
static inline BOOL vct_send_ssp_impl(u8 aid, SSPHeader* header) {
    u8 packet[sizeof(SSPHeader) * 2];

    if (sSkipSend) {
        return TRUE;
    }

    memcpy(packet, header, sizeof(SSPHeader));
    vct_add_hmac(packet + sizeof(SSPHeader), packet, sizeof(SSPHeader));
    return DWC_SendReliable(aid, packet, sizeof(packet));
}

static inline BOOL vct_send_ssp(VCTSession* session, SSPHeader* header) {
    if (header->method == VCT_METHOD_REQUEST && header->code == VCT_REQUEST_NOTIFY) {
        vct_send_notify_impl(session, header);
    } else {
        u8 aid;
        BOOL ret;

        VCTi_Report("----- Send ----------->\n");
        aid = session->aid;
        ret = vct_send_ssp_impl(aid, header);
        if (!ret) {
            VCTi_Report("Can't send request(%d) to aid %d (error: %d)", header->code, session->aid, ret);
            return FALSE;
        }
    }

    return TRUE;
}

VCTSession* VCT_CreateSession(u8 aid) {
    if (vct.SSPMode != VCT_MODE_NULL && vct.SSPMode != VCT_MODE_TRANSCEIVER) {
        return vct_create_session_impl(aid);
    } else {
        VCTi_Report("You can use this method only phone mode.\n");
        return NULL;
    }
}

BOOL VCT_DeleteSession(VCTSession* session) {
    VCTSession* prev = NULL;
    VCTSession* node = sSession.use;

    if (session == NULL) {
        OSReport("Session is null");
    }
    if (session == NULL) {
        return FALSE;
    }
    if (vct.SSPMode == VCT_MODE_TRANSCEIVER) {
        if (session == &sSession.trans) {
            sSession.trans.mode = VCT_MODE_NULL;
        }
        return TRUE;
    }
    while (node != NULL) {
        if (node == session) {
            session->mode = VCT_MODE_NULL;
            if (prev != NULL) {
                prev->next = session->next;
            } else {
                if (node->next != NULL) {
                    sSession.use = node->next;
                } else {
                    sSession.use = NULL;
                }
            }
            session->next = sSession.free;
            sSession.free = session;
            return TRUE;
        }
        prev = node;
        node = node->next;
    }
    VCTi_Report("VCT_DeleteSession: Session is not use!\n");
    return FALSE;
}

int vct_build_request_data(VCTSession* session, VCTRequestCode request, void* buffer, u32 size);
int vct_build_response_data(VCTSession* session, VCTResponseCode response, void* buffer, u32 size);

static inline int vct_build_ssp_header(VCTSession* session, SSPHeader* header, u8 code) {
    header->magic = VCT_MAGIC_PACKET_HEADER;
    header->version = VCT_SSP_HEADER_VERSION;
    header->code = code;
    header->from = vct.myAID;
    header->to = session->aid;
    header->info = VCT_NOTIFY_FREE;
    header->talking = vct.myAID;
    header->aidBitmap = 0;
    return 0;
}

static inline int vct_code_convert(SSPHeader* header) {
    int code = header->code;

    if (header->method == VCT_METHOD_RESPONSE) {
        code += VCT_REQUEST_END;
    }
    if (code < 0 || VCT_REQUEST_END + VCT_RESPONSE_END <= code) {
        return -1;
    }
    return code;
}

static inline int vct_check_send_state(VCTSession* session, SSPHeader* header) {
    int nextState;
    int code = vct_code_convert(header);

    if (code < 0) {
        return -1;
    }

    if (vct.SSPMode == VCT_MODE_PHONE) {
        nextState = sSendState[session->state][code];
    } else {
        nextState = sTransSendState[session->state][code];
    }

    if (nextState == VCT_NO_CHANGE) {
        return 1;
    } else if (nextState == VCT_STATE_ERROR) {
        return 0;
    } else {
        session->state = nextState;
        return 1;
    }
}

static inline BOOL vct_check_multisession(VCTSession* session) {
    VCTSession* node = sSession.use;

    while (node != NULL) {
        if (node->mode != VCT_MODE_NULL && node->state == VCT_STATE_TALKING && session != node) {
            return FALSE;
        }

        node = node->next;
    }

    return TRUE;
}

static inline VCTSession* vct_find_session(u8 aid) {
    VCTSession* node = sSession.use;

    if (vct.SSPMode == VCT_MODE_TRANSCEIVER) {
        if (sSession.trans.mode != 0 && sSession.trans.aid == aid) {
            return &sSession.trans;
        }
        return NULL;
    }
    while (node != NULL) {
        if (node->mode != VCT_MODE_NULL && node->aid == aid) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

int vct_build_request_data(VCTSession* session, VCTRequestCode request, void* buffer, u32 size) {
    SSPHeader* header = buffer;

    if (session == NULL) {
        OSReport("Session is null");
        return VCT_ERROR_BAD_PARAM;
    }
    if (size < sizeof(SSPHeader)) {
        return VCT_ERROR_BAD_PARAM;
    }
    if (vct.listenMode == LISTEN_MODE_SERVER) {
        return VCT_ERROR_BAD_REQUEST;
    }
    if (request < 0 || request >= VCT_REQUEST_END) {
        return VCT_ERROR_BAD_REQUEST;
    }
    header->method = VCT_METHOD_REQUEST;
    vct_build_ssp_header(session, header, request);
    if (vct_check_send_state(session, header) == 0) {
        return VCT_ERROR_BAD_REQUEST;
    }
    if (vct.SSPMode == VCT_MODE_PHONE && request == VCT_REQUEST_INVITE && !vct_check_multisession(session)) {
        return VCT_ERROR_BAD_REQUEST;
    }
    return VCT_ERROR_NONE;
}

static inline int vct_request_impl(VCTSession* session, VCTRequestCode request) {
    SSPHeader header;
    int ret = vct_build_request_data(session, request, &header, sizeof(SSPHeader));

    if (ret != VCT_ERROR_NONE) {
        return ret;
    }

    if (!vct_send_ssp(session, &header)) {
        return VCT_ERROR_SEND_FAIL;
    }

    return VCT_ERROR_NONE;
}

int VCT_Request(VCTSession* session, VCTRequestCode request) {
    if (vct.SSPMode == VCT_MODE_TRANSCEIVER) {
        return VCT_ERROR_BAD_REQUEST;
    }

    return vct_request_impl(session, request);
}

int vct_build_response_data(VCTSession* session, VCTResponseCode response, void* buffer, u32 size) {
    SSPHeader* header = buffer;

    if (session == NULL) {
        return VCT_ERROR_BAD_PARAM;
    }
    if (size < sizeof(SSPHeader)) {
        return VCT_ERROR_BAD_PARAM;
    }
    if (response < 0 || response >= VCT_RESPONSE_END) {
        return VCT_ERROR_BAD_REQUEST;
    }

    header->method = VCT_METHOD_RESPONSE;
    vct_build_ssp_header(session, header, response);
    if (response == VCT_RESPONSE_OK) {
        if (vct.SSPMode == VCT_MODE_PHONE && session->state == VCT_STATE_INCOMING && !vct_check_multisession(session)) {
            return VCT_ERROR_BAD_REQUEST;
        }

        vct_set_remote_aid(session, session->aid != 0 ? 1 << session->aid : 1);
    }

    if (vct_check_send_state(session, header) == 0) {
        return VCT_ERROR_BAD_REQUEST;
    }

    return VCT_ERROR_NONE;
}

static inline int vct_response_impl(VCTSession* session, VCTResponseCode response) {
    SSPHeader header;
    int ret = vct_build_response_data(session, response, &header, sizeof(SSPHeader));

    if (ret != 0) {
        return ret;
    }

    if (!vct_send_ssp(session, &header)) {
        return VCT_ERROR_SEND_FAIL;
    }

    return VCT_ERROR_NONE;
}

int VCT_Response(VCTSession* session, VCTResponseCode response) {
    if (vct.SSPMode == VCT_MODE_TRANSCEIVER) {
        return VCT_ERROR_BAD_REQUEST;
    }
    return vct_response_impl(session, response);
}

static inline int vct_send_notify(VCTNotify notify) {
    SSPHeader header;

    header.method = VCT_METHOD_REQUEST;
    vct_build_ssp_header(&sSession.trans, &header, VCT_REQUEST_NOTIFY);
    header.info = notify;
    return vct_send_notify_impl(&sSession.trans, &header);
}

static inline int vct_ssp_auto_response(VCTRequestCode request) {
    VCTEvent event;
    int ret;

    if (request == VCT_REQUEST_CONTACT) {
        sSession.trans.state = VCT_STATE_CONNECTED;
        event = VCT_EVENT_CONNECTED;
        sSession.trans.aid = vct.myAID;
        sSession.trans.talking = vct.myAID;
        ret = vct_send_notify(VCT_NOTIFY_BUSY);
        if (ret != VCT_ERROR_NONE) {
            return ret;
        }
        sTransTalkTime = OSGetTime();
    } else if (request == VCT_REQUEST_BYE) {
        ret = vct_send_notify(VCT_NOTIFY_FREE);
        if (ret != VCT_ERROR_NONE) {
            return ret;
        }
        sSession.trans.mode = VCT_MODE_NULL;
        sSession.trans.state = VCT_STATE_INIT;
        sTransTalkTime = 0;
        event = VCT_EVENT_DISCONNECTED;
    } else {
        return VCT_ERROR_BAD_REQUEST;
    }
    vct.callback(vct.myAID, event, &sSession.trans, vct.userData);
    return VCT_ERROR_NONE;
}

int VCT_Contact(VCTSession** outSession) {
    int ret;

    if (vct.SSPMode != VCT_MODE_TRANSCEIVER) {
        OSReport("This method only use transcever mode");
        return VCT_ERROR_BAD_REQUEST;
    }
    if (sSession.trans.mode == VCT_MODE_NULL) {
        vct_init_session(&sSession.trans, vct.serverAID);
    } else {
        return VCT_ERROR_TRANSCEIVER_BUSY;
    }
    if (vct.listenMode == LISTEN_MODE_CLIENT) {
        SSPHeader header;

        ret = vct_build_request_data(&sSession.trans, VCT_REQUEST_CONTACT, &header, sizeof(SSPHeader));
        if (ret != VCT_ERROR_NONE) {
            return ret;
        }
        if (vct_send_ssp(&sSession.trans, &header)) {
            ret = VCT_ERROR_NONE;
        } else {
            ret = VCT_ERROR_SEND_FAIL;
        }
    } else {
        SSPHeader header;

        header.method = VCT_METHOD_REQUEST;
        header.code = VCT_REQUEST_CONTACT;
        if (vct_check_send_state(&sSession.trans, &header) == 0) {
            return VCT_ERROR_BAD_REQUEST;
        }

        ret = vct_ssp_auto_response(VCT_REQUEST_CONTACT);
    }
    if (outSession != NULL) {
        *outSession = &sSession.trans;
    }
    return ret;
}

int VCT_Release(VCTSession* session) {
    SSPHeader header;

    if (vct.SSPMode != VCT_MODE_TRANSCEIVER) {
        OSReport("This method only use transcever mode");
        return VCT_ERROR_BAD_REQUEST;
    }
    if (session == NULL) {
        OSReport("session is null");
        return VCT_ERROR_BAD_PARAM;
    }

    if (vct.listenMode == LISTEN_MODE_CLIENT) {
        int ret = vct_build_request_data(session, VCT_REQUEST_BYE, &header, sizeof(SSPHeader));

        if (ret != VCT_ERROR_NONE) {
            return ret;
        }
        if (vct_send_ssp(session, &header)) {
            return VCT_ERROR_NONE;
        } else {
            return VCT_ERROR_SEND_FAIL;
        }
    }

    header.method = VCT_METHOD_REQUEST;
    header.code = VCT_REQUEST_BYE;
    if (vct_check_send_state(&sSession.trans, &header) == 0) {
        return VCT_ERROR_BAD_REQUEST;
    }

    return vct_ssp_auto_response(VCT_REQUEST_BYE);
}

int VCT_SetTransceiverServer(u8 aid) {
    if (vct.SSPMode != VCT_MODE_TRANSCEIVER) {
        return VCT_ERROR_BAD_MODE;
    }
    if (vct.listenMode != LISTEN_MODE_CLIENT) {
        return VCT_ERROR_BAD_MODE;
    }
    if (vct.myAID == aid) {
        VCTi_Report("Can't set server aid (aid '%d' is own aid)\n", aid);
        return VCT_ERROR_BAD_PARAM;
    }
    sSession.trans.aid = aid;
    vct.serverAID = aid;
    return VCT_ERROR_NONE;
}

int VCT_SetTransceiverClients(u8 aidList[], int num_of_aid) {
    u32 i;
    u32 j = 0;

    if (num_of_aid > VCT_MAX_TRANSCEIVER_CLIENT) {
        return VCT_ERROR_BAD_PARAM;
    }
    if (vct.listenMode != LISTEN_MODE_SERVER) {
        return VCT_ERROR_BAD_MODE;
    }
    if (aidList == NULL || num_of_aid == 0) {
        sNumOfNotifyAID = 0;
        return VCT_ERROR_NONE;
    }
    sNotifyAID = 0;
    for (i = 0; i < num_of_aid; i++) {
        sNotifyAID |= aidList[i] != 0 ? 1 << aidList[i] : 1;

        if (vct.myAID != aidList[i]) {
            j++;
        }
    }
    if (j > VCT_MAX_TRANSCEIVER_CLIENT - 1) {
        return VCT_ERROR_BAD_PARAM;
    }

    sNotifyAID |= vct.myAID != 0 ? 1 << vct.myAID : 1;
    return VCT_ERROR_NONE;
}

void VCT_SetTransceiverTimeout(u32 sec) {
    sTransceiverLimit = (u64)sec * 60750000;
}

int VCT_AddConferenceClient(u8 aid) {
    VCTSession* session;

    if (vct.SSPMode != VCT_MODE_CONFERENCE) {
        return VCT_ERROR_BAD_MODE;
    }
    if (sNumOfNotifyAID == VCT_MAX_CONFERENCE_CLIENT - 1) {
        return VCT_ERROR_EXHAUST_CLIENTS;
    }
    if (aid == vct.myAID) {
        OSReport("Can't set self aid");
    }
    if (aid == vct.myAID) {
        return VCT_ERROR_BAD_PARAM;
    }
    if (vct_find_session(aid) != 0) {
        VCTi_Report("AID %d already join conference\n", aid);
        return VCT_ERROR_NONE;
    }
    session = VCT_CreateSession(aid);
    if (session == NULL) {
        return VCT_ERROR_BAD_PARAM;
    }
    session->aid = aid;
    session->talking = aid;
    session->state = VCT_STATE_TALKING;
    session->aidBitmap = 1 << aid;
    vct.callback(aid, VCT_EVENT_CONNECTED, session, vct.userData);
    sNumOfNotifyAID++;
    return VCT_ERROR_NONE;
}

int VCT_RemoveConferenceClient(u8 aid) {
    VCTSession* session;

    if (vct.SSPMode != VCT_MODE_CONFERENCE) {
        return VCT_ERROR_BAD_MODE;
    }
    session = vct_find_session(aid);
    if (session == NULL) {
        VCTi_Report("AID %d not join conference\n", aid);
        return VCT_ERROR_BAD_PARAM;
    }
    vct.callback(aid, VCT_EVENT_DISCONNECTED, session, vct.userData);
    sNumOfNotifyAID--;
    return VCT_ERROR_NONE;
}

static u64 vct_get_tick_ssp(void) {
    return OSGetTime();
}

BOOL vct_init_ssp(VCTConfig* config) {
    u32 i;

    if (config->session == NULL || config->numSession == 0 || config->numSession > 8) {
        return FALSE;
    }

    sSession.free = config->session;
    MI_CpuFill8(sSession.free, 0, config->numSession * sizeof(VCTSession));
    MI_CpuFill8(&sSession.trans, 0, sizeof(VCTSession));
    MI_CpuFill8(&sSession.tmp, 0, sizeof(VCTSession));
    for (i = 0; i < config->numSession - 1; i++) {
        sSession.free[i].next = &sSession.free[i + 1];
    }

    sSession.free[config->numSession - 1].next = NULL;
    sSession.use = 0;
    sNotifyAID = 0;
    sNumOfNotifyAID = 0;
    vct.autoResponse = 0;
    MATH_InitRand16(&sSession.rand, (u32)vct_get_tick_ssp);
    return TRUE;
}

void vct_finish_ssp(void) {
    sSession.use = NULL;
    sSession.free = NULL;
}

void vct_check_timeout(void) {
    u64 tick = vct_get_tick_ssp();

    if (vct.SSPMode != VCT_MODE_TRANSCEIVER) {
        return;
    }
    if (vct.listenMode == LISTEN_MODE_CLIENT) {
        return;
    }
    if (sTransTalkTime == 0) {
        return;
    }
    if (tick - sTransTalkTime <= sTransceiverLimit) {
        return;
    }

    sTransTalkTime = 0;
    sSession.trans.mode = VCT_MODE_NULL;
    sSession.trans.state = VCT_STATE_INIT;
    vct_send_notify(VCT_NOTIFY_FREE);
    vct.callback(sSession.trans.aid, VCT_EVENT_DISCONNECTED, &sSession.trans, vct.userData);
}

static inline BOOL vct_response_reject_impl(SSPHeader* header, VCTResult* result) {
    VCTSession* session;

    if (header->method == VCT_METHOD_REQUEST && header->code == VCT_REQUEST_NOTIFY) {
        return FALSE;
    }

    session = vct_find_session(header->from);
    if (session != NULL) {
        result->event = VCT_EVENT_REJECT;
        result->session = session;
        session->mode = VCT_MODE_NULL;
        vct_response_impl(session, VCT_RESPONSE_NOT_ACCEPTABLE);
        return TRUE;
    } else {
        vct_init_session(&sSession.tmp, header->from);
        vct_response_impl(&sSession.tmp, VCT_RESPONSE_NOT_ACCEPTABLE);
        return FALSE;
    }
}

BOOL vct_handle_ssp(u8 aid, u8* buffer, int size, VCTResult* result) {
    SSPHeader* header = (SSPHeader*)buffer;

    if (size != sizeof(SSPHeader)) {
        VCTi_Report("Discard packet by invalid length (%d != %d)\n", size, sizeof(SSPHeader));
        return FALSE;
    }
    if (header->from != aid || header->to != vct.myAID) {
        VCTi_Report("Discard packet by AID mismatch [%d->%d != %d->%d]\n", header->from, header->to, aid, vct.myAID);
        return FALSE;
    }

    if (header->version != VCT_SSP_HEADER_VERSION) {
        VCTi_Report("Discard packet by invalid version (%x)\n", header->version);
        return vct_response_reject_impl(header, result);
    }

    VCTi_Report("<----------- Recv -----\n");
    if (vct.listenMode == LISTEN_MODE_CLIENT) {
        return vct_handle_ssp_client(header, result);
    } else {
        return vct_handle_ssp_server(header, result);
    }
}

static VCTSession* vct_create_session_impl(u8 aid) {
    VCTSession* result;

    if (aid >= DWC_MAX_CONNECTIONS) {
        return NULL;
    }
    if (aid == vct.myAID) {
        VCTi_Report("Can't create self aid session\n");
        return NULL;
    }

    result = sSession.free;
    if (result == NULL) {
        VCTi_Report("No left session\n");
        return NULL;
    }

    sSession.free = result->next;
    vct_init_session(result, aid);
    result->next = sSession.use;
    sSession.use = result;
    return result;
}

static inline void vct_add_notify_aid_list(VCTSession* session, SSPHeader* header) {
    if (header->info != VCT_NOTIFY_BUSY) {
        return;
    }
    VCTi_Report("Now talking aid:%d\n", session->aid);
    header->talking = session->aid;
    header->aidBitmap = sNotifyAID;
}

static int vct_send_notify_impl(VCTSession* session, SSPHeader* header) {
    int count = 0;
    u8 aid;

    vct_add_notify_aid_list(session, header);
    VCTi_Report("----- Send Notify ---->\n");
    VCTi_Report("Send notify:\n");
    for (aid = 0; aid < DWC_MAX_CONNECTIONS; aid++) {
        if ((sNotifyAID & (aid != 0 ? 1 << aid : 1)) && (aid != vct.myAID)) {
            header->to = aid;
            if (vct_send_ssp_impl(aid, header)) {
                VCTi_Report(" OK -> %d\n", aid);
                count++;
            } else {
                VCTi_Report(" NG -> %d\n", aid);
            }
        }
    }

    if (header->info == VCT_NOTIFY_BUSY) {
        vct_set_remote_aid(session, sNotifyAID);
    } else if (header->info == VCT_NOTIFY_FREE) {
        vct_set_remote_aid(session, 0);
    }

    if (count == 0) {
        return VCT_ERROR_SEND_FAIL;
    }

    return VCT_ERROR_NONE;
}

static inline void vct_init_session(VCTSession* session, u8 aid) {
    memset(session, 0, sizeof(VCTSession));
    session->mode = vct.SSPMode;
    session->state = VCT_STATE_INIT;
    session->aid = aid;
}

static int vct_dispatch_session(SSPHeader* header, VCTSession* session) {
    session->talking = header->talking;

    if (header->method == VCT_METHOD_REQUEST) {
        if (header->code == VCT_REQUEST_INVITE) {
            vct_set_remote_aid(session, session->aid != 0 ? 1 << session->aid : 1);
        }

        return sRequestEvent[header->code];
    } else if (header->method == VCT_METHOD_RESPONSE) {
        if (header->code == VCT_RESPONSE_OK) {
            if (session->state == VCT_STATE_OUTGOING) {
                vct_set_remote_aid(session, session->aid != 0 ? 1 << session->aid : 1);
                return VCT_EVENT_CONNECTED;
            } else if (session->state == VCT_STATE_DISCONNECTING) {
                return VCT_EVENT_DISCONNECTED;
            } else {
                return VCT_EVENT_ABORT;
            }
        }

        return sResponseEvent[header->code];
    } else {
        return VCT_EVENT_ABORT;
    }
}

static int vct_check_recv_state(VCTSession* session, SSPHeader* header) {
    int nextState;
    int code = vct_code_convert(header);

    if (code < 0) {
        return -1;
    }

    if (vct.SSPMode == VCT_MODE_PHONE) {
        nextState = sRecvState[session->state][code];
    } else {
        nextState = sTransRecvState[session->state][code];
    }

    switch (nextState) {
    case VCT_NO_CHANGE:
        return nextState;
    case VCT_STATE_ERROR:
        vct_response_impl(session, VCT_RESPONSE_BAD_REQUEST);
        session->state = VCT_STATE_INIT;
        session->mode = VCT_MODE_NULL;
        return nextState;
    case VCT_CONTACT_FAIL:
        vct_response_impl(session, VCT_RESPONSE_BUSY_HERE);
        session->state = VCT_STATE_INIT;
        session->mode = VCT_MODE_NULL;
        return nextState;
    default:
        return nextState;
    }
}

static BOOL vct_handle_ssp_client(SSPHeader* header, VCTResult* result) {
    VCTSession* session;
    int nextState;

    if (header->method == VCT_METHOD_REQUEST) {
        if (header->code == VCT_REQUEST_NOTIFY) {
            return vct_handle_notify(header, result);
        }
        if (header->code == VCT_REQUEST_CONTACT) {
            VCTi_Report("Discard packet by invalid contact request\n");
            return FALSE;
        }
    }
    session = vct_find_session(header->from);
    if (session != NULL) {
        nextState = vct_check_recv_state(session, header);
        switch (nextState) {
        case VCT_NO_CHANGE:
            return FALSE;
        case VCT_STATE_ERROR:
            result->event = VCT_EVENT_ABORT;
            session->mode = VCT_MODE_NULL;
            result->session = session;
            return TRUE;
        default:
            result->event = vct_dispatch_session(header, session);
            result->session = session;
            session->state = nextState;
            if (result->event != VCT_EVENT_NONE) {
                return TRUE;
            } else {
                return FALSE;
            }
        }
    } else {
        session = vct_create_session_impl(header->from);
        if (session == NULL) {
            VCTi_Report("SDK audio response BUSY\n");
            vct_init_session(&sSession.tmp, header->from);
            vct_response_impl(&sSession.tmp, VCT_RESPONSE_BUSY_HERE);
            return FALSE;
        }

        nextState = vct_check_recv_state(session, header);
        if (nextState == VCT_NO_CHANGE || nextState == VCT_STATE_ERROR) {
            VCT_DeleteSession(session);
            return FALSE;
        }

        result->event = vct_dispatch_session(header, session);
        result->session = session;
        session->state = nextState;
        if (result->event != VCT_EVENT_NONE) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

static BOOL vct_handle_ssp_server(SSPHeader* header, VCTResult* result) {
    int nextState;

    if (sSession.trans.mode == VCT_MODE_TRANSCEIVER) {
        if (sSession.trans.aid == header->from) {
            nextState = vct_check_recv_state(&sSession.trans, header);
            switch (nextState) {
            case VCT_STATE_ERROR:
                result->event = VCT_EVENT_ABORT;
                result->session = NULL;
                return TRUE;
            case VCT_CONTACT_FAIL:
            case VCT_NO_CHANGE:
                return FALSE;
            default:
                result->event = vct_dispatch_session(header, &sSession.trans);
                result->session = &sSession.trans;
                result->session->state = nextState;
            }
        } else {
            VCTi_Report("*** Server is busy ***\n");
            vct_send_notify(VCT_NOTIFY_BUSY);
            return FALSE;
        }
    } else {
        if (!(sNotifyAID & (header->from != 0 ? 1 << header->from : 1))) {
            return FALSE;
        }

        sSession.trans.aid = header->from;
        sSession.trans.mode = VCT_MODE_TRANSCEIVER;
        sSession.trans.state = VCT_STATE_INIT;

        nextState = vct_check_recv_state(&sSession.trans, header);
        switch (nextState) {
        case VCT_CONTACT_FAIL:
        case VCT_STATE_ERROR:
        case VCT_NO_CHANGE:
            sSession.trans.mode = VCT_MODE_NULL;
            return FALSE;
        default:
            result->event = vct_dispatch_session(header, &sSession.trans);
            result->session = &sSession.trans;
            result->session->state = nextState;
            break;
        }
    }

    if (result->event == VCT_EVENT_CONTACT) {
        vct_send_notify(VCT_NOTIFY_BUSY);
        sTransTalkTime = OSGetTime();
    } else {
        vct_send_notify(VCT_NOTIFY_FREE);
        sTransTalkTime = 0;
    }

    return TRUE;
}

static BOOL vct_handle_notify(SSPHeader* header, VCTResult* result) {
    if (vct.SSPMode != VCT_MODE_TRANSCEIVER) {
        return FALSE;
    }
    if (vct.serverAID != header->from) {
        VCTi_Report("Discard packet by invalid server notify from aid:%d\n", header->from);
        return FALSE;
    }

    switch (header->info) {
    case VCT_NOTIFY_FREE:
        if (sSession.trans.mode == VCT_MODE_NULL) {
            return FALSE;
        }
        if (sSession.trans.state == VCT_STATE_DISCONNECTING || sSession.trans.state == VCT_STATE_TALKING) {
            result->event = VCT_EVENT_DISCONNECTED;
        } else {
            result->event = VCT_EVENT_NOTIFY_FREE;
        }
        sSession.trans.mode = VCT_MODE_NULL;
        sSession.trans.state = VCT_STATE_INIT;
        result->session = &sSession.trans;
        break;
    case VCT_NOTIFY_BUSY:
        sSession.trans.mode = VCT_MODE_TRANSCEIVER;
        if (header->talking == vct.myAID) {
            sSession.trans.state = VCT_STATE_TALKING;
            result->event = VCT_EVENT_CONNECTED;
            vct_set_remote_aid(&sSession.trans, header->aidBitmap);
        } else {
            sSession.trans.state = VCT_STATE_CONNECTED;
            result->event = VCT_EVENT_NOTIFY_BUSY;
            vct_set_remote_aid(&sSession.trans, header->talking != 0 ? 1 << header->talking : 1);
        }

        sSession.trans.talking = header->talking;
        result->session = &sSession.trans;
        break;
    default:
        return FALSE;
    }

    return TRUE;
}
