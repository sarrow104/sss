#include "DBLink.hpp"

#include <sss/log.hpp>

#include "Login.hpp"
#include "query.hpp"

namespace sss {
namespace tdspp2 {

DBLink::DBLink()
    : dbprocess(0), pLogin(0)
{
}

DBLink::DBLink(Login& login)
    : dbprocess(0), pLogin(&login)
{
    this->pLogin->create_link(*this);
    this->pre_link_settings();
}

void DBLink::pre_link_settings()
{
    // 2013-06-25
    // 远程多数据服务器联合
    if (this->dbprocess)
    {
        dbcmd(this->dbprocess, "set ANSI_NULLS ON set ANSI_WARNINGS ON");
        dbsqlexec(this->dbprocess);
        dbcancel(this->dbprocess);

        // 2015-04-04 去除 image/text 字段 4096 限制
        // 不过freetds貌似没有实现这个功能
        // E:\lib\freetds-0.91\src\dblib\dblib.c|4376
        if (FAIL == dbsetopt(this->dbprocess, DBTEXTSIZE, "2147483647", 0)) {
            sss::tdspp2::Query q(this);
            q.sql("SET TEXTSIZE 2147483647");
            q.execute();
        }
        if (FAIL == dbsetopt(this->dbprocess, DBTEXTLIMIT, "0", 0)) {
            //sss::tdspp2::Query q(&dblink);
            //q.sql("SET TEXTLIMIT 0");
            //q.execute();
            //'TEXTLIMIT' is not a recognized SET statement.
        }
    }
}

DBLink::DBLink(const DBLink& ref)
    : dbprocess(0), pLogin(ref.pLogin)
{
    if (this->pLogin)
        this->pLogin->create_link(*this);
}

DBLink::~DBLink()
{
    if (this->pLogin)
        this->pLogin->destroy_link(*this);
}

void DBLink::connect(Login& login)
{
    DBLink tmp_dblink(login);
    std::swap(this->dbprocess, tmp_dblink.dbprocess);
    std::swap(this->pLogin, tmp_dblink.pLogin);
}

DBLink& DBLink::operator = (const DBLink& ref)
{
    if (this->pLogin != ref.pLogin)
    {
        DBLink tmp_dblink(ref);
        std::swap(this->dbprocess, tmp_dblink.dbprocess);
        std::swap(this->pLogin, tmp_dblink.pLogin);
    }
    return *this;
}

Login& DBLink::get_login()
{
    return *pLogin;
}

DBPROCESS * DBLink::get_dbprocess()
{
    return this->dbprocess;
}

void DBLink::use_db(const std::string& dbname)
{
    if (dbname.length())
    {
        SSS_LOG_DEBUG("dbuse(%p, \"%s\");\n",
                      this->dbprocess,
                      dbname.c_str());
        dbuse(this->dbprocess, dbname.c_str());
    }
}


} // namespace tdspp2
} // namespace sss
