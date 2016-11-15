#ifndef __VALUE_MSG_HPP_1476947215__
#define __VALUE_MSG_HPP_1476947215__

#include <cstring>
#include <iostream>
#include <string>

#include <sss/raw_print.hpp>
#include <sss/string_view.hpp>

namespace sss {
namespace debug {
template <typename T>
class value_msg_t {
    template <typename TChar, typename TCharTraits, typename U>
    struct PrintHelper {
        static void print_to(std::basic_ostream<TChar, TCharTraits>& o,
                             const U& u)
        {
            o << u;
        }
    };

    template <typename TChar, typename TCharTraits>
    struct PrintHelper<TChar, TCharTraits, const char*> {
        static void print_to(std::basic_ostream<TChar, TCharTraits>& o,
                             const char* u)
        {
            o << sss::raw_string(u);
        }
    };
    template <typename TChar, typename TCharTraits>
    struct PrintHelper<TChar, TCharTraits, bool> {
        static void print_to(std::basic_ostream<TChar, TCharTraits>& o,
                             bool u)
        {
            if (u) {
                o.write("true", 4);
            } else {
                o.write("false", 5);
            }
        }
    };
    template <typename TChar, typename TCharTraits>
    struct PrintHelper<TChar, TCharTraits, std::string> {
        static void print_to(std::basic_ostream<TChar, TCharTraits>& o,
                             const std::string& u)
        {
            o << sss::raw_string(u);
        }
    };

    template <typename TChar, typename TCharTraits>
    struct PrintHelper<TChar, TCharTraits, char> {
        static void print_to(std::basic_ostream<TChar, TCharTraits>& o, char u)
        {
            o << sss::raw_char(u);
        }
    };

public:
    value_msg_t(const char* n, const T& v) : name(n), value(v) {}
public:
    void print(std::ostream& o) const
    {
        std::size_t len = std::strlen(name);
        if (len && name[0] == '(' && name[len - 1] == ')') {
            o.write(name + 1, len - 2);
        }
        else {
            o.write(name, len);
        }
        o.put(' ');
        o.put('=');
        o.put(' ');
        PrintHelper<char, std::char_traits<char>, T>::print_to(o, value);
    }

private:
    const char* name;
    const T& value;
};

template <>
class value_msg_t<raw_string_t<char>> {
public:
    value_msg_t(const char* n, const raw_string_t<char>& v) : name(n), value(v)
    {
    }

public:
    void print(std::ostream& o) const
    {
        std::size_t len = std::strlen(name);
        if (len && name[0] == '(' && name[len - 1] == ')') {
            o.write(name + 1, len - 2);
        }
        else {
            o.write(name, len);
        }
        o.put(' ');
        o.put('=');
        o.put(' ');
        o << value;
    }

private:
    const char* name;
    const raw_string_t<char> value;
};

template <typename T>
inline std::ostream& operator<<(std::ostream& o, const value_msg_t<T>& vm)
{
    vm.print(o);
    return o;
}

template <typename T>
inline value_msg_t<T> value_msg(const char* n, const T& v)
{
    return {n, v};
}

template<typename TChar, typename TCharTraits = std::char_traits<TChar> >
inline value_msg_t<sss::raw_string_t<TChar>> value_msg(const char* n,
                                                      const sss::basic_string_view<TChar, TCharTraits>& s)
{
    return {n, {s.data(), s.size()}};
}

inline value_msg_t<sss::raw_string_t<char>> value_msg(const char* n,
                                                      const char* s)
{
    return {n, {s, std::strlen(s)}};
}

}  // namespace debug

}  // namespace sss

#ifndef SSS_VALUE_MSG
#define SSS_VALUE_MSG(a) sss::debug::value_msg((#a), (a))
#endif

// TODO 增加格式控制宏；比如：
// VALUE_MSG_CFMT(a, f) sss::debug::value_msg_cfmt((#a), (a), (f))
// 其中，(f)表示sprintf中的格式；是字符串形式，可以表示宽度、对齐等信息；当然，还有fill-char和解析格式。
// 这样的话，必须的格式信息有：a. 解析格式
// 可选信息有：a. 宽度；b. 对其；c. fill-char 三个；
// 一共4组信息；
// 当然，由于是模板类，因此内部，已经知晓类型信息了。因此，按整数还是字符串来解释信息，
// 就有些多余——按debug目的，肯定是按照用户当时的使用逻辑习惯来打印信息，即，不可能要
// 求按整型打印字符串，或者按照字符串，打印数字。

#endif /* __VALUE_MSG_HPP_1476947215__ */
