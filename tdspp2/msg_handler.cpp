#include "handlers.hpp"

#include <sybdb.h>

#include <sss/log.hpp>
#include <sss/path.hpp>

namespace sss {
namespace tdspp2 {

// When first writing a handler, pay careful attention to the precise type of
// each parameter. Only by carefully matching them will you convince a modern C
// compiler that the address of your function is of the type accepted by
// dbmsghandle().
//
// TODO 如何通过 DBPROCESS * 句柄 回溯数据库链接信息？
// FIXME 可以将DBPROCESS *dbproc, DBINT msgno, int msgstate, int severity,
//             char *msgtext, char *srvname, char *procname, int line
// 这些信息，直接存放进一个结构体，然后抛出该结构体类型的异常。
// 然后，由上层，决定如何分析这些信息，而不是一股脑地输出。
int
msg_handler(DBPROCESS *dbproc, DBINT msgno, int msgstate, int severity,
            char *msgtext, char *srvname, char *procname, int line)
{
    // Some messages don't convey much, as though the server gets lonely
    // sometimes. You're not obliged to print every one.
    enum {changed_database = 5701, changed_language = 5703 };

    if (msgno == changed_database || msgno == changed_language)
        return 0;

    if (msgno > 0) {
        std::ostringstream o;
        o << "Msg " << (long)msgno
          << ", Level " << severity
          << ", State " << msgstate;

        // 数据服务器名
        if (strlen(srvname) > 0)
            o << ", Server '" << srvname << "'";
        // 存储过程名
        if (strlen(procname) > 0)
            o << ", Procedure '" << procname << "'";
        // 行号
        if (line > 0)
            o << ", Line " << line << "";

        if (severity > 10)
        {
            SSS_LOG_ERROR("%s\n", o.str().c_str());
        }
        else
        {
            SSS_LOG_INFO("%s\n", o.str().c_str());
        }
    }
    if (severity > 10)
    {
        SSS_LOG_ERROR("\t%s\n", msgtext);
    }
    else
    {
        SSS_LOG_INFO("\t%s\n", msgtext);
    }

    // Severities are defined in the server documentation, and can be set by
    // the T-SQL RAISERROR statement.
    if (severity > 10) {
        SSS_LOG_ERROR("while executing \"%s\"; "
                      "error: severity %d > 10; by %s\n",
                      dbgetchar(dbproc, 0),
                      severity,
                      sss::path::getbin().c_str());
        // NOTE 执行错误，不应该强退！
        // 当下一句exit(severity);没有被注释掉的时候，根本 dbsqlexec()函数都不会返回！
        // 而是程序直接退出。
        // 所以，本回调函数，和原有进程，根本不是异步执行的关系；而是顺序执行，
        // 只不过提供了一个错误消息的额外接口而已！
        // 另外，错误级别 severity 是一个很重要的参数；该值决定了当前执行的程序
        // ，是否成功执行、数据库状态是否OK！
        // 最好能将这个值，与DBPROCESS*捆绑在一起！
        // exit(severity);
    }

    // Message handlers always and only ever return zero.
    return 0;
}

} // namespace tdspp2
} // namespace sss
