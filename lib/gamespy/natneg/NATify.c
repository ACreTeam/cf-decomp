/******
GameSpy NAT Negotiation SDK - NAT detection (NATify)
******/

#include <stdlib.h>
#include <string.h>

#include "../common/gsAvailable.h"
#include "../common/gsCommon.h"
#include "nninternal.h"

// The compiler lays out .sbss in reverse declaration order
int gotERT1;
int gotERT2;
int gotERT3;
int gotADDRESS1a;
int gotADDRESS1b;
int gotADDRESS2;
int gotADDRESS3;

static char strAddressArray[2][22];
static int nIndex;

char *AddressToString(unsigned int ip, unsigned short port, char *string) {
    char *strAddress;
    SOInAddr inAddr;

    if (string) {
        strAddress = string;
    } else {
        nIndex ^= 1;
        strAddress = strAddressArray[nIndex];
    }

    inAddr.addr = ip;
    if (ip) {
        if (port) {
            sprintf(strAddress, "%s:%d", inet_ntoa(inAddr), port);
        } else {
            sprintf(strAddress, "%s", inet_ntoa(inAddr));
        }
    } else if (port) {
        sprintf(strAddress, ":%d", port);
    } else {
        strAddress[0] = '\0';
    }

    return strAddress;
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

void OutputMapping(const AddressMapping *theMap) {
    if (theMap == NULL) {
        gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Misc, GSIDebugLevel_Notice,
                      "ERROR: Port mapping not available.");
    } else {
        gsDebugFormat(GSIDebugCat_NatNeg, GSIDebugType_Misc, GSIDebugLevel_Notice,
                      "Discovered map: [%s] -> [%s]\n",
                      AddressToString(theMap->privateIp, theMap->privatePort, NULL),
                      AddressToString(theMap->publicIp, theMap->publicPort, NULL));
    }
}

static char data[512] POST_ALIGN(16);

static int Think(SOCKET sock, NAT *nat) {
    int length;
    unsigned char ptype;
    NatNegPacket p;
    SOSockAddrIn saddr;
    int saddrlen = sizeof(saddr);

    if (gotERT1 && gotERT2 && gotERT3 && gotADDRESS1a && gotADDRESS1b &&
        gotADDRESS2 && gotADDRESS3) {
        return 0;
    }

    if (sock != INVALID_SOCKET) {
        while (CanReceiveOnSocket(sock)) {
            length = recvfrom(sock, data, sizeof(data), 0, (SOCKADDR *)&saddr,
                              &saddrlen);
            if (gsiSocketIsError(length)) {
                GOAGetLastError(sock);
                break;
            }

            if (memcmp(data, NNMagicData, NATNEG_MAGIC_LEN) != 0) {
                return 1;
            }

            ptype = data[7];
            if (length < INITPACKET_SIZE) {
                return 1;
            }

            if (ptype == NN_ERTTEST) {
                memcpy(&p, data, INITPACKET_SIZE);
                switch (p.Packet.Init.porttype) {
                case NN_PT_NN1:
                    gotERT1 = 1;
                    AddressToString(saddr.addr.addr, ntohs(saddr.port), NULL);
                    break;
                case NN_PT_NN2:
                    nat->ipRestricted = 0;
                    gotERT2 = 1;
                    AddressToString(saddr.addr.addr, ntohs(saddr.port), NULL);
                    break;
                case NN_PT_NN3:
                    nat->portRestricted = 0;
                    gotERT3 = 1;
                    AddressToString(saddr.addr.addr, ntohs(saddr.port), NULL);
                    break;
                }
            } else if (ptype == NN_ADDRESS_REPLY) {
                memcpy(&p, data, INITPACKET_SIZE);
                p.cookie = ntohl(p.cookie);
                switch (p.cookie) {
                case packet_map1a:
                    gotADDRESS1a = 1;
                    break;
                case packet_map1b:
                    gotADDRESS1b = 1;
                    break;
                case packet_map2:
                    gotADDRESS2 = 1;
                    break;
                case packet_map3:
                    gotADDRESS3 = 1;
                    break;
                }

                nat->mappings[p.cookie].privateIp = GetLocalIP();
                nat->mappings[p.cookie].privatePort = GetLocalPort(sock);
                nat->mappings[p.cookie].publicIp = p.Packet.Init.localip;
                nat->mappings[p.cookie].publicPort =
                    ntohs(p.Packet.Init.localport);
                AddressToString(saddr.addr.addr, ntohs(saddr.port), NULL);
                OutputMapping(&nat->mappings[p.cookie]);
            }

            if (sock == INVALID_SOCKET) {
                break;
            }
        }
    }

    return 1;
}

int NatifyThink(SOCKET sock, NAT *nat) {
    return Think(sock, nat);
}

gsi_bool DetermineNatType(NAT *nat) {
    nat->natType = unknown;
    nat->promiscuity = promiscuity_not_applicable;
    nat->qr2Compatible = gsi_true;

    if (!nat->mappings[0].publicIp || !nat->mappings[1].publicIp ||
        !nat->mappings[2].publicIp) {
        return gsi_false;
    }

    if (!nat->portRestricted && !nat->ipRestricted &&
        nat->mappings[0].publicIp == nat->mappings[0].privateIp) {
        nat->natType = no_nat;
    } else if (nat->mappings[0].publicIp == nat->mappings[0].privateIp) {
        nat->natType = firewall_only;
    } else if (!nat->ipRestricted && !nat->portRestricted &&
               abs(nat->mappings[2].publicPort - nat->mappings[1].publicPort) >=
                   1) {
        nat->natType = symmetric;
        nat->promiscuity = promiscuous;
    } else if (nat->ipRestricted && !nat->portRestricted &&
               abs(nat->mappings[2].publicPort - nat->mappings[1].publicPort) >=
                   1) {
        nat->natType = symmetric;
        nat->promiscuity = port_promiscuous;
    } else if (!nat->ipRestricted && nat->portRestricted &&
               abs(nat->mappings[2].publicPort - nat->mappings[1].publicPort) >=
                   1) {
        nat->natType = symmetric;
        nat->promiscuity = ip_promiscuous;
    } else if (nat->ipRestricted && nat->portRestricted &&
               abs(nat->mappings[2].publicPort - nat->mappings[1].publicPort) >=
                   1) {
        nat->natType = symmetric;
        nat->promiscuity = not_promiscuous;
    } else if (nat->portRestricted) {
        nat->natType = port_restricted_cone;
    } else if (nat->ipRestricted && !nat->portRestricted) {
        nat->natType = restricted_cone;
    } else if (!nat->ipRestricted && !nat->portRestricted) {
        nat->natType = full_cone;
    } else {
        nat->natType = unknown;
    }

    if (nat->mappings[0].publicPort == nat->mappings[0].privatePort &&
        nat->mappings[1].publicPort == nat->mappings[1].privatePort &&
        nat->mappings[2].publicPort == nat->mappings[2].privatePort) {
        nat->mappingScheme = private_as_public;
    } else if (nat->mappings[0].publicPort == nat->mappings[1].publicPort &&
               nat->mappings[1].publicPort == nat->mappings[2].publicPort) {
        nat->mappingScheme = consistent_port;
    } else if (nat->mappings[0].publicPort == nat->mappings[0].privatePort &&
               nat->mappings[2].publicPort - nat->mappings[1].publicPort == 1) {
        nat->mappingScheme = mixed;
    } else if (nat->mappings[2].publicPort - nat->mappings[1].publicPort == 1) {
        nat->mappingScheme = incremental;
    } else {
        nat->mappingScheme = unrecognized;
    }

    if (nat->mappings[3].publicPort &&
        nat->mappings[0].publicPort != nat->mappings[3].publicPort) {
        nat->qr2Compatible = gsi_false;
    }

    return gsi_true;
}
