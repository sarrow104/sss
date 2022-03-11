#include  "bit_operation.h"

#include <sss/utlstring.hpp>

char sss::bit::bit_order()
{
    // NOTE Linux 内核代码——判断系统 是little-endian 还是 big-endian；

    // #define  ENDIANNESS ((char)endian_test.mylong)
    static union { char c[4]; unsigned long mylong; } endian_test = { { 'l', '?', '?', 'b' } };
    return static_cast<char>(endian_test.mylong);
}

const char * ext::binary_out_t::binary_table[] =
{
    "0000", "0001", "0010", "0011",
    "0100", "0101", "0110", "0111",
    "1000", "1001", "1010", "1011",
    "1100", "1101", "1110", "1111",
};

void sss::bit::buff_hex_reverse_print(std::ostream& out, const uint8_t * pdata, size_t size)
{
    for (size_t i = size; i != 0; --i) {
        out << sss::lower_hex2char(pdata[i - 1] >> 4U);
        out << sss::lower_hex2char(pdata[i - 1]);
    }
}

