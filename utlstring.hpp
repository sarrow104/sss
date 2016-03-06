#ifndef __UTLSTRING_HPP_1317397545__
#define __UTLSTRING_HPP_1317397545__

#include <sss/log.hpp>
#include <sss/util/PostionThrow.hpp>
#include <sss/util/StringSlice.hpp>

#include <sss/algorithm.hpp>

#ifndef __WIN32__
#       include <sss/iConvpp.hpp>
#endif

//#define	_UNICODE
#include <iosfwd>

#include <typeinfo>
#include <cwchar>
#include <sstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <locale>
#include <iomanip>
#include <iostream>
#include <cassert>
#include <cstdio>
#include <stdexcept>

#ifdef __WIN32
#       include <tchar.h>
#       include <windows.h>
#else
#define stricmp strcasecmp      // <strings.h> man 3 strcasecmp
#endif


// #C++ 预处理指令，有哪些？
namespace sss{
    // 忽略大小写比较-字符串-小于
    struct stricmp_t
    {
        bool operator()(const std::string& s1, const std::string& s2) const
        {
            int min = std::min(s1.length(), s2.length());
            for (int i = 0; i < min; ++i)
            {
                char c1 = std::toupper(s1[i]);
                char c2 = std::toupper(s2[i]);
                if (c1 < c2)
                    return true;
                else if (c1 > c2)
                    return false;
                else
                    continue;
            }
            return s1.length() < s2.length();
        }
    };

    // 忽略大小写比较-字符-小于
    struct char_less_casei
    {
        bool operator()(const char ch1, const char ch2)
        {
            return std::toupper(ch1) < std::toupper(ch2);
        }
    };

    // 忽略大小写比较-字符-等于
    struct char_equal_casei
    {
        bool operator()(const char ch1, const char ch2)
        {
            return std::toupper(ch1) == std::toupper(ch2);
        }
    };

    template<typename T> const T* safe_str(const T * ) { return 0; };
    template<> inline const char * safe_str<char>(const char * s) { return (s ? s : ""); }
    template<> inline const wchar_t * safe_str<wchar_t>(const wchar_t * s) { return (s ? s : L""); }

    template<typename Iterator1, typename Iterator2>
    inline bool is_equal(Iterator1 first1, Iterator1 last1, Iterator2 first2, Iterator2 last2, int ignore_case = false)
    {
        if (ignore_case) {
            return sss::equal(first1, last1, first2, last2, char_equal_casei());
        }
        else {
            return sss::equal(first1, last1, first2, last2);
        }
    }

    template<typename Iterator1, typename Container>
    inline bool is_equal(Iterator1 first1, Iterator1 last1, const Container& pattern, int ignore_case = false)
    {
        return is_equal(first1, last1, pattern.begin(), pattern.end(), ignore_case);
    }

    template<typename Iterator1, typename Container>
    inline bool is_equal(const Container& s, Iterator1 first1, Iterator1 last1, int ignore_case = false)
    {
        return is_equal(s.begin(), s.end(), first1, last1, ignore_case);
    }

    template<typename Iterator1, typename T, int size>
    inline bool is_equal(const T(&s)[size], Iterator1 first1, Iterator1 last1, int ignore_case = false)
    {
        return is_equal(s, s + size - 1, first1, last1, ignore_case);
    }

    template<typename Iterator1, typename T, int size>
    inline bool is_equal(Iterator1 first1, Iterator1 last1, const T(&pattern)[size], int ignore_case = false)
    {
        return is_equal(first1, last1, pattern, pattern + size - 1, ignore_case);
    }

    template<typename T1, int size1, typename T2, int size2>
    inline bool is_equal(const T1(&s)[size1], const T2(&pattern)[size2], int ignore_case = false)
    {
        return is_equal(s, s + size1 - 1, pattern, pattern + size2 - 1, ignore_case);
    }

    inline bool is_equal(const std::string& s, const std::string& pattern, int ignore_case = false)
    {
        return is_equal(s.begin(), s.end(), pattern.begin(), pattern.end(), ignore_case);
    }

    template<typename Iterator1, typename Iterator2>
    inline bool is_has(Iterator1 first1, Iterator1 last1, Iterator2 first2, Iterator2 last2, int ignore_case = false)
    {
        if (ignore_case) {
            return std::search(first1, last1, first2, last2, char_equal_casei()) != last1;
        }
        else {
            return std::search(first1, last1, first2, last2) != last1;
        }
    }

    template<typename Iterator1, typename Container>
    inline bool is_has(Iterator1 first1, Iterator1 last1, const Container& pattern, int ignore_case = false)
    {
        return is_has(first1, last1, pattern.begin(), pattern.end(), ignore_case);
    }

    template<typename Iterator1, typename Container>
    inline bool is_has(const Container& s, Iterator1 first1, Iterator1 last1, int ignore_case = false)
    {
        return is_has(s.begin(), s.end(), first1, last1, ignore_case);
    }

    template<typename Iterator1, typename T, int size>
    inline bool is_has(const T(&s)[size], Iterator1 first1, Iterator1 last1, int ignore_case = false)
    {
        return is_has(s, s + size - 1, first1, last1, ignore_case);
    }

    template<typename Iterator1, typename T, int size>
    inline bool is_has(Iterator1 first1, Iterator1 last1, const T(&pattern)[size], int ignore_case = false)
    {
        return is_has(first1, last1, pattern, pattern + size - 1, ignore_case);
    }

    // NOTE is_has("str1", "str2") 这种调用，是没有多少意义的——既然都是字符常
    // 量，那么还需要运行时去判断包含关系吗？
    // 不过，为了完备性，以及可能的"宏"传值，还是提供一个吧！
    // NOTE 字符串常量数组，需要排除掉末尾的"\0"
    template<typename T1, int size1, typename T2, int size2>
    inline bool is_has(const T1(&s)[size1], const T2(&pattern)[size2], int ignore_case = false)
    {
        return is_has(s, s + size1 - 1, pattern, pattern + size2 - 1, ignore_case);
    }

    inline bool is_has(const std::string& s, const std::string& pattern, int ignore_case = false)
    {
#if 1
        return is_has(s.begin(), s.end(), pattern.begin(), pattern.end(), ignore_case);
#else
        if (ignore_case) {
            return std::search(s.begin(), s.end(), pattern.begin(), pattern.end(), char_equal_casei()) != s.end();
        }
        else {
            return std::search(s.begin(), s.end(), pattern.begin(), pattern.end()) != s.end();
        }
#endif
    }

    bool is_all_blank(const std::string& line)
    {
        return sss::is_all(line.begin(), line.end(), static_cast<int(*)(int)>(std::isspace));
    }

    template<typename IteratorT>
    bool is_all_blank(IteratorT first, IteratorT last)
    {
        return sss::is_all(first, last, static_cast<int(*)(int)>(std::isspace));
    }

    inline bool is_contain_with(const std::string& s, const std::string& pattern, bool ignore_case = false)
    {
        if (ignore_case) {
            return
                s.length() >= pattern.length() &&
                std::search(s.begin(), s.end(), pattern.begin(), pattern.end(), char_equal_casei()) != s.end();
        }
        else {
            return
                s.length() >= pattern.length() &&
                std::search(s.begin(), s.end(), pattern.begin(), pattern.end()) != s.end();
        }
    }

    // TODO 应当将如下几个函数，做成模板函数，以适应各种迭代器，以及指针；
    //
    // FIXME C++的类型自动转换：
    // const char * -> const std::string&
    // const char * -> bool
    // 可能会让你发疯！
    inline bool is_begin_with(const std::string& s, const std::string& pattern, int ignore_case = false)
    {
        if (ignore_case) {
            return s.length() >= pattern.length()
                && std::equal(pattern.begin(), pattern.end(), s.begin(), char_equal_casei());
        }
        else {
            return s.length() >= pattern.length()
                && std::equal(pattern.begin(), pattern.end(), s.begin());
        }
    }

    template<typename Iterator1, typename Iterator2> bool is_begin_with(Iterator1 s1b,
                                                                        Iterator1 s1e,
                                                                        Iterator2 s2b,
                                                                        Iterator2 s2e,
                                                                        bool ignore_case = false)
    {
        if (ignore_case) {
            return std::distance(s1b, s1e) >= std::distance(s2b, s2e)
                && std::equal(s2b, s2e, s1b, char_equal_casei());
        }
        else {
            return std::distance(s1b, s1e) >= std::distance(s2b, s2e)
                && std::equal(s2b, s2e, s1b);
        }
    }

    // inline bool is_begin_with(std::string::const_iterator s1b,
    //                           std::string::const_iterator s1e,
    //                           std::string::const_iterator s2b,
    //                           std::string::const_iterator s2e,
    //                           bool ignore_case = false)
    // {
    //     if (ignore_case) {
    //         return std::distance(s1b, s1e) >= std::distance(s2b, s2e)
    //             && std::equal(s2b, s2e, s1b, char_equal_casei());
    //     }
    //     else {
    //         return std::distance(s1b, s1e) >= std::distance(s2b, s2e)
    //             && std::equal(s2b, s2e, s1b);
    //     }
    // }

    inline bool is_begin_with(std::string::const_iterator s1b,
                              std::string::const_iterator s1e,
                              const char * s2b,
                              bool ignore_case = false)
    {
        ssize_t len1 = std::distance(s1b, s1e);
        ssize_t len2 = std::strlen(s2b);
        if (ignore_case) {
            return len1 >= len2 && std::equal(s2b, s2b + len2, s1b, char_equal_casei());
        }
        else {
            return len1 >= len2 && std::equal(s2b, s2b + len2, s1b);
        }
    }

    inline bool is_begin_with(std::string::const_iterator s1b,
                              std::string::const_iterator s1e,
                              const std::string& s2,
                              bool ignore_case = false)
    {
        ssize_t len1 = std::distance(s1b, s1e);
        ssize_t len2 = s2.length();
        if (ignore_case) {
            return len1 >= len2 && std::equal(s2.begin(), s2.end(), s1b, char_equal_casei());
        }
        else {
            return len1 >= len2 && std::equal(s2.begin(), s2.end(), s1b);
        }
    }

    inline bool is_end_with(const std::string& s, const std::string& pattern,
                            int ignore_case = false)
    {
        if (ignore_case) {
            return s.length() >= pattern.length()
                && std::equal(pattern.rbegin(), pattern.rend(), s.rbegin(), char_equal_casei());
        }
        else {
            return s.length() >= pattern.length()
                && std::equal(pattern.rbegin(), pattern.rend(), s.rbegin());
        }
    }

    // inline bool is_end_with(std::string::const_iterator s1b,
    //                         std::string::const_iterator s1e,
    //                         std::string::const_iterator s2b,
    //                         std::string::const_iterator s2e,
    //                         bool ignore_case = false)
    // {
    //     ssize_t len1 = std::distance(s1b, s1e);
    //     ssize_t len2 = std::distance(s2b, s2e);
    //     if (len1 >= len2) {
    //         std::advance(s1b, len1-len2);
    //     }
    //     if (ignore_case) {
    //         return len1 >= len2 && std::equal(s2b, s2e, s1b, char_equal_casei());
    //     }
    //     else {
    //         return len1 >= len2 && std::equal(s2b, s2e, s1b);
    //     }
    // }

    template<typename Iterator1, typename Iterator2>  bool is_end_with(Iterator1 s1b,
                                                                       Iterator1 s1e,
                                                                       Iterator2 s2b,
                                                                       Iterator2 s2e,
                                                                       bool ignore_case = false)
    {
        ssize_t len1 = std::distance(s1b, s1e);
        ssize_t len2 = std::distance(s2b, s2e);
        if (len1 >= len2) {
            std::advance(s1b, len1-len2);
        }
        if (ignore_case) {
            return len1 >= len2 && std::equal(s2b, s2e, s1b, char_equal_casei());
        }
        else {
            return len1 >= len2 && std::equal(s2b, s2e, s1b);
        }
    }

    inline bool is_end_with(std::string::const_iterator s1b,
                            std::string::const_iterator s1e,
                            const char * s2b,
                            bool ignore_case = false)
    {
        ssize_t len1 = std::distance(s1b, s1e);
        ssize_t len2 = std::strlen(s2b);
        if (len1 >= len2) {
            std::advance(s1b, len1 - len2);
        }
        if (ignore_case) {
            return len1 >= len2 && std::equal(s2b, s2b + len2, s1b, char_equal_casei());
        }
        else {
            return len1 >= len2 && std::equal(s2b, s2b + len2, s1b);
        }
    }

    namespace utlstr {
        // 2014-07-02
        std::string sample_string(std::string::const_iterator s_ini,
                                  std::string::const_iterator s_fin,
                                  int len = 20);

        inline
        std::string sample_string(const std::string& str, int len = 20)
        {
            return sample_string(str.begin(), str.end(), len);
        }
    }

    // 原地去掉单引号
    inline std::string& unsquote(std::string& s)
    {
        if (s.length() && *s.begin() == '\'' && *s.rbegin() == '\'')
        {
            s.assign(s, 1, s.length() - 2);
        }
        return s;
    }

    // 原地去掉双引号
    inline std::string& undquote(std::string& s)
    {
        if (s.length() && *s.begin() == '"' && *s.rbegin() == '"')
        {
            s.assign(s, 1, s.length() - 2);
        }
        return s;
    }

    // before and after 将原始字符串，按照锚点字符串的位置，分割为三部分；
    // 锚点字符串，忽略；
    // shrink_to ...
    inline void before(std::string& s, const std::string& arch)
    {
        std::string::size_type pos = s.find(arch);
        if (pos != std::string::npos) {
            s.resize(pos);
        }
    }

    inline void after(std::string& s, const std::string& arch)
    {
        std::string::size_type pos = s.find(arch);
        // sample:
        //    "abc" "c"
        //    pos = 2
        //    new_len = 3 - 2 - 1 = 0
        if (pos != std::string::npos) {
            size_t new_len = s.length() - pos - arch.length();
            if (new_len) {
                char * to = const_cast<char*>(s.c_str());
                const char * from = s.c_str() + pos + arch.length();
                std::memmove(to, from, new_len);
            }
            s.resize(new_len);
        }
        else {
            s.resize(0);
        }
    }

    // 去掉单引号
    inline std::string unsquote_copy(const std::string& s)
    {
        std::string ret;
        ret.assign(s.begin(), s.end());
        unsquote(ret);
        return ret;
    }

    // 去掉双引号
    inline std::string undquote_copy(const std::string& s)
    {
        std::string ret;
        ret.assign(s.begin(), s.end());
        undquote(ret);
        return ret;
    }
    // 将连续的两个字符，理解为hex序列，然后重新输出为字符串
    // 新字符串的长度为原来的1/2。
    inline int hex2int(char c)
    {
        assert(isxdigit(c));
        return int(isdigit(c) ? (c) - '0' : toupper(c)- 'A' + 10);
    }

    std::string& hex2string(std::string& s);

    inline std::string hex2string_copy(const std::string& s)
    {
        std::string ret;
        ret.assign(s);
        sss::hex2string(ret);
        return ret;
    }

    // 2012-01-01
    // 按16进制输出内存
    template <typename T>
        std::string to_hex(T begin, T end)
        {
            std::string ret;
            char buffer[3] = {'\0', '\0', '\0'};
            for (T it = begin; it != end; ++it)
            {
                char buffer2[sizeof(*it)];
                std::memcpy(buffer2, reinterpret_cast<void*>(&(*it)), sizeof(*it));
                for (size_t i = 0; i < sizeof(*it); ++i)
                {
                    std::sprintf(buffer, "%.02X", (unsigned char)buffer2[i]);
                    ret += buffer[0];
                    ret += buffer[1];
                }
            }
            return ret;
        }
// 2012-01-01
// http://cboard.cprogramming.com/cplusplus-programming/118266-strings-find-replace.html
// NOTE tar 指连续的字符串，搜索然后匹配、替换。
//
// 2015-11-30
//    * 为replace_xxx 系列函数增加是否忽略大小写比较参数
//    * 为 replace 和 replace_copy 增加，开始匹配的位置偏移参数
std::string& replace_all(std::string& src, const std::string& tar, const std::string& with,
                         bool ignore_case = false);

std::string  replace_all_copy(const std::string& src, const std::string& tar, const std::string& with,
                              bool ignore_case = false);

std::string& replace(std::string& src, const std::string& tar, const std::string& with,
                     int offset = 0,
                     bool ignore_case = false);

std::string  replace_copy(const std::string& src, const std::string& tar, const std::string& with,
                          int offset = 0,
                          bool ignore_case = false);

typedef std::string (*replace_functor_t)(const std::string& s);

std::string& callback_replace_all(std::string& src, const std::string& tar, replace_functor_t functor,
                                  bool ignore_case = false);

std::string callback_replace_all_copy(const std::string& src, const std::string& tar, replace_functor_t functor,
                                      bool ignore_case = false);

std::string& wrapper_replace_all(std::string& src, const std::string& tar,
                                 const std::string& before, const std::string& after,
                                 bool ignore_case = false);

std::string wrapper_replace_all_copy(const std::string& src, const std::string& tar,
                                     const std::string& before, const std::string& after,
                                     bool ignore_case = false);

// 2011-12-27
inline void to_lower(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

inline std::string to_lower_copy(const std::string& s)
{
    std::string ret(s.length(), '\0');
    std::transform(s.begin(), s.end(), ret.begin(), ::tolower);
    return ret;
}

inline void to_lower(std::wstring& ws)
{
    std::transform(ws.begin(), ws.end(), ws.begin(), ::towlower);
}

inline std::wstring to_lower_copy(const std::wstring& ws)
{
    std::wstring ret(ws.length(), L'\0');
    std::transform(ws.begin(), ws.end(), ret.begin(), ::towlower);
    return ret;
}

inline void to_lower_facet( std::string &src )
{
    //!http://hi.baidu.com/dbus/blog/item/79acead6be27dc2907088be3.html/cmtid/d104b05490b33056d0090650
    std::use_facet< std::ctype<char> >( std::locale() ).tolower(
            &(*src.begin()), &(*src.end()) );
}

// 2011-12-27
inline void to_upper(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
}

inline std::string to_upper_copy(const std::string& s)
{
    std::string ret(s.length(), '\0');
    std::transform(s.begin(), s.end(), ret.begin(), ::toupper);
    return ret;
}

inline void to_upper(std::wstring& ws)
{
    std::transform(ws.begin(), ws.end(), ws.begin(), ::towupper);
}

inline std::wstring to_upper_copy(const std::wstring& ws)
{
    std::wstring ret(ws.length(), L'\0');
    std::transform(ws.begin(), ws.end(), ret.begin(), ::towupper);
    return ret;
}

inline void to_upper_facet( std::string &src )
{
    std::use_facet< std::ctype<char> >( std::locale() ).toupper(
            &(*src.begin()), &(*src.end()) );
}

std::string& trim(std::string & source, char const* delims = " \t\r\n");

std::string  trim_copy(std::string const& source, char const* delims = " \t\r\n");

std::string& rtrim(std::string & source, char const* delims = " \t\r\n");

std::string  rtrim_copy(std::string const& source, char const* delims = " \t\r\n");

std::string& ltrim(std::string & source, char const* delims = " \t\r\n");

std::string  ltrim_copy(std::string const& source, char const* delims = " \t\r\n");

template<typename T>
inline
std::string cast_string(const T& val)
{
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

template<>
inline
std::string cast_string<std::string>(const std::string& s)
{
    return s;
}

template<typename T>
inline
T string_cast(const std::string& sVal)
{
    std::istringstream iss(sVal);
    T val;
    iss >> val;
    if (iss.fail()) {
        SSS_POSTION_THROW(std::runtime_error,
                          "unable to convert `" << sVal
                          << "` to type `" << typeid(T).name() << "`");
    }
    if (sVal.length() > size_t(iss.tellg())) {
        SSS_POSTION_THROW(std::runtime_error,
                          "convert `" << sVal << "` to type `" << typeid(T).name()
                          << "` with tailing bytes `" << sVal.substr(iss.tellg()) << "`");
    }
    return val;
}

template<typename T>
inline
T string_cast_nothrow(const std::string& sVal)
{
    std::istringstream iss(sVal);
    T val;
    iss >> val;
    return val;
}

template<>
inline
std::string string_cast<std::string>(const std::string& sVal)
{
    return sVal;
}

template<>
inline
std::string string_cast_nothrow<std::string>(const std::string& sVal)
{
    return sVal;
}

// NOTE add 2013-02-19
// FIXME CP936 转换为 USC2，而且，还区分LE 和 BE 两个版本，是没有必要的。因为，
// CP936本质是用char[]来保存，不存在字节序问题。
//
// 而 UCS2 在内存中的时候，也是不存在字节序问题；只有 UCS2 被写入外部文件的时候
// ，才需要考虑字节序！
//
// 同样的问题，对于 UCS4 也是成立的。
//
// CP_ACP 是什么意思呢？
//      means：Code Page = Ansi Code Page
//
// CP_OEMCP
//      means: OEM Code Page
//
// 另外，C语言标准的做法是：
//      setlocale(LC_CTYPE,"");//把当前locale字符环境从C/C++缺省的"C"设置，改为
//      操作系统的设置（即代码页936）
//
//! http://bbs.csdn.net/topics/340204336?page=1

// 函数名字le的说法，不实！
// le或者是be，只针对写入外存的时候，才有意义；在内存中，不用关心le还是be；
// 因为ucs2(或者utf16)，本质上就是unicode编码！
// 就是代表一个数字；至于le还是be，只是因为，写入外存的时候，必然是一个字节、一
// 个字节地写，就需要考虑字节顺序的问题了。
inline int cp936_to_ucs2(const std::string& from, std::wstring& to)
{
#ifdef __WIN32__
    int len = ::MultiByteToWideChar(CP_ACP, 0, &from[0], from.length(), NULL, 0);
    if (len == 0)
        return 0;

    std::vector<wchar_t> ret(len + 1);
    ::MultiByteToWideChar(CP_ACP, 0, &from[0], from.length(), &ret[0], len);

    to.assign(ret.begin(), ret.end() - 1);
#else
    sss::iConv ic("WCHAR_T", "cp936");
    ic.mbstoucs4(from, to);
#endif
    return to.length();
}

inline int ucs2_to_cp936(const std::wstring& from, std::string& to)
{
#ifdef __WIN32
    // NOTE 转换得到的到底是 cp936 还是 utf8 呢？
    int len = ::WideCharToMultiByte(CP_ACP, 0, &from[0], from.length(), NULL, 0, NULL, NULL);
    if (len == 0)
        return 0;

    std::vector<char> ret(len + 1);
    ::WideCharToMultiByte(CP_ACP, 0, &from[0], from.length(), &ret[0], len, NULL, NULL);

    to.assign(ret.begin(), ret.end() - 1);
#else
    sss::iConv ic("cp936", "WCHAR_T");
    ic.ucs4tombs(from, to);
#endif
    return to.length();
}

inline int utf8_to_ucs2(const std::string& from, std::wstring& to)
{
#ifdef __WIN32
    int len = ::MultiByteToWideChar(CP_UTF8, 0, &from[0], from.length(), NULL, 0);
    if (len == 0)
        return 0;

    std::vector<wchar_t> ret(len + 1);
    *ret.rbegin() = 10;
    ::MultiByteToWideChar(CP_UTF8, 0, &from[0], from.length(), &ret[0], len);
    printf("%d\n", *ret.rbegin());
    fflush(stdout);

    to.assign(ret.begin(), ret.end() - 1);
#else
    sss::iConv ic("WCHAR_T", "utf8");
    ic.mbstoucs4(from, to);
#endif
    return to.length();
}

inline int ucs2_to_utf8(const std::wstring& from, std::string& to)
{
#ifdef __WIN32
    int len = ::WideCharToMultiByte(CP_UTF8, 0, &from[0], from.length(), NULL, 0, NULL, NULL);
    if (len == 0)
        return 0;

    std::vector<char> ret(len + 1);
    ::WideCharToMultiByte(CP_UTF8, 0, &from[0], from.length(), &ret[0], len, NULL, NULL);
    to.assign(ret.begin(), ret.end() - 1);
#else
    sss::iConv ic("utf8", "WCHAR_T");
    ic.ucs4tombs(from, to);
#endif
    return to.length();
}

// 把一个wstring转化为string
inline std::string& to_string(std::string& dest, std::wstring const & src)
{
    //使用这两个函数(wcstombs;mbstowcs)之前必须将你的LC_CTYPE设置成你想要的本地
    //编码方案（使用setlocale函数）
    setlocale(LC_CTYPE, "");

    size_t const mbs_len = wcstombs(NULL, src.c_str(), 0);
    std::vector<char> tmp(mbs_len + 1);
    wcstombs(&tmp[0], src.c_str(), tmp.size());

    dest.assign(tmp.begin(), tmp.end() - 1);

    return dest;
}

// 把一个string转化为wstring
inline std::wstring& to_wstring(std::wstring& dest, std::string const & src)
{
    setlocale(LC_CTYPE, "");

    size_t const wcs_len = mbstowcs(NULL, src.c_str(), 0);
    std::vector<wchar_t> tmp(wcs_len + 1);
    mbstowcs(&tmp[0], src.c_str(), src.size());

    dest.assign(tmp.begin(), tmp.end() - 1);

    return dest;
}

/************************************************************************/
/*
编码转换

utf8 - gb2312
在unix平台中可以使用iconv来做转换
在windows平台可以用MultiByteToWideChar/WideCharToMultiByte 函数.

char - wchar_t
使用CRT库的mbstowcs()函数和wcstombs()函数，平台无关，需设定locale
The behavior of mbstowcs() depends on the LC_CTYPE category of the current locale.

参考：

    ~/extra/sss/doc/Ubuntu下wcstombs失败的问题原因及解决方法——使用localedef命令.txt

*/
/************************************************************************/
#include <locale.h>

#ifndef __WIN32__
#include <iconv.h>
#endif

#include <string>

//GB2312 转为 UTF-8
inline void GB2312ToUTF_8(std::string& pOut,char *pText, int pLen);
//UTF-8 转为 GB2312
void UTF_8ToGB2312(std::string &pOut, char *pText, int pLen);

#ifndef __WIN32__
//代码转换:从一种编码转为另一种编码
int code_convert(const char *from_charset, const char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen);

//UNICODE码转为GB2312码
int u2g(char *inbuf, int inlen, char *outbuf, int outlen);

//GB2312码转为UNICODE码
int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
#else

// 把UTF-8转换成Unicode
void UTF_8ToUnicode(wchar_t* pOut, char *pText);
// Unicode 转换成UTF-8
void UnicodeToUTF_8(char* pOut, wchar_t* pText);
// 把Unicode 转换成 GB2312
void UnicodeToGB2312(char* pOut, unsigned short uData);
// GB2312 转换成　Unicode
void Gb2312ToUnicode(wchar_t* pOut, char *gbBuffer);

//GB2312 转为 UTF-8
void GB2312ToUTF_8ByWin(std::string& pOut, char *pText, int pLen);
//UTF-8 转为 GB2312
void UTF_8ToGB2312ByWin(std::string &pOut, char *pText, int pLen);

char* UTF8ToString(const char* src, char* dest, int dest_size);
#endif

/* {{{1
inline std::string IntToString( int nVal )
{
    // integer MAX : 4294967295L
    char Buf[16] = {'\0'};
    _itoa_s(nVal, Buf, sizeof(Buf), 10L);
    return std::string(Buf);
}

inline std::string Int64ToString( __int64 liVal )
{
    // integer_64 MAX : 18446744073709551615L
    char Buf[32] = {'\0'};
    _i64toa_s(liVal, Buf, sizeof(Buf), 10L);
    return std::string(Buf);
}

inline std::string FloatToString( float fVal )
{
    char Buf[32] = {'\0'};
    sprintf_s( Buf, sizeof(Buf), "%f", fVal);
    return std::string(Buf);
}

inline int StringToInt( const char * pVal )
{
    assert(pVal);
    return ( ::atoi(pVal) );
}

inline __int64 StringToInt64( const char * pVal )
{
    assert(pVal);
    return ( ::_atoi64(pVal) );
}

inline float StringToFloat( const char* pVal )
{
    assert(pVal);
    return float( ::atof(pVal) );
}

inline int WideStrToInt( const wchar_t * pVal )
{
    assert(pVal);
    return (::_wtoi(pVal));
}

inline __int64 WideStrToInt64( const wchar_t * pVal )
{
    assert(pVal);
    return (::_wtoi64(pVal));
}

inline float WideStrToFloat( const wchar_t * pVal )
{
    assert(pVal);
    return ((float)::_wtof(pVal));
}

inline wstring IntToWideStr( int nVal )
{
    wchar_t buf[32] = {L"\0"};

    _itow_s(
        nVal,
        buf,
        32,
        10 );

    return wstring(buf);
}

inline std::wstring Int64ToWideStr( __int64 lnVal )
{
    wchar_t buf[32] = {L"\0"};

    _i64tow_s(
        lnVal,
        buf,
        32,
        10 );

    return std::wstring(buf);
}

inline std::wstring FloatToWideStr( float fVal )
{
    wchar_t buf[32] = {L"\0"};

    wsprintfW(buf, L"%f", fVal);

    return std::wstring(buf);
}
}}}1*/

inline std::string ws2s(const std::wstring& ws)
{
    std::string curLocale = setlocale(LC_ALL, NULL); // curLocale = "C";

    //以gbk页码来翻译为中文的双字节
    setlocale(LC_ALL, "chs");

    size_t _Dsize = 2 * ws.size() + 1;
    // FIXME 裸指针 -- 如果后面的 result 构造失败，那么_Dest的delete动作就不会
    // 执行，从而导致内存泄露
    char *_Dest = new char[_Dsize];
    std::memset(_Dest, 0, _Dsize);
    std::wcstombs(_Dest, ws.c_str(), _Dsize);
    std::string result = _Dest;
    delete []_Dest;

    setlocale(LC_ALL, curLocale.c_str());

    return result;
}

inline std::wstring s2ws(const std::string& s)
{
    //以gbk页码来翻译为中文的双字节
    std::string curLocale = setlocale(LC_ALL, "chs");

    size_t _Dsize = s.size() + 1;
    // FIXME 裸指针
    wchar_t *_Dest = new wchar_t[_Dsize];
    std::wmemset(_Dest, 0, _Dsize);
    std::mbstowcs(_Dest, s.c_str(), _Dsize);
    std::wstring result = _Dest;
    delete []_Dest;

    setlocale(LC_ALL, curLocale.c_str());

    return result;
}

#ifndef __WIN32__
//代码转换:从一种编码转为另一种编码
int code_convert(const char *from_charset, const char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen);

// utf8 码转为GB2312码
inline int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
    return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}
//GB2312码转为 utf8 码
inline int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}
#else

// 把Unicode 转换成 GB2312
inline void UnicodeToGB2312(char* pOut,unsigned short uData)
{
    ::WideCharToMultiByte(CP_ACP, 0,(wchar_t*)&uData,1,pOut,sizeof(wchar_t),NULL,NULL);
    return;
}
// GB2312 转换成　Unicode
inline void Gb2312ToUnicode(wchar_t* pOut,char *gbBuffer)
{
    ::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,gbBuffer,2,pOut,1);
    return;
}
//GB2312 转为 UTF-8
void GB2312ToUTF_8ByWin(std::string& pOut,char *pText, int pLen);

inline char* UTF8ToString(const char* src, char* dest, int dest_size)
{
    wchar_t wbuffer[2048];
#ifdef __WIN32__
    ::MultiByteToWideChar(CP_ACP, 0, src, -1, wbuffer, 2048);
    ::WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, dest, dest_size, NULL, NULL);
#else
    mbstowcs(wbuffer, src, 2048);
    wcstombs(dest, wbuffer, dest_size);
#endif
    return dest;
}

//UTF-8 转为 GB2312
void UTF_8ToGB2312ByWin(std::string &pOut, char *pText, int pLen);
#endif

//GB2312 转为 UTF-8
inline void GB2312ToUTF_8(std::string& pOut,char *pText, int pLen)
{
#ifndef __WIN32__
    int outLen = pLen + (pLen >> 2) + 2;
    g2u(pText, pLen, &pOut[0], outLen);
#else
    GB2312ToUTF_8ByWin(pOut, pText, pLen);
#endif
}

//UTF-8 转为 GB2312
inline void UTF_8ToGB2312(std::string &pOut, char *pText, int pLen)
{
#ifndef __WIN32__
    u2g(pText, pLen, &pOut[0], pLen+1);
#else
    UTF_8ToGB2312ByWin(pOut, pText, pLen);
#endif
}

//一个完整的UTF8到WCS的转换函数（SWI库里面的---实际上它是在APACHE的库里面的实现，不过是一个旧一些时候的版本了）
//     gUTFBytes
//             A list of counts of trailing bytes for each initial byte in the input.
//
//     gUTFOffsets
//             A list of values to offset each result char type, according to how
//             many source bytes when into making it.
//
//     gFirstByteMark
//             A list of values to mask onto the first byte of an encoded sequence,
//             indexed by the number of bytes used to create the sequence.

bool utf8towcs(const unsigned char *src, wchar_t *dst, int maxdstlen );

} // end of namespace sss
#endif /* __UTLSTRING_HPP_1317397545__ */
