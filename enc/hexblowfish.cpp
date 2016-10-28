#include <sss/enc/hexblowfish.hpp>

#include <string>
#include <cstring>

#include <sss/utlstring.hpp>

namespace sss { namespace enc {

Hexblowfish::Hexblowfish(const std::string& key)
        :bf((const unsigned char*)key.c_str(), key.length())
{
}

Hexblowfish::~Hexblowfish()
{
}

std::string Hexblowfish::encode( const std::string& pass)
{
    int len = int(pass.length());
    if (len % 8 != 0)
        len += (8 - (len % 8));
    unsigned char password[len];       // 长度参数，必须是8的整数倍！
    memset(password, 0, sizeof(password));
    memcpy((char*)password, &pass[0], pass.length());

    this->bf.Encrypt(password, sizeof(password));
    //std::cout << "加密后：" << sss::to_hex(password, password+16) << std::endl;
    return sss::to_hex(password, password + sizeof(password));
}

std::string Hexblowfish::decode( const std::string& pass)
{
    assert(pass.length()%8 == 0);
    //unsigned char buffer[password.length() /2 ];
    int buff_len = pass.length();
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
