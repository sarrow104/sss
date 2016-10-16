#ifndef __POSTIONTHROW_HPP_1452068734__
#define __POSTIONTHROW_HPP_1452068734__

#include <sstream>

#ifdef SSS_POSTION_THROW
#undef SSS_POSTION_THROW
#endif

#if defined(__cplusplus) && __cplusplus >= 201103L
namespace {

inline void log_out_impl(std::ostringstream&) {}
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

template <typename... Args>
inline void log_out(std::ostringstream& e, const char* file, int line,
                    const char* func, Args... args)
{
    e << file << ":" << line << " " << func << "() ";
    log_out_impl(e, args...);
}
}
#define SSS_POSTION_THROW(type, args...)                    \
    do {                                                    \
        std::ostringstream oss;                             \
        log_out(oss, __FILE__, __LINE__, __func__, ##args); \
        throw type(oss.str());                              \
    } while (false)
#else
#define SSS_POSTION_THROW(type, msg)                                          \
    do {                                                                      \
        std::ostringstream oss;                                               \
        oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
        throw type(oss.str());                                                \
    } while (false)

#ifdef SSS_POSTION_THROW_1
#undef SSS_POSTION_THROW_1
#endif
#define SSS_POSTION_THROW_1(type, msg, v1)                                    \
    do {                                                                      \
        std::ostringstream oss;                                               \
        oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
        throw type(oss.str(), v1);                                            \
    } while (false)

#ifdef SSS_POSTION_THROW_2
#undef SSS_POSTION_THROW_2
#endif
#define SSS_POSTION_THROW_2(type, msg, v1, v2)                                \
    do {                                                                      \
        std::ostringstream oss;                                               \
        oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
        throw type(oss.str(), v1, v2);                                        \
    } while (false)

#ifdef SSS_POSTION_THROW_3
#undef SSS_POSTION_THROW_3
#endif
#define SSS_POSTION_THROW_3(type, msg, v1, v2, v3)                            \
    do {                                                                      \
        std::ostringstream oss;                                               \
        oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
        throw type(oss.str(), v1, v2, v3);                                    \
    } while (false)

#ifdef SSS_POSTION_THROW_4
#undef SSS_POSTION_THROW_4
#endif
#define SSS_POSTION_THROW_4(type, msg, v1, v2, v3, v4)                        \
    do {                                                                      \
        std::ostringstream oss;                                               \
        oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
        throw type(oss.str(), v1, v2, v3, v4);                                \
    } while (false)

#ifdef SSS_POSTION_THROW_5
#undef SSS_POSTION_THROW_5
#endif
#define SSS_POSTION_THROW_5(type, msg, v1, v2, v3, v4, v5)                    \
    do {                                                                      \
        std::ostringstream oss;                                               \
        oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
        throw type(oss.str(), v1, v2, v3, v4, v5);                            \
    } while (false)

#ifdef SSS_POSTION_THROW_6
#undef SSS_POSTION_THROW_6
#endif
#define SSS_POSTION_THROW_6(type, msg, v1, v2, v3, v4, v5, v6)                \
    do {                                                                      \
        std::ostringstream oss;                                               \
        oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
        throw type(oss.str(), v1, v2, v3, v4, v5, v6);                        \
    } while (false)
#ifdef SSS_POSTION_THROW_7
#undef SSS_POSTION_THROW_7
#endif
#define SSS_POSTION_THROW_7(type, msg, v1, v2, v3, v4, v5, v6, v7)            \
    do {                                                                      \
        std::ostringstream oss;                                               \
        oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
        throw type(oss.str(), v1, v2, v3, v4, v5, v6, v7);                    \
    } while (false)
#ifdef SSS_POSTION_THROW_8
#undef SSS_POSTION_THROW_8
#endif
#define SSS_POSTION_THROW_8(type, msg, v1, v2, v3, v4, v5, v6, v7, v8)        \
    do {                                                                      \
        std::ostringstream oss;                                               \
        oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
        throw type(oss.str(), v1, v2, v3, v4, v5, v6, v7, v8);                \
    } while (false)
#ifdef SSS_POSTION_THROW_9
#undef SSS_POSTION_THROW_9
#endif
#define SSS_POSTION_THROW_9(type, msg, v1, v2, v3, v4, v5, v6, v7, v8, v9)    \
    do {                                                                      \
        std::ostringstream oss;                                               \
        oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
        throw type(oss.str(), v1, v2, v3, v4, v5, v6, v7, v8, v9);            \
    } while (false)
#endif

#endif /* __POSTIONTHROW_HPP_1452068734__ */
