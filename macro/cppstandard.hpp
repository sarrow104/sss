// macro/cppstandard.hpp
#ifndef __CPPSTANDARD_HPP_1599367472__
#define __CPPSTANDARD_HPP_1599367472__

// https://stackoverflow.com/questions/26089319/is-there-a-standard-definition-for-cplusplus-in-c14
// https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=vs-2019
// https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/preprocessor/macros/predefined/__cplusplus

#define SSS_STD_VER11 (201103L)
#define SSS_STD_VER14 (201402L)
#define SSS_STD_VER17 (201402L)

#if defined(__cplusplus)
#define SSS_STD_GE_11() (__cplusplus >= SSS_STD_VER11)
#define SSS_STD_GE_14() (__cplusplus >= SSS_STD_VER14)
#define SSS_STD_GE_17() (__cplusplus >= SSS_STD_VER17)

#define SSS_STD_EQ_11() (__cplusplus == SSS_STD_VER11)
#define SSS_STD_EQ_14() (__cplusplus == SSS_STD_VER14)
#define SSS_STD_EQ_17() (__cplusplus == SSS_STD_VER17)
#else
#define SSS_STD_GE_11() (0)
#define SSS_STD_GE_14() (0)
#define SSS_STD_GE_17() (0)

#define SSS_STD_EQ_11() (0)
#define SSS_STD_EQ_14() (0)
#define SSS_STD_EQ_17() (0)
#endif

#endif /* __CPPSTANDARD_HPP_1599367472__ */
