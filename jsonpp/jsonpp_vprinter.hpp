#ifndef  __JSONPP_VPRINTER_HPP_1437020123__
#define  __JSONPP_VPRINTER_HPP_1437020123__

#include "jsonpp.hpp"

#include <stdexcept>

#include <iostream>

namespace sss {
namespace jsonpp
{
    class JVPrinter : public JVisitor
    {
    public:
        JVPrinter(std::ostream& os):
            p_os(&os)
        {
        }
        ~JVPrinter()
        {
        }

    public:
        virtual void visit(const JArray * pj)
        {
            bool is_first = true;
            (*p_os) << "[";
            for (JArray::data_t::const_iterator it = pj->get_data().begin();
                 it != pj->get_data().end();
                 ++it)
            {
                if (!is_first) {
                    (*p_os) << ", ";
                }
                (*it)->accept(*this);
                is_first = false;
            }
            (*p_os) << "]";
        }

        virtual void visit(const JObject * pj)
        {
            bool is_first = true;
            (*p_os) << "{";
            for (JObject::data_t::const_iterator it = pj->get_data().begin();
                 it != pj->get_data().end();
                 ++it)
            {
                if (!is_first) {
                    (*p_os) << ", ";
                }
                (*p_os) << '"' << it->first << "\": ";
                it->second->accept(*this);
                is_first = false;
            }
            (*p_os) << "}";
        }

        virtual void visit(const JBool * pj)
        {
            (*p_os) << pj->to_str();
        }
        virtual void visit(const JDouble * pj)
        {
            (*p_os) << pj->data;
        }
        virtual void visit(const JInt * pj)
        {
            (*p_os) << pj->data;
        }
        virtual void visit(const JNull * pj)
        {
            (*p_os) << pj->to_str();
        }

        virtual void visit(const JString * pj)
        {
            (*p_os) << '\"' << pj->get_string() << '\"';
        }

    private:
        std::ostream * p_os;
    };
}
}

#endif  /* __JSONPP_VPRINTER_HPP_1437020123__ */


