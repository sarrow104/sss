#include "jsonpp.hpp"

#include <iostream>

namespace sss{
namespace jsonpp
{
    JString::JString(const std::string& val)
        : JValue(JSON_STRING), data(val)
    {
    }

    JString::~JString()
    {
    }

    std::string     JString::get_string()            const
    {
        return this->data;
    }

    bool   JString::swap(JValue& val)
    {
        JString * ref = dynamic_cast<JString*>(&val);
        if (ref) {
            this->data.swap(ref->data);
            return true;
        }
        return false;
    }

    JValue * JString::clone()                        const
    {
        return new jsonpp::JString(this->get_string());
    }

    std::string     JString::to_str()                const
    {
        return this->get_string();
    }
}
}
