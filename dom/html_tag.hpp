#ifndef  __HTML_TAG_HPP_1390916796__
#define  __HTML_TAG_HPP_1390916796__

#include "html_util.hpp"

#include <typeinfo>

#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace sss {

namespace dom {
class html_doc;
}
}

namespace sss {
namespace html_tags {

    class html_tag { // {{{1
    public:
        html_tag    *parent;    // 指向父节点；
        int         html_tag_id;
        bool        is_single;  // 是否单标签（非成对）
        sss::dom::html_doc    *p_document;        // 文档对象指针；以根html_tag的该属性为准；
        std::string text;

        typedef std::map<std::string, std::string> propertys_t;
        propertys_t propertys;

    public:
        html_tag(int html_tag_id, bool is_single = false)
            : parent(0),
              html_tag_id(html_tag_id),
              is_single(is_single),
              p_document(0)
        {
        }
        // NOTE 默认的复制构造函数即可满足要求吗？
        html_tag(const html_tag& ref)
            : parent(0),
              html_tag_id(ref.html_tag_id),
              is_single(ref.is_single),
              p_document(0),
              propertys(ref.propertys)
        {
        }
        virtual ~html_tag() = 0;

    public:
        virtual html_tag * clone() const = 0;

        virtual void print_impl(std::ostream& o, html_util::indent_wraper& ind) const = 0;

        // 无需覆盖，但是不能调用--assert(false);
        virtual void add(html_tag*);

        virtual std::string begin_tag() const;

        virtual std::string end_tag() const;

        virtual html_tag& operator << (const html_tag& sub);

        virtual html_tag& operator () (const std::string& txt);

        virtual void add(const std::string& key, const std::string& css);

        typedef std::vector<html_tag*> subnodes_t;
        virtual subnodes_t *get_subnodes();

    public:
        // 返回根节点；
        // 如果自己就是根，则返回自己；
        html_tag *get_root_tag();

        html_tag *get_parent() const {
            return this->parent;
        }

        inline sss::dom::html_doc * get_htmldocument() {
            return this->get_root_tag()->p_document;
        }

        inline void       set_htmldocument(sss::dom::html_doc * p_html_document) {
            this->get_root_tag()->p_document = p_html_document;
        }

        static html_tag *create_tag(const std::string& tag_name);

        html_tag *set_parent(html_tag * father) {
            this->parent = father;
            return this->parent;
        }

        inline propertys_t::mapped_type& operator[] (const std::string& key) {
            return this->propertys[key];
        }

        const std::string& get_tag_name() const;

        void print(std::ostream& out, const char * indent_str = " ") const {
            html_util::indent_wraper ind(indent_str);
            this->print_impl(out, ind);
        }

        std::string get_property(const std::string& key) const;

        inline void set_property(const std::string& key, const std::string& value) {
            this->propertys[key] = value;
        }

        inline std::string get_class() const {
            return this->get_property("class");
        }

        inline std::string get_type() const {
            return this->get_property("type");
        }

        inline std::string get_id() const {
            return this->get_property("id");
        }

        inline void set_class(const std::string& c) {
            this->set_property("class", c);
        }

        inline void set_type(const std::string& t) {
            this->set_property("type", t);
        }

        inline void set_id(const std::string& t) {
            this->set_property("id", t);
        }
    };
// }}}1
} // end of namespace html_tags

} // end of namespace sss

#endif  /* __HTML_TAG_HPP_1390916796__ */
