#ifdef __MINGW32__
#include <windows.h>
 #endif
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>

// #include "nanodbc_utils.hpp"

namespace odbc {
namespace util {

bool sqltype_is_numeric(int type)
{
    switch (type)
    {
        case SQL_C_SSHORT:
        case SQL_C_USHORT:
        case SQL_C_SLONG:
        case SQL_C_ULONG:
        case SQL_C_FLOAT:
        case SQL_C_DOUBLE:
        case SQL_C_STINYINT:
        case SQL_C_UTINYINT:
        case SQL_C_SBIGINT:
        case SQL_C_UBIGINT:
            return true;

        default:
            return false;
    }
}

bool sqltype_is_binary(int type)
{
    switch (type)
    {
        case SQL_C_BINARY:
            return true;

        default:
            return false;
    }
}

} // namespace util
} // namespace odbc
