#include "jsonpp.hpp"

#include <sss/utlstring.hpp>

namespace sss{
namespace jsonpp
{
    JDouble::JDouble(double val)
        : JValue(JSON_DOUBLE), data(val)
    {
    }

    JDouble::~JDouble()
    {
    }

    double JDouble::get_double()            const
    {
        return this->data;
    }

    bool   JDouble::swap(JValue& val)
    {
        JDouble * ref = dynamic_cast<JDouble*>(&val);
        if (ref) {
            std::swap(this->data, ref->data);
            return true;
        }
        return false;
    }

    JValue * JDouble::clone()                        const
    {
        return new jsonpp::JDouble(this->get_double());
    }

    std::string     JDouble::to_str()                const
    {
        return sss::cast_string(this->data);
    }
}
}
