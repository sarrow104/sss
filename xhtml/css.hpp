#ifndef  __CSS_HPP_1388674276__
#define  __CSS_HPP_1388674276__

#include <string>
#include <map>

namespace sss {
class css_inline : private std::map<std::string, std::string>
{
public:
    explicit css_inline(const std::string& style);

public:
    inline bool has_style(const std::string& s)
    {
        return this->find(s) != this->end();
    }

    const std::string& operator [] (const std::string& key);
};

class css_color{
public:
    // 可识别的有：
    // 1. #xxxxxxx      -- OK
    // 2. rgb(num1, num2, num3) -- rgb(1,2,3) NOTE 只支持10进制数字
    // 3. 实名 见 css_color_table
    // NOTE
    // 1. 默认传入的style 已经 sss::trim 之后
    // 2. 如果 传入 的 描述颜色的字符串，无法解析，则默认为 rgb(0,0,0)，即黑色
    css_color(const std::string& style);

public:
    int r;
    int g;
    int b;
};

}


#endif  /* __CSS_HPP_1388674276__ */
