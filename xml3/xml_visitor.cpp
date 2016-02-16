#include "xml_visitor.hpp"

#include <stdexcept>
#include <sstream>

#include "xml_node.hpp"

namespace sss{
    namespace xml3 {
            void xml_visitor::dispatch(sss::xml3::node* n)
            {
                switch (n->type())
                {
                case sss::xml3::type_node: dynamic_cast<sss::xml3::node*>(n)->accept(*this); break;
                case sss::xml3::type_text: dynamic_cast<sss::xml3::node_text*>(n)->accept(*this); break;
                case sss::xml3::type_PI: dynamic_cast<sss::xml3::node_PI*>(n)->accept(*this); break;
                case sss::xml3::type_info: dynamic_cast<sss::xml3::node_info*>(n)->accept(*this); break;
                case sss::xml3::type_cdata: dynamic_cast<sss::xml3::node_cdata*>(n)->accept(*this); break;
                case sss::xml3::type_comment: dynamic_cast<sss::xml3::node_comment*>(n)->accept(*this); break;
                case sss::xml3::type_doctype: dynamic_cast<sss::xml3::node_doctype*>(n)->accept(*this); break;
                case sss::xml3::type_xmldoc: dynamic_cast<sss::xml3::xml_doc*>(n)->accept(*this); break;
                default:
                     {
                         std::ostringstream oss;
                         oss << __func__ << ": cannot handle " << n->type();
                         throw std::runtime_error(oss.str());
                     }
                }
            }
    }
}
