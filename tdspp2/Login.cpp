#include "tdspp.hpp"
#include "handlers.hpp"
#include "Environment.hpp"

#include <sss/log.hpp>
#include <sss/path.hpp>

#include <iostream>

namespace sss {
namespace tdspp2 {

// NOTE 不在构造函数中设置登录属性的目的：
// 允许使用 std::vector<Login>容器！
Login::Login()
    :loginrec(0)
{
    // 数据库环境初始化
    Environment::make();

    // 获取 LOGINREC 资源句柄
    this->loginrec = dblogin();
    SSS_LOG_DEBUG("%p = dblogin();\n", this->loginrec);
    if (!this->loginrec)
    {
        throw Exception("Login: Init failed");
    }

    this->appname = sss::path::no_suffix(sss::path::basename(sss::path::getbin()));
}

Login::~Login()
{
    for (dblink_manager_t::iterator it = this->dblink_manager.begin();
         it != this->dblink_manager.end();
         ++it)
    {
        if (it->first)
        {
            SSS_LOG_DEBUG("dbclose(%p);\n", it->first);
            dbclose(it->first);
        }
    }
    if (this->loginrec)
    {
        SSS_LOG_DEBUG("dbloginfree(%p);\n", this->loginrec);
        dbloginfree(this->loginrec);
        this->loginrec = 0;
    }
}

void Login::set_properties(const std::string& server,
                           const std::string& username,
                           const std::string& password,
                           const std::string& dbname,
                           const std::string& charset)
{
    this->set_server(server);
    this->set_ussername(username);
    this->set_password(password);
    this->set_dbname(dbname);
    this->set_charset(charset);
    // NOTE APPname 不能包含特殊字符！比如斜杠
    DBSETLAPP(this->loginrec, this->appname.c_str());
}

// NOTE server 的形式为ip:port
// mssql 的 port 为1433
// 如果省略端口号，则默认为1433
void Login::set_server(const std::string& server)
{
    this->server = server;
    {
        // 2013-04-08
        std::string domain_name;
        domain_name.resize(server.length());

        int len = -1, port = -1;
        int status = sscanf(server.c_str(), "%[^:]:%d%n", &domain_name[0], &port, &len);

        // NOTE 只有当 status == 2，len == server.length() （能分解成域名和端口
        // 两部分，并且消耗完所有字符）的时候，才说明是符合函数接口的参数。
        switch (status)
        {
        case 2: // 正确读取两个数据，但是还有剩余字符
            if (len != int(server.length()))
            {
                // 说明端口输入是错误的
                throw Exception("The port part of '" + server + "' is invalid.");
            }
            break;

        case 1:
            {
                // 用户没有提供端口信息，添加上默认值
                if (domain_name == server)
                {
                    // 说明用户只输入了域名信息——域名是否合法，就不验证了。
                    this->server += ":1433";
                }
                else    // 用户提供了端口信息，但是不正确
                {
                    throw Exception("The server '" + server + "' is invalid.");
                }
            }
            break;

        case 0: // 解析失败——一般是空串
        default:        // sscanf 返回了其他值
            throw Exception("The server '" + server + "' is empty or invalid.");
            break;
        }
    }
}

void Login::set_ussername(const std::string& username)
{
    this->username = username;
    // 用户名
    DBSETLUSER(this->loginrec, this->username.c_str());
}

void Login::set_password(const std::string& password)
{
    this->password = password;
    // 密码
    DBSETLPWD(this->loginrec, this->password.c_str());
}

void Login::set_dbname(const std::string& dbname)
{
    this->dbname = dbname;
}

void Login::set_charset(const std::string& charset)
{
    this->charset = charset;
    // 设置字符集
    DBSETLCHARSET(this->loginrec, this->charset.c_str());
}

std::string Login::get_server()
{
    return this->server;
}

std::string Login::get_ussername()
{
    return this->username;
}

std::string Login::get_password()
{
    return this->password;
}

std::string Login::get_dbname()
{
    return this->dbname;
}

std::string Login::get_charset()
{
    return this->charset;
}

// 创建一个新的数据库进程连接
void Login::create_link(DBLink & dblink)
{
    //连接数据库服务器
    dblink.dbprocess = dbopen(this->loginrec, this->server.c_str());
    SSS_LOG_DEBUG("%p = dbopen(%p, \"%s\");\n",
                  dblink.dbprocess,
                  this->loginrec,
                  this->server.c_str());

    dblink.use_db(this->dbname);

    dblink_manager_t::iterator it = this->dblink_manager.find(dblink.dbprocess);
    if (it == this->dblink_manager.end())
    {
        this->dblink_manager.insert(it, std::make_pair(dblink.dbprocess, &dblink));
    }
}

// 销毁制定的数据库进程链接
void Login::destroy_link(DBLink & dblink)
{
    dblink_manager_t::iterator it = this->dblink_manager.find(dblink.dbprocess);
    if (it != this->dblink_manager.end())
    {
        this->dblink_manager.erase(it);
    }

    if (dblink.dbprocess)
    {
        SSS_LOG_DEBUG("dbclose(%p);\n", dblink.dbprocess);
        dbclose(dblink.dbprocess);
        dblink.dbprocess = 0;
        dblink.pLogin = 0;
    }
}

// NOTE
// 在执行一个语句的时候，返回的结果集可能有0个，也可能有一个甚至多个。
// 所以，很多sql库，都将结果集，用 result set 来表示：即，结果的集合。
//
// 另外，单个的"结果"里面，除了检索出来的"表"，还可能附带有 COMPUTE 集合。它的
// 数量，同样不定。就是说，对于一个待执行的sql语句来说，其返回的结果，是一个森
// 林结构。每棵树的深度最多为2 —— 如果把 某一单独的 select 检索结果，看作是一
// 个行列式的整体的话。
//
// out:
//    |-result-set 1/
//    |    |- COMPUTE id 1
//    |    |- COMPUTE id 2
//    |    |- ...
//    |    `- COMPUTE id n
//    |-result-set 2
//    |-...
//    `-result-set 3
//
// 当然，如果是执行的存储过程，那么还有返回值的问题。

} //namespace tdspp2

} // namespace sss
