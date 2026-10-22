#include <revolution/dvd.h>
#include <revolution/os.h>
#include <revolution/sc.h>

#include <private/dvd.h>

static void (*FatalFunc)();

const char* const __DVDErrorMessageDefault[] = {
    "\n"
    "\n"
    "\n"
    "\203G\203\211\201[\202\252\224\255\220\266\202\265\202\334\202\265\202\275\201B\n"
    "\n"
    "\203C\203W\203F\203N\203g\203{\203^\203\223\202\360\211\237\202\265\202\304\203f\203B\203X\203N\202\360\216\346\202\350\217o\202\265\202\304\202\251\n"
    "\202\347\201A\226{\221\314\202\314\223d\214\271\202\360OFF\202\311\202\265\202\304\201A\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\314\n"
    "\216w\216\246\202\311\217]\202\301\202\304\202\255\202\276\202\263\202\242\201B",

    "\n"
    "\n"
    "\n"
    "An error has occurred.\n"
    "Press the Eject Button, remove the\n"
    "Game Disc, and turn the power off.\n"
    "Please read the Wii Operations Manual\n"
    "for more information.",

    "\n"
    "\n"
    "\n"
    "Ein Fehler ist aufgetreten.\n"
    "Dr\374cke den Ausgabeknopf, entnimm die\n"
    "Disc und schalte die Wii-Konsole aus.\n"
    "Bitte lies die Wii-Bedienungsanleitung,\n"
    "um weitere Informationen zu erhalten.",

    "\n"
    "\n"
    "\n"
    "Une erreur est survenue.\n"
    "Appuyez sur le bouton EJECT, retirez\n"
    "le disque et \351teignez la console.\n"
    "Veuillez vous r\351f\351rer au mode d'emploi\n"
    "Wii pour plus de d\351tails.",

    "\n"
    "\n"
    "\n"
    "Se ha producido un error.\n"
    "Pulsa el Bot\363n EJECT, extrae el disco y\n"
    "apaga la consola. Consulta el manual de\n"
    "instrucciones de la consola Wii para\n"
    "obtener m\341s informaci\363n.",

    "\n"
    "\n"
    "\n"
    "Si \350 verificato un errore.\n"
    "Premi il pulsante EJECT, estrai il disco\n"
    "e spegni la console. Per maggiori\n"
    "informazioni, consulta il manuale di\n"
    "istruzioni della console Wii.",

    "\n"
    "\n"
    "\n"
    "Er is een fout opgetreden.\n"
    "Druk op de EJECT-knop, verwijder de\n"
    "disk en zet het Wii-systeem uit. Lees\n"
    "de handleiding voor meer informatie."

};

const char* const __DVDErrorMessageEurope[] = {
    "\n"
    "\n"
    "\n"
    "\203G\203\211\201[\202\252\224\255\220\266\202\265\202\334\202\265\202\275\201B\n"
    "\n"
    "\203C\203W\203F\203N\203g\203{\203^\203\223\202\360\211\237\202\265\202\304\203f\203B\203X\203N\202\360\216\346\202\350\217o\202\265\202\304\202\251\n"
    "\202\347\201A\226{\221\314\202\314\223d\214\271\202\360OFF\202\311\202\265\202\304\201A\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\314\n"
    "\216w\216\246\202\311\217]\202\301\202\304\202\255\202\276\202\263\202\242\201B",

    "\n"
    "\n"
    "\n"
    "An error has occurred.\n"
    "Press the EJECT Button, remove the\n"
    "Game Disc, and turn the power off.\n"
    "Please read the Wii Operations Manual\n"
    "for more information.",

    "\n"
    "\n"
    "\n"
    "Ein Fehler ist aufgetreten.\n"
    "Dr\374cke den Ausgabeknopf, entnimm die\n"
    "Disc und schalte die Wii-Konsole aus.\n"
    "Bitte lies die Wii-Bedienungsanleitung,\n"
    "um weitere Informationen zu erhalten.",

    "\n"
    "\n"
    "\n"
    "Une erreur est survenue.\n"
    "Appuyez sur le bouton EJECT, retirez\n"
    "le disque et \351teignez la console.\n"
    "Veuillez vous r\351f\351rer au mode d'emploi\n"
    "Wii pour plus de d\351tails.",

    "\n"
    "\n"
    "\n"
    "Se ha producido un error.\n"
    "Pulsa el Bot\363n EJECT, extrae el disco y\n"
    "apaga la consola. Consulta el manual de\n"
    "instrucciones de la consola Wii para\n"
    "obtener m\341s informaci\363n.",

    "\n"
    "\n"
    "\n"
    "Si \350 verificato un errore.\n"
    "Premi il pulsante EJECT, estrai il disco\n"
    "e spegni la console. Per maggiori\n"
    "informazioni, consulta il manuale di\n"
    "istruzioni della console Wii.",

    "\n"
    "\n"
    "\n"
    "Er is een fout opgetreden.\n"
    "Druk op de EJECT-knop, verwijder de\n"
    "disk en zet het Wii-systeem uit. Lees\n"
    "de handleiding voor meer informatie."

};

void __DVDShowFatalMessage(void) {
    const char* message;
    const char* const* messageList;
    GXColor bg = { 0, 0, 0, 0 };
    GXColor fg = { 255, 255, 255, 0 };

    if (SCGetLanguage() == SC_LANG_JAPANESE) {
        OSSetFontEncode(OS_FONT_ENCODE_SJIS);
    } else {
        OSSetFontEncode(OS_FONT_ENCODE_ANSI);
    }

    switch (SCGetProductGameRegion()) {
    default:
        messageList = __DVDErrorMessageDefault;
        break;
    case 2:
        messageList = __DVDErrorMessageEurope;
        break;
    }

    if (SCGetLanguage() > SC_LANG_DUTCH) {
        message = messageList[1];
    } else {
        message = messageList[SCGetLanguage()];
    }

    OSFatal(fg, bg, message);
}

BOOL DVDSetAutoFatalMessaging(BOOL enable) {
    BOOL enabled, prev;
    enabled = OSDisableInterrupts();
    prev = FatalFunc ? TRUE : FALSE;
    FatalFunc = enable ? __DVDShowFatalMessage : NULL;
    OSRestoreInterrupts(enabled);
    return prev;
}

BOOL __DVDGetAutoFatalMessaging(void) {
    return FatalFunc ? TRUE : FALSE;
}

void __DVDPrintFatalMessage(void) {
    if (FatalFunc) {
        FatalFunc();
    }
}
