#include "xhtml_doc.hpp"

#include <sss/regex/simpleregex.hpp>

namespace sss {
    namespace xml {
        // 创建空的xml文档对象
        xhtml_doc::xhtml_doc()
        {
            this->node_type = type_xhtmldoc;
        }

        // 创建字符集为charset的xml文档对象
        // 同时设置是否打印bom――NOTE bom只在打印的时候有效。
        xhtml_doc::xhtml_doc(const std::string& charset,
                             bool has_bom)
            : xml_doc(charset, has_bom)
        {
            this->node_type = type_xhtmldoc;
        }

        // 创建根节点为root_name的xml文档对象
        // 其余同上；
        xhtml_doc::xhtml_doc(const std::string& root_name,
                             const std::string& charset,
                             bool has_bom)
            : xml_doc(root_name, charset, has_bom)
        {
            this->node_type = type_xhtmldoc;
        }

        // ……
        xhtml_doc::~xhtml_doc()
        {
        }

        // ……
        node * xhtml_doc::root() const
        {
            return this->xml_doc::root();
        }

        node * xhtml_doc::root(const std::string& name)
        {
            return this->xml_doc::root(name);
        }

        // 获取根节点名字
        std::string xhtml_doc::root_name() const
        {
            return this->xml_doc::root_name();
        }

        std::string xhtml_doc::set_charset(const std::string& charset)
        {
            //<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
            sss::xml::node * p_head = this->verify_path("html>head");

            node_list_t metas = p_head->find_child("meta");
            node_list_t charsets;
            for (node_list_t::iterator it = metas.begin();
                 it != metas.end();
                 ++it)
            {
                if ((*it)->get("http-equiv") == "Content-Type") {
                    charsets.push_back(*it);
                }
            }

            std::string old_charset;
            if (charsets.empty()) {
                node * p_meta = this->create_node("meta");
                p_meta->set("http-equiv",       "Content-Type");
                p_meta->set("content",          "text/html; charset=" + charset);
                p_head->insert_before(p_meta);
            }
            else {
                node * p_meta = charsets[0];
                sss::regex::simpleregex charset_reg("text/html;\\s*charset=\\(\\S+\\)$");
                if (charset_reg.match(p_meta->get("content"))) {
                    old_charset = charset_reg.get_submatch(1);
                    p_meta->set("content",          "text/html; charset=" + charset);
                }
            }
            return old_charset;
        }

        std::string xhtml_doc::get_charset()
        {
            sss::xml::node * p_meta = this->verify_path("html>head>meta");
            if (p_meta && p_meta->get("http-equiv") == "Content-Type") {
                sss::regex::simpleregex charset_reg("text/html;\\s*charset=\\(\\S+\\)$");
                if (charset_reg.match(p_meta->get("content"))) {
                    return charset_reg.get_submatch(1);
                }
            }
            return "";
        }

        bool        xhtml_doc::validate() const
        {
            return true;
        }

        // 确保描述charset的节点存在于根节点之前！
        node      * xhtml_doc::verify_charset_node()
        {
            return 0;
        }
    }
}

