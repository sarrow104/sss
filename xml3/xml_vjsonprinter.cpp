#include "xml_vjsonprinter.hpp"

#include <sss/log.hpp>

namespace sss{
    namespace xml3 {
        xml_vjsonprinter::xml_vjsonprinter(std::ostream& o)
            : _o(o), _is_first(true)
        {
        }

        xml_vjsonprinter::~xml_vjsonprinter()
        {
        }

        // 如何比较树形结构，节点的大小？
        // 同父亲，以sibling先后判断大小；
        // 不同父亲，以同级别的祖先的判断大小；
        // 直系祖先关系，祖辈为大；
        //
        // 那么，不是一个文档，或者，其中一个是游离状态，如何比较大小（即，祖先
        // 节点不等于document指针）
        bool xml_vjsonprinter::is_same_Children(xml3::node* n ) const
        {
            //SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
            if (n && n->is_type(sss::xml3::type_node) && n->size() > 1u) {
                std::string tagName;
                for (sss::xml3::node * sub = n->firstChild();
                     sub;
                     sub = sub->nextSibling())
                {
                    switch (sub->type()) {
                    case sss::xml3::type_node:
                        if (tagName.empty()) {
                            tagName = sub->get_data();
                        }
                        else {
                            if (tagName != sub->get_data()) {
                                return false;
                            }
                        }
                        break;
                    case sss::xml3::type_text: return false; break;
                    case sss::xml3::type_info: return false; break;
                    case sss::xml3::type_doctype: return false; break;
                    case sss::xml3::type_comment: continue; break;
                    case sss::xml3::type_cdata: return false; break;
                    case sss::xml3::type_xmldoc: return false; break;
                    case sss::xml3::type_xhtmldoc: return false; break;
                    case sss::xml3::type_PI: return false; break;
                    default: throw std::runtime_error("unknown xml-node type.");
                    }
                }
                return true;
            }
            return false;
        }

//        { tagName: {} }
//        或者
//        { tagName: [] }
//          NOTE
//            其实，很大程度上，json的数组，仅仅是[1,2,3]，这样的
//            字符、数字；而这对于xml，恰恰是没法表达的东西……
//        或者
//        { tagName: 1.2e3 }
//        或者
//        { tagName: "" }
//
// 此时，属性值，就表达为 "@p1": "v1", "@p2: "v2" ... 即，与表签名同级别的其他属性
// ；
//
// 当然 xml_prolog 这些就会被忽略了。

        void xml_vjsonprinter::visit(xml3::node* n)
        {
            if (!n->is_type(sss::xml3::type_node)) {
                this->dispatch(n);
                return;
            }
            if (!_is_first) {
                this->o() << ", " << std::endl;
            }
            this->o() << "{\"" << n->get_data() << "\": ";
            this->_is_first = true;
            if (is_same_Children(n)) {
                this->o() << '[';
                for (node * it = n->firstChild(); it; it = it->nextSibling()) {
                    it->accept(*this);
                }
                this->o() << ']';
            }
            else if (n->hasChildren())
            {
                for (node * it = n->firstChild(); it; it = it->nextSibling()) {
                    it->accept(*this);
                }
            }
            else {
                this->o() << "\"\"";
            }
            for (sss::xml3::properties_t::const_iterator it = n->properties.begin();
                 it != n->properties.end();
                 ++it)
            {
                this->o() << ", \"@" << it->first << "\": \"" << it->second << "\"";
            }
            this->o() << "}";
            this->_is_first = false;
        }

        // ignore
        void xml_vjsonprinter::visit(xml3::node_info* )
        {
        }

        void xml_vjsonprinter::visit(xml3::node_text* n)
        {
            this->o() << '"' << n->get_data() << '"';
        }

        // ignore ?
        void xml_vjsonprinter::visit(xml3::node_cdata* )
        {
        }

        // ignore
        void xml_vjsonprinter::visit(xml3::node_doctype* )
        {
        }

        // ignore
        void xml_vjsonprinter::visit(xml3::node_comment* )
        {
        }

        // ignore
        void xml_vjsonprinter::visit(xml3::node_PI* )
        {
        }

        void xml_vjsonprinter::visit(xml3::xml_doc* n)
        {
            n->root()->accept(*this);
        }
    }
}
