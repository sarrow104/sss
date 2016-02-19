#include "glob_path.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "filter_t.hpp"

#include <sss/Terminal.hpp>

#include <sss/log.hpp>
#include <sss/path.hpp>
#include <sss/utlstring.hpp>
#include <sss/log.hpp>


// NOTE win32api FindFirstFile 必须要提供匹配串；并且，不支持多后缀选择
// 因此，不管是 win32 还是 linux，都默认使用 regex 库，用来……
// NOTE linux下，有 glob()@glob.h 枚举函数，也支持类windows下的文件搜索；

namespace sss { namespace path {

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
        return _entry.d_type == DT_REG
            || (this->is_symlink() &&
                sss::PATH_TO_FILE == sss::path::file_exists(this->get_path()));
#endif
    }

    glob_path::glob_path(const std::string& root,
              file_descriptor& fd,
              filter_t * filter)
    :
        _is_next(false),
        is_ok(true),
        _pfd(&fd),
        _dir(root),
        _filter(filter)
#ifdef __WIN32__
      ,hFind(INVALID_HANDLE_VALUE)
#else
      ,dirp(0)
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
            this->dirp = ::opendir(_dir.c_str());
            this->is_ok = this->dirp;
            if (!this->dirp) {
                ::fprintf(stderr, "opendir(\"%s\") : %s%s%s\n",
                          _dir.c_str(),
                          sss::Terminal::style::begin(sss::Terminal::style::FONT_BOLD |
                                                      sss::Terminal::style::FONT_F_RED).data(),
                          ::strerror(errno),
                          sss::Terminal::style::end.data());
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
        if (dirp) {
            (void)::closedir(dirp);
            dirp = 0;
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
        if (this->good()) {
            errno = 0;
            ::readdir_r(dirp, &_pfd->_entry, &_pfd->_pentry);
            if (!_pfd->is_ok() && errno) {
                ::fprintf(stderr, "readdir_r(\"%s\") : %s%s%s\n",
                          _dir.c_str(),
                          sss::Terminal::style::begin(sss::Terminal::style::FONT_BOLD |
                                                      sss::Terminal::style::FONT_F_RED).data(),
                          ::strerror(errno),
                          sss::Terminal::style::end.data());
                (void)::closedir(this->dirp);
                this->dirp = 0;
                this->is_ok = false;
            }
            return _pfd->is_ok();
        }
        return false;
#endif
    }

} // end of namespace sss
} // end of namespace path
