#include <sss/iConvpp.hpp>

#include <iconv.h>
#include <errno.h>

#include <map>
#include <string>
#include <sstream>

#include <cstdlib>
#include <stdexcept>

#include <sss/log.hpp>
#include <sss/bit_operation/bit_operation.h>
#include <sss/utlstring.hpp>

namespace {
    bool is_wchar_t_codename(const std::string& codename)
    {
        return
            (sss::is_begin_with(codename, "wchar_t", true) ||
             sss::is_begin_with(codename, "ucs4", true) ||
             sss::is_begin_with(codename, "ucs-4", true));
    }
}

namespace sss {

    // 貌似iconv遇到utf8的bom会发生错误！
iConv::iConv()
    : icv(INVALID_ICONV_T)
{
}

iConv::iConv(const std::string& to_code, const std::string& from_code)
    : icv(INVALID_ICONV_T), tocode(to_code), fromcode(from_code)
{
    this->do_setting();
}

iConv::~iConv()
{
    this->clear();
}

bool iConv::is_ok() const
{
    return this->icv != INVALID_ICONV_T;
}

bool iConv::is_coding_equal() const
{
    std::string seq = "//";

    std::string::const_iterator fromend =
        std::search(fromcode.begin(), fromcode.end(), seq.begin(), seq.end());

    std::string::const_iterator toend   =
        std::search(tocode.begin(), tocode.end(), seq.begin(), seq.end());

    return
        std::distance(fromcode.begin(), fromend) == std::distance(tocode.begin(), toend) &&
        sss::is_begin_with(fromcode.begin(), fromend, tocode.begin(), toend, true);
}

void iConv::set_from(const std::string& from_code)
{
    this->fromcode = from_code;
    this->do_setting();
}

void iConv::set_to(const std::string& to_code)
{
    this->tocode = to_code;
    this->do_setting();
}

void iConv::set(const std::string& to_code, const std::string& from_code)
{
    this->tocode = to_code;
    this->fromcode = from_code;
    this->do_setting();
}

void iConv::clear()
{
    if (this->is_ok())
    {
        iconv_close(this->icv);
        // NOTE 不要清空tocode 和 fromcode !
        //! this->tocode.clear();
        //! this->fromcode.clear();
    }
}

// 注意，iconv内部，使用char * 来处理任意编码。就是说，本来需要wchar_t才能保存
// 的UCS-2字符串，在转换的时候，也使用char序列来进行中间处理。
//
// 问题就来了，如何转换成用户需要的类型呢？
//
// std::wstring 与 std::string 还有 wchar_t 与 char。
//
// 前两个还好说，后两个就必须要考虑结束符的问题。wchar_t 的结束符是L"\0"，而 char 是 "\0"！
//
// NOTE 本程序利用 ostringstream 作为缓存，然后利用稍微小一点的buffer，一段一段地翻译字符，

int iConv::convert(std::string& to, const std::string& from)
{
    if (!this->is_ok())
    {
        throw Exception(0, __func__, *this);
    }

    std::ostringstream oss;
    std::ostringstream msg;
    msg << "iConv(" << std::ios::hex << this << ")::" << __func__ << "()";

    //size_t char_count = 0;

    char *in = const_cast<char*>(from.c_str());
    size_t  in_len = from.length();

    char  to_buffer[iConv::buffer_length + 1];

    while (true)
    {
        // NOTE
        // 1. iconv() 函数，不会以 NTS (Null Terminate String) 形式输出转换后的字节序；
        //    即，当读取一个正确的多字节字符，就转换该字符，并写入合适的字节数；
        //    同时修改输入的四个指针变量，所指向的对象；
        //    如果需要附加"\0"或者L"\0"，需要用户自己处理——因为，只有用户自己
        //    才知道，转换后的序列，到底应当用几个字节的"\0"，来作为串的结束标志
        //    ！
        // 2. &out_buf_left 不包括最后的结尾NULL字符——虽然确实会有"写"内存的动作。
        //    证据是，当做cp936 中文字符，转utf-8的时候，2 byte的流，将得到 3 byte长的流。
        //    如果 out_buf_left = 2，则会发生 E2BIG 错误；如果 out_buf_left = 3
        //    则会正常转换。并且输出第四个字节的时候，会发现是一个"\0"。
        //    就是说，在提供 iconv() 函数的输出 buffer 的时候，需要使用者自行多
        //    提供一个字节。
        char * out = to_buffer;
        size_t out_buf_len = iConv::buffer_length;
        size_t out_buf_left = out_buf_len;
        size_t nonreversible_converts = iconv(this->icv, &in, &in_len, &out, &out_buf_left);

        sss::log::debug("transfered %d bytes.\n", out_buf_len - out_buf_left);
        if (sss::log::is_level_on(sss::log::log_DEBUG))
        {
            for (int i = 0; i <= int(out_buf_len - out_buf_left); ++i)
            {
                std::cout << ext::binary << to_buffer[i] << std::endl;
            }
        }

        // 确保末尾字节是一个"\0"。
        to_buffer[out_buf_len - out_buf_left] = '\0';

        oss << to_buffer;

        if (nonreversible_converts == INVALID_ICONV_RESULT )
        {
            switch (errno)
            {
            case E2BIG:
                {
                    sss::log::debug("%s %s\n",
                                    msg.str().c_str(),
                                    " NOTE: note enough buffer for converting\n");
                    continue;
                }
                break;

            case EILSEQ:
                {
#if 0
                    sss::log::error("%s in string %s ... of %d-th byte, can-not convert sequence %s had-been found\n",
                                    msg.str().c_str(),
                                    from.substr(0, 5).c_str(), int(in - &from[0]),
                                    from.substr(in - &from[0], 5).c_str());
                    return false;
#else
                    msg << " in string " << from.substr(0, 5) << " ... of "
                        << int(in - &from[0]) << "-th byte, `"
                        << from.substr(in - &from[0], 5) << "` convert failed; "
                        << strerror(errno);
                    throw std::runtime_error(msg.str());
#endif
                }
                break;

            case EINVAL:
                {
#if 0
                    // NOTE 不完整的输入序列，通常不意味着错误，而是意味着后面还
                    // 有字符……
                    sss::log::error("%s in string %s ... of %d-th byte, in-complete sequence %s had-been found\n",
                                    msg.str().c_str(),
                                    from.substr(0, 5).c_str(), int(in - &from[0]),
                                    from.substr(in - &from[0], 5).c_str());
                    return false;
#else
                    msg << " in string " << from.substr(0, 5) << " ... of "
                        << int(in - &from[0]) << "-th byte, `"
                        << from.substr(in - &from[0], 5) << "` convert failed; "
                        << strerror(errno);
                    throw std::runtime_error(msg.str());
#endif
                }
                break;
            }
        }
        else
        {
            sss::log::debug("nonreversible_converts cout : %d\n",
                            nonreversible_converts);
            break;
        }
    }
    to = oss.str();
    return true;
}

std::string iConv::convert(const std::string& from)
{
    std::string to;
    this->convert(to, from);
    return to;
}

// NOTE 需要确保，fromcode 或者 tocode 必须是"WCHAR_T"
// 返回成功消耗的字节数
// 转换后的字符数，见
int iConv::mbstoucs4(const std::string& mbs, std::wstring& ucs4)
{
    if (this->_mode != MODE_TO_WCHAR_T) {
        throw Exception(0, __func__, *this);
    }

    // NOTE errno 是线程安全的！
    // 我下面一开始就分配足够的空间，避免多次分配
    std::vector<wchar_t> tmpout;
    tmpout.resize(mbs.length());

    char * inptr = const_cast<char*>(mbs.c_str());
    size_t insize = mbs.length();

    char  * outptr = reinterpret_cast<char*>(tmpout.data());
    size_t outsize = tmpout.size() * sizeof(wchar_t);

    size_t nconv = iconv(this->icv, &inptr, &insize, &outptr, &outsize);
    // NOTE
    if (nconv == INVALID_ICONV_RESULT) {
        switch (errno)
        {
        case E2BIG:
            {
                std::ostringstream oss;
                oss << __func__ << " already allocate enough space; but iconv(): " << strerror(errno);
                throw std::logic_error(oss.str());
            }
            break;

        case EILSEQ:
        case EINVAL:
            break;
        }
    }
    if (outsize % sizeof(wchar_t)) {
        std::ostringstream oss;
        oss << __func__ << " iconv() write byte alignment error; remainder : " << (outsize % sizeof(wchar_t));
        throw std::logic_error(oss.str());
    }
    tmpout.resize(tmpout.size() - outsize / sizeof(wchar_t));
    ucs4.resize(tmpout.size());
    std::copy(tmpout.begin(), tmpout.end(), ucs4.begin());
    return mbs.length() - insize;
}

// NOTE 需要确保，fromcode 或者 tocode 必须是"WCHAR_T"
// 返回成功消耗的字符数
int iConv::ucs4tombs(const std::wstring& ucs4, std::string& mbs)
{
    if (this->_mode != MODE_FROM_WCHAR_T) {
        throw Exception(0, __func__, *this);
    }
    std::vector<char> tmpout;
    // 一个wchar_4最多转换到6个byte！
    tmpout.resize(ucs4.length() * 6);

    char * inptr = const_cast<char*>(reinterpret_cast<const char *>(ucs4.c_str()));
    size_t insize = ucs4.length() * sizeof(wchar_t);

    char * outptr  = tmpout.data();
    size_t outsize = tmpout.size() * sizeof(char);

    size_t nconv = iconv(this->icv, &inptr, &insize, &outptr, &outsize);

    if (nconv == INVALID_ICONV_RESULT) {
        switch (errno)
        {
        case E2BIG:
            {
                std::ostringstream oss;
                oss << __func__ << " already allocate enough space; but iconv(): " << strerror(errno);
                throw std::logic_error(oss.str());
            }
            break;

        case EILSEQ:
        case EINVAL:
            break;
        }
    }
    if (outsize % sizeof(char)) {
        std::ostringstream oss;
        oss << __func__ << " iconv() write byte alignment error; remainder : " << (outsize % sizeof(char));
        throw std::logic_error(oss.str());
    }
    tmpout.resize(tmpout.size() - outsize / sizeof(char));
    mbs.resize(tmpout.size());
    std::copy(tmpout.begin(), tmpout.end(), mbs.begin());
    return ucs4.length() * sizeof(wchar_t) - insize;
}

void iConv::do_setting()
{
    this->clear();
    this->icv = iconv_open(this->tocode.c_str(), this->fromcode.c_str());
    if (!this->is_ok())
    {
        throw Exception("iconv_open", __func__, *this);
    }
    if (is_wchar_t_codename(this->tocode)) {
        this->_mode = MODE_TO_WCHAR_T;
    }
    else if (is_wchar_t_codename(this->fromcode)) {
        this->_mode = MODE_FROM_WCHAR_T;
    }
}

} // end of namespace sss

// NOTE 范例与说明 {{{1
//iconv_t iconv_open(const char *tocode, const char *fromcode);
//size_t iconv(iconv_t cd, char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);
//int iconv_close(iconv_t cd);
//
//iconv_open函数用来打开一个编码转换的流，iconv函数的作用是实际进行转换，
//iconv_close函数的作用就是关闭这个流。实际用法参见下面的例子，下面是一个将
//UTF-8码转换成GBK码的例子，我们假设已经有了一个uft8编码的输入缓冲区inbuf以及这
//个缓冲区的长度inlen。
//iconv_t cd = iconv_open( "GBK", "UTF-8");
//char *outbuf = (char *)malloc(inlen * 4 );
//bzero( outbuf, inlen * 4);
//char *in = inbuf;
//char *out = outbuf;
//size_t outlen = inlen *4;
//iconv（cd, &in, (size_t *)&inlen, &out,&outlen）;
//outlen = strlen(outbuf);
//printf("%s\n",outbuf);
//free(outbuf);
//iconv_close(cd);
//非常值得注意的地方是：iconv函数会修改参数in和参数out指针所指向的地方，也就是
//说，在调用iconv函数之前，我们的in和inbuf指针以及out和outbuf指针指向的是同一块
//内存区域，但是调用之后out指针所指向的地方就不是outbuf了，同理in指针。所以要
//char *in = inbuf;
//char *out = outbuf;
//另存一下，使用或者释放内存的时候也要使用原先的那个指针outbuf和inbuf。
//
//----------------------------------------------------------------------
//
//bool gbk2utf8(std::string& to, const std::string& from)
//{
//    iconv_t ict = iconv_open("utf8", "cp936");
//    to.resize(from.length()*3);
//    char *in = const_cast<char*>(&from[0]);
//    char *out = &to[0];
//    size_t in_len = from.length();
//    // cp936字符串 如果是偶数bytes长，那么转换为utf8后，长度增加1/2。
//    // 如果为奇数长。那么，至少有一个asc字符——而这个字符是无需转换的。
//    // 于是，有了下面这个缓冲区计算公式。
//    size_t out_buf_len = (from.length()/2)*3 + from.length()%2;
//    size_t out_buf_left = out_buf_len;
//    size_t nonreversible_converts = iconv(ict, &in, &in_len, &out, &out_buf_left);
//
//    if (nonreversible_converts == (size_t)-1)
//        return false;
//    //std::cout << "nonreversible_converts" << nonreversible_converts << std::endl;
//    to.resize(out_buf_len - out_buf_left);
//    iconv_close(ict);
//
//    return true;
//}
////
//

/**
  iconv(3) - Linux man page
  iconv_open(3)
  iconv_close(3)

  ../../doc/iconv-man-page.chn.txt
  */
