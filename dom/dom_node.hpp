#ifndef  __DOM_NODE_HPP_1392001923__
#define  __DOM_NODE_HPP_1392001923__

#include "html_util.hpp"

#include <string>
#include <iostream>
#include <map>

typedef std::string DOMString;

namespace sss {
namespace dom {
    //
    // 比如：
    // PCDATA,CDAT,<!DOCTYPE,<!-- comment,TEXT等等；
    enum dom_node_t {
        ELEMENT_NODE = 1,   // html,head,body,...
        ATTRIBUTE_NODE = 2,
        TEXT_NODE = 3,      // text
        CDATA_SECTION_NODE = 4,     // CDATA in <style>, <script>
        ENTITY_REFERENCE_NODE = 5,
        ENTITY_NODE = 6,
        PROCESSING_INSTRUCTION_NODE = 7,
        COMMENT_NODE = 8,
        DOCUMENT_NODE = 9,
        DOCUMENT_TYPE_NODE = 10,
        DOCUMENT_FRAGMENT_NODE = 11,
        NOTATION_NODE = 12
    };

    // html_tag 的基类；用于处理一些特殊的、少量的html元素种类；
    class dom_node{
    private:
        dom_node_t node_type;

    public:
        dom_node(dom_node_t type) : node_type(type) {
        }

        virtual ~dom_node() {
        }

    public:
        dom_node_t get_node_type() const {
            return this->node_type;
        }

        virtual DOMString begin_tag() const = 0;
        virtual DOMString end_tag() const = 0;
        virtual void print_impl(std::ostream& o,
                                html_util::indent_wraper& ind) const = 0;
    };

    class document_type : public dom_node {
    public:
        enum document_type_t {
            HTML401_LOOSE = 0,
            HTML401_STRICT,
            HTML401_FRAMESET,
            XHTML10_TRANSITIONAL,
            XHTML10_STRICT,
            XHTML10_FRAMESET,
            XHTML11,
            HTML5,
        };

        class document_type_notation
            : private std::map<document_type_t, std::string> {
        public:
            typedef std::map<document_type_t, std::string> value_type;
            using value_type::operator[];
            using value_type::iterator;
            using value_type::const_iterator;
            using value_type::find;

            document_type_notation();
        };

        explicit document_type(document_type_t _type = XHTML10_STRICT)
            : dom_node(DOCUMENT_TYPE_NODE),
              type(_type)
        {
        }

    public:
        void print(std::ostream& out,
                   const char * indent_str = " ") const {
            html_util::indent_wraper ind(indent_str);
            this->print_impl(out, ind);
        }

        virtual void print_impl(std::ostream& o,
                                html_util::indent_wraper& ind) const;
        virtual DOMString begin_tag() const;
        virtual DOMString end_tag() const;

    private:
        document_type_t type;
    };

    class text : public dom_node {
    private:
        DOMString data;

    public:
        explicit text(const DOMString& value)
            : dom_node(TEXT_NODE), data(value) {
        }

        virtual void print_impl(std::ostream& o,
                                html_util::indent_wraper& ind) const;
        virtual DOMString begin_tag() const;
        virtual DOMString end_tag() const;
    };

    class cdata_section : public dom_node {
    private:
        DOMString data;

    public:
        explicit cdata_section(const DOMString& value)
            : dom_node(CDATA_SECTION_NODE), data(value) {
        }

        virtual void print_impl(std::ostream& o,
                                html_util::indent_wraper& ind) const;
        virtual DOMString begin_tag() const;
        virtual DOMString end_tag() const;
    };

    class comment : public dom_node {
    private:
        DOMString data;

    public:
        explicit comment(const DOMString& value)
            : dom_node(COMMENT_NODE), data(value) {
        }

        virtual void print_impl(std::ostream& o,
                                html_util::indent_wraper& ind) const;
        virtual DOMString begin_tag() const;
        virtual DOMString end_tag() const;
    };
}       // end of namespace dom

}       // end of namespace sss

#endif  /* __DOM_NODE_HPP_1392001923__ */
