#include "ps.hpp"

#include <sss/util/Escaper.hpp>

#ifdef __WIN32__
#include <sss/process.hpp>
#include <sss/environ.hpp>
#else
#include <sss/popenRWE.h>

namespace  {

int runPipe(int rwepipe[3],
            const std::string& cmd,
            const std::string& cwd,
            const std::map<std::string, std::string>& env)
{
    const char * command = cmd.c_str();
    int in[2];
    int out[2];
    int err[2];
    int pid;
    int rc;

    rc = ::pipe(in);
    if (rc<0) {
        goto error_in;
    }

    rc = ::pipe(out);
    if (rc<0) {
        goto error_out;
    }

    rc = ::pipe(err);
    if (rc<0) {
        goto error_err;
    }

    //! http://stackoverflow.com/questions/18437779/do-i-need-to-do-anything-with-a-sigchld-handler-if-i-am-just-using-wait-to-wai
    // signal(SIGCHLD, SIG_IGN);
    // 设置上句之后，子进程的退出，会导致父进程也退出？
    // 太奇怪了

    pid = ::fork();

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
        ::close(0);
        if(!dup(in[0])) {
            ;
        }
        ::close(1);
        if(!dup(out[1])) {
            ;
        }
        ::close(2);
        if(!dup(err[1])) {
            ;
        }

        for (std::map<std::string, std::string>::const_iterator it = env.begin();
             it != env.end();
             ++it)
        {
            ::setenv(it->first.c_str(), it->second.c_str(), 1);
        }
        (void)::chdir(cwd.c_str());

        execl( "/bin/sh", "sh", "-c", command, NULL );
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

} // namespace
#endif

namespace sss
{
    namespace ps
    {

        std::string PipeRun(const std::string& command_line,
                            const std::string& dir,
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
            char buf[1024];
            while(true) {
                size_t cnt = ::read(rwe_pipe[1], buf, sizeof(buf));
                if (!cnt) {
                    break;
                }
                oss.write(buf, cnt);
            }
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
            // static sss::util::Escaper esp(R"(\ "'[](){}?*$)"); // -std=c++11
            static sss::util::Escaper esp("\\ \"'[](){}?*$&");
            esp.escapeToStream(oss, param);
#endif
        }
        void StringPipe::streamAddParams(std::ostream& o, int argc, char *argv[])
        {
            for (int i = 0; i < argc; ++i) {
                o << " ";
                StringPipe::streamAddParam(o, argv[i]);
            }
        }
    }
}
