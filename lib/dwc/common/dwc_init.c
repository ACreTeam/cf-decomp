#include <dwc/dwc_init.h>

#include <dwc/dwc_auth_interface.h>
#include <dwc/dwc_cfriend.h>
#include <dwc/dwc_nonport.h>
#include <dwc/dwc_report.h>
#include <dwc/dwci_memfunc.h>
#include <gamespy/common/gsMemory.h>
#include <gamespy/gstats/gstats.h>
#include <revolution/os.h>
#include <string.h>

static u32 dwci_gamecode = 0;

#define DWC_SDKDEV_SERVER_HOSTNAME "sdkdev.gamespy.com"
#define DWC_STATS_SERVER_HOSTNAME "gamestats.gs.nintendowifi.net"

int DWC_Init(DWC_AuthServer authserver, const char* gameName, u32 gameCode,
             DWCAllocEx alloc, DWCFreeEx free) {
  const char* __DWCVersion =
      "<< RVL_SDK - DWC \trelease build: Sep 17 2008 18:07:38 (0x4199_60831) >>";

  OSRegisterVersion(__DWCVersion);

  DWCi_SetMemFunc(alloc, free);
  if (!DWCi_Auth_InitInterface(authserver)) {
    DWC_Printf(DWC_REPORTFLAG_WARNING, "Failed to initialize auth interface.\n");
    return -1;
  }
  dwci_gamecode = gameCode;

  gsiMemoryCallbacksSet(&DWCi_GsMalloc, &DWCi_GsFree, &DWCi_GsRealloc,
                        &DWCi_GsMemalign);

  strcpy(gcd_gamename, gameName);

  if (authserver == DWC_AUTHSERVER_DEBUG) {
    strcpy(StatsServerHostname, DWC_SDKDEV_SERVER_HOSTNAME);
  } else {
    strcpy(StatsServerHostname, DWC_STATS_SERVER_HOSTNAME);
  }

  DWCi_Np_SetInitFlag(TRUE);
  DWCi_Np_GetConsoleId();

  return 0;
}

void DWC_Shutdown() {
  gsiMemoryCallbacksSet(&DWCi_GsMalloc, &DWCi_GsFree, &DWCi_GsRealloc,
                        &DWCi_GsMemalign);
  gethostbyname("clear");
  DWCi_CfReset();
  DWCi_Np_SetInitFlag(FALSE);
}

u32 DWCi_GetGamecode() {
  return dwci_gamecode;
}
