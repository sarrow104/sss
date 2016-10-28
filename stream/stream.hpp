#ifndef __STREAM_HPP_1475317140__
#define __STREAM_HPP_1475317140__

#include <cstdio>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <fstream>

// NOTE 参考
// ~/extra/sss/old_sss/include/sss/strings/CharGetter.h|14

template <typename TChar = char,
          typename TCharTraits = ::std::char_traits<TChar>>
struct GenericOutputStream {
};

template <>
struct GenericOutputStream<std::FILE> {
    std::FILE* _file;
    explicit GenericOutputStream(std::FILE* file) : _file(file) {}
    void putChar(char c)
    {
        if (this->_file) {
            std::fputc(c, this->_file);
        }
    }
    void putCStr(const char* s)
    {
        if (this->_file && s) {
            std::fputs(s, this->_file);
        }
    }
    void put(const char* s, size_t len)
    {
        if (this->_file && s) {
            std::fwrite(s, len, 1, this->_file);
        }
    }
    void flush()
    {
        if (this->_file) {
            std::fflush(this->_file);
        }
    }
    std::string to_string() { return ""; }
    const char* to_cstr() { return nullptr; }
};

//  template<typename _CharT, typename _Traits>
//    class basic_ostream : virtual public basic_ios<_CharT, _Traits>
template <typename TChar, typename TCharTraits>
struct GenericOutputStream<std::basic_ostream<TChar, TCharTraits>> {
    std::basic_ostream<TChar, TCharTraits>& _o;
    GenericOutputStream(std::basic_ostream<TChar, TCharTraits>& o) : _o(o) {}
    void putChar(char c) { this->_o << c; }
    void putCStr(const char* s) { this->_o << s; }
    void put(const char* s, size_t len) { this->_o.write(s, len); }
    void flush() { this->_o.flush(); }
    std::string to_string() { return ""; }
    const TChar* to_cstr() { return nullptr; }
};

template <typename TChar, typename TCharTraits>
struct GenericOutputStream<std::basic_ofstream<TChar, TCharTraits>> {
    std::basic_ofstream<TChar, TCharTraits>& _o;
    GenericOutputStream(std::basic_ofstream<TChar, TCharTraits>& o) : _o(o) {}
    void putChar(char c) { this->_o << c; }
    void putCStr(const char* s) { this->_o << s; }
    void put(const char* s, size_t len) { this->_o.write(s, len); }
    void flush() { this->_o.flush(); }
    std::string to_string() { return ""; }
    const TChar* to_cstr() { return nullptr; }
};
template <typename TChar, typename TCharTraits>
struct GenericOutputStream<std::basic_ostringstream<TChar, TCharTraits>> {
    std::basic_ostream<TChar, TCharTraits>& _o;
    GenericOutputStream(std::basic_ostream<TChar, TCharTraits>& o) : _o(o) {}
    void putChar(char c) { this->_o << c; }
    void putCStr(const char* s) { this->_o << s; }
    void put(const char* s, size_t len) { this->_o.write(s, len); }
    void flush() { return; }
    std::string to_string() { return this->_o.str(); }
    const TChar* to_cstr() { return this->_o.str().c_str(); }
};

template <typename TChar, typename TCharTraits>
struct GenericOutputStream<std::basic_string<TChar, TCharTraits>> {
    std::basic_string<TChar, TCharTraits>& _o;
    GenericOutputStream(std::basic_string<TChar, TCharTraits>& o) : _o(o) {}
    void putChar(char c) { this->_o.append(c); }
    void putCStr(const char* s) { this->_o.append(s); }
    void put(const char* s, size_t len) { this->_o.append(s, len); }
    void flush() { return; }
    std::string to_string() { return this->_o; }
    const TChar* to_cstr() { return this->_o.c_str(); }
};

#endif /* __STREAM_HPP_1475317140__ */
