#ifndef REVOLUTION_SO_H
#define REVOLUTION_SO_H

#include <revolution/types.h>
#include <revolution/ncd/NCDTypes.h>

#include <revolution/os/OSTime.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*SO_AllocFunc)(u32, s32);
typedef void (*SO_FreeFunc)(u32, void*, s32);

typedef SO_AllocFunc SOAlloc;
typedef SO_FreeFunc SOFree;

typedef struct SOLibraryConfig {
  SOAlloc alloc;
  SOFree free;
} SOLibraryConfig;

typedef struct SOSysWork {
  SO_AllocFunc allocFunc; // 0x00
  SO_FreeFunc freeFunc;   // 0x04
  s32 rmState;            // 0x08
  s32 rmFd;               // 0x0C
  u32 _unk10;             // 0x10
  s32 allocCount;         // 0x14
} SOSysWork;

typedef struct NETSoSocket {
  int af;       // 0x00
  int type;     // 0x04
  int protocol; // 0x08
} NETSoSocket;

enum {
  SO_INTERNAL_STATE_TERMINATED = 0,
  SO_INTERNAL_STATE_READY = 1,
  SO_INTERNAL_STATE_ACTIVE = 2
};

enum {
  SO_INTERNAL_RM_STATE_OPENED = 0,
  SO_INTERNAL_RM_STATE_WORKING = -1,
  SO_INTERNAL_RM_STATE_CLOSED = -2
};

enum { SO_ERR_LINK_UP_TIMEOUT = -121 };

enum {
  NET_SO_SOCKET = 0xf,
};

enum {
  SO_PF_INET6 = 0x17,
};

#define SO_SOL_SOCKET 0xffff
#define SO_SOL_IP 0
#define SO_SOL_ICMP 1
#define SO_SOL_TCP 6
#define SO_SOL_UDP 17

#define SO_SOL_CONFIG 0xfffe

#define SO_SO_SNDBUF 0x1001
#define SO_SO_RCVBUF 0x1002

#define SO_IPPROTO_ICMP 1
#define SO_IPPROTO_IGMP 2
#define SO_IPPROTO_TCP 6
#define SO_IPPROTO_UDP 17

#define SO_CONFIG_IP_ADDR_NUMBER 0x4002
#define SO_CONFIG_IP_ADDR_TABLE 0x4003

#define SO_F_GETFL 3
#define SO_F_SETFL 4
#define SO_O_NONBLOCK 4

#define SO_CONFIG_FILTER_INPUT 0x1001
#define SO_CONFIG_FILTER_OUTPUT 0x1002

#define SO_CONFIG_ERROR 0x1003
#define SO_CONFIG_MAC_ADDRESS 0x1004
#define SO_CONFIG_LINK_STATE 0x1005
#define SO_CONFIG_INTERFACE_STATISTICS 0x1006
#define SO_CONFIG_MUTE 0x1007

typedef struct IPInterface {
  s32 type;
  int up;
  s32 err;
} IPInterface;

typedef struct SOHostEnt {
  char* name;
  char** aliases;
  s16 addrType;
  s16 length;
  u8** addrList;
} SOHostEnt;

typedef struct SOAddrInfo SOAddrInfo;
struct SOAddrInfo {
  int flags;
  int family;
  int sockType;
  int protocol;
  unsigned addrLen;
  char* canonName;
  void* addr;
  SOAddrInfo* next;
};

extern const char* __SO_VERSION;
extern const char* __SOCKET_VERSION;

int SOInit(const SOLibraryConfig* config);
int SOFinish(void);
int SOStartup(void);
int SOiStartupCore(int timeout);
int SOCleanup(void);
SOSysWork* SOiGetSysWork(void);
int SOiIsBufferAddrCheck(void);
int SOiIsInitialized(void);
void* SOiAlloc(u32, s32);
void SOiFree(u32, void*, s32);
int SOiPrepare(const char*, s32*);
int SOiConclude(const char*, int);
int SOiPrepareTempRm(const char*, s32*, int*);
int SOiConcludeTempRm(const char*, int, int);
int SOiWaitForDHCPEx(int timeout);

int __SOCreateSocket(int pf, int type, int protocol);

SOHostEnt* SOGetHostByName(const char* name);
SOHostEnt* SOGetHostByAddr(const void* addr, int len, int type);
int SOGetNameInfo(const void* sockAddr, char* node, unsigned nodeLen,
                  char* service, unsigned serviceLen, int flags);
int SOGetAddrInfo(const char* nodeName, const char* servName,
                  const SOAddrInfo* hints, SOAddrInfo** res);
void SOFreeAddrInfo(SOAddrInfo* head);

typedef struct SOInAddr {
  u32 addr;

} SOInAddr;

typedef struct IPAddrEntry {
  u8 addr[4];
  u8 netMask[4];
  u8 bcastAddr[4];
} IPAddrEntry;

typedef struct SOSockAddr {
  u8 len;
  u8 family;
  u8 data[6];

} SOSockAddr;

typedef struct SOSockAddrIn {
  u8 len;
  u8 family;
  u16 port;
  SOInAddr addr;
} SOSockAddrIn;

typedef struct SOPollFD {
  int fd;
  int events;
  int revents;
} SOPollFD;

int SOInetAtoN(const char* cp, SOInAddr* inp);
char* SOInetNtoA(SOInAddr in);
int SOInetPtoN(int af, const char* src, void* dst);
const char* SOInetNtoP(int af, const void* src, char* dst, unsigned len);
u32 SONtoHl(u32 netlong);
u16 SONtoHs(u16 netshort);
u32 SOHtoNl(u32 hostlong);
u16 SOHtoNs(u16 hostshort);

int SOSocket(int pf, int type, int protocol);
int SOListen(int s, int backlog);
int SOAccept(int s, void* sockAddr);
int SOBind(int s, const void* sockAddr);
int SOConnect(int s, const void* sockAddr);
int SOGetSockName(int s, void* sockAddr);
int SOGetPeerName(int s, void* sockAddr);
int SORecvFrom(int s, void* buf, int len, int flags, void* sockFrom);
int SORecv(int s, void* buf, int len, int flags);
int SORead(int s, void* buf, int len);
int SOSendTo(int s, const void* buf, int len, int flags, const void* sockTo);
int SOSend(int s, const void* buf, int len, int flags);
int SOWrite(int s, const void* buf, int len);
int SOFcntl(int s, int cmd, ...);
int SOShutdown(int s, int how);
int SOPoll(SOPollFD fds[], unsigned nfds, OSTime timeout);
int SOSockAtMark(int s);
int SOClose(int s);
long SOGetHostID(void);

int SOGetSockOpt(int s, int level, int optname, void* optval, int* optlen);
int SOSetSockOpt(int s, int level, int optname, const void* optval, int optlen);
int SOGetInterfaceOpt(IPInterface* interface, int level, int optname,
                      void* optval, int* optlen);
int SOSetInterfaceOpt(IPInterface* interface, int level, int optname,
                      const void* optval, int optlen);

#define SO_POLLRDNORM 0x0001
#define SO_POLLRDBAND 0x0002
#define SO_POLLPRI 0x0004
#define SO_POLLWRNORM 0x0008
#define SO_POLLWRBAND 0x0010
#define SO_POLLERR 0x0020
#define SO_POLLHUP 0x0040
#define SO_POLLNVAL 0x0080
#define SO_POLLIN (SO_POLLRDNORM | SO_POLLRDBAND)
#define SO_POLLOUT SO_POLLWRNORM

#define SO_INFTIM (-1)

#define SO_PF_INET 2
#define SO_SOCK_STREAM 1
#define SO_SOCK_DGRAM 2

enum SOError {
  SO_SUCCESS = 0,
  SO_EFATAL = (int)0x80000000,
  SO_E2BIG = -1,
  SO_EACCES = -2,
  SO_EADDRINUSE = -3,
  SO_EADDRNOTAVAIL = -4,
  SO_EAFNOSUPPORT = -5,
  SO_EAGAIN = -6,
  SO_EALREADY = -7,
  SO_EBADF = -8,
  SO_EBADMSG = -9,
  SO_EBUSY = -10,
  SO_ECANCELED = -11,
  SO_ECHILD = -12,
  SO_ECONNABORTED = -13,
  SO_ECONNREFUSED = -14,
  SO_ECONNRESET = -15,
  SO_EDEADLK = -16,
  SO_EDESTADDRREQ = -17,
  SO_EDOM = -18,
  SO_EDQUOT = -19,
  SO_EEXIST = -20,
  SO_EFAULT = -21,
  SO_EFBIG = -22,
  SO_EHOSTUNREACH = -23,
  SO_EIDRM = -24,
  SO_EILSEQ = -25,
  SO_EINPROGRESS = -26,
  SO_EINTR = -27,
  SO_EINVAL = -28,
  SO_EIO = -29,
  SO_EISCONN = -30,
  SO_EISDIR = -31,
  SO_ELOOP = -32,
  SO_EMFILE = -33,
  SO_EMLINK = -34,
  SO_EMSGSIZE = -35,
  SO_EMULTIHOP = -36,
  SO_ENAMETOOLONG = -37,
  SO_ENETDOWN = -38,
  SO_ENETRESET = -39,
  SO_ENETUNREACH = -40,
  SO_ENFILE = -41,
  SO_ENOBUFS = -42,
  SO_ENODATA = -43,
  SO_ENODEV = -44,
  SO_ENOENT = -45,
  SO_ENOEXEC = -46,
  SO_ENOLCK = -47,
  SO_ENOLINK = -48,
  SO_ENOMEM = -49,
  SO_ENOMSG = -50,
  SO_ENOPROTOOPT = -51,
  SO_ENOSPC = -52,
  SO_ENOSR = -53,
  SO_ENOSTR = -54,
  SO_ENOSYS = -55,
  SO_ENOTCONN = -56,
  SO_ENOTDIR = -57,
  SO_ENOTEMPTY = -58,
  SO_ENOTSOCK = -59,
  SO_ENOTSUP = -60,
  SO_ENOTTY = -61,
  SO_ENXIO = -62,
  SO_EOPNOTSUPP = -63,
  SO_EOVERFLOW = -64,
  SO_EPERM = -65,
  SO_EPIPE = -66,
  SO_EPROTO = -67,
  SO_EPROTONOSUPPORT = -68,
  SO_EPROTOTYPE = -69,
  SO_ERANGE = -70,
  SO_EROFS = -71,
  SO_ESPIPE = -72,
  SO_ESRCH = -73,
  SO_ESTALE = -74,
  SO_ETIME = -75,
  SO_ETIMEDOUT = -76,
  SO_ETXTBSY = -77,
  SO_EWOULDBLOCK = SO_EAGAIN,
  SO_EXDEV = -78
};

#define SO_INADDR_ANY ((u32)0x00000000)

#define SO_BROADCAST 9

#ifdef __cplusplus
}
#endif

#endif // REVOLUTION_SO_H
