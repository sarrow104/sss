#include "css.hpp"

#include <sss/spliter.hpp>
#include <sss/utlstring.hpp>
#include <sss/log.hpp>

#include "css_color_table.hpp"

namespace sss {
css_inline::css_inline(const std::string& style) {
    sss::Spliter sp(style, ';');
    std::string stem;
    while(sp.fetch_next(stem)) {
        if (stem.empty()) {
            continue;
        }
        std::string::size_type pos = stem.find(':');
        if (pos == std::string::npos) {
            continue;
        }
        std::string name = sss::trim_copy(stem.substr(0, pos));
        std::string value = sss::trim_copy(stem.substr(pos + 1));
        SSS_LOG_DEBUG("css_inline[%s] = %s;\n", name.c_str(), value.c_str());

        iterator it = this->find(name);
        if (it != this->end()) {
            it->second = value;
        }
        else {
            (*this).insert(this->end(), std::make_pair(name, value));
        }
        //(*this)[name] = value;
    }
}

const std::string& css_inline::operator [] (const std::string& key) {
    static const std::string default_value = "";
    iterator it = this->find(key);
    if (it != this->end()) {
        return it->second;
    }
    else {
        return default_value;
    }
}

css_color::css_color(const std::string& style)
    : r(0), g(0), b(0)
{
    // 1. #xxxxxxx
    if (style.length() == 7 && style[0] == '#') {
        int num = sscanf(style.c_str() + 1, "%02x%02x%02x", &this->r, &this->g, &this->b);
        SSS_LOG_DEBUG("%d = rgb(%d, %d, %d);\n", num, this->r, this->g, this->b);
    }

    // 2. rgb(num1, num2, num3) -- rgb(1,2,3)
    else if (style.length() >= 10 && style.substr(0, 3) == "rgb") {

        while (true) {
            int red = 0, green = 0, blue = 0, n = -1;
            int cnt = sscanf(style.c_str() + 3,
                             " %*1[(] %d %*1[,] %d %*1[,] %d %*1[)]%n",
                             &r, &g, &b, &n);
            // 未读完，或者还有剩余
            // ??? cnt
            if (cnt != 3 || n == -1 || int(style.length()) > 3 + n) {
                break;
            }
            this->r = red;
            this->g = green;
            this->b = blue;

            break; // 无论成功与否，都跳出循环
        }
    }
    // 3. 实名：
    else if (css_color_table::is_valid_name(style)) {
        int color = css_color_table::convert(style);
        this->r = (color >> 16) & 0xFF;
        this->g = (color >>  8) & 0xFF;
        this->b =  color        & 0xFF;
    }
}

} // end of namespace sss
