#include "tdspp.hpp"

namespace sss {
namespace tdspp2 {

const char * Exception::what() const throw()
{
    return this->message.c_str();
}

} // namespace tdspp2
} // namespace sss
