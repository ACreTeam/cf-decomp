#include <dwc/dwc_init.h>

#include <dwc/dwc_auth_interface.h>
#include <dwc/dwc_nonport.h>
#include <dwc/dwc_report.h>
#include <dwc/dwci_memfunc.h>
#include <gamespy/common/gsMemory.h>
#include <gamespy/common/gsPlatformSocket.h>
#include <gamespy/gstats/gstats.h>
#include <revolution/os.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static u32 dwci_gamecode = 0;

#define DWC_VERSION_STRING \
  "<< RVL_SDK - DWC \trelease build: May 11 2007 13:30:47 (0x4199_60831) >>"

#define DWC_SDKDEV_SERVER_HOSTNAME "sdkdev.gamespy.com"

#define MULTIPLIER -1664117991
static int StringHash(const char* s, int numbuckets) {
  u32 hashcode = 0;
  while (*s != 0) {
    hashcode = (u32)((int)hashcode * MULTIPLIER + tolower(*s));
    s++;
  }
  return (int)(hashcode % numbuckets);
}

int DWC_Init(DWC_AuthServer authSvr, const char* gameName, u32 gameCode,
             DWCAllocEx allocator, DWCFreeEx freeer) {
  char hostname[256];

  if (strcmp("RVLDWC", "RVLDWC") == 0) {
    OSRegisterVersion(DWC_VERSION_STRING);
  } else if (strcmp("RVLDWC", "RVLDWC_DL") == 0) {
    OSRegisterVersion(
        "<< RVL_SDK - DWCDL \trelease build: May 11 2007 13:30:47 (0x4199_60831) >>");
  } else {
    OSRegisterVersion(DWC_VERSION_STRING);
  }

  DWCi_SetMemFunc(allocator, freeer);
  DWCi_Auth_InitInterface(authSvr);
  dwci_gamecode = gameCode;
  DWCi_Np_GetConsoleId();

  gsiMemoryCallbacksSet(&DWCi_GsMalloc, &DWCi_GsFree, &DWCi_GsRealloc,
                        &DWCi_GsMemalign);

  strcpy(gcd_gamename, gameName);

  if (authSvr == DWC_SVR_DEV) {
    strcpy(StatsServerHostname, DWC_SDKDEV_SERVER_HOSTNAME);
  }

  if (gethostbyname("gpcm.gs.nintendowifi.net") == NULL) goto error;
  if (gethostbyname("gpsp.gs.nintendowifi.net") == NULL) goto error;
  if (gethostbyname("gamestats.gs.nintendowifi.net") == NULL) goto error;
  if (gethostbyname("gamestats2.gs.nintendowifi.net") == NULL) goto error;

  if (gameName) {
    sprintf(hostname, "%s.available.gs.nintendowifi.net", gameName);
    if (gethostbyname(hostname) == NULL) goto error;
    sprintf(hostname, "%s.natneg1.gs.nintendowifi.net", gameName);
    if (gethostbyname(hostname) == NULL) goto error;
    sprintf(hostname, "%s.natneg2.gs.nintendowifi.net", gameName);
    if (gethostbyname(hostname) == NULL) goto error;
    sprintf(hostname, "%s.natneg3.gs.nintendowifi.net", gameName);
    if (gethostbyname(hostname) == NULL) goto error;
    sprintf(hostname, "%s.master.gs.nintendowifi.net", gameName);
    if (gethostbyname(hostname) == NULL) goto error;
    sprintf(hostname, "%s.gamestats.gs.nintendowifi.net", gameName);
    if (gethostbyname(hostname) == NULL) goto error;
    sprintf(hostname, "%s.gamestats2.gs.nintendowifi.net", gameName);
    if (gethostbyname(hostname) == NULL) goto error;
    sprintf(hostname, "%s.ms%d.gs.nintendowifi.net", gameName,
            StringHash(gameName, 20));
    if (gethostbyname(hostname) == NULL) goto error;
  }

  return 0;

error:
  DWC_Printf(8, "Failed to cache DNS query.\n");
  return -1;
}

void DWC_Shutdown() {
  gsiMemoryCallbacksSet(&DWCi_GsMalloc, &DWCi_GsFree, &DWCi_GsRealloc,
                        &DWCi_GsMemalign);
  gethostbyname("clear");
}

u32 DWCi_GetGamecode() {
  return dwci_gamecode;
}
