#include <private/gx.h>
#include <revolution/gx.h>

#include <private/os.h>
#include <revolution/os.h>

#include <string.h>

static GXFifoObj DisplayListFifo;
static GXFifoObj OldCPUFifo;
static GXData __savedGXdata;

void GXBeginDisplayList(void *list, u32 size)
{
    __GXFifoObj* dlFifo = (__GXFifoObj*)&DisplayListFifo;
    if (__GXData->dirtyState) {
        __GXSetDirtyState();
    }

    if (__GXData->dlSaveContext) {
        memcpy(&__savedGXdata, __GXData, sizeof(GXData));
    }

    dlFifo->base = (u8*)list;
    dlFifo->top = (u8*)list + size - 4;
    dlFifo->size = size;
    dlFifo->count = 0;
    dlFifo->rdPtr = list;
    dlFifo->wrPtr = list;

    __GXData->inDispList   = GX_TRUE;

    GXFlush();
    GXGetCPUFifo(&OldCPUFifo);
    GXSetCPUFifo(&DisplayListFifo);
    GXResetWriteGatherPipe();
}

u32 GXEndDisplayList(void) {
    GXBool ov;
    BOOL enabled;
    u32 cpenable;

    GXFlush();
    GXGetCPUFifo(&DisplayListFifo);
    ov = GXGetFifoWrap(&DisplayListFifo);

    GXSetCPUFifo(&OldCPUFifo);

    if (__GXData->dlSaveContext)
    {
        enabled = OSDisableInterrupts();
        cpenable = __GXData->cpEnable;
        memcpy(__GXData, &__savedGXdata, sizeof(GXData));
        __GXData->cpEnable = cpenable;
        OSRestoreInterrupts(enabled);
    }

    __GXData->inDispList = GX_FALSE;
    
    if (!ov) {
        return GXGetFifoCount(&DisplayListFifo);
    }
    else
    {
        return 0;
    }
}

void GXCallDisplayList(void* list, u32 nbytes) {
    if (__GXData->dirtyState)  {
        __GXSetDirtyState();
    }

    if (!(*(u32*)(&__GXData->vNumNot))) {
        __GXSendFlushPrim();
    }

    GX_WRITE_U8(GX_CMD_CALL_DL);
    GX_WRITE_U32((u32)list);
    GX_WRITE_U32((u32)nbytes);
}
