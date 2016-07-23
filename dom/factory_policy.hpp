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
    // C++ �涨��ģ�������������Ϊ��Ԫ
    // friend typename TB;
    // friend typename TD;
    factory_policy() {
    }
    ~factory_policy() {
    }

public:
    static bool __regist;       // ��ʼ����ʱ�򣬻��Զ�����regist_me������������Զ���"ע��"
    static bool regist_me() {
        return Factory::reg_cloner(sss::dom::html_tag_name_map::get_name_by_id(TD::tag_id), &create);
    }

    //  ���յĹ������cpp�ļ��У�ʹ�����к꣺
    //
    //  REG_CLONER(fsm)
    //
    //  �����ע�᣻
    //

    static TB* create() {
        return dynamic_cast<TB*>(new TD);
    }
};

#define REG_FACTORY(Factory,tb,td) \
    template<> bool factory_policy<Factory,tb,td>::__regist = factory_policy<Factory,tb,td>::regist_me();

} // end of namespace sss
#endif  /* __FACTORY_POLICY_HPP_1391063871__ */

// NOTE �������þ�̬��Ա�����ĸ�ֵ������regist_me()���������ע�����ע�ᶯ����
// ������Ҳ������һ���ڲ��ࣨclass in class���Ĺ��캯������ɶ�����Ȼ��cpp�ļ�
// �У�д���þ�̬���󼴿ɣ�
//
// �磺
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
