#include "utlstring.hpp"

#include <sss/util/utf8.hpp>
#include <sss/config.hpp>

//#include <locale> // stl: locale_facets

#if defined(SSS_PLT_MSVC)
#       include <tchar.h>
#       include <windows.h>
#else
#       include <sss/iConvpp.hpp>
#define stricmp strcasecmp      // <strings.h> man 3 strcasecmp
#endif


namespace sss{

#if defined(SSS_PLT_MSVC)
// covert Unicode character into GB2312 character
static void UnicodeToGB2312(char* pOut,unsigned short uData)
{
    ::WideCharToMultiByte(CP_ACP, 0, (wchar_t*)&uData, 1, pOut, sizeof(wchar_t), NULL, NULL);
    return;
}

// covert GB2312  character into Unicode character
static void Gb2312ToUnicode(wchar_t* pOut,char *gbBuffer)
{
    ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, gbBuffer, 2, pOut, 1);
    return;
}
#endif

std::string& hex2string(std::string& s)
{
    assert(s.length() % 2 == 0);
    int len = 0;
    for (size_t i = 0;
         i < s.length()/2 && isxdigit(s[2*i]) && isxdigit(s[2*i + 1]);
         ++i, len = i)
    {
        s[i] = hex2int(s[2*i]) * 16 + hex2int(s[2*i + 1]);
        //s[i] = ((unsigned char)(s[2*i]) << 8) + (unsigned char)(s[2*i + 1]);
    }
    s.resize(len);
    return s;
}

namespace utlstr {
std::string sample_string(
    std::string::const_iterator s_ini,
    std::string::const_iterator s_fin,
    int len)
{
    std::string::const_iterator s_end = s_ini;
    if (std::distance(s_ini, s_fin) > len + 3) {
        std::advance(s_end, len);
        return std::string(s_ini, s_end) + "...";
    }
    else {
        return std::string(s_ini, s_fin);
    }
}
}

// 2012-01-01
// http://cboard.cprogramming.com/cplusplus-programming/118266-strings-find-replace.html
std::string replace_all_copy(
    const std::string& src,
    const std::string& tar,
    const std::string& with,
    bool ignore_case)
{
    std::string ret;
    ret.reserve(src.length());

    std::string::const_iterator it = src.begin();
    std::string::const_iterator first = src.begin();

    while (it != src.end()) {
        if (ignore_case) {
            first = std::search(it, src.end(), tar.begin(), tar.end(), sss::char_equal_casei());
        }
        else {
            first = std::search(it, src.end(), tar.begin(), tar.end());
        }
        ret.append(it, first);
        if (first == src.end()) {
            break;
        }
        ret.append(with);
        it = first + tar.length();
    }
    return ret;
}

std::string& callback_replace_all(
    std::string& src,
    const std::string& tar,
    replace_functor_t functor,
    bool ignore_case)
{
    std::string ret = callback_replace_all_copy(src, tar, functor, ignore_case);
    std::swap(ret, src);
    return src;
}

std::string callback_replace_all_copy(
    const std::string& src,
    const std::string& tar,
    replace_functor_t functor,
    bool ignore_case)
{
    std::string ret;
    ret.reserve(src.length());

    std::string::const_iterator it = src.begin();
    std::string::const_iterator first = src.begin();

    while (it != src.end()) {
        if (ignore_case) {
            first = std::search(it, src.end(), tar.begin(), tar.end(), sss::char_equal_casei());
        }
        else {
            first = std::search(it, src.end(), tar.begin(), tar.end());
        }
        ret.append(it, first);
        if (first == src.end()) {
            break;
        }
        ret.append(functor(tar));
        it = first + tar.length();
    }
    return ret;
}

std::string& wrapper_replace_all(
    std::string& src,
    const std::string& tar,
    const std::string& before,
    const std::string& after,
    bool ignore_case)
{
    std::string ret = wrapper_replace_all_copy(src, tar, before, after, ignore_case);
    std::swap(ret, src);
    return src;
}

std::string wrapper_replace_all_copy(
    const std::string& src,
    const std::string& tar,
    const std::string& before,
    const std::string& after,
    bool ignore_case)
{
    std::string ret;
    ret.reserve(src.length());

    std::string::const_iterator it = src.begin();
    std::string::const_iterator first = src.begin();

    while (it != src.end()) {
        if (ignore_case) {
            first = std::search(
                it, src.end(),
                tar.begin(), tar.end(),
                sss::char_equal_casei());
        }
        else {
            first = std::search(it, src.end(), tar.begin(), tar.end());
        }
        ret.append(it, first);
        if (first == src.end()) {
            break;
        }
        ret.append(before);
        ret.append(first, first + tar.length());
        ret.append(after);
        it = first + tar.length();
    }
    return ret;
}

std::string& replace_all(
    std::string& src,
    const std::string& tar,
    const std::string& with,
    bool ignore_case)
{
#if 1
    std::string ret = replace_all_copy(src, tar, with, ignore_case);
    std::swap(src, ret);
#else
    for(std::string::size_type i = 0; (i = src.find(tar, i)) != std::string::npos;)
    {
        src.replace(i, tar.length(), with);
        i += with.length() - tar.length() + 1;
    }
#endif
    return src;
}

std::string& replace(
    std::string& src,
    const std::string& tar,
    const std::string& with,
    int offset,
    bool ignore_case)
{
#if 1
    std::string::const_iterator first = src.end();

    if (ignore_case) {
        first = std::search(src.begin() + offset, src.end(), tar.begin(), tar.end(), sss::char_equal_casei());
    }
    else {
        first = std::search(src.begin() + offset, src.end(), tar.begin(), tar.end());
    }

    if (first != src.end()) {
        src.replace(first - src.begin(), tar.length(), with);
    }
#else
    std::string::size_type i = src.find(tar);
    if (i != std::string::npos)
    {
        src.replace(i, tar.length(), with);
    }
#endif
    return src;
}

std::string replace_copy(
    const std::string& src,
    const std::string& tar,
    const std::string& with,
    int offset,
    bool ignore_case)
{
    std::string ret(src);
    replace(ret, tar, with, offset, ignore_case);
    return ret;
}

std::string  trim_copy(
    std::string const& source,
    char const* delims)
{
    std::string result(source);
    return sss::trim(result, delims);
}

std::string& trim(
    std::string & result,
    char const* delims)
{
    //std::string::size_type index = result.find_last_not_of(delims);
    //if(index != std::string::npos)
    //    result.erase(++index);

    //index = result.find_first_not_of(delims);
    //if(index != std::string::npos)
    //    result.erase(0, index);
    //else
    //    result.erase();
    sss::rtrim(result, delims);
    sss::ltrim(result, delims);
    return result;
}

std::string& rtrim(std::string & result, char const* delims)
{
    std::string::size_type index = result.find_last_not_of(delims);

    if(index != std::string::npos)
    {
        result.erase(++index);
    }
    else
    {
        result.erase();
    }

    return result;
}

std::string  rtrim_copy(std::string const& source, char const* delims)
{
    std::string result(source);
    return sss::rtrim(result, delims);
}

std::string& ltrim(std::string & result, char const* delims)
{
    std::string::size_type index = result.find_first_not_of(delims);
    if(index != std::string::npos)
    {
        result.erase(0, index);
    }
    else
    {
        result.erase();
    }
    return result;
}

std::string  ltrim_copy(std::string const& source, char const* delims)
{
    std::string result(source);
    return sss::ltrim(result, delims);
}

int cp936_to_ucs2(const std::string& from, std::wstring& to)
{
#if defined(SSS_PLT_MSVC)
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

int ucs2_to_cp936(const std::wstring& from, std::string& to)
{
#if defined(SSS_PLT_MSVC)
    // NOTE 转换得到的到底是 cp936 还是 utf8 呢？
    int len = ::WideCharToMultiByte(CP_ACP, 0, &from[0], from.length(), NULL, 0, NULL, NULL);
    if (len == 0)
    {
        return 0;
    }

    std::vector<char> ret(len + 1);
    ::WideCharToMultiByte(CP_ACP, 0, &from[0], from.length(), &ret[0], len, NULL, NULL);

    to.assign(ret.begin(), ret.end() - 1);
#else
    sss::iConv ic("cp936", "WCHAR_T");
    ic.ucs4tombs(from, to);
#endif
    return to.length();
}

int utf8_to_ucs2(const std::string& from, std::wstring& to)
{
#if defined(SSS_PLT_MSVC)
    int len = ::MultiByteToWideChar(CP_UTF8, 0, &from[0], from.length(), NULL, 0);
    if (len == 0)
    {
        return 0;
    }

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

int ucs2_to_utf8(const std::wstring& from, std::string& to)
{
#if defined(SSS_PLT_MSVC)
    int len = ::WideCharToMultiByte(CP_UTF8, 0, &from[0], from.length(), NULL, 0, NULL, NULL);
    if (len == 0)
    {
        return 0;
    }

    std::vector<char> ret(len + 1);
    ::WideCharToMultiByte(CP_UTF8, 0, &from[0], from.length(), &ret[0], len, NULL, NULL);
    to.assign(ret.begin(), ret.end() - 1);
#else
    sss::iConv ic("utf8", "WCHAR_T");
    ic.ucs4tombs(from, to);
#endif
    return to.length();
}

// 把UTF-8转换成Unicode
// Unicode是一种理论上的编码值；如果将这个值，用int来表示，则不存在字节序的
// 问题（整数）；
// 如果用于二进制传输、写入文件，就会退化到需要考虑字节序问题了。
// 比如，到底是 UCS-32LE,UCS-32BE, UCS-16LE, UCS-16BE 中的那种？（另外，还有
// ：UTF-32LE, UTF-32BE, ... 不存在字节序问题的是utf-8；当然，变长编码的
// cp936,Big5也不存在问题。但问题是，可能被认错编码。）
void UTF_8ToUnicode(wchar_t* pOut, char *pText)
{
    if (!pOut || !pText) {
        return;
    }

    std::pair<uint32_t, int> st = sss::util::utf8::peek(pText);
    if (st.second) {
        pOut[0] = st.first;
    }
    return;
}

// Unicode 转换成UTF-8
void UnicodeToUTF_8(char* pOut, wchar_t* pText)
{
    if (!pOut || !pText) {
        return;
    }

    sss::util::utf8::dumpout2utf8_once(reinterpret_cast<uint32_t*>(pText), pOut);

    return;
}

bool utf8towcs(const unsigned char *src, wchar_t *dst, int maxdstlen )
{
    static const char gUTFBytes[256] = {
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
        3,   3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5
    };

    static const unsigned long gUTFOffsets[6] = {
        0, 0x3080, 0xE2080, 0x3C82080, 0xFA082080, 0x82082080};

    // not used
    //static const unsigned char gFirstByteMark[7] = {
    //    0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

    // Get pointers to our start and end points of the input buffer
    const unsigned char* srcPtr = src;
    const unsigned char* srcEnd = src + strlen((const char *)src);

    // not used
    //wchar_t *dstSave = dst;
    wchar_t *dstEnd = dst + maxdstlen; /* leave room for null */

    // We now loop until we run out of input data.
    while (srcPtr < srcEnd)
    {
        // Get the next leading byte out
        const unsigned char firstByte = (unsigned char)*srcPtr;

        // Special-case ASCII, which is a leading byte value of <= 127
        if (!(firstByte & 0xE0)) {
            *dst++ = (wchar_t) firstByte;
            srcPtr++;
            continue;
        }

        //   See how many trailing src bytes this sequence is going to require
        unsigned int trailingBytes = gUTFBytes[firstByte];

        //   If there are not enough source bytes to do this one, then we
        //   are done.  Note that we done > = here because we are implicitly
        //   counting the 1 byte we get no matter what.
        if (srcPtr + trailingBytes >= srcEnd)
        {
            // 我觉得应该返回失败处的byte位置
            return false;     // ??
        }

        //   Looks ok,   so lets build up the value
        unsigned long tmpVal = 0;
        switch (trailingBytes) {
        case 5: tmpVal += *srcPtr++; tmpVal <<= 6; SSS_FALLTHROUGH;
        case 4: tmpVal += *srcPtr++; tmpVal <<= 6; SSS_FALLTHROUGH;
        case 3: tmpVal += *srcPtr++; tmpVal <<= 6; SSS_FALLTHROUGH;
        case 2: tmpVal += *srcPtr++; tmpVal <<= 6; SSS_FALLTHROUGH;
        case 1: tmpVal += *srcPtr++; tmpVal <<= 6; SSS_FALLTHROUGH;
        case 0: tmpVal += *srcPtr++;
                break;
        default:
                return false;
        }
        tmpVal -= gUTFOffsets[trailingBytes];

        //   If surrogate pairs would be required for 16-bit characters,   fail.
        if (tmpVal & 0xFFFF0000)
            return false;

        if (dst >= dstEnd ) {
            return false;
        }

        *dst++ = (wchar_t)tmpVal;
    }

    *dst = L'\0';

    // return dst-dstSave;
    return true; // check this (CARO)
}

#if defined(SSS_PLT_MSVC)
void UTF_8ToGB2312ByWin(std::string &pOut, char *pText, int pLen)
{
    std::vector<char> newBuf(pLen + 1);
    newBuf[pLen] = '\0';

    char Ctemp[4] = {"\0"};

    int i = 0;
    int j = 0;

    while(i < pLen)
    {
        if(pText[i] > 0)
        {
            newBuf[j++] = pText[i++];
        }
        else
        {
            wchar_t Wtemp;
            UTF_8ToUnicode(&Wtemp,pText + i);

            UnicodeToGB2312(Ctemp,Wtemp);

            newBuf[j] = Ctemp[0];
            newBuf[j + 1] = Ctemp[1];

            i += 3;
            j += 2;
        }
    }
    pOut.assign(newBuf.data(), j);

    return;
}

void GB2312ToUTF_8ByWin(std::string& pOut, char *pText, int pLen)
{
    char buf[4] = {'0'};
    std::vector<char> rst;
    rst.resize(pLen + (pLen >> 2) + 2);

    int i = 0;
    int j = 0;
    while(i < pLen)
    {
        //如果是英文直接复制就可以
        if( *(pText + i) >= 0)
        {
            rst[j++] = pText[i++];
        }
        else
        {
            wchar_t pbuffer;
            Gb2312ToUnicode(&pbuffer, pText+i);

            UnicodeToUTF_8(buf, &pbuffer);

            rst[j] = buf[0];
            rst[j+1] = buf[1];
            rst[j+2] = buf[2];

            j += 3;
            i += 2;
        }
    }
    pOut.assign(rst.data(), j);

    return;
}
#endif

#if !defined(SSS_PLT_MSVC)
int code_convert(
    const char *from_charset,
    const char *to_charset,
    char *inbuf,
    size_t inlen,
    char *outbuf,
    size_t outlen)
{
    char **pin = &inbuf;
    char **pout = &outbuf;

    bool is_ok = true;
    iconv_t cd = ::iconv_open(to_charset,from_charset);
    if (cd==0)
    {
        is_ok = false;
    }
    if (is_ok)
    {
        std::memset(outbuf,0,outlen);
        if (::iconv(cd, pin, &inlen, pout, &outlen) == static_cast<size_t>(-1))
        {
            is_ok = false;
        }
    }
    if (cd)
    {
        ::iconv_close(cd);
    }
    return is_ok? 0 : -1;
}
#endif

} // namespace sss
