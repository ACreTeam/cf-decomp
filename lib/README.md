# lib

Library sources and headers copied verbatim from other decompilation projects.
Some of these translation units match City Folk exactly (`Matching` in
`configure.py`) and the rest are imported as `NonMatching` starting points.

| Folder | Upstream | Contents |
| --- | --- | --- |
| `RVL_SDK/`, `RevoEX/` | [koopthekoopa/wii-ipl](https://github.com/koopthekoopa/wii-ipl) `libs/` | SDK and RevoEX sources plus the headers they need |
| `NW4R/`, `MSL/`, `Runtime/`, `MetroTRK/` | wii-ipl `libs/` | headers only, as needed by the above |
| `keyboard/` | wii-ipl `src/keyboard`, `include/keyboard` | keyboard (textinput) sources and headers |
| `common/` | wii-ipl `include/global` | shared helper headers (`decomp.h`, `config.h`) |
| `gamespy/` | [doldecomp/mkw](https://github.com/doldecomp/mkw) `lib/gamespy` | GameSpy sources and headers |
| `gamespy/support/` | mkw `include`, `lib/MSL`, `lib/rvl` | mkw headers GameSpy depends on |
| `dwc/` | [yannicksuter/mscharged-decomp](https://github.com/yannicksuter/mscharged-decomp) `src/RVL_SDK/dwc`, `libs/RVL_SDK/include/dwc`, by way of another project's adaptation to a later DWC revision (`dwc_init.c` directly from mscharged) | DWC (Nintendo Wi-Fi Connection) sources and headers; built against the wii-ipl SDK headers and mkw's GameSpy headers |
| `dwc/support/` | written here | the few SDK/NITRO headers DWC needs that neither tree has (NCD, NHTTP, NITRO math) and shims mapping mkw's GameSpy support includes onto the wii-ipl SDK |

All upstream projects are CC0 (`LICENSE.wii-ipl`, `LICENSE.mkw`, `LICENSE.mscharged`). Each group is
built with its upstream project's own compiler flags (the `cflags_ext_*` lists in
`configure.py`). Keep edits to a minimum so the files can be refreshed from upstream.
