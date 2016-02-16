#ifndef __POSTIONTHROW_HPP_1452068734__
#define __POSTIONTHROW_HPP_1452068734__

#include <sstream>

#ifdef SSS_POSTION_THROW
#undef SSS_POSTION_THROW
#endif

#define SSS_POSTION_THROW(type, msg) \
{                       \
    std::ostringstream oss;       \
    oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
    throw type(oss.str()); \
}

#ifdef SSS_POSTION_THROW_1
#undef SSS_POSTION_THROW_1
#endif
#define SSS_POSTION_THROW_1(type, msg, v1) \
{                       \
    std::ostringstream oss;       \
    oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
    throw type(oss.str(), v1); \
}

#ifdef SSS_POSTION_THROW_2
#undef SSS_POSTION_THROW_2
#endif
#define SSS_POSTION_THROW_2(type, msg, v1, v2) \
{                       \
    std::ostringstream oss;       \
    oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
    throw type(oss.str(), v1, v2); \
}

#ifdef SSS_POSTION_THROW_3
#undef SSS_POSTION_THROW_3
#endif
#define SSS_POSTION_THROW_3(type, msg, v1, v2, v3) \
{                       \
    std::ostringstream oss;       \
    oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
    throw type(oss.str(), v1, v2, v3); \
}

#ifdef SSS_POSTION_THROW_4
#undef SSS_POSTION_THROW_4
#endif
#define SSS_POSTION_THROW_4(type, msg, v1, v2, v3, v4) \
{                       \
    std::ostringstream oss;       \
    oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
    throw type(oss.str(), v1, v2, v3, v4); \
}

#ifdef SSS_POSTION_THROW_5
#undef SSS_POSTION_THROW_5
#endif
#define SSS_POSTION_THROW_5(type, msg, v1, v2, v3, v4, v5) \
{                       \
    std::ostringstream oss;       \
    oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
    throw type(oss.str(), v1, v2, v3, v4, v5); \
}

#ifdef SSS_POSTION_THROW_6
#undef SSS_POSTION_THROW_6
#endif
#define SSS_POSTION_THROW_6(type, msg, v1, v2, v3, v4, v5, v6) \
{                       \
    std::ostringstream oss;       \
    oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
    throw type(oss.str(), v1, v2, v3, v4, v5, v6); \
}
#ifdef SSS_POSTION_THROW_7
#undef SSS_POSTION_THROW_7
#endif
#define SSS_POSTION_THROW_7(type, msg, v1, v2, v3, v4, v5, v6, v7) \
{                       \
    std::ostringstream oss;       \
    oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
    throw type(oss.str(), v1, v2, v3, v4, v5, v6, v7); \
}
#ifdef SSS_POSTION_THROW_8
#undef SSS_POSTION_THROW_8
#endif
#define SSS_POSTION_THROW_8(type, msg, v1, v2, v3, v4, v5, v6, v7, v8) \
{                       \
    std::ostringstream oss;       \
    oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
    throw type(oss.str(), v1, v2, v3, v4, v5, v6, v7, v8); \
}
#ifdef SSS_POSTION_THROW_9
#undef SSS_POSTION_THROW_9
#endif
#define SSS_POSTION_THROW_9(type, msg, v1, v2, v3, v4, v5, v6, v7, v8, v9) \
{                       \
    std::ostringstream oss;       \
    oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
    throw type(oss.str(), v1, v2, v3, v4, v5, v6, v7, v8, v9); \
}


#endif /* __POSTIONTHROW_HPP_1452068734__ */
