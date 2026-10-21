/******
GameSpy NAT Negotiation SDK - negotiation client
******/

#include <stdio.h>
#include <string.h>

#include "../common/gsAvailable.h"
#include "../common/gsCommon.h"
#include "../darray.h"
#include "nninternal.h"

#define MATCHUP1_HOSTNAME "natneg1." GSI_DOMAIN_NAME
#define MATCHUP2_HOSTNAME "natneg2." GSI_DOMAIN_NAME
#define MATCHUP3_HOSTNAME "natneg3." GSI_DOMAIN_NAME

unsigned char NNMagicData[NATNEG_MAGIC_LEN] = {
    NN_MAGIC_0, NN_MAGIC_1, NN_MAGIC_2, NN_MAGIC_3, NN_MAGIC_4, NN_MAGIC_5};

// Big-endian stores into the (byte packed) packet buffer
#define NN_STORE32(dst, v)                                                     \
    (((unsigned char *)(dst))[0] = (v) >> 24,                                  \
     ((unsigned char *)(dst))[1] = (v) >> 16,                                  \
     ((unsigned char *)(dst))[2] = (v) >> 8,                                   \
     ((unsigned char *)(dst))[3] = (v))
#define NN_STORE16(dst, v)                                                     \
    (((unsigned char *)(dst))[0] = (v) >> 8, ((unsigned char *)(dst))[1] = (v))

// The compiler lays out .sbss in reverse declaration order
char *Matchup1Hostname;
char *Matchup2Hostname;
char *Matchup3Hostname;
NatDetectionResultsFunc natifyCallback;
static unsigned int natifyStartTime;
static NatMappingScheme natMappingScheme;
static int activeNatify;
unsigned int matchup3ip;
unsigned int matchup2ip;
unsigned int matchup1ip;
DArray negotiateList;

static SOCKET mappingSock = INVALID_SOCKET;
static SOCKET ertSock = INVALID_SOCKET;
static NatType natType = unknown;

static NATNegotiator *FindNegotiatorForCookie(int cookie) {
    int i;
    NATNegotiator *neg;

    if (negotiateList == NULL) {
        return NULL;
    }

    for (i = 0; i < ArrayLength(negotiateList); i++) {
        neg = (NATNegotiator *)ArrayNth(negotiateList, i);
        if (neg->cookie == cookie) {
            return neg;
        }
    }

    return NULL;
}

static NATNegotiator *AddNegotiator() {
    NATNegotiator _neg;

    memset(&_neg, 0, sizeof(NATNegotiator));

    if (negotiateList == NULL) {
        negotiateList = ArrayNew(sizeof(NATNegotiator), 4, NULL);
    }

    ArrayAppend(negotiateList, &_neg);
    return (NATNegotiator *)ArrayNth(negotiateList,
                                     ArrayLength(negotiateList) - 1);
}

static void RemoveNegotiator(NATNegotiator *neg) {
    int i;

    for (i = 0; i < ArrayLength(negotiateList); i++) {
        if (neg == (NATNegotiator *)ArrayNth(negotiateList, i)) {
            ArrayRemoveAt(negotiateList, i);
            return;
        }
    }
}

void NNFreeNegotiateList() {
    if (negotiateList != NULL) {
        ArrayFree(negotiateList);
        negotiateList = NULL;
    }
}

static int CheckMagic(char *data) {
    return memcmp(data, NNMagicData, NATNEG_MAGIC_LEN) == 0;
}

static void SendPacket(SOCKET sock, unsigned int toaddr, unsigned short toport,
                       void *data, int len) {
    SOSockAddrIn saddr;

    saddr.family = AF_INET;
    saddr.port = htons(toport);
    saddr.addr.addr = toaddr;
    sendto(sock, (char *)data, len, 0, (SOCKADDR *)&saddr, sizeof(saddr));
}

static unsigned int GetLocalIP() {
    int num_local_ips;
    HOSTENT *phost;
    IN_ADDR *addr;
    unsigned int localip = 0;

    phost = getlocalhost();
    if (phost == NULL) {
        gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                      GSIDebugLevel_HotError, "getlocalhost failed\n");
        return 0;
    }

    for (num_local_ips = 0;; num_local_ips++) {
        addr = (IN_ADDR *)phost->h_addr_list[num_local_ips];
        if (addr == NULL) {
            break;
        }

        if (addr->addr != htonl(0x7F000001)) {
            localip = addr->addr;
            if (IsPrivateIP(addr)) {
                return localip;
            }
        }
    }

    return localip;
}

static unsigned short GetLocalPort(SOCKET sock) {
    int ret;
    SOSockAddrIn saddr;
    int saddrlen = sizeof(saddr);

    ret = getsockname(sock, (SOCKADDR *)&saddr, &saddrlen);
    return ntohs(gsiSocketIsError(ret) ? 0 : saddr.port);
}

static void SendReportPacket(NATNegotiator *neg) {
    NatNegPacket p;

    memcpy(p.magic, NNMagicData, NATNEG_MAGIC_LEN);
    p.version = NN_PROTVER;
    p.packettype = NN_REPORT;
    p.cookie = htonl(neg->cookie);
    p.Packet.Report.clientindex = (unsigned char)neg->clientindex;
    p.Packet.Report.negResult = (neg->result == nr_success) ? 1 : 0;
    p.Packet.Report.natType = natType;
    p.Packet.Report.natMappingScheme = natMappingScheme;
    if (strlen(__GSIACGamename)) {
        memcpy(p.Packet.Report.gamename, __GSIACGamename,
               sizeof(p.Packet.Report.gamename));
    }

    gsDebugFormat(
        GSIDebugCat_NatNeg, GSIDebugType_Network, GSIDebugLevel_Comment,
        "Sending REPORT to %s:%d (result: %d)\n",
        inet_ntoa(*(IN_ADDR *)&matchup1ip), MATCHUP_PORT, neg->result);
    SendPacket(neg->negotiateSock, matchup1ip, MATCHUP_PORT, &p,
               REPORTPACKET_SIZE);
}

static void StartReport(NATNegotiator *neg, NegotiateResult result,
                        SOCKET socket, SOSockAddrIn *remoteaddr) {
    neg->result = result;
    neg->connectedSocket = socket;
    if (remoteaddr != NULL) {
        memcpy(&neg->remoteaddr, remoteaddr, sizeof(SOSockAddrIn));
    }

    if (result == nr_deadbeatpartner || result == nr_inittimeout) {
        neg->state = ns_finished;
        neg->completedCallback(neg->result, neg->connectedSocket,
                               &neg->remoteaddr, neg->userdata);
        NNCancel(neg->cookie);
    } else {
        SendReportPacket(neg);
        neg->state = ns_reportsent;
        neg->retryTime = current_time() + REPORT_RETRY_TIME;
        neg->retryCount = 0;
        neg->maxRetryCount = REPORT_RETRY_COUNT;
    }
}

static void SendInitPackets(NATNegotiator *neg) {
    char buffer[85];
    NatNegPacket *p = (NatNegPacket *)buffer;
    unsigned int localip;
    unsigned short localport;
    int packetlen;

    memcpy(p->magic, NNMagicData, NATNEG_MAGIC_LEN);
    p->version = NN_PROTVER;
    p->packettype = NN_INIT;
    p->cookie = htonl(neg->cookie);
    p->Packet.Init.clientindex = (unsigned char)neg->clientindex;
    p->Packet.Init.usegameport = (neg->gameSock != INVALID_SOCKET);
    localip = GetLocalIP();
    localip = ntohl(localip);
    NN_STORE32(&p->Packet.Init.localip, localip);
    localport = 0;
    NN_STORE16(&p->Packet.Init.localport, localport);
    strcpy(buffer + INITPACKET_SIZE, __GSIACGamename);
    packetlen = INITPACKET_SIZE + strlen(__GSIACGamename) + 1;

    if (p->Packet.Init.usegameport && !neg->initAckRecv[NN_PT_GP]) {
        p->Packet.Init.porttype = NN_PT_GP;
        gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                      GSIDebugLevel_Comment, "Sending INIT (GP) to %s:%d...\n",
                      inet_ntoa(*(IN_ADDR *)&matchup1ip), MATCHUP_PORT);
        SendPacket(neg->gameSock, matchup1ip, MATCHUP_PORT, p, packetlen);
    }

    if (!neg->initAckRecv[NN_PT_NN1]) {
        p->Packet.Init.porttype = NN_PT_NN1;
        gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                      GSIDebugLevel_Comment, "Sending INIT (NN1) to %s:%d...\n",
                      inet_ntoa(*(IN_ADDR *)&matchup1ip), MATCHUP_PORT);
        SendPacket(neg->negotiateSock, matchup1ip, MATCHUP_PORT, p, packetlen);
    }

    localport = GetLocalPort(p->Packet.Init.usegameport ? neg->gameSock
                                                        : neg->negotiateSock);
    NN_STORE16(&p->Packet.Init.localport, localport);

    if (!neg->initAckRecv[NN_PT_NN2]) {
        p->Packet.Init.porttype = NN_PT_NN2;
        gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                      GSIDebugLevel_Comment, "Sending INIT (NN2) to %s:%d...\n",
                      inet_ntoa(*(IN_ADDR *)&matchup2ip), MATCHUP_PORT);
        SendPacket(neg->negotiateSock, matchup2ip, MATCHUP_PORT, p, packetlen);
    }

    if (!neg->initAckRecv[NN_PT_NN3]) {
        p->Packet.Init.porttype = NN_PT_NN3;
        gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                      GSIDebugLevel_Comment, "Sending INIT (NN3) to %s:%d...\n",
                      inet_ntoa(*(IN_ADDR *)&matchup3ip), MATCHUP_PORT);
        SendPacket(neg->negotiateSock, matchup3ip, MATCHUP_PORT, p, packetlen);
    }

    neg->retryTime = current_time() + INIT_RETRY_TIME;
    neg->maxRetryCount = INIT_RETRY_COUNT;
}

static void SendPingPacket(NATNegotiator *neg) {
    NatNegPacket p;

    memcpy(p.magic, NNMagicData, NATNEG_MAGIC_LEN);
    p.version = NN_PROTVER;
    p.packettype = NN_CONNECT_PING;
    p.cookie = htonl(neg->cookie);
    p.Packet.Connect.remoteIP = neg->guessedIP;
    p.Packet.Connect.remotePort = htons(neg->guessedPort);
    p.Packet.Connect.gotyourdata = neg->gotRemoteData;
    p.Packet.Connect.finished = (neg->state == ns_connectping) ? 0 : 1;

    gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                  GSIDebugLevel_Comment,
                  "Sending PING to %s:%d (got remote data: %d)\n",
                  inet_ntoa(*(IN_ADDR *)&neg->guessedIP), neg->guessedPort,
                  neg->gotRemoteData);
    SendPacket(neg->gameSock != INVALID_SOCKET ? neg->gameSock
                                               : neg->negotiateSock,
               neg->guessedIP, neg->guessedPort, &p, CONNECTPACKET_SIZE);

    neg->retryTime = current_time() + PING_RETRY_TIME;
    neg->maxRetryCount = PING_RETRY_COUNT;
    if (neg->gotRemoteData) {
        neg->sendGotRemoteData = 1;
    }
}

NAT nat;

static int CheckNatifyStatus(SOCKET sock) {
    int active = 1;
    int success;

    if (sock != INVALID_SOCKET) {
        if (current_time() - natifyStartTime < NATIFY_TIMEOUT) {
            active = NatifyThink(sock, &nat);
        } else {
            active = 0;
        }

        if (!active) {
            success = DetermineNatType(&nat);
            natifyCallback(success, nat);
            natType = nat.natType;
            natMappingScheme = nat.mappingScheme;

            if (mappingSock != INVALID_SOCKET) {
                closesocket(mappingSock);
            }
            mappingSock = INVALID_SOCKET;

            if (ertSock != INVALID_SOCKET) {
                closesocket(ertSock);
            }
            ertSock = INVALID_SOCKET;
        }
    }

    return active;
}

static unsigned int NameToIp(const char *name) {
    unsigned int ret;
    HOSTENT *hent;

    ret = inet_addr(name);
    if (ret == INADDR_NONE) {
        hent = gethostbyname(name);
        if (hent == NULL) {
            ret = 0;
        } else {
            ret = *(unsigned int *)hent->h_addr_list[0];
        }
    }

    return ret;
}

static unsigned int ResolveServer(const char *overrideHostname,
                                  const char *defaultHostname) {
    const char *hostname;
    char hostnameBuffer[64];

    if (overrideHostname == NULL) {
        snprintf(hostnameBuffer, sizeof(hostnameBuffer), "%s.%s",
                 __GSIACGamename, defaultHostname);
        hostname = hostnameBuffer;
    } else {
        hostname = overrideHostname;
    }

    return NameToIp(hostname);
}

static int ResolveServers() {
    if (!matchup1ip) {
        matchup1ip = ResolveServer(Matchup1Hostname, MATCHUP1_HOSTNAME);
    }
    if (!matchup2ip) {
        matchup2ip = ResolveServer(Matchup2Hostname, MATCHUP2_HOSTNAME);
    }
    if (!matchup3ip) {
        matchup3ip = ResolveServer(Matchup3Hostname, MATCHUP3_HOSTNAME);
    }

    if (!matchup1ip || !matchup2ip || !matchup3ip) {
        return 0;
    }

    return 1;
}

NegotiateError
NNBeginNegotiationWithSocket(SOCKET gamesocket, int cookie, int clientindex,
                             NegotiateProgressFunc progresscallback,
                             NegotiateCompletedFunc completedcallback,
                             void *userdata) {
    NATNegotiator *neg;

    if (__GSIACResult != GSIACAvailable) {
        return ne_socketerror;
    }

    if (!ResolveServers()) {
        return ne_dnserror;
    }

    neg = AddNegotiator();
    if (neg == NULL) {
        return ne_allocerror;
    }

    neg->gameSock = gamesocket;
    neg->clientindex = clientindex;
    neg->cookie = cookie;
    neg->progressCallback = progresscallback;
    neg->completedCallback = completedcallback;
    neg->userdata = userdata;
    neg->negotiateSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    neg->retryCount = 0;
    neg->gotRemoteData = 0;
    neg->sendGotRemoteData = 0;
    neg->guessedIP = 0;
    neg->guessedPort = 0;
    neg->maxRetryCount = 0;
    neg->result = nr_noresult;

    if (neg->negotiateSock == INVALID_SOCKET) {
        RemoveNegotiator(neg);
        return ne_socketerror;
    }

    SendInitPackets(neg);
    return ne_noerror;
}

void NNCancel(int cookie) {
    NATNegotiator *neg;

    neg = FindNegotiatorForCookie(cookie);
    if (neg != NULL) {
        if (neg->negotiateSock != INVALID_SOCKET) {
            closesocket(neg->negotiateSock);
        }
        neg->negotiateSock = INVALID_SOCKET;
        neg->state = ns_canceled;
    }
}

static char indata[512] POST_ALIGN(16);

static void NegotiateThink(NATNegotiator *neg) {
    SOSockAddrIn saddr;
    int saddrlen = sizeof(saddr);
    int error;

    if (activeNatify) {
        activeNatify = CheckNatifyStatus(mappingSock);
        activeNatify = CheckNatifyStatus(ertSock);
    }

    if (neg == NULL) {
        return;
    }

    if (neg->state == ns_canceled) {
        gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Misc,
                      GSIDebugLevel_Notice, "Removing canceled negotiator\n");
        RemoveNegotiator(neg);
        return;
    }

    if (neg->negotiateSock != INVALID_SOCKET) {
        while (CanReceiveOnSocket(neg->negotiateSock)) {
            error = recvfrom(neg->negotiateSock, indata, sizeof(indata), 0,
                             (SOCKADDR *)&saddr, &saddrlen);
            if (gsiSocketIsError(error)) {
                gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                              GSIDebugLevel_HotError, "RECV SOCKET ERROR: %d\n",
                              GOAGetLastError(neg->negotiateSock));
                break;
            }

            NNProcessData(indata, error, &saddr);
            if (neg->state == ns_canceled ||
                neg->negotiateSock == INVALID_SOCKET) {
                break;
            }
        }
    }

    if (neg->state == ns_initsent || neg->state == ns_connectping) {
        if (current_time() > neg->retryTime) {
            if (neg->retryCount > neg->maxRetryCount) {
                gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                              GSIDebugLevel_Notice, "RETRY FAILED...\n");
                if (neg->state == ns_initsent) {
                    StartReport(neg, nr_inittimeout, INVALID_SOCKET, NULL);
                } else {
                    StartReport(neg, nr_pingtimeout, INVALID_SOCKET, NULL);
                }
            } else {
                neg->retryCount++;
                if (neg->state == ns_initsent) {
                    SendInitPackets(neg);
                } else {
                    SendPingPacket(neg);
                }
                gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                              GSIDebugLevel_Comment, "[retry]\n");
            }
        }
    }

    if (neg->state == ns_finished) {
        if (current_time() > neg->retryTime) {
            SOSockAddrIn toaddr;

            toaddr.family = AF_INET;
            toaddr.port = htons(neg->guessedPort);
            toaddr.addr.addr = neg->guessedIP;
            if (neg->gameSock == INVALID_SOCKET) {
                StartReport(neg, nr_success, neg->negotiateSock, &toaddr);
            } else {
                StartReport(neg, nr_success, neg->gameSock, &toaddr);
            }
        }
    }

    if (neg->state == ns_initack) {
        if (current_time() > neg->retryTime) {
            StartReport(neg, nr_deadbeatpartner, INVALID_SOCKET, NULL);
        }
    }

    if (neg->state == ns_reportsent) {
        if (current_time() > neg->retryTime) {
            if (neg->retryCount > neg->maxRetryCount) {
                gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                              GSIDebugLevel_Notice, "REPORT retry FAILED...\n");
                neg->completedCallback(neg->result, neg->connectedSocket,
                                       &neg->remoteaddr, neg->userdata);
                if (neg->gameSock == INVALID_SOCKET) {
                    neg->negotiateSock = INVALID_SOCKET;
                }
                NNCancel(neg->cookie);
            } else {
                SendReportPacket(neg);
                neg->retryCount++;
                neg->retryTime = current_time() + REPORT_RETRY_TIME;
            }
        }
    }
}

void NNThink() {
    int i;

    if (negotiateList == NULL || ArrayLength(negotiateList) == 0) {
        NegotiateThink(NULL);
    } else {
        for (i = ArrayLength(negotiateList) - 1; i >= 0; i--) {
            NegotiateThink((NATNegotiator *)ArrayNth(negotiateList, i));
        }
    }
}

static void SendConnectAck(NATNegotiator *neg, SOSockAddrIn *toaddr) {
    NatNegPacket p;

    gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                  GSIDebugLevel_Comment, "Sending connect ack...\n");

    memcpy(p.magic, NNMagicData, NATNEG_MAGIC_LEN);
    p.version = NN_PROTVER;
    p.packettype = NN_CONNECT_ACK;
    p.cookie = htonl(neg->cookie);
    p.Packet.Init.clientindex = (unsigned char)neg->clientindex;
    SendPacket(neg->negotiateSock, toaddr->addr.addr, ntohs(toaddr->port), &p,
               INITPACKET_SIZE);
}

static void ProcessConnectPacket(NATNegotiator *neg, NatNegPacket *p,
                                 SOSockAddrIn *fromaddr) {
    NegotiateResult errcode;

    gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                  GSIDebugLevel_Comment,
                  "Got connect packet (finish code: %d), guess: %s:%d\n",
                  p->Packet.Connect.finished,
                  inet_ntoa(*(IN_ADDR *)&p->Packet.Connect.remoteIP),
                  ntohs(p->Packet.Connect.remotePort));

    if (!p->Packet.Connect.finished) {
        SendConnectAck(neg, fromaddr);
    }

    if (neg->state < ns_connectping) {
        if (p->Packet.Connect.finished) {
            errcode = nr_unknownerror;
            if (p->Packet.Connect.finished == NN_CONNECT_DEADBEAT) {
                errcode = nr_deadbeatpartner;
            } else if (p->Packet.Connect.finished == NN_CONNECT_INITTIMEOUT) {
                errcode = nr_inittimeout;
            }
            StartReport(neg, errcode, INVALID_SOCKET, NULL);
        } else {
            neg->guessedIP = p->Packet.Connect.remoteIP;
            neg->guessedPort = ntohs(p->Packet.Connect.remotePort);
            neg->retryCount = 0;
            neg->state = ns_connectping;
            neg->progressCallback(ns_connectping, neg->userdata);
            SendPingPacket(neg);
        }
    }
}

static void ProcessPingPacket(NATNegotiator *neg, NatNegPacket *p,
                              SOSockAddrIn *fromaddr) {
    if (neg->state >= ns_connectping) {
        gsDebugFormat(
            GSIDebugCat_NatNeg, GSIDebugType_Network, GSIDebugLevel_Comment,
            "Got ping from: %s:%d (gotmydata: %d, finished: %d)\n",
            inet_ntoa(fromaddr->addr), ntohs(fromaddr->port),
            p->Packet.Connect.gotyourdata, p->Packet.Connect.finished);

        neg->guessedIP = fromaddr->addr.addr;
        neg->guessedPort = ntohs(fromaddr->port);
        neg->gotRemoteData = 1;

        if (!p->Packet.Connect.gotyourdata) {
            SendPingPacket(neg);
        } else if (neg->state == ns_connectping) {
            if (!neg->sendGotRemoteData) {
                SendPingPacket(neg);
            }
            gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                          GSIDebugLevel_Comment, "CONNECT FINISHED\n");
            neg->state = ns_finished;
            neg->retryTime = current_time() + FINISH_WAIT_TIME;
        } else if (!p->Packet.Connect.finished) {
            SendPingPacket(neg);
        }
    }
}

static void ProcessInitPacket(NATNegotiator *neg, NatNegPacket *p,
                              SOSockAddrIn *fromaddr) {
    switch (p->packettype) {
    case NN_INITACK:
        if (p->Packet.Init.porttype <= NN_PT_NN3) {
            gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                          GSIDebugLevel_Comment, "Got init ack for port %d\n",
                          p->Packet.Init.porttype);
            neg->initAckRecv[p->Packet.Init.porttype] = 1;
            if (neg->state == ns_initsent && neg->initAckRecv[NN_PT_NN1] &&
                neg->initAckRecv[NN_PT_NN2] && neg->initAckRecv[NN_PT_NN3] &&
                (neg->gameSock == INVALID_SOCKET ||
                 neg->initAckRecv[NN_PT_GP])) {
                neg->state = ns_initack;
                neg->retryTime = current_time() + INITACK_WAIT_TIME;
                neg->progressCallback(neg->state, neg->userdata);
            }
        }
        break;
    case NN_ERTTEST:
        gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                      GSIDebugLevel_Comment, "Got ERT\n");
        p->packettype = NN_ERTACK;
        SendPacket(neg->negotiateSock, fromaddr->addr.addr,
                   ntohs(fromaddr->port), p, INITPACKET_SIZE);
        break;
    case NN_REPORT_ACK:
        gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                      GSIDebugLevel_Comment, "Got REPORT ACK\n");
        neg->state = ns_reportack;
        neg->completedCallback(neg->result, neg->connectedSocket,
                               &neg->remoteaddr, neg->userdata);
        if (neg->gameSock == INVALID_SOCKET) {
            neg->negotiateSock = INVALID_SOCKET;
        }
        NNCancel(neg->cookie);
        break;
    }
}

void NNProcessData(char *data, int len, SOSockAddrIn *fromaddr) {
    NatNegPacket p;
    NATNegotiator *neg;
    unsigned char ptype;

    if (!CheckMagic(data)) {
        return;
    }

    ptype = ((NatNegPacket *)data)->packettype;
    gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Network,
                  GSIDebugLevel_Comment,
                  "Process data, packet type: %d, %d bytes (%s:%d)\n", ptype,
                  len, inet_ntoa(fromaddr->addr), ntohs(fromaddr->port));

    if (ptype == NN_CONNECT || ptype == NN_CONNECT_PING) {
        if (len < CONNECTPACKET_SIZE) {
            return;
        }

        memcpy(&p, data, CONNECTPACKET_SIZE);
        neg = FindNegotiatorForCookie(ntohl(p.cookie));
        if (neg == NULL) {
            return;
        }

        if (ptype == NN_CONNECT) {
            ProcessConnectPacket(neg, &p, fromaddr);
        } else {
            ProcessPingPacket(neg, &p, fromaddr);
        }
    } else {
        if (len < INITPACKET_SIZE) {
            return;
        }

        memcpy(&p, data, INITPACKET_SIZE);
        neg = FindNegotiatorForCookie(ntohl(p.cookie));
        if (neg == NULL) {
            return;
        }

        ProcessInitPacket(neg, &p, fromaddr);
    }
}
