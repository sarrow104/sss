#include  "bit_operation.h"

#include <sss/utlstring.hpp>

char sss::bit::bit_order()
{
// NOTE Linux 内核代码——判断系统 是little-endian 还是 big-endian；

// #define  ENDIANNESS ((char)endian_test.mylong)
    static union { char c[4]; unsigned long mylong; } endian_test = { { 'l', '?', '?', 'b' } };
    return (char)endian_test.mylong;
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
        out << sss::lower_hex2char(pdata[i - 1] >> 4);
        out << sss::lower_hex2char(pdata[i - 1]);
    }
}

#ifdef  __DEBUG_BIT_OPERATION_CPP__     //{{{1

template<typename T>
void test_bit_reverse(std::ostream& o, T c)
{
    o << int(c) << ":" << ext::binary << c << "|"
        << ext::binary << bit_reverse(c) << std::endl;
}

template<typename T>
void test_count_1_bit(std::ostream& o, T c)
{
    o << int(c) << ":" << count_1_bit(c) << "|"
        << _count_1_bit(c) << " highest_bit_pos at: " << highest_bit_pos(c) << std::endl;
}

int main(int argc, char *argv[])
{//{{{2
    std::ofstream ofs("out.txt");
    test_bit_reverse(ofs, 100006);
    test_bit_reverse<unsigned short>(ofs, 106);
    test_bit_reverse(ofs, '\x06');

    test_count_1_bit(ofs, 100006);
    test_count_1_bit<unsigned short>(ofs, 106);
    test_count_1_bit(ofs, '\x06');

    return 0;
}//}}}
#endif  /*__DEBUG_BIT_OPERATION_CPP__*/

/* MAKE {{{1
 !g++ bit_operation.cpp -D__DEBUG_BIT_OPERATION_CPP__ -o bit_operation
 */
