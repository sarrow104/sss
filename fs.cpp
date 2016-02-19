#include <sss/fs.hpp>

#ifdef __WIN32__
#       include <windows.h>
#       include <winbase.h>
#       include <shlwapi.h>
#else
#       include <sys/types.h>
#       include <sys/stat.h>
#       include <unistd.h>
#       include <utime.h>
#endif

#include <cstring>
#include <fstream>
#include <stdexcept>

#include <sss/path.hpp>

namespace sss { namespace fs {

//inline long long sss::fs::filetime2longlong(const FILETIME& ft)
//{
//    long long i64 = ft.dwHighDateTime;
//    i64 = (i64 << 32) | ft.dwLowDateTime;
//    return i64;
//}

// 2012-06-15
// 程序逻辑：
// 如果制定的path不存在，则创建一个该路径的文件——内容为空；
// 如果已经存在，则将其"最后更改时间"修改为当前时间；
bool touch(const std::string& path)
{
    bool ret = false;
    std::string full = sss::path::full_of_copy(path);
    if (!sss::path::file_exists(full))
    {
        if (sss::path::file_exists(sss::path::dirname(full)) == sss::PATH_TO_DIRECTORY) {
            std::ofstream ofs(full.c_str(), std::ios_base::binary | std::ios_base::out);
            ret = ofs.good();
        }
    }
    else {
#ifdef  __WIN32__
        // NOTE
        // 修改文件夹的最后修改时间，和修改文件的信息，处理方式一样。
        HANDLE hFile = ::CreateFile(path.c_str(),
                                    FILE_WRITE_ATTRIBUTES,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            FILETIME ft;
            SYSTEMTIME st;
            ::GetSystemTime(&st); // gets current time
            ::SystemTimeToFileTime(&st, &ft); // converts to file time format
            ret = ::SetFileTime(hFile, // sets last-write time for file
                                (LPFILETIME)NULL, (LPFILETIME)NULL, &ft);

            // TODO 错误处理
            ::CloseHandle(hFile);
        }
        else {
            // . To get extended error information, call GetLastError.
        }
#else
        ret = (utime(full.c_str(), 0) == 0);
#endif
    }
    return ret;
}

// 2012-06-15
bool is_path_exists(const std::string& path)
{
#ifdef  __WIN32__
    return ::PathFileExists(path.c_str());
#else
    return sss::path::file_exists(path);
#endif
}

// 2012-06-15
long long last_modify_time(const std::string& path)
{
#ifdef __WIN32__
    HANDLE hFile = ::CreateFile(path.c_str(),
                                GENERIC_READ,
                                FILE_SHARE_VALID_FLAGS,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    FILETIME ft;
    ::GetFileTime(hFile, NULL, NULL, &ft);
    ::CloseHandle(hFile);
    return sss::fs::filetime2longlong(ft);
#else
    unsigned long modifyTime = -1;
    struct stat statbuff;
    if(stat(path.c_str(), &statbuff) >= 0){
        modifyTime = statbuff.st_mtime;
    }

    return modifyTime;
#endif
}

// 2012-06-15
long long construct_time(const std::string& path)
{
#ifdef __WIN32__
    HANDLE hFile = ::CreateFile(path.c_str(),
                                GENERIC_READ,
                                FILE_SHARE_VALID_FLAGS,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    FILETIME ft;
    ::GetFileTime(hFile, &ft, NULL, NULL);
    ::CloseHandle(hFile);
    return sss::fs::filetime2longlong(ft);
#else
    return last_modify_time(path);
#endif
}

#ifdef __WIN32__
//! http://blog.csdn.net/xinchimaker/article/details/6250524
BOOL CreateNullFile(DWORD dwHigh, DWORD dwLow, LPCTSTR lpcszFileName)
{
    BOOL bResult = FALSE;
    HANDLE hFile = ::CreateFile(lpcszFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return (BOOL)::GetLastError();
    }

    HANDLE hFileMap = ::CreateFileMapping(hFile,
                                          NULL,
                                          PAGE_READWRITE,
                                          dwHigh,
                                          dwLow,
                                          NULL);
    if (NULL == hFileMap)
    {
        return (BOOL)::GetLastError();
    }

    ::CloseHandle(hFileMap);
    ::CloseHandle(hFile);

    return bResult;
}
#endif

#ifdef __WIN32__
//! http://blog.csdn.net/ouchengguo/article/details/7998384
BOOL CreateFileDemo(TCHAR* pFileName)
{
    HANDLE hFile;
    HANDLE hMapFile;
    hFile = CreateFile(
        pFileName,
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if( hFile == INVALID_HANDLE_VALUE )
    {
        //OutputDebugString(_T("CreateFile fail!/r/n"));
        return FALSE;
    }
    hMapFile = CreateFileMapping(
        hFile,
        NULL,
        PAGE_READWRITE,
        0,
        512*1024*1024, // 1024 MByte
        NULL
        );
    if( hMapFile == NULL )
    {
        CloseHandle( hFile );
        return FALSE;
    }
    CloseHandle( hMapFile );
    CloseHandle( hFile );
    return TRUE;
}
#endif

} // end-of namespace fs
} // end-of namespace sss
