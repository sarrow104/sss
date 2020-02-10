#ifndef __INIPARSER_HPP_1465385991__
#define __INIPARSER_HPP_1465385991__

#include <string>

namespace sss {
    // 解析ini文件中，特定规则；并返回消耗的字节数；同时会写hook到的字段范围；
    char firstNoneSpace(const std::string& line, int& pos);

    void trimRange(const std::string& line, int &left, int &right);

    int iniParseSection(const std::string& line, int& left, int& right, int pos = 0);

    int iniParseKeyValue(const std::string& line,
                         int& key_left, int& key_right,
                         int& value_left, int & value_right,
                         int pos = 0);
    
    // 注意，注释的判断有些不同；
    // dosini的规则是，注释行，必须';'开头！
    int iniParseComment(const std::string& line, int& left, int & right);

    int iniParseEmptyLine(const std::string& line, int& left, int & right, int pos = 0);

} // namespace sss



#endif /* __INIPARSER_HPP_1465385991__ */
