#include <prfile2/puh_msc.h>
#include <revolution/os.h>

void* uhg_msc_mem_pool;
s32 uhg_msc_message_id;
s32 uhg_msc_state;

s32 usbh_msc_read10(UHF_MSC_DEVICE* p_dev, u32 lba, u16 num_blocks, u8* buf, u8* sense) {
    UHF_MSC_REQ req = {0};

    if (uhg_msc_state != UHF_MSC_STATE_STARTED) {
        return UHF_MSC_ERR_NOT_STARTED;
    }
    req.p_dev = p_dev;
    req.lba = lba;
    req.num_blocks = num_blocks;
    req.p_buf = buf;
    req.p_sense = sense;
    return _uhf_msc_api_send_message(&req, UHF_MSC_CMD_READ10);
}

s32 usbh_msc_write10(UHF_MSC_DEVICE* p_dev, u32 lba, u16 num_blocks, u8* buf, u8* sense) {
    UHF_MSC_REQ req = {0};

    if (uhg_msc_state != UHF_MSC_STATE_STARTED) {
        return UHF_MSC_ERR_NOT_STARTED;
    }
    req.p_dev = p_dev;
    req.lba = lba;
    req.num_blocks = num_blocks;
    req.p_buf = buf;
    req.p_sense = sense;
    return _uhf_msc_api_send_message(&req, UHF_MSC_CMD_WRITE10);
}

s32 _uhf_msc_api_send_message(UHF_MSC_REQ* p_req, u32 cmd) {
    s32 sem_id;
    UHF_MSC_MSG* p_msg;
    s32 result;

    result = 0;
    p_msg = SDOS_AllocMem(uhg_msc_mem_pool, sizeof(UHF_MSC_MSG), UHF_MSC_MSG_ALIGN);
    if (p_msg == NULL) {
        return UHF_MSC_ERR_SYSTEM;
    }

    sem_id = SDOS_CreateSemaphore(0, 0);
    if (sem_id <= 0) {
        SDOS_FreeMem(uhg_msc_mem_pool, p_msg);
        return UHF_MSC_ERR_SYSTEM;
    }

    p_msg->cmd = cmd;
    p_msg->sem_id = sem_id;
    p_msg->p_result = &result;
    p_msg->req = *p_req;
    if (SDOS_SendMessage(uhg_msc_message_id, p_msg, OS_MESSAGE_NOBLOCK) != 0) {
        SDOS_FreeMem(uhg_msc_mem_pool, p_msg);
        SDOS_DeleteSemaphore(sem_id);
        return UHF_MSC_ERR_SYSTEM;
    }
    if (SDOS_WaitSemaphore(sem_id) != 0) {
        SDOS_DeleteSemaphore(sem_id);
        return UHF_MSC_ERR_SYSTEM;
    }
    if (SDOS_DeleteSemaphore(sem_id) != 0) {
        return UHF_MSC_ERR_SYSTEM;
    }
    return result;
}

s32 uhf_msc_get_message_id(void) {
    return uhg_msc_message_id;
}
