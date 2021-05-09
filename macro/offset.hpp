#ifndef  __OFFSET_HPP_1363875578__
#define  __OFFSET_HPP_1363875578__

#define FLD_SIZEOF(s,m) sizeof(((s *)0)->m)
#define OFFSETOF(s,m)   ((size_t)&(((s*)0)->m))

#ifdef __cplusplus
#ifdef  _WIN64
#       define OFFSETOF(s,m)   (size_t)( (ptrdiff_t)&reinterpret_cast<const volatile char&>((((s *)0)->m)) )
#else
#       define OFFSETOF(s,m)   (size_t)&reinterpret_cast<const volatile char&>((((s *)0)->m))
#endif

#else

#ifdef  _WIN64
#       define OFFSETOF(s,m)   (size_t)( (ptrdiff_t)&(((s *)0)->m) )
#else
#       define OFFSETOF(s,m)   (size_t)&(((s *)0)->m)
#endif

#endif

#endif  /* __OFFSET_HPP_1363875578__ */
//! http://www.jxva.com/blog/201202/335.html
