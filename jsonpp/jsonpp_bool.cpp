#include "jsonpp.hpp"

namespace sss {
namespace jsonpp
{
    JBool::JBool(bool val)
        : JValue(JSON_BOOLEAN), data(val)
    {
    }

    JBool::~JBool()
    {
    }

    bool  JBool::get_bool()                           const
    {
        return this->data;
    }

    std::string     JBool::to_str()                const
    {
        return this->data ? "true" : "false";
    }

    bool   JBool::swap(JValue& val)
    {
        JBool * ref = dynamic_cast<JBool*>(&val);
        if (ref) {
            std::swap(this->data, ref->data);
            return true;
        }
        return false;
    }

    JValue * JBool::clone()                        const
    {
        return new jsonpp::JBool(this->get_bool());
    }
}

}
