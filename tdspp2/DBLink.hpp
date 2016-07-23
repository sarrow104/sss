#ifndef  __DBLINK_HPP_1365611540__
#define  __DBLINK_HPP_1365611540__

#include <sybdb.h>

#include <string>

namespace sss {
namespace tdspp2 {

class Login;    // Ϊʲôclass ��ǰ����������friend class���ܺϲ���һ���أ�
                // ���ע�͵��˾䣬����ʹ��Login�ĵط�������ʾδ���塣

// �����Ƕ�DBPROCESS * �ķ�װ������DBPROCESS����������̫�������Ըĳ�DBLink����
// ��΢��һ������֡�
class DBLink
{
    friend class Query; // ִ��sql �������ڲ��� dbprocess
    friend class Login; // ����ʱ����Login�����ܹ��ط��Լ��ڲ��� dbprocess

public:
    DBLink();

    explicit DBLink(Login& login);

    // ����������
    DBLink(const DBLink& ref);
    ~DBLink();

public:
    DBLink& operator = (const DBLink&);

    Login& get_login();

    /** use certain database by name. */
    void use_db(const std::string& dbname);

    void connect(Login& login);

protected:
    DBPROCESS * get_dbprocess();

private:
    void pre_link_settings();

private:
    DBPROCESS * dbprocess;
    Login * pLogin;     // ������ݻ���������Ϣ

    int verbose;
};

} // namespace tdspp2
} // namespace sss


#endif  /* __DBLINK_HPP_1365611540__ */
