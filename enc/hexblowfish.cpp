#include <sss/enc/hexblowfish.hpp>

#include <cstring>
#include <string>

#include <sss/utlstring.hpp>

namespace sss {
namespace enc {

const int byte_width = 8;

Hexblowfish::Hexblowfish(const std::string& key)
        :bf(reinterpret_cast<const unsigned char*>(key.c_str()), key.length())
{
}

Hexblowfish::~Hexblowfish() = default;

std::string Hexblowfish::encode( const std::string& pass)
{
    int len = int(pass.length());
    if (len % byte_width != 0) {
        len += (byte_width - (len % byte_width));
    }
    unsigned char password[len];       // 长度参数，必须是8的整数倍！
    memset(password, 0, sizeof(password));
    memcpy(password, &pass[0], pass.length());

    this->bf.Encrypt(password, sizeof(password));
    //std::cout << "加密后：" << sss::to_hex(password, password+16) << std::endl;
    return sss::to_hex(password, password + sizeof(password));
}

std::string Hexblowfish::decode( const std::string& pass)
{
    assert(pass.length()%byte_width == 0);
    //unsigned char buffer[password.length() /2 ];
    int buff_len = int(pass.length());
    std::string ret = sss::hex2string_copy(pass);
    //std::cout << "after :" << password << password.length() << std::endl;
    this->bf.Decrypt(reinterpret_cast<unsigned char*>(const_cast<char*>(ret.c_str())), buff_len);

    // NOTE
    // 莫名其妙，写成
    // ret.resize(strlen(ret.c_str());
    // 就没有变化！
    std::string::size_type last = ret.find('\0');
    if (last != std::string::npos)
    {
        ret.resize(last);
    }
    //int len = strlen(ret.c_str());
    //ret.resize(len);

    return ret;
}

} // namespace enc
} // namespace sss
