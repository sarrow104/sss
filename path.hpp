//{ln:-lshlwapi}
#ifndef  __PATH_HPP_1300351807__
#define  __PATH_HPP_1300351807__

#ifdef __WIN32__
#       include <shlwapi.h>
#       include <windows.h>
#       include <io.h>
#else
#       include <sys/stat.h>
#       include <unistd.h>
#endif

#include <cstdlib>
#include <cstring>
#include <cassert>

#include <stdexcept>
#include <string>
#include <iostream>

#include <sss/util/PostionThrow.hpp>

namespace sss {

enum path_type{PATH_NOT_EXIST = 0, PATH_TO_DIRECTORY = 1, PATH_TO_FILE = 2};

    // Sarrow: 2011-09-25
std::string escape_str(const std::string & s, const std::string& patterns, const char escape_char);

namespace path {

    // 这函数，有必要吗？
    // pop_front(std::stirng& path);
    //
    // pop_back() 等效于 to_upper_dir()
    //
    // 但是前者，pop_front，为什么不太有用，并有可能产生错误，是因为地址字符串的
    // 多样性；
    //
    // 对于windows来说，它的地址字符串，都有一个特征，存在"根目录"这个概念；比如
    // 网络共享目录，其前面的 \\machine-name，可以看做是"根目录"；
    // C:\Windows\system32 中的 C: 可以看做是"根目录"；
    //
    // 甚至linux中，单独的 / 也可以看做是"根目录"；
    //
    // pop_front，就需要，也能够处理"根目录"这个概念；就到了这个问题，前导的
    // sp_char 到底去掉还是保留的讨论了。这会有大不同！

    const char sp_char_win = '\\';
    const char sp_char_linux = '/';

#ifdef __WIN32__
    const char sp_char = sp_char_win;
#else
    const char sp_char = sp_char_linux;
#endif

    inline bool is_sp_char(char ch) {
#ifndef __WIN32__
        return ch == sss::path::sp_char;
#else
        return ch == sss::path::sp_char_linux || ch == sss::path::sp_char_win;
#endif
    }

    bool is_end_with_spchar(const std::string& path);
    bool is_end_with_spchar(std::string::const_iterator ini, std::string::const_iterator fin);
    bool is_begin_with_spchar(const std::string& path);
    bool is_begin_with_spchar(std::string::const_iterator ini, std::string::const_iterator fin);

    bool is_path_under_dir(const std::string& path, const std::string& dir);

    bool is_home_dir(std::string::const_iterator ini, std::string::const_iterator fin);
    bool is_home_dir(const std::string& path, int pos = 0);

    bool is_current_dir(std::string::const_iterator ini, std::string::const_iterator fin);
    bool is_current_dir(const std::string& path, int pos = 0);
    bool is_parent_dir(std::string::const_iterator ini, std::string::const_iterator fin);
    bool is_parent_dir(std::string& ini, int pos = 0);

    void replace_home(std::string& path);
    // NOTE 或许应该假如类似 vim 中的 expand() 函数，以替换路径中的环境变量
    // 不过，我已经有了Penvmgr2；
    // 但是，对于windows中，习惯的%xxx%变量，是不支持的……
    void replace_home(std::string& path);

    inline std::string get_home() {
        return std::getenv("HOME");
    }

    void to_upper_dir(std::string& path);

    std::string::const_iterator next_stem(std::string::const_iterator stem_beg,
                                          std::string::const_iterator stem_end);

    std::string::const_iterator end_of_stem(std::string::const_iterator stem_beg,
                                            std::string::const_iterator stem_end);

    // 在path后面，在附加一个stem；
    // NOTE 如果该stem是绝对路径，则path会被stem替换掉，并执行simplify
    // stem结束的sp_char，会保留？
    std::string& append(std::string& path, const std::string& stem);

    std::string& append(std::string& path,
                        std::string::const_iterator stem_beg,
                        std::string::const_iterator stem_end);

    // 两个路径，最近的公共父目录的长度
    size_t max_dirname_length(const std::string& lhs, const std::string& rhs);

    // 比较两个路径，出现不匹配的——最大相同路径长度外一点的位置
    size_t max_mismatch_pos(const std::string& lhs, const std::string& rhs);

    /* 返回二进制工作文件路径(在很多时候，相当于argv[0]) */
    std::string getbin();

    /**
     * @brief escape 将路径，按照命令行参数规则要求，进行转义
     * 注意，windows系统下，只是cygwin类似系统，才需要这种转义；
     * windows的cmd下，常用的是双引号转义；
     *
     * @param path
     *
     * @return 
     */
    std::string& escape(std::string& path);
    inline std::string escape_copy(const std::string& path)
    {
        std::string path_cp = path;
        (void)escape(path_cp);
        return path_cp;
    }

    /**
     * @brief unescape 反 escape
     *
     * @param path
     *
     * @return 
     */
    std::string& unescape(std::string& path);
    inline std::string unescape_copy(const std::string& path)
    {
        std::string path_cp = path;
        (void)unescape(path_cp);
        return path_cp;
    }

    /**
     * @brief en_dquote 为路径添加双引号；并转义内部特殊字符
     *
     * @param path
     *
     * @return 
     */
    std::string& en_dquote(std::string& path);
    inline std::string en_dquote_copy(std::string& path)
    {
        std::string path_cp = path;
        (void)en_dquote(path_cp);
        return path_cp;
    }

    /**
     * @brief un_dquote 取消路径双引号——注意，双引号的配对性
     *
     * @param path
     *
     * @return 
     */
    std::string& un_dquote(std::string& path);
    inline std::string un_dquote_copy(std::string& path)
    {
        std::string path_cp = path;
        (void)un_dquote(path_cp);
        return path_cp;
    }

    // 2012-06-11
    // true. succeed
    // false. failed
    bool mkdir(const std::string& dir);
    bool mkdir(const char * dir);
    bool rmdir(const std::string& dir, bool is_force);
    bool remove(const std::string& path);
    bool copy(const std::string& src, const std::string& tar);
    bool copy_dir(const std::string& src, const std::string& tar);

    bool is_empty_tree(const std::string& path);

    // 内部，没有文件，并且，若内部只包含文件夹，并且，也没有文件，或者只有空文件的时候，
    // 删除当前文件夹
    inline bool remove_empty_tree(const std::string& path)
    {
        if (sss::path::is_empty_tree(path)) {
            return sss::path::rmdir(path, true);
        }
        return false;
    }

    // 递归创建目录路径
    // 2012-06-11
    // true. succeed
    // false. failed
    bool mkpath(const std::string& path);

    std::string basename(const std::string& path);
    std::string dirname(const std::string& path);

    std::string  no_suffix(const std::string& path);
    // return suffix of a path-base
    // NOTE 后缀包括"."!
    std::string  suffix(const std::string& path);

    bool filereadable(const std::string& path);
    bool filewriteable(const std::string& path);

    std::string mime_type(const std::string& path);

    std::string  modify_copy(const std::string& path, const std::string& modifier);
    std::string& modify(std::string& path, const std::string& modifier);

#ifdef __WIN32__
#else
    bool chmod(const std::string& path, int mode);
    bool chmod(const char * path, int mode);

    bool chmod(const std::string& path, char option, int mode);
    bool chmod(const char * path, char option, int mode);

    int  getmod(const char * path);

    inline int  getmod(const std::string& path)
    {
        return getmod(path.c_str());
    }
#endif

    std::string  full_of_copy(const std::string& path);
    std::string& full_of(std::string& path);

    std::string  relative_to(const std::string& path, const std::string& dir);

    std::string  simplify_copy(const std::string& path);
    std::string& simplify(std::string& path);

    bool dirClear(const std::string& dir);

    std::string getcwd();
    bool chgcwd_nothrow(const std::string& dir);

    inline bool chgcwd(const std::string& dir)
    {
        if (!sss::path::chgcwd_nothrow(dir)) {
            SSS_POSTION_THROW(std::runtime_error, "unable to chdir(" << dir << ")");
        }
        return true;
    }

    /* 当输入的path是一个空白字符串的时候，返回当前工作目录的根目录
     * ——在windows下，就是驱动器号+冒号+斜杠
     * ——在Linux下，就是/
     * 当输入的path非空的时候；则解析出根目录——如果能解析出来的话。*/
    // 为什么不返回一个char呢？
    // '/'表示linux根目录，'C'，'D'等表示盘符，'\0'表示；
    // 这种策略可行，并且没有动态内存管理的问题。
    // 问题在于，我需要额外提供一个append函数，以组合路径；
    // 如果使用std::string接口，我则不用提供额外的函数。
    std::string getroot(const std::string& path="");

    /**
     * @brief generate random hex style file/dir name under specified path
     *
     * @param [in] path which path to generate; default current working dir
     *
     * @param [in] len  the empty file/dir name length
     *
     * @return the empty fname; on failed length == 0;
     */
    std::string gen_random_fname(const std::string& path = "", size_t len = 8);

    bool rename(const std::string& oldpath, const std::string& newpath);
    bool rename(const char * oldpath,       const char * newpath);

    size_t is_symlink(const std::string& path);

    std::string readlink(const std::string& path);

    bool symlink(const std::string& path);

    bool is_equal(const std::string& path1, const std::string& path2);

    bool is_relative(const std::string& path);

    bool is_absolute(const std::string& path);

    bool is_absolute(std::string::const_iterator path_beg, std::string::const_iterator path_end);

    // TODO 这个函数，与 "sss::path" 无关！应该挪到类似 binio.hpp 里面！
    bool file2string(const std::string& file_name, std::string& content);

    path_type file_exists(const char * path);
    path_type file_exists(const std::string& path);

    inline bool is_directory(const std::string& path) {
#ifdef __WIN32__
        return PathIsDirectory(path.c_str());
#else
        std::string full_path = path::full_of_copy(path);
        // int _access();
        struct stat st;
        stat(full_path.c_str(), &st);
        // FIXME 不能识别 "~" ?!
        // 相对路径，全路径？
        return S_ISDIR(st.st_mode);
#endif
    }

    inline bool is_file(const std::string& path) {
#ifdef __WIN32__
        return PathIsFileSpec(path.c_str());
#else
        std::string full_path = path::full_of_copy(path);

        // int _access();
        struct stat st;
        stat(full_path.c_str(), &st);
        return !S_ISDIR(st.st_mode);
#endif
    }

    std::string& append(std::string& path, const std::string& stem);

    inline std::string append_copy(const std::string& path, const std::string& stem)
    {
        std::string ret_path(path);
        return sss::path::append(ret_path, stem);
    }

    inline std::string suffix_strip(const std::string& path)
    {
        return path::no_suffix(path);
    }

    // NOTE 后缀替换用的后缀分割小数点'.'，应当由用户提供！
    // NOTE 2015-10-22
    // 另外，如果原始文件名，以'.'开头，怎么办？
    inline std::string suffix_alter(const std::string& path, const std::string& suffix)
    {
        return path::no_suffix(path) + suffix;
    }
    bool rmdir_force(const std::string& dir);
}
} /* namespace sss*/

#endif  /* __PATH_HPP_1300351807__ */
