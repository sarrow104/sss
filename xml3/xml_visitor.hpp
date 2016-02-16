#ifndef  __XML_VISITOR_HPP_1441208791__
#define  __XML_VISITOR_HPP_1441208791__


#include "xml_node.hpp"
#include "xml_doc.hpp"

namespace sss{
    namespace xml3 {
        class xml_visitor
        {
        public:
            xml_visitor()
            {
            }

            virtual ~xml_visitor()
            {
            }

            void dispatch(xml3::node* );

        public:
            virtual void visit(xml3::node* ) = 0;
            virtual void visit(xml3::node_info* ) = 0;
            virtual void visit(xml3::node_text* ) = 0;
            virtual void visit(xml3::node_cdata* ) = 0;
            virtual void visit(xml3::node_doctype* ) = 0;
            virtual void visit(xml3::node_comment* ) = 0;
            virtual void visit(xml3::node_PI* ) = 0;
            virtual void visit(xml3::xml_doc* ) = 0;
        };
    }
}


#endif  /* __XML_VISITOR_HPP_1441208791__ */
