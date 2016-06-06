#include <sss/path.hpp>

#ifdef __WIN32__
#       include <windows.h>
#       include <shlwapi.h>
#else
#       include <sys/types.h>
#       include <sys/stat.h>
#       include <unistd.h> // int access(const char *pathname, int mode);
#endif

#ifndef __WIN32__
#       include <error.h>
#endif

#include <errno.h>

#include <stdlib.h>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include <sstream>
#include <string>

#include <cstring>
#include <cstdio>
#include <cctype>

#include <sss/algorithm.hpp>
#include <sss/spliter.hpp>
#include <sss/utlstring.hpp>
#include <sss/ps.hpp>
#include <sss/path/glob_path.hpp>

#include <sss/bit_operation/bit_operation.h>

namespace {
    void sub_dir_after(std::string& path, size_t pos)
    {
        if (path.length() > pos && path[pos] == sss::path::sp_char) {
            path = path.substr(pos + 1);
        }
        else {
            path = path.substr(pos);
        }
    }

    bool mkpath_inner(std::string& path)
    {
#if 0
        int last_sp_pos = 0;
        if (path.length()) {
            last_sp_pos = path.length() - 1;
            while (last_sp_pos >= 0 && path[last_sp_pos] == sss::path::sp_char) {
                last_sp_pos --;
            }
            while (last_sp_pos > 0 && path[last_sp_pos] != sss::path::sp_char) {
                last_sp_pos --;
            }

            if (path[last_sp_pos] == sss::path::sp_char) {
                path[last_sp_pos] = '\0';
            }
        }

        // SSS_LOG_EXPRESSION(sss::log::log_ERROR, path.c_str());
        if (!sss::path::file_exists(path.c_str()))
        {
            if (!mkpath_inner(path)) {
                return false;
            }
        }
        if (path[last_sp_pos] == '\0') {
            path[last_sp_pos] = sss::path::sp_char;
        }
        // SSS_LOG_EXPRESSION(sss::log::log_ERROR, path.c_str());
        return sss::path::mkdir(path);
#else
        int last_sp_pos = path.length() - 1;
        while (!sss::path::file_exists(path.c_str())) {
            while (last_sp_pos >= 0 && path[last_sp_pos] == sss::path::sp_char) {
                last_sp_pos --;
            }
            while (last_sp_pos > 0 && path[last_sp_pos] != sss::path::sp_char) {
                last_sp_pos --;
            }

            if (path[last_sp_pos] == sss::path::sp_char) {
                path[last_sp_pos] = '\0';
            }
        }

        while (last_sp_pos < int(path.length())) {
            if (!path[last_sp_pos]) {
                path[last_sp_pos] = sss::path::sp_char;
            }

            if (!sss::path::mkdir(path.c_str())) {
                return false;
            }
            last_sp_pos += std::distance(path.c_str() + last_sp_pos,
                                         std::strchr(path.c_str() + last_sp_pos, '\0'));
        }
        return true;
#endif
    }
}

namespace sss {

namespace path {

    // NOTE
    // 必须是真子路径！而不是是等效！
    // 这意味着，规整之后，path
    bool is_path_under_dir(const std::string& path, const std::string& dir)
    {

        std::string pathTemp = sss::path::simplify_copy(path);
        std::string dirTemp  = sss::path::simplify_copy(dir);

        if (sss::path::is_absolute(pathTemp) && sss::path::is_relative(dirTemp)) {
            sss::path::full_of(dirTemp);
        }

        if (sss::path::is_absolute(dirTemp) && sss::path::is_relative(pathTemp)) {
            sss::path::full_of(pathTemp);
        }

        if (!sss::path::is_end_with_spchar(dirTemp)) {
            dirTemp += sss::path::sp_char;
        }

#ifdef __WIN32__
        bool is_ic = true;
#else
        bool is_ic = false;
#endif
        return sss::is_begin_with(pathTemp, dirTemp, is_ic) && pathTemp.length() > dirTemp.length();
    }

    bool is_home_dir(std::string::const_iterator ini, std::string::const_iterator fin)
    {
        return
            ini != fin && *ini == '~' &&
            (std::distance(ini, fin) == 1 || *(ini + 1) == path::sp_char);
    }

    bool is_home_dir(const std::string& path, int pos)
    {
        // SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
        return is_home_dir(path.begin() + pos, path.end());
    }

    void replace_home(std::string& path)
    {
        // SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
        if (is_home_dir(path))
        {
            std::string home = ::getenv("HOME");

            // SSS_LOG_EXPRESSION(sss::log::log_ERROR, home);

            // 去掉home末尾的path::sp_char，如果有的话
            if (home.length() && *home.rbegin() == sss::path::sp_char)
                home.resize(home.length() - 1);

            std::string::const_iterator cbeg = path.begin() + 1;
            std::string::const_iterator cend = path.end();
            home.append(cbeg, cend);
            sss::path::simplify(home);
            std::swap(home, path);
        }
    }

    bool is_current_dir(std::string::const_iterator ini, std::string::const_iterator fin)
    {
        return
            ini != fin && *ini == '.' &&
            (std::distance(ini, fin) == 1 || *(ini + 1) == path::sp_char);
    }

    bool is_current_dir(const std::string& path, int pos)
    {
        return is_current_dir(path.begin() + pos, path.end());
    }

    bool is_parent_dir(std::string::const_iterator ini, std::string::const_iterator fin)
    {
        return
            std::distance(ini, fin) >= 2 &&
            *ini == '.' && *(ini + 1) == '.' &&
            (std::distance(ini, fin) == 2 || *(ini + 2) == path::sp_char);
    }

    bool is_parent_dir(std::string& path, int pos)
    {
        return is_parent_dir(path.begin() + pos, path.end());
    }

    bool is_end_with_spchar(const std::string& path) {
        return !path.empty() && *path.rbegin() == path::sp_char;
    }

    // 'abc/' -> ''
    void to_upper_dir(std::string& path)
    {
        sss::path::simplify(path);
        if (path.length()) {
            int last_sp_pos = path.length() - 1;
            while (last_sp_pos >= 0 && path[last_sp_pos] == sss::path::sp_char) {
                last_sp_pos --;
            }
            while (last_sp_pos > 0 && path[last_sp_pos] != sss::path::sp_char) {
                last_sp_pos --;
            }

            path.resize(last_sp_pos);
        }
        else {
            path.assign("..");
        }
    }

    // stem_beg 必须是结束或者==sp_char
    std::string::const_iterator next_stem(std::string::const_iterator stem_beg,
                                          std::string::const_iterator stem_end)
    {
        while (stem_beg < stem_end && *stem_beg == sss::path::sp_char) {
            stem_beg++;
        }
        return stem_beg;
    }

    std::string::const_iterator end_of_stem(std::string::const_iterator stem_beg,
                                            std::string::const_iterator stem_end)
    {
        while (stem_beg < stem_end && *stem_beg == sss::path::sp_char) {
            stem_beg++;
        }
        while (stem_beg < stem_end && *stem_beg != sss::path::sp_char) {
            stem_beg++;
        }
        return stem_beg;
    }

    // {{{1
    // NOTE
    // append(".", ".")  == "."   // 当前目录的当前目录，还是当前目录
    // append(".", "..") == ".."  // 当前目录的父目录，还是父目录
    // append("..", ".") == ".."  // 类似
    // append("/", "..") == "/"   // 根目录不能在往上；于是还得自己；
    std::string& append(std::string& path,
                        std::string::const_iterator stem_beg,
                        std::string::const_iterator stem_end)
    {
        // SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
        // SSS_LOG_EXPRESSION(sss::log::log_ERROR, path);
        if (stem_beg < stem_end) {
            if (sss::path::is_absolute(stem_beg, stem_end)) {
                path.resize(0);
                while (stem_beg < stem_end) {
                    path += *stem_beg;
                    if (*++stem_beg == sss::path::sp_char) {
                        break;
                    }
                }
                sss::path::append(path, next_stem(stem_beg, stem_end), stem_end);
            }
            else {
                if (sss::path::is_current_dir(stem_beg, stem_end)) {
                    if (std::distance(stem_beg, stem_end) >= 3) {
                        sss::path::append(path, next_stem(stem_beg + 2, stem_end), stem_end);
                    }
                }
                else if (sss::path::is_parent_dir(stem_beg, stem_end)) {
                    to_upper_dir(path);
                    if (std::distance(stem_beg, stem_end) >= 4) {
                        append(path, next_stem(stem_beg + 3, stem_end), stem_end);
                    }
                }
                else {
                    if (!path.empty() && *path.rbegin() != path::sp_char) {
                        path += path::sp_char;
                    }
                    std::string::const_iterator cur_end = end_of_stem(stem_beg, stem_end);
                    path.append(stem_beg, cur_end);
                    sss::path::append(path, next_stem(cur_end, stem_end), stem_end);
                }
            }
        }
        return path;
    }

    std::string& append(std::string& path, const std::string& stem) // {{{1
    {
        return sss::path::append(path, stem.begin(), stem.end());
    }

    // 两个路径，最近的公共父目录的长度——根据情况，决定是否包含末尾的
    // path::sp_char;
    size_t max_dirname_length(const std::string& lhs, const std::string& rhs) // {{{1
    {
        std::string::const_iterator first1 = lhs.begin();
        std::string::const_iterator last1 = lhs.end();

        std::string::const_iterator first2 = rhs.begin();
        std::string::const_iterator last2 = rhs.end();

        int last_sp_pos = -1;
        while (first1 != last1 && first2 != last2) {
            bool is_equal =
#ifdef __WIN32__
                sss::char_equal_casei()(*first1, *first2);
#else
            *first1 == *first2;
#endif
            if (!is_equal) {
                break;
            }

            if (*first1 == path::sp_char) {
                last_sp_pos = first1 - lhs.begin();
            }

            ++first1;
            ++first2;
        }
        // 退出循环，有两种情况：
        // 1. 一种一个路径，到头了；此时，必要至少有一个first是等于last的；
        // 2. 遇到了不同；此时，必然两个first迭代器，都不等于last
        //
        // 第一种
        //    如果 都等于last，说明两个路径完全等效；
        //        返回单个路径长度即可；
        //
        //    如果，其中一个有剩余；
        //         如果，剩下部分，是以 sp_char 开始，如
        //                  /abc
        //                  /abc/
        //              那么，返回
        //              first1 - lhs.begin()
        //
        //         如果，剩余部分，不是以sp_char开始
        //                  /abc
        //                  /abc123
        //              那么，返回 last_sp_pos
        //
        // 第二种
        //    如果，两个都有剩余
        //         （都以sp_char开始，——这是不可能的）
        //         如果，余下的，有以 sp_char 开始的，
        //                  /abc/
        //                  /abc123
        //               那么，返回 last_sp_pos
        //
        //         如果，余下的，都不以 sp_char 开始
        //                  /abcxyz
        //                  /abc123
        //
        //               那么，返回 last_sp_pos
        //

        // 从一开始就不同
        if (-1 == last_sp_pos && (first1 != last1 && first2 != last2)) {
            return 0;
        }

        if (first1 == last1 && first2 == last2) {
            last_sp_pos = lhs.length() - 1;
        }
        else if (first1 != last1 && first2 != last2) {
            // std::cout << "all not end" << std::endl;
            // nothing
        }
        else if (first1 != last1) {
            if (*first1 == path::sp_char) {
                last_sp_pos = first1 - lhs.begin() - 1;
            }
        }
        else if (first2 != last2) {
            // std::cout << "rhs not end; " << first2 - rhs.begin() << std::endl;
            if (*first2 == path::sp_char) {
                last_sp_pos = first2 - rhs.begin() - 1;
            }
        }
        else {
            // cannot go throw here!
        }

        return last_sp_pos + 1;
    }

    // 可以循环的方式获取路径节点；
    // 注意，第一个可能是根节点，比如"/"或者"D:\\"
    // class iterator()

    // 比较两个路径，出现不匹配的——最大相同路径长度
    size_t max_mismatch_pos(const std::string& lhs, const std::string& rhs) // {{{1
    {
        return max_dirname_length(lhs, rhs) + 1;
    }

    bool dirClear(const std::string& dir) // {{{1
    {
        sss::ps::StringPipe ps;
#ifdef __WIN32__
        ps << "rmdir /s";
        ps.add(dir);
#else
        ps.add("rm").add("-rf").add(dir+"/");
#endif
        return std::system(ps.str().c_str()) == -1;
    }

    bool file2string(const std::string& file_name, std::string& content) // {{{1
    {
        if (!sss::path::filereadable(file_name)) {
            std::ostringstream oss;
            oss << "sss::path::" << __func__ << " unable to open file `" << file_name << "` to read!";
            throw std::logic_error(oss.str());
        }
#if 0
        FILE * pf = fopen(file_name.c_str(), "rb");
        if (!pf) {
            return false;
        }
        fseek(pf, 0, SEEK_END);
        size_t pos_end = ftell(pf);
        fseek(pf, 0, SEEK_SET);
        if (pos_end) {
            content.resize(pos_end);
            size_t cnt = fread(const_cast<char*>(content.c_str()), pos_end, 1, pf);
            if (cnt != 1) {
                std::ostringstream oss;
                oss << __func__ << "(cnt = " << cnt << ", pos_end = " << pos_end << "); fread error!";
                throw std::logic_error(oss.str());
            }
        }
#else
    std::ifstream t(file_name.c_str(), std::ios_base::in | std::ios_base::binary);
    std::string buffer((std::istreambuf_iterator<char>(t)),
                       std::istreambuf_iterator<char>());
    buffer.swap(content);
#endif
        return true;
    }

    sss::path_type file_exists(const char * path) // {{{1
    {
        sss::path_type ret = PATH_NOT_EXIST;
#ifdef __WIN32__
        DWORD attrib = ::GetFileAttributes(path);
        if ( attrib == 0xFFFFFFFF ) {
            ret = PATH_NOT_EXIST;
        }
        else if ( attrib & FILE_ATTRIBUTE_DIRECTORY ) {
            ret = PATH_TO_DIRECTORY;
        }
        else if ( attrib & FILE_ATTRIBUTE_ARCHIVE ) {
            ret = PATH_TO_FILE;
        }
        else {
            throw std::logic_error("unexpect file attributes!");
        }
#else
        struct stat sb;

        // NOTE stat函数，对于 symlink文件，透明；
        // 所以可以直接使用！
        if (stat(path, &sb) == 0)
        {
            if (S_ISDIR(sb.st_mode)) {
                ret = PATH_TO_DIRECTORY;
            }
            else {
                ret = PATH_TO_FILE;
            }
        }
#endif
        return ret;
    }

    sss::path_type file_exists(const std::string& path) // {{{1
    {
        return sss::path::file_exists(path.c_str());
    }

    size_t is_symlink(const std::string& path) // {{{1
    {
#ifdef __WIN32__
        // TODO FIXME
        return false;
#else
        struct stat statbuff;
        std::string buf;
        size_t path_len = 0;
        if (lstat(path.c_str(), &statbuff) < 0) {
            throw std::runtime_error(std::strerror(errno));
        }
        if (S_ISLNK(statbuff.st_mode)) {
            path_len = statbuff.st_size;
        }
        return path_len;
#endif
    }

    std::string readlink(const std::string& path) // {{{1
    {
#ifdef  __WIN32__
        // TODO FIXME
        return path;
#else
        std::string buf(path.begin(), path.end());

        if (size_t path_len = sss::path::is_symlink(path)) {
            buf.resize(path_len);
            if (::readlink(path.c_str(), const_cast<char*>(buf.c_str()), buf.size()) == -1) {
                throw std::runtime_error(std::strerror(errno));
            }
        }
        return buf;
#endif
    }

    bool symlink(const std::string& oldpath, const std::string& newpath)
    {
#ifdef  __WIN32__
        // TODO FIXME
        return false;
#else
        return ::symlink(oldpath.c_str(), newpath.c_str()) == 0;
#endif
    }

    bool is_equal(const std::string& path1, const std::string& path2)
    {
        std::string full_path1 = sss::path::full_of_copy(path1);
        std::string full_path2 = sss::path::full_of_copy(path2);
        sss::path::simplify(full_path1);
        sss::path::simplify(full_path2);

        if (!full_path1.empty() && *full_path1.rbegin() == sss::path::sp_char) {
            full_path1.resize(full_path1.length() - 1);
        }

        if (!full_path2.empty() && *full_path2.rbegin() == sss::path::sp_char) {
            full_path2.resize(full_path2.length() - 1);
        }

        std::string::const_iterator first1 = full_path1.begin();
        std::string::const_iterator last1 = full_path1.end();

        std::string::const_iterator first2 = full_path2.begin();
        std::string::const_iterator last2 = full_path2.end();

        if (full_path1.length() != full_path2.length()) {
            return false;
        }

        while (first1 != last1 && first2 != last2) {
            bool is_equal =
#ifdef __WIN32__
                sss::char_equal_casei()(*first1, *first2);
#else
            *first1 == *first2;
#endif
            if (!is_equal) {
                return false;
            }
        }
        return true;
    }

    bool is_relative(const std::string& path) // {{{1
    {
        return !is_absolute(path);
    }

    bool is_absolute(std::string::const_iterator path_beg, std::string::const_iterator path_end) // {{{1
    {
#ifdef __WIN32__
        // NOTE
        // 文件名允许开始的字符是空格，所以，不能有什么trim的预处理动作！切记！
        return (std::distance(path_beg, path_end) >= 2 && *(path_beg + 1) == ':' && std::isalpha(*path_beg));
#else
        return (path_beg < path_end && *path_beg == path::sp_char);
#endif
    }

    bool is_absolute(const std::string& path) // {{{1
    {
        return sss::path::is_absolute(path.begin(), path.end());
    }

    std::string getroot(const std::string& path) // {{{1
    {
#ifdef __WIN32__
        // NOTE
        // 文件名允许开始的字符是空格，所以，不能有什么trim的预处理动作！切记！
        if (sss::path::is_absolute(path))
            return path.substr(0, 2) + "\\";
        else if (path.empty())
            return getroot(sss::path::getcwd());
        else
            return "";
#else
        (void) path;
        // 在linux系统下，求根目录是没有什么意义的。根目录总是"/"
        return "/";
#endif
    }

    std::string gen_random_fname(const std::string& path, size_t len)
    {
        if (len == 0) {
            return "";
        }
        std::string target_path = sss::path::full_of_copy(path);

        size_t max_try = 10;
        static bool init_srand = (std::srand(time(0)), true);
        (void)init_srand;

        std::string random_fname;
        random_fname.reserve(len);
        size_t base_length = target_path.length();
        while (max_try--) {
            while (random_fname.length() < len) {
                int number = std::rand();
                while (random_fname.length() < len && number) {
                    int remainder = number % 16;
                    number /= 16;
                    if (remainder < 10) {
                        random_fname += '0' + remainder;
                    }
                    else {
                        random_fname += 'A' + remainder - 10;
                    }
                }
            }
            sss::path::append(target_path, random_fname);
            if (sss::path::file_exists(target_path) == sss::PATH_NOT_EXIST) {
                break;
            }
            random_fname.resize(0);
            target_path.resize(base_length);
        }

        return random_fname;
    }

    bool rename(const std::string& oldpath, const std::string& newpath)
    {
        return sss::path::rename(oldpath.c_str(), newpath.c_str());
    }

    bool rename(const char * oldpath, const char * newpath)
    {
        bool ret = (std::rename(oldpath, newpath) != -1);
        // std::cout << oldpath << " -> " << newpath << (ret ? " ok" : " failed") << std::endl;
        return ret;
    }

    std::string getbin() // {{{1
    {
        std::string ret;
#ifdef __WIN32__
        char buffer[MAX_PATH];
        // GetModuleFileName的第一个参数是 模块的句柄——当传入的句柄为NULL的时候，
        // 则返回当前工作程序的路径。
        // 根据GetModuleFileName的函数说明，不管缓冲区够不够，在xp/2000系统下，这个
        // 函数都会设置错误状态为ERROR_SUCCESS！并且，返回值也是拷贝进buffer中的字
        // 符数。就是说，要知道名字到底全了没有，你只能靠猜！
        // 如果有一个操蛋的exe文件名是：
        // xxxx.exe.sgdsdg.exe
        // 的话，并且，所在的目录足够深……
        // 这真是一个操蛋的API！为什么不能像其他的API一样，当给的buffer长度为0的时
        // 候，直接返回所需的buffer长呢？
        // 那么，GetModuleFileName这个函数又是如何实现的呢？
        int len = ::GetModuleFileName(NULL, buffer, sizeof(buffer));
        ret.assign(buffer, len);
#else
        size_t linksize = 256;
        char exeName[linksize];
        ssize_t len = 0;
        if ( (len = ::readlink("/proc/self/exe", exeName, linksize)) != -1 )
        {
            exeName[len] = '\0';
            ret = exeName;
        }
        else {
            std::ostringstream oss;
            oss << __func__ << ": readlink() " << std::strerror(errno) << std::endl;
            throw std::runtime_error(oss.str());
        }
#endif
        return ret;
    }

    std::string getcwd() // {{{1
    {
#ifdef __WIN32__
        std::string ret(::GetCurrentDirectory(0, 0), '\0');
        // NOTE:
        // 1. 使用函数::GetCurrentDirectory得到的路径，是含有盘符的。
        // 2. GetCurrentDirectory函数拷贝的路径包含了null-terminal字符
        ::GetCurrentDirectory(ret.length(), const_cast<char *>(ret.c_str()));
        ret.resize(ret.length() - 1);
        return ret;
#else
        char buf[256];
        if (::getcwd(buf, sizeof(buf)) == 0) {
            std::ostringstream oss;
            oss << "sss::path::" << __func__ << ": getcwd: " << strerror(errno);
            throw std::logic_error(oss.str());
        }

        return buf;
#endif
    }

    bool chgcwd(const std::string& dir) // {{{1
    {
        if (chdir (dir.c_str()) != 0) {
            throw std::runtime_error("::chdir error!");
        }
        return true;
    }

    // 2012-06-11
    bool mkdir(const std::string& dir) // {{{1
    {
        return sss::path::mkdir(dir.c_str());
    }

    bool mkdir(const char * dir) // {{{1
    {
        // io.h / mkdir
        // !0. false
        // 0. succeed
#ifdef __WIN32__
        return !::mkdir(dir);
#else
        // <sys/stat.h>
        // 0    [on] succeed
        // -1   [on] failed
        // FIXME 奇怪了，我明明传入的是0x777参数，为什么创建的文件夹，自己却没有写入的权限？
        //! http://www.linuxidc.com/Linux/2013-08/88797.htm
        return !::mkdir(dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH);
#endif
    }

    // 2012-06-11
    bool rmdir(const std::string& dir, bool is_force) // {{{1
    {
        // io.h / rmdir
        // !0. false
        // 0. succeed
        if (!is_force) {
            return ::rmdir(dir.c_str());
        }
        else {
            return sss::path::dirClear(dir);
        }
    }

    bool remove(const std::string& path) // {{{1
    {
        return ::remove(path.c_str());
    }

    bool is_empty_tree(const std::string& path) // {{{1
    {
        sss::path::file_descriptor fd;
        sss::path::glob_path gp(path, fd);
        while (gp.fetch()) {
            if (fd.is_normal_file()) {
                return false;
            }
            else if (fd.is_normal_dir()) {
                bool is_inner_empty = sss::path::is_empty_tree(fd.get_path());
                if (!is_inner_empty) {
                    return false;
                }
            }
        }
        return true;
    }

    bool copy(const std::string& src, const std::string& tar) // {{{1
    {
        sss::ps::StringPipe ps;

#ifdef __WIN32__
        ps.add("copy");
#else
        ps.add("cp");
#endif
        ps.add(src);
        ps.add(tar);

        return std::system(ps.str().c_str()) == -1;
    }

    // NOTE
    // 文件夹拷贝，windows下需要使用xcopy；
    // linux，则是cp -rf
    // 需要注意的是 tar 是"源"将存放的路径！
    // 如果tar表示的路径不存在，则有两种情况：
    // 1. tar的dirname是存在的路径，那么，将会把src拷贝，并重命名为basename(tar)；
    //    相当于，将src另存为另外一个名字；
    // 2. 如果dirname(tar)不存在，那么就会报错了——因为，cp并不支持创建目标目录；
    bool copy_dir(const std::string& src, const std::string& tar) // {{{1
    {
        sss::ps::StringPipe ps;

#ifdef __WIN32__
        ps << "xcopy";
#else
        ps << "cp -rf";
#endif
        ps.add(src);
        ps.add(tar);

        return std::system(ps.str().c_str()) == -1;
    }

    // FIXME
    // 输入的有效性检查;
    // 比如：
    // 1. 输入的串，长度是0
    // 2. 输入的是一个'.' —— 此时，返回0（表示成功）即可。
    // 3. 输入的是".."——需要判断当前是否是'根目录'。
    // 当然，最好是先取得全路径比较好。
    bool mkpath(const std::string& path) // {{{1
    {
        // SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
        // NOTE
        // 针对 win32 的路径，需要做特殊处理——不断的递归循环，到最后，是驱动器根目录。
        // 如果对应的根目录不存在，这是如何也不可能通过'mkdir'能创建出来的！
        // SSS_LOG_EXPRESSION(sss::log::log_DEBUG, path);
        std::string full_path = sss::path::full_of_copy(path);

        // 路径已经存在
        if (sss::path::file_exists(full_path)) {
            return false;
        }
        sss::path::simplify(full_path);
        // SSS_LOG_EXPRESSION(sss::log::log_DEBUG, full_path);
        return ::mkpath_inner(full_path);
    }

    // NOTE
    // 注意
    //  路径首尾是可有空格的。所以，用户应该注意，是否有键入多有的空格！
    std::string basename(const std::string& path) // {{{1
    {
        /*
         * $ basename /
         * /
         * $ basename /123/x.txt
         * x.txt
         * $ basename /123/x.txt/
         * x.txt
         */
        int tail_pos = path.length() - 1;
        if (tail_pos >= 0 && path[tail_pos] == path::sp_char) {
            --tail_pos;
        }
        int before_head_pos = tail_pos;
        for (; before_head_pos > -1; --before_head_pos) {
            if (path[before_head_pos] == path::sp_char) {
                break;
            }
        }
        size_t len = tail_pos - (before_head_pos + 1) + 1;
        if (len) {
            return path.substr(before_head_pos + 1, len);
        }
        else {
            char root[2] = {path::sp_char, '\0'};
            return root;
        }
    }

    std::string dirname(const std::string& path) // {{{1
    {
        /* $ dirname /123/x.txt
         * /123
         * $ dirname /
         * /
         * $ dirname /123/x.txt/
         * /123
         * $ dirname 123
         * .
         */
        // case ""
        //  返回一个错误
        // case x.txt
        //  返回.
        int tail_pos = path.length() - 1;
        if (path.length() && *path.rbegin() == path::sp_char) {
            --tail_pos;
        }
        int last_sp_pos = tail_pos;
        for (; last_sp_pos > -1; --last_sp_pos) {
            if (path[last_sp_pos] == path::sp_char) {
                break;
            }
        }
        std::string ret;
        if (last_sp_pos != -1) {
            ret = path.substr(0, last_sp_pos);
        }
        if (ret.empty()) {
            if (sss::path::is_absolute(path)) {
                ret += path::sp_char;
            }
            else {
                ret = ".";
            }
        }
        return ret;
    }

    std::string no_suffix(const std::string& path) // {{{1
    {
        size_t last_dot_pos = -1;
        for (int i = path.length() - 1; i >= 0 &&  path[i] != path::sp_char; --i)
        {
            if (path[i] == '.')
            {
                last_dot_pos = i;
                break;
            }
        }
        if (last_dot_pos == size_t(-1))
            last_dot_pos = std::string::npos;
        return path.substr(0, last_dot_pos);
    }

    // NOTE 后缀包括"."!
    std::string suffix(const std::string& path) // {{{1
    {
        int last_dot_pos = -1;
        for (int i = path.length() - 1; i >= 0 && path[i] != path::sp_char; --i)
        {
            if (path[i] == '.')
            {
                last_dot_pos = i;
                break;
            }
        }

        // 没找到last_dot_pos或者第一个遇到的是 path::sp_char
        if (last_dot_pos == -1)
            return "";
        else if (last_dot_pos > 0 && path[last_dot_pos - 1] == path::sp_char)
            return "";
        else
            return path.substr(last_dot_pos);
    }

    bool filereadable(const std::string& path) // {{{1
    {
#ifdef __WIN32__
        FILE * pf = fopen(path.c_str(), "r");
        bool ret(pf);
        if (pf)
            fclose(pf);
        return ret;
#else
        return 0 == access(path.c_str(), R_OK);
#endif
    }

    bool filewriteable(const std::string& path) // {{{1
    {
#ifdef __WIN32__
        assert(false);
        // TODO
        // not implemented! do not use this function!
        return true;
#else
        return 0 == access(path.c_str(), W_OK);
#endif
    }

    std::string mime_type(const std::string& path) // {{{1
    {
#ifdef __WIN32__
        assert(false);
        // TODO
        // not implemented! do not use this function!
        return "";
#else
        sss::ps::StringPipe ps;
        ps.add("file").add("--brief").add("--mime-type").add(path);
        ps << " 2>/dev/null";
        std::string ret = ps.run();
        sss::trim(ret);
        return ret;
#endif
    }

    std::string modify_copy(const std::string& path, const std::string& modifier) // {{{1
    {
        std::string ret = path;
        return modify(ret, modifier);
    }

    std::string& modify( std::string& path, const std::string& modifier) // {{{1
    {
        // TODO
        // vim 下的 fnamemodify函数，有时候真的很好用；在自己的库里面，实现同样的功能
        // ——是同样，不是类似！起码两者的行为要尽可能相同！
        /* 原文{{{1
         *	:p	Make file name a full path.  Must be the first modifier.  Also
         *		changes "~/" (and "~user/" for Unix and VMS) to the path for
         *		the home directory.  If the name is a directory a path
         *		separator is added at the end.  For a file name that does not
         *		exist and does not have an absolute path the result is
         *		unpredictable.
         *	:8	Converts the path to 8.3 short format (currently only on
         *		win32).  Will act on as much of a path that is an existing
         *		path.
         *	:~	Reduce file name to be relative to the home directory, if
         *		possible.  File name is unmodified if it is not below the home
         *		directory.
         *	:.	Reduce file name to be relative to current directory, if
         *		possible.  File name is unmodified if it is not below the
         *		current directory.
         *		For maximum shortness, use ":~:.".
         *	:h	Head of the file name (the last component and any separators
         *		removed).  Cannot be used with :e, :r or :t.
         *		Can be repeated to remove several components at the end.
         *		When the file name ends in a path separator, only the path
         *		separator is removed.  Thus ":p:h" on a directory name results
         *		on the directory name itself (without trailing slash).
         *		When the file name is an absolute path (starts with "/" for
         *		Unix; " x:\ " for MS-DOS, WIN32, OS/2; " drive: " for Amiga), that
         *		part is not removed.  When there is no head (path is relative
         *		to current directory) the result is empty.
         *	:t	Tail of the file name (last component of the name).  Must
         *		precede any :r or :e.
         *	:r	Root of the file name (the last extension removed).  When
         *		there is only an extension (file name that starts with '.',
         *		e.g., ".vimrc"), it is not removed.  Can be repeated to remove
         *		several extensions (last one first).
         *	:e	Extension of the file name.  Only makes sense when used alone.
         *		When there is no extension the result is empty.
         *		When there is only an extension (file name that starts with
         *		'.'), the result is empty.  Can be repeated to include more
         *		extensions.  If there are not enough extensions (but at least
         *		one) as much as possible are included.
         *	:s?pat?sub?
         *		Substitute the first occurrence of "pat" with "sub".  This
         *		works like the |:s| command.  "pat" is a regular expression.
         *		Any character can be used for '?', but it must not occur in
         *		"pat" or "sub".
         *		After this, the previous modifiers can be used again.  For
         *		example ":p", to make a full path after the substitution.
         *	:gs?pat?sub?
         *		Substitute all occurrences of "path" with "sub".  Otherwise
         *		this works like ":s".
         *
         *Examples, when the file name is "src/version.c", current dir
         *"/home/mool/vim": >
         *  :p			/home/mool/vim/src/version.c
         *  :p:.				       src/version.c
         *  :p:~				 ~/vim/src/version.c
         *  :h				       src
         *  :p:h			/home/mool/vim/src
         *  :p:h:h		        /home/mool/vim
         *  :t					   version.c
         *  :p:t					   version.c
         *  :r				       src/version
         *  :p:r			/home/mool/vim/src/version
         *  :t:r					   version
         *  :e						   c
         *  :s?version?main?		       src/main.c
         *  :s?version?main?:p	/home/mool/vim/src/main.c
         *  :p:gs?/?\\?		\home\mool\vim\src\version.c
         *
         *Examples, when the file name is "src/version.c.gz": >
         *  :p			/home/mool/vim/src/version.c.gz
         *  :e						     gz
        *  :e:e						   c.gz
            *  :e:e:e					           c.gz
            *  :e:e:r					           c
            *  :r				       src/version.c
            *  :r:e						   c
            *  :r:r				       src/version
            *  :r:r:r			               src/version
            * }}}1
            */
            // 首先，保证modifier的长度，是2的整数倍
            bool is_status_ok = (0 == modifier.length() % 2);
        for (int i = 0; is_status_ok && i < int(modifier.length()); i += 2)
        {
            if (modifier[i] != ':')
            {
                is_status_ok = false;
            }
            else
            {
                switch (std::tolower(modifier[i + 1]))
                {
                case 'p':   // 获取目标的绝对路径
                    sss::path::full_of(path);
                    break;

                case '8':   // 转化为8.3格式文件名。not impliment
                    {
#ifdef __WIN32__
                        // GetShortPathName 将长路径转为8.3格式的短路径格式
                        // GetLongPathName 将短路径格式转为长路径。
                        // PathGetShortPath 将长路径转为短路径格式（8.3格式）
                        // PathCreateFromUrl 将URL路径转为MS-DOS格式
                        char shortPath[MAX_PATH];
                        std::memset(shortPath, 0, sizeof(shortPath));
                        if (!GetShortPathName(path.c_str(), shortPath, path.length())) {
                            path.assign(shortPath);
                        }
#endif
                    }
                    break;

                case '~':   // 获得相对于主文件的相对路径--其值，应当从环境变量中获取
                    // 首先，获得目标的绝对路径，然后再根据主文件的路径来计算。
                    path = sss::path::relative_to(path, getenv("home"));
                    break;

                case '.':   // 相对于当前工作目录的相对路径
                    path = sss::path::relative_to(path, sss::path::getcwd());
                    break;

                case 'h':   // 行为有点怪——返回路径的'head'部分。有点像*unix:dirname
                    // NOTE
                    // 这里，需要path.length() > 1 而不是path.length() > 0！
                    // 因为，path有可能等于'/'。
                    if (sss::path::is_end_with_spchar(path))
                        path.resize(path.length() - 1);
                    else
                        path = sss::path::dirname(path);
                    break;

                case 't':   // 类似：*unix.basename
                    path = sss::path::basename(path);
                    break;

                    // TODO
                    // 针对连续的:r,:e，应该单独处理——或许用一个整数，来作为遇到
                    // 的次数，并标记状态。
                case 'r':   // 去后缀--针对.vim这样仅有后缀的文件名，不予处理
                    path = sss::path::no_suffix(path);
                    break;

                case 'e':   // 获得后缀 (vim下，会忽略'.')
                    path = sss::path::suffix(path);
                    break;

                default:
                    is_status_ok = false;
                    break;
                }
            }
        }
        return path;
    }

    bool chmod(const std::string& path, int mode)
    {
        return sss::path::chmod(path.c_str(), mode);
    }

    inline int fmode_dec2hex(int mode) {
        int sys_mode = (((mode / 100) % 10) & 7u);
        sys_mode <<= 3;
        sys_mode |= (((mode / 10) % 10) & 7u);
        sys_mode <<= 3;
        sys_mode |= ((mode % 10) & 7u);
        return sys_mode;
    }

    inline int fmode_hex2dec(int mode) {
        int sys_mode = ((mode >> 6) & 7u);
        sys_mode *= 10;
        sys_mode += ((mode >> 3) & 7u);
        sys_mode *= 10;
        sys_mode += ((mode >> 0) & 7u);
        return sys_mode;
    }

    // TODO
    // 要不，额外用'+'，'-'参数，表示增加、减去某属性？
    bool chmod(const char * path, int mode)
    {
        ::chmod(path, fmode_dec2hex(mode));

        return true;
    }

    bool chmod(const std::string& path, char option, int mode)
    {
        return sss::path::chmod(path.c_str(), option, mode);
    }

    bool chmod(const char * path, char option, int mode)
    {
        struct stat statbuff;
        std::string buf;
        bool ret = false;
        if (stat(path, &statbuff) == 0) {
            int old_mode = statbuff.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

            int sys_mode = fmode_dec2hex(mode);

            int new_mode = -1;
            switch (option) {
            case '+':
                new_mode = old_mode | sys_mode;
                break;

            case '-':
                new_mode = old_mode & ~sys_mode;
                break;

            default:
                break;
            }

            if (new_mode != -1 && new_mode != old_mode) {
                ::chmod(path, sys_mode);
                ret = true;
            }
        }

        return ret;
    }

    int  getmod(const char * path)
    {
        struct stat statbuff;
        std::string buf;
        if (stat(path, &statbuff) == 0) {
            int old_mode = statbuff.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
            return fmode_hex2dec(old_mode);
        }
        return 0;
    }

    // 返回全路径
    std::string full_of_copy(const std::string& path) // {{{1
    {
        std::string ret = path;
        return path::full_of(ret);
    }

    std::string& full_of(std::string& path) // {{{1
    {
#ifdef __WIN32__
        // 这在win32下，是一种特殊的'相对路径'，需要单独处理：
        if (path.length() && path[0] == path::sp_char)
            path = sss::path::getroot(sss::path::getcwd()) + path.substr(1);
#endif
        if (sss::path::is_relative(path))
        {
            // 2012-06-06 FIXME
            // path = sss::dirname(sss::getcwd()) + path::sp_char + path;
            // NOTE linux 下，相对路径有两种：
            // 1. 非'/'开头 也非 '~'开头；——这是以"工作目录"为比较对象；
            // 2. '~'开头；——这是以环境变量 $HOME 为比较对象；
            replace_home(path);
            if (sss::path::is_relative(path)) {
                std::string cwd(sss::path::getcwd());
                std::swap(cwd, path);
                append(path, cwd);
            }
        }
        return path;
    }

    // 返回，path相对于路径dir的相对路径
    //
    // 流程：
    // 1. 补全路径
    // 2. mismatch
    // 3. dirname
    //
    // 对于输入的要求：
    //  - 不需要是全路径；只要求，通过了simplify动作；
    //
    //  反之，如果参数path已经是全路径，而参数dir是相对路径，这就需要dir也执行路
    //  径补全为全路径；
    //
    //  另外，dir部分，必须被理解为目录；而path部分则没有关系，允许是文件；
    //  由于是计算表达式，所以，没有必要保证得出的相对路径存在；
    //
    std::string relative_to(const std::string& path, const std::string& dir) // {{{1
    {
        std::string fullFilePath(path);
        sss::path::simplify(fullFilePath);

        std::string mainDir(dir);
        sss::path::simplify(mainDir);

        if (sss::path::is_absolute(fullFilePath) && sss::path::is_relative(mainDir)) {
            sss::path::full_of(mainDir);
        }

        if (mainDir.empty() || *mainDir.rbegin() != path::sp_char)
        {
            mainDir += path::sp_char;
        }

        if (sss::path::is_absolute(mainDir) && sss::path::is_relative(fullFilePath)) {
            fullFilePath = mainDir + fullFilePath;
            return sss::path::simplify(fullFilePath);
        }

        // NOTE 对于windows路径来说，如果连盘符都不一样，那么说明，没有必要再计
        // 算相对路径了，直接返回path即可；

        // std::cout << "fullFilePath: " << fullFilePath << std::endl;
        // std::cout << "mainDir: " << mainDir << std::endl;

        size_t mdlength = path::max_dirname_length(fullFilePath, mainDir);

        // std::cout << "fullFilePath<: " << fullFilePath.substr(0, mdlength) << std::endl;
        // std::cout << "mainDir<: " << mainDir.substr(0, mdlength) << std::endl;

        // 都是绝对路径，但是没有相同的地方，那么，取fullFilePath即可；
        if (sss::path::is_absolute(mainDir) && sss::path::is_absolute(fullFilePath) && !mdlength) {
            return fullFilePath;
        }

        sub_dir_after(fullFilePath, mdlength);

        sub_dir_after(mainDir, mdlength);

        // fullFilePath = fullFilePath.substr(mdlength);
        // mainDir      = mainDir.substr(mdlength);

        // std::cout << "fullFilePath-left: " << fullFilePath << std::endl;
        // std::cout << "mainDir-left: " << mainDir << std::endl;

        // mainDir一开始是附加了 path::sp_char 的
        // 所以，……
        if (mainDir.empty() || mainDir == std::string(1, path::sp_char))
        {
            if (fullFilePath == "") {
                return ".";
            }
            else if (sss::path::is_relative(fullFilePath)) {
                return fullFilePath;
            }
        }

        int sp_cnt = std::count(mainDir.begin(), mainDir.end(), path::sp_char);

#ifdef __WIN32__
        const char * parent_s = "..\\";
#else
        const char * parent_s = "../";
#endif

        std::string ret;

        for (int i = 0; i < sp_cnt; ++i) {
            ret.append(parent_s);
        }

        return ret + fullFilePath;
    }

    // 简化路径中出现的./../~等符号
    std::string simplify_copy(const std::string& path) // {{{1
    {
        std::string ret = path;
        return simplify(ret);
    }

    // NOTE 对于simplify来说，\\ -> \ 并不是总成立的事情；
    // 对于windows来说，\\ 后面跟一个计算机名，表示的是网络共享文件夹的地址；
    // 而不应当简化；
    // 于是，应该额外做一个函数，专门用来分析地址的头部，以便分清楚"协议"

    std::string& simplify(std::string& path) // {{{1
    {
        // TODO
        // 利用spliter，挨个拆分path为stem。然后利用栈的方式，处理'.'和".."，'~'，
        // 还有连续的path::sp_char符号。另外，还得事先进行：
        // 当然，也可以不使用spliter，而是直接前后搜索path::sp_char的位置，然后用remove的
        // 方式，来决定是否挪动后面的字符——其实，就是类似两个指针的形式：
        // *(p1+x) = *(p2+y);
        // path_isvalid()判断。
        // 另外，文件名是否允许这样，N个'.'？如，"..."？
        // 经过实验，linux下，可以创建名为"..."的文件；windows下失败-不管是用shell
        // ，还是用程序。

        // 处理'~'——只处理'~'出现在排头的情况：后面要么没有，要么跟一个path::sp_char。
        sss::path::replace_home(path);

        bool is_end_with_slash = sss::path::is_end_with_spchar(path);

        std::string ret;
        sss::path::append(ret, path);
        std::swap(ret, path);
        if (is_end_with_slash && !sss::path::is_end_with_spchar(path)) {
            path += sss::path::sp_char;
        }
        return path;
    }

    bool rmdir_force(const std::string& dir) // {{{1
    {
        // io.h / rmdir
        // !0. false
        // 0. succeed
        return ::rmdir(dir.c_str());
    }
}

// TODO
// 2012-05-17
// 如果，我希望，在*unix系统下，处理win32路径，应该怎么办？遇到反过来的情况呢，
// 又如何？
//
// 所以，一个好的办法是，把路径的两种处理方式，都同时实现。并放入不同的名字空间
// 中。然后，特定环境下，默认调用当前系统环境的路径转化函数组。

// Sarrow: 2011-09-25
std::string escape_str(const std::string & s, const std::string& patterns, const char escape_char)
{
    std::string ret;
    ret.reserve( s.length()/2 + s.length() );
    for (int i = 0; i < static_cast<int>(s.length()); ++i)
    {
        if (patterns.find(s[i]) != std::string::npos) // find
        {
            ret += escape_char;
        }
        ret += s[i];
    }
    return ret;
}

} /* namespace sss*/
