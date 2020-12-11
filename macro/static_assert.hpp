#ifndef  __STATIC_ASSERT_H__
#define  __STATIC_ASSERT_H__

template <bool x> struct STATIC_ASSERTION_FAILURE;
template <> struct STATIC_ASSERTION_FAILURE<true>{};
template<int x> struct static_assert_test{};

#define STATIC_ASSERT(B) \
  typedef static_assert_test<sizeof(STATIC_ASSERTION_FAILURE<B>)> MACRO_JOIN(static_assert_typedef_, __LINE__)

//防止传入的X,Y也是宏
#define  MACRO_JOIN(X, Y) MACRO_DO_JOIN1(X, Y)
#define  MACRO_DO_JOIN1(X, Y) MACRO_DO_JOIN2(X, Y)
#define  MACRO_DO_JOIN2(X, Y) a##b

#endif  /*__STATIC_ASSERT_H__*/
