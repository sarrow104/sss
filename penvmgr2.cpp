#include "penvmgr2.hpp"

#include <assert.h>
#include <sstream>
#include <stdexcept>
#include <string>

#include <sss/path.hpp>
#include <sss/ps.hpp>
#include <sss/time.hpp>
#include <sss/util/Parser.hpp>
#include <sss/util/PostionThrow.hpp>
#include <sss/utlstring.hpp>
#include <sss/raw_print.hpp>

#ifdef __WIN32__
#include <sss/environ.hpp>
#include <sss/util/Escaper.hpp>
#endif

namespace {
const char* penvmg_script_path = "_penvmg_script_path_";
const char* g_shellscript_workdir = "g.shellscript_workdir";
}  // namespace

namespace sss {

PenvMgr2::PenvMgr2(PenvMgr2* parent) : _parent(parent)
{
    SSS_LOG_DEBUG("%p\n", this);
}

PenvMgr2::~PenvMgr2() {}
PenvMgr2::iterator PenvMgr2::begin() { return _env.begin(); }
PenvMgr2::iterator PenvMgr2::end() { return _env.end(); }
PenvMgr2::const_iterator PenvMgr2::begin() const { return _env.begin(); }
PenvMgr2::const_iterator PenvMgr2::end() const { return _env.end(); }
void PenvMgr2::print(std::ostream& o) const
{
    for (PenvMgr2::const_iterator it = this->_env.begin();
         it != this->_env.end(); ++it) {
        o << it->first << " = `";
        it->second.print_body(o);
        o << "`" << std::endl;
    }
}

/**
 * @brief dump 所有变量；本质就是对所有变量进行求值；
 *        "可能的优化"
 *           应该复用 depend_checker2_t；因为在对某一个变量求值的过程中，已经对
 *           它所依赖的其他值，进行了求值；
 *        另外，此种行为，可能导致循环
 *
 * @param[out] out
 */
void PenvMgr2::dump2map(std::map<std::string, std::string>& out) const
{
    // NOTE 我的 PenvMgr2对象，是单根的链表形式。
    // 需要注释的是，根据变量名，检索变量的时候，"子"会逻辑上覆盖父的变量；
    // 因此，如果要实现"dump"动作，我需要从父到子，依次进行；
    // 由于需要从根开始。
    std::vector<const PenvMgr2*> ancestors;
    const PenvMgr2* p_penvmg2 = this;
    while (p_penvmg2) {
        ancestors.push_back(p_penvmg2);
        p_penvmg2 = p_penvmg2->has_parent() ? &p_penvmg2->parent() : 0;
    }
    for (size_t i = ancestors.size(); i != 0; --i) {
        for (PenvMgr2::const_iterator it = ancestors[i - 1]->_env.begin();
             it != ancestors[i - 1]->_env.end(); ++it) {
            out[it->first] = ancestors[i - 1]->get(it->first);
        }
    }
}

void PenvMgr2::var_body_t::print_body(std::ostream& o) const
{
    if (this->isFunction()) {
        o << "{userdef funciton}";
    }
    for (expression_t::const_iterator it = this->second.begin();
         it != this->second.end(); ++it) {
        o << *it;
    }
}

//----------------------------------------------------------------------

typedef std::string::const_iterator iter_t;
typedef sss::util::StringSlice<iter_t> StringSlice_t;
typedef sss::util::Parser<iter_t> Parser_t;
typedef Parser_t::Rewinder Rewinder_t;

namespace {
/**
 * @brief 变量定义体，解析器；
 *      将字符串形式的"变量定义体"，解析重组为
 *      expression_t 和 var_list_t 量部分；
 */
class env_parser {
public:
    env_parser(PenvMgr2::var_body_t& vb)
        : _varlist(vb.first), _expr(vb.second), _p_str(0)
    {
    }
    ~env_parser() {}
public:
    typedef std::string::const_iterator iter_t;
    bool parse(const std::string& expr)
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        this->_varlist.clear();
        this->_expr.clear();
        this->_expr.m_data.assign(expr);
        this->_p_str = &expr;
        iter_t s_beg = expr.begin();
        return parse_expr(s_beg, expr.end());
    }

    bool parseRawStr(const std::string& expr)
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        _varlist.clear();
        _expr.clear();
        _expr.m_data.assign(expr);
        _expr.push_back(0, expr.length());
        return true;
    }

    bool parseFunc(PenvMgr2::expression_t::FuncT func,
                   PenvMgr2::expression_t::FuncParamT para)
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        _varlist.clear();
        _expr.clear();

        _expr.m_func = func;
        _expr.m_func_param = para;
        return true;
    }

    static bool is_var_refer(const StringSlice_t& var)
    {
        StringSlice_t::iterator s_beg = var.begin();
        return env_parser::parse_Var(s_beg, var.end()) && s_beg == var.end();
    }

    static bool is_var_refer(const std::string& var)
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        std::string::const_iterator s_beg = var.begin();
        std::string::const_iterator s_end = var.end();
        return env_parser::parse_Var(s_beg, s_end) && s_beg == s_end;
    }

    // expr ::= char* (Var expr)?
    // =>
    // expr ::= (char* Var?) expr
    // ==>
    // expr ::= (char* Var?)+
    // ==>
    // expr ::= ('$$' | char)+ ....
    //
    // expr ::= ((char - '$$' - Var)* Var | '$$' EmptyVar)*
    bool parse_expr(iter_t& s_beg, iter_t s_end)
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        if (s_beg == s_end) {
            this->_expr.push_back(*this->_p_str, s_beg, s_end);
        }
        else {
            while (s_beg != s_end) {
                bool is_end_with_escape = false;
                size_t var_name_length = 0u;
                {
                    Rewinder_t r(s_beg);
                    while (s_beg != s_end) {
                        Rewinder_t r1(s_beg);
                        if (parse_Var(s_beg, s_end)) {
                            var_name_length = r1.distance();
                            // TODO 这里应该记录 var_name 长度；以避免重复解析；
                            r1.commit(false);
                            break;
                        }
                        else {
                            if (sss::is_begin_with(s_beg, s_end, "$$")) {
                                is_end_with_escape = true;
                                s_beg += 2;
                                r1.commit(true);
                                break;
                            }
                            else {
                                s_beg++;
                                r1.commit(true);
                            }
                        }
                    }
                    // NOTE 空串 ""，有没有问题？
                    if (r.distance()) {
                        r.commit(true);
                        StringSlice_t slice = r.getSlice();
                        if (is_end_with_escape) {
                            slice.shrink(0, 1);  // "$$" -> "$";
                        }
                        this->_expr.push_back(*this->_p_str, slice);  //  1st
                        if (is_end_with_escape) {
                            slice.clear();                                // ""
                            this->_expr.push_back(*this->_p_str, slice);  // 2nd
                        }
                        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, slice);
                    }
                }

                {
                    Rewinder_t r2(s_beg);
                    std::advance(s_beg, var_name_length);
                    // r2.commit(parse_Var(s_beg, s_end));
                    if (r2.distance()) {
                        r2.commit(true);
                        StringSlice_t var_name = r2.getSlice();
                        if (var_name[1] == '{') {
                            var_name.shrink(2, 1);  // strip "${" and "}"
                        }
                        else {
                            var_name.shrink(1, 0);  // strip "$"
                        }
                        // NOTE 确保 满足 [串 变量]... 的形式; 0-based
                        if (!(this->_expr.size() & 1u)) {
                            this->_expr.push_back(*this->_p_str, s_beg, s_beg);
                        }
                        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var_name);
                        // std::string var_name = PenvMgr2::refer2name(.str());
                        this->_expr.push_back(*this->_p_str, var_name);
                        this->_varlist.insert(var_name.str());
                    }
                }
            }
        }
        assert(s_beg == s_end);
        return true;
    }

    // shell
    // ${t.(`command par par`)} // dot 是为了与 egs匹配
    static bool parse_Var_name(iter_t& it_beg, iter_t it_end)
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        Rewinder_t r1(it_beg);
        Rewinder_t r2(it_beg);
#ifdef PENVMGR2_SUPPORT_SHELL_CMD
        Rewinder_t r3(it_beg);
#endif

        StringSlice_t id(it_beg, it_beg);
        r1.commit(
            Parser_t::parseCIdentifier(it_beg, it_end, id) &&
            ((r2.begin() &&
              r2.commit(id.length() == 1 && std::strchr("egs", *id.begin()) &&
                        Parser_t::parseChar(it_beg, it_end, '.') &&
                        Parser_t::parseCIdentifier(it_beg, it_end, id)))
#ifdef PENVMGR2_SUPPORT_SHELL_CMD
             || (r3.begin() &&
                 r3.commit(id == "t" &&
                           Parser_t::parseSequence(it_beg, it_end, ".(`") &&
                           Parser_t::parseAfterSequence(it_beg, it_end, "`)")))
#endif
             || true));
        return r1.is_commited();
    }

    // Var ::= '$' name | "${" name '}'
    static bool parse_Var(iter_t& it_beg, iter_t it_end)
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        Rewinder_t r1(it_beg);
        Rewinder_t r2(it_beg);

        bool has_brase = false;

        // char prefix = '\0';
        StringSlice_t id(it_beg, it_beg);

        r1.commit(
            Parser_t::parseChar(it_beg, it_end, '$') &&
            ((has_brase = Parser_t::parseChar(it_beg, it_end, '{')) || true) &&
            r2.begin() && r2.commit(parse_Var_name(it_beg, it_end)) &&
            ((has_brase && Parser_t::parseChar(it_beg, it_end, '}')) ||
             !has_brase));
        return r1.is_commited();
    }

    // g.xyz
    // s.abc
    // var
    static PenvMgr2::var_type_t get_var_type(const std::string& str)
    {
        if (PenvMgr2::is_var(str)) {
            return str.length() >= 2 && str[1] == '.'
                       ? PenvMgr2::var_type_t(str[0])
                       : PenvMgr2::TYPE_NORMAL;
        }
        return PenvMgr2::TYPE_UNKNOWN;
    }

private:
    PenvMgr2::var_list_t& _varlist;
    PenvMgr2::expression_t& _expr;
    const std::string* _p_str;
};
}

PenvMgr2& PenvMgr2::getGlobalEnv()
{
    PenvMgr2* p = this;
    while (p->_parent) {
        p = p->_parent;
    }
    return *p;
}

const PenvMgr2& PenvMgr2::getGlobalEnv() const
{
    const PenvMgr2* p = this;
    while (p->_parent) {
        p = p->_parent;
    }
    return *p;
}

bool PenvMgr2::set(std::string var, const std::string& expr)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, expr);

    var_type_t type = env_parser::get_var_type(var);
    switch (type) {
        case TYPE_SYSTEM:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is a system-style var name; cannot modified");
            break;

        case TYPE_GLOBAL:
            var = var.substr(2);
            if (this->_parent) {
                return this->getGlobalEnv().set(var, expr);
            }
            break;

        case TYPE_OSENV:
            SSS_POSITION_THROW(
                std::runtime_error, "PenvMgr2: `", var,
                "` is a OS-environment var name; cannot modified");
            break;

        case TYPE_SHELL:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is a shell-cmmand; cannot modified");
            break;

        case TYPE_NORMAL:
            break;

        default:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is not a valid var name");
            break;
    }
    env_t::iterator it = this->_env.find(var);
    if (it == this->_env.end()) {
        var_body_t vb;
        this->_env.insert(it, std::make_pair(var, vb));
    }
    bool ret = (env_parser(this->_env[var]).parse(expr));
    if (!ret) {
        SSS_POSITION_THROW(std::runtime_error, " (", sss::raw_string(var), ", ",
                          sss::raw_string(expr), ") parse failed.");
    }
    return true;
}

bool PenvMgr2::setRawStr(std::string var, const std::string& expr)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, expr);

    var_type_t type = env_parser::get_var_type(var);
    switch (type) {
        case TYPE_SYSTEM:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is a system-style var name; cannot modified");
            break;

        case TYPE_GLOBAL:
            var = var.substr(2);
            if (this->_parent) {
                return this->getGlobalEnv().setRawStr(var, expr);
            }
            break;

        case TYPE_OSENV:
            SSS_POSITION_THROW(
                std::runtime_error, "PenvMgr2: `", var,
                "` is a OS-environment var name; cannot modified");
            break;

        case TYPE_SHELL:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is a shell-cmmand; cannot modified");
            break;

        case TYPE_NORMAL:
            break;

        default:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is not a valid var name;");

            break;
    }
    env_t::iterator it = this->_env.find(var);
    if (it == this->_env.end()) {
        var_body_t vb;
        this->_env.insert(it, std::make_pair(var, vb));
    }
    bool ret = (env_parser(this->_env[var]).parseRawStr(expr));
    if (!ret) {
        SSS_POSITION_THROW(std::runtime_error, " (", sss::raw_string(var), ", ",
                          sss::raw_string(expr), ") parse failed.");
    }
    return true;
}

bool PenvMgr2::set(std::string var, expression_t::FuncT func,
                   expression_t::FuncParamT param)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, func);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, param);

    var_type_t type = env_parser::get_var_type(var);
    switch (type) {
        case TYPE_SYSTEM:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is a system-style var name; cannot modified");
            break;

        case TYPE_GLOBAL:
            var = var.substr(2);
            if (this->_parent) {
                return this->getGlobalEnv().set(var, func, param);
            }
            break;

        case TYPE_OSENV:
            SSS_POSITION_THROW(
                std::runtime_error, "PenvMgr2: `", var,
                "` is a OS-environment var name; cannot modified");
            break;

        case TYPE_SHELL:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is a shell-cmmand; cannot modified");
            break;

        case TYPE_NORMAL:
            break;

        default:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is not a valid var name;");
            break;
    }
    env_t::iterator it = this->_env.find(var);
    if (it == this->_env.end()) {
        var_body_t vb;
        this->_env.insert(it, std::make_pair(var, vb));
    }
    bool ret = (env_parser(this->_env[var]).parseFunc(func, param));
    if (!ret) {
        SSS_POSITION_THROW(std::runtime_error, " (`", var, "`, `", func, "`, `",
                          param, "`) parse failed.");
    }
    return true;
}

/**
 * @brief 循环依赖异常
 */
class ExceptionDependLoop : public std::runtime_error {
public:
    explicit ExceptionDependLoop(const std::string& msg)
        : std::runtime_error(msg){};
    ~ExceptionDependLoop() throw() {}
};

/**
 * @brief 依赖状态变量类型；用来记录所依赖变量的求值状态；
 */
struct depend_t {
    depend_t() : _ok(false) {}
    explicit depend_t(const std::string& value) : _ok(true), _value(value) {}
    ~depend_t() {}
    bool _ok;
    std::string _value;

    void print(std::ostream& o) const
    {
        if (this->_ok) {
            o << "\"" << this->_value << "\"";
        }
        else {
            o << 0;
        }
    }

public:
};

std::ostream& operator<<(std::ostream& o, const depend_t& d)
{
    d.print(o);
    return o;
}

/**
 * @brief 依赖检查器；
 *        在对 "$环境变量" 求值的过程中，不可避免会遇到变量之间的相互依赖问题；
 *        比如 a->b; b->c; a->c；
 *        如果没有一个中间的机制，我在对a求值的过程中，可能会需要两次对c求值；
 *        类似，还有如何避免循环依赖的问题；
 *        对此，我引入了 这个检查器类；
 *        在递归求值过程中，它可以记录中间计算过程，以避免重复求值；并且，在递
 *        归前后，对所依赖的变量，都会进行记录；如果发现出现了 a->b->a 的循环，
 *        会立即抛出异常。
 *
 *        后来，我引入了 ${t.(`echo $my_var`)} shell脚本类型变量；此时的循环依
 *        赖检查，就变得复杂了。此时，我额外引入了一个循环深度控制的概念，同样
 *        放进这个"检查类"里面。
 *
 *        当这个依赖检查器，检测到递归深度，超过某一个深度(比如500)，的时候，同
 *        样会抛出异常。
 */
class depend_checker2_t : private std::map<std::string, depend_t> {
    int m_depth;
    static const int max_depth = 500;

public:
    typedef std::map<std::string, depend_t> Base_t;
    typedef Base_t::const_iterator const_iterator;
    typedef Base_t::iterator iterator;

    using Base_t::begin;
    using Base_t::end;
    using Base_t::find;

    depend_checker2_t() : m_depth(0) {}
    ~depend_checker2_t() {}
public:
    void push()
    {
        if (m_depth++ > max_depth) {
            SSS_POSITION_THROW(std::runtime_error,
                              "depend_checker2_t max depth > ", max_depth);
        }
    }
    void pop()
    {
        if (m_depth-- < 0) {
            SSS_POSITION_THROW(std::runtime_error,
                              "depend_checker2_t max depth < 0 ");
        }
    }

    void put(const std::string& var)
    {
        assert(PenvMgr2::is_var(var));
        Base_t::iterator it = this->Base_t::find(var);
        if (it != this->Base_t::end() && !it->second._ok) {
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, it->first);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, it->second);
            SSS_POSITION_THROW(ExceptionDependLoop, var);
        }
        this->Base_t::insert(it, std::make_pair(var, depend_t()));
    }

    void put(const std::string& var, const std::string& value)
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        SSS_LOG_DEBUG("(%s, %s)\n", var.c_str(), value.c_str());
        assert(PenvMgr2::is_var(var));
        Base_t::iterator it = this->Base_t::find(var);
        if (it == this->Base_t::end()) {
            this->Base_t::insert(it, std::make_pair(var, depend_t(value)));
        }
        else {
            if (it->second._ok && it->second._value != value) {
                SSS_LOG_ERROR("re-assign `%s` \"%s\" -> \"%s\"\n",
                              it->first.c_str(), it->second._value.c_str(),
                              value.c_str());
                SSS_LOG_EXPRESSION(sss::log::log_ERROR, it->first);
                SSS_LOG_EXPRESSION(sss::log::log_ERROR, it->second);
                SSS_POSITION_THROW(ExceptionDependLoop, var);
            }
            it->second._ok = true;
            it->second._value = value;
        }
    }

    bool pull(const std::string& var, std::string& value)
    {
        assert(PenvMgr2::is_var(var));
        Base_t::iterator it = this->Base_t::find(var);
        if (it == this->Base_t::end()) {
            return false;
        }
        else {
            if (it->second._ok) {
                value = it->second._value;
            }
            return it->second._ok;
        }
    }

    void print(std::ostream& o) const
    {
        o << "[";
        for (Base_t::const_iterator it = this->Base_t::begin();
             it != this->Base_t::end(); ++it) {
            o << it->first << ":" << it->second << "\";";
        }
        o << "]";
    }

    void dump2map(std::map<std::string, std::string>& var_map)
    {
        for (Base_t::const_iterator it = this->Base_t::begin();
             it != this->Base_t::end(); ++it) {
            if (it->second._ok) {
                var_map[it->first] = it->second._value;
            }
        }
    }

private:
};

/**
 * @brief helper class for depend_checker2_t::depth_check
 */
class dc_depth_wrapper {
public:
    dc_depth_wrapper(depend_checker2_t& dc) : m_ref_dc(dc) { m_ref_dc.push(); }
    ~dc_depth_wrapper() { m_ref_dc.pop(); }
private:
    depend_checker2_t& m_ref_dc;
};

std::ostream& operator<<(std::ostream& o, const depend_checker2_t& dc)
{
    dc.print(o);
    return o;
}

std::string PenvMgr2::get(std::string var) const
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    var_type_t type = env_parser::get_var_type(var);
    assert(type);

    switch (type) {
        case TYPE_GLOBAL:
            var = var.substr(2);
            if (this->_parent) {
                return this->getGlobalEnv().get(var);
            }
            break;

        case TYPE_SYSTEM:
            var = var.substr(2);
            return this->getSystemVar(var);
            break;

        case TYPE_OSENV:
            var = var.substr(2);
            return this->getEnvVar(var);
            break;

        case TYPE_SHELL:
            // t.(`...`)
            return this->getShellComandFromVar(var);
            break;

        case TYPE_NORMAL:
            break;

        default:
            break;
    }
    std::string ret;
    // 先看变量是否存在；如果不存在，直接返回""；
    // 如果变量存在，则查看其var_body_t对象；
    // 此时，同时检测循环依赖问题；
    // 如何解决？需要额外创建一个(依赖->值)；并且，用额外的整数，标记其可用性；
    // 然后，这个对象引用传递给递归调用的函数；
    // 对于根据变量名求值，那么该变量名首先写入上述对象，并且标记为待求值状态；
    // 然后，根据其变量列表，挨个处理；
    // 如果递归调用过程中，出现对同一个变量求值，且值不一样，那么肯定有循环依赖！
    //
    // 内部调用 evaluator_impl 过程；该过程递归调用自己；
    //
    // impl 函数，递归传递一个依赖列表；它迭代的时候，检查，当前依赖是否与已经
    // 有的部分，重复了；如果有，说明依赖循环了

    if (!this->has(var)) {
        ret = "";
    }
    else {
        try {
            depend_checker2_t dc;
            dc.put(var);
            dc_depth_wrapper dcg(dc);
            ret = evaluator_impl(var, dc);
        }
        catch (ExceptionDependLoop& e) {
            SSS_POSITION_THROW(std::runtime_error, "ExceptionDependLoop ",
                              std::string(e.what()), "; when requre ", var);
        }
    }
    return ret;
}

// TODO
// 增加支持的 PenvMgr2系统变量
std::string PenvMgr2::getSystemVar(const std::string& var) const
{
    if (var == "timestamp") {
        std::string fmt = this->get("timestamp_fmt");
        if (fmt.empty()) {
            fmt.assign("%FT%T");
        }
        // std::cout << __FILE__ << ':' << __func__ << sss::time::strftime(fmt)
        // << std::endl;
        return sss::time::strftime(fmt);
    }
    else if (var == "cwd" || var == "getcwd") {
        return sss::path::getcwd();
    }
    else if (var == "bin" || var == "getbin") {
        return sss::path::getbin();
    }
    SSS_LOG_ERROR("var %s not supported!\n", var.c_str());
    return "";
}

std::string PenvMgr2::getEnvVar(const std::string& var) const
{
    const char* e = ::getenv(var.c_str());
    return e ? e : "";
}

std::string PenvMgr2::getShellComandFromVar(const std::string& var,
                                            depend_checker2_t& dc) const
{
    std::string cmd(var, 4, var.length() - 6);
// static const char * err_to_null = " 2>/dev/null";
// if (!sss::is_end_with(cmd, err_to_null)) {
//     cmd.append(err_to_null);
// }
#ifdef __WIN32__
    const char* shell_path = sss::env::get("SHELL");
    if (shell_path) {
        // FIXME escape or quote
        cmd = shell_path + std::string(" -c ") + sss::util::dquote_copy(cmd);
    }
#endif
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, cmd);

#if 0
    sss::ps::StringPipe ps;
    ps << cmd;
    std::string out = ps.run();
    sss::rtrim(out);
    return out;
#else
    std::string script_wd = ".";
    std::map<std::string, std::string> script_env;
    if (this->has(g_shellscript_workdir) &&
        this->get(g_shellscript_workdir) != "") {
        script_wd = sss::path::full_of_copy(this->get(g_shellscript_workdir));
    }
    else if (this->has(penvmg_script_path)) {
        script_wd = sss::path::dirname(this->get(penvmg_script_path));
    }
    if (script_wd != "." &&
        sss::path::file_exists(script_wd) != sss::PATH_TO_DIRECTORY) {
        script_wd = ".";
    }
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, script_wd);
    // FIXME
    // 如果，某变量的定义，就是shell形式。
    // 在dump2map的时候，这个变量会导致无穷递归——会dump自己；
    // 有两个解决方案；
    // 1. 遇到shell变量的求值，先从env中扣除这个变量。
    // 2. 或者， 解析这个shell命令行，它所依赖的环境变量。然后，只dump，这个命
    // 令行，所依赖的变量即可。
    //
    // 如果，再出现循环依赖，就是用户的问题了——检测依赖，确实麻烦。
    // 第一个方案可行不？
    // 如果 env.set("var", "${t.(`echo $var2`)}${var3}")
    // 即，var依赖于 1. 一个shell值——shell值依赖var2；2. var3；此时，就不能简
    // 单的将var从env中扣除。
    //
    // 还有一个办法，是从depend_checker2_t& dc参数着手。evaluator_impl()函数，
    // 会递归调用，并用这个dc，保存依赖关系(变量临时值)，以避免，对某一个变量，
    // 进行重复计算。
    //
    // 即，可以在调用 getShellComandFromVar()
    // 函数的时候，也传入这个dc，并避免导出重复的值？
    //
    // 另外，还应该假如深度控制的保险。
    // 多提供一个深度参数，比如，就保存在 dc对象中；进出一次
    // evaluator_impl，就加减一次；
    // 如果，超出"熔断"值，就抛出一个异常。

    var_body_t vb;
    if (!env_parser(vb).parse(cmd)) {
        SSS_POSITION_THROW(std::runtime_error, " (" , sss::raw_string(cmd) , ") parse failed.");
    }
    // 即， cmd 串，依赖于 变量形参列表 vb.first
    for (var_list_t::const_iterator it_var = vb.first.begin();
         it_var != vb.first.end(); ++it_var) {
        dc_depth_wrapper dcg(dc);
        dc.put(*it_var, this->evaluator_impl(*it_var, dc));
    }
    dc.dump2map(script_env);
    std::string ret = sss::ps::PipeRun(cmd, script_wd, script_env);

    // NOTE
    // 就执行shell脚本而言；获得的输出，一般是多行；并且末尾会附带一个换行符。
    // 而这个换行符，一般情况下，是不需要的；
    if (!ret.empty()) {
        if (sss::is_end_with(ret, "\n")) {
            ret.resize(ret.length() - 1);
        }
        else if (sss::is_end_with(ret, "\r\n")) {
            ret.resize(ret.length() - 2);
        }
    }
    return ret;
#endif
}

std::string PenvMgr2::getShellComandFromVar(const std::string& var) const
{
    depend_checker2_t dc;
    return this->getShellComandFromVar(var, dc);
}

std::string PenvMgr2::get_expr(const std::string& expr) const
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, expr);
    std::string ret;

    var_body_t vb;

    if (!env_parser(vb).parse(expr)) {
        SSS_POSITION_THROW(std::runtime_error, " (", sss::raw_string(expr), ") parse failed.");
    }

    try {
        // TODO
        // 在本层，记录引用过哪些变量。
        // 并输出
        depend_checker2_t dc;
        for (var_list_t::const_iterator it_var = vb.first.begin();
             it_var != vb.first.end(); ++it_var) {
            dc_depth_wrapper dcg(dc);
            dc.put(*it_var, this->evaluator_impl(*it_var, dc));
        }
        ret = PenvMgr2::generate(vb, dc);
    }
    catch (ExceptionDependLoop& e) {
        SSS_POSITION_THROW(std::runtime_error, "ExceptionDependLoop ", e.what(),
                          "; when requre ", expr);
    }
    return ret;
}

std::string PenvMgr2::get_expr_file(const std::string& file_script) const
{
    PenvMgr2 inner(const_cast<PenvMgr2*>(this));
    std::string script_path = sss::path::full_of_copy(file_script);
    std::string script_value;
    if (sss::path::filereadable(script_path)) {
        inner.set(penvmg_script_path, script_path);
        std::string content;
        sss::path::file2string(script_path, content);
        script_value = inner.get_expr(content);
    }
    return script_value;
}

bool PenvMgr2::is_var_refer(const std::string& var)
{
    return env_parser::is_var_refer(var);
}

bool PenvMgr2::is_var_refer(const StringSlice_t& var)
{
    return env_parser::is_var_refer(var);
}

bool PenvMgr2::is_var(const std::string& var)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    std::string::const_iterator s_beg = var.begin();
    std::string::const_iterator s_end = var.end();
    bool ret = env_parser::parse_Var_name(s_beg, s_end) && s_beg == s_end;

    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, ret);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *s_beg);
    return ret;
}

std::string PenvMgr2::refer2name(const std::string& var)
{
    assert(PenvMgr2::is_var_refer(var));

    if (*var.rbegin() != '}') {
        return var.substr(1);
    }
    else {
        return var.substr(2, var.length() - 3);
    }
}

const PenvMgr2::var_body_t* PenvMgr2::find_body(const std::string& var) const
{
    const PenvMgr2::var_body_t* p_body = 0;
    const_iterator it = this->_env.find(var);
    if (it != this->_env.end()) {
        p_body = &(it->second);
    }
    else {
        if (this->has_parent()) {
            p_body = this->parent().find_body(var);
        }
    }
    return p_body;
}

// 2015-08-10
// 或者这样检测循环依赖：
//   记录 evaluator_impl
//   的调用条件！理论上，不能对同一个变量，两次通过evaluator_impl来求值！
//------------------------------------
// 求出var的值；
// 将var所依赖变量的值，都通过depend_checker2_t管理起来，以便在求值的过程中，找
// 出循环依赖额情况；
std::string PenvMgr2::evaluator_impl(std::string var,
                                     depend_checker2_t& dc) const
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    var_type_t type = env_parser::get_var_type(var);
    SSS_LOG_DEBUG("type of %s is %c\n", var.c_str(),
                  type ? type : var_type_t('0'));
    switch (type) {
        case TYPE_GLOBAL:
            var = var.substr(2);
            if (this->_parent) {
                return this->getGlobalEnv().evaluator_impl(var, dc);
            }
            break;

        case TYPE_SYSTEM:
            var = var.substr(2);
            return this->getSystemVar(var);
            break;

        case TYPE_OSENV:
            var = var.substr(2);
            return this->getEnvVar(var);
            break;

        case TYPE_SHELL:
            return this->getShellComandFromVar(var);
            break;

        case TYPE_NORMAL:
            break;

        default:
            assert(false);
            break;
    }

    std::string ret;

    const var_body_t* definition = this->find_body(var);
    if (!definition) {
        SSS_LOG_DEBUG("this->_env no %s\n", var.c_str());
        ret = "";
        // 对于不存在的变量，对依赖关系对象dc的变化是0；
        // 默认不存在的值，返回空串""；
    }
    else {
        if (definition->isFunction()) {
            return definition->evalFunction();
        }
        for (var_list_t::const_iterator it_var = definition->var_list().begin();
             it_var != definition->var_list().end(); ++it_var) {
            dc_depth_wrapper dcg(dc);
            dc.put(*it_var);
            std::string val = this->evaluator_impl(*it_var, dc);
            SSS_LOG_DEBUG("put %s, %s\n", (*it_var).c_str(), val.c_str());
            dc.put(*it_var, val);
        }
        ret = PenvMgr2::generate(*definition, dc);
    }
    return ret;
}

/**
 * @brief
 *
 * @param bd
 * @param dc
 *
 * @return
 */
std::string PenvMgr2::generate(const var_body_t& bd,
                               const depend_checker2_t& dc)
{
    const PenvMgr2::expression_t& expr(bd.second);
    std::ostringstream oss;
    int index = 0;
    for (PenvMgr2::expression_t::const_iterator it = expr.begin();
         it != expr.end(); ++it, ++index) {
        // std::cout << __LINE__ << ":" << __func__ << " index = " << index <<
        // std::endl;
        // NOTE 奇数位，变量
        // 偶数位，raw字符串
        if (index & 1) {
            const std::string& var_name = (*it).str();
            if (var_name.empty()) {
                continue;
            }
            depend_checker2_t::const_iterator it_dc = dc.find(var_name);
            assert(it_dc != dc.end() && it_dc->second._ok);
            oss << it_dc->second._value;
            // std::cout << __LINE__ << ":" << __func__ << " var_name = " <<
            // var_name << std::endl;
        }
        else {
            // std::cout << __LINE__ << ":" << __func__ << " raw_str = " << *it
            // << std::endl;
            oss << *it;
        }
    }
    return oss.str();
}

// FIXME 下面这几个函数，还得针对 type进行分解！
bool PenvMgr2::unset(std::string var)
{
    var_type_t type = env_parser::get_var_type(var);
    switch (type) {
        case TYPE_OSENV:
            SSS_POSITION_THROW(
                std::runtime_error, "PenvMgr2: `", var,
                "` is a OS-environment var name; cannot modified");
            break;

        case TYPE_GLOBAL:
            var = var.substr(2);
            if (this->_parent) {
                return this->getGlobalEnv().unset(var);
            }
            break;

        case TYPE_SYSTEM:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is a system-style var name; cannot modified");
            break;

        case TYPE_SHELL:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is a shell-cmmand; cannot modified");
            break;

        default:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is not a valid var name");
            break;
    }
    env_t::iterator it = this->_env.find(var);
    if (it != this->_env.end()) {
        this->_env.erase(it);
        return true;
    }

    if (this->has_parent()) {
        return this->parent().unset(var);
    }
    return false;
}

// FIXME 下面这几个函数，还得针对 type进行分解！
bool PenvMgr2::has(std::string var) const
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    var_type_t type = env_parser::get_var_type(var);
    switch (type) {
        case TYPE_OSENV: {
            return ::getenv(var.substr(2).c_str());
        } break;

        case TYPE_GLOBAL:
            var = var.substr(2);
            if (this->_parent) {
                return this->getGlobalEnv().has(var);
            }
            break;

        case TYPE_SYSTEM:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is a system-style var name;"
                              " do not support `has` operation.");
            break;

        case TYPE_SHELL:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is a shell-cmmand;"
                              " do not support `has` operation.");
            break;

        case TYPE_NORMAL:
            break;

        default:
            SSS_POSITION_THROW(std::runtime_error, "PenvMgr2: `", var,
                              "` is not a valid var name");
            break;
    }
    return this->_env.find(var) != this->_env.end() ||
           (this->has_parent() && this->parent().has(var));
}

// TODO 另外，最好再增加一个find()函数；返回一个自定义类型的iterator；这个类型
// 本身，包含一个PenvMgr2*和PenvMgr2::env::iterator；
//
// 需要用双重指针来定位具体的var！

const PenvMgr2& PenvMgr2::parent() const
{
    if (!this->has_parent()) {
        return *this;
    }
    return *this->_parent;
}

PenvMgr2& PenvMgr2::parent()
{
    if (!this->has_parent()) {
        return *this;
    }
    return *this->_parent;
}

void PenvMgr2::set_shellscript_workdir(const std::string& path)
{
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, path);
    // NOTE 应该动态去full！
    // 以免传入一个'.';
    // 或者临时修改目录等等。
    this->set(g_shellscript_workdir, path);
}

void PenvMgr2::unset_shellscript_workdir()
{
    this->unset(g_shellscript_workdir);
}
}

#if 0
void PenvMgr_test()
{
    PenvMgr2 env;
    std::cout << "-============================-" << std::endl;
    //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, PenvMgr2::is_var("a"));
    //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, PenvMgr2::is_var("$a"));

    //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, PenvMgr2::is_var_refer("a"));
    //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, PenvMgr2::is_var_refer("$a"));
    //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, PenvMgr2::is_var_refer("${a}"));

    env.set("suffix", ".epub.d");
    env.set("meta",   "META-INF");
    env.set("oebps",  "OPS");
    env.set("css",    "$oebps/css");
    env.set("img",    "$oebps/images");
    env.set("ncx",    "fb.ncx");
    env.set("opf",    "fb.opf");

    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get_expr(""));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get_expr("$suffix"));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get_expr("a$meta"));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get_expr("b$oebps"));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get_expr("b$css"));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get_expr("cde${img}"));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get_expr("cde${ncx}"));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get_expr("cde${opf} hello"));

    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get("suffix"));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get("meta"));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get("oebps"));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get("css"));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get("img"));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get("ncx"));
    SSS_LOG_EXPRESSION(sss::log::log_INFO, env.get("opf"));
}
#endif
