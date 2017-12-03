#ifndef  __DBLINK_HPP_1365611540__
#define  __DBLINK_HPP_1365611540__

#include <sybdb.h>

#include <string>

namespace sss {
namespace tdspp2 {

class Login;    // 为什么class 的前置声明，和friend class不能合并在一起呢？
                // 如果注释掉此句，后面使用Login的地方，会提示未定义。

// 本质是对DBPROCESS * 的封装；但是DBPROCESS这样的名字太长，所以改成DBLink这样
// 稍微短一点的名字。
class DBLink
{
    friend class Query; // 执行sql ，访问内部的 dbprocess
    friend class Login; // 构造时候，让Login对象，能够回访自己内部的 dbprocess

public:
    DBLink();

    explicit DBLink(Login& login);

    // 允许拷贝构造
    DBLink(const DBLink& ref);
    ~DBLink();

public:
    DBLink& operator = (const DBLink&);

    Login& get_login();

    /** use certain database by name. */
    void use_db(const std::string& dbname);

    void connect(Login& login);

    bool good() const;

protected:
    DBPROCESS * get_dbprocess();

private:
    void pre_link_settings();

private:
    DBPROCESS * dbprocess;
    Login * pLogin;     // 方便回溯基本链接信息

    int verbose;
};

} // namespace tdspp2
} // namespace sss


#endif  /* __DBLINK_HPP_1365611540__ */
