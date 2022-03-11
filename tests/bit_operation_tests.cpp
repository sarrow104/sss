#include <fmt/core.h>
#include <gtest/gtest.h>

#include <sstream>
#include <string_view>

#include "../bit_operation/bit_operation.h"


template<typename T>
void test_binary_print(const T& c, std::string_view expect)
{
    std::ostringstream o;

    o << ext::binary << c;

    std::string out = o.str();

    std::cout << c << ":b" << out << std::endl;

    GTEST_ASSERT_EQ(out, expect);
}

template<typename T>
std::string do_binary_print(const T& c)
{
    std::ostringstream o;

    o << ext::binary << c;

    std::string out = o.str();

    std::cout << c << ":b" << out << std::endl;

    return out;
}

template<typename T>
void test_bit_reverse(const T& c)
{
    std::ostringstream o;
    std::ostringstream o1;
    std::ostringstream o2;

    o << int(c) << ":" << ext::binary << c << "|"
        << ext::binary << sss::bit::bit_reverse(c);

    o1 << ext::binary << c;
    o2 << ext::binary << sss::bit::bit_reverse(c);
    std::string reSet = o2.str();
    for (size_t i = 0U, j = reSet.size() - 1U; i < j; ++i, --j) {
        auto c = reSet[j];
        reSet[j] = reSet[i];
        reSet[i] = c;
    }

    std::string origin = o1.str();
    std::cout << o.str() << std::endl;

    GTEST_ASSERT_EQ(reSet, origin);
}

template<typename T>
void test_count_1_bit(const T& c)
{
    std::ostringstream o;
    o << int(c) << ":" << sss::bit::count_1_bit(c) << "|"
        << sss::bit::_count_1_bit(c);

    std::cout << o.str() << std::endl;
    GTEST_ASSERT_EQ(sss::bit::count_1_bit(c), sss::bit::_count_1_bit(c));
}

template<typename T>
size_t do_get_highest_bit_pos(const T& c)
{
    auto highest_bit_pos = sss::bit::highest_bit_pos(c);
    std::cout << c << ":" << "highest_bit_pos at: " << highest_bit_pos;
    return highest_bit_pos;
}

TEST(bit_operation, basic)
{
    GTEST_ASSERT_EQ(do_binary_print( 0+0), "00000000000000000000000000000000");
    GTEST_ASSERT_EQ(do_binary_print( 0+1), "00000000000000000000000000000001");
    GTEST_ASSERT_EQ(do_binary_print( 0+2), "00000000000000000000000000000010");
    GTEST_ASSERT_EQ(do_binary_print( 0+3), "00000000000000000000000000000011");
    GTEST_ASSERT_EQ(do_binary_print( 4+0), "00000000000000000000000000000100");
    GTEST_ASSERT_EQ(do_binary_print( 4+1), "00000000000000000000000000000101");
    GTEST_ASSERT_EQ(do_binary_print( 4+2), "00000000000000000000000000000110");
    GTEST_ASSERT_EQ(do_binary_print( 4+3), "00000000000000000000000000000111");

    GTEST_ASSERT_EQ(do_binary_print( 8+0), "00000000000000000000000000001000");
    GTEST_ASSERT_EQ(do_binary_print( 8+1), "00000000000000000000000000001001");
    GTEST_ASSERT_EQ(do_binary_print( 8+2), "00000000000000000000000000001010");
    GTEST_ASSERT_EQ(do_binary_print( 8+3), "00000000000000000000000000001011");
    GTEST_ASSERT_EQ(do_binary_print(12+0), "00000000000000000000000000001100");
    GTEST_ASSERT_EQ(do_binary_print(12+1), "00000000000000000000000000001101");
    GTEST_ASSERT_EQ(do_binary_print(12+2), "00000000000000000000000000001110");
    GTEST_ASSERT_EQ(do_binary_print(12+3), "00000000000000000000000000001111");

    {
        uint8_t x = ~uint8_t(0);
        uint16_t y = x;
        uint32_t z = x;
        GTEST_ASSERT_EQ(do_binary_print(x), "11111111"                        );
        GTEST_ASSERT_EQ(do_binary_print(y), "0000000011111111"                );
        GTEST_ASSERT_EQ(do_binary_print(z), "00000000000000000000000011111111");

    }
    {
        int8_t x = ~int8_t(0);
        int16_t y = x;
        int32_t z = x;

        GTEST_ASSERT_EQ(do_binary_print(x), "11111111"                        );
        GTEST_ASSERT_EQ(do_binary_print(y), "1111111111111111"                );
        GTEST_ASSERT_EQ(do_binary_print(z), "11111111111111111111111111111111");
    }
}

TEST(bit_operation, reverse)
{
    test_bit_reverse(100006);
    test_bit_reverse(100006U);
    test_bit_reverse<unsigned short>(106);
    test_bit_reverse<char>('\x06');

}

TEST(bit_operation, count_1)
{
    test_count_1_bit(100006);
    test_count_1_bit<unsigned short>(106);
    test_count_1_bit('\x06');
}

TEST(bit_operation, highest_bit_pos)
{
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<< 0), 1U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<< 1), 2U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<< 2), 3U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<< 3), 4U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<< 4), 5U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<< 5), 6U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<< 6), 7U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<< 7), 8U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<< 8), 9U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<< 9),10U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<10),11U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<11),12U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<12),13U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<13),14U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<14),15U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<15),16U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<16),17U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<17),18U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<18),19U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<19),20U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<20),21U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<21),22U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<22),23U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<23),24U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<24),25U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<25),26U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<26),27U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<27),28U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<28),29U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<29),30U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<30),31U);
    GTEST_ASSERT_EQ(do_get_highest_bit_pos(1<<31),32U);
}
