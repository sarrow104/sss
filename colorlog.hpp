#ifndef __COLORLOG_HPP_1476324771__
#define __COLORLOG_HPP_1476324771__

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <vector>
#include <cstdio>

// third
#include <sss/bit_operation/bit_operation.h>
#include <sss/Terminal.hpp>
#include <sss/path.hpp>
#include <sss/raw_print.hpp>
#include <sss/time.hpp>

#include <prettyprint.hpp>

// TODO-list
// 增加 file 风格限定；
// 比如fnamemodify-string;
// 比如vim-style shortpath，dirname部分，仅用一到两个字母表示路径；basename完全显示；
// relative-to：相对于某路径；
// 内部，用std::unique_ptr管理这个路径信息；

namespace pretty_print {
template <typename TChar, typename TCharTraits = ::std::char_traits<TChar>>
std::basic_ostream<TChar, TCharTraits>& operator<<(
    std::basic_ostream<TChar, TCharTraits>& o, const TChar* s)
{
    o << sss::raw_string(s);
    return o;
}

template <typename TChar, typename TCharTraits = ::std::char_traits<TChar>>
std::basic_ostream<TChar, TCharTraits>& operator<<(
    std::basic_ostream<TChar, TCharTraits>& o,
    const std::basic_string<TChar, TCharTraits>& s)
{
    o << sss::raw_string(s);
    return o;
}
}

#ifdef COLOG_IMBUE_RAW_SRTING
template <typename TChar, typename TCharTraits = ::std::char_traits<TChar>>
inline std::basic_ostream<TChar, TCharTraits>& operator<<(
    std::basic_ostream<TChar, TCharTraits>& o, const TChar* s)
{
    o << sss::raw_string(s);
    return o;
}

template <typename TChar, typename TCharTraits = ::std::char_traits<TChar>>
inline std::basic_ostream<TChar, TCharTraits>& operator<<(
    std::basic_ostream<TChar, TCharTraits>& o,
    const std::basic_string<TChar, TCharTraits>& s)
{
    o << sss::raw_string(s);
    return o;
}
#endif

// http://stackoverflow.com/questions/12937963/get-local-time-in-nanoseconds
// 关于nano-timer；
// 标准库 std::chrono
// 提供了一系列的时间处理函数。但是，它的处理方式，和我的直觉，不是很试用。
//
// 首先，
// 1. std::chrono 强调的时间间隔；比如now()函数，也是求的是到epoch的间隔。
// 2. 与std::ratio强耦合。内部用std::ratio表示一定比例的间隔单位（24,60,60,
//    1/1000,1/1000,/1000）
//    不过，这个以"比例"，为std::ratio类型的做法，在某些时候，可能会误导人。
//    比如，对于时间的秒，和角度的秒，虽然可能取同一个名字。但这两个单位的
//    数字，其实是不能做加减运算的。但在 C++
//    看来，它们都是分数，可以相加减。
// 3. time_t到char*的相关函数，才考虑了leep-year等计算。std::chrono的count()
// 只是输出按提供的时间间隔单位，所count()到的次数！
// 也就是说，如果我要提供nanoseconds精度，基于std::chrono的话，需要分开处理时间。
// TODO log_info 静态对象；数组，级别的enum；名字，开关，重定向，等等。
namespace sss {
struct timestamp_t {
    enum style {
        DATE_F    = 1u,
        TIME_T    = 2u,
        DOT_MILLS = (1u << 8),
        DOT_MICRS = (3u << 8),
        DOT_NANOS = (7u << 8),
    };
    timestamp_t(int style = DATE_F | TIME_T | DOT_NANOS) : m_style(style) {}
    int m_style;

public:
    void print(std::ostream& o) const
    {
        if (!this->m_style) {
            return;
        }
        auto now =
            ::std::chrono::system_clock::now();  // 这个得到一个 time_point;
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&t);
        char buf[40];
        int offset = 0;
        if (m_style & DATE_F) {
            int cnt = std::strftime(buf, sizeof(buf), "%F ", &tm);
            if (cnt <= 0) {
                return;
            }
            offset += cnt;
        }
        if (m_style & TIME_T) {
            int cnt = std::strftime(buf + offset, sizeof(buf), "%T", &tm);
            if (cnt <= 0) {
                return;
            }
            offset += cnt;
        }
        if (m_style & DOT_NANOS) {
            if (offset > 0 && !(m_style & TIME_T)) {
                o.put(' ');
            }
            int cnt = 0;
            auto duration =
                now.time_since_epoch();  // 从 time_point 得到一个 duration
            switch (m_style & DOT_NANOS) {
                case DOT_MILLS: {
                    std::chrono::milliseconds ms =
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            duration % std::chrono::seconds(1));
                    // 针对 "秒" 为单位，取余数。
                    cnt = std::sprintf(buf + offset, ".%03ld", ms.count());
                } break;

                case DOT_MICRS: {
                    std::chrono::microseconds is =
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            duration % std::chrono::seconds(1));
                    // 针对 "秒" 为单位，取余数。
                    cnt = std::sprintf(buf + offset, ".%06ld", is.count());
                } break;

                case DOT_NANOS: {
                    std::chrono::nanoseconds ns =
                        std::chrono::duration_cast<std::chrono::nanoseconds>(
                            duration % std::chrono::seconds(1));
                    // 针对 "秒" 为单位，取余数。
                    cnt = std::sprintf(buf + offset, ".%09ld", ns.count());
                } break;
            }
            if (cnt <= 0) {
                return;
            }
            offset += cnt;
        }
        if (offset) {
            o.write(buf, offset);
        }
    }
};

inline timestamp_t timestamp(int style = timestamp_t::DATE_F |
                                         timestamp_t::TIME_T |
                                         timestamp_t::DOT_NANOS)
{
    return timestamp_t(style);
}

inline std::ostream& operator<<(std::ostream& o, const timestamp_t& ts)
{
    ts.print(o);
    return o;
}

namespace colog {
enum log_level {
    ll_NONE       = 0,
    ll_DEBUG      = 1,
    ll_INFO       = 2,
    ll_INFO_MASK  = 3,
    ll_WARN       = 4,
    ll_WARN_MASK  = 7,
    ll_ERROR      = 8,
    ll_ERROR_MASK = 15,
    ll_FATAL      = 16,
    ll_FATAL_MASK = 31,
    ll_MASK       = ll_FATAL_MASK,
};
enum log_style {
    ls_NONE        = 0,
    ls_DATE        = (1 << 0),
    ls_TIME        = (1 << 1),
    ls_TIME_MILL   = (1 << 2),
    ls_TIME_MICR   = (2 << 2),
    ls_TIME_NANO   = (3 << 2),
    ls_TIME_MASK   = (3 << 2),
    ls_LEVEL       = (1 << 4),
    ls_LEVEL_SHORT = (2 << 4),
    ls_LEVEL_MASK  = (3 << 4),
    ls_FILE        = (1 << 6),
    ls_FILE_SHORT  = (2 << 6),
    ls_FILE_VIM    = (3 << 6),
    ls_FILE_MASK   = (3 << 6),
    ls_LINE        = (1 << 8),
    ls_FUNC        = (1 << 9),
};

inline log_level operator|(log_level lhs, log_level rhs)
{
    return static_cast<log_level>(static_cast<int>(lhs) |
                                  static_cast<int>(rhs));
}

inline log_level operator&(log_level lhs, log_level rhs)
{
    return static_cast<log_level>(static_cast<int>(lhs) &
                                  static_cast<int>(rhs));
}

inline log_level operator~(log_level lhs)
{
    return static_cast<log_level>(~static_cast<int>(lhs) & static_cast<int>(ll_MASK));
}

inline log_style operator|(log_style lhs, log_style rhs)
{
    return static_cast<log_style>(static_cast<int>(lhs) |
                                  static_cast<int>(rhs));
}

inline int index_of_log_level(log_level ll)
{
    return sss::bit::highest_bit_pos(ll);
}

inline const std::string& get_level_name(log_level ll)
{
    static std::string ll_names[] = {"NONE", "DEBUG", "INFO",
                                     "WARN", "ERROR", "FATAL"};
    return ll_names[colog::index_of_log_level(ll)];
}

class log_env {
    friend void regist(colog::log_level ll, const std::string& fname);
    friend void set_log_elements(colog::log_style ls);
    friend uint32_t get_log_elements();
    friend void set_log_levels(colog::log_level ll);
    friend colog::log_level get_log_levels();

public:
    static log_env& get_env()
    {
        static log_env single_env;
        return single_env;
    }

protected:
    log_env() : m_log_levels(colog::ll_MASK)
    {
        this->m_path_base =
            sss::path::modify_copy(sss::path::getbin(), ":p:h:h");
        this->m_log_style = log_style(ls_TIME | ls_TIME_MILL | ls_FILE_VIM |
                                      ls_LINE | ls_LEVEL_SHORT | ls_FUNC);
        this->m_styles = std::vector<sss::Terminal::style::begin>{
            sss::Terminal::end, sss::Terminal::debug, sss::Terminal::info,
            sss::Terminal::warning, sss::Terminal::error,
            sss::Terminal::style::begin{sss::Terminal::style::FONT_F_WHITE |
                                        sss::Terminal::style::FONT_G_RED |
                                        sss::Terminal::style::FONT_BOLD}};
        this->m_level_listeners[ll_INFO] = std::set<std::ostream*>{&std::cerr};
        this->m_level_listeners[ll_WARN] = std::set<std::ostream*>{&std::cerr};
        this->m_level_listeners[ll_DEBUG] = std::set<std::ostream*>{&std::cerr};
        this->m_level_listeners[ll_ERROR] = std::set<std::ostream*>{&std::cerr};
        this->m_level_listeners[ll_FATAL] = std::set<std::ostream*>{&std::cerr};
    }

private:
    std::string m_path_base;
    std::map<log_level, std::set<std::ostream*>> m_level_listeners;
    // FileName, ref-count, std::ofstream*
    std::map<std::string, std::pair<int, std::shared_ptr<std::ofstream>>>
        m_map2ofstream;

    std::vector<sss::Terminal::style::begin> m_styles;

    log_style m_log_style;
    log_level m_log_levels;

public:
    void set_log_elements(log_style ls) { this->m_log_style = ls; }
    log_style get_log_elements() { return this->m_log_style; }
    void set_log_levels(colog::log_level ll) { this->m_log_levels = (ll & ll_MASK); }

    colog::log_level get_log_levels() const { return this->m_log_levels; }
    void set_level_style(log_level ll, const sss::Terminal::style::begin& s)
    {
        this->m_styles[sss::colog::index_of_log_level(ll)] = s;
    }
    void set_path_base(const std::string& path) { this->m_path_base = path; }
    const sss::Terminal::style::begin& get_level_style(log_level ll)
    {
        return this->m_styles[sss::colog::index_of_log_level(ll)];
    }

    /**
     * @brief regist batch regist log_file to ll_mask
     *
     * @param[in] ll_mask
     * @param[in] log_file
     */
    void regist(log_level ll_mask, const std::string& fname)
    {
        if (!(ll_mask & ll_MASK)) {
            return;
        }
        std::string fpath = sss::path::full_of_copy(fname);
        auto it = this->m_map2ofstream.find(fpath);
        int refcnt = 0;
        std::ofstream* stream = 0;
        if (it != this->m_map2ofstream.end()) {
            refcnt = it->second.first;
            stream = it->second.second.get();
        }
        else {
            std::shared_ptr<std::ofstream> ss = std::make_shared<std::ofstream>(
                fname, std::ios_base::out | std::ios_base::binary);
            if (!ss->good()) {
                return;
            }
            refcnt = 0;
            stream = ss.get();
            it = this->m_map2ofstream.insert(it, {fpath, {refcnt, ss}});
        }
        for (int i = 0; i < 5; ++i) {
            log_level ll = log_level(1 << i);
            if (ll_mask & ll) {
                auto it_list = this->m_level_listeners.find(ll);
                if (it_list != this->m_level_listeners.end()) {
                    if (it_list->second.find(stream) == it_list->second.end()) {
                        it_list->second.insert(stream);
                        refcnt++;
                    }
                }
            }
        }
        it->second.first = refcnt;
    }
    static bool is_enable(log_level ll)
    {
        log_env& e = log_env::get_env();
        return (e.m_log_levels & ll) &&
               e.m_level_listeners.find(ll) != e.m_level_listeners.end();
    }
    static void putmsg(log_level ll, const char* file, int line,
                       const char* func, const std::string& msg)
    {
        log_env& e = log_env::get_env();
        auto it = e.m_level_listeners.find(ll);
        bool is_first_element = true;

        std::ostringstream oss;
        oss.write(e.get_level_style(ll).data(),
                  e.get_level_style(ll).data_len());
        int timestamp_style = 0;
        if (e.m_log_style & ls_DATE) {
            timestamp_style |= timestamp_t::DATE_F;
        }
        if (e.m_log_style & ls_TIME_MASK) {
            timestamp_style |= timestamp_t::TIME_T;
            switch (e.m_log_style & ls_TIME_MASK) {
                case ls_TIME:
                    break;

                case ls_TIME_MILL:
                    timestamp_style |= timestamp_t::DOT_MILLS;
                    break;

                case ls_TIME_MICR:
                    timestamp_style |= timestamp_t::DOT_MICRS;
                    break;

                default:
                    timestamp_style |= timestamp_t::DOT_NANOS;
                    break;
            }
        }
        if (timestamp_style) {
            if (!is_first_element) {
                oss.put(' ');
            }
            is_first_element = false;
            oss << timestamp(timestamp_style);
        }

        if (e.m_log_style & ls_LEVEL_MASK) {
            if (!is_first_element) {
                oss.put(' ');
            }
            is_first_element = false;

            const std::string& name = sss::colog::get_level_name(ll);
            oss.put('[');
            switch (e.m_log_style & ls_LEVEL_MASK) {
                case ls_LEVEL:
                    oss.write(name.c_str(), name.length());
                    for (int i = 5 - name.length(); i > 0; --i) {
                        oss.put(' ');
                    }
                    break;

                default:
                    oss.put(name[0]);
                    break;
            }
            oss.put(']');
        }

        if ((e.m_log_style & ls_FILE_MASK) && file && file[0]) {
            if (!is_first_element) {
                oss.put(' ');
            }
            is_first_element = false;
            switch (e.m_log_style & ls_FILE_MASK) {
                case ls_FILE:
                    oss.write(file, std::strlen(file));
                    break;

                case ls_FILE_SHORT: {
                    if (e.m_path_base.empty()) {
                        oss.write(file, std::strlen(file));
                    }
                    else {
                        std::string short_path = file;
                        short_path =
                            sss::path::relative_to(file, e.m_path_base);
                        oss.write(short_path.c_str(), short_path.length());
                    }
                } break;

                case ls_FILE_VIM: {
                    // /path/to/header.hpp
                    const char* p_cur = file;
                    const char* p_sep =
                        std::strchr(file + 1, sss::path::sp_char);
                    for (;
                         p_cur && p_cur[0];
                         p_cur = p_sep, p_sep = std::strchr(p_sep + 1, sss::path::sp_char))
                    {
                        if (p_sep) {
                            std::ptrdiff_t min_size = p_cur[0] == sss::path::sp_char ? 2 : 1;
                            oss.write(p_cur, std::min(std::distance(p_cur, p_sep), min_size));
                        }
                        else {
                            oss.write(p_cur, std::strlen(p_cur));
                            break;
                        }
                    }
                } break;
            }
        }

        if ((e.m_log_style & ls_LINE) && line > 0) {
            if (!is_first_element) {
                if (e.m_log_style & ls_FILE_MASK) {
                    oss.put(':');
                }
                else {
                    oss.put(' ');
                }
            }
            is_first_element = false;
            // oss << std::setw(4) << line;
            oss << line;
        }
        if ((e.m_log_style & ls_FUNC) && func) {
            if (!is_first_element) {
                oss.put(' ');
            }
            is_first_element = false;
            oss.write(func, std::strlen(func));
            oss.put('(');
            oss.put(')');
        }
        oss.put(':');

#ifdef HIGHLIGHT_MSG
        oss.put(' ');

        oss.write(msg.c_str(), msg.length());
        oss.write(sss::Terminal::end.data(), sss::Terminal::end.data_len());
#else
        oss.write(sss::Terminal::end.data(), sss::Terminal::end.data_len());
        oss.put(' ');

        oss.write(msg.c_str(), msg.length());
#endif
        oss.put('\n');
        std::string msg_final = oss.str();
#ifdef HIGHLIGHT_MSG
        std::string msg_final_normal = msg_final.substr(e.get_level_style(ll).data_len(), msg_final.c_str() +
                                                        e.get_level_style(ll).data_len(),
                                                        msg_final.length() -
                                                        e.get_level_style(ll).data_len() -
                                                        sss::Terminal::end.data_len());
        msg_final_normal.back() = '\n';
#else
        std::string msg_final_normal = msg_final;
        msg_final_normal.erase(0, e.get_level_style(ll).data_len());
        msg_final_normal.erase(msg_final_normal.size() - msg.length() - 2, sss::Terminal::end.data_len());
#endif
        // 如果要忽略高亮——比如输出到外部文件，如何操作？
        // 1. 临时修改换行符
        // 2. 提供偏移和长度，以只打印msg主体。
        // 3. 输出完毕之后，再修改回去
        if (it != e.m_level_listeners.end()) {
            for (auto listen : it->second) {
                if (listen) {
                    if (sss::Terminal::isatty(*listen)) {
                        listen->write(msg_final.c_str(), msg_final.length());
                    }
                    else {
                        listen->write(msg_final_normal.c_str(), msg_final_normal.length());
// #ifdef HIGHLIGHT_MSG
//                         char old = '\n';
//                         std::swap(old,
//                                   msg_final[msg_final.length() -
//                                             sss::Terminal::end.data_len() - 1]);
//                         listen->write(msg_final.c_str() +
//                                           e.get_level_style(ll).data_len(),
//                                       msg_final.length() -
//                                           e.get_level_style(ll).data_len() -
//                                           sss::Terminal::end.data_len());
//                         std::swap(old,
//                                   msg_final[msg_final.length() -
//                                             sss::Terminal::end.data_len() - 1]);
// #else
// #endif
                    }
                }
            }
        }
    }
};

inline void regist(colog::log_level ll_mask, const std::string& fname)
{
    log_env::get_env().regist(ll_mask, fname);
}

inline void set_log_elements(colog::log_style ls)
{
    log_env::get_env().set_log_elements(ls);
}

inline uint32_t get_log_elements()
{
    return log_env::get_env().get_log_elements();
}

inline void set_log_levels(colog::log_level ll)
{
    log_env::get_env().set_log_levels(ll);
}

inline colog::log_level get_log_levels()
{
    return log_env::get_env().get_log_levels();
}

#ifdef SSS_COLOG_TURNOFF
#define COLOG_FATAL(args...)  // NOTE TODO 就算关闭了消息，FATAL，也应该调用
                              // abort()——或者，需要用户自己abort！
#define COLOG_ERROR(args...)
#define COLOG_WARN(args...)
#define COLOG_INFO(args...)
#define COLOG_DEBUG(args...)
#else
// NOTE TODO 如何防止，提供 空参数 时候，g++编译器，出现剩余逗号的问题？__VA_ARGS__ ？
#define COLOG_FATAL(args...) \
    sss::colog::fatal(__FILE__, __LINE__, __func__, ##args)
#define COLOG_ERROR(args...) \
    sss::colog::error(__FILE__, __LINE__, __func__, ##args)
#define COLOG_WARN(args...) \
    sss::colog::warn(__FILE__, __LINE__, __func__, ##args)
#define COLOG_INFO(args...) \
    sss::colog::info(__FILE__, __LINE__, __func__, ##args)
#define COLOG_DEBUG(args...) \
    sss::colog::debug(__FILE__, __LINE__, __func__, ##args)
#endif
/**
 * @brief log_out_impl for empty args-list
 *
 * @param[in] e
 */
inline void log_out_impl(std::ostringstream& e) {}
/**
 * @brief log_out_impl for only-one parameter
 *
 * @tparam T
 * @param[in] e
 * @param[in] v
 */
template <typename T>
inline void log_out_impl(std::ostringstream& e, const T& v)
{
    e << v;
}
/**
 * @brief log_out_impl variant Params unpack with First and Rest
 *
 * @tparam First
 * @tparam Rest
 * @param[in] e
 * @param[in] first
 * @param[in] rest...
 */
template <typename First, typename... Rest>
inline void log_out_impl(std::ostringstream& e, const First& first,
                         const Rest&... rest)
{
    e << first;
    e.put(' ');
    log_out_impl(e, rest...);
}

// prettyprint fuck 了 std::stream::operation<<
// ，因此，如果要要打印raw_string，并且，要正常输出sss::Terminal::xxx.data()，并且要保证输出的整体性的话，
// 就需要调用底层的函数，以避免被fuck。
//
// TODO
// 1. 需要传入 std::ostream& ，然后判断是否tty
// 2. 通过宏，或者是全局变了来完成开关。
// NOTE 除了时间戳，log系统，还应该提供当前的堆栈信息。
// 有两种方案，一种是通过宏参数；另外一种是通过traceback信息——依赖于汇编？
// http://stackoverflow.com/questions/15129089/is-there-a-way-to-dump-stack-trace-with-line-number-from-a-linux-release-binary
// 利用 backtrace()，backtrace_symbols()函数，可以获取到"-g"后的堆栈信息。
// 然后，利用外部程序addr2line，可以对获取到的"地址"，进行解析，以得到源文件位置信息。
// 至于函数名，需要用 c++filt 进行转换。
// 这样来看，还是用宏来处理吧！
template <typename... Args>
inline void log_out(std::ostringstream& e, Args... args)
{
    log_out_impl(e, args...);
}

template <typename... Args>
inline void error(const char* file, const int line, const char* func,
                  const Args&... args)
{
    if (log_env::get_env().is_enable(colog::ll_ERROR)) {
        std::ostringstream e;
        // "[ ERROR ]", sss::Terminal::error,
        log_out(e, args...);
        log_env::putmsg(colog::ll_ERROR, file, line, func, e.str());
    }
}
template <typename... Args>
inline void debug(const char* file, const int line, const char* func,
                  const Args&... args)
{
    if (log_env::get_env().is_enable(colog::ll_DEBUG)) {
        std::ostringstream e;
        log_out(e, args...);
        log_env::putmsg(colog::ll_DEBUG, file, line, func, e.str());
    }
}
template <typename... Args>
inline void info(const char* file, const int line, const char* func,
                 const Args&... args)
{
    if (log_env::get_env().is_enable(colog::ll_INFO)) {
        std::ostringstream e;
        log_out(e, args...);
        log_env::putmsg(colog::ll_INFO, file, line, func, e.str());
    }
}
template <typename... Args>
inline void warn(const char* file, const int line, const char* func,
                 const Args&... args)
{
    if (log_env::get_env().is_enable(colog::ll_WARN)) {
        std::ostringstream e;
        log_out(e, args...);
        log_env::putmsg(colog::ll_WARN, file, line, func, e.str());
    }
}
template <typename... Args>
inline void fatal(const char* file, const int line, const char* func,
                  const Args&... args)
{
    if (log_env::get_env().is_enable(colog::ll_FATAL)) {
        std::ostringstream e;
        log_out(e, args...);
        log_env::get_env().putmsg(colog::ll_FATAL, file, line, func, e.str());
    }
    // FIXME TODO exit(); abort(); ... throw ...
}

inline std::ostream& writeString(std::ostream& o, const char* s, size_t len)
{
    o.write(s, len);
    return o;
}

inline std::ostream& writeString(std::ostream& o, const char* s)
{
    return writeString(o, s, std::strlen(s));
}

// NOTE 防止 std::cout << std::string{"abc"}; 的行为，被修改！用户需要手动 sss::raw_string
// template <typename TChar, typename TCharTraits = ::std::char_traits<TChar>>
// inline std::basic_ostream<TChar, TCharTraits>& operator<<(
//     std::basic_ostream<TChar, TCharTraits>& o, const TChar* s)
// {
//     o << sss::raw_string(s);
//     return o;
// }
// 
// template <typename TChar, typename TCharTraits = ::std::char_traits<TChar>>
// inline std::basic_ostream<TChar, TCharTraits>& operator<<(
//     std::basic_ostream<TChar, TCharTraits>& o,
//     const std::basic_string<TChar, TCharTraits>& s)
// {
//     o << sss::raw_string(s);
//     return o;
// }

}  // namespace colog
}  // namespace sss

#endif /* __COLORLOG_HPP_1476324771__ */
