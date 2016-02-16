#ifndef  __XML_VPRINTER_HPP_1441241318__
#define  __XML_VPRINTER_HPP_1441241318__

#include "xml_node.hpp"
#include "xml_doc.hpp"
#include "xml_visitor.hpp"

#include <sss/dom/html_util.hpp>

#include <iostream>

namespace sss{
    namespace xml3 {
        class xml_vprinter : public xml_visitor
        {
        public:
            // 构造函数，需要解决两个问题：
            // 1. 输出到哪里？
            // 2. 缩进是多少？
            //
            // 缩进，对于程序源代码的文本输出来说，是一个通用的、必须的组件，最
            // 好能够模块化！
            // 如何命名、安排位置呢？
            // formattor？indentor?
            // text/
            // utilstring/
            xml_vprinter(std::ostream& o, const char * sep = "\t");

            virtual ~xml_vprinter();

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

        private:
            std::ostream& _o;
            html_util::indent_wraper ind;
        };
    }
}

#endif  /* __XML_VPRINTER_HPP_1441241318__ */
