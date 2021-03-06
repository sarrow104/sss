#include "Environment.hpp"

namespace sss {
namespace tdspp2 {

    bool Environment::insert_query(DBPROCESS *, Query*)
    {
        return true;
    }
    bool Environment::remove_query(DBPROCESS *)
    {
        return true;
    }

    bool Environment::insert_dblink(DBPROCESS *, DBLink*)
    {
        return true;
    }
    bool Environment::remove_dblink(DBPROCESS *)
    {
        return true;
    }

    Query& Environment::refer_query(DBPROCESS*)
    {
        // TODO FIXME
        static Query q{nullptr};
        return q;
    }
    DBLink& Environment::refer_dblink(DBPROCESS*)
    {
        // TODO FIXME
        static Query q{nullptr};
        return *(DBLink*)(nullptr);
    }

} //namespace tdspp2
} // namespace sss
