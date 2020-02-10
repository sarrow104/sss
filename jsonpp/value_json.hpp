#pragma once

#include <iostream>

#include <map>

#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <sss/string_view.hpp>

namespace json {
struct null {
    bool operator==(null) const { return true; }
};

inline static std::ostream& operator<<(std::ostream& os, null) { return os << "null"; }

using text   = std::string;
using value  = boost::make_recursive_variant<
    null,
    text,                                      // "string" (roughly!)
    int64_t,                                   // integer
    double,                                    // number
    std::map<text, boost::recursive_variant_>, // object
    std::vector<boost::recursive_variant_>,    // array
    bool
>::type;
using object = std::map<text, value>;
using array  = std::vector<value>;

// NOTE FIXME
// 感觉上，自定义的ParserHandle，应该继承，或者包含sss::json::Parser。
// 而不是Reader来套。（或者，继承Reader）
// 因为，sax风格的解析器，用户定义的handle，本身，也需要去保存当前状态。(虽然这个状态，与业务很接近)
// 但是，这样使用起来很繁琐。
// 而本质上，用户可能更需要的递归下降的sax。
// 对于用户的具体需求来说，他其实，只想定义几个函数而已。
// 因为，用户需要的结构，其实很简单。往往只需要两三层。
// 当然，我们内部的Reader，逻辑上是需要支持无限层次的。
//
// 而且，当前这个基于boost::recursive_variant_的通用结构，获取数据来，也不方便。
// 远没有利用C++11(14)技术，自定义的树形结构方便。而且，貌似也不节省内存。
//
// 当然，如果采用类似ajson的解析模式，其实也是可以的——起内部，是复用传入的json字符串。
// ——会对原有的字符串字节，做修改。
// 当前的模式，parser是各种consume动作；reader本质上，是一个大循环，解析字符串，然后按照json
// 语法，利用状态，以及树（对象还是数组）粘，来判断字符串合法性。
// 如果要套成递归下降，那么就需要改造这个reader的循环。
// 将其拆分成fetch()动作。同时，标记当前reader的状态（expect啥），读到的是是啥。
struct value_handle_t
{
    // 如何表示路径？
    // {}属性，名字；
    // []用数字；
    // 还在于，类型不同，如何保存？
    enum state_t {
        kNone,
        kExpectKeyOrObjectEnd,  // {
        kExpectValue,           // "config"
        kExpectElementOrArrayEnd
    };
    state_t m_state;
    bool m_parse_elment;
    std::vector<value*> m_path;
    std::vector<text> m_keys;
    value m_v;
    value * p_v;
    text   m_key;

    explicit value_handle_t(bool parse_element = false)
        : m_state(kNone),
          m_parse_elment(parse_element)
    {
        p_v = &m_v;
    }

    bool Null();

    bool Bool(bool b);

    bool Int64(int64_t i);

    bool Double(double d);

    bool String(sss::string_view s);

    bool StartObject();

    bool Key(sss::string_view s);

    bool EndObject(int /*memberCount*/);

    bool StartArray();

    bool EndArray(int /*memberCount*/);
};

} // namespace json

