#pragma once

#include <sss/jsonpp/parser.hpp>

#include <sss/string_view.hpp>
#include <sss/colorlog.hpp>
#include <sss/debug/value_msg.hpp>

// -std=c++14 201402L
#if __cplusplus == 201103L
namespace std {
template< bool B, class T = void >
    using enable_if_t = typename std::enable_if<B,T>::type;
} // namespace std
#endif

namespace sss {
namespace json {

// json-sax-query
// 
// 不创建dom，而是通过组合query_key, query_index 这两个函数，反复使用（流式），
// 然后获取到原有json字符串的一个refer（string_view）
// 
// 然后，当需要的时候——判断所需要值的类型，通过一个字符——然后用具体的函数，
// 将信息，抽取、转化出来。
// 
// 就可以达到最快的速度了。

// 支持链式处理——要点就是不会更改传入数据
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

template<typename RetValue>
std::enable_if_t<std::is_same<bool, RetValue>::value, RetValue>
query_converter(sss::json::Parser& jp, sss::string_view s)
{
    if (s.empty()) {
        throw "empty";
    }
    switch (jp.peekType(s)) {
        case sss::json::Parser::kJE_TRUE:
            if (jp.consumeTrue(s)) {
                return true;
            }
            break;

        case sss::json::Parser::kJE_FALSE:
            if (jp.consumeFalse(s)) {
                return false;
            }
            break;
        default:
            throw s[0];
    }
    throw "parse error";
}

template<typename RetValue>
std::enable_if_t<std::is_arithmetic<RetValue>::value, RetValue>
query_converter(sss::json::Parser& jp, sss::string_view s)
{
    if (s.empty()) {
        throw "empty";
    }
    std::tuple<double, int64_t> number;
    int index;
    switch (jp.peekType(s)) {
        case sss::json::Parser::kJE_NUMBER:
            if (jp.consumeNumber(s, number, index)) {
                if (index == 0) {
                    return RetValue(std::get<0>(number));
                }
                else {
                    return RetValue(std::get<1>(number));
                }
            }
            break;

        default:
            throw s[0];
    }
    throw "parse error";
}

template<typename RetValue>
std::enable_if_t<std::is_same<std::string, RetValue>::value, RetValue>
query_converter(sss::json::Parser& jp, sss::string_view s)
{
    if (s.empty()) {
        throw "empty";
    }
    std::string value;
    switch (jp.peekType(s)) {
        case sss::json::Parser::kJE_STRING:
            if (jp.consumeString(s, value)) {
                return value;
            }
            break;

        default:
            throw s[0];
    }
    throw "parse error";
}

struct Null_t
{
    
};
inline ::std::ostream& operator << (::std::ostream& o, const Null_t&)
{
    o.write("null", 4);
    return o;
}

template<typename RetValue>
std::enable_if_t<std::is_same<Null_t, RetValue>::value, RetValue>
query_converter(sss::json::Parser& jp, sss::string_view s)
{
    if (s.empty()) {
        throw "empty";
    }
    switch (jp.peekType(s)) {
        case sss::json::Parser::kJE_NULL:
            if (jp.consumeNull(s)) {
                return Null_t{};
            }
            break;

        default:
            throw s[0];
    }
    throw "parse error";
}

template<typename Id>
std::enable_if_t<std::is_same<sss::string_view, Id>::value, sss::string_view>
 query_wrapper(sss::string_view s, Id id)
{
    return sss::json::query_object_name(s, id);
}

template<typename Id>
std::enable_if_t<std::is_same<int, Id>::value, sss::string_view>
query_wrapper(sss::string_view s, Id id)
{
    return sss::json::query_array_index(s, id);
}

template<typename... ArgsT>
sss::string_view query_wrapper(sss::string_view s, sss::string_view key, ArgsT... args)
{
    return query_wrapper(sss::json::query_object_name(s, key), args...);
}

template<typename... ArgsT>
sss::string_view query_wrapper(sss::string_view s, int index, ArgsT... args)
{
    return query_wrapper(sss::json::query_array_index(s, index), args...);
}

template<typename RetValue, typename... ArgsT>
RetValue query(sss::string_view s, ArgsT... args)
{
    sss::json::Parser jp;
    return query_converter<RetValue>(jp, query_wrapper(s, args...));
}

// void test_query(sss::string_view s)
// {
//     // std::vector<std::function<sss::string_view(sss::string_view)> > query_list = {
//     //     std::bind(sss::json::query_object_name, std::placeholders::_1, "hello"),
//     //     std::bind(sss::json::query_array_index, std::placeholders::_1, 2)
//     // };
// 
//     // sss::string_view value = s;
//     // for (auto& f : query_list) {
//     //     value = f(value);
//     // }
//     // COLOG_ERROR(SSS_VALUE_MSG(value));
//     // std::cout << sss::raw_string(value) << std::endl;
// 
//     // std::cout << query<int>(s, 1, "hello", 0) << std::endl;
//     std::cout << query<int>(s, sss::string_view("hello"), 0) << std::endl;
//     std::cout << query<int>(s, sss::string_view("hello"), 1) << std::endl;
//     std::cout << query<int>(s, sss::string_view("hello"), 2) << std::endl;
//     std::cout << query<Null_t>(s, sss::string_view("hello"), 3) << std::endl;
// }

} // namespace json
} // namespace sss
