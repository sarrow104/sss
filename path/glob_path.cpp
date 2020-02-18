#include "glob_path.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <mutex>

#include "filter_t.hpp"

#include <sss/Terminal.hpp>

#include <sss/log.hpp>
#include <sss/path.hpp>
#include <sss/utlstring.hpp>
#include <sss/log.hpp>

#ifdef __WIN32__
#include <sss/os/win32/timeconvert.hpp>
#endif

// NOTE win32api FindFirstFile 必须要提供匹配串；并且，不支持多后缀选择
// 因此，不管是 win32 还是 linux，都默认使用 regex 库，用来……
// NOTE linux下，有 glob()@glob.h 枚举函数，也支持类windows下的文件搜索；

namespace sss {
namespace path {

std::string file_descriptor::get_mime() const
{
    if (this->is_ok()) {
        return sss::path::mime_type(this->get_path());
    }
    else {
        return "";
    }
}

std::string file_descriptor::get_path() const
{
    //        SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
    assert(this->_p_pg);
    //std::cout << __func__ << ": " << this->_p_pg->basedir() << " | " << this->get_name() << std::endl;
    return sss::path::append_copy(this->_p_pg->basedir(), this->get_name());
}

time_t      file_descriptor::get_mtime() const
{
#ifdef __WIN32__
    HANDLE hFile = ::CreateFile(this->get_path().c_str(),
                                GENERIC_READ,
                                FILE_SHARE_VALID_FLAGS,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    FILETIME ft;
    ::GetFileTime(hFile, &ft, NULL, NULL);
    ::CloseHandle(hFile);

    time_t ret;
    sss::win32::FileTimeToUnixTime(&ft, &ret);
    return ret;
#else
    struct stat info;
    if (stat(this->get_path().c_str(), &info) == 0) {
        return info.st_mtime;
    }
    return 0;
#endif
}

time_t      file_descriptor::get_ctime() const
{
#ifdef __WIN32__
    HANDLE hFile = ::CreateFile(this->get_path().c_str(),
                                GENERIC_READ,
                                FILE_SHARE_VALID_FLAGS,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    FILETIME ft;
    ::GetFileTime(hFile, &ft, NULL, NULL);
    ::CloseHandle(hFile);

    time_t ret;
    sss::win32::FileTimeToUnixTime(&ft, &ret);
    return ret;
#else
    struct stat info;
    if (stat(this->get_path().c_str(), &info) == 0) {
        return info.st_mtime;
    }
    return 0;
#endif
}

int  file_descriptor::get_type() const
{
#ifdef __WIN32__
    return _ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
#else
    struct stat info;
    if (stat(this->get_path().c_str(), &info) == 0) {
        return info.st_mode;
    }
    return 0;
#endif
}

bool file_descriptor::is_dir() const {
#ifdef __WIN32__
    return _ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
#else
    // mingw 的 <dirent.h> 结构体，没有d_type ！
    // 需要额外调用lstat函数，才能获取是否文件夹信息。
    return _type == DT_DIR
        || (this->is_symlink() &&
            sss::PATH_TO_DIRECTORY == sss::path::file_exists(this->get_path()));
#endif
}

bool file_descriptor::is_normal_dir() const {
    return
        this->is_dir() &&
        (std::strcmp(this->get_name(), "..") != 0 && std::strcmp(this->get_name(), ".") != 0);
}

bool file_descriptor::is_normal_file() const
{
#ifdef __WIN32__
    return !this->is_dir();
#else
    // DT_LNK
    // 关于区分硬链接和软链接
    // hard link vs symlink(soft link)
    // stat 只能区分 softlink与普通文件；
    // 对于linux文件系统来说，硬链接就等于文件；
    // 区别在于硬链接的文件，可能一个文件块，对应着多个名字；
    // 而一般的文件，一个文件块，就对应着一个名字；
    // 要区分的话，需要用lstat函数。
    // 所谓符号链接文件，可以认为，它所refer的文件的路径名，就是这个链接文
    // 件的文件内容；
    // lstat获取到的struct stat.st_size，就相当于路径名称长度……
    // 至于stat函数遇到符号链接的时候，相当于直接索引到源文件了；
    // 那么，如何读取这个符号链接文件中的内容呢？
    // C语言的函数，显然是没法的；因为符号链接文件对于它们来说是透明的；
    // 需要 readlink@<unistd.h>
    return _type == DT_REG
        || (this->is_symlink() &&
            sss::PATH_TO_FILE == sss::path::file_exists(this->get_path()));
#endif
}

bool file_descriptor::is_symlink() const {
#ifdef __WIN32__
    // http://www.howtogeek.com/howto/16226/complete-guide-to-symbolic-links-symlinks-on-windows-or-linux/
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa363866(v=vs.85).aspx
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa363878(v=vs.85).aspx
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa365006(v=vs.85).aspx
    //
    // windows 下，自从2000+ntfs文件系统后，就支持hardlink和junction；
    // 前者类似linux的硬链接；
    // 后者部分等于linux的软件链接——它只支持文件夹，多名字；
    //
    // 而完全等效于 软连接的 函数 CreateSymbolicLink，则要到windows vista【桌面版】，之后才支持！
    // TODO FIXME
    return false;
#else
    return _type == DT_LNK;
#endif
}

bool file_descriptor::is_hide() const {
#ifdef __WIN32__
    return _ffd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN;
#else
    return _name.size() >= 2 && _name[0] == '.';
#endif
}

int file_descriptor::attribute() const {
#ifdef __WIN32__
    return _ffd.dwFileAttributes;
#else
#if 0
    // FIXME 在linux下，要获取，与windows下类似的文件属性，必须额外调用 stat 系列函数
    struct stat buf;
    stat(file, buf);//file 为你需要读的文件 int型的参数
    // if(S_ISREG(buf.st_mode))
    // printf("普通文件");
    //
    // S_ISREG() 为类型宏 普通文件
    // S_ISDIR() 目录文件
    // S_ISCHR() 字符特殊文件
    // S_ISBLK() 块特殊文件
    // S_ISFIFO() 管道或FIFO
    // S_ISLNK() 符号连接
    // S_ISSOCK() 套接字
#endif
    return _type;
#endif
}

bool file_descriptor::is_ok() const {
#ifdef __WIN32__
    return reinterpret_cast<const uint32_t&>(this->_ffd);
    //return _ffd.cFileName; // 之前，已经将 结构体 memeset 为0 了；于是……
#else
    return _type != DT_UNKNOWN;
#endif
}

glob_path::glob_path(const std::string& root,
                     file_descriptor& fd,
                     filter_t * filter) :
    _is_next(false),
    is_ok(true),
    _pfd(&fd),
    _dir(root),
    _filter(filter),
#ifdef __WIN32__
    hFind(INVALID_HANDLE_VALUE)
#else
_pdir(0)
#endif
{
    // std::cout << __func__ << "(" << root << ")" << std::endl;
    try {
        if (!sss::path::is_directory(_dir)) {
            is_ok = false;
            throw std::runtime_error("invalid path `" + root + "`");
        }
#ifdef __WIN32__
        // 2015-12-09
        hFind = ::FindFirstFile((_dir + "/*").c_str(), &ffd_first);
        this->is_ok = (hFind != INVALID_HANDLE_VALUE);
#else
        // 2015-12-09
        this->_pdir = ::opendir(_dir.c_str());
        this->is_ok = this->_pdir;
        if (!this->_pdir) {
            ::fprintf(stderr, "opendir(\"%s\") : %s%s%s\n",
                      _dir.c_str(),
                      sss::Terminal::error.data(),
                      ::strerror(errno),
                      sss::Terminal::end.data());
        }
#endif
        if (this->good() && this->_pfd) {
            _pfd->bind(*this);
        }
    }
    catch(...) {
        throw;
    }
}

glob_path::~glob_path()
{
#ifdef __WIN32__
    if (INVALID_HANDLE_VALUE != hFind) {
        ::FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
#else
    if (_pdir) {
        (void)::closedir(_pdir);
        _pdir = 0;
    }
#endif
}

bool glob_path::fetch()
{
    if (!this->good()) {
        return false;
    }

    bool is_valid = false;
#ifdef __WIN32__
    if (this->is_first()) {
        _pfd->_ffd = this->ffd_first;

        is_valid = true;
        this->mark_not_first();
    }
    else {
        is_valid = this->next();
    }
#else
    is_valid = this->next();
    this->mark_not_first();
#endif
    if (is_valid && _filter) {
        do {
            if (_filter->is_match(*_pfd)) {
                break;
            }
        } while ( (is_valid = this->next()) );
    }
    return is_valid;
}

bool glob_path::next()
{
#ifdef __WIN32__
    // 为了包装 win32 下，next()动作失败后，行为与 linux 下类似（dirent * == 0）
    // 特意进行memset()动作
    // 当然，还有一个办法，就是让 linux 下，file_descriptor 的行为变得与
    // windows 的 FindFirstFile, FindNextFile类似；
    //
    // file_descriptor::findFist(glob_path&)
    // file_descriptor::findNext(glob_path&)
    return (this->is_ok && ::FindNextFile(hFind, &_pfd->_ffd)) || (_pfd->clear(), false);
#else
    const char * c_fmt = 0;
    if (this->good()) {
        errno = 0;
        c_fmt = "readdir";
        {
            std::lock_guard<std::mutex> lck (_mtx);
            dirent *p_ent = ::readdir(_pdir);
            if (p_ent)
            {
                _pfd->_type = p_ent->d_type;
                _pfd->_name = p_ent->d_name;
            }
            else
            {
                _pfd->_type = DT_UNKNOWN;
                _pfd->_name.clear();
            }
        }

        if (!_pfd->is_ok() && errno) {
            ::fprintf(stderr, "%s(\"%s\") : %s%s%s\n",
                      c_fmt,
                      _dir.c_str(),
                      sss::Terminal::error.data(),
                      ::strerror(errno),
                      sss::Terminal::end.data());
            (void)::closedir(this->_pdir);
            this->_pdir = 0;
            this->is_ok = false;
        }
        return _pfd->is_ok();
    }
    return false;
#endif
}

} // end of namespace sss
} // end of namespace path
