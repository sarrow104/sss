// { o: -lshlwapi -lregex }
#ifndef  __PATH_GLOB_HPP_1395147786__
#define  __PATH_GLOB_HPP_1395147786__

#if (defined UNICODE) || (defined _UNICODE)
#define UNICODE_OUTER
#endif

#ifdef UNICODE
#undef UNICODE
#endif

#ifdef _UNICODE
#undef _UNICODE
#endif

#ifdef __WIN32__
#       include <windows.h>
#else
#       include <sys/types.h>
#       include <dirent.h>
#       include <sys/stat.h>
#       include <unistd.h>
#endif

#include <sss/bit_operation/bit_operation.h>

#include <sss/utlstring.hpp>
#include <sss/path.hpp>

#include <mutex>
#include <string>
#include <cstring>
#include <algorithm>
#include <stdexcept>

namespace sss {
namespace path {

//#ifdef __WIN32__
//    const char sp_char = '\\';
//#else
//    const char sp_char = '/';
//#endif

// 上一篇中介绍了C运行库中的时间处理函数。这一篇介绍Windows SDk中提供的时间函数
// 。两种时间系统之间没有本质区别(事实上CRT时间是用Windows时间实现的，当然这是
// 说的VC实现)，同样提供本地时间和UTC时间之间的转换。不过CRT中的tm时间在SDK中对
// 应为系统时间(SYSTEMTIME)，CRT中的time_t时间在SDK中对应的为文件时间(FILETIME)
// ，那个"特殊时刻"也变成1601年1月1日的子夜。

// NOTE linux 下 获取信息：
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <unistd.h>
//
// int stat(const char *path, struct stat *buf);
// int fstat(int fd, struct stat *buf);
// int lstat(const char *path, struct stat *buf);
//
// sss/path/glob_path 模块，注重枚举；所以，其包裹枚举得到的对象，是用
// file_descriptor 来命名的；
//
// 至于其他的获取信息的操作，还是放在 fs.hpp 模块好。

// 枚举特定目录下的文件；返回结果保存在文件描述符里面；
// 描述符的属性有：
//    - 是否文件
//    - 是否文件夹
//    - 文件名正则表达式
//    - 文件名后缀
//    - 文件权限
//    - 文件所有者
//    - 文件创建时间（linux下不支持）
//    - 文件最后修改时间
//    - 文件大小（对于文件夹来说，也可以指内部节点数）
//
//    - 文件mime类型
//    - CRC2指纹
//
//      NOTE 前面部分的属性，与文件内容无关；后面两个属性，与文件内
//      容有关。
//
//    当然，也可以不提供这些筛选器，而直接让用户通过循环，然后自己进行判断；
//
//! 文件描述符
//
//用来保存内部枚举的位置——即，方便filter进行筛选；
class glob_path;
class glob_path_recursive;

class file_descriptor {
    friend class glob_path;
    friend class glob_path_recursive;
    friend class glob_path_recursive_bf;

protected:
#ifdef __WIN32__
    WIN32_FIND_DATAA   _ffd;
#else
    //struct dirent {
    //    ino_t          d_ino;       /* Inode number */
    //    off_t          d_off;       /* Not an offset; see below */
    //    unsigned short d_reclen;    /* Length of this record */
    //    unsigned char  d_type;      /* Type of file; not supported
    //                                   by all filesystem types */
    //    char           d_name[256]; /* Null-terminated filename */
    //};
    uint8_t            _type;
    std::string        _name;

#endif
    glob_path *        _p_pg;

public:
    file_descriptor()
    {
        this->clear();
    }

    file_descriptor(const file_descriptor& ref)
        :
#ifdef __WIN32__
            _ffd  (ref._ffd),
#else
            _type (ref._type),
            _name (ref._name),
#endif
            _p_pg (ref._p_pg)
    {
    }

    ~file_descriptor()
    {
    }

    file_descriptor& operator = (const file_descriptor& ref)
    {
        if (this != &ref) {
            file_descriptor tmp(ref);
            this->swap(tmp);
        }
        return *this;
    }

public:
    bool clear() {
        memset(this, 0, sizeof(file_descriptor));
        return true;
    }

    std::string get_mime() const;

    std::string get_path() const;

    time_t      get_mtime() const;

    time_t      get_ctime() const;

    int         get_type() const;

    void swap(file_descriptor& ref)
    {
#ifdef __WIN32__
        std::swap(this->_ffd,  ref._ffd);
#else
        std::swap(this->_type, ref._type);
        std::swap(this->_name, ref._name);
#endif
        std::swap(this->_p_pg, ref._p_pg);
    }

    inline const char * get_name() const {
#ifdef __WIN32__
        return _ffd.cFileName;
#else
        return _name.c_str();
#endif
    }

    inline long long get_fsize() const;

public:
    bool is_dir() const;

    bool is_normal_dir() const;

    bool is_normal_file() const;

    bool is_symlink() const;

    bool is_hide() const;

    int attribute() const;

    bool is_ok() const;

protected:
    void bind(glob_path& pg) {
        this->_p_pg = &pg;
    }
};

inline long long file_descriptor::get_fsize() const
{
#ifdef __WIN32__
    LARGE_INTEGER filesize;
    filesize.LowPart =  _ffd.nFileSizeLow;
    filesize.HighPart = _ffd.nFileSizeHigh;

    return filesize.QuadPart;
#else
    unsigned long filesize = -1;
    struct stat statbuff;
    if (stat((this->get_path()).c_str(), &statbuff) < 0) {
        return filesize;
    } else {
        filesize = statbuff.st_size;
    }
    return filesize;
#endif
}


class filter_t;

class glob_path {
public:
    glob_path(const std::string& root,
              file_descriptor& fd,
              filter_t * filter = 0);

    ~glob_path();

public:
    bool fetch();

    bool good() const
    {
        return this->is_ok;
    }

    bool next();

    const std::string& basedir() const
    {
        return _dir;
    }

protected:
    bool is_first() const
    {
        return !this->_is_next;
// #ifdef __WIN32__
//         return this->hFind == INVALID_HANDLE_VALUE;
// #else
//         return this->_pdir == 0;
// #endif
    }

    void mark_not_first()
    {
        this->_is_next = true;
    }

private:
    bool                _is_next;
    bool                is_ok;
    file_descriptor *   _pfd;
    std::string         _dir;
    filter_t *          _filter;
#ifdef __WIN32__
    HANDLE              hFind;
    WIN32_FIND_DATAA    ffd_first;
#else
    DIR *               _pdir;
    std::mutex          _mtx;
#endif
};


} // end of namespace path
} // end of namespace sss


#ifdef UNICODE_OUTER
#define UNICODE
#define _UNICODE
#endif

#endif  /* __PATH_GLOB_HPP_1395147786__ */
