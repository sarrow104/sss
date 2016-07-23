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
    // �����Ͼ�֮���ӽ��̵��˳����ᵼ�¸�����Ҳ�˳���
    // ̫�����

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

        // system ��linux��windows�£����ǵ����Լ��������н���������Ŀ¼���н��ͣ�
        // linux�£���Ȼ����shell��
        // shell�£��Բ����������ַ��������ִ���취��һ���������ţ���˫�Կɣ�
        // �����������壻һ�֣������÷�б�ܽ���ת�壻
        //
        // �������м����ַ��Ƚ����⣬Ӧ������Ҫ��ת�壻
        // '>','<','|',����"&&"��"||"
        // ǰ���������¶���ܵ������������Ƕ�·ִ�У�
        // �����������£��������˵��ļ�����ֱ����'>',...��������OK����������
        // ����ļ�����ȷʵ���ܺ���'&'�ַ���
        // �������п�����Ҫ����������Ծ����Ƿ������Ű���������ת�壻
        void StringPipe::streamAddParam(std::ostream& oss, const std::string& param)
        {
#if 0
            if (param.empty()) {
                return;
            }
            // oss << ' '; �Ƿ���' '��������ϲ������

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
