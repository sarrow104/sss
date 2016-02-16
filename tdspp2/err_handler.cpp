#include "handlers.hpp"

#include <sybdb.h>
#include <sss/path.hpp>
#include <sss/log.hpp>

namespace sss {
    namespace tdspp2 {

// When first writing the handler, pay careful attention to the precise type of
// each parameter. Only by carefully matching them will you convince a modern C
// compiler that the address of your function is of the type accepted by
// dberrhandle(). [5]
int
err_handler(DBPROCESS * dbproc,
            int severity, int dberr, int oserr,
            char *dberrstr, char *oserrstr)
{
    (void)dbproc;
    // Some messages are so severe they provoke DB-Library into calling the
    // error handler, too! If you have both installed ¡ª and of course you do,
    // right? ¡ª then you can skip those lacking an error number.

    std::string appname = sss::path::getbin();
    if (dberr) {
        SSS_LOG_ERROR("%s: Msg %d, Level %d\n",
                      appname.c_str(), dberr, severity);
        SSS_LOG_ERROR("\t%s\n", dberrstr);
    }

    if (oserr) {
        SSS_LOG_ERROR("%s: DB-LIBRARY error:\n", appname.c_str());
        SSS_LOG_ERROR("\t%s\n", oserrstr);
    }

    // While INT_CANCEL is the most common return code, it's not the only one.
    // For one thing, the error handler's return code can control how long
    // DB-Library keeps retrying timeout errors. See the documentation for
    // details.
    return INT_CANCEL;
}

} // namespace tdspp2
} // namespace sss
