#ifndef __PENVMGR2_HPP_1451874522__
#define __PENVMGR2_HPP_1451874522__

//! 支持重新取出 setRefer()的对象吗？
// 关键在于safe cast
//
// 如果，如果能把原始的类型信息，按字符保存，有用吗？
//
// 如果能将该信息，和具体的构造函数对应起来，就可以动态构造对象了；先分配合适大小的内存，然后用placement new；
// 下面是关于类型到字符串的文章
// http://www.2cto.com/kf/201405/301319.html
// http://blog.csdn.net/lnwaycool/article/details/6960350
// http://blog.sina.com.cn/s/blog_5d3402010100plsu.html

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
//============================================================================
// TODO 
// 2016-03-19 支持"函数调用"；形如：
//
//  ${varname:substr(0,5):toupper()}
// 函数是串联的；上一个函数的结果，可以当做下一个函数的参数；——当然，高级一点
// 的话，还可以支持占位符，以完成多参数的调用；
//
// 比如，用{}，表示上一个指令的结果，替换此处；
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
// C++的虚拟函数是如何实现的？
//
// C++的继承和虚拟函数表，的覆盖特性，与上述说法类似；继承自同一个基类的实体类
// ，其虚表与父类，有同样的结构——顺序一致；
//
// 如果没有覆盖，该位置的函数指针，就是null；
//
// 而我这里的情况，除了有继承、覆盖，还相当于定义了自己的虚函数；
//
// 对于C++虚表的和我的env区别的理解，首先要明白的一点是，C++语言是强类型的编译
// 型语言；动态函数的调用，意味着，函数名 -to- 虚函数表下标 ，这个转换过程，是
// 在编译时就完成了的！
//
// 而我这个，明显是动态的；我记录变量名的时候，不能只记录一个id，而必须记录一个
// 具体的名字；
//
// C++中，如果子类有增加虚函数槽，那么子类的虚函数表，就比父类长即可；以此，达
// 到了C++继承、覆盖、速度等等的平衡；
//
// 而我这个env，其子作用域，往往比父作用域，只多增加少数几个变量，覆盖少许，甚
// 至不覆盖；
//
// 那么，如果完全空出父作用域那么多的变量槽，就显得徒然耗费内存了。
//
// 因此，枚举的解决办法就是，额外增加一个set结构，在枚举过程中，记录已经枚举过
// 的变量的名字；
//
// 关于这个问题的讨论，可以参见：
//
//! http://blog.csdn.net/hackbuteer1/article/details/7883531
//
// 即，如果虚函数表很大、覆盖不多的时候，采取名字查找比较好，比如C++的GUI库，
// MFC和QT就是如此；——上文语。
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
//
// 除非，我额外提供了json操作函数，如提取一类的东西，那么传入json为变量值的作法
// ，才显得完整。
//
// {""}
//
// 不对——如果支持json的话，其某字符串值内部，若有"${var}"字样，那岂不是会导致
// 错误替换？
//
// 所以，安全的办法是，提供另外一个，如raw_string的set函数，即，禁用这种解析行
// 为。
// 这样的话，就要求，我使用类似 sss/util/ArgsStringBuilder.hpp|253
// 中的表达方式；
//
// ——"串"与"变量"间隔表示；
//
//! 内存使用的问题
//
// PenvMgr用作模板引擎的时候，内存使用的问题；大文件模板？流输出，还是……
// 
// 我觉得，可以在解析过程中，保存中间结果；遇到重复的变量引用，可以马上输出……
//
// 假设，模板文件是大文件；那么，在替换的变量的时候，也保存一个完整的串，显然是不智的；
// 最好是一边解析，一边输出……
//
//! 我这个工具的可以看做支持函数的子语言吗
// 
// 内嵌脚本的一个特点是支持自定义函数。我这个东西，更像是自动求值的环境变量——
// 说实话，其特性，特别像makefile里面的变量。
//
// 它是变量的堆嵌，而不是定义新函数。
//
// 也不支持调用形式……
// env.setFunction(funcName, funcBody);

#include <sss/util/StringSlice.hpp>
#include <sss/utlstring.hpp>
#include <sss/log.hpp>

#include <iterator>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <set>

#define PENVMGR2_SUPPORT_SHELL_CMD

namespace sss{

    class depend_checker2_t;

    class PenvMgr2
    {
    public:
        enum var_type_t {
            TYPE_UNKNOWN = '\0',
            TYPE_OSENV  = 'e',
            TYPE_GLOBAL = 'g',
            TYPE_NORMAL = 'n',
            TYPE_SYSTEM = 's',
            TYPE_SHELL  = 't'
        };

        typedef std::string::const_iterator iter_t;
        typedef sss::util::StringSlice<iter_t> StringSlice_t;

        // 改进：
        // 用奇偶区分字符串和变量；
        // 然后变量，取-去掉{}后的位置；
        // expression_t，可以增加std::function<std::string(*)()>这样的成员；
        // 但是，再额外套上 template<typename T>的引用成员，就不太合适了。
        //
        // 会导致类型碎片；
        //
        // 然后一个expression_t对应一个std::string;
        //
        // is_var_refer(),is_var()这些接口，也需要增加重载，以支持类StringSlice接口；
        // std::ostream&的输出，可能需要使用write()函数。
        struct expression_t
        {

            template<typename T>
                static std::string AnyRefer(void * p)
                {
                    return sss::cast_string(*reinterpret_cast<T*>(p));
                }

            typedef  std::vector<std::pair<int, int> >  Slice_T;
            typedef  void * FuncParamT;
            typedef  std::string(*FuncT) (FuncParamT);

            expression_t()
                : m_func(0), m_func_param(0)
            {
            }

            expression_t(const expression_t& rhs)
                : m_slice(rhs.m_slice), m_data(rhs.m_data),
                  m_func(rhs.m_func), m_func_param(rhs.m_func_param)
            {
            }

            ~expression_t()
            {
            }

            expression_t& operator = (const expression_t& rhs)
            {
                if (this != &rhs) {
                    m_slice = rhs.m_slice;
                    m_data.assign(rhs.m_data);
                }
                return *this;
            }

            void clear()
            {
                this->m_slice.clear();
                this->m_data.clear();
                this->m_func = 0;
                this->m_func_param = 0;
            }

            void push_back(int beg, int end)
            {
                this->m_slice.push_back(std::make_pair(beg, end));
            }

            void push_back(std::string::const_iterator it_beg,
                           std::string::const_iterator it_end)
            {
                this->m_slice.push_back(std::make_pair(std::distance(m_data.cbegin(), it_beg),
                                                       std::distance(m_data.cbegin(), it_end)));
            }

            void push_back(const sss::util::StringSlice<std::string::const_iterator>& slice)
            {
                this->push_back(slice.begin(), slice.end());
            }

            StringSlice_t operator[] (int idx) const
            {
                return StringSlice_t(this->m_data.begin() + this->m_slice[idx].first,
                                     this->m_data.begin() + this->m_slice[idx].second);
            }

            size_t size() const {
                return this->m_slice.size();
            }

            void print(std::ostream& o) const {
                o << &this->m_data << std::endl;
                for (size_t i = 0; i != this->size(); ++i) {
                    if (i & 1) {
                        o << "\t${" << this->operator[](i) << "}";
                    }
                    else {
                        o << "\t" << this->operator[](i);
                    }
                    o << " [" << this->m_slice[i].first << ", " << this->m_slice[i].second << ']' << std::endl;
                }
            }

            class const_iterator;

            class iterator
            {
                friend class const_iterator;
            public:
                explicit iterator(expression_t& e, expression_t::Slice_T::iterator it)
                    : m_e(e), m_s(it)
                {
                }
                ~iterator()
                {
                }

            public:
                iterator& operator++()
                {
                    m_s++;
                    return *this;
                }

                iterator operator++(int)
                {
                    iterator tmp(*this);
                    m_s++;
                    return tmp;
                }

                bool operator == (const iterator& rhs) const
                {
                    return this->m_s == rhs.m_s;
                }

                bool operator != (const iterator& rhs) const
                {
                    return !operator == (rhs);
                }

                StringSlice_t operator *()
                {
                    return StringSlice_t(m_e.m_data.begin() + m_s->first,
                                         m_e.m_data.begin() + m_s->second);
                }

            private:
                expression_t& m_e;
                expression_t::Slice_T::iterator m_s;
            };

            class const_iterator
            {
            public:
                const_iterator(const iterator& i)
                    : m_e(i.m_e), m_s(i.m_s)
                {
                }
                explicit const_iterator(const expression_t& e, expression_t::Slice_T::const_iterator it)
                    : m_e(e), m_s(it)
                {
                }
                ~const_iterator()
                {
                }

            public:
                const_iterator& operator++()
                {
                    m_s++;
                    return *this;
                }

                const_iterator operator++(int)
                {
                    const_iterator tmp(*this);
                    m_s++;
                    return tmp;
                }

                bool operator == (const const_iterator& rhs) const
                {
                    return this->m_s == rhs.m_s;
                }

                bool operator != (const const_iterator& rhs) const
                {
                    return !operator == (rhs);
                }

                StringSlice_t operator *()
                {
                    return StringSlice_t(m_e.m_data.begin() + m_s->first,
                                         m_e.m_data.begin() + m_s->second);
                }

            private:
                const expression_t& m_e;
                expression_t::Slice_T::const_iterator m_s;
            };

            iterator begin()
            {
                return iterator(*this, this->m_slice.begin());
            }

            const_iterator begin() const
            {
                return const_iterator(*this, this->m_slice.begin());
            }

            iterator end()
            {
                return iterator(*this, this->m_slice.end());
            }

            const_iterator end() const
            {
                return const_iterator(*this, this->m_slice.end());
            }

            Slice_T         m_slice;
            std::string     m_data;
            FuncT           m_func;
            FuncParamT      m_func_param;
        };

        typedef std::set<std::string>                       var_list_t;

        struct var_body_t
        {
            var_list_t      first;
            expression_t    second;

            bool isFunction() const {
                return this->second.m_func;
            }

            std::string evalFunction() const {
                if (this->isFunction()) {
                    return this->second.m_func(this->second.m_func_param);
                }
                return "";
            }

            var_list_t& var_list()
            {
                return first;
            }

            expression_t& def_body()
            {
                return second;
            }

            const var_list_t& var_list() const
            {
                return first;
            }

            const expression_t& def_body() const
            {
                return second;
            }

            void print_body(std::ostream& o) const;
        };

        typedef std::map<std::string, var_body_t>           env_t;
        typedef env_t::iterator                             iterator;
        typedef env_t::const_iterator                       const_iterator;

    public:
        explicit PenvMgr2(PenvMgr2 * parent = 0);


        ~PenvMgr2();

    private:
        PenvMgr2(const PenvMgr2& ref);

    public:
        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;

    public:
        bool        set(std::string var, const std::string& expr);
        bool        setRawStr(std::string var, const std::string& expr);

        bool        set(std::string var, expression_t::FuncT func, expression_t::FuncParamT param = 0);

        template<typename T> bool setRefer(std::string var, T & value)
        {
            this->set(var, &expression_t::AnyRefer<T>, &value);
        }

        std::string get(std::string var) const;       // 将var字符串，理解为一个变量

        // 将expr，理解为变量表达式，在替换之后，再返回结果；
        std::string get_expr(const std::string& expr) const;
        std::string get_expr_file(const std::string& file_script) const;

        // TODO 未定义变量，是返回空串，还是抛出异常？返回空串！
        // 因为导入变量的过程，可能不是依赖排序后的顺序！
        bool        unset(std::string var);
        bool        has(std::string var) const;

        bool        has_parent() const
        {
            return this->_parent;
        }

        PenvMgr2&   getGlobalEnv();
        const PenvMgr2&   getGlobalEnv() const;

        PenvMgr2&   parent() const;

        void print(std::ostream& o) const;

        void dump2map(std::map<std::string, std::string>& out) const;

        void set_shellscript_workdir(const std::string& path);
        void unset_shellscript_workdir();

    public:
        // ${\w[\w\d]*}
        static bool is_var_refer(const std::string & var);
        static bool is_var_refer(const StringSlice_t& var);
        // \w[\w\d]*
        static bool is_var(const std::string & var);

        static std::string refer2name(const std::string & var);

    protected:

        std::string getSystemVar(const std::string& var) const;

        std::string getEnvVar(const std::string& var) const;

        std::string getShellComandFromVar(const std::string& var) const;
        std::string getShellComandFromVar(const std::string& var, depend_checker2_t & dc) const;

        const var_body_t *   find_body(const std::string& var) const;
        std::string evaluator_impl(std::string var, depend_checker2_t & dc) const;

    protected:
        static std::string generate(const var_body_t & bd, const depend_checker2_t & dc);

    private:
        env_t       _env;
        PenvMgr2    *_parent;
    };

    inline std::ostream& operator << (std::ostream& o, const PenvMgr2::expression_t& e)
    {
        e.print(o);
        return o;
    }

    inline std::ostream& operator << (std::ostream& o, const PenvMgr2& env)
    {
        env.print(o);
        return o;
    }

}


#endif

