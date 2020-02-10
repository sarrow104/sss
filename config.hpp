#ifndef __CONFIG_HPP_1556974397__
#define __CONFIG_HPP_1556974397__

#if __cplusplus >= 201103L
#define SSS_NOEXCEPT noexcept
#else
#define SSS_NOEXCEPT throw()
#endif

// __cplusplus > 201402L

#if __cplusplus >= 201703L
#define SSS_FALLTHROUGH [[fallthrough]]
#else
#define SSS_FALLTHROUGH __attribute__((fallthrough))
#endif

#endif /* __CONFIG_HPP_1556974397__ */
