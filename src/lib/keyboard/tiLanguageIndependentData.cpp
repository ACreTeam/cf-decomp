#include <lib/textinput/tiLanguageIndependentData.h>
#include <lib/textinput/tiCpData.h>

namespace textinput {
namespace keyboard {
namespace cellphonetype {

const wchar_t *csSpaceZero = L"\xE057"
                             L"0";
const wchar_t *csSpace[2] = {L"\xE057", NULL};

} // namespace cellphonetype
} // namespace keyboard

namespace langindependent {

const wchar_t *cLanguageIndependentString[LANG_STRID_MAX][LanguageEnd] = {
    {L"OK", L"OK", L"OK", L"OK", L"O.K.", L"OK", L"Aceptar", L"Ok\x00E9"},
    {L"\x3084\x3081\x308B", L"Quit", L"Quit", L"Quitter", L"Abbruch",
     L"Annulla", L"Salir", L"Annuleren"},
    {L"\x4E88\x6E2C\x8F9E\x66F8\x8A00\x8A9E\x306E\x9078\x629E", L"Dictionary",
     L"Language", L"Langue", L"Sprachauswahl", L"Lingua", L"Elegir idioma",
     L"Taalkeuze"},
    {L"\x3064\x304E", L"\x2192", L"\x2192", L"\x2192", L"\x2192", L"\x2192",
     L"\x2192", L"\x2192"},
    {L"\x307E\x3048", L"\x2190", L"\x2190", L"\x2190", L"\x2190", L"\x2190",
     L"\x2190", L"\x2190"},
    {L"a", L"a", L"a", L"a", L"a", L"a", L"a", L"a"},
    {L"\x3068\x3058\x308B", L"Close", L"Close", L"Fermer",
     L"Schlie\x00DF"
     L"en",
     L"Chiudi", L"Cerrar", L"Sluiten"},
    {L"\x8A18\x53F7", L"More", L"\x20AC\x00E9\x03B1", L"\x20AC\x00E5\x03B1",
     L"\x20AC\x00E9\x03B1", L"\x20AC\x00E5\x03B1", L"\x20AC\x00E5\x03B1",
     L"\x20AC\x00E9\x03B1"},
    {L"Space", L"Space", L"Space", L"", L"", L"", L"", L"Space"},
    {L"\x3057\x307E\x3046", L"Back", L"Back", L"Fermer",
     L"Zur\x00FC"
     L"ck",
     L"Chiudi", L"Cerrar", L"Stoppen"},
};

} // namespace langindependent
} // namespace textinput
