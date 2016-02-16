#ifndef  __PENVSYS_HPP_1436487885__
#define  __PENVSYS_HPP_1436487885__

//! 程序级别的环境变量管理器
// 特点，类似bash一样，变量定义和获取值，有多种形式；一种是动态赋值，一种是为了
// 避免循环定义，而可以是立即赋值；
// 整个管理器，我并不关心单个值如何、如何；主要还是如下操作：
// penvmgr
//
// 变化的主要是前缀；
// 还是写产生式吧（应该类似 makefile)：
// bash中，变量定义：
// port=777
// 127.0.0.1:$port
//
// ${port} 也可引用变量
//
// 至于复杂的指令，以及外部调用，则用$( ... )包裹
//
// 还有，为了避免循环引用；可定义计算式获取——不保存变量，而是直接用变量定义式
// ，获取一个值，再将该值，反存到管理器里面。这样，就避免了循环定义；
//
// 如何检测循环定义？
// 利用依赖关系"图"？
// 根据定义式，可生成一个依赖关系；然后挨个检查；——逐个剔除只有一个的关系；
// 首先，
// "依赖与自己"，
//      这本身就不成立！
//
// 参考：e:\project\dbclone\obj_depend_sort.hpp|34
//       里面的"依赖排序算法"；
//
// 在同样的挨个剔除依赖为0，直到无法再剔除池子中的对象的时候，就说明有依赖重复！
// 不过，这样的话，不好区分那些在一起，是循环依赖……
//
//还有一个问题，就是，如何进行动态检测？我总不可能没新建一个变量，就如此搞一通吧？
//
//这不现实；
//
// 要知道，这种结构一般是逐条增加的，所以，肯定是来一个检测一个；
//
// 不对，如果定义式，涉及到还未定义的变量，怎么办？
//
// 还有一种办法，就是记录调用栈——接口，调用 impl 函数；如果impl函数发生了重复
// 调用（获取同一个变量值——而且是在同一条"线"上？），则马上抛出异常！
//
// 应该对求值过程中，变量的值，做一个记录，如果出现覆盖的情况，则抛出异常；另外
// ，如果一个变量的定义式，两次引用到另外一个变量，则只做一次求值！
//
// 如此的话，可以结合loopCheck() 函数，实现静态检查。
//
// 还有变量，以及定义式，可以写入外存，并动态载入。
//
// 方案C
//    每增加一个变量或者修改一个变量，就对所有变量，进行重算；
//
// 方案D
//    每个变量（相当于函数名），其定义式，相当于函数体；依赖的变量，相当于参数
//    列表；
//    利用参数列表，建立树；如果出现循环结构，就说明有循环依赖；
//
//    另外，变量定义式的话，还可以调用shell指令，比如，可以获取pwd，time等变化信息；
//
// 另外，可以参考
// ~/extra/sss/include/sss/tdspp2/sql_var_replacer.hpp|23
//
// TODO 还可以参考多级求值；即，env可以设置一个链表；找不到的变量可以往上级求索
// ——相当于作用域；
//
// NOTE 注意，需要避免作用域发生重复引用！
//
// 比如，必须在构造函数，提供上下级关系，且不允许修改，这就能在一定程度避免链表
// 循环了。
//
// 当然，求值机制，也需要修改。
//
// 最后，我的变量名，还有一个问题：变量名本身不能体现作用域；比如vim中，有系统
// 变量s:开头，
//
// 有临时变量l:开头——不过，一般省略；还有v:开头的变量；

// TODO
// 1. template <typename T> set_refer(var, T & ref);
//
// 不过，不管是何种类型，检出的都是std::string值
//
// 这问题来了，我需要在内部，支持真正的多种数据类型吗？
// 还有，是 refer 还是 自己管理内存呢？
//
// 如果要真正支持多种数据类型，那就相当于内嵌一个解释器了。
//
// 2. “父环境”；get动作，可以获取上级环境里面的变量——如果当前找不到的话；
//    不过，set动作，只针对当前环境；
//    另外，需要注意的是，环境的作用域问题；
//    仔细想一下，好像api部分还需要思考；
//    我需要达到的效果：
//    get_expr的时候，对当前环境未知的值，可以向上层环境获取；由于同名的被覆盖
//    ，所以，也不存在不同次获取，命名干扰什么的问题。
//
//    不过，麻烦在于，如果支持$()，那么是否支持内部再定义变量呢？这会影响依赖判
//    断的。
//
//    还有，是细节的，某变量是否存在的判断问题；一般，编程语言，在使用某一个变
//    量的时候，都需要先定义；
//    然后再进行操作。而我的env::set动作，等于定义+赋值；那么，如果一个变量实际
//    定义在上层环境，那么set动作，实际是发生在哪里？
//
// 3. 支援 $() 方法；以便进行扩展；
//
// 4. ${} 变量问题；有些时候，可能需要输出形如 $abc 的字符串，像之前的模式，是
// 肯定不行的；即，必须设计一个 escape 方法，比如，$$表示单个的$字符；不过，此
// 时，就涉及到最大匹配的问题了。
//
// 当然，还是沿用最长匹配；即：
//
// $$abc
//
// 会被解析为 字符串 "$" "abc"， 而不是 "$" 变量引用"$abc"；
// =========================================================
// 如果要支持表达式，就意味着，我的变量的定义式，就不再是线性结构，而是一个树形结构。
//
// 变量列表的获取，也会变得麻烦。貌似N多东西，都需要重新设计。不过，可以限定一
// 下，表达式内部，只能有操作符和操作数？不能嵌套表达式？
//
// 不对，貌似不是一般的嵌套形式；在$()表达式内部，相当于lisp链表；这里，其实是
// 有数据类型的区别的，而不是像外部，字符串为第一元素；
//
// 这里的第一元素，是组成链表的对象！——不对，貌似lisp里面，字符串，也是要单独制定的？
//
// $(operator operand_1 ... operand_n)
//
// 第一个operator，也可以是 $()形式；系统先完成内部解释、替换之后，并将其理解为
// 函数名；然后，将操作符拷贝到参数列表里面，
//
// 并在一个函数的新空间（这是必要的，lisp语言，允许动态修改函数对象；将函数代码
// 单独拷贝，再运行，才能达到动态修改自己的效果）里面，运行它；
// 同样，将返回结果，替换上层结构。
//
// 要内嵌那些函数呢？当然是path类的函数；还有，最好有bash 的 `` 执行外部指令！
//
// =======================
// TODO 可能，还需要做一个能容纳所有对象的管理器！必须继承自某一个基类，然后可
// 以cast为其容纳的对象。
// 再一个问题是，是否仅pod对象还是，可以含有指针？
// 即，这仅仅是一个基本数据类型的有序容器，还是完全动态的一个玩意儿？
// 或者说，与C++基本类型没多大关系，而是类似一个脚本引擎？
// 还是脚本引擎靠谱一点。
// 这样用户可以尽可能地在运行时，改变程序模式。
//
// NOTE 变量名扩展
//
// 当前的\w\+的模式，还是太单调了；不支持一些复杂的情形；
//
// 可以类vim那样，增加一些前缀，比如s:,g:这种东西；
//
// 分别用于代表全局变量、系统变量，当前域变量等等；
//
// 比如时间戳，就可以加入系统变量；还有当前工作目录等等东西。
//
// 所谓系统变量，是用户可以引用他们，不能删除和修改他们——逻辑上满足"只读"；
// 当然，可以修改其属性，比如时间戳记，可以修改其表现形式，所谓的stringformat；
//
// 而全局变量，相当于用户数据交换的一个桥梁；以部分消除，作用域概念下，变量定位
// 的含糊性；
//
//! 最后是可扩展性
//
// 扩展性最好是可以调用shell脚本；另外调用shell的时候，应该默认丢掉错误输出，即：
// 附加上 2>/dev/null
//
// 语法呢？
//
// ${shell(command p1 p2 ... pn)}
//
// 即，parse的时候，以")}"为结尾；前面，则以"${shell("为开头；
// 说实话，这样装车的可能性还是很大；
// 再加上""标记更好……
//
// shell中的字符串替换，很简单，直接用的是``为间隔。要不，我也用这种模式？
// 即：
//
// ${`command p1 p2 ... pn`}
//
// 这样，就人工的文本来说，貌似OK了；——但还是怕注入啊！
//
// 可以用StringSlice替换内部的string吗？
//
// expression_t 本质上是由一个字符串切割而成的字符串数组；
//
// 如果要用StringSlice代替这个数组中的字符串，那么：
// 1. 需要保存原始构造字符串；
// 2. StringSlice内部本质上保存的是字符串边界指针，所以expression_t的拷贝就是一
//    个问题；所以，更好的方案是，既然我保存了原始的字符串，那么保存将上述指针
//    ，变为标量即可；即，记录偏移量；
//
//    然后使用的时候，可以临时构造一个StringSlice出来……
//
//! 是否支持变量修饰符？
//
// 类 vim.fnamemodifier("%:p:h")
//
// ——这个归于函数调用；
//
//! 枚举如何实现？
//
// 这个的难点在于，我设计之初，是为了实现作用域这个概念——有继承性；这同时就有
// 了变量覆盖的特性。即，自作用域的变量，可以与上层同名；如果同名，就会覆盖；
//
//! 数据类型的扩展
//
// 之前，我提到，用类似refvar的办法，引用用户代码中的变量——这意味着，用户可以
// 方便地存入各种类型的变量；并用于字符串拼接；
//
// 但是，按照原有类型，并取出数据的话，就不方便操作了。boost::any ?
//
// 一个可以考虑的替换方案是，存放json字符串；
//
// 即，变量本身的定义式是json字符串；不过，json串的值，如何与其他字符串进行运算
// 呢？难道只能存取json，而不能操作它？
//
// 那么多少会显得有些无用；

#include <map>
#include <string>
#include <vector>
#include <set>

namespace sss{

class depend_checker_t;
class PenvMgr
{
public:
    typedef std::vector<std::string>                    expression_t;
    typedef std::set<std::string>                       var_list_t;
#if 0
    typedef std::pair<var_list_t, expression_t>         var_body_t;
#else
    struct var_body_t : public std::pair<var_list_t, expression_t>
    {
        typedef std::pair<var_list_t, expression_t> Base_t;
        //using var_list = Base_t::first;
        var_list_t& var_list()
        {
            return Base_t::first;
        }

        expression_t& def_body()
        {
            return Base_t::second;
        }

        const var_list_t& var_list() const
        {
            return Base_t::first;
        }

        const expression_t& def_body() const
        {
            return Base_t::second;
        }

        void print_body(std::ostream& o) const;
    };

#endif
    typedef std::map<std::string, var_body_t>           env_t;
    typedef env_t::iterator                             iterator;
    typedef env_t::const_iterator                       const_iterator;

public:
    explicit PenvMgr(PenvMgr * parent = 0);

    PenvMgr(const PenvMgr& ref);

    ~PenvMgr();

public:
    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

public:
    bool        set(const std::string& var, const std::string& expr);
    std::string get(const std::string& var);       // 将var字符串，理解为一个变量

    // 将expr，理解为变量表达式，在替换之后，再返回结果；
    std::string get_expr(const std::string& expr);

    // TODO 未定义变量，是返回空串，还是抛出异常？返回空串！
    // 因为导入变量的过程，可能不是依赖排序后的顺序！
    bool        unset(const std::string& var);
    bool        has(const std::string& var);

    bool        has_parent() const
    {
        return this->_parent;
    }

    PenvMgr& parent() const;

    void print(std::ostream& o) const;

public:
    // ${\w[\w\d]*}
    static bool is_var_refer(const std::string & var);
    // \w[\w\d]*
    static bool is_var(const std::string & var);

    static std::string refer2name(const std::string & var);

protected:

    const var_body_t *   find_body(const std::string& var) const;
    std::string evaluator_impl(const std::string& var, depend_checker_t & dc);

protected:
    static std::string generate(const var_body_t & bd, const depend_checker_t & dc);

//    bool        loopCheck();

private:
    env_t       _env;
    PenvMgr    *_parent;
};

inline std::ostream& operator << (std::ostream& o, const PenvMgr& env)
{
    env.print(o);
    return o;
}

}

#endif  /* __PENVSYS_HPP_1436487885__ */
