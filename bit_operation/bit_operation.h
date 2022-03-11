// howto find the position of the highest none zero bit of a
//particularly plain date type? eg: char, int, short etc.
#ifndef  __BIT_OPERATION_H__
#define  __BIT_OPERATION_H__

#include <bitset>
#include <cstdint> //for:uint8_t, uint16_t, uint32_t, uint64_t
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>

/*TODO{{{1
添加
size_t highest_0_bit(T c);
size_t lowest_0_bit(T c);
size_t highest_1_bit(T c);
size_t lowest_1_bit(T c);

NOTE 上述看起来是4个函数，其实只有两个；因为通过bit取反，可以将highest_1_bit转
换为highest_0_bit的求解；

FIXME 2012-10-07

/bit_operation.h: In member function 'std::ostream& ext::binary_out_t::operator<<(T) [with T = float]':
/bit_operation.h:287: warning: dereferencing type-punned pointer will break strict-aliasing rules
/bit_operation.h: In member function 'std::ostream& ext::binary_out_t::operator<<(T) [with T = double]':
/bit_operation.h:303: warning: dereferencing type-punned pointer will break strict-aliasing rules

这个警告是因为 gcc 默认开启内存对齐，而 reinterpret_cast<> 可能误判边界；

"参考"下面的解决方案：
http://stackoverflow.com/questions/4163126/c-dereferencing-type-punned-pointer-will-break-strict-aliasing-rules-warnin

1. Yes. GCC will assume that the pointers cannot alias. For instance, if you
   assign through one then read from the other, GCC may, as an optimisation,
   reorder the read and write - I have seen this happen in production code, it
   is not pleasant to debug.

2. Several. You could use a union to represent the memory you need to
   reinterpret. You could use a reinterpret_cast. You could cast via char * at
   the point where you reinterpret the memory - char * are defined as being
   able to alias anything. You could use a type which has
   __attribute__((__may_alias__)).  You could turn off the aliasing assumptions
   globally using -fno-strict-aliasing.

3. __attribute__((__may_alias__)) on the types used is probably the closest you
   can get to disabling the assumption for a particular section of code.

}}} */

//2009-06-16
namespace sss { namespace bit {

// 判断系统字节序：
// 如果ENDIANNESS='l' 表示系统为little-endian; 为'b'表示big-endian
// 返回'?' 表示未知
char bit_order();

static const auto FOUR_BIT_MASK = 0x7UL;
static const auto HEX_BIT_MASK = 0x0FU;

inline size_t round_8(size_t bytes) { return (bytes + FOUR_BIT_MASK) & ~FOUR_BIT_MASK; }

//! http://blog.chinaunix.net/xmlrpc.php?r=blog/article&uid=25272011&id=3658875
// NOTE m,n 取值范围 [0, 31]
inline uint32_t swapbit(uint32_t data, uint8_t m, uint8_t n)
{
    return (data & (1U << m)) == (data & (1U << n)) ? data : data ^ ((1U << m) | (1U << n));
}

template<size_t i> struct byte_type;//{{{1

template<> struct byte_type<1> //for one byte type!{{{1
{
    using value_type = uint32_t;
    using ret_t = uint8_t;
    static const auto mask_level_1 = 0x55555555U;
    static const auto mask_level_2 = 0x33333333U;
    static const auto mask_level_3 = 0x0F0F0F0FU;

    static inline value_type bit_reverse_impl(value_type c)//{{{2
    {
        c = ((c & mask_level_1) << 1U) | ((c >>  1U) & mask_level_1);
        c = ((c & mask_level_2) << 2U) | ((c >>  2U) & mask_level_2);
        c = ((c & mask_level_3) << 4U) | ((c >>  4U) & mask_level_3);

        return c;
    }

    static inline size_t count_1_bit_impl(value_type c)
    {
        c = (c & value_type(mask_level_1)) + ((c >>  1U) & value_type(mask_level_1));
        c = (c & value_type(mask_level_2)) + ((c >>  2U) & value_type(mask_level_2));
        c = (c & value_type(mask_level_3)) + ((c >>  4U) & value_type(mask_level_3));

        return c;
    }

    static inline value_type on_low_bits_impl(value_type c)
    {
        c |= (c >> 1U);//1000 0000 -> 1100 0000
        c |= (c >> 2U);//1100 0000 -> 1111 0000
        c |= (c >> 4U);//1111 0000 -> 1111 1111

        return c;
    }//}}}2
};

template<> struct byte_type<2> //for two byte type!{{{1
{
    using value_type = uint32_t;
    using ret_t = uint16_t;

    static const auto mask_level_4 = 0x00FF00FF;
    static const auto level_4 = 8U;

    static inline value_type bit_reverse_impl(value_type c)//{{{2
    {
        c = byte_type<1>::bit_reverse_impl(c);
        c = ((c & value_type(mask_level_4)) << level_4) | ((c >> level_4) & value_type(mask_level_4));

        return c;
    }

    static inline size_t count_1_bit_impl(value_type c)
    {
        c = byte_type<1>::count_1_bit_impl(c);
        c = (c & value_type(mask_level_4)) + ((c >>  level_4) & value_type(mask_level_4));

        return c;
    }

    static inline value_type on_low_bits_impl(value_type c)
    {
        c = byte_type<1>::on_low_bits_impl(c);
        c |= (c >> level_4);

        return c;
    }//}}}
};

template<> struct byte_type<4> //for four byte type!{{{1
{
    using value_type = uint32_t;
    using ret_t = uint32_t;
    static const auto mask_level_5 = 0x0000FFFFU;
    static const auto level_5 = 16U;

    static inline value_type bit_reverse_impl(value_type c)//{{{2
    {
        c = byte_type<2>::bit_reverse_impl(c);
        c = ((c & mask_level_5) << level_5) | ((c >> level_5) & mask_level_5);

        return c;
    }

    static inline size_t count_1_bit_impl(value_type c)
    {
        c = byte_type<2>::count_1_bit_impl(c);
        c = (c & mask_level_5) + ((c >> level_5) & mask_level_5);

        return c;
    }

    static inline value_type on_low_bits_impl(value_type c)
    {
        c = byte_type<2>::on_low_bits_impl(c);
        c |= (c >> level_5);

        return c;
    }//}}}
};

template<> struct byte_type<8> //for eight byte type!{{{1
{
    using value_type = uint64_t;
    using ret_t = uint64_t;

    static const auto mask_level_1 = 0x5555555555555555LLU;
    static const auto mask_level_2 = 0x3333333333333333LLU;
    static const auto mask_level_3 = 0x0F0F0F0F0F0F0F0FLLU;
    static const auto mask_level_4 = 0x00FF00FF00FF00FFLLU;
    static const auto mask_level_5 = 0x0000FFFF0000FFFFLLU;
    static const auto mask_level_6 = 0x00000000FFFFFFFFLLU;

    static const auto level_1 =  1U;
    static const auto level_2 =  2U;
    static const auto level_3 =  4U;
    static const auto level_4 =  8U;
    static const auto level_5 = 16U;
    static const auto level_6 = 32U;

    static inline value_type bit_reverse_impl(value_type c)//{{{2
    {
        c = ((c & mask_level_1) << level_1) | ((c >> level_1) & mask_level_1);
        c = ((c & mask_level_2) << level_2) | ((c >> level_2) & mask_level_2);
        c = ((c & mask_level_3) << level_3) | ((c >> level_3) & mask_level_3);
        c = ((c & mask_level_4) << level_4) | ((c >> level_4) & mask_level_4);
        c = ((c & mask_level_5) << level_5) | ((c >> level_5) & mask_level_5);
        c = ((c & mask_level_6) << level_6) | ((c >> level_6) & mask_level_6);

        return c;
    }

    static inline size_t count_1_bit(value_type c)
    {
        c = (c & mask_level_1) + ((c >> level_1) & mask_level_1);
        c = (c & mask_level_2) + ((c >> level_2) & mask_level_2);
        c = (c & mask_level_3) + ((c >> level_3) & mask_level_3);
        c = (c & mask_level_4) + ((c >> level_4) & mask_level_4);
        c = (c & mask_level_5) + ((c >> level_5) & mask_level_5);
        c = (c & mask_level_6) + ((c >> level_6) & mask_level_6);

        return static_cast<size_t>(c);
    }

    static inline size_t count_1_bit_impl(value_type c)
    {
        c = (c & mask_level_1) + ((c >> level_1) & mask_level_1);
        c = (c & mask_level_2) + ((c >> level_2) & mask_level_2);
        c = (c & mask_level_3) + ((c >> level_3) & mask_level_3);
        c = (c & mask_level_4) + ((c >> level_4) & mask_level_4);
        c = (c & mask_level_5) + ((c >> level_5) & mask_level_5);
        c = (c & mask_level_6) + ((c >> level_6) & mask_level_6);

        return c;
    }

    static inline value_type on_low_bits_impl(value_type c)
    {
        c = byte_type<4>::on_low_bits_impl(static_cast<byte_type<4>::value_type>(c));
        c |= (c >> level_6);

        return c;
    }//}}}
};

// NOW, operate on type flowt, double, even user defined struct (size below .8.){{{1
//      is OK!

//off and on the lowest bit
template<typename T> inline typename byte_type<sizeof(T)>::value_type off_lowest_bit(T d)//{{{1
{
    using value_type = typename byte_type<sizeof(T)>::value_type;
    value_type _v = reinterpret_cast<value_type&>(d);
    return _v &= (_v - 1U);
}

template<typename T> inline typename byte_type<sizeof(T)>::value_type on_lowest_bit(T d)//{{{1
{
    using value_type = typename byte_type<sizeof(T)>::value_type;
    value_type _v = reinterpret_cast<value_type&>(d);
    return _v |= (_v - 1U);
}

template<typename T> inline bool is_power_of_2(T d)//{{{1
{
    using value_type = typename byte_type<sizeof(T)>::value_type;
    value_type _v = reinterpret_cast<value_type&>(d);
    return !(_v & (_v - 1U)) && _v;
    //  consider Zero as a power of 2 value, too!
}

template<typename T> typename byte_type<sizeof(T)>::ret_t bit_reverse(T c)//{{{1
{
    using value_type = typename byte_type<sizeof(T)>::value_type;
    using ret_t = typename byte_type<sizeof(T)>::ret_t;

    ret_t ret = byte_type<sizeof(T)>::bit_reverse_impl(static_cast<value_type>(c));

    return ret;
}

template<typename T> size_t count_1_bit(T c)//{{{1
{
    using value_type = typename byte_type<sizeof(T)>::value_type;

    size_t ret = byte_type<sizeof(T)>::count_1_bit_impl(static_cast<value_type>(c));

    return ret;
}

// return value,
//  0           : there is no 1 bit
//  none zero   : the left most 1 bit pos
template<typename T> size_t highest_bit_pos(T c)//{{{1
{
    using value_type = typename byte_type<sizeof(T)>::value_type;

    typename byte_type<sizeof(T)>::ret_t ret
        = byte_type<sizeof(T)>::on_low_bits_impl(static_cast<value_type>(c));

    return count_1_bit(ret);
}

template<typename T> size_t lowest_bit_pos(T c) // {{{1
{
    using value_type = typename byte_type<sizeof(T)>::value_type;

    value_type _v = static_cast<T>(c) ^ off_lowest_bit(c);

    typename byte_type<sizeof(T)>::ret_t ret
        = byte_type<sizeof(T)>::on_low_bits_impl(_v);

    return count_1_bit(ret);
}

template<typename T> size_t _count_1_bit(T c)//for test suit use{{{1
{
    size_t ret = 0;
    char * pstr = reinterpret_cast<char*>(&c);
    for ( size_t i = 0; i < sizeof(T); ++i) {
        char c = pstr[i];
        while ( c )
        {
            ret++;
            c &= (c - 1U);
        }
    }

    return ret;
}

template<typename T>
void bit_off(T& v, size_t pos)
{
    using value_type = typename byte_type<sizeof(T)>::ret_t;
    value_type& ref = reinterpret_cast<value_type&>(v);
    ref &= (~value_type(1) << pos);
}

template<typename T>
void bit_on(T& v, size_t pos)
{
    using value_type = typename byte_type<sizeof(T)>::ret_t;
    value_type& ref = reinterpret_cast<value_type&>(v);
    ref |= (value_type(1) << pos);
}


void buff_hex_reverse_print(std::ostream& out, const uint8_t * pdata, size_t size);

template<typename T>
class hex_out_t
{
public:
    explicit hex_out_t(const T& val)
        : _data(val) {}
    hex_out_t(const hex_out_t&) = default;
    hex_out_t(hex_out_t&&) noexcept = default;

    hex_out_t& operator=(const hex_out_t&) = default;
    hex_out_t& operator=(hex_out_t&&) noexcept = default;

    ~hex_out_t() = default;

//public:
    void print(std::ostream& o) const
    {
        const auto * pdata = reinterpret_cast<const uint8_t*>(&_data);
        o << "0x";
        ::sss::bit::buff_hex_reverse_print(o, pdata, sizeof(T));
    }

private:
    const T& _data;
};

template<>
class hex_out_t<std::string>
{
public:
    using T = std::string;

    explicit hex_out_t(const T& val)
        : _data(val) {}
    hex_out_t(const hex_out_t&) = default;
    hex_out_t(hex_out_t&&) noexcept = default;

    //hex_out_t& operator=(const hex_out_t&) = default;
    //hex_out_t& operator=(hex_out_t&&) noexcept = default;
    ~hex_out_t() = default;

//public:
    void print(std::ostream& o) const
    {
        const auto * pdata = reinterpret_cast<const uint8_t*>(_data.data());
        o << "0x"; // NOTE byte order!
        ::sss::bit::buff_hex_reverse_print(o, pdata, _data.size());
    }

private:
    const T& _data;
};

template<typename T>
hex_out_t<T> hex_out(const T& val)
{
    return hex_out_t<T>(val);
}

template<typename T>
std::ostream & operator << (std::ostream & o, const hex_out_t<T>& b)
{
    b.print(o);
    return o;
}

} //namespace bit
} //namespace sss

namespace ext // binary out put for std::ostream{{{1
{
    class binary_out_t
    {
        static const char * binary_table[];
        // NOTE
        //  {
        //      "0000",
        //      "0001",
        //      ...
        //      "1110",
        //      "1111"
        // };
    private:
        //data member list:
        std::ostream& _o;

    public:
        //method member list:
        binary_out_t(std::ostream& o) : _o(o) { }

        template<typename T>
            std::ostream& operator<<(T d)
            {
                using value_type = typename sss::bit::byte_type<sizeof(T)>::ret_t;
                auto & v = reinterpret_cast<value_type&>(d);
                for ( size_t i = sizeof(T) * 2; i != 0 ; --i) {
                    int idx =  ((v >> (4 * value_type(i - 1))) & value_type(sss::bit::HEX_BIT_MASK));
                    _o << binary_table[idx];
                }
                return this->_o;
            }
    };

    inline binary_out_t binary(std::ostream& o)
    {
        return binary_out_t{o};
    }

template<>
inline
std::ostream& binary_out_t::operator<< <float>(float d)
{
    using value_type = uint32_t;
    value_type v = reinterpret_cast<const value_type&>(d);
    const auto bit_count = CHAR_BIT * sizeof(value_type);

    for ( size_t i = bit_count; i != 0 ; --i) {
        char c = char(((v >> value_type(i - 1)) & value_type(1U)) + '0');
        _o << c;
        if ( i == bit_count || i == bit_count - CHAR_BIT) {
            _o << ' ';
        }
    }
    return this->_o;
}

template<>
inline
std::ostream& binary_out_t::operator << <double>(double d)
{
    using value_type = uint64_t;
    value_type v = reinterpret_cast<const value_type&>(d);
    const auto bit_count = CHAR_BIT * sizeof(value_type);
    for ( size_t i = bit_count; i != 0 ; --i) {
        char c =  char(((v >> value_type(i - 1)) & value_type(1U)) + '0');
        _o << c;
        if ( i == bit_count || i == (bit_count - CHAR_BIT - 3) ) {
            _o << ' ';
        }
    }
    return this->_o;
}
} // namespace ext

template<typename ReturnType>
inline ReturnType operator<< (std::ostream& o, ReturnType (*func_oper)(std::ostream&) )//{{{1
{
    return func_oper(o);
}

#endif  /*__BIT_OPERATION_H__*/
