#ifndef __PS_HPP_1439992007__
#define __PS_HPP_1439992007__

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>
#include <stdexcept>
#include <string>

#include <iostream>
#include <map>

// 关于运作模式
//
// 最直接的，就是如下面这种，保存popen需要的条件，然后在需要的时候调用，
// 并按照需要，逐行，或者整体，转化为字符串，供用户处理；
//
// 还有，就是一些语法糖了；
//
// 与 std::stream 结合，有两种方式：
// 1. 继承自 ostreamstream，以完成复杂的命令行拼接；串接完成之后，再利用一个run
//    动作，以触发外部调用；
// 2. 与某std::stream 对象进行绑定，使用的时候，可以参考ext::binary 函数:
//    std::cout << ext::shell << "ls -l" << std::endl;
//
//    这会将"ls -l"的输出，转接到 std::cout 上；
//    streambuf

namespace sss {
namespace ps {

typedef int fd_type;

struct RWEPipe {
    RWEPipe() : pid(0) {}
    ~RWEPipe() { this->close(); }
    fd_type rwe[3];
    int pid;
    fd_type operator[](uint32_t idx) const { return rwe[idx]; }
    fd_type& operator[](uint32_t idx) { return rwe[idx]; }
    fd_type at(uint32_t idx) const
    {
        if (idx >= 3u) {
            throw std::out_of_range("sss::FdRWE::at()");
        }
        return rwe[idx];
    }
    fd_type& at(uint32_t idx)
    {
        {
            if (idx >= 3u) {
                throw std::out_of_range("sss::FdRWE::at()");
            }
            return rwe[idx];
        }
    }
    /**
     * @brief run fork() 子进程，并返回pid
     *
     * @param cmd the command tobe fork,pipe and execl
     * @param wd  sub-process workingdir
     * @param env sub-process environment-variable
     *
     * @return sub-process pid; -1 on error;
     */
    fd_type fork(const std::string& cmd, const std::string& wd,
                 const std::map<std::string, std::string>& env);

    inline fd_type fork(const std::string& cmd)
    {
        std::map<std::string, std::string> env;
        return fork(cmd, "", env);
    }
    /**
     * @brief close close all pipe fd; kill sub-process by pid
     *
     * @return waitpid status code
     */
    int close();

    int waitpid();

    // NOTE 理论上，最合适好用的办法是，std::function，来传入回调函数。
    // 以便管理子进程的 stdout和stderr输出。
    // 然后，通过 epoll ，就可以有效地管理多个fd；
    // epid = epoll_create(1);
    // epoll_ctl(EPOLL_CTL_ADD, OUT)
    // epoll_ctl(EPOLL_CTL_ADD, ERR)
    //
    // while (epoll_wait()) {}
    //
    // 这样来看的话，最好将epoll独立一个库出来；以便库用户进行自己的管理；
    // 另外，有两种方式；一种是使用std::function();
    // 另外一种，是使用模板类。即，用户需要提供自定义处理类；然后由库（epoll框架），完成调用。
    // static void EchoErrHandle(int) {}
    // static void EchoOutHandle(int) {}
    // typedef void (*fdHandle)(int);
    // void onStdOut(fdHandle);
    // void onStdErr(fdHandle);
};
class Pipe {
    static const size_t buf_size = 1024;

public:
    enum mode { pipe_read, pipe_write };

public:
    Pipe() : _fs(0), _mod(Pipe::pipe_read), _is_faild(false) {}
    Pipe(const std::string& cmd, Pipe::mode mod)
        : _cmd(cmd), _fs(0), _mod(mod), _is_faild(false)
    {
    }

    ~Pipe()
    {
        // void line
        this->clear();
    }

public:
    bool run(const std::string& cmd, Pipe::mode mod)
    {
        this->clear();
        this->_cmd = cmd;
        this->_mod = mod;
        return this->run();
    }

    bool run()
    {
        this->clear();
        // NOTE popen 默认只针对 stdin和stdout流；对于 stderr，则不支持！
        // popen本质上是开启一个shell，然后fork运行；所以，这是支持命令行解析的；
        // 于是，想合并stderr和stdout输出的话，可以简单地：
        // popen("prog 2>&1", "r")
        // 类似，想屏蔽错误输出，则：
        // popen("prog 2>/dev/null", "r");
        //
        // 要完全区分这三个流的话，就需要深入到popen的实现中去：
        //! https://github.com/sni/mod_gearman/blob/master/common/popenRWE.c
        // function popenRWE
        // by Bart Trojanowski. Clean way to do all 3 pipes.
        this->_fs =
            popen(this->_cmd.c_str(), (this->_mod == pipe_read ? "r" : "w"));
        this->_is_faild = !this->_fs;
        return !this->_is_faild;
    }

    void on_error() const
    {
        if (this->_is_faild) {
            std::ostringstream oss;
            oss << __func__ << ":execute command failed: " << strerror(errno);
            throw std::runtime_error(oss.str());
        }
    }

    void clear()
    {
        if (this->_fs) {
            pclose(this->_fs);
            this->_fs = 0;
        }
    }

    bool is_ok() const
    {
        return !(this->_is_faild || !this->_fs || feof(this->_fs));
    }

    void dump_out(std::ostream& out) const
    {
        if (!this->is_ok()) {
            return;
        }
        // 逐行获取，即，假设输出的字节流，都是文字，并且以行分割
        char buf[buf_size];
        while (!feof(this->_fs) && fgets(buf, sizeof(buf), this->_fs)) {
            out << buf;
        }
    }

    // 貌似popen 打开的流，不能ftell；fread失败，也无法重试！
    // 直接feof() = true;
    bool to_string(std::string& content) const
    {
        // 逐行获取，即，假设输出的字节流，都是文字，并且以行分割
        if (!this->is_ok()) {
            return false;
        }
        std::ostringstream oss;
        this->dump_out(oss);
        content = oss.str();
        return true;
    }

    bool fetch(std::string& line) const
    {
        // 逐行获取，即，假设输出的字节流，都是文字，并且以行分割
        if (!this->is_ok()) {
            return false;
        }
        char buf[buf_size];
        std::ostringstream oss;
        while (!feof(this->_fs) && fgets(buf, sizeof(buf), this->_fs)) {
            char* p_nl = 0;
            p_nl = strchr(buf, '\n');
            if (p_nl) {
                *p_nl = '\0';
            }
            oss << buf;
            if (feof(this->_fs) || p_nl) {
                break;
            }
        }
        line = oss.str();
        return true;
    }

public:
    std::string _cmd;
    FILE* _fs;
    mode _mod;
    bool _is_faild;
};

std::string PipeRun(const std::string& command_line,
                    const std::string& dir = ".",
                    const std::map<std::string, std::string>& env =
                        std::map<std::string, std::string>());

class StreamPipe {
public:
    StreamPipe(std::ostream& o) : _o(o) {}
    ~StreamPipe() {}
public:
    std::ostream& operator<<(const std::string& cmd)
    {
        Pipe p(cmd, Pipe::pipe_read);
        p.run();
        p.dump_out(this->_o);
        return this->_o;
    }

private:
    std::ostream& _o;
};

class StringPipe : public std::ostringstream {
public:
    StringPipe() {}
    ~StringPipe() {}
public:
    static void streamAddParam(std::ostream& o, const std::string& param);
    static void streamAddParams(std::ostream& o, int argc, char* argv[]);

public:
    // 空的参数，到底有没有意义？
    StringPipe& add(const std::string& param)
    {
        if (!param.empty()) {
            std::ostringstream& oss(*this);
            oss << ' ';
            StringPipe::streamAddParam(*this, param);
        }
        return *this;
    }
    StringPipe& addParams(int argc, char* argv[], int shift = 0)
    {
        // FIXME 是否检查shift的正负号？以及，不能超过argc ?
        if (shift >= 0 && shift < argc) {
            for (int i = shift; i < argc; i++) {
                this->add(argv[i]);
            }
        }
        return *this;
    }
    // NOTE std::ostringstream::clear() 是清楚错误标志！
    // void clear()
    //{
    //    std::ostringstream oss;
    //    std::swap(oss, *this);
    //}

public:
    std::string run()
    {
        std::ostringstream& oss(*this);
        sss::ps::Pipe p(oss.str(), sss::ps::Pipe::pipe_read);
        p.run();
        oss.str("");
        std::string content;
        if (p.to_string(content)) {
            return content;
        }
        else {
            return "";
        }
    }
};
}
}

namespace ext {
inline sss::ps::StreamPipe shell(std::ostream& o)
{
    return sss::ps::StreamPipe(o);
}
}

namespace {
template <typename ReturnType>
inline ReturnType operator<<(std::ostream& o,
                             ReturnType (*func_oper)(std::ostream&))
{
    return func_oper(o);
}
}

#endif /* __PS_HPP_1439992007__ */
