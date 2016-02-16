#include "utf8.hpp"

namespace sss {

    namespace util {

        namespace utf8 {

            const utf8_char_constant_type UTF8_table[] = {
                // {CUTF8_triByte},
                {UTF8_const<0>::LOW, UTF8_const<0>::HIGH, UTF8_const<0>::MASK, UTF8_const<0>::VALUE},
                {UTF8_const<1>::LOW, UTF8_const<1>::HIGH, UTF8_const<1>::MASK, UTF8_const<1>::VALUE},
                {UTF8_const<2>::LOW, UTF8_const<2>::HIGH, UTF8_const<2>::MASK, UTF8_const<2>::VALUE},
                {UTF8_const<3>::LOW, UTF8_const<3>::HIGH, UTF8_const<3>::MASK, UTF8_const<3>::VALUE},
                {UTF8_const<4>::LOW, UTF8_const<4>::HIGH, UTF8_const<4>::MASK, UTF8_const<4>::VALUE},
                {UTF8_const<5>::LOW, UTF8_const<5>::HIGH, UTF8_const<5>::MASK, UTF8_const<5>::VALUE},
                {UTF8_const<6>::LOW, UTF8_const<6>::HIGH, UTF8_const<6>::MASK, UTF8_const<6>::VALUE}
            };

            std::size_t get_ucs_code_length(u32t value)
            {
                const static std::size_t utf8_code_len_table[] = {
                    1, 1, 1, 1, 1, 1, 1, 1,             //0-7
                    2, 2, 2, 2,                         //8-11
                    3, 3, 3, 3, 3,                      //12-16
                    4, 4, 4, 4, 4,                      //17-21
                    5, 5, 5, 5, 5,                      //22-26
                    6, 6, 6, 6, 6,                      //27-31
                    0,                                  //32: error!
                };
                //NOTE
                //  highest_bit_pos<u32t> return value = {0, 1, 2, ... 32}
                //  0       : no 1 bit
                //  other   : as function name
                return utf8_code_len_table[sss::bit::highest_bit_pos(value)];
            }
        }
    }
}

