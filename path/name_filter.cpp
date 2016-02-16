#include "name_filter.hpp"


namespace sss{
    namespace path{

        name_filter_t::name_filter_t(const std::string& pattern)
        {
            // NOTE 特殊处理字符：' ','*','?',';'
#ifdef __WIN32__
            // NOTE "\r\n"可以出现在文件名中……不过得程序生成；无法手写进去；
            const std::string forbid_chars = "/\\<>\":<>|";
#else
            const std::string forbid_chars = "/\\<>\"";
#endif
            std::string::const_iterator it =
                std::find_first_of(pattern.begin(), pattern.end(),
                                   forbid_chars.begin(), forbid_chars.end());

            if (it != pattern.end()) {
                throw std::runtime_error(std::string("invalid path containing `")
                                         + *it
                                         + "`");
            }

            this->_regex.compile(name_filter_t::gen_pattern_regstr(pattern),
                                 REG_EXTENDED | REG_ICASE | REG_NOSUB);
        }

        name_filter_t::~name_filter_t()
        {
        }

        std::string name_filter_t::gen_pattern_regstr(const std::string& pattern)
        {
            // 思路很简单，替换大法：
            // '*' -> ".*"
            // '.' -> "\."
            // ' ' -> ""
            // ';' -> "|"
            //
            // 不过，pattern中，不应该包括 "\\"，"/"，"<",">" 这种符号；
            // 另外，'?'可以考虑替换为"."；
            // 最后，加上首尾标记：
            // 不过，多后缀怎么办？
            // linux\windows下，不同行为如何控制？
            // 所以，还是都用 regex的方式吧！
            // 反正足够强大；
            std::string ret(pattern);
            sss::replace_all(ret, ".", "\\.");
            sss::replace_all(ret, "?", ".");
            sss::replace_all(ret, "*", ".*");
            sss::replace_all(ret, " ", "");
            sss::replace_all(ret, "\n", "");
            sss::replace_all(ret, "\r", "");
            sss::replace_all(ret, "\t", "");
            sss::replace_all(ret, ";", "|");
            sss::replace_all(ret, "(", "\\(");
            sss::replace_all(ret, ")", "\\)");
            sss::replace_all(ret, "[", "\\[");
            sss::replace_all(ret, "]", "\\]");

            if (ret.empty()) {
                ret = ".*";
            }
            if (ret.find("|") != std::string::npos) {
                ret = "(" + ret + ")";
            }
            ret = "^" + ret + "$";
            return ret;
        }
    }
}
