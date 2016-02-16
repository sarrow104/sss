#ifndef  __CSS_COLOR_TABLE_HPP_1373345777__
#define  __CSS_COLOR_TABLE_HPP_1373345777__

#include <map>
#include <string>

namespace sss {

class css_color_table : private std::map<std::string, int>
{
private:
    css_color_table();
    ~css_color_table();

private:
    static css_color_table& get_singleton();

public:
    static bool is_valid_name(const std::string& name);

    static int convert(const std::string& name);
};

} // end of namespace sss

#endif  /* __CSS_COLOR_TABLE_HPP_1373345777__ */
