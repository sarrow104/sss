#ifndef __WIN32__

#include "xml_handle.hpp"

#include <assert.h>

#include <gq/Document.h>
#include <gq/Node.h>
#include <gq/DocType.h>
#include <gq/QueryUtil.h>

#include <sss/util/Memory.hpp>
#include <sss/utlstring.hpp>
#include <sss/encoding.hpp>
#include <sss/path.hpp>

namespace {
    void appendRecursively(sss::xml3::node * p, CNode n)
    {
        assert(p);
        sss::xml3::xml_doc * pdoc = p->get_doc();

        switch (n.nodeType()) {
        case GUMBO_NODE_TEXT:
            {
                bool is_raw_data = false;
                switch (gumbo_tag_enum(n.parent().tag().c_str())) {
                case GUMBO_TAG_SCRIPT:
                    is_raw_data = true;
                    break;

                default:
                    break;
                }
                p->append_child(pdoc->create_text(n.textGumbo(), is_raw_data));
            }
            break;

        case GUMBO_NODE_CDATA:
            p->append_child(pdoc->create_cdata(n.textGumbo()));
            break;

        case GUMBO_NODE_ELEMENT:
            {
                sss::scoped_ptr<sss::xml3::node> tmp(pdoc->create_node(n.tag()));
                for (size_t i = 0; i != n.attrNum(); ++i) {
                    tmp->set(n.attrNameAt(i), n.attrValueAt(i));
                }

                for (size_t i = 0; i != n.childNum(); ++i) {
                    appendRecursively(tmp.get(), n.childAt(i));
                }
                p->append_child(tmp.release());
            }
            break;

        case GUMBO_NODE_COMMENT:
            p->append_child(pdoc->create_comment(n.textGumbo()));
            break;

        case GUMBO_NODE_DOCUMENT:
            {
                std::ostringstream oss;
                sss::xml3::xml_doc * pdoc = dynamic_cast<sss::xml3::xml_doc *>(p);
                oss << CDocType(n);
                pdoc->append_child(pdoc->create_doctype(oss.str()));
            }
            for (size_t i = 0; i != n.childNum(); ++i) {
                appendRecursively(p, n.childAt(i));
            }
            break;

        case GUMBO_NODE_TEMPLATE:
            std::cout << "GUMBO_NODE_TEMPLATE: not implement" << std::endl;
            break;

        case GUMBO_NODE_WHITESPACE:
            // std::cout << "GUMBO_NODE_WHITESPACE: ignored" << std::endl;
            break;
        }
    }
}

namespace sss{
    namespace xml3 {
        DocHandle& DocHandle::loadFromGumbo(const std::string& fname)
        {
            std::string path = sss::path::full_of_copy(fname);
            if (!sss::path::file_exists(path)) {
                return *this;
            }

            std::string content;
            sss::path::file2string(path, content);
            sss::Encoding::ensure(content, "utf8");

            CDocument doc;
            doc.parse(content);

            DocHandle tmp(new sss::xml3::xml_doc);

            appendRecursively(tmp.get(), doc.document());

            // tmp->print(std::cout);
            tmp.swap(*this);

            return *this;
        }
    }
}

#endif
