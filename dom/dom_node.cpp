#include "dom_node.hpp"

namespace sss {
namespace dom {

    document_type::document_type_notation::document_type_notation() {
        (*this)[HTML401_LOOSE] = "PUBLIC"
            " \"-//W3C//DTD HTML 4.01 Transitional//EN\""
            " \"http://www.w3.org/TR/html4/loose.dtd\"";
        (*this)[HTML401_STRICT] = "PUBLIC"
            " \"-//W3C//DTD HTML 4.01//EN\""
            " \"http://www.w3.org/TR/html4/strict.dtd\"";
        (*this)[HTML401_FRAMESET] = "PUBLIC"
            " \"-//W3C//DTD HTML 4.01 Frameset//EN\""
            " \"http://www.w3.org/TR/html4/frameset.dtd\"";
        (*this)[XHTML10_TRANSITIONAL] = "PUBLIC"
            " \"-//W3C//DTD XHTML 1.0 Transitional//EN\""
            " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"";
        (*this)[XHTML10_STRICT] = "PUBLIC"
            " \"-//W3C//DTD XHTML 1.0 Strict//EN\""
            " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"";
        (*this)[XHTML10_FRAMESET] = "PUBLIC"
            " \"-//W3C//DTD XHTML 1.0 Frameset//EN\""
            " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-frameset.dtd\"";
        (*this)[XHTML11] = "PUBLIC"
            " \"-//W3C//DTD XHTML 1.1//EN\""
            " \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\"";
        (*this)[HTML5] = "";
    }

    void document_type::print_impl(std::ostream& o,
                                   html_util::indent_wraper& ind) const {
        static document_type_notation notations;
        const std::string& notation = notations[this->type];
        o << ind.get() << this->begin_tag();
        if (notation.length()) {
            o << " " << notation;
        }
        o << this->end_tag() << std::endl;
    }

    DOMString document_type::begin_tag() const {
        return "<!DOCTYPE html";
    }

    DOMString document_type::end_tag() const {
        return ">";
    }
    // "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\""
    // " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"

    // text node
    void text::print_impl(std::ostream& o,
                    html_util::indent_wraper& ind) const {
        o << ind.get() << this->data << std::endl;
    }

    DOMString text::begin_tag() const {
        return "";
    }

    DOMString text::end_tag() const {
        return "";
    }

    // cdata_section node
    void cdata_section::print_impl(std::ostream& o,
                    html_util::indent_wraper& ind) const {
        o << ind.get() << this->begin_tag() << std::endl;
        {
            html_util::indent_auto ind_auto(ind);
            o << ind.get() << this->data << std::endl;
        }
        o << ind.get() << this->end_tag() << std::endl;
    }

    DOMString cdata_section::begin_tag() const {
        return "<[CDATA[";
    }

    DOMString cdata_section::end_tag() const {
        return "]]>";
    }

    // comment node
    void comment::print_impl(std::ostream& o,
                    html_util::indent_wraper& ind) const {
        o << ind.get() << this->begin_tag()
            << this->data << this->end_tag() << std::endl;
    }

    DOMString comment::begin_tag() const {
        return "<!--";
    }

    DOMString comment::end_tag() const {
        return "-->";
    }

} // end of namespace dom

} // end of namespace sss
