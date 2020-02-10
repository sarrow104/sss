#pragma once

#include <boost/variant.hpp>

#include <sss/raw_print.hpp>

#include "value_json.hpp"

namespace json {

struct json_print_visitor : public boost::static_visitor<void> {
    struct IndentHelper;
    struct Indent
    {
        friend struct IndentHelper;
        struct endl_t {
            friend std::ostream& operator << (std::ostream&, const endl_t& i);
            bool m_enable;
            explicit endl_t(bool enable)
                : m_enable(enable)
            {
            }
            void print(std::ostream& o) const
            {
                if (m_enable) {
                    o << "\n";
                }
            }
        };
        friend std::ostream& operator << (std::ostream&, const Indent& i);
        explicit Indent(bool enable)
            : m_enable(enable), m_indent(0)
        {}
        void print(std::ostream& o) const;
        bool enable() const {
            return m_enable;
        }
        endl_t endl() const {
            return endl_t(m_enable);
        }
    private:
        bool  m_enable;
        int   m_indent;
    };
    struct IndentHelper
    {
        Indent & m_indent;
        IndentHelper(Indent& indent)
            : m_indent(indent)
        {
            if (m_indent.enable()) {
                ++m_indent.m_indent;
            }
        }
        ~IndentHelper()
        {
            if (m_indent.enable()) {
                --m_indent.m_indent;
            }
        }
    };

    std::ostream& m_o;
    Indent m_indent;
    json_print_visitor(std::ostream& o, bool is_indent = false)
        : m_o(o), m_indent(is_indent)
    {
    }
    json_print_visitor(std::ostream& o, const Indent& indent)
        : m_o(o), m_indent(indent)
    {
    }
    template <typename T>
    void operator()(const T& v) const
    {
        std::ostringstream oss;
        oss << v;
        m_o << sss::raw_string(oss.str());
    }

    void operator( ) (const null&     ) const ;
    void operator( ) (bool v          ) const ;
    void operator( ) (int64_t v       ) const ;
    void operator( ) (double v        ) const ;
    void operator( ) (const text& v   ) const ;
    void operator( ) (const array& s  ) const ;
    void operator( ) (const object& s ) const ;
};

inline std::ostream& operator << (std::ostream& o, const json_print_visitor::Indent& i)
{
    i.print(o);
    return o;
}

inline std::ostream& operator << (std::ostream& o, const json_print_visitor::Indent::endl_t& e)
{
    e.print(o);
    return o;
}

struct json_fmter_t
{
    explicit json_fmter_t(const json::value& v, bool pretty = false):
        j(v), pretty(pretty)
    {}
    const json::value& j;
    bool  pretty;
    void print(std::ostream& o) const {
        boost::apply_visitor(json::json_print_visitor(o, pretty), j);
    }
};

inline json_fmter_t json_fmter(const json::value& v, bool pretty = false)
{
    return json_fmter_t{v, pretty};
}

inline std::ostream& operator<<(std::ostream& o, const json_fmter_t& jf)
{
    jf.print(o);
    return o;
}


} // namespace varlisp
