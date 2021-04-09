// config.hpp
#ifndef __CONFIG_HPP_1556974397__
#define __CONFIG_HPP_1556974397__

#include <sss/macro/cppstandard.hpp>

#if SSS_STD_GE_11()
#define SSS_NOEXCEPT noexcept
#else
#define SSS_NOEXCEPT throw()
#endif

// __cplusplus > 201402L

#if SSS_STD_EQ_17()
#define SSS_FALLTHROUGH [[fallthrough]]
#else
#define SSS_FALLTHROUGH __attribute__((fallthrough))
#endif

#endif /* __CONFIG_HPP_1556974397__ */
