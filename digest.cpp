#include "digest.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#define endian32	0
#define Poly32_Normal	0x04C11DB7
#define Poly32_Mirror	0xEDB88320
#define Crc32_Init	0xFFFFFFFF
#define Crc32_XorOut	0xFFFFFFFF

inline bool crc32_table_init(uint32_t table[256])
{
#if endian32
    uint32_t Poly = Poly32_Normal;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i << 24;
        for (unsigned char j = 0; j < 8; j++)
            crc = (crc << 1) ^ ((crc & 0x80000000) ? Poly : 0);
        table[i] = crc & 0xFFFFFFFF;
    }
#else
    uint32_t Poly = Poly32_Mirror;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (unsigned char j = 0; j < 8; j++)
            crc = (crc >> 1) ^ ((crc & 0x00000001) ? Poly : 0);
        table[i] = crc & 0xFFFFFFFF;
    }
#endif
    return true;
}

void crc32Init(uint32_t *pCrc32) {
    *pCrc32 = Crc32_Init;
}

void crc32Update(uint32_t *pCrc32, const unsigned char *pData, uint32_t uSize) {
    static uint32_t table[256];
    static bool init = crc32_table_init(table);
    (void) init;

#if endian32
    for(uint32_t i = 0; i < uSize; i++)
        *pCrc32 = ((*pCrc32) << 8) ^ table[(pData[i] ^ (*pCrc32 >>24)) & 0xFF];
#else
    for(uint32_t i = 0; i < uSize; i++)
        *pCrc32 = ((*pCrc32) >> 8) ^ table[(pData[i] ^ *pCrc32) & 0xFF];
#endif
}

void crc32Finish(uint32_t *pCrc32) {
    *pCrc32 ^= Crc32_XorOut;
}

namespace sss{
    namespace digest{

        // init_crc32_table();
        // crc32Init(&crc32Result);
        // crc32Update(&crc32Result, dadaBuffer, sizeof(dadaBuffer)-1);
        // crc32Finish(&crc32Result);

        //uint32_t crc32(char *buff, int len, uint32_t crc)
        //{
        //    static uint32_t table[256] = {0};
        //    static bool init = ::crc32_table_init(table);
        //    (void) init;

        //    crc = crc ^ (-1);
        //    for (int i = 0; i < len; i++)
        //        crc = (crc >> 8) ^ table[(crc ^ buff[i]) & 0xff];
        //    return (crc ^ (-1));
        //}

        uint32_t crc32_str(const char *buf, int len)
        {
            uint32_t crc = 0u;

            crc32Init(&crc);

            crc32Update(&crc, reinterpret_cast<const unsigned char*>(buf), len);

            crc32Finish(&crc);

            return crc;
        }

        int crc32_file(const char *in_file, uint32_t & crc_ref)
        {
            //std::cout << __func__ << "(" << in_file << ")" << std::endl;
            const int BUFSIZE = 2*1024;
            uint8_t buf[BUFSIZE];
            /*第一次传入的值需要固定,如果发送端使用该值计算crc校验码,
             **那么接收端也同样需要使用该值进行计算*/
            uint32_t crc = 0u;

            crc32Init(&crc);

            int fd = open(in_file, O_RDONLY);
            if (fd < 0) {
                printf("%d:open %s.\n", __LINE__, strerror(errno));
                return -1;
            }

            int nread = 0;
            while ((nread = read(fd, buf, BUFSIZE)) > 0) {
                //std::cout << "read " << nread << " bytes" << std::endl;
                crc32Update(&crc, buf, nread);
            }

            close(fd);

            if (nread < 0) {
                printf("%d:read %s.\n", __LINE__, strerror(errno));
                return false;
            }
            else {
                crc32Finish(&crc);
                crc_ref = crc;
            }

            return true;
        }
    }
}

