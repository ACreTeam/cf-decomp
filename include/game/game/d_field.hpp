#pragma once

// Field grids: current scene (0), saved town (1), and saved house rooms (2..13).
#include <types.h>

class dFdBase_c;
class dField_c;

namespace dField {
void setInfo(dFdBase_c *field, int idx); // 80190C30
dFdBase_c *getInfo(int idx);            // 80190C44
int getSceneFieldId(u8 scene);          // 80190C58
int getHomeFieldId(u32 home, int room); // 80190CDC
u32 getHeapSize();                     // 8019112C
dFdBase_c *getHomeInfo(u32 home, int room); // 80191178
void reloadHomes();                    // 801911AC
void reloadHome(int home);             // 801911E8
BOOL createSaveFields();               // 80191248

extern dFdBase_c *sInfo[14]; // 80624060
extern bool sSceneReady;    // 8074E9F0: current field ready
} // namespace dField
