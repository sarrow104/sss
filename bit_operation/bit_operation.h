// howto find the position of the highest none zero bit of a
//particularly plain date type? eg: char, int, short etc.
#ifndef  __BIT_OPERATION_H__
#define  __BIT_OPERATION_H__

#ifdef   _MSC_VER
typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned int            uint32_t;
typedef unsigned long long      uint64_t;
#else
#   include <stdint.h>//for:uint8_t, uint16_t, uint32_t, uint64_t
#endif

#include <cstring>
#include <limits>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <bitset>

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

inline size_t round_8(size_t bytes) { return (bytes + 7UL) & ~7UL; }

//! http://blog.chinaunix.net/xmlrpc.php?r=blog/article&uid=25272011&id=3658875
// NOTE m,n 取值范围 [0, 31]
inline uint32_t swapbit(uint32_t data, int m, int n)
{
    return (data & (1 << m)) == (data & (1 << n)) ? data : data ^ ((1 << m) | (1 << n));
}

template<size_t i> struct byte_type;//{{{1

template<> struct byte_type<1> //for one byte type!{{{1
{
    typedef uint32_t value_type;
    typedef uint8_t ret_t;

    static inline value_type bit_reverse_impl(value_type c)//{{{2
    {
        c = ((c & 0x55555555u) <<  1) | ((c >>  1) & 0x55555555u);
        c = ((c & 0x33333333u) <<  2) | ((c >>  2) & 0x33333333u);
        c = ((c & 0x0F0F0F0Fu) <<  4) | ((c >>  4) & 0x0F0F0F0Fu);

        return c;
    }

    static inline size_t count_1_bit_impl(value_type c)
    {
        c = (c & 0x55555555u) + ((c >>  1) & 0x55555555u);
        c = (c & 0x33333333u) + ((c >>  2) & 0x33333333u);
        c = (c & 0x0F0F0F0Fu) + ((c >>  4) & 0x0F0F0F0Fu);

        return c;
    }

    static inline value_type on_low_bits_impl(value_type c)
    {
        c |= (c >> 1);//1000 0000 -> 1100 0000
        c |= (c >> 2);//1100 0000 -> 1111 0000
        c |= (c >> 4);//1111 0000 -> 1111 1111

        return c;
    }//}}}2
};

template<> struct byte_type<2> //for two byte type!{{{1
{
    typedef uint32_t value_type;
    typedef uint16_t ret_t;

    static inline value_type bit_reverse_impl(value_type c)//{{{2
    {
        c = byte_type<1>::bit_reverse_impl(c);
        c = ((c & 0x00FF00FF) << 8) | ((c >> 8) & 0x00FF00FF);

        return c;
    }

    static inline size_t count_1_bit_impl(value_type c)
    {
        c = byte_type<1>::count_1_bit_impl(c);
        c = (c & 0x00FF00FF) + ((c >>  8) & 0x00FF00FF);

        return c;
    }

    static inline value_type on_low_bits_impl(value_type c)
    {
        c = byte_type<1>::on_low_bits_impl(c);
        c |= (c >> 8);

        return c;
    }//}}}
};

template<> struct byte_type<4> //for four byte type!{{{1
{
    typedef uint32_t value_type;
    typedef uint32_t ret_t;

    static inline value_type bit_reverse_impl(value_type c)//{{{2
    {
        c = byte_type<2>::bit_reverse_impl(c);
        c = ((c & 0x0000FFFF) << 16) | ((c >> 16) & 0x0000FFFF);

        return c;
    }

    static inline size_t count_1_bit_impl(value_type c)
    {
        c = byte_type<2>::count_1_bit_impl(c);
        c = (c & 0x0000FFFF) + ((c >> 16) & 0x0000FFFF);

        return c;
    }

    static inline value_type on_low_bits_impl(value_type c)
    {
        c = byte_type<2>::on_low_bits_impl(c);
        c |= (c >> 16);

        return c;
    }//}}}
};

template<> struct byte_type<8> //for eight byte type!{{{1
{
    typedef uint64_t value_type;
    typedef uint64_t ret_t;

    static inline value_type bit_reverse_impl(value_type c)//{{{2
    {
        c = ((c & 0x5555555555555555LL) <<  1) | ((c >>  1) & 0x5555555555555555LL);
        c = ((c & 0x3333333333333333LL) <<  2) | ((c >>  2) & 0x3333333333333333LL);
        c = ((c & 0x0F0F0F0F0F0F0F0FLL) <<  4) | ((c >>  4) & 0x0F0F0F0F0F0F0F0FLL);
        c = ((c & 0x00FF00FF00FF00FFLL) <<  8) | ((c >>  8) & 0x00FF00FF00FF00FFLL);
        c = ((c & 0x0000FFFF0000FFFFLL) << 16) | ((c >> 16) & 0x0000FFFF0000FFFFLL);
        c = ((c & 0x00000000FFFFFFFFLL) << 32) | ((c >> 32) & 0x00000000FFFFFFFFLL);

        return c;
    }

    static inline size_t count_1_bit(value_type c)
    {
        c = (c & 0x5555555555555555LL) + ((c >>  1) & 0x5555555555555555LL);
        c = (c & 0x3333333333333333LL) + ((c >>  2) & 0x3333333333333333LL);
        c = (c & 0x0F0F0F0F0F0F0F0FLL) + ((c >>  4) & 0x0F0F0F0F0F0F0F0FLL);
        c = (c & 0x00FF00FF00FF00FFLL) + ((c >>  8) & 0x00FF00FF00FF00FFLL);
        c = (c & 0x0000FFFF0000FFFFLL) + ((c >> 16) & 0x0000FFFF0000FFFFLL);
        c = (c & 0x00000000FFFFFFFFLL) + ((c >> 32) & 0x00000000FFFFFFFFLL);

        return static_cast<size_t>(c);
    }

    static inline value_type on_low_bits_impl(value_type c)
    {
        c = byte_type<4>::on_low_bits_impl(static_cast<byte_type<4>::value_type>(c));
        c |= (c >> 32);

        return c;
    }//}}}
};
// NOW, operate on type flowt, double, even user defined struct (size below .8.){{{1
//      is OK!

//off and on the lowest bit
template<typename T> inline typename byte_type<sizeof(T)>::value_type off_lowest_bit(T d)//{{{1
{
    typedef typename byte_type<sizeof(T)>::value_type value_type;
    value_type _v = reinterpret_cast<value_type>(d);
    return _v &= (_v - 1u);
}

template<typename T> inline typename byte_type<sizeof(T)>::value_type on_lowest_bit(T d)//{{{1
{
    typedef typename byte_type<sizeof(T)>::value_type value_type;
    value_type _v = reinterpret_cast<value_type>(d);
    return _v |= (_v - 1u);
}

template<typename T> inline bool is_power_of_2(T d)//{{{1
{
    typedef typename byte_type<sizeof(T)>::value_type value_type;
    value_type _v = reinterpret_cast<value_type>(d);
    return !(_v & (_v - 1u)) && _v;
    //  consider Zero as a power of 2 value, too!
}

template<typename T> typename byte_type<sizeof(T)>::ret_t bit_reverse(T c)//{{{1
{
    typedef typename byte_type<sizeof(T)>::value_type value_type;
    typedef typename byte_type<sizeof(T)>::ret_t ret_t;

    ret_t ret = byte_type<sizeof(T)>::bit_reverse_impl(static_cast<value_type>(c));

    return ret;
}

template<typename T> size_t count_1_bit(T c)//{{{1
{
    typedef typename byte_type<sizeof(T)>::value_type value_type;

    size_t ret = byte_type<sizeof(T)>::count_1_bit_impl(static_cast<value_type>(c));

    return ret;
}

// return value,
//  0           : there is no 1 bit
//  none zero   : the left most 1 bit pos
template<typename T> size_t highest_bit_pos(T c)//{{{1
{
    typedef typename byte_type<sizeof(T)>::value_type value_type;

    typename byte_type<sizeof(T)>::ret_t ret
        = byte_type<sizeof(T)>::on_low_bits_impl(static_cast<value_type>(c));

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
            c &= (c - 1);
        }
    }

    return ret;
}

template<typename T>
class hex_out_t
{
public:
    explicit hex_out_t(const T& val)
        : _data(val)
    {
    }
    ~hex_out_t()
    {
    }

public:
    void print(std::ostream& o) const
    {
        const uint8_t * pdata = reinterpret_cast<const uint8_t*>(&_data);
        o << "0x";
        for (int i = 0; i < sizeof(T); ++i) {
            o << std::hex << std::setw(2) << std::setfill('0') << (pdata[i] & 0xFFu);
        }
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
    private:
        //data member list:
        std::ostream& _o;
    public:
        //method member list:
        binary_out_t(std::ostream& o) : _o(o) { }

        //TODO
        //  本函数可以用查找表进行少量的优化
        //  char * bin_bit[] = {
        //      "0000",
        //      "0001",
        //      ...
        //      "1110"};
        //      "1111"};
        template<typename T>
            std::ostream& operator<<(T d)
            {
                typedef typename sss::bit::byte_type<sizeof(T)>::ret_t value_type;
                value_type & v = reinterpret_cast<value_type&>(d);
                for ( size_t i = sizeof(T) * 8; i != 0 ; --i) {
                    char c =  ((v >> value_type(i - 1)) & value_type(1u)) + '0';
                    _o << c;
                }
                return this->_o;
            }
    };

    inline binary_out_t binary(std::ostream& o)
    {
        return binary_out_t(o);
    }

template<>
inline
std::ostream& binary_out_t::operator<< <float>(float d)
{
    typedef uint32_t value_type;
    value_type v;
    memcpy(&v, &d, sizeof(value_type));
    for ( size_t i = 32; i != 0 ; --i) {
        char c =  ((v >> value_type(i - 1)) & value_type(1u)) + '0';
        _o << c;
        if ( i == 32 || i == 24 ) {
            _o << ' ';
        }
    }
    return this->_o;
}

template<>
inline
std::ostream& binary_out_t::operator << <double>(double d)
{
    typedef uint64_t value_type;
    value_type v;
    memcpy(&v, &d, sizeof(value_type));
    for ( size_t i = 64; i != 0 ; --i) {
        char c =  ((v >> value_type(i - 1)) & value_type(1u)) + '0';
        _o << c;
        if ( i == 64 || i == 53 ) {
            _o << ' ';
        }
    }
    return this->_o;
}
}

template<typename ReturnType>
inline ReturnType operator<< (std::ostream& o, ReturnType (*func_oper)(std::ostream&) )//{{{1
{
    return func_oper(o);
}

#endif  /*__BIT_OPERATION_H__*/
