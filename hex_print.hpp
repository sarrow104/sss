#ifndef __HEX_PRINT_HPP_1476611331__
#define __HEX_PRINT_HPP_1476611331__

#include <cstdlib>
#include <iosfwd>
#include <type_traits>

#include <sss/stream/stream.hpp>
#include <sss/utlstring.hpp>
#include <sss/string_view.hpp>

namespace sss {
template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
class hex_string_t {
public:
    template <typename T>
    explicit hex_string_t(const T& v)
        : _s(reinterpret_cast<const char*>(&v)), _len(sizeof(T))
    {
    }
    hex_string_t(const TChar* s, size_t len) : _s(s), _len(len) {}
    std::basic_string<TChar, TCharTraits> to_string() const
    {
        std::string buf;
        GenericOutputStream<std::basic_string<TChar, TCharTraits>> w{buf};
        hex_print(w);
        return buf;
    }
    void print(std::basic_ostream<TChar, TCharTraits>& o) const
    {
        GenericOutputStream<std::basic_ostream<TChar, TCharTraits>> stream{o};
        hex_print(stream);
    }

    void print(std::FILE* f) const
    {
        GenericOutputStream<std::FILE> stream{f};
        hex_print(stream);
    }

protected:
    template <typename GenericOutputStream>
    void hex_print(GenericOutputStream& stream) const
    {
        const char null[] = "NULL";
        if (!this->_s) {
            stream.put(null, sizeof(null) - 1);
        }
        else {
            stream.putChar('"');
            const TChar* pC = this->_s;
            while (pC < this->_s + this->_len) {
                stream.putCStr("\\x");
                stream.putChar(sss::lower_hex2char((*pC & 0xF0) >> 4, false));
                stream.putChar(sss::lower_hex2char((*pC & 0x0F), false));
                ++pC;
            }
        }
        stream.putChar('"');
    }

private:
    const char* _s;
    std::size_t _len;
};
}  // namespace

namespace sss {
template <typename Dummy>
hex_string_t<char, std::char_traits<char>> hex_string(const Dummy& v)
{
    return hex_string_t<char, std::char_traits<char>>{v};
}
template <typename TChar, typename TCharTraits = std::char_traits<TChar> >
hex_string_t<TChar, TCharTraits> hex_string(const TChar* s)
{
    return hex_string_t<TChar, TCharTraits>{s, s ? TCharTraits::length(s) : 0u};
}
template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
hex_string_t<TChar, TCharTraits> hex_string(const TChar* s, size_t len)
{
    return hex_string_t<TChar, TCharTraits>{s, len};
}

template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
hex_string_t<TChar, TCharTraits> hex_string(sss::basic_string_view<TChar, TCharTraits> s)
{
    return hex_string_t<TChar, TCharTraits>{s.data(), size_t(s.size())};
}

template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
hex_string_t<TChar, TCharTraits> hex_string(
    const std::basic_string<TChar, TCharTraits>& s)
{
    return hex_string_t<TChar, TCharTraits>{s.c_str(), s.length()};
}

template <typename TChar, typename TCharTraits = std::char_traits<TChar>>
std::basic_ostream<TChar, TCharTraits>& operator<<(
    std::basic_ostream<TChar, TCharTraits>& o,
    const hex_string_t<TChar, TCharTraits>& rs)
{
    rs.print(o);
    return o;
}
}  // namespace sss

#endif /* __HEX_PRINT_HPP_1476611331__ */
