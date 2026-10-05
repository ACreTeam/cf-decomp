#pragma once
#include <types.h>

/// @brief Table-driven CRC32 (reflected, polynomial 0xEDB88320).
/// @ingroup slib
class sCrc {
public:
    /**
     * @brief Calculates the CRC32 checksum of a buffer.
     *
     * The size must be non-zero; the loop always reads at least one byte.
     *
     * @param data The buffer to checksum.
     * @param size The size of the buffer, in bytes.
     * @param crc The starting value (the save code passes -1 or 0x04201018).
     * @param xorOut The value XORed into the result (the save code passes -1).
     * @return The CRC32 checksum.
     */
    static ulong calcCRC32(const void *data, ulong size, ulong crc, ulong xorOut); // 802A98FC
};
