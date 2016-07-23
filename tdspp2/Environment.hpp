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

// ���ݿ⻷��
class Environment
{
private:
    Environment()
    {
        // Install the error- and mesage-handlers right away. They're explained in more detail later.
        dberrhandle(sss::tdspp2::err_handler);
        dbmsghandle(sss::tdspp2::msg_handler);

        // FIXME
        // ��̬��Ա���������Ӻͼ��٣�����ڲ����׳����쳣����ô��Ӧ�óɶԳ���
        // ��dbinit �� dbexit �ͻ����
        //
        // ���Ӧ����"��"����������
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

    // ͨ�� DBPROCESS ָ�룬�õ� Query �������á�
    Query& refer_query(DBPROCESS*);

    // ͨ�� DBPROCESS ָ�룬�õ� DBLink �������á�
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
