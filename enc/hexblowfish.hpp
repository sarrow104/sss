#ifndef  __HEXBLOWFISH_HPP_1336579487__
#define  __HEXBLOWFISH_HPP_1336579487__

#include <sss/enc/Blowfish.hpp>
#include <sss/enc/encbase.hpp>

#include <iosfwd>

namespace sss { namespace enc {
class Hexblowfish : public EncBase {
private:
    CBlowFish bf;
public:
    Hexblowfish(const std::string& key);
    ~Hexblowfish();

public:
    std::string encode(const std::string& pass);
    std::string decode(const std::string& pass);
};

} // namespace enc
} // namespace sss

#endif  /* __HEXBLOWFISH_HPP_1336579487__ */
