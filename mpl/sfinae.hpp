#ifndef __SFINAE_HPP_1476077150__
#define __SFINAE_HPP_1476077150__
namespace sss {

namespace mpl {
// SFINAE type trait to detect whether T::const_iterator exists.
struct sfinae_base {
    using yes = char;
    using no = yes[2];
};

template <typename T>
struct has_const_iterator : private sfinae_base {
private:
    template <typename C>
    static yes& test(typename C::const_iterator*);
    template <typename C>
    static no& test(...);

public:
    static const bool value = sizeof(test<T>(nullptr)) == sizeof(yes);
    using type = T;
};

template <typename T>
struct has_begin_end : private sfinae_base {
private:
    template <typename C>
    static yes&
    f(typename std::enable_if<
        std::is_same<decltype(static_cast<typename C::const_iterator (C::*)()
                                              const>(&C::begin)),
                     typename C::const_iterator (C::*)() const>::value>::type*);

    template <typename C>
    static no& f(...);

    template <typename C>
    static yes&
    g(typename std::enable_if<
        std::is_same<decltype(static_cast<typename C::const_iterator (C::*)()
                                              const>(&C::end)),
                     typename C::const_iterator (C::*)() const>::value,
        void>::type*);

    template <typename C>
    static no& g(...);

public:
    static bool const beg_value = sizeof(f<T>(nullptr)) == sizeof(yes);
    static bool const end_value = sizeof(g<T>(nullptr)) == sizeof(yes);
};
}  // namespace mpl

}  // namespace sss

// NOTE 对于const限定的成员函数，直接在Args后面添加const即可。形如：
// (arg-list ...)const
#ifndef GEN_MEMBERFUNC_TRAITS
#define GEN_MEMBERFUNC_TRAITS(Name, RetT, Args)                                \
    template <typename TP>                                                     \
    struct MembFuncTraits##Name : private sss::mpl::sfinae_base {              \
        template <typename P>                                                  \
        static yes& Name(                                                      \
            typename std::enable_if<                                           \
                std::is_same<decltype(static_cast<RetT(P::*) Args>(&P::Name)), \
                             RetT(P::*) Args>::value>::type*);                 \
        template <typename P>                                                  \
        static no& Name(...);                                                  \
        static bool const has_##Name =                                         \
            sizeof(Name<TP>(nullptr)) == sizeof(yes);                          \
    };
#endif

#ifndef MEMBERFUNC_TRAITS
#define MEMBERFUNC_TRAITS(Name, T) MembFuncTraits##Name<T>
#endif

#ifndef USING_MEMBERFUNC_TRAITS_VALUE
#define USING_MEMBERFUNC_TRAITS_VALUE(Name, T) \
    using MembFuncTraits##Name<T>::has_##Name
#endif

#endif /* __SFINAE_HPP_1476077150__ */
