#include "html_tag.hpp"

#include <string>
#include <cassert>
#include <sstream>

#include <sss/log.hpp>

#include "html_tag_name_map.hpp"
#include "html_tag_factory.hpp"

//#define HTML_TAG_USE_VIRTUAL_BASE

namespace sss {

namespace html_tags {

html_tag::~html_tag() {
    //SSS_LOG_DEBUG("%p\n", this );
}

html_tag *html_tag::get_root_tag() {
    html_tag *p_current = this;
    while (p_current->parent) {
        p_current = p_current->parent;
    }
    return p_current;
}

std::string html_tag::begin_tag() const {
#ifndef HTML_TAG_USE_VIRTUAL_BASE
    assert(false);
    return "";
#else
    std::ostringstream oss;
    oss << "<" << this->get_tag_name();
    for (propertys_t::const_iterator it = this->propertys.begin();
         it != this->propertys.end(); ++it) {
        oss << " " << it->first << "=\"" << it->second << "\"";
    }
    if (is_single) {
        oss << '/';
    }
    oss << '>';
    return oss.str();
#endif
}

std::string html_tag::end_tag() const {
#ifndef HTML_TAG_USE_VIRTUAL_BASE
    assert(false);
    return "";
#else
    if (!is_single) {
        return "</" + this->get_tag_name() + ">";
    }
    else {
        return "";
    }
#endif
}

html_tag * html_tag::clone() const {
    return 0;
}

std::string html_tag::get_property(const std::string& key) const {
    propertys_t::const_iterator it =  this->propertys.find(key);
    if (it != this->propertys.end()) {
        return it->second;
    }
    else {
        return "";
    }
}

const std::string& html_tag::get_tag_name() const {
    return sss::dom::html_tag_name_map::get_name_by_id(this->html_tag_id);
}

void html_tag::add(html_tag*) {
}

html_tag& html_tag::operator << (const html_tag& ) {
    // FIXME 应该抛出异常！而不是终止程序运行；
    assert(false);
    return *this;
}

html_tag& html_tag::operator () (const std::string& ) {
    assert(false);
    return *this;
}

void html_tag::add(const std::string& , const std::string& ) {
    assert(false);
}

html_tag::subnodes_t * html_tag::get_subnodes() {
    return 0;
}

html_tag *html_tag::create_tag(const std::string& tag_name) {
    return sss::dom::html_tag_factory::create(tag_name);
}

} // end of namespace html_tags

} // end of namespace sss

/*
id_A	a	true	true
id_ABBR	abbr	true	true
id_ACRONYM	acronym	true	true
id_ADDRESS	address	true	true
id_AREA	area	false	true
id_BASE	base	false	true
id_BDO	bdo	true	true
id_BLOCKCODE	blockcode	true	false
id_BODY	body	true	false
id_BR	br	false	false
id_BUTTON	button	true	true
id_CAPTION	caption	true	false
id_CITE	cite	true	true
id_CODE	code	true	true
id_COL	col	false	false
id_COLGROUP	colgroup	true	false
id_DD	dd	true	false
id_DEL	del	true	true
id_DFN	dfn	true	true
id_DIV	div	true	false
id_DL	dl	true	false
id_DT	dt	true	false
id_EM	em	true	false
id_FIELDSET	fieldset	true	false
id_FORM	form	true	false
id_FRAME	frame	false	false
id_FRAMESET	frameset	true	false
id_H1	h1	true	false
id_H2	h2	true	false
id_H3	h3	true	false
id_H4	h4	true	false
id_H5	h5	true	false
id_H6	h6	true	false
id_HEAD	head	true	false
id_HR	hr	false	false
id_HTML	html	true	false
id_IFRAME	iframe	true	true
id_IMG	img	false	true
id_INPUT	input	false	true
id_INS	ins	true	true
id_KBD	kbd	true	true
id_L	l	true	true
id_LABEL	label	true	false
id_LEGEND	legend	true	false
id_LI	li	true	false
id_LINK	link	false	true
id_MAP	map	true	false
id_META	meta	false	false
id_NOFRAMES	noframes	true	false
id_NOSCRIPT	noscript	true	false
id_OBJECT	object	true	false
id_OPTION	option	true	false
id_P	p	true	false
id_PARAM	param	false	false
id_PRE	pre	true	false
id_QUOTE	quote	true	true
id_Q	q	true	true
id_SAMP	samp	true	true
id_SCRIPT	script	true	false
id_SECTION	section	true	false
id_SELECT	select	true	true
id_SEPARATOR	separator	false	false
id_SPAN	span	true	true
id_STRONG	strong	true	true
id_STYLE	style	true	false
id_SUB	sub	true	true
id_SUP	sup	true	true
id_TABLE	table	true	false
id_TBODY	tbody	true	false
id_TD	td	true	true
id_TEXTAREA	textarea	true	false
id_TFOOT	tfoot	true	false
id_TH	th	true	true
id_THEAD	thead	true	false
id_TITLE	title	true	false
id_TR	tr	true	false
id_UL	ul	true	false
id_VAR	var	true	true
*/
