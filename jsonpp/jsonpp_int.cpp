#include "jsonpp.hpp"

#include <sss/utlstring.hpp>

namespace sss {
namespace jsonpp
{
    JInt::JInt(double val)
        : JValue(JSON_INT), data(val)
    {
    }

    JInt::~JInt()
    {
    }

    int JInt::get_int()            const
    {
        return this->data;
    }

    bool   JInt::swap(JValue& val)
    {
        JInt * ref = dynamic_cast<JInt*>(&val);
        if (ref) {
            std::swap(this->data, ref->data);
            return true;
        }
        return false;
    }

    JValue * JInt::clone()                        const
    {
        return new jsonpp::JInt(this->get_int());
    }

    std::string     JInt::to_str()                const
    {
        return sss::cast_string(this->data);
    }
}
}

