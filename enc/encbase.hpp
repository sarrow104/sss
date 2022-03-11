#ifndef  __ENCBASE_HPP_1373116057__
#define  __ENCBASE_HPP_1373116057__

#include <iosfwd>

namespace sss { namespace enc {

// 目的，为字符串的加密解密，提供一个基类，方便外部调用。
class EncBase
{
public:
    EncBase();
    virtual ~EncBase() = 0;

public:
    virtual std::string encode (const std::string& s) = 0;
    virtual std::string decode (const std::string& s) = 0;
};

} // namespace enc
} // namespace sss


#endif  /* __ENCBASE_HPP_1373116057__ */
