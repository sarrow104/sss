#ifndef __EPOLLPIPERUN_HPP_1476078393__
#define __EPOLLPIPERUN_HPP_1476078393__

#include <cstdlib>
#include <cstring>
#include <string>

#include <sss/linux/TEpollPipeRun.hpp>

namespace sss {
namespace processors {
    
struct EchoProcessor2 {
    explicit EchoProcessor2(std::string& resved_out, std::string& resved_err,
                            int mode, bool echo = false,
                            const std::string& in = "")
        : m_stdout(resved_out),
          m_stderr(resved_err),
          m_mode(mode),
          m_is_echo(echo),
          m_in(in)
    {
    }
    int on_write_stdin(char* buf, size_t len)
    {
        size_t need_write_len = m_in.length();
        if (!m_in.empty()) {
            size_t to_write_len = std::min(len, need_write_len);
            std::memmove(buf, m_in.c_str(), to_write_len);
            m_in = m_in.substr(to_write_len);
        }
        return need_write_len;
    }
    int on_read_stdout(const char* buf, size_t len)
    {
        if (m_mode & 0x01u) {
            this->m_stdout.append(buf, len);
        }
        if (m_is_echo) {
            std::fwrite(buf, len, 1, stdout);
            std::fflush(stdout);
        }
        return len;
    }
    int on_read_stderr(const char* buf, size_t len)
    {
        if (m_mode & 0x02u) {
            this->m_stderr.append(buf, len);
        }
        if (m_is_echo) {
            std::fwrite(buf, len, 1, stderr);
            std::fflush(stderr);
        }
        return len;
    }
    bool good() const {
        return true;
    }
    std::string& m_stdout;
    std::string& m_stderr;
    int m_mode;
    int m_is_echo;
    std::string m_in;
};
} // namespace processors

}  // namespace

namespace sss {
namespace epoll {
/**
 * @brief rwe_pipe_run
 *
 * @param cmd
 * @param mode
 * @param in
 *
 * @return
 * 这个应该如何设计？
 * 仅针对 echo "some variable " | app1 | app2 这种管道传递方式的执行，
 * 如何设计接口？
 * 对于popen函数来说，默认获得的，只有STDOUT_FILENO 中的内容，而STDERR_FILENO
 * 中的则没有。
 * bash中也类似，默认通过管道操作符 |，只传递STDOUT_FILENO；如果要同时传递
 * STDERR_FILENO的内容，需要 2>&1 这样来使用。
 *
 * 同样的，还应该支持设定actor——，以便设定如何响应截获到的输出。
 */
std::pair<std::string, std::string> rwe_pipe_run(const std::string& cmd,
                                                 int mode, bool echo = false,
                                                 const std::string& in = "")
{
    // TODO
    // 用类本身的功能，来完成echo，in等功能。不过，in，比较鸡肋。理论上，
    // 需要复杂的状态机，才能完成实际的交互。
    std::string resved_out;
    std::string resved_err;
    sss::processors::EchoProcessor2 p(resved_out, resved_err, mode, echo, in);
    sss::epoll::TEpollPipeRun<sss::processors::EchoProcessor2> myEp(cmd, p);
    myEp.loop();
    return {resved_out, resved_err};
}
}  // namespace epoll
}  // namespace sss

#endif /* __EPOLLPIPERUN_HPP_1476078393__ */
