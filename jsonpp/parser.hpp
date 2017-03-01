#pragma once

#include <sss/raw_print.hpp>
#include <sss/string_view.hpp>
#include <sss/util/PostionThrow.hpp>

// /home/sarrow/project/Lisp/varLisp/src/varlisp/json/parser.hpp:15
// stream-parser
// onObjectStart()
// onObjectEnd(int cnt)
// onArrayStart()
// onArrayEnd(int cnt)
// onString()
// onInt()
// onDouble()
// onNill()
// onTure()
// onFalse()
//
// 创建基本的解析函数，用来构建sax等等风格的解析器；并支持随时停止解析循环
// 另外，内部需要保持一个栈结构，用来记录深度等信息，以便获取特定的记录；
// 比如skipWhitSpace,skipComma,skipColon等等，这些自动完成；
// 对用户透明。
// 另外，用户还可以使用skip()，以便随时跳出对某结构(object，array)的解析
// 当然，实际上是不能避免解析的，只是说用户不用获取数据，解析的时候，内部也不
// 用缓存睡觉觉；另外，还可以考虑move语义的支持；
// 最终，将json的解析，与json的二进制内存表示分割开，完成解耦
namespace sss
{
namespace json {

class Parser
{
public:
    enum json_element_t {
        kJE_OBJECT_START = 1 << 0,  // 1 '{'
        kJE_OBJECT_END   = 1 << 1,  // 2 '}'
        kJE_ARRAY_START  = 1 << 2,  // 4 '['
        kJE_ARRAY_END    = 1 << 3,  // 8 ']'
        kJE_NULL         = 1 << 4,  // 16 "null"
        kJE_TRUE         = 1 << 5,  // 32 "true"
        kJE_FALSE        = 1 << 6,  // 64 "false"
        kJE_NUMBER       = 1 << 7,  // 128 \\d；json中的数字，必然是数字开头;还有负号
        kJE_STRING       = 1 << 8,  // 256 '"'
        kJE_COLON        = 1 << 9,  // 512 ':'
        kJE_COMMA        = 1 << 10, // 1024 ','
        kJE_EOF          = 1 << 11, // 2048 end-of-file -> 应该设置为0；
        kJE_WHITESPACE   = 1 << 12, // 4096
        kJE_ERROR        = 1 << 13, //
    };
    // TODO inputstream;
    Parser() {}
    // 检查类型
    json_element_t peekType(sss::string_view& s);
    int           consumeObjectStart(sss::string_view& s);
    int           consumeKey(sss::string_view& s, std::string& key);
    int           consumeString(sss::string_view& s, std::string& str);
    int           consumeDouble(sss::string_view& s, double& v);
    int           consumeInt64(sss::string_view& s,  int64_t& v);
    int           consumeNumber(sss::string_view& s,  std::tuple<double, int64_t>& v, int& index);
    // 完全精度的浮点数解析，参见：
    // /home/sarrow/Sources/JsonLib/rapidjson/include/rapidjson/reader.h:1362
    int           consumeObjectEnd(sss::string_view& s);
    int           consumeArrayStart(sss::string_view& s);
    int           consumeArrayEnd(sss::string_view& s);
    int           consumeNull(sss::string_view& s);
    int           consumeTrue(sss::string_view& s);
    int           consumeFalse(sss::string_view& s);
    int           consumeWhiteSpace(sss::string_view& s);

    int           consumeObject(sss::string_view& s);
    int           consumeArray(sss::string_view& s);
    int           consumeValue(sss::string_view& s);

    int consumeComma(sss::string_view& s)
    {
        return this->consume(s, ',');
    }
    int consumeColon(sss::string_view& s)
    {
        return this->consume(s, ':');
    }

protected:

    bool consume(sss::string_view& s, char c)
    {
        if (!s.empty() && s.front() == c) {
            s.pop_front();
            return true;
        }
        return false;
    }

public:
    template<typename T>
    void consume_or_throw(sss::string_view& s, const T& v, sss::string_view msg)
    {
        if (!this->consume(s, v)) {
            if (s.empty()) {
                SSS_POSITION_THROW(std::runtime_error, "enconter: eof; ", msg.to_string());
            }
            else {
                SSS_POSITION_THROW(std::runtime_error, "enconter: ", sss::raw_char(s.front()), msg.to_string());
            }
        }
    }

    // bool parse(Stream& s, SAX_Handle & h);
    // sss::string_view m_stream;
};

} // namespace json
} /* sss */ 
