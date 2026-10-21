#ifndef RVL_SDK_SO_H
#define RVL_SDK_SO_H

#ifdef __cplusplus
extern "C" {
#endif

// RevoEX SO basic socket API (same declarations as the game's revolution/SO/SOBasic.h)
int SOFinish(void);
int SOStartup(void);
int SOCleanup(void);

#ifdef __cplusplus
}
#endif

#endif
