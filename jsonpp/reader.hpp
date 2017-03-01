#pragma once

#include <vector>
#include <stdexcept>

#include <sss/colorlog.hpp>
#include <sss/jsonpp/parser.hpp>
#include <sss/string_view.hpp>
#include <sss/raw_print.hpp>
#include <sss/debug/value_msg.hpp>

namespace sss {
namespace json {

enum state_t {
    kExpectArrayObjectStart = sss::json::Parser::kJE_ARRAY_START |
        sss::json::Parser::kJE_OBJECT_START,

    kExpectName = sss::json::Parser::kJE_STRING,
    kExpectObjectEnd = sss::json::Parser::kJE_OBJECT_END,
    kExpectArrayEnd = sss::json::Parser::kJE_ARRAY_END,
    kExpectNameOrObjectEnd =
        sss::json::Parser::kJE_STRING | sss::json::Parser::kJE_OBJECT_END,
    // sss::json::Parser::kJE_STRING | sss::json::Parser::kJE_OBJECT_END | sss::json::kJE_COLON,
    kExpectValue =
        sss::json::Parser::kJE_NULL | sss::json::Parser::kJE_NUMBER |
        sss::json::Parser::kJE_TRUE | sss::json::Parser::kJE_FALSE |
        sss::json::Parser::kJE_STRING | kExpectArrayObjectStart,

    kExpectValueOrArrayEnd =
        kExpectValue | sss::json::Parser::kJE_ARRAY_END,
    kExpectComma = sss::json::Parser::kJE_COMMA,
    kExpectColon = sss::json::Parser::kJE_COLON,
    kExpectNone = 0,  // EOF
};

inline state_t operator | (state_t lhs, state_t rhs)
{
    return static_cast<state_t>(static_cast<int>(lhs) |
                                static_cast<int>(rhs));
}

inline state_t operator |= (state_t lhs, state_t rhs)
{
    return static_cast<state_t>(static_cast<int>(lhs) |
                                static_cast<int>(rhs));
}

template<typename SAX_handle_t>
class Reader
{
public:
    int parse(sss::string_view s, SAX_handle_t& h);

    Reader() : m_state(kExpectArrayObjectStart) {}
    ~Reader() = default;

public:

protected:
    bool or_throw(const char * msg, sss::string_view s)
    {
        SSS_POSITION_THROW(std::runtime_error, "expect ", msg, ", but ", sss::raw_string(s));
    }

    state_t next_element_state() const {
        return (m_array_or_map.back() ? kExpectObjectEnd : kExpectArrayEnd) | kExpectComma;
    }

private:
    sss::string_view  m_json_str;
    state_t           m_state;
    std::vector<bool> m_array_or_map; // false: array; true: object
    std::vector<int>  m_elements_cnt;
};

template<typename SAX_handle_t>
int Reader<SAX_handle_t>::parse(sss::string_view s, SAX_handle_t& h)
{
    this->m_state = kExpectArrayObjectStart;
    this->m_json_str = s;
    sss::json::Parser p;
    bool is_ok = true;
    auto element_type = sss::json::Parser::kJE_EOF;
    while (is_ok) {
        element_type = p.peekType(s);
        switch (element_type) {
            case sss::json::Parser::kJE_OBJECT_START:
                if (m_state & element_type) {
                    p.consumeObjectStart(s) || or_throw("object start {", s);
                    h.StartObject();
                    m_array_or_map.push_back(true);
                    m_elements_cnt.push_back(0);
                    m_state = kExpectNameOrObjectEnd;
                }
                else {
                    SSS_POSITION_THROW(std::runtime_error, "un Expect start of Object");
                }
                break;

            case sss::json::Parser::kJE_OBJECT_END:
                if (m_state & element_type) {
                    p.consumeObjectEnd(s) || or_throw("object end }", s);
                    h.EndObject(m_elements_cnt.back());
                    m_array_or_map.pop_back();
                    m_elements_cnt.pop_back();
                    if (m_array_or_map.empty()) {
                        m_state = kExpectNone;
                    }
                    else {
                        // 逗号？还有，上一层的计数，何时+1？遇到'[','{'就增加，还是等完成，']','}'？
                        ++m_elements_cnt.back();
                        m_state = this->next_element_state();
                    }
                }
                else {
                    SSS_POSITION_THROW(std::runtime_error, "un Expect end of Object");
                }
                break;

            case sss::json::Parser::kJE_ARRAY_START:
                if (m_state & element_type) {
                    p.consumeArrayStart(s) || or_throw("array start [", s);
                    h.StartArray();
                    m_array_or_map.push_back(false);
                    m_elements_cnt.push_back(0);
                    m_state = kExpectValueOrArrayEnd;
                }
                else {
                    SSS_POSITION_THROW(std::runtime_error, "un Expect start of array");
                }
                break;

            case sss::json::Parser::kJE_ARRAY_END:
                if (m_state & element_type) {
                    p.consumeArrayEnd(s) || or_throw("array end ]", s);
                    h.EndArray(m_elements_cnt.back());
                    m_array_or_map.pop_back();
                    m_elements_cnt.pop_back();
                    if (m_array_or_map.empty()) {
                        m_state = kExpectNone;
                    }
                    else {
                        // 逗号？还有，上一层的计数，何时+1？遇到'[','{'就增加，还是等完成，']','}'？
                        ++m_elements_cnt.back();
                        m_state = this->next_element_state();
                    }
                }
                else {
                    SSS_POSITION_THROW(std::runtime_error, "un Expect end of Object");
                }
                break;

            case sss::json::Parser::kJE_NULL:
                if (m_state & element_type) {
                    p.consumeNull(s) || or_throw("null", s);
                    h.Null();
                    ++m_elements_cnt.back();
                    m_state = this->next_element_state();
                }
                else {
                    SSS_POSITION_THROW(std::runtime_error, "un Expect null");
                }
                break;

            case sss::json::Parser::kJE_TRUE:
                if (m_state & element_type) {
                    p.consumeTrue(s) || or_throw("true", s);
                    h.Bool(true);
                    ++m_elements_cnt.back();
                    m_state = this->next_element_state();
                }
                else {
                    SSS_POSITION_THROW(std::runtime_error, "un Expect true");
                }
                break;

            case sss::json::Parser::kJE_FALSE:
                if (m_state & element_type) {
                    p.consumeFalse(s) || or_throw("false", s);
                    h.Bool(false);
                    ++m_elements_cnt.back();
                    m_state = this->next_element_state();
                }
                else {
                    SSS_POSITION_THROW(std::runtime_error, "un Expect false");
                }
                break;

            case sss::json::Parser::kJE_NUMBER:
                // FIXME 整数与浮点数……
                if (m_state & element_type) {
                    std::tuple<double, int64_t> number;
                    int index = -1;
                    p.consumeNumber(s, number, index) || or_throw("number", s);
                    if (index == 0) {
                        h.Double(std::get<0>(number));
                    }
                    else {
                        h.Int64(std::get<1>(number));
                    }
                    ++m_elements_cnt.back();
                    m_state = this->next_element_state();
                }
                else {
                    SSS_POSITION_THROW(std::runtime_error, "un Expect number");
                }
                break;

            case sss::json::Parser::kJE_STRING:
                if (m_state & element_type) {
                    std::string sValue;
                    // 三种情况：
                    // 1. array中的 element
                    // 2. object kv 的 name；
                    // 3. object kv 的 value
                    if (!m_array_or_map.back()) {
                        p.consumeString(s, sValue) || or_throw("string", s);
                        h.String(sss::string_view(sValue));
                        m_state = this->next_element_state();
                    }
                    else {
                        if ((m_state & kExpectValue) == kExpectValue) {
                            p.consumeString(s, sValue) || or_throw("string", s);
                            h.String(sss::string_view(sValue));
                            m_state = this->next_element_state();
                        }
                        else {
                            // (m_state & kExpectNameOrObjectEnd) == kExpectNameOrObjectEnd
                            p.consumeString(s, sValue) || or_throw("key", s);
                            h.Key(sss::string_view(sValue));
                            m_state = kExpectColon;
                        }
                    }
                }
                else {
                    SSS_POSITION_THROW(std::runtime_error, "un Expect string");
                }
                break;

            case sss::json::Parser::kJE_COLON:
                if (m_state & element_type) {
                    p.consumeColon(s) || or_throw(":", s);
                    m_state = kExpectValue;
                }
                else {
                    SSS_POSITION_THROW(std::runtime_error, "un Expect :");
                }
                break;

            case sss::json::Parser::kJE_COMMA:
                if (m_state & element_type) {
                    p.consumeComma(s) || or_throw(",", s);
                    if (m_array_or_map.back()) {
                        m_state = kExpectName;
                    }
                    else {
                        m_state = kExpectValue;
                    }
                }
                else {
                    SSS_POSITION_THROW(std::runtime_error, "un Expect ,");
                }
                break;

            case sss::json::Parser::kJE_EOF:
                if (m_state == kExpectNone) {
                    is_ok = false;
                }
                break;

            case sss::json::Parser::kJE_WHITESPACE:
                // NOTE 应该统计行号，以便出错的时候，提供信息……
                p.consumeWhiteSpace(s);
                break;

            case sss::json::Parser::kJE_ERROR:
                // NOTE 或者，保留最后一次成功的状态？
                m_state = kExpectNone;
                is_ok = false;
                break;

            default:
                is_ok = false;
                break;
        }
    }
}

// 什么也不做，只是用来提供继承的基类
struct DummyHandle
{
    bool Null()
    {
        return false;
    }
    bool Bool(bool )
    {
        return false;
    }
    bool Int64(int64_t )
    {
        return false;
    }
    bool Double(double )
    {
        return false;
    }
    // bool RawNumber(const Ch* str, SizeType length, bool copy);
    bool String(sss::string_view )
    {
        return false;
    }
    bool StartObject()
    {
        return false;
    }
    bool Key(sss::string_view )
    {
        return false;
    }
    bool EndObject(int )
    {
        return false;
    }
    bool StartArray()
    {
        return false;
    }
    bool EndArray(int )
    {
        return false;
    }
};

struct MessageHandle
{
    bool Null()
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << "null" << std::endl;
        return true;
    }
    bool Bool(bool b)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << (b ? "true" : "false") << std::endl;
        return true;
    }
    bool Int64(int64_t i)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << i << std::endl;
        return true;
    }
    bool Double(double d)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << d << std::endl;
        return true;
    }

    bool String(sss::string_view s)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << sss::raw_string(s) << std::endl;
        return true;
    }
    bool StartObject()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        return true;
    }
    bool Key(sss::string_view s)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << sss::raw_string(s) << std::endl;
        return true;
    }
    bool EndObject(int memberCount)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << memberCount << std::endl;
        return true;
    }
    bool StartArray()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        return true;
    }
    bool EndArray(int memberCount)
    {
        std::cout << __PRETTY_FUNCTION__ << ":" << memberCount << std::endl;
        return true;
    }
};

struct PrintHandle
{
    PrintHandle(std::ostream& o) : m_o(o) {}
    std::ostream& m_o;
    std::vector<bool> m_array_or_map;
    std::vector<int>  m_elements_cnt;
    void auto_comma()
    {
        if (!m_array_or_map.back() && m_elements_cnt.back()++) {
            m_o << ',';
        }
    }
    bool Null()
    {
        auto_comma();
        m_o << "null";
        return true;
    }
    bool Bool(bool b)
    {
        auto_comma();
        m_o << (b ? "true" : "false");
        return true;
    }
    bool Int64(int64_t i)
    {
        auto_comma();
        m_o << i;
        return true;
    }
    bool Double(double d)
    {
        auto_comma();
        m_o << d;
        return true;
    }
    bool String(sss::string_view s)
    {
        auto_comma();
        m_o << sss::raw_string(s);
        return true;
    }
    bool StartObject()
    {
        if (!m_array_or_map.empty()) {
            auto_comma();
        }
        m_array_or_map.push_back(true);
        m_elements_cnt.push_back(0);
        m_o << '{';
        return true;
    }
    bool Key(sss::string_view s)
    {
        if (m_array_or_map.back() && m_elements_cnt.back()++) {
            m_o << ',';
        }
        m_o << sss::raw_string(s) << ':';
        return true;
    }
    bool EndObject(int)
    {
        m_array_or_map.pop_back();
        m_elements_cnt.pop_back();
        m_o << '}';
        return true;
    }
    bool StartArray()
    {
        if (!m_array_or_map.empty()) {
            auto_comma();
        }
        m_array_or_map.push_back(false);
        m_elements_cnt.push_back(0);
        m_o << '[';
        return true;
    }
    bool EndArray(int )
    {
        m_array_or_map.pop_back();
        m_elements_cnt.pop_back();
        m_o << ']';
        return true;
    }
};

// {
//   "hello": "123",
//   "m-list": [
//     1,
//     2,
//     3,
//     null
//   ],
//   "true": true
// }
struct PrettyPrintHandle
{
    PrettyPrintHandle(std::ostream& o, const std::string& indent = "  ")
        : m_o(o)
    {
        for (size_t i = 0; i < indent.size() && std::isspace(indent[i]); ++i) {
            m_indent.push_back(indent[i]);
        }
    }
    std::ostream& m_o;
    std::string m_indent;
    std::vector<bool> m_array_or_map;
    std::vector<int>  m_elements_cnt;
    void auto_comma()
    {
        if (!m_array_or_map.back() && m_elements_cnt.back()++) {
            m_o << ',';
        }
        if (!m_array_or_map.back()) {
            m_o << "\n";
        }
    }
    void use_indent()
    {
        if (m_array_or_map.empty() || m_array_or_map.back()) {
            return;
        }
        for (size_t i = 0; i < m_array_or_map.size(); ++i) {
            m_o << m_indent;
        }
    }
    bool Null()
    {
        auto_comma();
        use_indent();
        m_o << "null";
        return true;
    }
    bool Bool(bool b)
    {
        auto_comma();
        use_indent();
        m_o << (b ? "true" : "false");
        return true;
    }
    bool Int64(int64_t i)
    {
        auto_comma();
        use_indent();
        m_o << i;
        return true;
    }
    bool Double(double d)
    {
        auto_comma();
        use_indent();
        m_o << d;
        return true;
    }
    bool String(sss::string_view s)
    {
        auto_comma();
        use_indent();
        m_o << sss::raw_string(s);
        return true;
    }
    // NOTE 只有在每次输出内部元素之后，才会输出换行符，以及使用缩进。
    // 也就是说，如果内部没有元素，就会紧挨着前导的'{'，输出'}'
    bool StartObject()
    {
        if (!m_array_or_map.empty()) {
            auto_comma();
        }

        use_indent();

        m_array_or_map.push_back(true);
        m_elements_cnt.push_back(0);

        m_o << '{';
        return true;
    }
    bool Key(sss::string_view s)
    {
        if (m_array_or_map.back() && m_elements_cnt.back()++) {
            m_o << ',';
        }
        m_o << '\n';
        for (size_t i = 0; i < m_array_or_map.size(); ++i) {
            m_o << m_indent;
        }
        m_o << sss::raw_string(s) << ':' << ' ';
        return true;
    }
    bool EndObject(int)
    {
        auto ele_cnt = m_elements_cnt.back();
        if (ele_cnt) {
            m_o << '\n';
        }

        m_array_or_map.pop_back();
        m_elements_cnt.pop_back();

        if (ele_cnt) {
            use_indent();
        }
        m_o << '}';
        return true;
    }
    bool StartArray()
    {
        if (!m_array_or_map.empty()) {
            auto_comma();
        }

        use_indent();
        m_array_or_map.push_back(false);
        m_elements_cnt.push_back(0);

        m_o << '[';
        return true;
    }
    bool EndArray(int )
    {
        auto ele_cnt = m_elements_cnt.back();
        if (ele_cnt) {
            m_o << '\n';
        }

        m_array_or_map.pop_back();
        m_elements_cnt.pop_back();

        if (ele_cnt) {
            for (size_t i = 0; i < m_array_or_map.size(); ++i) {
                m_o << m_indent;
            }
        }
        m_o << ']';
        return true;
    }
};

} // namespace json
} // namespace sss
