#include "jsonpp_vprinter.hpp"
namespace sss {
namespace jsonpp
{
    void JVPrinter::visit(const JArray * pj)
    {
        bool is_first = true;
        if (m_is_pretty) {
            o() << std::string(m_depth++, '\t');
        }
        o() << "[";
        if (pj->size()) {
            if (m_is_pretty) {
                o() << '\n';
            }
            for (JArray::data_t::const_iterator it = pj->get_data().begin();
                 it != pj->get_data().end();
                 ++it)
            {
                if (!is_first) {
                    o() << ",";
                    if (m_is_pretty) {
                        o() << '\n';
                    }
                    else {
                        o() << ' ';
                    }
                }
                (*it)->accept(*this);
                is_first = false;
            }
            if (m_is_pretty) {
                o() << '\n' << std::string(--m_depth, '\t');
            }
        }
        else {
            if (m_is_pretty) {
                --m_depth;
            }
        }
        o() << "]";
    }

    void JVPrinter::visit(const JObject * pj)
    {
        bool is_first = true;
        if (m_is_pretty) {
            o() << std::string(m_depth++, '\t');
        }
        o() << "{";

        if (pj->size()) {
            if (m_is_pretty) {
                o() << '\n';
            }
            for (JObject::data_t::const_iterator it = pj->get_data().begin();
                 it != pj->get_data().end();
                 ++it)
            {
                if (!is_first) {
                    o() << ",";
                    if (m_is_pretty) {
                        o() << '\n';
                    }
                    else {
                        o() << ' ';
                    }
                }
                if (m_is_pretty) {
                    o() << std::string(m_depth, '\t');
                }
                o() << '"' << it->first << "\":";
                if (m_is_pretty) {
                    o() << '\n';
                }
                else {
                    o() << ' ';
                }

                if (m_is_pretty) {
                    m_depth++;
                }
                it->second->accept(*this);
                if (m_is_pretty) {
                    m_depth--;
                }
                is_first = false;
            }
            if (m_is_pretty) {
                o() << '\n' << std::string(--m_depth, '\t');
            }
        }
        else {
            if (m_is_pretty) {
                --m_depth;
            }
        }
        o() << "}";
    }

    void JVPrinter::visit(const JBool * pj)
    {
        if (m_is_pretty) {
            o() << std::string(m_depth, '\t');
        }
        o() << pj->to_str();
    }

    void JVPrinter::visit(const JDouble * pj)
    {
        if (m_is_pretty) {
            o() << std::string(m_depth, '\t');
        }
        o() << pj->data;
    }

    void JVPrinter::visit(const JInt * pj)
    {
        if (m_is_pretty) {
            o() << std::string(m_depth, '\t');
        }
        o() << pj->data;
    }

    void JVPrinter::visit(const JNull * pj)
    {
        if (m_is_pretty) {
            o() << std::string(m_depth, '\t');
        }
        o() << pj->to_str();
    }

    void JVPrinter::visit(const JString * pj)
    {
        if (m_is_pretty) {
            o() << std::string(m_depth, '\t');
        }
        o() << '\"' << pj->get_string() << '\"';
    }
}
}
