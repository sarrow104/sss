#ifndef  __VHTMLPRINTER_HPP_1447306214__
#define  __VHTMLPRINTER_HPP_1447306214__

#include "xml_node.hpp"
#include "xml_doc.hpp"
#include "xml_visitor.hpp"

#include <sss/dom/html_util.hpp>

#include <iostream>

#ifndef __WIN32__

namespace sss{
    namespace xml3 {
        class xml_vhtmlprinter : public xml_visitor
        {
        public:
            xml_vhtmlprinter(std::ostream& o, const char * sep = "\t");

            virtual ~xml_vhtmlprinter();

        public:
            std::ostream& o()
            {
                return this->_o;
            }

        public:
            virtual void visit(xml3::node* );
            virtual void visit(xml3::node_info* );
            virtual void visit(xml3::node_text* );
            virtual void visit(xml3::node_cdata* );
            virtual void visit(xml3::node_doctype* );
            virtual void visit(xml3::node_comment* );
            virtual void visit(xml3::node_PI* );
            virtual void visit(xml3::xml_doc* );

        protected:
            static bool isSelfCloseTag(const std::string& tag);

        private:
            std::ostream& _o;
            html_util::indent_wraper ind;
        };
    }
}

#endif

#endif  /* __VHTMLPRINTER_HPP_1447306214__ */
