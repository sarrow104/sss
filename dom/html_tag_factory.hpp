#ifndef  __HTML_TAG_FACTORY_HPP_1391099096__
#define  __HTML_TAG_FACTORY_HPP_1391099096__

#include <string>
#include <map>

#include <sss/log.hpp>

namespace sss {

namespace html_tags{
class html_tag;
}

namespace dom {

class html_tag_factory {
public:
    typedef html_tags::html_tag* (* html_tag_creator_t) ();
    typedef std::map<std::string, html_tag_creator_t> instance_wrapper_inner_t;

    class instance_wrapper : private instance_wrapper_inner_t {
    public:
        using instance_wrapper_inner_t::operator[];

        using instance_wrapper_inner_t::find;
        using instance_wrapper_inner_t::size;
        using instance_wrapper_inner_t::begin;
        using instance_wrapper_inner_t::end;

        using instance_wrapper_inner_t::iterator;
        using instance_wrapper_inner_t::const_iterator;

    public:
        instance_wrapper();
        ~instance_wrapper();
    };

public:
    html_tag_factory() {
        //SSS_LOG_DEBUG("\n");
    }
    ~html_tag_factory() {
        //SSS_LOG_DEBUG("\n");
    }

    static bool reg_cloner(const std::string& name, html_tag_creator_t creator);

    static instance_wrapper& get_mapper();

    static html_tags::html_tag * create(const std::string& name);
};

} // end of namespace dom

} // end of namespace sss

#endif  /* __HTML_TAG_FACTORY_HPP_1391099096__ */

