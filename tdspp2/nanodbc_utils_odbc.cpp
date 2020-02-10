#include <vector>
#include <string>
#include <cstring>
#include <mutex>

// 因为odbcinst.h，nanodbc/nanodbc.hpp 与  sql.h sqlext.h 两组头文件之间有冲突
// 所以，相关的实现文件，被我分开了。
#ifdef __MINGW32__
#include <windows.h>
#endif
#include <odbcinst.h>

namespace odbc {
namespace util {

const std::vector<std::string>& get_driver_list()
{
    static std::vector<std::string> drivers;
    static std::once_flag oc;//用于call_once的局部静态变量
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
