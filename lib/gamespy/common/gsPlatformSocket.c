///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>

// include the revolution socket header
#include "gsPlatform.h"
#include "gsPlatformSocket.h"
#include "gsPlatformUtil.h"
#include "gsMemory.h"

#include <rvl/so/so.h>

#define SO_F_GETFL 3
#define SO_F_SETFL 4
#define SO_O_NONBLOCK 4

#define DNS_CACHE_SIZE 31
#define DNS_PROBE_COUNT 15

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// static variables
static int GSIRevolutionErrno;

// prototypes of static functions
static int CheckRcode(int rcode, int errCode);

#define REVOlUTION_SOCKET_ERROR -1
#define SOCKET int

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// parses rcode into a generic -1 error if an error has occured
static int CheckRcode(int rcode, int errCode) {
  if (rcode >= 0)
    return rcode;
  GSIRevolutionErrno = rcode;
  return errCode;
}

int socket(int pf, int type, int protocol) {
  int rcode = SOSocket(pf, type, 0);
  GSI_UNUSED(protocol);
  return CheckRcode(rcode, INVALID_SOCKET);
}
int closesocket(SOCKET sock) {
  int rcode = SOClose(sock);
  return CheckRcode(rcode, REVOlUTION_SOCKET_ERROR);
}
int shutdown(SOCKET sock, int how) {
  int rcode = SOShutdown(sock, how);
  return CheckRcode(rcode, REVOlUTION_SOCKET_ERROR);
}
int bind(SOCKET sock, const SOCKADDR* addr, int len) {
  SOCKADDR localAddr;
  int rcode;

  // with Revolution, don't bind to 0, just start using the port
  if (((const SOCKADDR_IN*)addr)->port == 0)
    return 0;

  memcpy(&localAddr, addr, sizeof(SOCKADDR));
  localAddr.len = (u8)len;

  rcode = SOBind(sock, &localAddr);
  return CheckRcode(rcode, REVOlUTION_SOCKET_ERROR);
}

int connect(SOCKET sock, const SOCKADDR* addr, int len) {
  SOCKADDR remoteAddr;
  int rcode;

  memcpy(&remoteAddr, addr, sizeof(SOCKADDR));
  remoteAddr.len = (u8)len;

  rcode = SOConnect(sock, &remoteAddr);
  return CheckRcode(rcode, REVOlUTION_SOCKET_ERROR);
}
/*
int listen(SOCKET sock, int backlog)
{
        int rcode = SOListen(sock, backlog);
        return CheckRcode(rcode, REVOlUTION_SOCKET_ERROR);
}
SOCKET accept(SOCKET sock, SOCKADDR* addr, int* len)
{
        int rcode;
        addr->len = (u8)*len;
        rcode = SOAccept(sock, addr);
        *len = addr->len;
        return CheckRcode(rcode, REVOlUTION_SOCKET_ERROR);
}
*/

int recv(SOCKET sock, char* buf, int len, int flags) {
  // TODO flags hardcoded to 4 here.
  int rcode = SORecv(sock, buf, len, 4);
  return CheckRcode(rcode, REVOlUTION_SOCKET_ERROR);
}
int recvfrom(SOCKET sock, char* buf, int len, int flags, SOCKADDR* addr,
             int* fromlen) {
  int rcode;
  addr->len = (u8)*fromlen;
  // TODO flags hardcoded to 4 here.
  rcode = SORecvFrom(sock, buf, len, 4, addr);
  *fromlen = addr->len;
  return CheckRcode(rcode, REVOlUTION_SOCKET_ERROR);
}
SOCKET send(SOCKET sock, const char* buf, int len, int flags) {
  int rcode = SOSend(sock, buf, len, flags);
  return CheckRcode(rcode, REVOlUTION_SOCKET_ERROR);
}
SOCKET sendto(SOCKET sock, const char* buf, int len, int flags,
              const SOCKADDR* addr, int tolen) {
  SOCKADDR remoteAddr;
  int rcode;

  memcpy(&remoteAddr, addr, sizeof(SOCKADDR));
  remoteAddr.len = (u8)tolen;

  rcode = SOSendTo(sock, buf, len, flags, &remoteAddr);
  return CheckRcode(rcode, REVOlUTION_SOCKET_ERROR);
}
/*
int getsockopt(SOCKET sock, int level, int optname, char* optval, int* optlen)
{
        int rcode = SOGetSockOpt(sock, level, optname, optval, optlen);
        return CheckRcode(rcode, REVOlUTION_SOCKET_ERROR);
}
*/
int getsockname(SOCKET sock, SOCKADDR* addr, int* len) {
  int rcode;
  addr->len = (u8)*len;
  rcode = SOGetSockName(sock, addr);
  *len = addr->len;
  return CheckRcode(rcode, REVOlUTION_SOCKET_ERROR);
}

unsigned long inet_addr(const char* name) {
  int rcode;
  SOInAddr addr;
  rcode = SOInetAtoN(name, &addr);
  if (rcode == false)
    return INADDR_NONE;
  return addr.addr;
}

int GOAGetLastError(SOCKET sock) {
  GSI_UNUSED(sock);
  return GSIRevolutionErrno;
}

int GSISocketSelect(SOCKET theSocket, int* theReadFlag, int* theWriteFlag,
                    int* theExceptFlag) {
  SOPollFD pollFD;
  int rcode;

  pollFD.fd = theSocket;
  pollFD.events = 0;
  if (theReadFlag != NULL)
    pollFD.events |= SO_POLLRDNORM;
  if (theWriteFlag != NULL)
    pollFD.events |= SO_POLLWRNORM;
  pollFD.revents = 0;

  rcode = SOPoll(&pollFD, 1, 0);
  if (rcode < 0)
    return REVOlUTION_SOCKET_ERROR;

  if (theReadFlag != NULL) {
    if ((rcode > 0) && (pollFD.revents & (SO_POLLRDNORM | SO_POLLHUP)))
      *theReadFlag = 1;
    else
      *theReadFlag = 0;
  }
  if (theWriteFlag != NULL) {
    if ((rcode > 0) && (pollFD.revents & SO_POLLWRNORM))
      *theWriteFlag = 1;
    else
      *theWriteFlag = 0;
  }
  if (theExceptFlag != NULL) {
    if ((rcode > 0) && (pollFD.revents & SO_POLLERR))
      *theExceptFlag = 1;
    else
      *theExceptFlag = 0;
  }
  return rcode;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct tagDNSHashMap {
  SOHostEnt entry;
  char* name;
} DNSHashMap;

HOSTENT* gethostbyname(const char* name) {
  static DNSHashMap** dnscache;
  unsigned long hash;
  unsigned long i;
  unsigned long len;
  HOSTENT* hostentry;
  int j;
  DNSHashMap* data;
  unsigned long newhash;

  // "clear" flushes the cache
  if (strncmp("clear", name, strlen("clear")) == 0) {
    if (dnscache != NULL) {
      for (i = 0; i < DNS_CACHE_SIZE; i++) {
        if (dnscache[i] != NULL) {
          for (j = 0; dnscache[i]->entry.addrList[j] != NULL; j++)
            gsifree(dnscache[i]->entry.addrList[j]);
          gsifree(dnscache[i]->entry.addrList[j]);
          gsifree(dnscache[i]->entry.addrList);
          gsifree(dnscache[i]->name);
        }
        gsifree(dnscache[i]);
      }
      gsifree(dnscache);
      dnscache = NULL;
    }
    return NULL;
  }

  if (dnscache == NULL) {
    dnscache = (DNSHashMap**)gsimalloc(sizeof(DNSHashMap*) * DNS_CACHE_SIZE);
    memset(dnscache, 0, sizeof(DNSHashMap*) * DNS_CACHE_SIZE);
  }

  len = strlen(name);
  hash = 0;
  for (i = 0; i < len; i++)
    hash += name[i] << ((i % 8) * 4);

  hash %= DNS_CACHE_SIZE;
  for (i = 0; i < DNS_PROBE_COUNT; i++) {
    newhash = (hash + i * i) % DNS_CACHE_SIZE;
    data = dnscache[newhash];
    if (data != NULL && strcmp(data->name, name) == 0)
      return &data->entry;
  }

  if (dnscache[hash] != NULL) {
    for (i = 1; i < DNS_PROBE_COUNT; i++) {
      newhash = (hash + i * i) % DNS_CACHE_SIZE;
      if (dnscache[newhash] == NULL) {
        hash = newhash;
        break;
      }
    }
    if (i == DNS_PROBE_COUNT)
      return SOGetHostByName(name);
  }

  hostentry = SOGetHostByName(name);
  if (hostentry == NULL)
    return NULL;

  dnscache[hash] = (DNSHashMap*)gsimalloc(sizeof(DNSHashMap));
  for (i = 0; hostentry->addrList[i] != NULL; i++) {
  }
  dnscache[hash]->entry.addrType = SO_PF_INET;
  dnscache[hash]->entry.length = hostentry->length;
  dnscache[hash]->entry.name = NULL;
  dnscache[hash]->entry.aliases = NULL;
  dnscache[hash]->entry.addrList =
      (unsigned char**)gsimalloc((i + 1) * sizeof(unsigned char*));
  for (j = 0; hostentry->addrList[j] != NULL; j++) {
    dnscache[hash]->entry.addrList[j] =
        (unsigned char*)gsimalloc(hostentry->length);
    memcpy(dnscache[hash]->entry.addrList[j], hostentry->addrList[j],
           hostentry->length);
  }
  dnscache[hash]->entry.addrList[j] = NULL;
  dnscache[hash]->name = (char*)gsimalloc(strlen(name) + 1);
  strcpy(dnscache[hash]->name, name);

  return hostentry;
}

int SetSockBlocking(SOCKET sock, int isblocking) {
  int rcode;
  int val;

  val = SOFcntl(sock, SO_F_GETFL, 0);
  if (isblocking)
    val &= ~SO_O_NONBLOCK;
  else
    val |= SO_O_NONBLOCK;
  rcode = SOFcntl(sock, SO_F_SETFL, val);
  if (rcode == 0)
    return 1;
  return 0;
}

int CanReceiveOnSocket(SOCKET sock) {
  int aReadFlag = 0;
  if (GSISocketSelect(sock, &aReadFlag, NULL, NULL) == 1)
    return aReadFlag;
  return 0;
}

int CanSendOnSocket(SOCKET sock) {
  int aWriteFlag = 0;
  if (GSISocketSelect(sock, NULL, &aWriteFlag, NULL) == 1)
    return aWriteFlag;
  return 0;
}

typedef struct IPAddrEntry {
  unsigned char addr[4];
  unsigned char netMask[4];
  unsigned char bcastAddr[4];
} IPAddrEntry;

HOSTENT* getlocalhost(void) {
  static SOHostEnt aLocalHost;
  static char* aliases;
  static unsigned char* ips[16];
  static IPAddrEntry aAddrs[16];
  int aNumOfIps;
  int i;
  int aSizeNumOfIps;
  int aSizeAddrList;

  aSizeNumOfIps = sizeof(aNumOfIps);
  SOGetInterfaceOpt(NULL, SO_SOL_CONFIG, 0x4002, &aNumOfIps, &aSizeNumOfIps);
  memset(aAddrs, 0xBE, aNumOfIps * sizeof(IPAddrEntry));
  aSizeAddrList = aNumOfIps * sizeof(IPAddrEntry);
  SOGetInterfaceOpt(NULL, SO_SOL_CONFIG, 0x4003, aAddrs, &aSizeAddrList);

  aLocalHost.name = "localhost";
  aLocalHost.aliases = &aliases;
  aLocalHost.addrType = SO_PF_INET;
  aLocalHost.length = aNumOfIps;
  for (i = 0; i < aNumOfIps; i++)
    ips[i] = (unsigned char*)&aAddrs[i];
  ips[i] = NULL;
  aLocalHost.addrList = ips;
  return &aLocalHost;
}

int IsPrivateIP(IN_ADDR* addr) {
  int b1;
  int b2;
  unsigned int ip;

  ip = ntohl(addr->addr);
  b1 = (ip >> 24) & 0xFF;
  b2 = (ip >> 16) & 0xFF;
  if (b1 == 10)
    return 1;
  if (b1 == 172 && b2 >= 16 && b2 <= 31)
    return 1;
  if (b1 == 192 && b2 == 168)
    return 1;
  return 0;
}
