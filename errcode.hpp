#ifndef __ERRCODE_HPP_1472967839__
#define __ERRCODE_HPP_1472967839__

#include <algorithm>
#include <stdexcept>
#include <string>

#include <cstring>

namespace sss {
struct errcode : public std::pair<int, std::string> {
    errcode() : std::pair<int, std::string>(0, "") {}
    explicit errcode(int err, const std::string& msg = "")
        : std::pair<int, std::string>(err,
                                      msg.empty() ? std::strerror(err) : "")
    {
    }

    void set(int err, const std::string& msg = "")
    {
        if (err) {
            this->first = err;
            this->second = msg.empty() ? std::strerror(err) : "";
        }
    }

    template <typename Exception>
    void throwMe() const
    {
        if (this->first) {
            throw Exception(this->to_str());
        }
    }
    int to_errno() const { return this->first; }
    operator int() const { return this->to_errno(); }
    const std::string& to_str() const { return this->second; }
};
}  // namespace sss

#endif /* __ERRCODE_HPP_1472967839__ */
