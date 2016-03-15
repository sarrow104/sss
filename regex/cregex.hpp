#ifndef  __CREGEX_HPP_1440427722__
#define  __CREGEX_HPP_1440427722__

#include <regex.h>

#include <stdint.h>

#include <sstream>
#include <iostream>
#include <vector>
#include <string>

/**
 * \file        cregex.hpp
 * \brief       a simple-cpp wrapper for gnu-regex
 * \date        2015-08-07
 * \versin      1.0
 *
 * \section cregex REGULAR EXPRESSIONS
 *
 *        A regular expression is a pattern that describes a set of strings.
 *        Regular expressions are constructed analogously to arithmetic
 *        expressions, by using various operators to combine smaller
 *        expressions.
 *
 *        grep understands three different versions of regular expression
 *        syntax: “basic” (BRE), “extended” (ERE)  and  “perl”  (PRCE).
 *        In GNU grep, there is no difference in available functionality
 *        between basic and extended syntaxes.  In other implementations, basic
 *        regular expressions are less powerful.  The following description
 *        applies to extended regular expressions; differences for  basic
 *        regular  expressions  are  summarized  afterwards.  Perl regular
 *        expressions give additional functionality, and are documented in
 *        pcresyntax(3) and pcrepattern(3), but only work if pcre is available
 *        in the system.
 *
 *        The fundamental building blocks are the regular expressions that
 *        match  a  single  character.   Most  characters,  including  all
 *        letters  and  digits,  are  regular  expressions that match
 *        themselves.  Any meta-character with special meaning may be quoted by
 *        preceding it with a backslash.
 *
 *        The period . matches any single character.
 *
 * \subsection  cregex_cc_and_bracket Character Classes and Bracket Expressions
 *
 *        A bracket expression is a list of characters enclosed by [ and ].  It
 *        matches any single character in that  list;  if  the  first character
 *        of  the  list  is  the  caret  ^  then  it matches any character not
 *        in the list.  For example, the regular expression [0123456789]
 *        matches any single digit.
 *
 *        Within a bracket expression, a range expression consists of two
 *        characters  separated  by  a  hyphen.   It  matches  any  single
 *        character  that  sorts  between  the  two  characters,  inclusive,
 *        using the locale's collating sequence and character set.  For
 *        example, in the default C locale, [a-d] is equivalent to [abcd].
 *        Many locales sort characters in dictionary order, and in  these
 *        locales [a-d] is typically not equivalent to [abcd]; it might be
 *        equivalent to [aBbCcDd], for example.  To obtain the traditional
 *        interpretation of bracket expressions, you can use the C locale by
 *        setting the LC_ALL environment variable to the value C.
 *
 *        Finally, certain named classes of characters are predefined within
 *        bracket  expressions,  as  follows.   Their  names  are  self
 *        explanatory,  and  they  are  [:alnum:],  [:alpha:], [:cntrl:],
 *        [:digit:], [:graph:], [:lower:], [:print:], [:punct:], [:space:],
 *        [:upper:], and [:xdigit:].  For example, [[:alnum:]] means the
 *        character class of numbers and letters in the current  locale.  In
 *        the C locale and ASCII character set encoding, this is the same as
 *        [0-9A-Za-z].  (Note that the brackets in these class names are part
 *        of the symbolic names, and must be included in addition to the
 *        brackets delimiting  the  bracket  expression.)   Most  meta-
 *        characters lose their special meaning inside bracket expressions.  To
 *        include a literal ] place it first in the list.  Similarly, to
 *        include a literal ^ place it anywhere but first.  Finally, to include
 *        a literal - place it last.
 *
 * \subsection cregex_anchoring   Anchoring
 *
 *        The caret ^ and the dollar sign $ are meta-characters that
 *        respectively match the empty string at the  beginning  and  end  of
 *        a line.
 *
 * \subsection cregex_backslash   The Backslash Character and Special Expressions
 *
 *        The  symbols  \<  and \> respectively match the empty string at the
 *        beginning and end of a word.  The symbol \b matches the empty string
 *        at the edge of a word, and \B matches the empty string provided it's
 *        not at the edge of  a  word.   The  symbol  \w  is  a synonym for
 *        [_[:alnum:]] and \W is a synonym for [^_[:alnum:]].
 *
 *
 * \subsection cregex_repetition   Repetition
 *
 *        A regular expression may be followed by one of several repetition operators:
 *        ?      The preceding item is optional and matched at most once.
 *        *      The preceding item will be matched zero or more times.
 *        +      The preceding item will be matched one or more times.
 *        {n}    The preceding item is matched exactly n times.
 *        {n,}   The preceding item is matched n or more times.
 *        {,m}   The preceding item is matched at most m times.  This is a GNU extension.
 *        {n,m}  The preceding item is matched at least n times, but not more than m times.
 *
 * \subsection cregex_concatenation   Concatenation
 *
 *        Two  regular  expressions  may  be  concatenated; the resulting
 *        regular expression matches any string formed by concatenating two
 *        substrings that respectively match the concatenated expressions.
 *
 * \subsection cregex_alternation   Alternation
 *
 *        Two regular expressions may be joined by the infix operator |; the
 *        resulting  regular  expression  matches  any  string  matching either
 *        alternate expression.
 *
 * \subsection cregex_Precedence   Precedence
 *
 *        Repetition  takes  precedence  over  concatenation,  which  in turn
 *        takes precedence over alternation.  A whole expression may be
 *        enclosed in parentheses to override these precedence rules and form a
 *        subexpression.
 *
 * \subsection cregex_backrefer   Back References and Subexpressions
 *
 *        The back-reference \n, where n is a single digit, matches the
 *        substring previously matched by the nth parenthesized subexpression
 *        of the regular expression.
 *
 * \subsection cregex_mode   Basic vs Extended Regular Expressions
 *
 *        In  basic  regular  expressions  the meta-characters ?, +, {, |, (,
 *        and ) lose their special meaning; instead use the backslashed
 *        versions \?, \+, \{, \|, \(, and \).
 *
 *        Traditional egrep did not support the { meta-character, and some
 *        egrep implementations support \{ instead,  so  portable  scripts
 *        should avoid { in grep -E patterns and should use [{] to match a
 *        literal {.
 *
 *        GNU  grep -E  attempts  to  support  traditional  usage  by assuming
 *        that { is not special if it would be the start of an invalid interval
 *        specification.  For example, the command grep -E '{1' searches for
 *        the two-character string {1 instead  of  reporting  a syntax error in
 *        the regular expression.  POSIX allows this behavior as an extension,
 *        but portable scripts should avoid it.
 */


// TODO 实现对 regex.h 的C++ 封装；
// 读取正则串可能的子匹配数，并生成合适的
// regmatch_t pm[10];
// 长度，以便获取子串；
// 另外，外部接口，需要参考 sss::simpleregex
//
// 另外，配对结果，最好挂载到外部；
// 比如解析regstr，看有多少引用；再根据引用数目，生成合适的 regmatch_t 数量。
// 然后 regmatch_t ，需要先判断compile时候的编译参数，是否有 REG_NOSUB ？
// 有的话，则 referMaxCount 返回0；否则，根据实际进行运算；
// ---------------------------------------
// 首先，是 POSIX 2 的扩展 regex 语法
// regex    ::= branches ( '|' branches )*
// branches ::= pieces+
// pieces   ::= atom ('*' | '+' | '?' | bound )
// bound    ::= '{' decimal (',' decimal ? )? '}'
// decimal  ::= \d+ [ 0 <= decimal, decimal <= 255 ]
// atom     ::= '(' regex_t ')'
//            | bracket-expr
//            | '.'
//            | '^'
//            | '$'
//            | "\\" ('^' | '.' | '[' | '$' | '(' | ')' | '|' | '*' | '+' | '?' | '{' | "\\" )
//            | "\\" char // 仅代表char；就好像，"\\" 不存在
//            | "\\" '1'-'9'
//            | char
//            | '{' ^\d // '{'后门跟一个非数字，代表'{'自己；
//            // 正则串，不能以"\\"结尾
// char     ::= 特殊字符之外的字符；
// bracket-expr ::= '[' (char | range)* ']'
//                | '[' '^' (char | range)* ']'
// range    ::= char '-' char
//          // NOTE "a-c-e" 非法；
//          // "[]]" ，"[^]]" 用来包含']' 或者 不包含']'；
//          // 包含"-"的话，需要'['后门
//                            | ']'前面
//                            | char '-' '-'
//                            | "[.-.]" '-' char
//          // 包含'^'的话，不要在第一个即可；
//          // "[]" 内的特殊字符，不再有特殊含义，包括"\\";
// collating-element ::= "[." char+ ".]"
//          // 代表字符序列，不可再切分；
//          // 序列 "[[.ch.]]*c" 将匹配 chchcc 的前5个字符；
// character-class ::= "[:"("alnum" | "digit" | "punct" | "alpha" | "graph" | "space" | "blank" | "lower" | "upper" | "cntrl" | "print" | "xdigit") ":]"
// equivalence-class ::= "[=" .. "=]"
// // equivalence-class,character-class 不能用于 range 的尾节点
//----------------------------------------------------------------
// reference:
//   man 7 regex
//
// NOTE
// 在 regcomp 的时候，加上 REG_NOSUB 条件，会使得match执行动作快近10倍！
// 见
//! http://www.cnblogs.com/pmars/archive/2012/10/24/2736831.html
//-------------------------------------
// 非贪婪搜索——需要在后面加上 '?'
//-------------------------------------
// NOTE regcomp 的mode参数，可以制定 REG_NOSUB ，按文档说明，此时不会记录匹配；
// 那么问题来了。
// regexec 动作，其目的就是用正则串与目标字符串进行匹配，并找到匹配部分的字串；
// "\0" 就是指的整个匹配串；
// 那么 指定了 REG_NOSUB 之后，并提供了一个长度的 regmatch_t 数组，还会获得整个
// 串的匹配长度吗？
// 有一个奇葩的特性，指定 REG_NOSUB 之后，如果有提供 regmatch_t 数组，那么第一个
// 位置，会写上 {0, 1}，而不管真实匹配了多长……
// 还有一个问题；如果我事先，将 regex_t 结构体 memset 为0；然后看regcomp的时候，
// 内存变化；编译成功如何？编译失败又如何？
// 貌似，编译正常，regex_t 的前面16个字节非0；regfree 动作，则将该部分，还有后门
// 若干字节置0；
// 编译不正常，则上述字节关键字节仍然为0；即，我可以通过regcomp 之后，regex_t的
// 内存状态，来判断编译是否成功；并且，编译失败的情况，执行regfree，内存无变化！
// ---------------------
// NOTE 为什么没有将CRegex拆分为至少两个类，比如再有一个叫match_t的，用来保存匹
// 配数据的类？
//
// 不行，因为glibc提供的这个regex库，是一个黑箱结构体；编译regex以及匹配中，状
// 态的变化，其实都是保存在这个结构体中的。
//
// 而不是额外的状态变量中；
//
// 而分离匹配结果，作为单独的类，无非就是让regex对象，可以反复、使用。但不幸的
// 是，不行；
//
// 所以，分离出匹配结果的方案，就流产了。

namespace sss {
    namespace regex {
        const int mode_default = REG_EXTENDED | REG_NEWLINE;
    class CRegex
    {
    public:
        CRegex();

        // mode 如下4个值的或运算
        // REG_EXTENDED 以功能更加强大的扩展正则表达式的方式进行匹配。
        // REG_ICASE 匹配字母时忽略大小写。
        // REG_NOSUB 不用存储匹配后的结果。
        // REG_NEWLINE 识别换行符，这样'$'就可以从行尾开始匹配，'^'就可以从行的开头开始匹配。
        CRegex(const std::string& regstr, int mode = mode_default);

        ~CRegex();

    public:
        bool compile(const std::string& regstr, int mode = mode_default);
        void destroy();

    public:
        bool match(const char * str) const;
        bool match(const std::string& ) const;

        //! 基础函数
        //> 返回是否匹配 并在对象内部记录本次匹配的位置信息
        //bool match(std::string::const_iterator,
        //           std::string::const_iterator) const;

        //> 返回是否匹配 并返回匹配的字串  v1
        bool match(const std::string& str, std::string& matched_sub) const;

        //> 返回是否匹配 并返回匹配的字串，以及开始匹配的位置 v1
        bool match(const std::string& str, std::string& matched_sub, int& begin_pos) const;


        ////> 返回是否匹配 并返回匹配的字串  v2
        //bool match(std::string::const_iterator,
        //           std::string::const_iterator,
        //           std::string& matched_sub);

        ////> 返回是否匹配 并返回匹配的字串，以及开始匹配的位置 v2
        //bool match(std::string::const_iterator,
        //           std::string::const_iterator,
        //           std::string& matched_sub,
        //           int& begin_pos);

    public:
        // NOTE 替换替换的模式
        // 正则表达式替换，如果以vim为参考，分为两部分。
        // s/parttern/subs/ge
        // 首先，subs也是一个可以解析的串；
        // 其中的\\1，被理解为字串。
        // 然后，整个匹配的部分，被用上面的替换掉；
        //
        // 返回替换后的字符串
        bool substitute(const std::string& in_str,
                        const std::string& fmt_str,
                        std::string& out_str) const;


        bool formatGenerator(const std::string& fmt_str, std::string& out_str) const;

        //std::string substitute(std::string::const_iterator,
        //                       std::string::const_iterator,
        //                       std::string::const_iterator,
        //                       std::string::const_iterator);

        //! sugar
        //> "将多次匹配的信息，依次写入传入的容器"——利用后插迭代器
        //! 使用范例：
        // std::vector<std::string> submatches;
        // sss::regex::CRegex("\\[sql\\d+\\]")
        //    .collect("[sql1] [sql23] [sql13] [sql0]",
        //             std::back_inserter(submatches));
        //
        // 或者：
        // std::ostream_iterator<std::string> out_it(std::cout, "\n");
        // sss::regex::CRegex("\\[sql\\d+\\]")
        //    .collect("[sql1] [sql23] [sql13] [sql0]",
        //             out_it);
        //! 输出：
        // [sql1]
        // [sql23]
        // [sql13]
        // [sql0]
        //
        template <typename Out_iterator>
            int  collect(const std::string& str,
                         Out_iterator bit)
            {
                try {
                    const char * str_beg = str.c_str();
                    int cnt = 0;
                    while (str_beg && *str_beg && match(str_beg)) {
                        str_beg += submatch_start(0);
                        bit = std::string(str_beg, str_beg + submatch_consumed(0));
                        str_beg += submatch_consumed(0);
                        ++cnt;
                    }
                    return cnt;
                }
                catch (std::exception & e) {
                    std::cout << __FILE__ << ", " << __LINE__ << std::endl;
                    throw;
                }
            }

        template <typename Out_iterator>
            int  collect(const std::string& str,
                         const std::string& fmt_str,
                         Out_iterator bit)
            {
                try {
                    const char * str_beg = str.c_str();
                    int cnt = 0;
                    while (str_beg && *str_beg && match(str_beg)) {
                        std::ostringstream oss;
                        for (std::string::size_type i = 0; i != fmt_str.length(); ++ i) {
                            if (fmt_str[i] == '\\' && i + 1 != fmt_str.length() && std::isdigit(fmt_str[i + 1])) {
                                int index = fmt_str[i + 1] - '0';
                                for (ssize_t j = submatch_start(index); j != submatch_end(index); ++ j)
                                {
                                    oss << char(str_beg[j]);
                                }
                                ++i;
                            }
                            else {
                                oss << fmt_str[i];
                            }
                        }
                        bit = oss.str();
                        str_beg += submatch_end(0);
                        ++cnt;
                    }
                    return cnt;
                }
                catch (std::exception & e) {
                    std::cout << __FILE__ << ", " << __LINE__ << std::endl;
                    throw;
                }
            }

        int  collectOstream(const std::string& str,
                            const std::string& fmt_str,
                            std::ostream& o,
                            const char * sep = "\n")
        {
            try {
                const char * str_beg = str.c_str();
                int cnt = 0;
                bool is_first = true;
                while (str_beg && *str_beg && match(str_beg)) {
                    if (is_first) {
                        o << sep;
                        is_first = false;
                    }
                    for (std::string::size_type i = 0; i != fmt_str.length(); ++ i) {
                        if (fmt_str[i] == '\\' && i + 1 != fmt_str.length() && std::isdigit(fmt_str[i + 1])) {
                            int index = fmt_str[i + 1] - '0';
                            for (ssize_t j = submatch_start(index); j != submatch_end(index); ++ j)
                            {
                                o << char(str_beg[j]);
                            }
                            ++i;
                        }
                        else {
                            o << fmt_str[i];
                        }
                    }
                    str_beg += submatch_end(0);
                    ++cnt;
                }
                return cnt;
            }
            catch (std::exception & e) {
                std::cout << __FILE__ << ", " << __LINE__ << std::endl;
                throw;
            }
        }

        int  collectOstream(const std::string& str,
                            std::ostream& o,
                            const char * sep = "\n")
        {
            try {
                const char * str_beg = str.c_str();
                int cnt = 0;
                bool is_first = true;
                while (str_beg && *str_beg && match(str_beg)) {
                    if (is_first) {
                        o << sep;
                        is_first = false;
                    }
                    o.write(str_beg + submatch_start(0), submatch_consumed(0));
                    str_beg += submatch_end(0);
                    ++cnt;
                }
                return cnt;
            }
            catch (std::exception & e) {
                std::cout << __FILE__ << ", " << __LINE__ << std::endl;
                throw;
            }
        }

    public:
        void build_errmsg() const;

    public:
        bool is_init() const;

        bool is_icase() const
        {
            return this->is_init() && (this->_mode & REG_ICASE);
        }

        bool is_extended() const
        {
            return this->is_init() && (this->_mode & REG_EXTENDED);
        }

        bool is_nosub() const
        {
            return this->is_init() && (this->_mode & REG_NOSUB);
        }

    public:

        const std::string& regstr() const
        {
            return this->_regstr;
        }

        std::string submatch(int idx) const
        {
            return this->_target.substr(this->submatch_start(idx),
                                        this->submatch_consumed(idx));
        }

        int         submatch_count() const
        {
            return this->_submatches.size();
        }

        bool is_subid_valid(int id) const
        {
            return (id >=0 && id < int(this->_submatches.size()) && this->_submatches[id].rm_so != -1);
        }

        int submatch_start(int id) const
        {
            return is_subid_valid(id) ? this->_submatches[id].rm_so : 0;
        }

        int submatch_end(int id) const
        {
            return is_subid_valid(id) ? this->_submatches[id].rm_eo : 0;
        }

        int submatch_consumed(int id) const
        {
            return this->submatch_end(id) - this->submatch_start(id);
        }

        void print(std::ostream& o) const;
        bool operator==(const CRegex& rhs) const
        {
            return this == &rhs || this->_regstr == rhs._regstr;
        }

    public:
        CRegex& operator = (const CRegex& );
        CRegex(const CRegex& );

        void swap(CRegex& ref)
        {
            std::swap(this->_regex, ref._regex);
            std::swap(this->_target, ref._target);
            std::swap(this->_regstr, ref._regstr);
            std::swap(this->_errcode, ref._errcode);
            std::swap(this->_errmsg, ref._errmsg);
            std::swap(this->_mode, ref._mode);
            std::swap(this->_submatches, ref._submatches);
        }

    private:

    private:
        // regex_t 本质上是一个非POD的结构体！是不能直接复制的方式进行拷贝的；
        regex_t                 _regex;
        mutable std::string     _target;
        std::string             _regstr;
        mutable std::string     _errmsg;
        mutable int             _errcode;
        int                     _mode;
        mutable std::vector<regmatch_t> _submatches;
    };

    inline std::ostream& operator << (std::ostream& o, const CRegex& r)
    {
        r.print(o);
        return o;
    }

    } // namespace regex
} // namespace sss

#endif  /* __CREGEX_HPP_1440427722__ */
