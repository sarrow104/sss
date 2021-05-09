// macro/platform.hpp
#ifndef __PLATFORM_HPP_1599362111__
#define __PLATFORM_HPP_1599362111__

#if defined(_WIN32) || defined(_WIN64)  // this if for _MSC_VER or __MINGW32__
#define SSS_PLT_WINDOWS
#else
#define SSS_PLT_LINUX
#endif

// https://stackoverflow.com/questions/1505582/determining-32-vs-64-bit-in-c
#if defined(_WIN64) || defined(__MINGW64__) || defined(__LP64__) || defined(__x86_64__) || defined(__ppc64__)
#define SSS_PLT_X64
#else
#define SSS_PLT_X32
#endif

#if defined(SSS_PLT_WINDOWS)
#  if defined(SSS_PLT_X32)
#define SSS_PLT_Win32
#  else
#define SSS_PLT_Win64
#  endif
#elif defined(SSS_PLT_LINUX)
#  if defined(SSS_PLT_X32)
#define SSS_PLT_linux32
#  else
#define SSS_PLT_linux64
#  endif
#endif

#if defined(_MSC_VER)
#define SSS_PLT_MSVC
#elif defined(__GNUC__) && !defined(__MINGW32__)
#define SSS_PLT_GCC
#elif defined(__MINGW32__) || defined(__MINGW64__)
#define SSS_PLT_MINGW
#endif

#endif /* __PLATFORM_HPP_1599362111__ */
