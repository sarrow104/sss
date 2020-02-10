#include <vector>
#include <string>
#include <cstring>
#include <mutex>

// ��Ϊodbcinst.h��nanodbc/nanodbc.hpp ��  sql.h sqlext.h ����ͷ�ļ�֮���г�ͻ
// ���ԣ���ص�ʵ���ļ������ҷֿ��ˡ�
#ifdef __MINGW32__
#include <windows.h>
#endif
#include <odbcinst.h>

namespace odbc {
namespace util {

const std::vector<std::string>& get_driver_list()
{
    static std::vector<std::string> drivers;
    static std::once_flag oc;//����call_once�ľֲ���̬����
    std::call_once(oc, [&drivers] {
        TCHAR szDrivers[4096];
        std::memset(szDrivers, 0, sizeof(szDrivers));
        WORD wRet = 0;
        if(::SQLGetInstalledDrivers(szDrivers, sizeof(szDrivers), &wRet))
        {
            LPTSTR pszDrv = szDrivers;
            while(*pszDrv)
            {
                auto len = std::strlen(pszDrv);
                drivers.emplace_back(std::string(pszDrv, len));
                pszDrv += len + 1;
            }
        }
    });
    return drivers;
}

} // namespace util
} // namespace odbc
