#ifndef __ERROR_HPP_1455717139__
#define __ERROR_HPP_1455717139__

#include <string>
#include <stdexcept>
#include <cstring>

namespace sss {
    namespace error {
        bool or_throw(const std::string& msg = "");
    } // namespace error
} // namespace sss


#endif /* __ERROR_HPP_1455717139__ */
