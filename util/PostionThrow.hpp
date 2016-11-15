#ifndef __POSTIONTHROW_HPP_1452068734__
#define __POSTIONTHROW_HPP_1452068734__

#include <cstring>
#include <sstream>

#ifdef SSS_POSTION_THROW
#undef SSS_POSTION_THROW
#endif

#if defined(__cplusplus) && __cplusplus >= 201103L
namespace {

inline void log_out_impl(std::ostringstream&, bool) {}
/**
 * @brief log_out_impl for only-one parameter
 *
 * @tparam T
 * @param[in] e
 * @param[in] v
 */
template <typename T>
inline void log_out_impl(std::ostringstream& e, bool, const T& v)
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
inline void log_out_impl(std::ostringstream& e, bool padding,
                         const First& first, const Rest&... rest)
{
    e << first;
    if (padding) {
        e.put(' ');
    }
    log_out_impl(e, padding, rest...);
}

template <typename... Args>
inline void log_out(std::ostringstream& e, bool padding, const char* file,
                    int line, const char* func, Args... args)
{
    e.write(file, std::strlen(file))
        .put(':')
        .
        operator<<(line)
        .put(' ')
        .write(func, std::strlen(func))
        .write("():", 3);
    log_out_impl(e, padding, args...);
}
}
namespace sss {
template <typename... Args>
std::string rope_string(const Args&... args)
{
    std::ostringstream oss;
    log_out_impl(oss, false, args...);
    return oss.str();
}
}  // namespace sss
#define SSS_POSTION_THROW(type, args...)                           \
    do {                                                           \
        std::ostringstream oss;                                    \
        log_out(oss, false, __FILE__, __LINE__, __func__, ##args); \
        throw type(oss.str());                                     \
    } while (false)
#define SSS_POSTION_PADDING_THROW(type, args...)                  \
    do {                                                          \
        std::ostringstream oss;                                   \
        log_out(oss, true, __FILE__, __LINE__, __func__, ##args); \
        throw type(oss.str());                                    \
    } while (false)
#define SSS_POSTION_ARGS_THROW(type, msg, args...)            \
    do {                                                      \
        std::ostringstream oss;                               \
        oss << __FILE__ << ":" << __LINE__ << " " << __func__ \
            << "(): " << msg;                                 \
        throw type(oss.str(), ##args);                        \
    } while (false)
#else
#define SSS_POSTION_THROW(type, msg, args...)                 \
    do {                                                      \
        std::ostringstream oss;                               \
        oss << __FILE__ << ":" << __LINE__ << " " << __func__ \
            << "(): " << msg;                                 \
        throw type(oss.str(), ##args);                        \
    } while (false)
#endif

#endif /* __POSTIONTHROW_HPP_1452068734__ */
