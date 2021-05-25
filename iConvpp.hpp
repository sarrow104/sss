// {o: -liconv.dll }
#ifndef  __ICONV_H_1361109813__
#define  __ICONV_H_1361109813__

#include <iconv.h>
#include <string>
#if defined(__linux__) || defined(__unix__) // || defined(_POSIX_VERSION) // for linux | not for OSX(BSD)
#       include <error.h>
#endif
#include <string.h>
#include <errno.h>

#include <algorithm>
#include <exception>
#include <sstream>

//! http://blog.csdn.net/veryhehe2011/article/details/23272927
// 带有BOM信息，直接用"utf-16"；否则应该特别指定"utf-16le"或是"utf-16be"，这个要
// 区分清楚，否则会出现乱码
// iconv_t cd = iconv_open("gb18030//TRANSLIT", "utf-16");
//
// utf-8-->gb18030转换示例
// iconv_t cd = iconv_open("gb18030//TRANSLIT", "utf-8");
//
//! http://blog.csdn.net/luketty/article/details/5747510
// 在Linux上我用的是iconv库.其中wchar_t一般以UCS-4标准。
//
//  UCS-4-INTERNAL ，UCS-2-INTERNAL  会根据本机的存储方式(大端、小端)进行处理。
//
//  还有UCS-2LE和UCS-2BE 分别代表小端和大端模式。
// wchar_t转成utf16
// env = iconv_open("UCS-2-INTERNAL","UCS-4-INTERNAL");
//
// utf16转成wchar_t
// env = iconv_open("UCS-4-INTERNAL","UCS-2-INTERNAL");
//
// TODO
// 这个转换类，用起来还是不是很爽；
// 能否继承自std::ostream ?然后将写入的流，根据需要进行转换呢？

namespace sss {

class iConv
{
#undef  INVALID_ICONV_T
#define INVALID_ICONV_T ((iconv_t)-1)
    enum { INVALID_ICONV_RESULT = (size_t)-1 };
    //static const iconv_t invalid_icv = (iconv_t)-1;
    //enum {invalid_icv = (iconv_t)-1};
    //enum {invalid_icv = -1};
    static const int buffer_length = 100;

    enum work_mode {MODE_TO_WCHAR_T, MODE_FROM_WCHAR_T, MODE_OTHERS};

public:
    class Exception;
    friend class Exception;

    class Exception : public std::exception
    {
    public:
        explicit Exception(const char * iconv_func,
                           const char * iConvFunc,
                           const iConv & i)
        {
            std::ostringstream oss;
            if (iconv_func && iconv_func[0]) {
                oss << iconv_func << ": " << strerror(errno) << "\n\tfrom ";
            }
            oss << "iConv::" << iConvFunc << "()."
                << " at " << std::ios::hex << &i
                << " with (" << i.tocode << ", " << i.fromcode << ") error";
            this->_msg = oss.str();
        }
        ~Exception() throw ()
        {
        }

        const char* what() const throw()
        {
            return _msg.c_str();
        }
    private:
        std::string _msg;
    };

public:
    iConv();
    iConv(const std::string& to_code, const std::string& from_code);
    ~iConv();

public:
    std::string operator() (const std::string& from)
    {
        return this->convert(from);
    }
    bool is_ok() const;

    bool is_coding_equal() const;

    void set_from(const std::string& );
    void set_to(const std::string& );
    void set(const std::string& to_code, const std::string& from_code);

    int  convert(std::string& to, const std::string& from);
    std::string convert(const std::string& from);

    int mbstoucs4(const std::string& mbs, std::wstring& ucs4);

    int ucs4tombs(const std::wstring& ucs4, std::string& mbs);

    void clear();
    void print(std::ostream& o) const;
    std::string describe() const {
        std::ostringstream oss;
        this->print(oss);
        return oss.str();
    }

private:
    void do_setting();

private:
    work_mode   _mode;
    iconv_t     icv;
    std::string tocode;
    std::string fromcode;
};

inline std::ostream& operator << (std::ostream& o, const sss::iConv& iv)
{
    iv.print(o);
    return o;
}

} // namespace sss

#endif  /* __ICONV_H_1361109813__ */
