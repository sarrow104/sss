#include "cregex.hpp"

#include <stdexcept>

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <sss/bit_operation/bit_operation.h>
#include <sss/stream/skipper.hpp>
#include <sss/utlstring.hpp>

namespace {
    class regstr_refer_counter
    {
    public:
        typedef sss::stream::skipper<std::string::const_iterator> Skipper_t;

    public:
        regstr_refer_counter(const std::string& regstr)
            : _regstr(regstr)
        {
        }
        ~regstr_refer_counter()
        {
        }

    public:
        int count()
        {
            std::string::const_iterator s_beg = this->_regstr.begin();
            std::string::const_iterator s_end = this->_regstr.end();
            // 避开集合[]即可！
            _count = 0;
            this->parse(s_beg, s_end);
            if (s_beg != s_end) {
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *s_beg);
                throw std::logic_error(__func__ + sss::cast_string(__LINE__));
            }
            return _count;
        }

    protected:
        void parse(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *s_beg);
            while (s_beg != s_end) {
                if (*s_beg == '[') {
                    this->parse_bracketexpr(s_beg, s_end);
                }
                else if (*s_beg == '\\') {
                    this->parse_backslash(s_beg, s_end);
                }
                else if (*s_beg == '(') {
                    this->parse_anchor(s_beg, s_end);
                }
                else if (*s_beg == ')') {
                    return;
                }
                else {
                    s_beg++;
                }
            }
        }

        void parse_anchor(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *s_beg);
            if (sk.is_begin_with(s_beg, s_end, '(')) {
                s_beg++;
                this->parse(s_beg, s_end);
                if (sk.is_begin_with(s_beg, s_end, ')')) {
                    s_beg++;
                    _count++;
                    return;
                }
                else {
                    throw std::logic_error(__func__ + sss::cast_string(__LINE__));
                }
            }
            else {
                throw std::logic_error(__func__ + sss::cast_string(__LINE__));
            }
        }

        void parse_bracketexpr(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *s_beg);
            // NOTE '[' ']' 之间，至少有一个以上的字符！
            bool is_first = true;
            if (s_beg != s_end && *s_beg == '[') {
                s_beg++;
                while (s_beg != s_end) {
                    if (is_first) {
                        if (*s_beg == ']') {
                            s_beg++;
                            is_first = false;
                            continue;
                        }
                        if (*s_beg == '-') {
                            s_beg++;
                            is_first = false;
                            continue;
                        }
                        else if (sk.is_begin_with(s_beg, s_end, "[.")) {
                            this->parse_collatingelement(s_beg, s_end);
                            is_first = false;
                            continue;
                        }
                        else if (sk.is_begin_with(s_beg, s_end, "[:")) {
                            this->parse_equivlenceclass(s_beg, s_end);
                            is_first = false;
                            continue;
                        }
                        else if (sk.is_begin_with(s_beg, s_end, "[=")) {
                            this->parse_characterclass(s_beg, s_end);
                            is_first = false;
                            continue;
                        }
                        else {
                            s_beg++;
                            is_first = false;
                        }
                    }
                    else {
                        if (*s_beg == ']') {
                            break;
                        }
                        else {
                            if (sk.is_begin_with(s_beg, s_end, "[.")) {
                                this->parse_collatingelement(s_beg, s_end);
                                is_first = false;
                                continue;
                            }
                            else if (sk.is_begin_with(s_beg, s_end, "[:")) {
                                this->parse_equivlenceclass(s_beg, s_end);
                                is_first = false;
                                continue;
                            }
                            else if (sk.is_begin_with(s_beg, s_end, "[=")) {
                                this->parse_characterclass(s_beg, s_end);
                                is_first = false;
                                continue;
                            }
                            else {
                                s_beg++;
                                is_first = false;
                            }
                        }
                    }
                }
                if (!sk.is_begin_with(s_beg, s_end, ']')) {
                    throw std::logic_error(__func__ + sss::cast_string(__LINE__));
                }
                s_beg++;
            }
        }

        void parse_atom(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *s_beg);
            (void) s_beg;
            (void) s_end;
        }

        void parse_range(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *s_beg);
            (void) s_beg;
            (void) s_end;
        }

        void parse_collatingelement(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *s_beg);
            if (sk.is_begin_with(s_beg, s_end, "[.")) {
                s_beg += 2;
                while (s_beg != s_end && *s_beg != '.') {
                    ++s_beg;
                }
                if (s_beg == s_end || !sk.is_begin_with(s_beg, s_end, ".]")) {
                    throw std::logic_error(__func__ + sss::cast_string(__LINE__));
                }
                s_beg += 2;
            }
            else {
                throw std::logic_error(__func__ + sss::cast_string(__LINE__));
            }
        }

        void parse_equivlenceclass(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *s_beg);
            if (sk.is_begin_with(s_beg, s_end, "[:")) {
                s_beg += 2;
                while (s_beg != s_end && *s_beg != ':') {
                    ++s_beg;
                }
                if (s_beg == s_end || !sk.is_begin_with(s_beg, s_end, ":]")) {
                    throw std::logic_error(__func__ + sss::cast_string(__LINE__));
                }
                s_beg += 2;
            }
            else {
                throw std::logic_error(__func__ + sss::cast_string(__LINE__));
            }
        }

        void parse_characterclass(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *s_beg);
            if (sk.is_begin_with(s_beg, s_end, "[=")) {
                s_beg += 2;
                // FIXME 这里，只需要一个元素吗？
                while (s_beg != s_end && *s_beg != ':') {
                    ++s_beg;
                }
                if (s_beg == s_end || !sk.is_begin_with(s_beg, s_end, "=]")) {
                    throw std::logic_error(__func__ + sss::cast_string(__LINE__));
                }
                s_beg += 2;
            }
            else {
                throw std::logic_error(__func__ + sss::cast_string(__LINE__));
            }
        }

        void parse_backslash(std::string::const_iterator& s_beg, std::string::const_iterator s_end)
        {
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *s_beg);
            if (s_beg != s_end && s_beg + 1 != s_end && *s_beg == '\\') {
                s_beg += 2;
                return;
            }
            else {
                throw std::logic_error(__func__ + sss::cast_string(__LINE__));
            }
        }

    private:
        const std::string &     _regstr;
        int                     _count;
        Skipper_t               sk;
    };
}

union regex_strict_aliasing
{
    regex_t  _reg_part;
    uint32_t _aint_part[sizeof(regex_t)/ sizeof(uint32_t)];
};

namespace sss {
    namespace regex {

        CRegex::CRegex()
            : _errcode(0), _mode(0)
        {
            std::memset(&_regex, 0, sizeof(regex_t));
        }
        CRegex::CRegex(const std::string& regstr, int mode)
        {
            new (this) CRegex;
            this->compile(regstr, mode);
        }

        CRegex::CRegex(const CRegex& ref)
        {
            new (this) CRegex(ref._regstr, ref._mode);
        }

        CRegex& CRegex::operator = (const CRegex& ref)
        {
            if (this != &ref) {
                CRegex tmp(ref);
                this->swap(tmp);
            }
            return *this;
        }

        CRegex::~CRegex()
        {
            this->destroy();
        }

        bool CRegex::compile(const std::string& regstr, int mode)
        {
            // mode = REG_EXTENDED | REG_ICASE | REG_NOSUB
            this->_regstr = regstr;
            this->_mode = mode;
            this->destroy();
            this->_errcode = regcomp(&_regex,
                                     _regstr.c_str(),
                                     _mode);
            if (this->_errcode) {
                this->build_errmsg();
                throw std::runtime_error(this->_errmsg);
                // 注意，就算 正则串 编译失败，其内部还是会发生变化——非0；
                this->destroy();
            }
            else {
                if (mode & REG_NOSUB) {
                    this->_submatches.resize(0);
                }
                else {
                    // NOTE，没有必要计算需要多少个refer；
                    // 因为 regex.h 的 正则表达式，最多只支持9+1个记录！
                    // 哦，还是有问题；万一"越界"访问，怎么办？
                    // 没关系，regexec 函数，会将未用到的 regmatch_t.rm_so 字段
                    // ，重新设置为-1；
                    this->_submatches.resize(10);
                    std::memset(this->_submatches.data(),
                                0,
                                this->_submatches.size() * sizeof(regmatch_t));
                }
            }
            return this->is_init();
        }

        bool CRegex::match(const std::string& str) const
        {
            return match(str.c_str());
        }

        bool CRegex::match(const char * str) const
        {
            bool is_match = false;
            this->_target = str;

            // regexec()
            //   is used to match a null-terminated string against the precompiled
            //   pattern buffer, preg.  nmatch and pmatch are used to provide
            //   information regarding the location of any matches.  eflags may be
            //   the  bitwise-or  of  one  or  both  of  *REG_NOTBOL*  and
            //   *REG_NOTEOL* which cause changes in matching behavior described
            //   below.
            //
            // REG_NOTBOL
            //        The  match-beginning-of-line operator always fails to match
            //        (but see the compilation flag REG_NEWLINE above) This flag may
            //        be used when different portions of a string are passed to
            //        regexec() and the beginning of the string should not  be
            //        inter‐preted as the beginning of the line.
            //
            // REG_NOTEOL
            //        The match-end-of-line operator always fails to match (but see
            //        the compilation flag REG_NEWLINE above)

            this->_errcode = regexec(&_regex,
                                     str,
                                     this->_submatches.size(),
                                     const_cast<regmatch_t*>(this->_submatches.data()),
                                     0);
            if (this->_errcode == 0) {
                is_match = true;
            }
            else if (this->_errcode != REG_NOMATCH){
                this->build_errmsg();
                throw std::runtime_error(this->_errmsg);
            }
            if (is_match && !this->is_nosub()) {
                // NOTE
                // regexec 会将 不存在的 子匹配，regmatch_t 设置为 [-1, 0] ！
                size_t submatch_cnt = 10;
                for (size_t i = 0; i != this->_submatches.size(); ++i) {
                    if (this->_submatches[i].rm_so == -1) {
                        submatch_cnt = i;
                        break;
                    }
                }
                this->_submatches.resize(submatch_cnt);
            }

            return is_match;
        }

        //> 返回是否匹配 并返回匹配的字串  v1
        bool CRegex::match(const std::string& str, std::string& matched_sub) const
        {
            bool is_match = this->match(str);
            if (is_match && !this->is_nosub()) {
                matched_sub.assign(str, submatch_start(0), submatch_consumed(0));
            }
            return is_match;
        }

        //> 返回是否匹配 并返回匹配的字串，以及开始匹配的位置 v1
        bool CRegex::match(const std::string& str, std::string& matched_sub, int& begin_pos) const
        {
            bool is_match = this->match(str, matched_sub);
            if (is_match && !this->is_nosub()) {
                begin_pos = submatch_start(0);
            }
            return is_match;
        }

        bool CRegex::substitute(const std::string& in_str, const std::string& fmt_str, std::string& out_str) const
        {
            bool is_match = this->match(in_str);
#if 1
            if (is_match && !this->is_nosub()) {
                std::ostringstream oss;
                for (std::string::size_type i = 0; i != fmt_str.length(); ++ i) {
                    if (fmt_str[i] == '\\' && i + 1 != fmt_str.length() && std::isdigit(fmt_str[i + 1])) {
                        int index = fmt_str[i + 1] - '0';
                        for (ssize_t j = submatch_start(index); j != submatch_end(index); ++ j)
                        {
                            oss << in_str[j];
                        }
                        ++i;
                    }
                    else {
                        oss << fmt_str[i];
                    }
                }
                out_str = oss.str();
            }
#else
            if (is_match) {
                (void)formatGenerator(fmt_str, out_str);
            }
#endif
            return is_match;
        }

//        bool CRegex::formatGenerator(const std::string& fmt_str, std::string& out_str) const
//        {
//            if (!this->is_nosub()) {
//                std::ostringstream oss;
//                for (std::string::size_type i = 0; i != fmt_str.length(); ++ i) {
//                    if (fmt_str[i] == '\\' && i + 1 != fmt_str.length() && std::isdigit(fmt_str[i + 1])) {
//                        int index = fmt_str[i + 1] - '0';
//                        for (ssize_t j = submatch_start(index); j != submatch_end(index); ++ j)
//                        {
//                            oss << in_str[j];
//                        }
//                        ++i;
//                    }
//                    else {
//                        oss << fmt_str[i];
//                    }
//                }
//                out_str = oss.str();
//                return true;
//            }
//            return false;
//        }

        void CRegex::build_errmsg() const
        {
            char ebuf[1024];
            regerror(_errcode, &_regex, ebuf, sizeof(ebuf));
            this->_errmsg.assign(ebuf);
            this->_errmsg += ": `";
            this->_errmsg += this->_regstr;
            this->_errmsg += "`";
        }

        bool CRegex::is_init() const
        {
#if 1
            const regex_strict_aliasing& tmp(reinterpret_cast<const regex_strict_aliasing&>(this->_regex));
            return tmp._aint_part[0];
#else
            return reinterpret_cast<const uint32_t&>(this->_regex);
#endif
        }

        void CRegex::destroy()
        {
            if (this->is_init())
            {
                ::regfree(&_regex);
                //std::cout << __func__ << "--after destroy" << std::endl;
                //std::cout << sss::bit::hex_out(this->_regex) << std::endl;
                std::memset(&_regex, 0, sizeof(regex_t));
            }
        }

    } // namespace regex
} // namespace sss
