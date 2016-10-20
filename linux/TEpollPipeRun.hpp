#ifndef __TEPOLLPIPERUN_HPP_1475896415__
#define __TEPOLLPIPERUN_HPP_1475896415__

#include <iosfwd>
#include <vector>

#include <sss/linux/epoll.hpp>
#include <sss/mpl/sfinae.hpp>
#include <sss/ps.hpp>

namespace sss {
namespace epoll {

GEN_MEMBERFUNC_TRAITS(on_write_stdin, int, (char* buf, size_t len));
GEN_MEMBERFUNC_TRAITS(on_read_stdout, int, (const char* buf, size_t len));
GEN_MEMBERFUNC_TRAITS(on_read_stderr, int, (const char* buf, size_t len));
// bool (::*)() const;
GEN_MEMBERFUNC_TRAITS(good, bool, (void)const);

template <typename TProcessor, size_t buf_init_size = 1024u>
class TEpollPipeRun : public MEMBERFUNC_TRAITS(on_write_stdin, TProcessor),
                      public MEMBERFUNC_TRAITS(on_read_stdout, TProcessor),
                      public MEMBERFUNC_TRAITS(on_read_stderr, TProcessor),
                      public MEMBERFUNC_TRAITS(good, TProcessor) {
public:
    USING_MEMBERFUNC_TRAITS_VALUE(on_write_stdin, TProcessor);
    USING_MEMBERFUNC_TRAITS_VALUE(on_read_stdout, TProcessor);
    USING_MEMBERFUNC_TRAITS_VALUE(on_read_stderr, TProcessor);
    USING_MEMBERFUNC_TRAITS_VALUE(good, TProcessor);

public:
    TProcessor& m_processor;
    std::string m_cmd;
    bool m_use_delimiter;
    char m_delimiter;

public:
    TEpollPipeRun(const std::string& cmd, TProcessor& p)
        : m_cmd(cmd), m_processor(p), m_use_delimiter(false), m_delimiter('\0')
    {
    }
    ~TEpollPipeRun() {}
public:
    void enable_delimiter(char delimiter)
    {
        this->m_use_delimiter = true;
        this->m_delimiter = delimiter;
    }
    void disable_delimiter() { this->m_use_delimiter = false; }
protected:
    template <typename Pu, bool enabl>
    struct Call_on_read_stderr;
    template <typename Pu>
    struct Call_on_read_stderr<Pu, true> {
        static int call(Pu& p, const char* buf, size_t len)
        {
            return p.on_read_stderr(buf, len);
        }
    };

    template <typename Pu>
    struct Call_on_read_stderr<Pu, false> {
        static int call(Pu&, const char*, size_t) { return 0; }
    };

    template <typename Pu, bool enabl>
    struct Call_on_read_stdout;
    template <typename Pu>
    struct Call_on_read_stdout<Pu, true> {
        static int call(Pu& p, const char* buf, size_t len)
        {
            return p.on_read_stdout(buf, len);
        }
    };
    template <typename Pu>
    struct Call_on_read_stdout<Pu, false> {
        static int call(Pu&, const char*, size_t) { return 0; }
    };

    template <typename Pu, bool enabl>
    struct Call_on_in_write;
    template <typename Pu>
    struct Call_on_in_write<Pu, true> {
        static int call(Pu& p, char* buf, size_t len)
        {
            return p.on_write_stdin(buf, len);
        }
    };
    template <typename Pu>
    struct Call_on_in_write<Pu, false> {
        static int call(Pu&, const char*, size_t) { return 0; }
    };

    template <typename Pu, bool enabl>
    struct Call_good;
    template <typename Pu>
    struct Call_good<Pu, true> {
        static bool call(Pu& p)
        {
            return p.good();
        }
    };
    template <typename Pu>
    struct Call_good<Pu, false> {
        static bool call(Pu&) { return true; }
    };

public:
    int call_on_in_write(char* buf, size_t len)
    {
        return Call_on_in_write<TProcessor, has_on_write_stdin>::call(
            m_processor, buf, len);
    }

    int call_on_read_stdout(const char* buf, size_t len)
    {
        return Call_on_read_stdout<TProcessor, has_on_read_stdout>::call(
            m_processor, buf, len);
    }

    int call_on_read_stderr(const char* buf, size_t len)
    {
        return Call_on_read_stderr<TProcessor, has_on_read_stderr>::call(
            m_processor, buf, len);
    }

    bool call_good()
    {
        return Call_good<TProcessor, has_good>::call(m_processor);
    }

public:
    typedef int (TEpollPipeRun<TProcessor>::*out_caller_t)(char* buf,
                                                           size_t len);
    void process_out_filedescriptor(struct epoll_event& ev, bool& is_child_over,
                                    bool has_on_this, int fd,
                                    out_caller_t caller, std::vector<char>& buf)
    {
        if (!sss::linux::epoll::is_event_any_match(ev, sss::linux::epoll::MASK_OUT)) {
            is_child_over = true;
            return;
        }
        if (!has_on_this) {
            return;
        }
        // char buf[1024];
        // NOTE FIXME
        // "in"的写入，其实不是很适合用这个循环来处理。
        // 为什么呢？
        // 因为，很多适合，写入不仅仅根据是否可写入，还根据，当前的输出，来决定的！
        // 比如，各种提示符。
        int possible_len = 0u;
        int offset = 0;
        do {
            // NOTE
            // 往自进程的stdin管道，写入数据。虽然是定长的buf，但是为了能够适应任意
            // 长度的写入，特意用返回值，和提供的buf长度，共同来判断，是否还有后续
            // 的字节。
            //
            // 如果，返回值大于buf长度，则说明还有后续。
            // 反之(<=)，则表示当次，可以获取完，需要的输入信息。

            possible_len =
                (this->*caller)(&buf[0] + offset, int(buf.size()) - 1 - offset);
            possible_len = std::max(possible_len, 0);
            if (possible_len <= int(buf.size()) - 1 - offset) {
                offset += possible_len;
                buf.resize(offset + 1);
                break;
            }
            else {
                int size_to_extend_to = offset + possible_len + 1;
                offset += buf.size() - 1;
                buf.resize(size_to_extend_to);
            }
        } while (true);
        buf[offset] = '\0';
        if (::write(fd, buf.data(), offset) == -1) {
            std::ostringstream oss;
            oss << __FILE__ << ":" << __LINE__
                << ":write():" << ::strerror(errno);
            is_child_over = true;
            throw std::runtime_error(oss.str());
        }
    }
    typedef int (TEpollPipeRun<TProcessor>::*caller_t)(const char* buf,
                                                       size_t len);
    void process_in_filedescriptor(struct epoll_event& ev, bool& is_child_over,
                                   bool has_on_this, int fd, caller_t caller,
                                   std::vector<char>& buf)
    {
        if (!sss::linux::epoll::is_event_any_match(ev, sss::linux::epoll::MASK_IN)) {
            is_child_over = true;
            return;
        }
        // NOTE
        // 关于buf的处理，也是一个问题。
        // 因为是定长的buf，而文本消息，比如utf8，是不定长的。
        // 因此需要"断字"。要保证正确性，要么底层这里，需要串接buf，再交给上层处理。
        // 比如这里的std::string::append();
        // 要么，上层自带一个部分，用来缓存没有处理到的"字节"。
        int cnt = 0;
        int offset = 0;
        while ((cnt = ::read(fd, &buf[0] + offset, buf.size() - 1 - offset)) >
               0) {
            bool break_here = (cnt < int(buf.size()) - 1 - offset);
            // if (has_on_this) {
            offset += cnt;
            if (offset >= int(buf.size() - 1)) {
                size_t new_size =
                    std::max(buf.size() + (buf.size() >> 1), buf.size() + 1);
                buf.resize(new_size, '\0');
            }
            // }
            if (break_here) {
                break;
            }
        }
        if (cnt == 0) {
            is_child_over = true;
        }
        else if (cnt <= -1) {
            if (errno && errno != EAGAIN) {
                std::ostringstream oss;
                oss << __FILE__ << ":" << __LINE__ << ":" << ::strerror(errno);
                is_child_over = true;
                throw std::runtime_error(oss.str());
            }
        }

        if (offset > 0) {
            buf[offset] = '\0';
            buf.resize(offset + 1);
            if (m_use_delimiter) {
                const char* last_newline_pos = &buf[0];
                char old_sep = '\0';
                do {
                    const char* current_newline_pos =
                        std::strchr(last_newline_pos, this->m_delimiter);
                    if (!current_newline_pos) {
                        current_newline_pos = &buf[offset - 1];
                    }
                    else {
                        old_sep = current_newline_pos[1];
                        const_cast<char*>(current_newline_pos)[1] = '\0';
                    }
                    (this->*caller)(last_newline_pos,
                                    current_newline_pos - last_newline_pos + 1);
                    last_newline_pos = current_newline_pos + 1;
                    if (last_newline_pos - &buf[0] < offset) {
                        const_cast<char*>(last_newline_pos)[0] = old_sep;
                    }
                } while (last_newline_pos < &buf[0] + offset);
            }
            else {
                (this->*caller)(buf.data(), offset);
            }
        }
    }

    void loop(int timeout = -1)
    {
        // TODO
        sss::ps::RWEPipe pipe;
        pipe.fork(m_cmd);
        std::string current_status;
        sss::linux::epoll ep;
        ep.timeout(timeout);

        if (has_on_write_stdin) {
            ep.addFd(pipe[0],
                     sss::linux::epoll::MASK_OUT | sss::linux::epoll::MASK_HUP |
                         sss::linux::epoll::MASK_ET,
                     0);
        }

        if (has_on_read_stdout) {
            ep.addFd(pipe[1],
                     sss::linux::epoll::MASK_IN | sss::linux::epoll::MASK_HUP |
                         sss::linux::epoll::MASK_ET,
                     0);
        }

        if (has_on_read_stderr) {
            ep.addFd(pipe[2],
                     sss::linux::epoll::MASK_IN | sss::linux::epoll::MASK_HUP |
                         sss::linux::epoll::MASK_ET,
                     0);
        }

        std::vector<struct epoll_event> events;

        int ep_wait_cnt = 0;
        bool is_child_over = false;
        try {
            // std::string buf;
            std::vector<char> buf;
            buf.resize(buf_init_size, '\0');
            while (!is_child_over && this->call_good() && (ep_wait_cnt = ep.wait(events, 0)) > 0) {
                buf.resize(buf.capacity(), '\0');
                for (int i = 0; !is_child_over && i < int(events.size()); ++i) {
                    if (has_on_write_stdin &&
                        sss::linux::epoll::is_event_eq_fd(events[i], pipe[0])) {
                        process_out_filedescriptor(
                            events[i], is_child_over, has_on_write_stdin,
                            pipe[0],
                            &TEpollPipeRun<TProcessor>::call_on_in_write, buf);
                    }
                    else if (has_on_read_stdout &&
                             sss::linux::epoll::is_event_eq_fd(events[i], pipe[1])) {
                        process_in_filedescriptor(
                            events[i], is_child_over, has_on_read_stdout,
                            pipe[1],
                            &TEpollPipeRun<TProcessor>::call_on_read_stdout,
                            buf);
                    }
                    else if (has_on_read_stderr &&
                             sss::linux::epoll::is_event_eq_fd(events[i], pipe[2])) {
                        process_in_filedescriptor(
                            events[i], is_child_over, has_on_read_stderr,
                            pipe[2],
                            &TEpollPipeRun<TProcessor>::call_on_read_stderr,
                            buf);
                    }
                }
            }
            // wait-timeout
            if (!is_child_over) {
                pipe.close();
            }
        }
        catch (...) {
            pipe.close();
            throw;
        }

        if (!is_child_over) {
            pipe.waitpid();
        }
    }
};
}  // namespace epoll
}  // namespace sss

#endif /* __TEPOLLPIPERUN_HPP_1475896415__ */
