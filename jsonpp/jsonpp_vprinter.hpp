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
        JVPrinter(std::ostream& os,
                  bool is_pretty = false):
            p_os(&os), m_is_pretty(is_pretty), m_depth(0)
        {
        }

        ~JVPrinter()
        {
        }

    public:
        virtual void visit(const JArray * pj);

        virtual void visit(const JObject * pj);

        virtual void visit(const JBool * pj);
        
        virtual void visit(const JDouble * pj);
        
        virtual void visit(const JInt * pj);
        
        virtual void visit(const JNull * pj);

        virtual void visit(const JString * pj);

    protected:
        std::ostream& o() const {
            return *p_os;
        }

    private:
        std::ostream *  p_os;
        bool            m_is_pretty;
        int             m_depth;
    };
}
}

#endif  /* __JSONPP_VPRINTER_HPP_1437020123__ */


