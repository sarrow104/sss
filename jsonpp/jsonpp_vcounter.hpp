#ifndef  __JSONPP_VCOUNTER_HPP_1437031319__
#define  __JSONPP_VCOUNTER_HPP_1437031319__

#include "jsonpp.hpp"

#include <stdexcept>

#include <iostream>

namespace sss{
namespace jsonpp
{
    class JVCounter : public JVisitor
    {
    public:
        JVCounter():
            cnt(0)
        {
        }
        ~JVCounter()
        {
        }

    public:
        virtual void visit(const JArray * pj)
        {
            for (JArray::data_t::const_iterator it = pj->data.begin();
                 it != pj->data.end();
                 ++it)
            {
                (*it)->accept(*this);
            }
        }

        virtual void visit(const JObject * pj)
        {
            for (JObject::data_t::const_iterator it = pj->data.begin();
                 it != pj->data.end();
                 ++it)
            {
                it->second->accept(*this);
            }
        }

        virtual void visit(const JBool * )
        {
            cnt++;
        }
        virtual void visit(const JDouble * )
        {
            cnt++;
        }
        virtual void visit(const JInt * )
        {
            cnt++;
        }
        virtual void visit(const JNull * )
        {
            cnt++;
        }

        virtual void visit(const JString * )
        {
            cnt++;
        }

    public:
        inline int size() const
        {
            return cnt;
        }

        inline void clear()
        {
            this->cnt = 0;
        }

    private:
        int cnt;
    };
}
}


#endif  /* __JSONPP_VCOUNTER_HPP_1437031319__ */
