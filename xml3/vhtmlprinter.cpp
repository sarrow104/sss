#include "vhtmlprinter.hpp"

#ifndef __WIN32__

#include <gumbo.h>

namespace sss{
    namespace xml3 {
        xml_vhtmlprinter::xml_vhtmlprinter(std::ostream& o, const char * sep)
            : _o(o), ind(sep)
        {
        }

        xml_vhtmlprinter::~xml_vhtmlprinter()
        {
        }

        void xml_vhtmlprinter::visit(xml3::node* n)
        {
            if (!n->is_type(sss::xml3::type_node)) {
                return this->dispatch(n);
            }

            bool is_self_close = xml_vhtmlprinter::isSelfCloseTag(n->get_data());

            // NOTE 如果内部只有一个节点（不管是text,comment,还是 node），都不额外输出回车符。
            bool is_self_node = !n->hasChildren();

            // 2015-11-08
            // 我原来的设计是，如果某节点内部有且只有一个节点，那么
            // <outer><inner/></outer>
            // 这样，不换行进行输出；
            // 此时，会遇到这种情况：
            // <body><div><div>...</div></div></body>
            // 这样就比较难看了。
            // 我觉得应该这样——只有子节点是叶子节点，才省略回车和缩进！
            if (!is_self_node || !is_self_close) {
                bool is_neat_print = (n->size() == 1 && n->firstChild()->is_leaf());

                this->o() << ind.get() << "<" << n->get_data() << n->properties << ">";
                {
                    html_util::indent_auto ind_auto(ind);
                    if (!is_neat_print && n->size()) {
                        this->o() << std::endl;
                    }
                    for (node * it = n->firstChild(); it; it = it->nextSibling()) {
                        if (is_neat_print) {
                            sss::xml3::xml_vhtmlprinter jv(this->o());
                            it->accept(jv);
                        }
                        else {
                            it->accept(*this);
                            if (!is_neat_print) {
                                this->o() << std::endl;
                            }
                        }
                    }
                }
                if (!is_neat_print && n->size()) {
                    this->o() << ind.get();
                }
                this->o() << "</" << n->get_data() << ">";
            }
            else {
                this->o() << ind.get() << "<" << n->get_data() << n->properties << "/>";
            }
        }

        void xml_vhtmlprinter::visit(xml3::node_info* n)
        {
            // <?xml version="1.0" encoding="utf-8" ?>
            this->o() << ind.get()
                << "<?" << n->get_data() << n->properties << "?>";
        }

        void xml_vhtmlprinter::visit(xml3::node_text* n)
        {
            if (n->parent() && n->parent()->size() >= 2) {
                this->o() << ind.get();
            }
            this->o() << n->get_data();
        }

        void xml_vhtmlprinter::visit(xml3::node_cdata* n)
        {
            this->o() << ind.get();
            this->o() << "<![CDATA[" << n->get_data() << "]]>";
        }

        void xml_vhtmlprinter::visit(xml3::node_doctype* n)
        {
            this->o() << ind.get() << n->get_data();
        }

        void xml_vhtmlprinter::visit(xml3::node_comment* n)
        {
            this->o() << ind.get();
            this->o() << "<!--" << n->get_data() << "-->";
        }

        void xml_vhtmlprinter::visit(xml3::node_PI* n)
        {
            this->o() << ind.get()
                << "<?" << n->get_data() << "?>" << std::endl;
        }

        void xml_vhtmlprinter::visit(xml3::xml_doc* n)
        {
            for (node * i = n->firstChild(); i; i = i->nextSibling()) {
                if (i != n->firstChild()) {
                    this->o() << std::endl;
                }
                i->accept(*this);
            }
        }

        bool xml_vhtmlprinter::isSelfCloseTag(const std::string& tag)
        {
            switch (gumbo_tag_enum(tag.c_str())) {
            case GUMBO_TAG_AREA:
            case GUMBO_TAG_BASE:
            case GUMBO_TAG_BR:
            case GUMBO_TAG_COL:
            case GUMBO_TAG_EMBED:
            case GUMBO_TAG_HR:
            case GUMBO_TAG_IMG:
            case GUMBO_TAG_INPUT:
            case GUMBO_TAG_KEYGEN:
            case GUMBO_TAG_LINK:
            case GUMBO_TAG_META:
            case GUMBO_TAG_PARAM:
            case GUMBO_TAG_SOURCE:
            case GUMBO_TAG_TRACK:
            case GUMBO_TAG_WBR:
                return true;

            default:
                return false;
            }
        }
    }
}

#endif
