#include "jsonpp.hpp"

#include "jsonpp_vprinter.hpp"

namespace sss {
namespace jsonpp
{
    std::ostream& operator << (std::ostream& o, const JValue& jval)
    {
        jval.print(o);
        return o;
    }
}

}
