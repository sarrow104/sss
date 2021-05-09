// 本宏主要用于位掩码形式的参数操作的函数生成。
//  本类函数考虑到这样的掩码的与或非运算的运行对象必须受到限定，以严办参数类型的关口
//  思路，来自STL中std::ios_base::openmode
//  注意，宏参数type为enum类型，而非struct 或者 class
#define SSS_BIT_MACK_ARGUMENT_OPERATORS(type)                       \
    inline type                                                     \
    operator&(type __a, type __b)                                   \
    { return type(static_cast<int>(__a) & static_cast<int>(__b)); } \
    inline type                                                     \
    operator|(type __a, type __b)                                   \
    { return type(static_cast<int>(__a) | static_cast<int>(__b)); } \
    inline type                                                     \
    operator^(type __a, type __b)                                   \
    { return type(static_cast<int>(__a) ^ static_cast<int>(__b)); } \
    inline type&                                                    \
    operator|=(type& __a, type __b)                                 \
    { return __a = __a | __b; }                                     \
    inline type&                                                    \
    operator&=(type& __a, type __b)                                 \
    { return __a = __a & __b; }                                     \
    inline type&                                                    \
    operator^=(type& __a, type __b)                                 \
    { return __a = __a ^ __b; }                                     \
    inline type                                                     \
    operator~(type __a)                                             \
    { return type(~static_cast<int>(__a)); }                        \

/*TODO
 *如何灵活地打开、关闭一个宏的定义？
 */
