#include "penvmgr2.hpp"

#include <assert.h>
#include <sstream>
#include <string>
#include <stdexcept>

#include <sss/util/Parser.hpp>
#include <sss/ps.hpp>
#include <sss/time.hpp>
#include <sss/path.hpp>
#include <sss/util/PostionThrow.hpp>

// #define SSS_POSTION_THROW(type, msg) \
// {                       \
//     std::ostringstream oss;       \
//     oss << __FILE__ << "|" << __LINE__ << "|" << __func__ << ": " << msg; \
//     throw type(oss.str()); \
// }

namespace sss{

PenvMgr2::PenvMgr2(PenvMgr2 * parent)
    : _parent(parent)
{
    SSS_LOG_DEBUG("%p\n", this);
}

PenvMgr2::~PenvMgr2()
{
}

// PenvMgr2::PenvMgr2(const PenvMgr2& ref)
//     : _env(ref._env), _parent(ref._parent)
// {
// }

PenvMgr2::iterator PenvMgr2::begin()
{
    return _env.begin();
}

PenvMgr2::iterator PenvMgr2::end()
{
    return _env.end();
}

PenvMgr2::const_iterator PenvMgr2::begin() const
{
    return _env.begin();
}

PenvMgr2::const_iterator PenvMgr2::end() const
{
    return _env.end();
}

void PenvMgr2::print(std::ostream& o) const
{
    for (PenvMgr2::const_iterator it = this->_env.begin();
         it != this->_env.end();
         ++it)
    {
        o << it->first << " = `";
        it->second.print_body(o);
        o << "`" << std::endl;
    }
}

void PenvMgr2::var_body_t::print_body(std::ostream& o) const
{
    if (this->isFunction()) {
        o << "{userdef funciton}";
    }
    for (expression_t::const_iterator it = this->second.begin();
         it != this->second.end();
         ++it)
    {
        o << *it;
    }
}

//----------------------------------------------------------------------

typedef std::string::const_iterator iter_t;
typedef sss::util::StringSlice<iter_t> StringSlice_t;
typedef sss::util::Parser<iter_t> Parser_t;
typedef Parser_t::Rewinder Rewinder_t;

namespace {
    class env_parser{
    public:
        env_parser(PenvMgr2::var_body_t& vb)
            : _varlist(vb.first), _expr(vb.second)
        {
        }
        ~env_parser()
        {
        }

    public:
        typedef std::string::const_iterator iter_t;
        bool parse(const std::string& expr)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            _varlist.clear();
            _expr.clear();
            _expr.m_data.assign(expr);
            iter_t s_beg = expr.begin();
            return parse_expr(s_beg, expr.end());
        }

        bool parseRawStr(const std::string& expr)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            _varlist.clear();
            _expr.clear();
            _expr.m_data.assign(expr);
            _expr.push_back(expr.cbegin(), expr.cend());
            return true;
        }

        bool parseFunc(PenvMgr2::expression_t::FuncT func, PenvMgr2::expression_t::FuncParamT para)
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
        bool parse_expr(iter_t & s_beg, iter_t s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            while (s_beg != s_end)
            {
                {
                    Rewinder_t r(s_beg);
                    while (s_beg != s_end) {
                        Rewinder_t r1(s_beg);
                        if (parse_Var(s_beg, s_end)) {
                            r1.commit(false);
                            break;
                        }
                        else {
                            s_beg++;
                            r1.commit(true);
                        }
                    }
                    if (r.distance()) {
                        r.commit(true);
                        this->_expr.push_back(r.getSlice());
                        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, r.getSlice());
                    }
                }

                {
                    Rewinder_t r2(s_beg);
                    r2.commit(parse_Var(s_beg, s_end));
                    if (r2.distance()) {
                        StringSlice_t var_name = r2.getSlice();
                        if (var_name[1] == '{') {
                            var_name.shrink(2, 1); // strip "${" and "}"
                        }
                        else {
                            var_name.shrink(1, 0); // strip "$"
                        }
                        if (!(this->_expr.size() & 1u)) {
                            this->_expr.push_back(s_beg, s_beg);
                        }
                        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var_name);
                        // std::string var_name = PenvMgr2::refer2name(.str());
                        this->_expr.push_back(var_name);
                        this->_varlist.insert(var_name.str());
                    }
                }
            }
            assert(s_beg == s_end);
            return true;
        }

        // shell
        // ${t.(`command par par`)} // dot 是为了与 egs匹配
        static bool parse_Var_name(iter_t & it_beg, iter_t it_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r1(it_beg);
            Rewinder_t r2(it_beg);
#ifdef PENVMGR2_SUPPORT_SHELL_CMD
            Rewinder_t r3(it_beg);
#endif

            StringSlice_t id(it_beg, it_beg);
            r1.commit(Parser_t::parseCIdentifier(it_beg, it_end, id) &&
                      ((r2.begin() && r2.commit(id.length() == 1 && std::strchr("egs", *id.begin()) &&
                                                Parser_t::parseChar(it_beg, it_end, '.') &&
                                                Parser_t::parseCIdentifier(it_beg, it_end, id)))
#ifdef PENVMGR2_SUPPORT_SHELL_CMD
                       ||
                       (r3.begin() && r3.commit(id == "t" && Parser_t::parseSequence(it_beg, it_end, ".(`") &&
                                  Parser_t::parseAfterSequence(it_beg, it_end, "`)")))
#endif
                       || true));
            return r1.is_commited();
        }

        // Var ::= '$' name | "${" name '}'
        static bool parse_Var(iter_t & it_beg, iter_t it_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r1(it_beg);
            Rewinder_t r2(it_beg);

            bool has_brase = false;

            // char prefix = '\0';
            StringSlice_t id(it_beg, it_beg);

            r1.commit(Parser_t::parseChar(it_beg, it_end, '$') &&
                      ((has_brase = Parser_t::parseChar(it_beg, it_end, '{')) || true) && r2.begin() &&
                      r2.commit(parse_Var_name(it_beg, it_end)) &&
                      ((has_brase && Parser_t::parseChar(it_beg, it_end, '}')) || !has_brase));
            return r1.is_commited();
        }

        // g.xyz
        // s.abc
        // var
        static PenvMgr2::var_type_t get_var_type(const std::string& str)
        {
            if (PenvMgr2::is_var(str)) {
                return str.length() >= 2 && str[1] == '.' ? PenvMgr2::var_type_t(str[0]) : PenvMgr2::TYPE_NORMAL;
            }
            return PenvMgr2::TYPE_UNKNOWN;
        }

    private:
        PenvMgr2::var_list_t      & _varlist;
        PenvMgr2::expression_t    & _expr;
    };
}


PenvMgr2&  PenvMgr2::getGlobalEnv()
{
    PenvMgr2 * p = this;
    while (p->_parent) {
        p = p->_parent;
    }
    return *p;
}

bool        PenvMgr2::set(std::string var, const std::string& expr)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, expr);

    var_type_t type = env_parser::get_var_type(var);
    switch (type) {
    case TYPE_SYSTEM:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a system-style var name; cannot modified");
        break;

    case TYPE_GLOBAL:
        var = var.substr(2);
        if (this->_parent) {
            return this->getGlobalEnv().set(var, expr);
        }
        break;

    case TYPE_OSENV:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a OS-environment var name; cannot modified");
        break;

    case TYPE_SHELL:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a shell-cmmand; cannot modified");
        break;

    case TYPE_NORMAL:
        break;

    default:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var << "` is not a valid var name");
        break;
    }
    env_t::iterator it = this->_env.find(var);
    if (it == this->_env.end()) {
        var_body_t vb;
        this->_env.insert(it, std::make_pair(var, vb));
    }
    bool ret = (env_parser(this->_env[var]).parse(expr));
    if (!ret) {
        SSS_POSTION_THROW(std::runtime_error,
                      " (`" << var << "`, `" << expr << "`) parse failed.");
    }
    return true;
}

bool        PenvMgr2::setRawStr(std::string var, const std::string& expr)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, expr);

    var_type_t type = env_parser::get_var_type(var);
    switch (type) {
    case TYPE_SYSTEM:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a system-style var name; cannot modified");
        break;

    case TYPE_GLOBAL:
        var = var.substr(2);
        if (this->_parent) {
            return this->getGlobalEnv().setRawStr(var, expr);
        }
        break;

    case TYPE_OSENV:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a OS-environment var name; cannot modified");
        break;

    case TYPE_SHELL:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a shell-cmmand; cannot modified");
        break;

    case TYPE_NORMAL:
        break;

    default:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
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
        SSS_POSTION_THROW(std::runtime_error,
                      " (`" << var << "`, `" << expr << "`) parse failed.");
    }
    return true;
}

bool PenvMgr2::set(std::string var, expression_t::FuncT func, expression_t::FuncParamT param)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, func);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, param);

    var_type_t type = env_parser::get_var_type(var);
    switch (type) {
    case TYPE_SYSTEM:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a system-style var name; cannot modified");
        break;

    case TYPE_GLOBAL:
        var = var.substr(2);
        if (this->_parent) {
            return this->getGlobalEnv().set(var, func, param);
        }
        break;

    case TYPE_OSENV:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a OS-environment var name; cannot modified");
        break;

    case TYPE_SHELL:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a shell-cmmand; cannot modified");
        break;

    case TYPE_NORMAL:
        break;

    default:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
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
        SSS_POSTION_THROW(std::runtime_error,
                      " (`" << var << "`, `" << func
                      << "`, `" << param << "`) parse failed.");
    }
    return true;
}

class ExceptionDependLoop : public std::runtime_error
{
public:
    explicit ExceptionDependLoop(const std::string& msg)
        : std::runtime_error(msg)
    {
    };
    ~ExceptionDependLoop() throw()
    {
    }
};

struct depend_t
{
    depend_t()
        : _ok(false)
    {
    }
    explicit depend_t(const std::string& value)
        : _ok(true), _value(value)
    {
    }

    ~depend_t()
    {
    }
    bool        _ok;
    std::string _value;

    void print(std::ostream& o) const {
        if (this->_ok) {
            o << "\"" << this->_value << "\"";
        }
        else {
            o << 0;
        }
    }
public:
};

std::ostream& operator << (std::ostream& o, const depend_t& d)
{
    d.print(o);
    return o;
}

class depend_checker2_t : private std::map<std::string, depend_t>
{
public:
    typedef std::map<std::string, depend_t> Base_t;
    typedef Base_t::const_iterator const_iterator;
    typedef Base_t::iterator       iterator;

    using Base_t::begin;
    using Base_t::end;
    using Base_t::find;

    depend_checker2_t()
    {
    }
    ~depend_checker2_t()
    {
    }

public:

    void put(const std::string& var)
    {
        assert(PenvMgr2::is_var(var));
        Base_t::iterator it = this->Base_t::find(var);
        if (it != this->Base_t::end() && !it->second._ok) {
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, it->first);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, it->second);
            SSS_POSTION_THROW(ExceptionDependLoop, var);
        }
        this->Base_t::insert(it, std::make_pair(var, depend_t()));
    }

    void put(const std::string& var, const std::string& value)
    {
        SSS_LOG_DEBUG("(%s, %s)\n", var.c_str(), value.c_str());
        assert(PenvMgr2::is_var(var));
        Base_t::iterator it = this->Base_t::find(var);
        if (it == this->Base_t::end()) {
            this->Base_t::insert(it, std::make_pair(var, depend_t(value)));
        }
        else {
            if (it->second._ok && it->second._value != value) {
                SSS_LOG_ERROR("re-assign `%s` \"%s\" -> \"%s\"\n",
                              it->first.c_str(),
                              it->second._value.c_str(),
                              value.c_str());
                SSS_LOG_EXPRESSION(sss::log::log_ERROR, it->first);
                SSS_LOG_EXPRESSION(sss::log::log_ERROR, it->second);
                SSS_POSTION_THROW(ExceptionDependLoop, var);
            }
            it->second._ok      = true;
            it->second._value   = value;
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

    void print(std::ostream& o) const{
        o << "[";
        for (Base_t::const_iterator it = this->Base_t::begin();
             it != this->Base_t::end();
             ++it)
        {
            o << it->first << ":" << it->second << "\";";
        }
        o << "]";
    }

private:
};

std::ostream& operator <<(std::ostream& o, const depend_checker2_t& dc)
{
    dc.print(o);
    return o;
}

std::string PenvMgr2::get(std::string var)
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
            ret = evaluator_impl(var, dc);
        }
        catch (ExceptionDependLoop& e) {
            SSS_POSTION_THROW(std::runtime_error,
                          "ExceptionDependLoop "
                          << std::string(e.what())
                          << "; when requre " << var);
        }
    }
    return ret;
}

// TODO
// 增加支持的 PenvMgr2系统变量
std::string PenvMgr2::getSystemVar(const std::string& var)
{
    if (var == "timestamp") {
        std::string fmt = this->get("timestamp_fmt");
        if (fmt.empty()) {
            fmt.assign("%FT%T");
        }
        return sss::time::strftime(fmt);
    }
    else if (var == "cwd") {
        return sss::path::getcwd();
    }
    SSS_LOG_ERROR("var %s not supported!\n", var.c_str());
    return "";
}

std::string PenvMgr2::getEnvVar(const std::string& var)
{
    const char * e = ::getenv(var.c_str());
    return e ? e : "";
}

std::string PenvMgr2::getShellComandFromVar(const std::string& var)
{
    std::string cmd(var, 4, var.length() - 6);
    const char * err_to_null = " 2>/dev/null";
    if (!sss::is_end_with(cmd, err_to_null)) {
        cmd.append(err_to_null);
    }
    SSS_LOG_DEBUG("%s\n", cmd.c_str());
    sss::ps::StringPipe ps;
    ps << cmd;
    std::string out = ps.run();
    sss::rtrim(out);
    return out;
}

std::string PenvMgr2::get_expr(const std::string& expr)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, expr);
    std::string ret;

    var_body_t vb;

    if (!env_parser(vb).parse(expr)) {
        SSS_POSTION_THROW(std::runtime_error, " (`" << expr << "`) parse failed.");
    }

    try {
        // TODO
        // 在本层，记录引用过哪些变量。
        // 并输出
        depend_checker2_t dc;
        for (var_list_t::const_iterator it_var = vb.first.begin();
             it_var != vb.first.end();
             ++it_var)
        {
            dc.put(*it_var, this->evaluator_impl(*it_var, dc));
        }
        ret = PenvMgr2::generate(vb, dc);
    }
    catch (ExceptionDependLoop& e) {
        SSS_POSTION_THROW(std::runtime_error,
                      "ExceptionDependLoop " <<
                      std::string(e.what()) <<
                      "; when requre " << expr);
    }
    return ret;
}

bool PenvMgr2::is_var_refer(const std::string & var)
{
    return env_parser::is_var_refer(var);
}

bool PenvMgr2::is_var_refer(const StringSlice_t& var)
{
    return env_parser::is_var_refer(var);
}

bool PenvMgr2::is_var(const std::string & var)
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

std::string PenvMgr2::refer2name(const std::string & var)
{
    assert(PenvMgr2::is_var_refer(var));

    if (*var.rbegin() != '}') {
        return var.substr(1);
    }
    else {
        return var.substr(2, var.length() - 3);
    }
}

const PenvMgr2::var_body_t *    PenvMgr2::find_body(const std::string& var) const
{
    const PenvMgr2::var_body_t * p_body = 0;
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
//   记录 evaluator_impl 的调用条件！理论上，不能对同一个变量，两次通过evaluator_impl来求值！
//------------------------------------
// 求出var的值；
// 将var所依赖变量的值，都通过depend_checker2_t管理起来，以便在求值的过程中，找
// 出循环依赖额情况；
std::string PenvMgr2::evaluator_impl(std::string var, depend_checker2_t & dc)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    var_type_t type = env_parser::get_var_type(var);
    SSS_LOG_DEBUG("type of %s is %c\n", var.c_str(), type ? type : var_type_t('0'));
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

    const var_body_t * definition = this->find_body(var);
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
             it_var != definition->var_list().end();
             ++it_var)
        {
            dc.put(*it_var);
            std::string val = this->evaluator_impl(*it_var, dc);
            SSS_LOG_DEBUG("put %s, %s\n", (*it_var).c_str(), val.c_str());
            dc.put(*it_var, val);
        }
        ret = PenvMgr2::generate(*definition, dc);
    }
    return ret;
}

std::string PenvMgr2::generate(const var_body_t & bd, const depend_checker2_t & dc)
{
    const PenvMgr2::expression_t & expr(bd.second);
    std::ostringstream oss;
    int index = 0;
    for (PenvMgr2::expression_t::const_iterator it = expr.begin();
         it != expr.end();
         ++it, ++index)
    {
        if (index & 1) {
            depend_checker2_t::const_iterator it_dc = dc.find((*it).str());
            assert(it_dc != dc.end() && it_dc->second._ok);
            oss << it_dc->second._value;
        }
        else {
            oss << *it;
        }
    }
    return oss.str();
}

// FIXME 下面这几个函数，还得针对 type进行分解！
bool        PenvMgr2::unset(std::string var)
{
    var_type_t type = env_parser::get_var_type(var);
    switch (type) {
    case TYPE_OSENV:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a OS-environment var name; cannot modified");
        break;
    
    case TYPE_GLOBAL:
        var = var.substr(2);
        if (this->_parent) {
            return this->getGlobalEnv().unset(var);
        }
        break;
    
    case TYPE_SYSTEM:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a system-style var name; cannot modified");
        break;
    
    case TYPE_SHELL:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a shell-cmmand; cannot modified");
        break;
    
    default:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var << "` is not a valid var name");
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
bool        PenvMgr2::has(std::string var)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    var_type_t type = env_parser::get_var_type(var);
    switch (type) {
    case TYPE_OSENV:
        {
            return ::getenv(var.substr(2).c_str());
        }
        break;
    
    case TYPE_GLOBAL:
        var = var.substr(2);
        if (this->_parent) {
            return this->getGlobalEnv().has(var);
        }
        break;
    
    case TYPE_SYSTEM:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a system-style var name;"
                      " do not support `has` operation.");
        break;
    
    case TYPE_SHELL:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var <<
                      "` is a shell-cmmand;"
                      " do not support `has` operation.");
        break;
    
    case TYPE_NORMAL:
        break;

    default:
        SSS_POSTION_THROW(std::runtime_error,
                      "PenvMgr2: `" << var << "` is not a valid var name");
        break;
    }
    return
        this->_env.find(var) != this->_env.end() ||
        (this->has_parent() && this->parent().has(var));
}

// TODO 另外，最好再增加一个find()函数；返回一个自定义类型的iterator；这个类型
// 本身，包含一个PenvMgr2*和PenvMgr2::env::iterator；
//
// 需要用双重指针来定位具体的var！

PenvMgr2& PenvMgr2::parent() const
{
    if (!this->has_parent()) {
        SSS_POSTION_THROW(std::runtime_error, "Null PenvMgr2::_parent dereferenced!");
    }
    return * this->_parent;
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
