#ifndef __UTF8_H_1452945809__
#define __UTF8_H_1452945809__

#include <sss/bit_operation/bit_operation.h>
#include <sss/util/PostionThrow.hpp>
#include <sss/utlstring.hpp>

#include <stdexcept>
#include <typeinfo>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

namespace sss {

    namespace util {

        namespace utf8 {

            typedef uint32_t    u32t;
            typedef uint8_t     u8t;

            template<int _codeLen> struct UTF8_const
            {
                enum { codeLen = _codeLen };
                const static u32t LOW = UTF8_const<codeLen - 1>::HIGH + 1;
                const static u32t BITLEN = UTF8_const<codeLen - 1>::BITLEN + (6 - 1);
                const static u32t HIGH = (u32t(1) << BITLEN) - 1;
                const static u8t MASK = ((1 << (codeLen + 1)) - 1) << (8 - codeLen - 1);
                const static u8t VALUE = MASK & (MASK - 1);
            };

            //  the tail byte
            template<> struct UTF8_const<0>
            {
                enum { codeLen = 1 };
                const static u32t BITLEN = 6;
                const static u32t LOW = 0;
                const static u32t HIGH = 0;
                const static u8t MASK = ((1 << (codeLen + 1)) - 1) << (8 - codeLen - 1);
                const static u8t VALUE = MASK & (MASK - 1);
            };

            //  the UTF8 byte length 1
            template<> struct UTF8_const<1>
            {
                enum {codeLen = 1};
                const static u32t LOW = 0;
                const static u32t BITLEN = 8 - 1;
                const static u32t HIGH = (u32t(1) << BITLEN) - 1;
                const static u8t MASK = 1 << 7;
                const static u8t VALUE = MASK & (MASK - 1);
            };

            //  the UTF8 byte length 2
            template<> struct UTF8_const<2>
            {
                enum {codeLen = 2};
                const static u32t LOW = UTF8_const<1>::HIGH + 1;
                const static u32t BITLEN = UTF8_const<1>::BITLEN + 4;
                const static u32t HIGH = (u32t(1) << BITLEN) - 1;
                const static u8t MASK = ((1 << (codeLen + 1)) - 1) << (8 - codeLen - 1);
                const static u8t VALUE = MASK & (MASK - 1);
            };

            struct utf8_char_constant_type
            {
                u32t LOW;       // 编码最小值
                u32t HIGH;      // 编码最大值
                u32t MASK;      // head-byte掩码
                u32t VALUE;     // head-byte有效值掩码
            };

            enum utf8_stream_value_type {
                /*UTF8_fsm_vt_*/h1,     // 0
                /*UTF8_fsm_vt_*/tail,   // 1
                /*UTF8_fsm_vt_*/h2,     // 2
                /*UTF8_fsm_vt_*/h3,     // 3
                /*UTF8_fsm_vt_*/h4,     // 4
                /*UTF8_fsm_vt_*/h5,     // 5
                /*UTF8_fsm_vt_*/h6,     // 6
                /*UTF8_fsm_vt_*/bad_byte
            };

            extern const utf8_char_constant_type UTF8_table[];

            inline bool is_valid_ucs_value(u32t value, std::size_t codLen )
            {
                assert(codLen >= 1u && codLen <= 6u);
                return value >= UTF8_table[codLen].LOW && value <= UTF8_table[codLen].HIGH;
            }

            std::size_t get_ucs_code_length(u32t value);

            inline bool is_utf8_end_byte(char ch)
            {
                return (ch & UTF8_const<0>::MASK) == UTF8_const<0>::VALUE;
            }

            inline static utf8_stream_value_type to_value_type(u8t ch)
            {
                // byte 1111 111x is invalid
                if ((ch & 0xFE) == 0xFE)
                    return bad_byte;
                else
                    return utf8_stream_value_type(8u - sss::bit::highest_bit_pos(char(~(ch & 0xFF))));
            }

            template<typename Iterator> int next_length(Iterator it_beg, Iterator it_end)
            {
                if (std::distance(it_beg, it_end) <= 0) {
                    return 0;
                }

                uint8_t lead = *it_beg;
                utf8::utf8_stream_value_type lead_type = utf8::to_value_type(lead);

                switch (lead_type) {
                case utf8::h1:
                    return 1;
                    break;

                case utf8::tail:
                case utf8::bad_byte:
                    return 0;
                    break;

                default:
                    return int(lead_type);
                    break;
                }
            }

            template<typename Iterator> std::pair<uint32_t, int> peek(Iterator it_beg, Iterator it_end)
            {
                std::pair<uint32_t, int> ret = std::make_pair(0u, 0);
                uint32_t cp = 0u;

                if (it_beg != it_end) {
                    uint8_t lead = *it_beg;
                    utf8::utf8_stream_value_type lead_type = utf8::to_value_type(lead);

                    switch (lead_type) {
                    case utf8::h1:
                        ret.first = lead;
                        ret.second = 1;
                        break;

                    case utf8::tail:
                        SSS_POSTION_THROW(std::runtime_error,
                                          "require head byte but tail `" << ext::binary << uint32_t(lead) << "`");
                        break;

                    case utf8::bad_byte:
                        SSS_POSTION_THROW(std::runtime_error,
                                          "bad byte `" << ext::binary << lead << "`");
                        break;

                    default:
                        {
                            if (std::distance(it_beg, it_end) < lead_type) {
                                SSS_POSTION_THROW(std::runtime_error,
                                                  "not enough spaces for tail bytes; with lead `"
                                                  << std::hex << int(lead) << "`");
                            }

                            int tail_len = lead_type - 1;
                            cp = (lead & ~utf8::UTF8_table[lead_type].MASK & 0xFF) << (6 * tail_len);
                            uint8_t tail_byte = 0u;

                            switch (lead_type) {
#define UTF8_CHECK_PUSH(tail_byte, tail_len, it_beg, idx)                       \
                                tail_byte = *(it_beg + idx);                    \
                                if (utf8::tail != utf8::to_value_type(tail_byte)) {     \
                                    SSS_POSTION_THROW(std::runtime_error,       \
                                                      "require tail byte but `" << ext::binary << tail_byte << "`");       \
                                }                                               \
                                cp |= (tail_byte & ~utf8::UTF8_table[0].MASK & 0xFF) << (6 * (tail_len - idx));

                            case utf8::h6:
                                UTF8_CHECK_PUSH(tail_byte, tail_len, it_beg, 5);

                            case utf8::h5:
                                UTF8_CHECK_PUSH(tail_byte, tail_len, it_beg, 4);

                            case utf8::h4:
                                UTF8_CHECK_PUSH(tail_byte, tail_len, it_beg, 3);

                            case utf8::h3:
                                UTF8_CHECK_PUSH(tail_byte, tail_len, it_beg, 2);

                            case utf8::h2:
                                UTF8_CHECK_PUSH(tail_byte, tail_len, it_beg, 1);

                                ret.first = cp;
                                ret.second = tail_len + 1;
                                break;

                            default:
                                break;
#undef UTF8_CHECK_PUSH
                            }
                        }
                        break;
                    }
                }
                return ret;
            }

            inline std::pair<uint32_t, int> peek(const char * u8s)
            {
                const char * u8_beg = u8s;
                const char * u8_end = std::strchr(u8s, '\0');
                return peek(u8_beg, u8_end);
            }

            inline std::pair<uint32_t, int> peek(const std::string& u8s)
            {
                return peek(u8s.begin(), u8s.end());
            }

            template<typename Iterator> int count(Iterator it_beg, Iterator it_end)
            {
                int cnt = 0;
                while (std::distance(it_beg, it_end) > 0) {
                    std::pair<uint32_t, int> st = peek(it_beg, it_end);
                    if (!st.second) {
                        break;
                    }
                    cnt++;
                    std::advance(it_beg, st.second);
                }
                return cnt;
            }

            template<typename Iterator> int count_nocheck(Iterator it_beg, Iterator it_end)
            {
                int cnt = 0;
                bool is_ok = true;
                int code_len = 0;
                while (is_ok && (code_len = utf8::next_length(it_beg, it_end))) {
                    cnt++;
                    std::advance(it_beg, code_len);
                }
                return cnt;
            }

            // mono-font textwidth count
            // 遇到流结尾，或者遇见换行符的时候，终止循环；
            template<typename Iterator> int text_width(Iterator it_beg, Iterator it_end, int tabsize = 8)
            {
                if (tabsize < 0) {
                    tabsize = 8;
                }
                int width = 0;

                std::pair<uint32_t, int> st;

                while (((st = peek(it_beg, it_end)), st.second > 0) &&
                       *it_beg != '\n' &&
                       !sss::is_begin_with<Iterator, const char*>(it_beg, it_end, "\r\n", "\r\n" + 2))
                {
                    if (*it_beg == '\t') {
                        width = width - width % tabsize + tabsize;
                        it_beg++;
                        // width = ((width - width % 8) / 8 + 1) * 8;
                    }
                    else {
                        // FIXME
                        // 不是说有的两字节utf8字符，都占两个em宽！
                        //
                        // 比如&nbsp;，即&#160，相当于一个空格；也是占一个空格的宽度；
                        width += st.second > 1 ? 2 : 1;
                        std::advance(it_beg, st.second);
                    }
                }

                return width;
            }

            // NOTE
            // 返回的是，消耗了的字节数；
            template<typename IteratorIn, typename IteratorOut>
            int dumpout2ucs(IteratorIn it_beg, IteratorIn it_end, IteratorOut it_out)
            {
                IteratorIn it_beg_sv = it_beg;
                while (true) {
                    std::pair<uint32_t, int> st = peek(it_beg, it_end);
                    if (!st.second) {
                        break;
                    }
                    *it_out++ = st.first;
                    std::advance(it_beg, st.second);
                }
                return std::distance(it_beg_sv, it_beg);
            }

            template<typename IteratorIn, typename IteratorOut>
            int dumpout2utf8_once(IteratorIn it_beg, IteratorIn it_end, IteratorOut& it_out)
            {
                IteratorIn it_beg_sv = it_beg;
                if (std::distance(it_beg, it_end) > 0) {
                    uint32_t ucs_ch = *it_beg;
                    size_t code_len = get_ucs_code_length(ucs_ch);
                            
                    switch (code_len) {
                    case 1:
                        *it_out++ = ucs_ch & 0xFF;
                        break;

                    case 2:
                        *it_out++
                            = (((ucs_ch >> 6) & ~utf8::UTF8_table[code_len].MASK)
                            | utf8::UTF8_table[code_len].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;
                        break;

                    case 3:
                        *it_out++
                            = (((ucs_ch >> 12) & ~utf8::UTF8_table[code_len].MASK)
                            | utf8::UTF8_table[code_len].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 6) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;
                        break;

                    case 4:
                        *it_out++
                            = (((ucs_ch >> 18) & ~utf8::UTF8_table[code_len].MASK)
                            | utf8::UTF8_table[code_len].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 12) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 6) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;
                        break;

                    case 5:
                        *it_out++
                            = (((ucs_ch >> 24) & ~utf8::UTF8_table[code_len].MASK)
                            | utf8::UTF8_table[code_len].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 18) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 12) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 6) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;
                        break;

                    case 6:
                        *it_out++
                            = (((ucs_ch >> 30) & ~utf8::UTF8_table[code_len].MASK)
                            | utf8::UTF8_table[code_len].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 24) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 18) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 12) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 6) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;
                        break;
                    }
                    if (code_len) {
                        ++it_beg;
                    }
                }
                return std::distance(it_beg_sv, it_beg);
            }

            template<typename IteratorIn, typename IteratorOut>
            int dumpout2utf8(IteratorIn it_beg, IteratorIn it_end, IteratorOut it_out)
            {
                IteratorIn it_beg_sv = it_beg;
#if 0
                while (std::distance(it_beg, it_end) > 0) {
                    uint32_t ucs_ch = *it_beg;
                    size_t code_len = get_ucs_code_length(ucs_ch);
                    if (!code_len) {
                        break;
                    }
                            
                    switch (code_len) {
                    case 1:
                        *it_out++ = ucs_ch & 0xFF;
                        break;

                    case 2:
                        *it_out++
                            = (((ucs_ch >> 6) & ~utf8::UTF8_table[code_len].MASK)
                            | utf8::UTF8_table[code_len].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;
                        break;

                    case 3:
                        *it_out++
                            = (((ucs_ch >> 12) & ~utf8::UTF8_table[code_len].MASK)
                            | utf8::UTF8_table[code_len].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 6) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;
                        break;

                    case 4:
                        *it_out++
                            = (((ucs_ch >> 18) & ~utf8::UTF8_table[code_len].MASK)
                            | utf8::UTF8_table[code_len].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 12) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 6) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;
                        break;

                    case 5:
                        *it_out++
                            = (((ucs_ch >> 24) & ~utf8::UTF8_table[code_len].MASK)
                            | utf8::UTF8_table[code_len].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 18) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 12) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 6) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;
                        break;

                    case 6:
                        *it_out++
                            = (((ucs_ch >> 30) & ~utf8::UTF8_table[code_len].MASK)
                            | utf8::UTF8_table[code_len].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 24) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 18) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 12) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch >> 6) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;

                        *it_out++
                            = (((ucs_ch) & ~utf8::UTF8_table[0].MASK)
                            | utf8::UTF8_table[0].VALUE)
                            & 0xFF;
                        break;
                    }
                    ++it_beg;
                }
#else
                while (dumpout2utf8_once(it_beg, it_end, it_out)) {
                    it_beg++;
                    // FIXME it_out需要累加吗？
                }
#endif
                return std::distance(it_beg_sv, it_beg);
            }

            inline int dumpout2utf8_once(const uint32_t * ucs_str, char * out)
            {
                if (!ucs_str || !out) {
                    return 0;
                }
                return dumpout2utf8_once(ucs_str, ucs_str + 1, out);
            }


            template<typename Iterator> int text_width_nocheck(Iterator it_beg, Iterator it_end, int tabsize = 8)
            {
                if (tabsize < 0) {
                    tabsize = 8;
                }
                int width = 0;

                int code_len = 0;
                while ((code_len = utf8::next_length(it_beg, it_end)) &&
                       *it_beg != '\n' &&
                       !sss::is_begin_with(it_beg, it_end, "\r\n", "\r\n" + 2))
                {
                    if (*it_beg == '\t') {
                        width = width - width % tabsize + tabsize;
                        it_beg++;
                        // width = ((width - width % 8) / 8 + 1) * 8;
                    }
                    else {
                        // FIXME
                        // 不是说有的两字节utf8字符，都占两个em宽！
                        //
                        // 比如&nbsp;，即&#160，相当于一个空格；也是占一个空格的宽度；
                        width += code_len > 1 ? 2 : 1;
                        std::advance(it_beg, code_len);
                    }
                }

                return width;
            }

            inline int text_width(const std::string& s, int tabsize = 8)
            {
                return text_width(s.begin(), s.end(), tabsize);
            }

            inline int text_width_nocheck(const std::string& s, int tabsize = 8)
            {
                return text_width_nocheck(s.begin(), s.end(), tabsize);
            }

            inline int text_width(const char * s, int tabsize = 8)
            {
                if (!s) {
                    return 0;
                }
                return text_width(s, static_cast<const char*>(std::strchr(s, '\0')), tabsize);
            }

            inline int text_width_nocheck(const char * s, int tabsize = 8)
            {
                if (!s) {
                    return 0;
                }
                return text_width_nocheck(s, static_cast<const char*>(std::strchr(s, '\0')), tabsize);
            }
        }
    }
}



#endif /* __UTF8_H_1452945809__ */
