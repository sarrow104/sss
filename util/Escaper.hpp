#include <iosfwd>
#include <string>
#include <sstream>

namespace sss{
    namespace util {
        // 比如linux命令行参数，转义空格等字符……
        // 仅针对ascii字符进行转义；
        // 特殊问题：
        //    孤立的转义字符，如何处理？
        //    比如c_str中，"\\"是用于转义的字符，如果字符串，恰好以"\\"结尾；会
        //    怎么样？
        //    当然；由于双引号本身也是需要转义的；所以这不是问题；
        //    但是，printf中的格式字符串，就有问题了；格式字符串中，是以"%"为转
        //    义字符的；
        //    就是说，"%%"表示一个"%"输出；其他的，则表示特定的输出……
        class Escaper
        {
        public:
            // escape_setting 的第一个字符是转义字符；后续的都是被转义字符；
            // 通常转义字符本身
            Escaper();
            explicit Escaper(const std::string& escape_setting);
            ~Escaper();

        public:
            void assign(const std::string& escape_setting = "");
            bool good() const;

            bool isEscapeChar(char ch) const;
            bool isNeedEscape(char ch) const;

        public:
            void escapeToStream(std::ostream& o, const std::string& s);
            void unescapeToStream(std::ostream& o, const std::string& s);

            std::string escape(const std::string& s);
            std::string unescape(const std::string& s);

        private:
            char _escape_char;
            char _escape_table[128];

        public:
            static
            std::string escape(const std::string& escape_setting, const std::string& s)
            {
                std::ostringstream oss;
                Escaper tmp_esc(escape_setting);
                tmp_esc.escapeToStream(oss, s);
                return oss.str();
            }
            static
            std::string unescape(const std::string& escape_setting, const std::string& s)
            {
                std::ostringstream oss;
                Escaper tmp_esc(escape_setting);
                tmp_esc.unescapeToStream(oss, s);
                return oss.str();
            }
        };

        bool isQuoted(const std::string& s, char escape = '\\');
        inline bool isDquoted(const std::string& s, char escape = '\\')
        {
            return isQuoted(s, escape) && s[0] == '"';
        }
        inline bool isSquoted(const std::string& s, char escape = '\\')
        {
            return isQuoted(s, escape) && s[0] == '\'';
        }
        std::string& dquote(std::string& s, char escape = '\\');
        std::string& squote(std::string& s, char escape = '\\');
        std::string& unquote(std::string& s, char escape = '\\');

        inline std::string dquote_copy(const std::string& s, char escape = '\\')
        {
            std::string ret(s);
            return dquote(ret, escape);
        }
        inline std::string squote_copy(const std::string& s, char escape = '\\')
        {
            std::string ret(s);
            return squote(ret, escape);
        }
        inline std::string unquote_copy(const std::string& s, char escape = '\\')
        {
            std::string ret(s);
            return unquote(ret, escape);
        }
    }
}

