#ifndef  __DIGEST_HPP_1439910717__
#define  __DIGEST_HPP_1439910717__

// 数据摘要算法
// 1. crc32
//    循环冗余校验算法；它常用于硬件级别的数据传输校验——及时检验一些突发
//    数据传输错误，并抛弃错误数据。避免干扰上层应用；
//    当然，很多硬盘也使用了该算法。
//
//    其算法简单，容易被逆运算，所以并不适用于 数字签名 这种场景；
//    还有特点是满足 XOR 异或交换律
//    > crc(x XOR y XOR z) = crc(x) XOR crc(y) XOR crc(z)
//
//    还有一个常用的使用场景，是作为 hash_map 的hash算法——因为其产生定长的数
//    字串。且，碰撞率较低；
//
// "CRC32手动冲突本质"
//
// CRC32计算的过程，非常类似除法，求余数的过程——虽然这个除法，是用异或运算来代替。
//
// "余数"得多少，可以通过调整最后4个字节，来调整。就是说，你想得到什么"余数"，
// 你都可以简单得到，而不用管前面的是多少。
//
// 如何破？
// a. 换sha1等，长度更高的算法；
// b. 分段求crc32；比如分为3段，每段分别求crc32，即可让上述办法无处遁形；不过，
// 这种伪造的办法，主要是针对网络的底层传输验证，而底层的算法，基本是不可能更改
// 的——一般是硬件保证。
//

#include <stdint.h>

#include <string>

namespace sss{
    namespace digest{
        //uint32_t crc32(char *buff, int len, uint32_t crc = 0);

        uint32_t crc32_str(const char *buf, int len);

        inline uint32_t crc32_str(const std::string& s)
        {
            return crc32_str(s.c_str(), s.length());
        }

        // return:
        // error code
        int crc32_file(const char *in_file, uint32_t & crc);
    }
}

#endif  /* __DIGEST_HPP_1439910717__ */

#if 0
    typedef std::uint64_t hash_t;

    constexpr hash_t prime = 0x100000001B3ull;
    constexpr hash_t basis = 0xCBF29CE484222325ull;

    hash_t hash_(char const* str)
    {
        hash_t ret{basis};

        while(*str){
            ret ^= *str;
            ret *= prime;
            str++;
        }

        return ret;
    }

#endif
