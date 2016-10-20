#ifndef __PROCESSERS_HPP_1475716888__
#define __PROCESSERS_HPP_1475716888__

#include <sss/time.hpp>

#include <iosfwd>
#include <sstream>
#include <string>

#include <fstream>

//#include <sss/raw_print.hpp>

namespace sss {
namespace PipeProcessor {

struct DummyProcessor {
};

struct EchoProcessor {
    // NOTE 需要一个状态机，才能很好地处理stdin的写入。
    EchoProcessor() : m_on_in_write_called_times(0) {}
    // int on_write_stdin(char * buf, size_t len)
    // {
    //     const char * msg = "hello world";
    //     size_t msg_len = std::strlen(msg);
    //     size_t to_cp_len = std::min(len, msg_len);
    //     std::memmove(buf, msg, to_cp_len);
    //     m_on_in_write_called_times++;
    //     return msg_len;
    // }
    int on_read_stdout(const char* buf, size_t len)
    {
        std::cout.write(buf, len);
        std::cout.flush();
        return 0;
    }
    int on_read_stderr(const char* buf, size_t len)
    {
        std::cerr.write(buf, len);
        std::cerr.flush();
        return 0;
    }
    void init_command_line(const std::string& cml);  // fork时，命令行参数。

    int m_on_in_write_called_times;
};

struct StdOutOnly {
    std::ostringstream m_out;
    int on_read_stdout(const char * buf, size_t len) { this->m_out.write(buf, len); return len; }
    std::string get_stdout() const { return m_out.str(); }
};
struct StdErrOnly {
    std::ostringstream m_err;
    int on_read_stderr(const char * buf, size_t len) { this->m_err.write(buf, len); return len; }
    std::string get_stderr() const { return m_err.str(); }
};
struct StdoutAndStderr : public StdOutOnly, public StdErrOnly {
};
}  // namespace PipeProcessor
}  // namespace sss

#endif /* __PROCESSERS_HPP_1475716888__ */
