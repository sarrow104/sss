#include "error.hpp"

namespace sss {
    namespace error {
        bool or_throw(const std::string& msg) {
            int prev_error = errno;
            errno = 0;
            throw std::runtime_error(msg + std::strerror(prev_error));
            return false;
        }

    } // namespace error
} // namespace sss
