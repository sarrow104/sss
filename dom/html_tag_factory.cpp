#include "html_tag_factory.hpp"

#include "html_tag.hpp"

namespace sss { namespace dom {
html_tag_factory::instance_wrapper::instance_wrapper() {
    //SSS_LOG_DEBUG("\n");
}

html_tag_factory::instance_wrapper::~instance_wrapper() {
}

html_tag_factory::instance_wrapper& html_tag_factory::get_mapper() {
    static html_tag_factory::instance_wrapper mapper;
    return mapper;
}

bool html_tag_factory::reg_cloner(const std::string& name, html_tag_creator_t creator) {
    //printf("reg %s => %p\n", name.c_str(), creator);
    (html_tag_factory::get_mapper())[name] = creator;
    return true;
}

html_tags::html_tag * html_tag_factory::create(const std::string& name) {
    html_tag_factory::instance_wrapper::iterator it = get_mapper().find(name);
    if (it != get_mapper().end()) {
        return (*it->second)();
    }
    else {
        return 0;
    }
}

} } // end of namespace sss::dom
