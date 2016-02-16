#include "jsonpp.hpp"

namespace sss{
namespace jsonpp
{
    JNull::JNull()
        : JValue(JSON_NULL)
    {
    }

    JNull::~JNull()
    {
    }

    void * JNull::get_null()            const
    {
        return 0;
    }

    std::string     JNull::to_str()                const
    {
        return "null";
    }

    bool   JNull::swap(JValue& val)
    {
        JNull * ref = dynamic_cast<JNull*>(&val);
        if (ref) {
            return true;
        }
        return false;
    }

    JValue * JNull::clone()                        const
    {
        return new jsonpp::JNull;
    }
}

}
