#include "ps.hpp"

#include <sss/util/Escaper.hpp>
#include <sss/util/PostionThrow.hpp>

#ifdef __WIN32__
#include <sss/environ.hpp>
#include <sss/process.hpp>
#else
#include <sss/popenRWE.h>  // for pcloseRWE()

#include <fcntl.h>
#include <unistd.h>

namespace {

int runPipe(int rwepipe[3], const std::string& cmd, const std::string& cwd,
            const std::map<std::string, std::string>& env)
{
    const char* command = cmd.c_str();
    int in[2];
    int out[2];
    int err[2];
    int pid;
    int rc;

    // NOTE 创建一个管道，并将读端，写端的描述符，分别赋值给in[0],in[1]
    // 返回值 >= 0 表示 创建是否成功
    rc = ::pipe(in);
    if (rc < 0) {
        goto error_in;
    }

    rc = ::pipe(out);
    if (rc < 0) {
        goto error_out;
    }

    rc = ::pipe(err);
    if (rc < 0) {
        goto error_err;
    }

    //! http://stackoverflow.com/questions/18437779/do-i-need-to-do-anything-with-a-sigchld-handler-if-i-am-just-using-wait-to-wai
    // signal(SIGCHLD, SIG_IGN);
    // 设置上句之后，子进程的退出，会导致父进程也退出？
    // 太奇怪了

    pid = ::fork();

    // NOTE 管道有一个特点，只能单向传输数据；即，fork()
    // (描述符可以看做整数，并且父
    // 子进程自动继承，并且可以正常访问)之后，必须根据传递方向，来手动关闭某一端；
    // 比如，对于父到子的pipe，父关闭读端；而子则关闭写端；反之亦然。
    //     0(Parent)     1(Child)
    // --- --------- ------------
    // in          w            r
    // out         r            w
    // err         r            w
    if (pid > 0) { /* parent */
        ::close(in[0]);
        ::close(out[1]);
        ::close(err[1]);
        rwepipe[0] = in[1];
        rwepipe[1] = out[0];
        rwepipe[2] = err[0];
        return pid;
    }
    else if (pid == 0) { /* child */
        ::close(in[1]);
        ::close(out[0]);
        ::close(err[0]);

// NOTE
// 关闭子进程本身的标准"输入输出错误"的文件描述符
// 因为，输入输出错误，被重定向到管道中了
#if 0
        ::close(STDIN_FILENO);
        if (!dup(in[0])) {
            ;
        }
        ::close(STDOUT_FILENO);
        if (!dup(out[1])) {
            ;
        }
        ::close(STDERR_FILENO);
        if (!dup(err[1])) {
            ;
        }
#else
        if (in[0] != STDIN_FILENO) {
            dup2(in[0], STDIN_FILENO);
            close(in[0]);
        }
        if (out[1] != STDOUT_FILENO) {
            dup2(out[1], STDOUT_FILENO);
            close(out[1]);
        }
        if (err[1] != STDERR_FILENO) {
            dup2(err[1], STDERR_FILENO);
            close(err[1]);
        }
#endif

        for (std::map<std::string, std::string>::const_iterator it =
                 env.begin();
             it != env.end(); ++it) {
            ::setenv(it->first.c_str(), it->second.c_str(), 1);
        }
        (void)::chdir(cwd.c_str());

        // NOTE 第二个参数，相当于 basename(arg1) (假设，参数序号从1开始）
        // execl("/bin/sh", "sh", "-c", command, NULL);
        execl("/bin/bash", "bash", "-c", command, NULL);
        _exit(1);
    }
    else {
        goto error_fork;
    }

    return pid;

error_fork:
    ::close(err[0]);
    ::close(err[1]);

error_err:
    ::close(out[0]);
    ::close(out[1]);

error_out:
    ::close(in[0]);
    ::close(in[1]);

error_in:
    return -1;
}

}  // namespace
#endif

namespace sss {
namespace ps {
#ifdef __WIN32__
#else
fd_type RWEPipe::fork(const std::string& cmd, const std::string& wd,
                      const std::map<std::string, std::string>& env)
{
    this->pid = ::runPipe(this->rwe, cmd, wd, env);
    return this->pid;
}

int RWEPipe::close() { return ::pcloseRWE(this->pid, this->rwe); }
int RWEPipe::waitpid()
{
    int status;
    return ::waitpid(pid, &status, 0);
}
#endif
std::string PipeRun(const std::string& command_line, const std::string& dir,
                    const std::map<std::string, std::string>& env)
{
#ifdef __WIN32__
    // TODO FIXME
    std::string ret;
    std::string env_str;
    sss::env::dump(env_str, env);
    sss::runPipeCmdLine(command_line, dir, env_str, ret);

    return ret;
#else
    std::ostringstream oss;
    int rwe_pipe[3];
    int pid = ::runPipe(rwe_pipe, command_line, dir, env);

// NOTE 文件描述符的阻塞与非阻塞
#ifdef SSS_NONBLOCKING_FILEDESCRIPTOR
    // set non-block of in_fd
    int fl = ::fcntl(rwe_pipe[1], F_GETFL, 0);
    ::fcntl(rwe_pipe[1], F_SETFL, fl | O_NONBLOCK);
#endif

    char buf[1024];
    while (true) {
        ssize_t cnt = ::read(rwe_pipe[1], buf, sizeof(buf));
        if (cnt < 0) {
            // NOTE 只有当fd被设置为 O_NONBLOCK 的时候，errno才会因为该
            // fd，未准备好，而将errno 设置为EAGAIN == try again
            if (errno != EAGAIN) {
                SSS_POSITION_THROW(std::runtime_error, strerror(errno));
            }
        }
        if (cnt <= 0) {
            break;
        }
        oss.write(buf, cnt);
    }

#ifdef SSS_NONBLOCKING_FILEDESCRIPTOR
    ::fcntl(rwe_pipe[1], F_SETFL, fl);
#endif

    int status;
    int rc = ::waitpid(pid, &status, 0);
    if (rc == -1) {
        return "";
    }

    pcloseRWE(pid, rwe_pipe);
    return oss.str();
#endif
}

// system 在linux和windows下，都是调用自己的命令行解释器，对目录进行解释；
// linux下，自然就是shell；
// shell下，对参数的特殊字符，有两种处理办法；一种是用引号（单双皆可）
// ，来消除歧义；一种，就是用反斜杠进行转义；
//
// 不过，有几个字符比较特殊，应按照需要来转义；
// '>','<','|',还有"&&"，"||"
// 前三个是重新定向管道；后面两个是短路执行；
// 绝大多数情况下（很少有人的文件名，直接是'>',...；），都OK；但来自网
// 络的文件名，确实可能含有'&'字符；
// 即，真有可能需要区分情况，以决定是否用引号包裹，或者转义；
void StringPipe::streamAddParam(std::ostream& oss, const std::string& param)
{
#if 0
            if (param.empty()) {
                return;
            }
            // oss << ' '; 是否补上' '间隔，由上层决定；

            bool is_need_quote =
                param.find(' ') != std::string::npos ||
                param.find('(') != std::string::npos ||
                param.find(')') != std::string::npos;

            if (is_need_quote) {
                oss << '"';
            }
            for (size_t i = 0; i != param.length(); ++i) {
                if ('"' == param[i]) {
                    oss << '\\';
                }
                oss << param[i];
            }
            if (is_need_quote) {
                oss << '"';
            }
#else
    // linux:
    // static sss::util::Escaper esp(R"(\ "'[](){}?*$)"); // -std=c++11
    // windows-cmd需要引号的特殊字符是:
    //     <space>
    //     &()[]{}^=;!'+,`~
    static sss::util::Escaper esp("\\ \"'[](){}?*$&");
    esp.escapeToStream(oss, param);
#endif
}
void StringPipe::streamAddParams(std::ostream& o, int argc, char* argv[])
{
    for (int i = 0; i < argc; ++i) {
        o << " ";
        StringPipe::streamAddParam(o, argv[i]);
    }
}
}
}
