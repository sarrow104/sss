#include "Escaper.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace sss{
    namespace util {

        Escaper::Escaper(const std::string& escape_setting)
        {
            this->assign(escape_setting);
        }

        Escaper::~Escaper()
        {
        }

        void Escaper::assign(const std::string& escape_setting)
        {
            this->_escape_char = '\0';
            std::memset(this->_escape_table, 0, sizeof(this->_escape_table));
            if (2u <= escape_setting.length()) {
                this->_escape_char = escape_setting[0];
                for (size_t i = 0; i != escape_setting.length(); ++i) {
                    if (!(0x80u & escape_setting[i])) {
                        this->_escape_table[int(escape_setting[i])] = 1;
                    }
                }
            }
        }

        bool Escaper::good() const
        {
            return this->_escape_char;
        }

        bool Escaper::isEscapeChar(char ch) const
        {
            return this->_escape_char == ch;
        }

        bool Escaper::isNeedEscape(char ch) const
        {
            return !(0x80u & ch) && this->_escape_table[int(ch)];
        }

        void Escaper::escapeToStream(std::ostream& o, const std::string& s)
        {
            if (this->good()) {
                for (std::string::const_iterator it = s.begin();
                     it != s.end();
                     ++it)
                {
                    if (this->isNeedEscape(*it)) {
                        o << this->_escape_char;
                    }
                    o << *it;
                }
            }
            else {
                o << s;
            }
        }

        void Escaper::unescapeToStream(std::ostream& o, const std::string& s)
        {
            if (this->good()) {
                bool is_prev_escape = false;

                for (std::string::const_iterator it = s.begin();
                     it != s.end();
                     ++it)
                {
                    if (!is_prev_escape && this->isEscapeChar(*it)) {
                        is_prev_escape = true;
                        continue;
                    }

                    if (is_prev_escape) {
                        if (!this->isNeedEscape(*it)) {
                            o << this->_escape_char;
                        }
                        o << *it;

                        is_prev_escape = false;
                    }
                    else {
                        o << *it;
                    }
                }
                if (is_prev_escape) {
                    o << this->_escape_char;
                    is_prev_escape = false;
                }
            }
            else {
                o << s;
            }
        }

        std::string Escaper::escape(const std::string& s)
        {
            std::ostringstream oss;
            this->escapeToStream(oss, s);
            return oss.str();
        }

        std::string Escaper::unescape(const std::string& s)
        {
            std::ostringstream oss;
            this->unescapeToStream(oss, s);
            return oss.str();
        }

        bool isQuoted(const std::string& s, char escape_char)
        {
            if (s.length() >= 2) {
                char quote_ch = s[0];
                if (quote_ch == '\'' || quote_ch == '"') {
                    if (*s.rbegin() == quote_ch) {
                        int escape_char_cnt = 0;
                        for (size_t i = s.length() - 2;  i > 0 && s[i] == escape_char; --i) {
                            escape_char_cnt++;
                        }
                        return (escape_char_cnt & 0x1u) == 0;
                    }
                }
            }
            return false;
        }

        // 前后加上双引号；并且转义内部已有的双引号！
        // 注意，不会管，是否已经是dquote状态！
        std::string& dquote(std::string& s, char escape_char)
        {
            std::string ret;
            ret += '"';
            for (size_t i = 0;  i < s.length(); ++i) {
                if (s[i] == '"' || s[i] == escape_char) {
                    ret += escape_char;
                }
                ret += s[i];
            }
            ret += '"';
            std::swap(s, ret);
            return s;
        }

        std::string& squote(std::string& s, char escape_char)
        {
            std::string ret;
            ret += '\'';
            for (size_t i = 0;  i < s.length(); ++i) {
                if (s[i] == '\'' || s[i] == escape_char) {
                    ret += escape_char;
                }
                ret += s[i];
            }
            ret += '\'';
            std::swap(s, ret);
            return s;
        }

        std::string& unquote(std::string& s, char escape_char)
        {
            if (isQuoted(s, escape_char)) {
                char quote_ch = s[0];
                int len = 0;
                for (size_t i = 1; i < s.length() - 1; ++i) {
                    if (s[i] == escape_char && i + 1 < s.length() &&
                        (s[i + 1] == quote_ch || s[i + 1] == escape_char))
                    {
                        s[len++] = s[i++ + 1];
                    }
                    else {
                        s[len++] = s[i];
                    }
                }
                s.resize(len);
            }
            return s;
        }
    }
}
