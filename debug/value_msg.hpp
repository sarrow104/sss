#ifndef __VALUE_MSG_HPP_1476947215__
#define __VALUE_MSG_HPP_1476947215__

#include <cstring>
#include <iostream>
#include <string>

#include <sss/raw_print.hpp>

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

inline value_msg_t<sss::raw_string_t<char>> value_msg(const char* n,
                                                      const char* s)
{
    return {n, {s, std::strlen(s)}};
}

}  // namespace debug

}  // namespace sss

#ifndef VALUE_MSG
#define VALUE_MSG(a) sss::debug::value_msg((#a), (a))
#endif

#endif /* __VALUE_MSG_HPP_1476947215__ */
