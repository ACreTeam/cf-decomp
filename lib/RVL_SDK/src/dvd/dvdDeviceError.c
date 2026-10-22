#include <private/dvd.h>
#include <private/os.h>
#include <revolution/dvd.h>
#include <revolution/os.h>
#include <revolution/sc.h>

static u8 CheckBuffer[32] __attribute__ ((aligned(32)));

static volatile BOOL lowDone = TRUE;
static volatile u32 lowIntType = 0;

static void lowCallback(u32 intType) {
    lowIntType = intType;
    lowDone = TRUE;
}

void __DVDShowDeviceErrorMessage(void);

BOOL __DVDCheckDevice(void) {
    u32 checkCode = 0x460A0000;
    u32 outOfRangeError = 0xFFFFFFFF;
    u32 reportKeyError = 0xFFFFFFFF;
    OSIOSRev iosRev;

    if (OSGetPhysicalMem2Size() == 0x08000000) {
        return TRUE;
    }

    __OSGetIOSRev(&iosRev);
    
    if (iosRev.major < 30 || iosRev.major >= 254) {
        return TRUE;
    }

    if (__OSDeviceCheckCode == 129) {
        checkCode = 0x7ED40000;
    }

    lowDone = FALSE;
    DVDLowUnencryptedRead((void*)CheckBuffer, 32, checkCode, lowCallback);\

    while (!lowDone) {

    }

    switch (lowIntType) {
        case 2:
            break;
        case 1:
            goto invalid;
            break;
        default:
            goto fatal;
            break;
    }

    lowDone = FALSE;
    DVDLowRequestError(lowCallback);

    while (!lowDone) {

    }

    outOfRangeError = DVDLowGetImmBufferReg();

    switch (lowIntType) {
        case 1:
        {
            if ((DVDLowGetImmBufferReg() & 0xFF000000) != 0) {
                goto recover;
                break;
            }

            switch (DVDLowGetImmBufferReg() & 0xFFFFFF) {
                case 0x52100:
                    break;
                default:
                    goto invalid;
                    break;
            }

            break;
        }

        default:
            goto fatal;
            break;
    }

    lowDone = FALSE;
    DVDLowReportKey((DVDVideoReportKey*)CheckBuffer, 0x40000, 0, lowCallback);

    while (!lowDone) {

    }

    switch (lowIntType) {
        case 2:
            break;
        case 1:
            goto invalid;
            break;
        default:
            goto fatal;
            break;
    }

    lowDone = FALSE;
    DVDLowRequestError(lowCallback);

    while (!lowDone) {

    }

    reportKeyError = DVDLowGetImmBufferReg();
    switch (lowIntType) {
        case 1:
            if ((DVDLowGetImmBufferReg() & 0xFF000000) != 0) {
                goto recover;
                break;
            }

            switch (DVDLowGetImmBufferReg() & 0xFFFFFF) {
                case 0x53100:
                case 0x52000:
                    break;
                default:
                    goto invalid;
                    break;
            }

            break;

        default:
            goto fatal;
            break;
    }

valid:
    return TRUE;

invalid:
    __DVDShowDeviceErrorMessage();
    return FALSE;

recover:
    return FALSE;

fatal:
    __DVDShowFatalMessage();
    return FALSE;
}

const char* const __DVDDeviceErrorMessage[] = {
    "\n"
    "\n"
    "\n"
    "\203G\203\211\201[\203R\201[\203h\202O\202O\202P\201B\n"
    "\225s\226\276\202\310\203f\203o\203C\203X\202\252\214\251\202\302\202\251\202\350\202\334\202\265\202\275\201B",

    "\n"
    "\n"
    "\n"
    "Error #001,\n"
    "unauthorized device has been detected.",

    "\n"
    "\n"
    "\n"
    "Fehler #001:\n"
    "Es wurde eine unzul\344ssige Komponente\n"
    "entdeckt.",

    "\n"
    "\n"
    "\n"
    "Erreur 001:\n"
    "un dispositif non autoris\351 a \351t\351 d\351tect\351.",

    "\n"
    "\n"
    "\n"
    "Error 001:\n"
    "Se ha detectado un dispositivo no\n"
    "autorizado.",

    "\n"
    "\n"
    "\n"
    "Errore #001:\n"
    "rilevato un dispositivo non autorizzato.",

    "\n"
    "\n"
    "\n"
    "Fout #001:\n"
    "ongeoorloofd onderdeel gevonden."

};

static void __DVDShowDeviceErrorMessage(void) {
    const char* message;
    const char* const* messageList;
    GXColor bg = { 0, 0, 0, 0 };
    GXColor fg = { 255, 255, 255, 0 };

    if (SCGetLanguage() == 0) {
        OSSetFontEncode(1);
    }
    else {
        OSSetFontEncode(0);
    }

    messageList = __DVDDeviceErrorMessage;

    if (SCGetLanguage() > 6) {
        message = messageList[1];
    }
    else {
        message = messageList[SCGetLanguage()];
    }

    OSFatal(fg, bg, message);
}