// {ln: -lshlwapi}
#ifndef  __FS_HPP_1339764273__
#define  __FS_HPP_1339764273__

//! linux下C获取文件信息
//! http://blog.csdn.net/gdujian0119/article/details/6363574
//
// 文件修改时间和创建时间的获取，是取决于操作系统的。标准C是没有提供这样的函数
// 。
//
// 标准C获取系统时间，使用std::time(std::time_t *)函数――注意，time_t只是一
// 个long的typdef而已！；而标准C又用struct tm来保存date！
//
// 对于win32来说，又分别用两个struct SYSTEMTIME和FILETIME 来表示系统时间和文件时间。
// SYSTEMTIME 其实是一个类似std::tm的结构体，分别保存了年月日，时分秒等信息。
//
// 所以，这两个结构体，都有一个缺点：
//
// 不应该直接做加减，以求得时间（日期）间隔。
//
// 而应该，分别转化成各自的整数表示形式――FILETIME或者time_t；（NOTE FILETIME
// 本质上可以看作是一个64位的 long long 类型）
//
// 以FILETIME和time_t作为中间桥梁，就能提供标准C和win32时间的格式转换了。
//
// 在此，有一个必须条件：
//
// 在 UNIX 平台下名为 time_t，表示自午夜 UTC （协调通用时间） 1970 年 1 月 1 日
// 以来的秒数的 ANSI C 运行时算术类型的窗体中维护文件的时间。
//
// 在 Win32 平台下主要在 64 位 FILETIME 结构，它表示的 UTC 1601 年 1 月 1 日以
// 来的 100 纳秒为间隔数 （坐标通用时间） 的窗体中维护文件的时间。
//
// NOTE 参考:
// http://support.microsoft.com/kb/167296/zh-cn
#ifdef __WIN32__
#       include <windows.h>
#       include <winbase.h>
#       include <winnt.h>
#else
#       include <sys/types.h>
#       include <sys/stat.h>
#       include <unistd.h>

#       include <stdint.h>

typedef int32_t DWORD;
typedef int16_t WORD;
typedef int32_t BOOL;

struct FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

struct SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};

inline void SystemTimeToFileTime(const SYSTEMTIME *, FILETIME *) {}
inline void FileTimeToSystemTime(const FILETIME *, SYSTEMTIME *) {}

inline int64_t Int32x32To64(int32_t a, int32_t b) { return int64_t(a) * int64_t(b); }

inline void GetSystemTime(SYSTEMTIME *) {}

#endif

#include <time.h>

#include <string>

#include <stdio.h>

#include <sss/time.hpp>

namespace sss { namespace fs {


// TODO
// 封装
// std::map<std::string, NODE> 创建，文件结构。然后可以根据需要，切换分隔符：('/'或者"\\")。
// 参考
// e:\project\myproject\sss_root\include\sss\filesystem\filesystem.h|9

inline FILETIME longlong2filetime(long long t)
{
    FILETIME ft;
    ft.dwLowDateTime = (DWORD)t;
    ft.dwHighDateTime = t >> 32;
    return ft;
}

inline long long filetime2longlong(const FILETIME& ft)
{
    return (((long long)ft.dwHighDateTime << 32LL) & 0xFFFFFFFF00000000LL) | (ft.dwLowDateTime & 0x00FFFFFFFFLL);
}

inline time_t filetime2time_t(const FILETIME& ft)
{
    return time_t((filetime2longlong(ft) - 116444736000000000LL) / 10000000LL);
}

inline time_t longlong2time_t(long long ll)
{
    return filetime2time_t(longlong2filetime(ll));
}

inline FILETIME systemtime2filetime(const SYSTEMTIME& st)
{
    FILETIME ft;
    ::SystemTimeToFileTime(&st, &ft);

    return ft;
}

inline SYSTEMTIME filetime2systemtime(const FILETIME& ft)
{
    SYSTEMTIME st;
    ::FileTimeToSystemTime(&ft, &st);

    return st;
}

// SYSTEMTIME 之间的时间差，不容易计算；可以将SYSTEMTIME 转化为longlong的FILETIME；然后，再转化为为秒差；
inline long long systemtime_diff_in_milliseconds(const SYSTEMTIME &t1, const SYSTEMTIME &t2)
{
    FILETIME f1 = sss::fs::systemtime2filetime(t1);
    FILETIME f2 = sss::fs::systemtime2filetime(t2);

    // NOTE，FILETIME 的单位是 100 纳秒; 那么 100 纳秒 到 秒的比率是7个0！到毫秒是4个0；
    // 10^10 纳秒 = 1 秒
    return (sss::fs::filetime2longlong(f2) - sss::fs::filetime2longlong(f1)) / 10000;
}

inline FILETIME time_t2filetime(time_t t)
{
    // Note that LONGLONG is a 64-bit value
    return longlong2filetime(Int32x32To64(t, 10000000) + 116444736000000000LL);
}

inline long long time_t2longlong(time_t t)
{
    return filetime2longlong(time_t2filetime(t));
}

//一旦 UNIX 时间转换为 FILETIME 结构，其他 Win32 时间格式可以轻松地获得通过使用
//Win32 函数 （如 FileTimeToSystemTime() 和 FileTimeToDosDateTime()。
inline SYSTEMTIME time_t2systemtime(time_t t)
{
    return filetime2systemtime(time_t2filetime(t));
}

inline time_t systemtime2time_t(const SYSTEMTIME& st)
{
    return filetime2time_t(systemtime2filetime(st));
}

inline SYSTEMTIME get_systemtime()
{
    SYSTEMTIME st;
    ::GetSystemTime(&st);
    return st;
}

inline std::string systemtime_format(const std::string& fmt, const SYSTEMTIME& st)
{
    sss::time::Date day(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return day.format(fmt);
}

// 2012-06-15
// 修改已经存在的目标（文件或文件夹）的修改时间，为当前系统时间；
// 或者创建一个空的文件或文件夹――当前，新建的文件（或文件夹）的修改时间，就是当前系统时间
// 决定具体要修改的目标是文件夹，还是路径，由当前文件系统，以及参数末尾是否带有"\\"或者'/'决定
bool touch(const std::string& path);

// 2012-06-15
// 文件、路径是否存在
bool is_path_exists(const std::string& path);

inline int  get_file_size(const std::string& path)
{
#ifdef __WIN32__
    HANDLE hFile = ::CreateFileA(path.c_str(),
                                 GENERIC_READ,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);

    DWORD file_size = ::GetFileSize(hFile,NULL);

    CloseHandle(hFile);

    return file_size;
#else
    unsigned long filesize = -1;
    struct stat statbuff;
    if(stat(path.c_str(), &statbuff) < 0){
        return filesize;
    }else{
        filesize = statbuff.st_size;
    }
    return filesize;
#endif
}

// 2012-06-15
// 获取文件的最后修改时间
long long last_modify_time(const std::string& path);

// 2012-06-15
// 获取文件创建修改时间
long long construct_time(const std::string& path);

} // end-of namespace fs
} // end-of namespace sss



#endif  /* __FS_HPP_1339764273__ */
