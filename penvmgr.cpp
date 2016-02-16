#include "penvmgr.hpp"

#include <assert.h>
#include <sstream>
#include <string>
#include <stdexcept>

#include <sss/stream/skipper.hpp>

namespace sss{

PenvMgr::PenvMgr(PenvMgr * parent)
    : _parent(parent)
{
}

PenvMgr::~PenvMgr()
{
}

PenvMgr::PenvMgr(const PenvMgr& ref)
    : _env(ref._env), _parent(ref._parent)
{
}

PenvMgr::iterator PenvMgr::begin()
{
    return _env.begin();
}

PenvMgr::iterator PenvMgr::end()
{
    return _env.end();
}

PenvMgr::const_iterator PenvMgr::begin() const
{
    return _env.begin();
}

PenvMgr::const_iterator PenvMgr::end() const
{
    return _env.end();
}

void PenvMgr::print(std::ostream& o) const
{
    for (PenvMgr::const_iterator it = this->_env.begin();
         it != this->_env.end();
         ++it)
    {
        o << it->first << " = `";
        it->second.print_body(o);
        o << "`" << std::endl;
    }
}

void PenvMgr::var_body_t::print_body(std::ostream& o) const
{
    for (expression_t::const_iterator it = this->second.begin();
         it != this->second.end();
         ++it)
    {
        o << *it;
    }
}

//----------------------------------------------------------------------

typedef sss::stream::skipper<std::string::const_iterator> Skipper_t;
typedef sss::stream::Rewinder<std::string::const_iterator> Rewinder_t;

namespace {
    class env_parser{
    public:
        env_parser(PenvMgr::var_body_t& vb)
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
            _varlist.clear();
            _expr.clear();
            iter_t s_beg = expr.begin();
            return parse_expr(s_beg, expr.end());
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
                            r1.rollback();
                            break;
                        }
                        else {
                            s_beg++;
                            r1.commit();
                        }
                    }
                    if (r.count()) {
                        this->_expr.push_back(r.get_consume());
                        r.commit();
                    }
                }

                {
                    Rewinder_t r2(s_beg);
                    r2.commit(parse_Var(s_beg, s_end));
                    if (r2.count()) {
                        this->_expr.push_back(r2.get_consume());
                        this->_varlist.insert(PenvMgr::refer2name(r2.get_consume()));
                    }
                }
            }
            assert(s_beg == s_end);
            return true;
        }

        // Var ::= '$' name | "${" name '}'
        static bool parse_Var(iter_t & s_beg, iter_t s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            Rewinder_t r(s_beg);
            Skipper_t sk;
            r.commit(sk.skip_char(s_beg, s_end, '$') &&
                     sk.skip_char(s_beg, s_end, '{') &&
                     sk.skip_word(s_beg, s_end)      &&
                     sk.skip_char(s_beg, s_end, '}')) ||
                r.commit(sk.skip_char(s_beg, s_end, '$') && sk.skip_word(s_beg, s_end));
            return r.count();
        }

    private:
        PenvMgr::var_list_t      & _varlist;
        PenvMgr::expression_t    & _expr;
    };
}

bool        PenvMgr::set(const std::string& var, const std::string& expr)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, expr);
    if (!this->is_var(var)) {
        throw std::runtime_error("PenvMgr: `" + var + "` is not a valid var name");
    }
    env_t::iterator it = this->_env.find(var);
    if (it == this->_env.end()) {
        var_body_t vb;
        bool ret = (env_parser(vb).parse(expr));
        assert(ret);
        this->_env.insert(it, std::make_pair(var, vb));
    }
    else {
        bool ret = (env_parser(it->second).parse(expr));
        assert(ret);
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

class depend_checker_t : private std::map<std::string, depend_t>
{
public:
    typedef std::map<std::string, depend_t> Base_t;
    typedef Base_t::const_iterator const_iterator;
    typedef Base_t::iterator       iterator;

    using Base_t::begin;
    using Base_t::end;
    using Base_t::find;

    depend_checker_t()
    {
    }
    ~depend_checker_t()
    {
    }

public:

    void put(const std::string& var)
    {
        assert(PenvMgr::is_var(var));
        Base_t::iterator it = this->Base_t::find(var);
        if (it != this->Base_t::end() && !it->second._ok) {
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, it->first);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, it->second);
            throw ExceptionDependLoop(var);
        }
        this->Base_t::insert(it, std::make_pair(var, depend_t()));
    }

    void put(const std::string& var, const std::string& value)
    {
        assert(PenvMgr::is_var(var));
        Base_t::iterator it = this->Base_t::find(var);
        if (it == this->Base_t::end()) {
            this->Base_t::insert(it, std::make_pair(var, depend_t(value)));
        }
        else {
            if (it->second._ok && it->second._value != value) {
                SSS_LOG_DEBUG("re-assign `%s` \"%s\" -> \"%s\"\n",
                              it->first.c_str(),
                              it->second._value.c_str(),
                              value.c_str());
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, it->first);
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, it->second);
                throw ExceptionDependLoop(var);
            }
            it->second._ok      = true;
            it->second._value   = value;
        }
    }

    bool pull(const std::string& var, std::string& value)
    {
        assert(PenvMgr::is_var(var));
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

std::ostream& operator <<(std::ostream& o, const depend_checker_t& dc)
{
    dc.print(o);
    return o;
}

std::string PenvMgr::get(const std::string& var)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    assert(is_var(var));
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
            depend_checker_t dc;
            dc.put(var);
            ret = evaluator_impl(var, dc);
        }
        catch (ExceptionDependLoop& e) {
            throw std::runtime_error("ExceptionDependLoop " +
                                     std::string(e.what()) +
                                     "; when requre " + var);
        }
    }
    return ret;
}

std::string PenvMgr::get_expr(const std::string& expr)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, expr);
    std::string ret;

    var_body_t vb;
    bool is_ok = (env_parser(vb).parse(expr));
    assert(is_ok);

    try {
        depend_checker_t dc;
        for (var_list_t::const_iterator it_var = vb.first.begin();
             it_var != vb.first.end();
             ++it_var)
        {
            dc.put(*it_var, this->evaluator_impl(*it_var, dc));
        }
        ret = PenvMgr::generate(vb, dc);
    }
    catch (ExceptionDependLoop& e) {
        throw std::runtime_error("ExceptionDependLoop " +
                                 std::string(e.what()) +
                                 "; when requre " + expr);
    }
    return ret;
}

bool PenvMgr::is_var_refer(const std::string & var)
{
    return env_parser::is_var_refer(var);
}

bool PenvMgr::is_var(const std::string & var)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    Skipper_t sk;
    std::string::const_iterator s_beg = var.begin();
    std::string::const_iterator s_end = var.end();
    return sk.skip_word(s_beg, s_end) && s_beg == s_end;
}

std::string PenvMgr::refer2name(const std::string & var)
{
    assert(PenvMgr::is_var_refer(var));

    if (*var.rbegin() != '}') {
        return var.substr(1);
    }
    else {
        return var.substr(2, var.length() - 3);
    }
}

//iterator          PenvMgr::find_body(const std::string& var)
//{
//}

const PenvMgr::var_body_t *    PenvMgr::find_body(const std::string& var) const
{
    const PenvMgr::var_body_t * p_body = 0;
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
// 将var所依赖变量的值，都通过depend_checker_t管理起来，以便在求值的过程中，找
// 出循环依赖额情况；
std::string PenvMgr::evaluator_impl(const std::string& var, depend_checker_t & dc)
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, var);
    assert(is_var(var));
    std::string ret;

    const var_body_t * definition = this->find_body(var);
    if (!definition) {
        SSS_LOG_DEBUG("this->_env no %s\n", var.c_str());
        ret = "";
        // 对于不存在的变量，对依赖关系对象dc的变化是0；
        // 默认不存在的值，返回空串""；
    }
    else {
        for (var_list_t::const_iterator it_var = definition->var_list().begin();
             it_var != definition->var_list().end();
             ++it_var)
        {
            dc.put(*it_var);
            std::string val = this->evaluator_impl(*it_var, dc);
            SSS_LOG_DEBUG("put %s, %s\n", (*it_var).c_str(), val.c_str());
            dc.put(*it_var, val);
        }
        ret = PenvMgr::generate(*definition, dc);
    }
    return ret;
}

std::string PenvMgr::generate(const var_body_t & bd, const depend_checker_t & dc)
{
    const PenvMgr::expression_t & expr(bd.second);
    std::ostringstream oss;
    for (PenvMgr::expression_t::const_iterator it = expr.begin();
         it != expr.end();
         ++it)
    {
        if (is_var_refer(*it)) {
            std::string name = PenvMgr::refer2name(*it);
            depend_checker_t::const_iterator it_dc = dc.find(name);
            assert(it_dc != dc.end() && it_dc->second._ok);
            oss << it_dc->second._value;
        }
        else {
            oss << *it;
        }
    }
    return oss.str();
}

bool        PenvMgr::unset(const std::string& var)
{
    assert(is_var(var));
    env_t::iterator it = this->_env.find(var);
    if (it != this->_env.end()) {
        this->_env.erase(it);
    }
    else {
        if (this->has_parent()) {
            this->parent().unset(var);
        }
    }
    return true;
}

bool        PenvMgr::has(const std::string& var)
{
    assert(is_var(var));
    return
        this->_env.find(var) != this->_env.end() ||
        (this->has_parent() && this->parent().has(var));
}

PenvMgr& PenvMgr::parent() const
{
    if (!this->has_parent()) {
        throw std::runtime_error("Null PenvMgr::_parent dereferenced!");
    }
    return * this->_parent;
}

}

#if 0
void PenvMgr_test()
{
    PenvMgr env;
    std::cout << "-============================-" << std::endl;
    //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, PenvMgr::is_var("a"));
    //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, PenvMgr::is_var("$a"));

    //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, PenvMgr::is_var_refer("a"));
    //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, PenvMgr::is_var_refer("$a"));
    //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, PenvMgr::is_var_refer("${a}"));

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
