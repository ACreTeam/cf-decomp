#include <internal/RFLi_LibConfig.h>

#include <RVLFaceLib.h>
#include <RVLFaceLibInternal.h>

#include <internal/RFLi_StoreData.h>

#include <string.h>

u32 RFLiGetExchangeBufferSize(u16 num) {
    return num * sizeof(RFLiStoreData) + 8;
}

RFLErrcode RFLiInitExchangeBuffer(RFLiExchangeBuffer* buffer, u16 num) {
    if (buffer == NULL) {
        return RFLErrcode_WrongParam;
    }

    if (num == 0 || num >= 32) {
        return RFLErrcode_WrongParam;
    }

    if (!RFLAvailable()) {
        return RFLErrcode_NotAvailable;
    }

    memset(buffer, 0, RFLiGetExchangeBufferSize(num));
    buffer->mNum = num;
    buffer->mValidMask = 0;
    buffer->padding = 0;

    return RFLErrcode_Success;
}

RFLErrcode RFLiSetExchangeData(RFLiExchangeBuffer* buffer, u16 index, RFLDataSource source, u16 srcIndex, u16 num) {
    u32 flag = 1 << index;
    RFLErrcode err;
    RFLiCharInfo info;
    RFLiStoreData store;
    RFLiCharData raw;

    buffer->mValidMask &= ~flag;

    if (buffer == NULL) {
        return RFLErrcode_WrongParam;
    }

    if (index >= buffer->mNum) {
        return RFLErrcode_WrongParam;
    }

    if (source == RFLDataSource_Middle) {
        return RFLErrcode_WrongParam;
    }

    if (!RFLAvailable()) {
        return RFLErrcode_NotAvailable;
    }

    err = RFLiPickupCharInfo(&info, source, NULL, srcIndex);
    if (err == RFLErrcode_Success) {
        RFLiConvertInfo2Raw(&info, &raw);
        raw.favorite = FALSE;

        if (RFLiGetIsolation()) {
            raw.localonly = TRUE;
        }

        memcpy(&store.rawdata, &raw, sizeof(RFLiCharData));
        store.crc = 0;
        store.crc = RFLiCalculateCRC(&store, sizeof(RFLiStoreData));

        memcpy(&buffer->mData[index], &store, sizeof(RFLiStoreData));
        buffer->mValidMask |= flag;
    }

    return err;
}

u32 RFLiGetReceiveBufferSize(u16 memberNum, u16 num) {
    u32 size = RFLiGetExchangeBufferSize(num);
    return size * (memberNum - 1);
}

RFLErrcode RFLiInitReceiveConverter(RFLiReceiveConverter* converter, RFLiExchangeBuffer* buffer, u16 myNo, u16 memberNum,
                                    u16 num) {
    if (converter == NULL) {
        return RFLErrcode_WrongParam;
    }

    if (buffer == NULL) {
        return RFLErrcode_WrongParam;
    }

    if (myNo >= memberNum) {
        return RFLErrcode_WrongParam;
    }

    if (memberNum >= 32) {
        return RFLErrcode_WrongParam;
    }

    if ((memberNum - 1) * num >= 100) {
        return RFLErrcode_WrongParam;
    }

    memset(converter, 0, sizeof(RFLiReceiveConverter));
    converter->num = num;
    converter->memberNum = memberNum;
    converter->myNo = myNo;
    converter->storedMask = 0;
    converter->receivedMask = 0;
    converter->buffer = buffer;

    return RFLErrcode_Success;
}

static u16 getSlot_(const RFLiReceiveConverter* converter, u16 member) {
    u16 slot = member;

    if (member == converter->myNo) {
        slot = 0;
    } else if (member >= converter->myNo) {
        slot = member - 1;
    }

    return slot;
}

static RFLiExchangeBuffer* getSlotBuffer_(const RFLiReceiveConverter* converter, u16 slot) {
    return (RFLiExchangeBuffer*)((u8*)converter->buffer + slot * RFLiGetExchangeBufferSize(converter->num));
}

RFLErrcode RFLiStoreReceivedData(RFLiReceiveConverter* converter, const RFLiExchangeBuffer* data, u16 member) {
    RFLErrcode err = RFLErrcode_Success;
    u16 slot;
    u32 size;
    u16 i;

    if (converter->myNo == member) {
        return RFLErrcode_WrongParam;
    }

    slot = getSlot_(converter, member);
    if (slot >= converter->memberNum - 1) {
        return RFLErrcode_WrongParam;
    }

    converter->storedMask &= ~(1 << member);
    size = RFLiGetExchangeBufferSize(data->mNum);

    for (i = 0; i < data->mNum; i++) {
        if ((data->mValidMask & (1 << i)) && !RFLiIsValidRawData((RFLiCharData*)&data->mData[i])) {
            err = RFLErrcode_Broken;
        }
    }

    memcpy(getSlotBuffer_(converter, slot), data, size);

    return err;
}

RFLErrcode RFLiReceiveData(RFLiReceiveConverter* converter, const RFLiExchangeBuffer* data, u16 member) {
    return RFLiReceiveDataCore(converter, data, member, TRUE);
}

RFLErrcode RFLiReceiveDataCore(RFLiReceiveConverter* converter, const RFLiExchangeBuffer* data, u16 member, BOOL write) {
    RFLErrcode err;

    if (converter == NULL) {
        return RFLErrcode_WrongParam;
    }

    if (data == NULL) {
        return RFLErrcode_WrongParam;
    }

    if (!RFLAvailable()) {
        return RFLErrcode_NotAvailable;
    }

    if (RFLiIsWorking()) {
        return RFLErrcode_NotAvailable;
    }

    err = RFLiStoreReceivedData(converter, data, member);
    if (err != RFLErrcode_Success) {
        return err;
    }

    RFLiStartWorking();

    if (RFLiGetIsolation() || !write) {
        converter->storedMask |= 1 << member;
        RFLiEndWorking(RFLErrcode_Success);
    } else {
        RFLiWriteExchangeToHiddenDB(data);
    }

    return RFLGetAsyncStatus();
}

RFLErrcode RFLiClearReceivedMember(RFLiReceiveConverter* converter, u16 member) {
    u16 slot;

    if (converter == NULL) {
        return RFLErrcode_WrongParam;
    }

    if (converter->myNo == member) {
        return RFLErrcode_WrongParam;
    }

    slot = getSlot_(converter, member);
    if (slot >= converter->memberNum - 1) {
        return RFLErrcode_WrongParam;
    }

    converter->receivedMask &= ~(1 << member);
    converter->storedMask &= ~(1 << member);

    return RFLErrcode_Success;
}

RFLErrcode RFLiAddReceivedToMiddleDB(RFLMiddleDatabase* db, const RFLiReceiveConverter* converter, u16 member) {
    RFLiMiddleDatabase* mdb = (RFLiMiddleDatabase*)db;
    const RFLiExchangeBuffer* buffer;
    int i;
    u16 slot;

    if (db == NULL) {
        return RFLErrcode_WrongParam;
    }

    if (converter == NULL) {
        return RFLErrcode_WrongParam;
    }

    if (!RFLAvailable()) {
        return RFLErrcode_NotAvailable;
    }

    if (converter->storedMask & !(1 << member)) {
        return RFLErrcode_DBNodata;
    }

    slot = getSlot_(converter, member);
    buffer = getSlotBuffer_(converter, slot);

    if (mdb->size == 0) {
        return RFLErrcode_WrongParam;
    }

    if (mdb->type != RFLMiddleDBType_Reserved1) {
        return RFLErrcode_WrongParam;
    }

    mdb->storedSize = 0;
    memset(mdb->data, 0, mdb->size * sizeof(RFLiHiddenCharData));

    for (i = 0; i < buffer->mNum; i++) {
        if (RFLiIsValidRawData((RFLiCharData*)&buffer->mData[i])) {
            if (mdb->storedSize >= mdb->size) {
                return RFLErrcode_DBFull;
            }

            RFLiConvertRaw2HRaw((RFLiCharData*)&buffer->mData[i], &mdb->data[mdb->storedSize]);
            mdb->storedSize++;
        }
    }

    return mdb->storedSize < mdb->size ? RFLErrcode_DBNodata : RFLErrcode_Success;
}
