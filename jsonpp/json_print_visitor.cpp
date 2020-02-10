
#include <stdexcept>

#include <sss/util/PostionThrow.hpp>

#include "json_print_visitor.hpp"

namespace json {

void json_print_visitor::Indent::print(std::ostream& o) const {
    if (m_enable) {
        for (int i = 0; i < m_indent; ++i) {
            o << "\t";
        }
    }
}

void json_print_visitor::operator()(const null& ) const
{
    m_o << "null";
}

void json_print_visitor::operator()(bool v ) const
{
    m_o << (v ? "true" : "false");
}
void json_print_visitor::operator()(int64_t v) const
{
    m_o << v;          
}

void json_print_visitor::operator()(double v) const
{
    m_o << v;
}

void json_print_visitor::operator()(const text& v) const
{
    m_o << sss::raw_string(v);
}

void json_print_visitor::operator()(const array& s) const
{
    m_o << '[';
    if (!s.empty()) {
        Indent inner(m_indent);
        IndentHelper ind(inner);
        bool is_first = true;
        for (auto it = s.begin(); it != s.end(); ++it) {
            if (is_first) {
                is_first = false;
            }
            else {
                m_o << ",";
            }
            m_o << m_indent.endl() << inner;
            boost::apply_visitor(json_print_visitor(m_o, inner), *it);
        }
        m_o << m_indent.endl() << m_indent;
    }
    m_o << ']';
}

void json_print_visitor::operator()(const object& s) const
{
    m_o << '{';
    if (!s.empty()) {
        bool is_first = true;
        Indent inner(m_indent);
        IndentHelper ind(inner);
        for (auto it = s.begin(); it != s.end(); ++it) {
            if (is_first) {
                is_first = false;
            }
            else {
                m_o << ",";
            }
            m_o << m_indent.endl() << inner;
            m_o << sss::raw_string(it->first) << ":";
            if (m_indent.enable()) {
                m_o <<" ";
            }
            boost::apply_visitor(json_print_visitor(m_o, inner), it->second);
        }
        m_o << m_indent.endl() << m_indent;
    }
    m_o << '}';
}

} // namespace varlisp
