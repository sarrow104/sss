#ifndef __RAW_PRINT_HPP_1475162433__
#define __RAW_PRINT_HPP_1475162433__

#include <cstdlib>
#include <initializer_list>
#include <iostream>

#include <sss/string_view.hpp>
#include <sss/stream/stream.hpp>
#include <sss/utlstring.hpp>

namespace sss {
// NOTE raw_string 只是一个wrapper模板函数。
//
// 它需要提供一个OStream模板对象；该模板对象，提供了基本的输出。
// 通过绑定不同的操作buffer(比如,std::ostream, std::FILE*,
// std::ostringstream,std::string)等，以达到动态书写的目的。
// 具体，参考rapidjson::BasicStream的实现。
//
// 并往上提供typedef 如：ostreamBuffer, ostringStreamBuffer,stringBuffer
// 另外，使用上，有两种方案。一种是 "<<" 风格；还有一种是模板变参Arg...风格。
// 前者，虽然在设置NODEBUG之后，仍然可能优化为若干函数调用。但后者，完全有可能置空操作！

// raw_print
// 模仿 pretyprint
// 可视化 '\t','\n','\r'等字符。
//
// 另外，可参考自己的 varlisp，cppcolor 工程。

// 合法的转义字符有：（见iso C99 p19）
// \a: alert
// \b: backspace
// \f: form feed
// \n: new line
// \r: carriage return
// \t: horizontal tab
// \v: vertical tab

// NOTE std::initialize_list{} 可以保持参数原有类型——当然，模板类也可以。
// 可以参考 sqt::layout
/**
 * @brief 用来wrapper被输出对象。
 */
template <typename T>
class raw_print_t;

template <typename T>
class raw_print_t {
public:
    void print(std::ostream& o);
};

// template <typename T, size_t Len>
// class raw_print_t {
// };

template <typename T>
raw_print_t<T> raw_print(const T& v)
{
    return {v};
}

// NOTE
// 有两种使用方式，一种是to_string；另外一种是输出到std::ostream或者FILE中。
// std::cout << sss::raw_string("") << std::endl;

namespace detail {
template <typename GenericOutputStream, typename Tchar>
void raw_print_char(GenericOutputStream& stream, Tchar ch)
{
    switch (ch) {
        case '\0':
            stream.put("\\0", 2);
            break;
        case '\a':
            stream.put("\\a", 2);
            break;
        case '\b':
            stream.put("\\b", 2);
            break;
        case '\f':
            stream.put("\\f", 2);
            break;
        case '\n':
            stream.put("\\n", 2);
            break;
        case '\r':
            stream.put("\\r", 2);
            break;
        case '\t':
            stream.put("\\t", 2);
            break;
        case '\v':
            stream.put("\\v", 2);
            break;
        case '"':
            stream.put("\\\"", 2);
            break;
        case '\\':
            stream.put("\\\\", 2);
            break;
        default:
            if (!(ch & 0x80) && !std::isprint(ch)) {
                stream.put("\\x", 2);
                stream.putChar(sss::lower_hex2char(ch >> 4));
                stream.putChar(sss::lower_hex2char(ch));
            }
            else {
                stream.putChar(ch);
            }
    }
}
}  // namespace detail

template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
class raw_char_t {
public:
    explicit raw_char_t(const TChar c) : _c(c) {}
    raw_char_t(const raw_char_t& ref) : _c(ref._c) {}
    ~raw_char_t() {}
public:
    raw_char_t& operator=(const raw_char_t& ref) = default;

public:
    std::basic_string<TChar, TCharTraits> to_string() const
    {
        std::string buf;
        GenericOutputStream<std::basic_string<TChar, TCharTraits>> w{buf};
        raw_print(w);
        return buf;
    }
    void print(std::basic_ostream<TChar, TCharTraits>& o) const
    {
        GenericOutputStream<std::basic_ostream<TChar, TCharTraits>> stream{o};
        raw_print(stream);
    }

    void print(std::FILE* f) const
    {
        GenericOutputStream<std::FILE> stream{f};
        raw_print(stream);
    }

protected:
    template <typename GenericOutputStream>
    void raw_print(GenericOutputStream& stream) const
    {
        stream.putChar('\'');
        detail::raw_print_char(stream, this->_c);
        stream.putChar('\'');
    }

private:
    const char _c;
};

template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
class raw_string_t {
public:
    raw_string_t(const TChar* s, size_t len) : _s(s), _len(len) {}
    raw_string_t(const raw_string_t& ref) : _s(ref._s), _len(ref._len) {}
    ~raw_string_t() {}
public:
    raw_string_t& operator=(const raw_string_t& ref)
    {
        if (this != &ref) {
            this->_s = ref._s;
            this->_len = ref._len;
        }
    };

public:
    std::basic_string<TChar, TCharTraits> to_string() const
    {
        std::string buf;
        GenericOutputStream<std::basic_string<TChar, TCharTraits>> w{buf};
        raw_print(w);
        return buf;
    }
    void print(std::basic_ostream<TChar, TCharTraits>& o) const
    {
        GenericOutputStream<std::basic_ostream<TChar, TCharTraits>> stream{o};
        raw_print(stream);
    }

    void print(std::FILE* f) const
    {
        GenericOutputStream<std::FILE> stream{f};
        raw_print(stream);
    }

protected:
    template <typename GenericOutputStream>
    void raw_print(GenericOutputStream& stream) const
    {
        const char null[] = "NULL";
        if (!this->_s && this->_len) {
            stream.put(null, sizeof(null) - 1);
        }
        else {
            stream.putChar('"');
            for (const TChar* pc = this->_s; pc < this->_s + this->_len; ++pc) {
                detail::raw_print_char(stream, *pc);
            }
            stream.putChar('"');
        }
    }

private:
    const char* _s;
    std::size_t _len;
};

// NOTE raw_string("abc") 也会选择这个函数
template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
raw_string_t<TChar, TCharTraits> raw_string(const TChar* s)
{
    return raw_string_t<TChar, TCharTraits>{s, s ? TCharTraits::length(s) : 0u};
}
// template <typename TChar, typename TCharTraits = std::char_traits<TChar>,
// size_t len>
// raw_string_t<TChar, TCharTraits> raw_string(const char(&s)[len])
// {
//     return raw_string_t<TChar, TCharTraits>{s, len};
// }
template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
raw_string_t<TChar, TCharTraits> raw_string(const TChar* s, size_t len)
{
    return raw_string_t<TChar, TCharTraits>{s, len};
}

template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
raw_string_t<TChar, TCharTraits> raw_string(sss::basic_string_view<TChar, TCharTraits> s)
{
    return raw_string_t<TChar, TCharTraits>{s.data(), size_t(s.size())};
}

template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
raw_string_t<TChar, TCharTraits> raw_string(
    const std::basic_string<TChar, TCharTraits>& s)
{
    return raw_string_t<TChar, TCharTraits>{s.c_str(), s.length()};
}

template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
std::basic_ostream<TChar, TCharTraits>& operator<<(
    std::basic_ostream<TChar, TCharTraits>& o,
    const raw_string_t<TChar, TCharTraits>& rs)
{
    rs.print(o);
    return o;
}

template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
raw_char_t<TChar, TCharTraits> raw_char(TChar c)
{
    return raw_char_t<TChar, TCharTraits>{c};
}

template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
std::basic_ostream<TChar, TCharTraits>& operator<<(
    std::basic_ostream<TChar, TCharTraits>& o,
    const raw_char_t<TChar, TCharTraits>& rs)
{
    rs.print(o);
    return o;
}
}  // namespace sss

#endif /* __RAW_PRINT_HPP_1475162433__ */
