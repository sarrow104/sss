#include <sss/CMLParser.hpp>

#include <sstream>
#include <map>
#include <algorithm>

#include <sss/spliter.hpp>

namespace sss {
    namespace CMLParser {

    bool shift(int & argc, char **& argv, int offset)
    {
        if (offset > 0 && offset <= argc) {
            argc -= offset;
#if 0
            for (int i = 0; i < argc; ++i) {
                argv[i] = argv[i + offset];
            }
#else
            argv += offset;
#endif
            return true;
        }
        return false;
    }

//! RuleBase
RuleBase::RuleBase()
    : must_supply(false)
{ }

RuleBase::RuleBase(const std::string& msg_on_empty)
: must_supply(true), message_on_empty(msg_on_empty)
{ }

RuleBase::~RuleBase()
{ }

namespace {

class MapValue_get_first
{
public:
    std::string operator()(const RuleBase::optional_values_t::value_type& p)
    {
        return p.first;
    }
};

}

#if __cplusplus >= 201103L
bool RuleBase::assert_valid_value(const char * val)
#else
bool RuleBase::assert_valid_value(const char * val) throw (CMLParser::Exception)
#endif
{
    // 1. 如果没有设置该值，说明任何类型的字符串，都是允许的；当然不报错。
    if (this->optional_values.empty())
        return true;

    // 2. 有可能收到0指针。
    if (!val)
        return true;

    optional_values_t::iterator it = this->optional_values.find(val);
    if (it != this->optional_values.end())
        return true;

    // 3. 未找到匹配
    //
    // NOTE 因为本函数只有出错的时候，才会走到这里；错误的参数，才会调用
    // sss::util::join()，才会生成一个列表，才会抛出异常。
    // 当然不会再一次调用本函数。
    // 所以 sss::util::join() 的结果，不用static保存起来——另外，就算要保存，也
    // 只能保存到 RuleBase 结构体下面。
    std::ostringstream oss;
    oss << "input value `"  << val << "` not in set ["
        << sss::util::join(this->optional_values.begin(),
                           this->optional_values.end(), "|", MapValue_get_first())
        << "]" << std::endl;
    throw CMLParser::Exception(oss.str());
}

bool    RuleBase::add_option(const char * val, int id)
{
    if (!val)
        return false;
    optional_values_t::iterator it = this->optional_values.find(val);
    if (it != this->optional_values.end())
        return false;
    this->optional_values.insert(it, std::make_pair(val, id));
    // this->optional_values.insert(it, std::make_pair<std::string, int>(std::string(val), id));
    return true;
}

//! RuleSingleValue
RuleSingleValue::RuleSingleValue(const std::string& msg_on_empty)
: RuleBase(msg_on_empty), is_assigned(false), value(0)
{
}

RuleSingleValue::RuleSingleValue()
: is_assigned(false), value(0)
{
}

RuleSingleValue::~RuleSingleValue()
{}

bool RuleSingleValue::assign(const char * val)
{
    if (!this->is_assigned)
    {
        this->RuleBase::assert_valid_value(val);
        this->value = val;
        this->is_assigned = true;
        return true;
    }
    else
    {
        std::ostringstream oss;
        oss << "RuleSingleValue: dumplicate value `" << val << "` found.";
        throw CMLParser::Exception(oss.str());
    }
}

size_t RuleSingleValue::size() const
{
    return this->is_assigned ? 1 : 0;
}
const char * RuleSingleValue::get(int /* index */) const
{
    return this->value;
}

//! RuleMultyValue
RuleMultyValue::RuleMultyValue(const std::string& msg_on_empty)
: RuleBase(msg_on_empty)
{
}

RuleMultyValue::RuleMultyValue()
{ }

RuleMultyValue::~RuleMultyValue()
{ }

size_t RuleMultyValue::size() const
{
    return this->ctainer.size();
}

bool RuleMultyValue::assign(const char * val)
{
    this->RuleBase::assert_valid_value(val);
    this->ctainer.push_back(val);
    return true;
}

bool RuleMultyValue::has_value(const std::string& val)
{
    for (std::vector<const char *>::iterator it = this->ctainer.begin();
         it != this->ctainer.end();
         ++it)
    {
        if ((*it) == val)
        {
            return true;
        }
    }
    return false;
}

const char * RuleMultyValue::get(int index) const
{
    return (index >= 0 && index < int(this->ctainer.size())) ? this->ctainer[index] : 0;
}

ParseBase::ParseBase()
{ }

ParseBase::~ParseBase()
{ }

// opt:
//      匹配项关键字
// type:
//      决定匹配规则，读入参数策略
// rule:
//      决定保存值的策略
bool ParseBase::add_rule(const std::string& opt,
                         ParseBase::rule_param_t type,
                         RuleBase & rule)
{
    // FIXME 为什么 使用 typeid的策略失败了？
    //SSS_LOG_DEBUG("typeid(rule) %s\n", typeid(rule).name());
    //SSS_LOG_DEBUG("typeid(RuleMultyValue()) %s\n", typeid(RuleMultyValue()).name());
    // typeid(rule) N3sss9CMLParser14RuleMultyValueE
    // typeid(RuleMultyValue()) FN3sss9CMLParser14RuleMultyValueEvE
    //if (type == ParseBase::r_option && typeid(rule) == typeid(RuleMultyValue()))
    //
    // NOTE 原因，参见：
    //! http://en.wikipedia.org/wiki/Name_mangling
    //
    //即：typeid::name() 是实现相关的。而g++的做法是直接 返回 mangling 后的名字。
    // 可以用 binutil::c++filt 工具，解析这个名字；比如：
    // Administrator@wj-5 /D/Program/MSYS/extra/sss/include/sss
    // $ c++filt.exe -t FN3sss9CMLParser14RuleMultyValueEvE
    // sss::CMLParser::RuleMultyValue ()()      -- 这里附加了两个括号，可能指的是临时对象的副本
    //
    // Administrator@wj-5 /D/Program/MSYS/extra/sss/include/sss
    // $ c++filt.exe -t N3sss9CMLParser14RuleMultyValueE
    // sss::CMLParser::RuleMultyValue
    //
    // Complex example
    //
    // For a more complex example, we'll consider an example of a real-world
    // name mangling implementation: that used by GNU GCC 3.x, and how it
    // mangles the following example class. The mangled symbol is shown below
    // the respective identifier name.
    //
    // namespace wikipedia
    // {
    //    class article
    //    {
    //    public:
    //       std::string format (void);
    //          /* = _ZN9wikipedia7article6formatEv */
    //
    //       bool print_to (std::ostream&);
    //          /* = _ZN9wikipedia7article8print_toERSo */
    //
    //       class wikilink
    //       {
    //       public:
    //          wikilink (std::string const& name);
    //             /* = _ZN9wikipedia7article8wikilinkC1ERKSs */
    //       };
    //    };
    // }
    //
    // All mangled symbols begin with _Z (note that an underscore followed by a
    // capital is a reserved identifier in C and C++, so conflict with user
    // identifiers is avoided); for nested names (including both namespaces and
    // classes), this is followed by N, then a series of <length, id> pairs (the
    // length being the length of the next identifier), and finally E. For
    // example, wikipedia::article::format becomes
    //
    //  _ZN·9wikipedia·7article·6format·E
    //
    // For functions, this is then followed by the type information; as format()
    // is a void function, this is simply v; hence:
    //
    //  _ZN·9wikipedia·7article·6format·E·v
    //
    // For print_to, a standard type std::ostream (or more properly
    // std::basic_ostream<char, char_traits<char> >) is used, which has the
    // special alias So; a reference to this type is therefore RSo, with the
    // complete name for the function being:
    //
    //  _ZN·9wikipedia·7article·8print_to·E·RSo
    //
    // 而要在程序中，直接输出类型的 demangling 名字，可以使用 <cxxabi.h> 中的
    // abi::__cxa_demangle().
    //
    // "范例"
    // int status;
    // char *realname = abi::__cxa_demangle(typeid(obj).name(), 0, 0, &status);
    // std::cout << realname;
    // free(realname);
    //
    // 而且，要注意的是，上述方法，不方便封装到一个函数里面；因为函数，会改变类
    // 型；比方，原本是"值"；经过一层函数之后，会附加上"引用"属性。
    //
    if (type == ParseBase::r_option)
    {
        if (dynamic_cast<RuleMultyValue*>(&rule))
            throw CMLParser::Exception("RuleMultyValue cannot used with ParseBase::r_option!");
    }

    rule_map_t::iterator it = this->rules.find(opt);
    // FIXME 同一个规则前导 opt ，不能与不同的rule进行绑定！
    // 必须在编译的时候——或者运行的时候，就检测到这一点！
    if (it == this->rules.end())
    {
        this->rules.insert(it, std::make_pair(opt, std::make_pair(type, &rule)));
        std::set<RuleBase*>::iterator rule_iter = this->rules_set.find(&rule);
        if (rule_iter == this->rules_set.end())
        {
            this->rules_set.insert(rule_iter, &rule);
        }
        return true;
    }
    else
    {
        throw CMLParser::Exception("command-line rule `" + it->first + "`, detecte dumplicate added.");
        return false;
    }
}

bool ParseBase::del_rule(const std::string& opt)
{
    rule_map_t::iterator it = this->rules.find(opt);
    if (it == this->rules.end())
    {
        return false;
    }
    else
    {
        this->rules.erase(it);
        return true;
    }
}

// 提供给这个子函数的参数太多了，不利于过程的复用。
// 应该提供一个半开半闭区间。
int ParseBase::do_parse_current(char ** argv_b, char ** argv_e)
{
    int current_used = 0;                       // 消耗参数（字符串）的数量

    // 因为是upper_bound 所以，不用判断返回值是否等于this->rules.end();
    // NOTE upper_bound() 之后，再一个 -- 操作；这两个操作要能运行有前提的，就
    // 是：
    //   !this->rules.empty();
    // 这个条件，是调用 do_parse_current 的上层函数保证的！
    //
    // NOTE 不过；因为要"--"操作，所以要检查是否是begin()!
    rule_map_t::iterator it = this->rules.upper_bound(*argv_b);
    if (it == this->rules.begin()) {
        return current_used;
    }
    --it;
    const std::string & param = it->first;
    RuleBase & rule = *it->second.second;
    try
    {
        switch (it->second.first)
        {
        case ParseBase::r_option:
            if (param == *argv_b)
            {
                rule.assign(0);
                current_used = 1;
                SSS_LOG_DEBUG("parse r_option %s\n", *argv_b);
            }
            break;

        case ParseBase::r_prefix:
            if (param == std::string(*argv_b, 0, param.length()))
            {
                rule.assign(*argv_b + param.length());
                current_used = 1;
                SSS_LOG_DEBUG("parse r_prefix %s %s\n",
                              param.c_str(),
                              *argv_b + param.length());
            }
            break;

        case ParseBase::r_parameter:
            if (param == *argv_b)
            {
                if (++argv_b == argv_e)
                {
                    throw CMLParser::Exception("one parameter required after `" +
                                               param + ".");
                }
                rule.assign(*argv_b);
                current_used = 2;
                SSS_LOG_DEBUG("parse r_parameter %s %s\n", argv_b[-1], *argv_b);
                SSS_LOG_DEBUG("parse retrive : r_parameter %s %s\n",
                              param.c_str(),
                              rule.get(rule.size() - 1));
            }
            break;
        }
    }
    catch(CMLParser::Exception& e)
    {
        SSS_LOG_ERROR("when processing command-line parameter %s.\n", it->first.c_str());
        throw;
    }
    return current_used;
}

void ParseBase::check_must_supply_options()
{
    std::ostringstream oss;
    for (std::set<RuleBase*>::iterator it = this->rules_set.begin();
         it != this->rules_set.end();
         ++it)
    {
        if ((*it)->must_supply && !(*it)->size())
        {
            oss << (*it)->message_on_empty << std::endl;
        }
    }

    if (!oss.str().empty()) {
        //throw CMLParser::Exception((*it)->message_on_empty);
        throw CMLParser::Exception(oss.str());
    }
}

Exclude::Exclude()
{ }

Exclude::~Exclude()
{ }

int Exclude::parse(int & argc, char **& argv)
{
    // Exclude 策略下，如果rules 是空的，那么直接返回。
    // 就是说，所有命令行参数，还给库用户
    if (this->rules.empty())
        return 0;

#if 1
    // return this->continue_parse(argc - 1, argv + 1);
    int     t_argc = argc - 1;
    char ** t_argv = argv + 1;
    int cnt = this->continue_parse(t_argc, t_argv);
    // std::cout << __func__ << " Consume " << cnt << std::endl;
    if (cnt) {
        argc = t_argc + 1;
        t_argv--;
        t_argv[0] = argv[0];
        argv = t_argv;
    }
    return cnt;
    // return ;
#else
    std::vector<char *> argv_buffer;
    argv_buffer.reserve(argc);
    argv_buffer.push_back(argv[0]);

    // argv[]的第一个指针，默认是程序名字；忽略之。
    for (int i = 1; i < argc;)
    {
        int current_used = this->ParseBase::do_parse_current(argv + i, argv + argc);
        // 如果没有消耗参数
        if (!current_used)
        {
            argv_buffer.push_back(argv[i]);
            i += 1;
        }
        else
        {
            i += current_used;
        }
    }
    int all_used = argc - int(argv_buffer.size());
    if (argv_buffer.size() != size_t(argc))
    {
        argc = argv_buffer.size();
        for (size_t i = 1; i < argv_buffer.size(); ++i)
        {
            argv[i] = argv_buffer[i];
        }
    }
    this->ParseBase::check_must_supply_options();
    return all_used;
#endif
}

// NOTE 要确保，被本函数过滤之后，剩余的参数，和后续（如果还有的话），中间是连续的
// 即，我需要把数据，往后面挪！
int Exclude::continue_parse(int & argc, char ** & argv)
{
    int i = 0;
    int j = 0;
    while (i < argc) {
        int current_used = this->ParseBase::do_parse_current(argv + i, argv + argc);
        if (!current_used) {
            if (i != j) {
                argv[j] = argv[i];
            }
            i++;
            j++;
        }
        else {
            // for (int k = 0; k < current_used; ++k) {
            //     std::cout << "Consume " << i + k << " " << argv[i + k] << std::endl;
            // }
            i += current_used;
        }
    }
    if (i != j) {
        for (int k = 1; k <= j; ++k) {
            argv[i - k] = argv[j - k];
        }
        argc -= i - j;
        argv += i - j;
    }
    this->ParseBase::check_must_supply_options();
    return i - j;
}

Include::Include()
{ }

Include::~Include()
{ }

int Include::parse(int & argc, char **& argv)
{
    // Include 策略下，如果rules 是空的，就是说，不允许输入命令行参数！
    // 此时，如果argc >= 2，则说明至少输入了一个参数。
    if (this->rules.empty() && argc >= 2)
    {
        throw CMLParser::Exception("no command-line parameter wanted!");
    }
#if 1
    int  t_argc = argc - 1;
    char ** t_argv = argv + 1;
    int used_cnt = this->continue_parse(t_argc, t_argv);
    if (t_argc) {
        throw CMLParser::Exception("unexpected command line option: " +
                                   std::string(t_argv[0]));
    }

    t_argv--;
    t_argv[0] = argv[0];
    argc = t_argc + 1;
    argv = t_argv;

    return used_cnt;
#else
    // argv[]的第一个指针，默认是程序名字；忽略之。
    for (int i = 1; i < argc;)
    {
        int current_used = this->ParseBase::do_parse_current(argv + i, argv + argc);
        // 如果没有消耗参数
        if (!current_used)
        {
            throw CMLParser::Exception("unexpected command line option: " +
                                       std::string(argv[i]));
        }
        i += current_used;
    }
    this->ParseBase::check_must_supply_options();
    return argc - 1;
#endif
}

// 如果参数还有剩余，则不再抛出异常；
// 当然，还是会检测必须提供的参数；
int Include::continue_parse(int & argc, char ** & argv)
{
    int i = 0;
    while (i < argc)
    {
        int current_used = this->ParseBase::do_parse_current(argv + i, argv + argc);
        // 如果没有消耗参数
        if (!current_used)
        {
            break;
        }
        i += current_used;
    }
    this->ParseBase::check_must_supply_options();
    sss::CMLParser::shift(argc, argv, i);
    return i;
}

} }
