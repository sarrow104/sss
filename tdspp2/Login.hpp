// {o: -lsybdb -lws2_32 -liconv}
#ifndef  __LOGIN_HPP_1365697760__
#define  __LOGIN_HPP_1365697760__

#include <sybdb.h>

#include <string>
#include <exception>
#include <map>

namespace sss {
namespace tdspp2 {

class Query;
class DBLink;

/** TDS++ class */
class Login
{
    friend class DBLink;
    friend class Query;

public:
    /** Constructor */
    Login();
    /** Destructor */
    ~Login();

public:
    // 设置登录信息
    void set_properties(const std::string& server,
                        const std::string& username,
                        const std::string& password,
                        const std::string& dbname = "",
                        const std::string& charset = "GBK");

    void set_server(const std::string& server);
    void set_ussername(const std::string& username);
    void set_password(const std::string& password);
    void set_dbname(const std::string& dbname);
    void set_charset(const std::string& charset);
    void set_logintime(int seconds);
    //! https://github.com/rails-sqlserver/tiny_tds/issues/2
    // freetds 不支持直接使用port number！
    // 原因，见上面的链接。
    // 变通的办法，是使用 .conf文件。
    // void set_portnumber(int port);

    std::string get_server();
    std::string get_ussername();
    std::string get_password();
    std::string get_dbname();
    std::string get_charset();
    // int get_portnumber();

private:
    // 仅供DBLink 的构造函数调用
    void create_link(DBLink & dblink);

    // 仅供DBLink 的析构函数调用
    void destroy_link(DBLink & dblink);

private:
    LOGINREC *  loginrec;
    std::string server,
                username,
                password,
                dbname,
                appname,
                charset;
    // int         port;
    int         logintime;

private:
    typedef std::map<DBPROCESS*, DBLink*> dblink_manager_t;
    dblink_manager_t dblink_manager;
};

} // namespace tdspp2
} // namespace sss


#endif  /* __LOGIN_HPP_1365697760__ */

