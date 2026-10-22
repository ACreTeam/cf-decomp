#ifndef REVOLUTION_GX_LIGHTING_H
#define REVOLUTION_GX_LIGHTING_H

#include <revolution/gx/GXEnum.h>
#include <revolution/gx/GXStruct.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void GXInitLightPos(GXLightObj* lt_obj, f32 x, f32 y, f32 z);
void GXInitLightColor(GXLightObj* lt_obj, GXColor color);

void GXLoadLightObjImm(const GXLightObj* lt_obj, GXLightID light);

void GXSetChanAmbColor(GXChannelID chan, GXColor amb_color);
void GXSetChanMatColor(GXChannelID chan, GXColor mat_color);

void GXSetNumChans(u8 nChans);

void GXSetChanCtrl(GXChannelID chan, GXBool enable, GXColorSrc amb_src, GXColorSrc mat_src, u32 light_mask, GXDiffuseFn diff_fn, GXAttnFn attn_fn);



void GXInitLightAttn(GXLightObj*, f32, f32, f32, f32, f32, f32);
void GXInitLightAttnA(GXLightObj* lt_obj, f32 a0, f32 a1, f32 a2);
void GXGetLightPos(const GXLightObj* lt_obj, f32* x, f32* y, f32* z);
void GXGetLightDir(const GXLightObj* lt_obj, f32* nx, f32* ny, f32* nz);
void GXInitLightAttnK(GXLightObj* lt_obj, f32 k0, f32 k1, f32 k2);

void GXInitLightDistAttn(GXLightObj*, f32, f32, GXDistAttnFn);

void GXInitLightDir(GXLightObj*, f32, f32, f32);

void GXInitLightSpot(GXLightObj*, f32, GXSpotFn);

void GXInitSpecularDir(GXLightObj*, f32, f32, f32);
void GXInitSpecularDirHA(GXLightObj*, f32, f32, f32, f32, f32, f32);

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_GX_LIGHTING_H
