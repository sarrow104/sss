#ifndef  __NAME_FILTER_HPP_1440926255__
#define  __NAME_FILTER_HPP_1440926255__

#include "glob_path.hpp"
#include "filter_t.hpp"

#include <sss/regex/cregex.hpp>

namespace sss{
    namespace path{

class name_filter_t : public filter_t
{
public:
    explicit name_filter_t(const std::string& pattern);

    ~name_filter_t();

public:
    bool is_match(const file_descriptor& fd)
    {
        return this->_regex.match(fd.get_name());
    }

    sss::regex::CRegex& regex()
    {
        return this->_regex;
    }

    static std::string gen_pattern_regstr(const std::string& pattern);

public:
    sss::regex::CRegex _regex;
};
    }
}


#endif  /* __NAME_FILTER_HPP_1440926255__ */


