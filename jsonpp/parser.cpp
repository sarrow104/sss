#include "parser.hpp"

#include <cctype>
#include <tuple>

#include <sss/utlstring.hpp>
#include <sss/util/utf8.hpp>
#include <sss/raw_print.hpp>
#include <sss/colorlog.hpp>

#include <sss/debug/value_msg.hpp>

namespace sss
{
namespace json {

Parser::json_element_t Parser::peekType(sss::string_view& s)
{
    // this->consumeWhiteSpace(s);
    if (s.empty()) {
        return Parser::kJE_EOF;
    }
    switch (s.front()) {
        case '"': return Parser::kJE_STRING;
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
                 return Parser::kJE_NUMBER;
        case 't': return Parser::kJE_TRUE;
        case 'f': return Parser::kJE_FALSE;
        case 'n': return Parser::kJE_NULL;
        case '{': return Parser::kJE_OBJECT_START;
        case '}': return Parser::kJE_OBJECT_END;
        case '[': return Parser::kJE_ARRAY_START;
        case ']': return Parser::kJE_ARRAY_END;
        case ':': return Parser::kJE_COLON;
        case ',': return Parser::kJE_COMMA;
        default:
            if (std::isspace(s.front())) {
                return Parser::kJE_WHITESPACE;
            }
            else {
                return Parser::kJE_ERROR;
            }
    }
}

int           Parser::consumeObjectStart(sss::string_view& s)
{
    return this->consume(s, '{');
}

int           Parser::consumeKey(sss::string_view& s, std::string& key)
{
    COLOG_DEBUG(SSS_VALUE_MSG(s));
    return this->consumeString(s, key);
}

int           Parser::consumeString(sss::string_view& s, std::string& str)
{
    if (!this->consume(s, '\"')) {
        return 0;
    }
    auto old_start = s.data();
    str.resize(0);

    while (!s.empty() && s.front() != '"') {
        switch (s.front()) {
            case '\\':
                {
                    if (s.size() < 2) {
                        SSS_POSITION_THROW(std::runtime_error,
                                           "unfinished escape char", s);
                    }
                    const char * it_b2 = s.begin();
                    ++it_b2;
                    switch (*it_b2) {
                        case '"':   // quotation mark
                            str.push_back('"');
                            s.remove_prefix(2);
                            break;
                        case '\\':  // reverse solidus
                            str.push_back('\\');
                            s.remove_prefix(2);
                            break;
                        case '/':   // solidus
                            str.push_back('/');
                            s.remove_prefix(2);
                            break;
                        case 'b':   // backspace
                            str.push_back('\b');
                            s.remove_prefix(2);
                            break;
                        case 'f':   // formfeed
                            str.push_back('\f');
                            s.remove_prefix(2);
                            break;
                        case 'n':   // newline
                            str.push_back('\n');
                            s.remove_prefix(2);
                            break;
                        case 'r':   // carriage return
                            str.push_back('\r');
                            s.remove_prefix(2);
                            break;

                        case 't':   // horizontal tab
                            str.push_back('\t');
                            s.remove_prefix(2);
                            break;

                        case 'u':   // 4hexadecimal digits
                            // \"\u77e5\u4e4e\u7528\u6237\"
                            {
                                int32_t code_point = 0u;
                                if (s.size() < 6) {
                                    SSS_POSITION_THROW(std::runtime_error,
                                                       "unfinished ascii unicode", s);
                                }
                                const char * it_hex = s.begin() + 2;
                                for (int i = 0; i < 4; ++i, ++it_hex) {
                                    if (!std::isxdigit(*it_hex)) {
                                        SSS_POSITION_THROW(std::runtime_error,
                                                           "unfinished ascii unicode", s);
                                    }
                                    code_point *= 16;
                                    code_point += sss::hex2int(*it_hex);
                                }
                                sss::util::utf8::dumpout2utf8(&code_point, &code_point + 1, std::back_inserter(str));
                                s.remove_prefix(6);
                            }
                            break;

                        default:
                            SSS_POSITION_THROW(std::runtime_error,
                                               "unkown escape sequence", s);
                    }
                }
                break;

            default:
                if (std::iscntrl(s.front())) {
                    SSS_POSITION_THROW(std::runtime_error,
                                       "unkonw control char", sss::raw_char(s.front()));
                }
                else {
                    str.push_back(s.front());
                    s.pop_front();
                }
                break;
        }
    }
    this->consume_or_throw(s, '"', "expect '\"'");
    return s.data() - old_start;
}

int           Parser::consumeDouble(sss::string_view& s, double& v)
{
        sss::string_view s_saved = s;
        bool is_double = false;
        switch (s.front()) {
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            break;

        default:
            SSS_POSITION_THROW(std::runtime_error,
                               "unkonw char", sss::raw_char(s.front()));
        }
        // 先跳过负号
        if (s.front() == '-') {
            s.pop_front();
            if (s.empty()) {
                SSS_POSITION_THROW(std::runtime_error,
                                   "unfinished -number");
            }
        }

        // 整数部分
        if (s.front() == '0' && (s.size() >= 2 && !std::isdigit(s[1]))) {
            s.pop_front();
        }
        else if ('1' <= s.front() && s.front() <= '9') {
            while (!s.empty() && std::isdigit(s.front())) {
                s.pop_front();
            }
        }
        else {
            SSS_POSITION_THROW(std::runtime_error,
                               "unfinished number", s);
        }

        // 小数部分
        if (s.size() >= 2 && s[0] == '.' && std::isdigit(s[1]))
        {
            is_double = true;
            s.remove_prefix(2);
            while (!s.empty() && std::isdigit(s.front())) {
                s.pop_front();
            }
        }

        // e指数部分
        if (!s.empty() && std::toupper(s[0]) == 'E') {
            s.pop_front();
            is_double = true;
            this->consume(s, '+') ||this->consume(s, '-');
            int e_cnt = 0;
            while (!s.empty() && std::isdigit(s.front())) {
                s.pop_front();
                e_cnt++;
            }
            if (!e_cnt) {
                SSS_POSITION_THROW(std::runtime_error,
                                   "unfinished e-number", s);
            }
        }

        v = sss::string_cast<double>(std::string(s_saved.begin(), s.begin()));
        return s.begin() - s_saved.begin();
}

int           Parser::consumeInt64(sss::string_view& s,  int64_t& v)
{
    auto old_start = s.begin();
    switch (s.front()) {
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            break;

        default:
            SSS_POSITION_THROW(std::runtime_error,
                               "unkonw char", sss::raw_char(s.front()));
    }
    // 先跳过负号
    if (s.front() == '-') {
        s.pop_front();
        if (s.empty()) {
            SSS_POSITION_THROW(std::runtime_error,
                               "unfinished -number");
        }
    }

    // 整数部分
    if (s.front() == '0' && (s.size() >= 2 && !std::isdigit(s[1]))) {
        s.pop_front();
    }
    else if ('1' <= s.front() && s.front() <= '9') {
        while (!s.empty() && std::isdigit(s.front())) {
            s.pop_front();
        }
    }
    else {
        SSS_POSITION_THROW(std::runtime_error,
                           "unfinished number", s);
    }
    v = sss::string_cast<int64_t>(std::string(old_start, s.begin()));
    return s.begin() - old_start;
}

int           Parser::consumeNumber(sss::string_view& s,  std::tuple<double, int64_t>& v, int& index)
{
    COLOG_DEBUG(SSS_VALUE_MSG(s));
    sss::string_view s_saved = s;
    bool is_double = false;
    switch (s.front()) {
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            break;

        default:
            SSS_POSITION_THROW(std::runtime_error,
                               "unkonw char", sss::raw_char(s.front()));
    }
    // 先跳过负号
    if (s.front() == '-') {
        s.pop_front();
        if (s.empty()) {
            SSS_POSITION_THROW(std::runtime_error,
                               "unfinished -number");
        }
    }

    // 整数部分
    if (s.front() == '0' && (s.size() >= 2 && !std::isdigit(s[1]))) {
        s.pop_front();
    }
    else if ('1' <= s.front() && s.front() <= '9') {
        while (!s.empty() && std::isdigit(s.front())) {
            s.pop_front();
        }
    }
    else {
        SSS_POSITION_THROW(std::runtime_error,
                           "unfinished number", s);
    }

    // 小数部分
    if (s.size() >= 2 && s[0] == '.' && std::isdigit(s[1]))
    {
        is_double = true;
        s.remove_prefix(2);
        while (!s.empty() && std::isdigit(s.front())) {
            s.pop_front();
        }
    }

    // e指数部分
    if (!s.empty() && std::toupper(s[0]) == 'E') {
        s.pop_front();
        is_double = true;
        this->consume(s, '+') ||this->consume(s, '-');
        int e_cnt = 0;
        while (!s.empty() && std::isdigit(s.front())) {
            s.pop_front();
            e_cnt++;
        }
        if (!e_cnt) {
            SSS_POSITION_THROW(std::runtime_error,
                               "unfinished e-number", s);
        }
    }

    if (is_double) {
        std::get<0>(v) = sss::string_cast<double>(std::string(s_saved.begin(), s.begin()));
        index = 0;
    }
    else {
        std::get<1>(v) = sss::string_cast<int64_t>(std::string(s_saved.begin(), s.begin()));
        index = 1;
    }
    return s.begin() - s_saved.begin();
}

int           Parser::consumeObjectEnd(sss::string_view& s)
{
    return this->consume(s, '}');
}

int           Parser::consumeArrayStart(sss::string_view& s)
{
    return this->consume(s, '[');
}

int           Parser::consumeArrayEnd(sss::string_view& s)
{
    return this->consume(s, ']');
}

int           Parser::consumeNull(sss::string_view& s)
{
    COLOG_DEBUG(SSS_VALUE_MSG(s));
    auto old_start = s.begin();
    if (s.substr(0, 4) == sss::string_view("null")) {
        s.remove_prefix(4);
    }
    return s.begin() - old_start;
}

int           Parser::consumeTrue(sss::string_view& s)
{
    COLOG_DEBUG(SSS_VALUE_MSG(s));
    auto old_start = s.begin();
    if (s.substr(0, 4) == sss::string_view("true")) {
        s.remove_prefix(4);
    }
    return s.begin() - old_start;
}

int           Parser::consumeFalse(sss::string_view& s)
{
    COLOG_DEBUG(SSS_VALUE_MSG(s));
    auto old_start = s.begin();
    if (s.substr(0, 5) == sss::string_view("false")) {
        s.remove_prefix(5);
    }
    return s.begin() - old_start;
}

int           Parser::consumeWhiteSpace(sss::string_view& s)
{
    int consumed_cnt = 0;
    while (!s.empty() && std::isspace(s.front())) {
        ++consumed_cnt;
        s.pop_front();
    }
    return consumed_cnt;
}

int           Parser::consumeObject(sss::string_view& s)
{
    COLOG_DEBUG(SSS_VALUE_MSG(s));
    auto old_s = s;
    if (!s.empty() && this->peekType(s) == kJE_OBJECT_START) {
        this->consume_or_throw(s, '{', "expect '{'");
        bool is_first = true;
        while (!s.empty() && s.front() != '}') {
            if (is_first) {
                is_first = false;
            }
            else {
                this->consumeWhiteSpace(s);
                this->consume_or_throw(s, ',', "expect ','");
            }
            std::string key;
            this->consumeWhiteSpace(s);
            this->consumeString(s, key);
            this->consumeWhiteSpace(s);
            this->consume_or_throw(s, ':', "expect ':'");
            this->consumeWhiteSpace(s);
            this->consumeValue(s);
            this->consumeWhiteSpace(s);
        }
        this->consume_or_throw(s, '}', "expect '}'");
    }

    return old_s.size() - s.size();
}

int           Parser::consumeArray(sss::string_view& s)
{
    COLOG_DEBUG(SSS_VALUE_MSG(s));
    auto old_s = s;
    if (!s.empty() && this->peekType(s) == kJE_ARRAY_START) {
        this->consume_or_throw(s, '[', "expect '['");
        bool is_first = true;
        while (!s.empty() && s.front() != ']') {
            if (is_first) {
                is_first = false;
            }
            else {
                this->consumeWhiteSpace(s);
                this->consume_or_throw(s, ',', "expect ','");
            }
            this->consumeWhiteSpace(s);
            this->consumeValue(s);
            this->consumeWhiteSpace(s);
        }
        this->consume_or_throw(s, ']', "expect ']'");
    }
    return old_s.size() - s.size();
}

int           Parser::consumeValue(sss::string_view& s)
{
    auto old_s = s;
    COLOG_DEBUG(SSS_VALUE_MSG(s));

    switch (this->peekType(s)) {
        case sss::json::Parser::kJE_STRING:
            {
                std::string str;
                this->consumeString(s, str);
            }
            break;

        case sss::json::Parser::kJE_NUMBER:
            {
                std::tuple<double, int64_t> number;
                int index;
                this->consumeNumber(s, number, index);
            }
            break;

        case sss::json::Parser::kJE_TRUE:
            this->consumeTrue(s);
            break;

        case sss::json::Parser::kJE_FALSE:
            this->consumeFalse(s);
            break;

        case sss::json::Parser::kJE_NULL:
            this->consumeNull(s);
            break;

        case sss::json::Parser::kJE_OBJECT_START:
            this->consumeObject(s);
            break;

        case sss::json::Parser::kJE_ARRAY_START:
            this->consumeArray(s);
            break;

        default:
            SSS_POSITION_THROW(std::runtime_error,
                               "unexpect string ", sss::raw_string(s));
            break;
    }
    return old_s.size() - s.size();
}



} // namespace json
} /* sss */ 
