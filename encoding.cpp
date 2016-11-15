#include "encoding.hpp"

#include <stdexcept>

#include <sss/iConvpp.hpp>
#include <sss/path.hpp>
#include <sss/spliter.hpp>
#include <sss/utlstring.hpp>

#ifdef __WIN32__
#define _USING_UCHARDET_
#endif

#ifdef _USING_UCHARDET_
#include <uchardet/uchardet.h>
#else
#include <sss/ps.hpp>
#include "popenRWE.h"
#endif

namespace {
inline void encoding_normalize(std::string& encoding)
{
    sss::to_lower(encoding);
    // utf-8, utf-16be utf-32le
    if (sss::is_begin_with(encoding, "utf-") ||
        sss::is_begin_with(encoding, "ucs-")) {
        encoding.erase(encoding.begin() + 3);
    }
    if (encoding.empty() || encoding == "none") {
        encoding.assign("ascii");
    }
    // ucs-2     16 bit UCS-2 encoded Unicode (ISO/IEC 10646-1)
    // ucs-2le   like ucs-2, little endian
    // utf-16    ucs-2 extended with double-words for more characters
    // utf-16le  like utf-16, little endian
    // ucs-4     32 bit UCS-4 encoded Unicode (ISO/IEC 10646-1)
    // ucs-4le   like ucs-4, little endian
}
}

namespace sss {
// 确保编码满足——有些标准是包含关系，所以，有些时候，就算编码不同，也没有必要转化；
void Encoding::ensure(std::string& content, std::string to_encoding,
                      const std::string& encodings)
{
    encoding_normalize(to_encoding);
    std::string from_encoding;
    if (encodings.empty()) {
        from_encoding = Encoding::detect(content);
    }
    else {
        from_encoding = Encoding::encodings(content, encodings);
    }
    if (!Encoding::isCompatibleWith(from_encoding, to_encoding)) {
        std::string out;
        sss::iConv ic(to_encoding, from_encoding);
        if (ic.convert(out, content)) {
            std::swap(out, content);
        }
        else {
            SSS_LOG_ERROR("an error occured while iconv from %s to %s\n",
                          from_encoding.c_str(), to_encoding.c_str());
        }
    }
}

// NOTE 所谓 compatible
// 是说，一种编码如果写成另外一种编码，是否可以省略转换的过程？
// 因为，有些编码方式，它们之间存在，单向包含的情况；因此，可以忽略编码的不同，从而省略掉iconv转
// 化的动作。
/// ----------------------------------------------
// 其实主要是针对中文编码；
// 貌似，中文编码，都是向下兼容的；所以，根本没有必要转化？
// cp936<gb2312<gbk<gb18030
// 需要注意的是，上面这几种中文编码方式，都是变长的字符序列；mbcs（多字节字符序列）
// 比如gb18030，它兼容ascii，就是说，最短的是一个字节长度！；然后大部分是2字节长度；但需要注意的
// 是，还有6000+个的4字节长度字符！见：
/// http://blog.csdn.net/zhoubl668/article/details/6914018
//
// 另外，还有繁体字符集：
// BIG5<CP950
// 也是向下兼容的；当然，也是变长的字符序列——英文单字节，中文双字节；
//
// 最后需要注意的是，虽然CP950,gb18030都可以转化成utf8编码；但是，这两种编码，只能单向转化！
//
// 因为，utf8对应着所有的UNICODE字符集；而gb18030和CP950，只是有交集的UNICODE的子集而已！
// gb18030完全覆盖了CP950的字符集！是可以转化的；
//
// 另外，ucs2与utf16也存在上述单向转化的问题；ucs2是固定长度为2的文本编码方式；就是说，它只能涵
// 盖UNICODE的部分字符；而utf16，则是变长的规范；它可能的长度是2字节或者4字节；4字节的时候，被称
// 为一个"代理对"；
//
// 简单的说，UTF-16可看成是UCS-2的父集。在没有辅助平面字符（surrogate code
// points）前，UTF-16与
// UCS-2所指的是同一的意思。（严格的说这并不正确，因为在UTF-16中从U+D800到U+DFFF的码位不对应于任
// 何字符，而在使用UCS-2的时代，U+D800到U+DFFF内的值被占用。）但当引入辅助平面字符后，就称为
// UTF-16了。
//
// 参考链接：
/// http://zh.wikipedia.org/wiki/UTF-16
bool Encoding::isCompatibleWith(std::string from, std::string to)
{
    encoding_normalize(from);
    encoding_normalize(to);

    if (from == to) {
        return true;
    }
    else if (from == "big5" && to == "cp950") {
        return true;
    }
    else if ((from == "cp936" &&
              (to == "gb2312" || to == "gbk" || to == "gb18030")) ||
             (from == "gb2312" && (to == "gbk" || to == "gb18030")) ||
             (from == "gbk" && to == "gb18030")) {
        return true;
    }
    else if ((from == "ucs2" && to == "utf16") ||
             (from == "ucs2le" && to == "utf16le")) {
        return true;
    }
    // NOTE
    // uchardet-lib，will return "" while parse ascii docment.
    // uchardet will return "ascii/unkown", from the same file.
    else if ((from == "ascii" || from == "") &&
             (to == "utf8" || to == "cp936" || to == "cp950" || to == "big5" ||
              to == "euc-jp" || to == "euc-kr" || to == "euc-cn" ||
              to == "gb18030")) {
        return true;
    }

    return false;
}

// 从某个方向，到某个方向是可转化的不？
// 对于字符集来说，虽然有相交、没有交集；包含这些关系；但是，实际iConv::convert动作的时候，针对
// 的，其实是字符序列；
//
// 如果，一个 gb18030
// 编码的字符串，里面没有简体中文字符，全部都是由繁体字符组成的，那么这个
// gb18030的字符串，也是可以转化成cp950编码的！
// 或者说，是否可以转化，是iconv说了算；具体错误的bite在什么位置，也只有转化的时候才知道！
// bool isConvertable(std::string from, std::string to)
//{
//}

// 返回编码信息；
std::string Encoding::detect(const std::string& content)
{
#ifdef _USING_UCHARDET_
    uchardet_t ud = uchardet_new();
    int err = uchardet_handle_data(ud, content.c_str(), content.length());
    if (!err)
    {
        uchardet_data_end(ud);

        std::string encoding = uchardet_get_charset(ud);
    }

    uchardet_delete(ud);

    if (!err) {
        encoding_normalize(encoding);
        if (encoding.empty()) {
            encoding.assign("ascii");
        }
    }

    if (err) {
        /* 如果样本字符不够，那么有可能导致分析失败 */
        throw std::runtime_error("uchardet: analyze coding faild！\n");
    }
    return encoding;
#else
    // Python 支持两种使用方式：
    //
    // 1. 外部文件；
    // 2. stdin读取；
    //
    // 前者，不一定适用；因为字符不一定来自外存；
    // 后者的麻烦在于，需要实现双向管道！
    int rwepipe_data[3] = {0, 0, 0};
    int pid = popenRWE(rwepipe_data, "chardet");
    if (pid == -1) {
        return "";
    }
    // TODO
    // 只写入至多1000行；貌似，这是不可能完成的任务！
    //
    // 这是因为，如果是没有'\0'字节的还好，判断'\n'，就能分辨出，是否过了一行；
    // 如果是有'\0'的，就说明是ucs2、ucs4这种编码——意味着，需要判断“字”
    // 的长度，才方便传入合适的字节数；
    //
    // 不然，如果在某一个“字”的中间，发生了切断，很可能导致判断出错；
    //
    // 当然，这种情况下，传入可以被4整除的字节数是一个安全的作法；
    //
    // 不定长的编码，那么，判断行（或者任意一个ascii字符），为结尾，比较安全；
    //
    // 那么，麻烦了——这是一个死循环；因为ucs2，将很难与不定长类的编码，区分开！
    size_t w_cnt = write(rwepipe_data[0], content.c_str(), content.length());
    if (w_cnt != content.length()) {
        return "";
    }
    close(rwepipe_data[0]);
    rwepipe_data[0] = -1;
    char out_buf[1024];
    size_t cnt = read(rwepipe_data[1], out_buf, sizeof(out_buf));
    out_buf[cnt] = '\0';
    pcloseRWE(pid, rwepipe_data);
    if (!sss::is_begin_with(out_buf, "<stdin>: ")) {
        return "";
    }
    char* next_space = std::strchr(out_buf + 9, ' ');
    if (next_space) {
        next_space[0] = '\0';
        std::string encoding(out_buf + 9);

        encoding_normalize(encoding);
        return encoding;
    }
    return "";
#endif
}

std::string Encoding::fencoding(const std::string& fname)
{
    std::string fpath = sss::path::full_of_copy(fname);
#ifdef _USING_UCHARDET_
    std::string content;
    sss::path::file2string(fpath, content);
    return Encoding::detect(content);
#else
    sss::ps::StringPipe sp;
    sp.add("chardet").add(fpath);
    std::string raw_encoding = sp.run();
    if (sss::is_begin_with(raw_encoding, fpath)) {
        const char* s_enc = raw_encoding.c_str() + fpath.length();
        if (sss::is_begin_with(s_enc, ": ")) {
            s_enc += 2;
            const char* s_enc_end = std::strchr(s_enc, ' ');
            if (s_enc_end) {
                std::string encoding(s_enc, s_enc_end - s_enc);
                encoding_normalize(encoding);
                return encoding;
            }
        }
    }
    return "";
#endif
}

std::string Encoding::encodings(const std::string& content,
                                const std::string& encodings)
{
    std::string encoding;
    bool has_found = false;
    sss::Spliter sp(encodings, ',');
    std::string out;
    while (sp.fetch_next(encoding)) {
        sss::trim(encoding);
        try {
            sss::iConv ic(encoding, encoding);
            if (!ic.is_ok()) {
                continue;
            }
            if (ic.convert(out, content)) {
                has_found = true;
                break;
            }
        }
        catch (...) {
            continue;
        }
    }

    return has_found ? encoding : "";
}
}
