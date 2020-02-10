#include "value_json.hpp"

#include <sss/colorlog.hpp>

namespace json {

bool value_handle_t::Null()
{
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    if (m_state == kExpectValue && !m_key.empty()) {
        boost::get<object>(p_v)->emplace(m_key, null{});
        m_key.clear();
        m_state = kExpectKeyOrObjectEnd;
    }
    else if (m_state == kExpectElementOrArrayEnd) {
        boost::get<array>(p_v)->emplace_back(null{});
    }
    else if (m_parse_elment && p_v) {
        *p_v = null{};
    }
    else {
        throw __PRETTY_FUNCTION__;
    }
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    return true;
}

bool value_handle_t::Bool(bool b)
{
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    if (m_state == kExpectValue && !m_key.empty()) {
        boost::get<object>(p_v)->emplace(m_key, b);
        m_key.clear();
        m_state = kExpectKeyOrObjectEnd;
    }
    else if (m_state == kExpectElementOrArrayEnd) {
        boost::get<array>(p_v)->emplace_back(b);
    }
    else if (m_parse_elment && p_v) {
        *p_v = b;
    }
    else {
        throw __PRETTY_FUNCTION__;
    }
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    return true;
}

bool value_handle_t::Int64(int64_t i)
{
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    if (m_state == kExpectValue && !m_key.empty()) {
        boost::get<object>(p_v)->emplace(m_key, i);
        m_key.clear();
        m_state = kExpectKeyOrObjectEnd;
    }
    else if (m_state == kExpectElementOrArrayEnd) {
        boost::get<array>(p_v)->emplace_back(i);
    }
    else if (m_parse_elment && p_v) {
        *p_v = i;
    }
    else {
        throw __PRETTY_FUNCTION__;
    }

    COLOG_DEBUG(__PRETTY_FUNCTION__);
    return true;
}

bool value_handle_t::Double(double d)
{
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    if (m_state == kExpectValue && !m_key.empty()) {
        boost::get<object>(p_v)->emplace(m_key, d);
        m_key.clear();
        m_state = kExpectKeyOrObjectEnd;
    }
    else if (m_state == kExpectElementOrArrayEnd) {
        boost::get<array>(p_v)->emplace_back(d);
    }
    else if (m_parse_elment && p_v) {
        *p_v = d;
    }
    else {
        throw __PRETTY_FUNCTION__;
    }
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    return true;
}

bool value_handle_t::String(sss::string_view s)
{
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    if (m_state == kExpectValue && !m_key.empty()) {
        boost::get<object>(p_v)->emplace(m_key, s.to_string());
        m_key.clear();
        m_state = kExpectKeyOrObjectEnd;
    }
    else if (m_state == kExpectElementOrArrayEnd) {
        boost::get<array>(p_v)->emplace_back(s.to_string());
    }
    else if (m_parse_elment && p_v) {
        *p_v = s.to_string();
    }
    else {
        throw __PRETTY_FUNCTION__;
    }
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    return true;
}

bool value_handle_t::StartObject()
{
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    if (m_state == kNone) {
        m_state = kExpectKeyOrObjectEnd;
        *p_v = object{};
        m_path.push_back(p_v);
    }
    else if (m_state == kExpectValue && !m_key.empty()) {
        boost::get<object>(p_v)->emplace(m_key, object{});
        p_v = &boost::get<object>(p_v)->at(m_key);
        m_key.clear();
        m_state = kExpectKeyOrObjectEnd;
    }
    else if (m_state == kExpectElementOrArrayEnd) {
        boost::get<array>(p_v)->emplace_back(object{});
        p_v = &boost::get<array>(p_v)->back();
        m_state = kExpectKeyOrObjectEnd;
    }
    else {
        throw __PRETTY_FUNCTION__;
    }
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    return true;
}

bool value_handle_t::Key(sss::string_view s)
{
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    if (m_state == kExpectKeyOrObjectEnd && m_key.empty()) {
        m_key = s.to_string();
        m_state = kExpectValue;
    }
    else {
        throw __PRETTY_FUNCTION__;
    }
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    return true;
}

bool value_handle_t::EndObject(int /*memberCount*/)
{
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    if (m_state == kExpectKeyOrObjectEnd && m_key.empty()) {
        if (m_path.empty()) {
            p_v = 0;
        }
        else {
            p_v = m_path.back();
            m_path.pop_back();
        }
    }
    else {
        throw __PRETTY_FUNCTION__;
    }

    if (p_v) {
        if (boost::get<object>(p_v)) {
            m_state = kExpectKeyOrObjectEnd;
        }
        else if (boost::get<array>(p_v)){
            m_state = kExpectElementOrArrayEnd;
        }
        else {
            throw __PRETTY_FUNCTION__;
        }
    }
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    return true;
}

bool value_handle_t::StartArray()
{
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    if (m_state == kNone) {
        m_state = kExpectElementOrArrayEnd;
        *p_v = array{};
        m_path.push_back(p_v);
    }
    else if (m_state == kExpectValue && !m_key.empty()) {
        boost::get<object>(p_v)->emplace(m_key, object{});
        p_v = &boost::get<object>(p_v)->at(m_key);
        m_path.push_back(p_v);
        m_key.clear();
        m_state = kExpectKeyOrObjectEnd;
    }
    else if (m_state == kExpectElementOrArrayEnd && m_key.empty()) {
        boost::get<array>(p_v)->emplace_back(object{});
        p_v = &boost::get<array>(p_v)->back();
        m_path.push_back(p_v);
        m_state = kExpectKeyOrObjectEnd;
    }
    else {
        throw __PRETTY_FUNCTION__;
    }
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    return true;
}

bool value_handle_t::EndArray(int /*memberCount*/)
{
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    if (m_state == kExpectElementOrArrayEnd && m_key.empty()) {
        if (m_path.empty()) {
            p_v = 0;
        }
        else {
            p_v = m_path.back();
            m_path.pop_back();
        }
    }
    else {
        throw __PRETTY_FUNCTION__;
    }

    if (p_v) {
        if (boost::get<object>(p_v)) {
            m_state = kExpectKeyOrObjectEnd;
        }
        else if (boost::get<array>(p_v)){
            m_state = kExpectElementOrArrayEnd;
        }
        else {
            throw __PRETTY_FUNCTION__;
        }
    }
    COLOG_DEBUG(__PRETTY_FUNCTION__);
    return true;
}

} // namespace json
