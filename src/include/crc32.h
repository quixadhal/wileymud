/*
 * We want CRC32 for our URL storage system, to be compatible with what
 * we'd already used elsewhere.  Also, sometimes a shorter hash is more
 * useful, when humans are involved.
 */

#ifndef _CRC32_H
#define _CRC32_H

u_int32_t crc32(const char *buf, int len);

#endif
