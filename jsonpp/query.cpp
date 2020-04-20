// $sss/jsonpp/query.cpp
#include "query.hpp"

namespace sss {
namespace json {

sss::string_view query_object_name(sss::string_view s, sss::string_view name)
{
    sss::string_view value;
    sss::json::Parser jp;
    jp.consumeWhiteSpace(s);
    COLOG_DEBUG(SSS_VALUE_MSG(s), SSS_VALUE_MSG(name));

    if (!s.empty() && jp.peekType(s) == sss::json::Parser::kJE_OBJECT_START) {
        jp.consume_or_throw(s, '{', "expect '{'");
        jp.consumeWhiteSpace(s);
        bool is_first = true;
        while (!s.empty() && s.front() != '}') {
            COLOG_DEBUG(SSS_VALUE_MSG(s));
            if (is_first) {
                is_first = false;
            }
            else {
                jp.consumeWhiteSpace(s);
                jp.consume_or_throw(s, ',', "expect ','");
            }
            std::string key;
            jp.consumeWhiteSpace(s);
            jp.consumeString(s, key) || (throw "expect key", 0);
            COLOG_DEBUG(SSS_VALUE_MSG(key));
            jp.consumeWhiteSpace(s);
            jp.consume_or_throw(s, ':', "expect ':'");
            jp.consumeWhiteSpace(s);
            if (sss::string_view(key) == name) {
                value = s;
                break;
            }
            jp.consumeValue(s);
            jp.consumeWhiteSpace(s);
        }
        if (value.empty()) {
            jp.consume_or_throw(s, '}', "expect '}'");
        }
    }
    return value;
}

sss::string_view query_array_index(sss::string_view s, int index) // 负数，表示反向处理。暂时只支持正数以及0；
{
    sss::string_view value;
    sss::json::Parser jp;
    jp.consumeWhiteSpace(value);

    if (!s.empty() && jp.peekType(s) == sss::json::Parser::kJE_ARRAY_START) {
        jp.consume_or_throw(s, '[', "expect '['");
        jp.consumeWhiteSpace(s);
        bool is_first = true;
        while (!s.empty() && s.front() != ']') {
            if (is_first) {
                is_first = false;
            }
            else {
                jp.consumeWhiteSpace(s);
                jp.consume_or_throw(s, ',', "expect ','");
            }
            jp.consumeWhiteSpace(s);
            if (index == 0) {
                value = s;
                break;
            }
            jp.consumeValue(s);
            jp.consumeWhiteSpace(s);
            --index;
        }
        if (value.empty()) {
            jp.consume_or_throw(s, ']', "expect ']'");
        }
    }
    return value;
}

} // namespace json
} // namespace sss
