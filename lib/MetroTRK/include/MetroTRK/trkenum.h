#ifndef METROTRK_TRKENUM_H
#define METROTRK_TRKENUM_H

typedef enum DSIOResult
{
    DS_IONoError = 0,
    DS_IOError = 1,
    DS_IOEOF = 2,
} DSIOResult;

typedef enum MessageCommandID
{
    DSMSG_Ping = 0x00,
    DSMSG_Connect = 0x01,
    DSMSG_Disconnect = 0x02,
    DSMSG_Reset = 0x03,
    DSMSG_Versions = 0x04,
    DSMSG_SupportMask = 0x05,
    DSMSG_Override = 0x07,
    DSMSG_ReadMemory = 0x10,
    DSMSG_WriteMemory = 0x11,
    DSMSG_ReadRegisters = 0x12,
    DSMSG_WriteRegisters = 0x13,
    DSMSG_SetOption = 0x17,
    DSMSG_Continue = 0x18,
    DSMSG_Step = 0x19,
    DSMSG_Stop = 0x1A,
    DSMSG_ReplyACK = 0x80,
    DSMSG_NotifyStopped = 0x90,
    DSMSG_NotifyException = 0x91,
    DSMSG_WriteFile = 0xD0,
    DSMSG_ReadFile = 0xD1,
    DSMSG_OpenFile = 0xD2,
    DSMSG_CloseFile = 0xD3,
    DSMSG_PositionFile = 0xD4,
    DSMSG_ReplyNAK = 0xFF,
} MessageCommandID;

typedef enum DSReplyError
{
    DSREPLY_NoError = 0x00,
    DSREPLY_Error = 0x01,
    DSREPLY_PacketSizeError = 0x02,
    DSREPLY_CWDSError = 0x03,
    DSREPLY_EscapeError = 0x04,
    DSREPLY_BadFCS = 0x05,
    DSREPLY_Overflow = 0x06,
    DSREPLY_SequenceMissing = 0x07,
    DSREPLY_UnsupportedCommandError = 0x10,
    DSREPLY_ParameterError = 0x11,
    DSREPLY_UnsupportedOptionError = 0x12,
    DSREPLY_InvalidMemoryRange = 0x13,
    DSREPLY_InvalidRegisterRange = 0x14,
    DSREPLY_CWDSException = 0x15,
    DSREPLY_NotStopped = 0x16,
    DSREPLY_BreakpointsFull = 0x17,
    DSREPLY_BreakpointConflict = 0x18,
    DSREPLY_OSError = 0x20,
    DSREPLY_InvalidProcessID = 0x21,
    DSREPLY_InvalidThreadID = 0x22,
    DSREPLY_DebugSecurityError = 0x23,
} DSReplyError;

#endif // METROTRK_TRKENUM_H
