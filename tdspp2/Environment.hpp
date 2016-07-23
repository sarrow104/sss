#ifndef  __ENVIRONMENT_HPP_1365774226__
#define  __ENVIRONMENT_HPP_1365774226__

#include "handlers.hpp"
#include "tdspp.hpp"

#include <sss/log.hpp>

#include <sybdb.h>
#include <map>

class Query;
class DBLink;

namespace sss {
namespace tdspp2 {

// 数据库环境
class Environment
{
private:
    Environment()
    {
        // Install the error- and mesage-handlers right away. They're explained in more detail later.
        dberrhandle(sss::tdspp2::err_handler);
        dbmsghandle(sss::tdspp2::msg_handler);

        // FIXME
        // 静态成员变量的增加和减少，如果内部还抛出了异常；那么本应该成对出现
        // 的dbinit 和 dbexit 就会出错。
        //
        // 这里，应该用"类"包裹起来。
        SSS_LOG_DEBUG("dbinit();\n");
        if (dbinit() != SUCCEED)
        {
            throw Exception("Login: dbinit() failed.");
        }

    }
    ~Environment()
    {
        SSS_LOG_DEBUG("dbexit();\n");
        dbexit();
    }

public:
    bool insert_query(DBPROCESS *, Query*);
    bool remove_query(DBPROCESS *);

    bool insert_dblink(DBPROCESS *, DBLink*);
    bool remove_dblink(DBPROCESS *);

    // 通过 DBPROCESS 指针，得到 Query 对象引用。
    Query& refer_query(DBPROCESS*);

    // 通过 DBPROCESS 指针，得到 DBLink 对象引用。
    DBLink& refer_dblink(DBPROCESS*);

public:
    static Environment& ref_env()
    {
        static Environment env;
        return env;
    }

    static void make()
    {
        Environment& e = Environment::ref_env();
        (void)e;
    }

    std::map<DBPROCESS *, Query*>  query_table;
    std::map<DBPROCESS *, DBLink*> dblink_table;
};


} //namespace tdspp2
} // namespace sss


#endif  /* __ENVIRONMENT_HPP_1365774226__ */
