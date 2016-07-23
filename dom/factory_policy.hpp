#ifndef  __FACTORY_POLICY_HPP_1391063871__
#define  __FACTORY_POLICY_HPP_1391063871__

#include "html_tag_name_map.hpp"
#include <sss/log.hpp>

#if 0
namespace html_tags{
    class html_tag;
}
#endif

namespace sss {

template <typename Factory, typename TB, typename TD>
class factory_policy{   //{{{1
public:
    // C++ 规定，模版参数不能声明为友元
    // friend typename TB;
    // friend typename TD;
    factory_policy() {
    }
    ~factory_policy() {
    }

public:
    static bool __regist;       // 初始化的时候，会自动调用regist_me函数，以完成自动机"注册"
    static bool regist_me() {
        return Factory::reg_cloner(sss::dom::html_tag_name_map::get_name_by_id(TD::tag_id), &create);
    }

    //  最终的工作类的cpp文件中，使用下列宏：
    //
    //  REG_CLONER(fsm)
    //
    //  以完成注册；
    //

    static TB* create() {
        return dynamic_cast<TB*>(new TD);
    }
};

#define REG_FACTORY(Factory,tb,td) \
    template<> bool factory_policy<Factory,tb,td>::__regist = factory_policy<Factory,tb,td>::regist_me();

} // end of namespace sss
#endif  /* __FACTORY_POLICY_HPP_1391063871__ */

// NOTE 上面利用静态成员变量的赋值动作“regist_me()“，来完成注册机的注册动作；
// 上述，也可以用一个内部类（class in class）的构造函数来完成动作；然后cpp文件
// 中，写出该静态对象即可；
//
// 如：
//
// hpp:
// class factory_policy {
//   class register {
//   };
//
//   static register regist_me;
// };
//
// cpp:
// factory_policy<...>::register regist_me;
