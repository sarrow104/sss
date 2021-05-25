#include "tag_element.hpp"

namespace sss {

REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::title);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::head);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::link);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::html);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::meta);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::img);

REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::body);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::div_node);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::h1);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::h2);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::h3);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::h4);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::h5);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::h6);

REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::span);

REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::table);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::tbody);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::tr);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::th);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::td);

REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::p);

REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::hr);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::br);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::style);

REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::a);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::li);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::ol);
REG_FACTORY(sss::dom::html_tag_factory, html_tags::html_tag, html_tags::ul);

} // end of namespace sss
