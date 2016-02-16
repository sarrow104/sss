// {ln: -lshlwapi}
#ifndef  __FS_HPP_1339764273__
#define  __FS_HPP_1339764273__

//! linux��C��ȡ�ļ���Ϣ
//! http://blog.csdn.net/gdujian0119/article/details/6363574
//
// �ļ��޸�ʱ��ʹ���ʱ��Ļ�ȡ����ȡ���ڲ���ϵͳ�ġ���׼C��û���ṩ�����ĺ���
// ��
//
// ��׼C��ȡϵͳʱ�䣬ʹ��std::time(std::time_t *)��������ע�⣬time_tֻ��һ
// ��long��typdef���ѣ�������׼C����struct tm������date��
//
// ����win32��˵���ֱַ�������struct SYSTEMTIME��FILETIME ����ʾϵͳʱ����ļ�ʱ�䡣
// SYSTEMTIME ��ʵ��һ������std::tm�Ľṹ�壬�ֱ𱣴��������գ�ʱ�������Ϣ��
//
// ���ԣ��������ṹ�壬����һ��ȱ�㣺
//
// ��Ӧ��ֱ�����Ӽ��������ʱ�䣨���ڣ������
//
// ��Ӧ�ã��ֱ�ת���ɸ��Ե�������ʾ��ʽ����FILETIME����time_t����NOTE FILETIME
// �����Ͽ��Կ�����һ��64λ�� long long ���ͣ�
//
// ��FILETIME��time_t��Ϊ�м������������ṩ��׼C��win32ʱ��ĸ�ʽת���ˡ�
//
// �ڴˣ���һ������������
//
// �� UNIX ƽ̨����Ϊ time_t����ʾ����ҹ UTC ��Э��ͨ��ʱ�䣩 1970 �� 1 �� 1 ��
// ������������ ANSI C ����ʱ�������͵Ĵ�����ά���ļ���ʱ�䡣
//
// �� Win32 ƽ̨����Ҫ�� 64 λ FILETIME �ṹ������ʾ�� UTC 1601 �� 1 �� 1 ����
// ���� 100 ����Ϊ����� ������ͨ��ʱ�䣩 �Ĵ�����ά���ļ���ʱ�䡣
//
// NOTE �ο�:
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
// ��װ
// std::map<std::string, NODE> �������ļ��ṹ��Ȼ����Ը�����Ҫ���л��ָ�����('/'����"\\")��
// �ο�
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

// SYSTEMTIME ֮���ʱ�������׼��㣻���Խ�SYSTEMTIME ת��Ϊlonglong��FILETIME��Ȼ����ת��ΪΪ��
inline long long systemtime_diff_in_milliseconds(const SYSTEMTIME &t1, const SYSTEMTIME &t2)
{
    FILETIME f1 = sss::fs::systemtime2filetime(t1);
    FILETIME f2 = sss::fs::systemtime2filetime(t2);

    // NOTE��FILETIME �ĵ�λ�� 100 ����; ��ô 100 ���� �� ��ı�����7��0����������4��0��
    // 10^10 ���� = 1 ��
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

//һ�� UNIX ʱ��ת��Ϊ FILETIME �ṹ������ Win32 ʱ���ʽ�������ɵػ��ͨ��ʹ��
//Win32 ���� ���� FileTimeToSystemTime() �� FileTimeToDosDateTime()��
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
// �޸��Ѿ����ڵ�Ŀ�꣨�ļ����ļ��У����޸�ʱ�䣬Ϊ��ǰϵͳʱ�䣻
// ���ߴ���һ���յ��ļ����ļ��С�����ǰ���½����ļ������ļ��У����޸�ʱ�䣬���ǵ�ǰϵͳʱ��
// ��������Ҫ�޸ĵ�Ŀ�����ļ��У�����·�����ɵ�ǰ�ļ�ϵͳ���Լ�����ĩβ�Ƿ����"\\"����'/'����
bool touch(const std::string& path);

// 2012-06-15
// �ļ���·���Ƿ����
bool is_path_exists(const std::string& path);

inline int  get_file_size(const std::string& path)
{
#ifdef __WIN32__
    HANDLE hFile = ::CreateFile(path.c_str(),
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
// ��ȡ�ļ�������޸�ʱ��
long long last_modify_time(const std::string& path);

// 2012-06-15
// ��ȡ�ļ������޸�ʱ��
long long construct_time(const std::string& path);

} // end-of namespace fs
} // end-of namespace sss



#endif  /* __FS_HPP_1339764273__ */
