#ifndef  __CMLPARSER_HPP_1364974733__
#define  __CMLPARSER_HPP_1364974733__

#include <sss/log.hpp>
#include <sss/utlstring.hpp>

#include <string>

#include <map>
#include <vector>
#include <set>

#include <typeinfo>

// NOTE main函数，命令行参数说明；
// 1. argv[0] 是当前可执行文件全路径
// 2. 后续的1到n-1是，参数
// 3. argv[n] 也是存在的，不过理论上是0！
//
//! sample
//! // 定义接受容器
//  //  此时有两种，分别可以存放一个值和多个值；
// sss::CMLParser::RuleSingleValue e_fname; // ("未提供 -t targetName");
//
//! // 定义命令行处理器：
//  // 有两种，分别是完全包含（即提供的参数，经过处理之后，不能有剩余）
//  //         仅过滤匹配部分（经过处理后，允许有剩余）
// sss::CMLParser::Include cmlparser;
//
//! // 定义与特定容器相关的匹配规则和存储规则
//  // 参数分别是规则前缀参数、规则、容器
// cmlparser.add_rule("-t", sss::CMLParser::ParseBase::r_parameter, e_fname);
// cmlparser.add_rule("--target", sss::CMLParser::ParseBase::r_parameter, e_fname);
//
//! // 使用处理器处理命令行参数
// cmlparser.parse(argc, argv);

// TODO 如何支持 int main(int argc, wchar ** argv); ？
//
// 最好用模版，而不是转换过去转换过来。 因为 argv 是字符串指针数组！
// 而我内部为了节省内存，没有用basic_string<>，而是用的原生的字符串指针！
// 因为，我本质上，没有去修改argv数组所指向的内存空间！
//
namespace sss {

namespace CMLParser
{
    bool shift(int & argc, char **& argv, int offset = 1);

    class Exception : public std::exception
    {
    public:
        Exception(const std::string& s) throw()
            : message(s)
        {}
        virtual ~Exception() throw()
        {}

    public:
        virtual const char * what() const throw()
        {
            return this->message.c_str();
        }
    private:
        std::string message;
    };

    // TODO 要实现值容器的互斥，很简单；再构建一个叫Seed的对象即可
    // Seed my_seed("-x 参数 -o 参数只能提供其中一个");
    // 然后，让RuleBase 对象，绑定上述my_seed对象即可。
    // 缺点：
    //  "语义矛盾问题"：
    //  当两个容器，都设定了MustSupply，那么解析器就不可能正常运行了。因为互斥
    //  对象，要求他们两个，只能其中一个有值。而MustSupply属性又要求它们都提供
    //  值。
    //  当然，其中一个设定了MustSupply，也会导致语义矛盾。
    //  只有当这些容器，都不设定MustSupply属性，才OK！
    //
    //  此时，有两种处理办法——构建解析结构的时候动态检测。发现设置了
    //  MustSupply，还进行绑定动作，则马上绑定/构造失败。

    //  NOTE 忽略，或者覆盖的办法不可取；出现这种情况，往往是程序逻辑设计上的错
    //  误，必须即时反映给库用户。
    //  如果，能在编译阶段就给出错误提示就最好了。
    ///////////////////////////////////////////
    //
    // TODO 必要条件语义
    // 有些选项，只有在某些选项开关打开之后，才能使用。虽然这个功能比较鸡肋，但
    // 还是可以想它是如何实现的。
    // 上面完成了"互斥"语义，已经完成——让Rule内部绑定一个计数器；如果两个绑定
    // 同一个计数器，就可以实现互斥了。
    // 而这个"必要条件"，则可以考虑"链表形式"。就是构造某Rule需要另一个Rule计数
    // 不为0的时候，才支持。则可以将"必要添加"的Rule，作为另外一个Rule构造时候的
    // 参数。内部，则可以维护一个类似链表的结构，以表示依赖关系。
    //
    // 当然，上述功能，基本没有实现的必要。因为，如果必要条件是一个开关类的选项
    // ，那根本可以省略掉；因为，一旦提供后一个选项，那可以认为前置调价，也同时
    // 提供了。
    //
    class RuleBase
    {
    public:
        RuleBase();

        // 通过构造函数，是否提供参数，决定该类型参数是否必须；并且定义未提供的
        // 时候，返回的出错信息
        explicit RuleBase(const std::string& msg_on_empty);

        virtual ~RuleBase() = 0;

    protected:
        // 检查输入的值，是否在允许值范围内。如果不是，抛出异常。
        bool assert_valid_value(const char * val) throw (CMLParser::Exception);

    public:
        bool    add_option(const char * val, int id);
        virtual bool assign(const char *) = 0;
        virtual const char * get(int index) const = 0;
        virtual size_t size() const = 0;

        template <typename T>
            T get_id(int index) const
            {
                optional_values_t::const_iterator it = this->optional_values.find(this->get(index));
                if (it != this->optional_values.end())
                {
                    return T(it->second);
                }
                else
                {
                    std::ostringstream oss;
                    oss << "CMLParser::RuleBase : option by int id `" << index << "`not found";
                    throw CMLParser::Exception(oss.str());
                }
            }
    public:

        bool            must_supply;            // 是否必须赋值？
        std::string     message_on_empty;       // 如果必须赋值，那么未赋值时，出错信息是？
        typedef std::map<std::string, int> optional_values_t;
        optional_values_t optional_values;      // 允许输入哪些值？
    };

    // 存放单值
    class RuleSingleValue : public RuleBase
    {
    public:
        explicit RuleSingleValue(const std::string& msg_on_empty);

        RuleSingleValue();

        ~RuleSingleValue();

    public:
        virtual bool assign(const char * val);
        virtual size_t size() const;
        virtual const char * get(int /* index */) const;

    private:
        bool is_assigned;
        const char * value;
    };

    // 存放多值
    class RuleMultyValue : public RuleBase
    {
    public:
        explicit RuleMultyValue(const std::string& msg_on_empty);

        RuleMultyValue();

        ~RuleMultyValue();

    public:
        virtual bool assign(const char * val);

        size_t size() const;

        virtual const char * get(int index) const;

        bool has_value(const std::string& val);

    private:
        std::vector<const char *> ctainer;
    };

    // 一些基本操作定义在这里，比如添加add_rule；del_rule；
    // 如何处理遇到的参数，则由派生类自己处理。
    class ParseBase
    {
    public:
        enum rule_param_t {
            r_prefix,           // 形如 -lxxx 返回给用户的value值是 xxx
            r_parameter,        // 形如 -o outfname.txt   value值是 outfname.txt
            r_option            // 形如 -a            没有value值；
        };
        ParseBase();

        virtual ~ParseBase() = 0;

    public:
        // NOTE 因为rule_param_t保存在RuleBase结构体外面，所以库使用者，完全可
        // 能弄错rule_param_t 和 容器RuleBase的配对关系。比方说，将 r_option 和
        // RuleMultyValue 类型的容器传递给add_rule()函数；
        bool add_rule(const std::string& opt,   // 匹配项关键字
                      ParseBase::rule_param_t type, // 决定匹配规则，读入参数策略
                      RuleBase & rule);         // 决定保存值的策略

        bool del_rule(const std::string& opt);

        virtual int parse(int & argc, char **& argv) = 0;

        // 继续解析模式——不再抛出异常；并且，不再默认第一个参数是函数名；
        virtual int continue_parse(int & argc, char ** & argv) = 0;

    protected:
        // NOTE 输入参数代表一个左闭右开区间。
        int do_parse_current(char ** argv_b, char ** argv_e);

        void check_must_supply_options();

        // 在处理一个命令行参数的时候，这个参数是prefix,parameter还是option，是
        // 与该参数直接相关的。
        // 因为 std::string 和 rule_param_t 是一一对应关系；而多个std::string
        // 可以对应同一个 RuleBase，所以内部存储结构为如下所示：
        typedef std::map<std::string,
                         std::pair<ParseBase::rule_param_t, RuleBase*> > rule_map_t;
        rule_map_t      rules;

        std::set<RuleBase*> rules_set;
    };

    // 该规则的 parse 之后，会剔除符合规则部分的参数；
    // 即，允许出现不符合定义规则部分的参数；剩下的部分，留给库用户自行处理——
    // argc,argv的值，可能会发生变化；
    class Exclude : public ParseBase
    {
    public:
        Exclude();

        ~Exclude();

    public:
        // 如何过滤已经处理过的命令行参数呢？
        // 先创建一个char * buffer[]，用于保存处理后的字符指针数组。
        // 等到循环处理命令行之后，看argc处理的个数，来决定如何处理buffer数组。
        int parse(int & argc, char **& argv);
        int continue_parse(int & argc, char ** & argv);
    };

    // 该规则的 parse 动作，不允许出现 不符合规则的命令行参数；
    // 也不允许剩余参数；
    class Include : public ParseBase
    {
    public:
        Include();
        ~Include();

    public:
        int parse(int & argc, char **& argv);
        int continue_parse(int & argc, char ** & argv);
    };
}

} // namespace sss


#endif  /* __CMLPARSER_HPP_1364974733__ */
