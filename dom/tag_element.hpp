#ifndef  __TAG_ELEMENT_HPP_1391696194__
#define  __TAG_ELEMENT_HPP_1391696194__

#include "html_tag.hpp"

#include "html_tag_name_map.hpp"
#include "factory_policy.hpp"
#include "html_tag_factory.hpp"

#include <sss/log.hpp>

#include <typeinfo>

#include <string>
#include <cassert>
#include <sstream>
#include <map>
#include <cctype>
#include <vector>
#include <iostream>

namespace sss {
namespace html_tags {

    //����ģ���html�ڵ�����
    // ������д���룬һ���Ǽ̳У������࣬������Ҫ������������̳������൱�鷳����
    // һ����ģ���ࣻ����visitor����ڲ������Կ���ģ���Ա�����ķ�ʽ�������ٴ����
    // ��д��
    //
    // ������������Ҫdispatcher�Ĵ��룬ģ�溯���������������𣿻�������ģ���࿿��
    // һЩ��
    //----------------------------------------------------------------------
    //
    // TODO
    // �Ľ���
    //
    // ����ģ������������ṩ������ܵĶ��ƣ�
    //
    // ����:
    // template<int html_tag_id, bool is_single> class html_tag_impl
    //     : public html_tag,
    //       public html_data<is_single> {
    // };
    //
    // template<bool is_single> class html_data {
    // };
    //
    // template<> class html_data<true>{
    // ...
    // };
    //
    // template<> class html_data<false>{
    // ...
    // };
    //
    // ���⣬����ܽ���һ��ģ����� int html_tag_id������ const char * tag_name���͸����ˣ�
    //
    // ���У���������ʹ���У���Ҫ���ϵ�new��Ҳ��һ���鷳�£����libxl��Ӧ��ʹ�ù����ࣺ
    //
    template<bool is_single, int id> class tag_style { }; // {{{1

    template<int id> class tag_style<false, id> : public html_tag { // {{{1
    public:
        tag_style() : html_tag(id, false) {
        }

        ~tag_style() {
        }

        std::string begin_tag() const {
            std::ostringstream oss;
            oss << "<" << this->get_tag_name();
            for (propertys_t::const_iterator it = this->propertys.begin();
                 it != this->propertys.end(); ++it) {
                oss << " " << it->first << "=\"" << it->second << "\"";
            }
            oss << '>';
            return oss.str();
        }

        std::string end_tag() const {
            return "</" + this->get_tag_name() + ">";
        }
    };

    // �Ապ� ���� begin_tag
    template<int id> class tag_style<true, id> : public html_tag { // {{{1
    public:
        tag_style() : html_tag(id, true) {
        }

        std::string begin_tag() const {
            std::ostringstream oss;
            oss << "<" << this->get_tag_name();
            for (propertys_t::const_iterator it = this->propertys.begin();
                 it != this->propertys.end(); ++it) {
                oss << " " << it->first << "=\"" << it->second << "\"";
            }
            oss << "/>";
            return oss.str();
        }

        std::string end_tag() const {
            return "";
        }
    };

    // ��Ա�������ɲ�����ƫ�ػ��أ�
    // template<int id>
    //     html_tag_impl<int id, false, true>::add() {
    // }

    // ���ڷ��Ապͱ�ǩ�����ӡ����������ַ�ʽ��һ���ǻ��У�һ���ǽ�����һ�У�
    // ���ң�ͨ�����еģ����ڲ������ӽṹ�ģ�������һ�����ڲ�ֻ��text�ṹ��
    template<bool is_inline, bool is_single, int id, typename T> class print_style { }; // {{{1

    template<bool is_single, int id, typename T> class print_style<false, is_single, id, T> // {{{1
        : public tag_style<is_single, id > {
    public:
        typedef tag_style<is_single, id > tag_style_t;
        print_style() : tag_style_t() {
        }

        typedef std::vector<html_tag*> elements_t;
        elements_t elements;

        ~print_style () {
            for (elements_t::iterator it = elements.begin(); it != elements.end(); it++) {
                if (*it) {
                    delete *it;
                }
            }
        }

        html_tag& operator << (const html_tag& sub) {
            html_tag *p_tmp = sub.clone();
            this->add(p_tmp);
            return *this;
        }

        void add(html_tag* p_sub) {
            p_sub->parent = this;
            this->elements.push_back(p_sub);
        }

        html_tag * clone() const {
            // NOTE ��̬���죡
            print_style * ret = dynamic_cast<print_style*>(new T(dynamic_cast<const T&>(*this)));

            ret->elements.clear();
            for (elements_t::const_iterator it = elements.begin(); it != elements.end(); it++) {
                ret->add((*it)->clone());
            }

            return ret;
        }

        print_style& operator () (const std::string& txt) {
            this->text = txt;
            return *this;
        }

        html_tags::html_tag::subnodes_t *get_subnodes() {
            return &this->elements;
        }

        void print_impl(std::ostream& out, html_util::indent_wraper& ind) const {
            out << ind.get() << this->begin_tag() << std::endl;
            {
                html_util::indent_auto ind_auto(ind);
                for (elements_t::const_iterator it = this->elements.begin();
                     it != this->elements.end(); ++it) {
                    (*it)->print_impl(out, ind);
                }
                if (!this->text.empty()) {
                    out << ind.get() << this->text << std::endl;
                }
            }
            out << ind.get() << this->end_tag() << std::endl;
        }
    };

    template<bool is_single, int id, typename T> class print_style<true, is_single, id, T>  // {{{1
        : public tag_style<is_single, id> {
    public:
        typedef tag_style<is_single, id > tag_style_t;
        print_style() : tag_style_t() {
        }

        typedef std::vector<html_tag*> elements_t;
        elements_t elements;

        //print_style(const print_style& ref) : html_tag(ref.id, ref.is_single) {
        //    this->text = ref.text;
        //}
        ~print_style() {
            for (elements_t::iterator it = elements.begin(); it != elements.end(); it++) {
                if (*it) {
                    delete *it;
                }
            }
        }

        html_tag& operator << (const html_tag& sub) {
            html_tag *p_tmp = sub.clone();
            this->add(p_tmp);
            return *this;
        }

        void add(html_tag* p_sub) {
            p_sub->parent = this;
            this->elements.push_back(p_sub);
        }

        print_style& operator () (const std::string& txt) {
            this->text = txt;
            return *this;
        }

        html_tag * clone() const {
            //SSS_LOG_DEBUG("%s\n", typeid(*this).name());
            //T * tmp = new T(dynamic_cast<const T&>(*this));
            //SSS_LOG_DEBUG("%s\n", typeid(*tmp).name());
            //SSS_LOG_DEBUG("clone from %p get text = %s at %p\n", this, tmp->text.c_str(), tmp);
            //return tmp;
            print_style * ret = dynamic_cast<print_style*>(new T(dynamic_cast<const T&>(*this)));

            ret->elements.clear();
            for (elements_t::const_iterator it = elements.begin(); it != elements.end(); it++) {
                ret->add((*it)->clone());
            }

            return ret;
        }

        void print_impl(std::ostream& out, html_util::indent_wraper& ind) const {
            out << ind.get() << this->begin_tag();
            out << this->text;
            if (!elements.empty()) {
                html_util::indent_wraper ind("");
                for (elements_t::const_iterator it = this->elements.begin();
                     it != this->elements.end(); ++it)
                {
                    (*it)->print_impl(out, ind);
                }
            }
            out << this->end_tag();
            if (ind.get().length()) {
                out << std::endl;
            }
        }
    };

    template<int id, bool is_single, bool is_inline> class html_tag_impl    // {{{1
        : public print_style<is_inline, is_single, id,
                             html_tag_impl<id, is_single, is_inline> >,
          public sss::factory_policy<sss::dom::html_tag_factory, html_tag,
                                     html_tag_impl<id, is_single, is_inline> >{
    public:
        // is_single:
        //      �Ƿ񵥱�ǩ�� true: <br /> ; false: <p> </p>
        // is_inline: FIXME inline��Ҳ�������ӽṹ��
        //      �Ƿ�û���ӽṹ��true: <br />,<td>; false: <body>...</body> <tr>...</tr>
        static const int tag_id = id;
        typedef html_tag_impl<id, is_single, is_inline> html_tag_impl_t;
        typedef print_style<is_inline, is_single, id, html_tag_impl_t > print_style_t;
        typedef sss::factory_policy<sss::dom::html_tag_factory, html_tag, html_tag_impl_t > factory_policy_t;

        html_tag_impl() :
            print_style_t(),
            factory_policy_t() {
        }
        ~html_tag_impl() {
        }
    };

    // �ػ� html �������
    template<> class html_tag_impl<sss::dom::html_tag_name_map::id_HTML, false, false>  // {{{1
        : public print_style<false, false, sss::dom::html_tag_name_map::id_HTML, html_tag_impl<sss::dom::html_tag_name_map::id_HTML, false, false> >,
          public sss::factory_policy<sss::dom::html_tag_factory, html_tag,
                                     html_tag_impl<sss::dom::html_tag_name_map::id_HTML, false, false> >{
    public:
        static const int tag_id = sss::dom::html_tag_name_map::id_HTML;
        typedef html_tag_impl<tag_id, false, false> html_tag_impl_t;
        typedef print_style<false, false, tag_id, html_tag_impl_t > print_style_t;
        typedef sss::factory_policy<sss::dom::html_tag_factory, html_tag, html_tag_impl_t> factory_policy_t;

        html_tag_impl()
            : print_style_t(),
              factory_policy_t() {
        }
        ~html_tag_impl() {
        }

        void print_impl(std::ostream& out, html_util::indent_wraper& ind) const {
            // out << ind.get()
            //     << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\""
            //     << " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">" << std::endl;
            print_style_t::print_impl(out, ind);
        }
    };

    template<> class html_tag_impl<sss::dom::html_tag_name_map::id_STYLE, false, false> // {{{1
        : public tag_style<false, sss::dom::html_tag_name_map::id_STYLE>,
          public sss::factory_policy<sss::dom::html_tag_factory, html_tag,
                                     html_tag_impl<sss::dom::html_tag_name_map::id_STYLE, false, false> > {
    public:
        static const int tag_id = sss::dom::html_tag_name_map::id_STYLE;
        typedef tag_style<false, tag_id> tag_style_t;
        typedef html_tag_impl<tag_id, false, false> html_tag_impl_t;
        typedef sss::factory_policy<sss::dom::html_tag_factory, html_tag, html_tag_impl_t > factory_policy_t;
        typedef std::map<std::string, std::string> elements_t;

        elements_t elements;

        html_tag_impl()
            : tag_style_t(),
              factory_policy_t() {
            this->set_type("text/css");
        }

        ~html_tag_impl() {
        }

        void add(const std::string& key, const std::string& css) {
            this->elements[key] = css;
        }

        html_tag_impl * clone() const {
            return new html_tag_impl(*this);
        }

        // FIXME
        void print_impl(std::ostream& out, html_util::indent_wraper& ind) const {
            out << ind.get() << this->begin_tag() << std::endl;
            {
                html_util::indent_auto ind_auto(ind);
                for (elements_t::const_iterator it = this->elements.begin();
                     it != this->elements.end(); ++it) {
                    out << ind.get() << it->first << "{" << it->second << "}" << std::endl;
                }
            }
            out << ind.get() << this->end_tag() << std::endl;
        }
    };

    // NOTE id_STYLE��Ҫ�ػ�����

    // html_tag typedef-class list {{{1
    // vim:'<,'>s/^\s*\<\(id_\)\(\w\+\)\>,/typedef html_tag_impl<html_tag_name_map::id_\2> \U\2;/ge
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_HTML, false, false> html;

    typedef html_tag_impl<sss::dom::html_tag_name_map::id_HEAD, false, false> head;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_LINK, true, true> link;

    typedef html_tag_impl<sss::dom::html_tag_name_map::id_TITLE, false, true> title;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_META, true, true> meta;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_IMG, true, true> img;

    typedef html_tag_impl<sss::dom::html_tag_name_map::id_BODY, false, false> body;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_DIV, false, false> div_node;

    typedef html_tag_impl<sss::dom::html_tag_name_map::id_H1, false, true> h1;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_H2, false, true> h2;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_H3, false, true> h3;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_H4, false, true> h4;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_H5, false, true> h5;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_H6, false, true> h6;

    typedef html_tag_impl<sss::dom::html_tag_name_map::id_SPAN, false, true> span;

    typedef html_tag_impl<sss::dom::html_tag_name_map::id_TABLE, false, false> table;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_TBODY, false, false> tbody;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_TR, false, false> tr;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_TH, false, true> th;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_TD, false, true> td;

    typedef html_tag_impl<sss::dom::html_tag_name_map::id_P, false, false> p;

    typedef html_tag_impl<sss::dom::html_tag_name_map::id_HR, true, true> hr;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_BR, true, true> br;

    typedef html_tag_impl<sss::dom::html_tag_name_map::id_STYLE, false, false> style;

    typedef html_tag_impl<sss::dom::html_tag_name_map::id_A, false, true> a;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_LI, false, true> li;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_OL, false, false> ol;
    typedef html_tag_impl<sss::dom::html_tag_name_map::id_UL, false, false> ul;

    // end html_tag typedef-class list }}}1
} // end of namespace html_tags

} // end of namespace sss

#endif  /* __TAG_ELEMENT_HPP_1391696194__ */
