#include "IniParser.hpp"

namespace sss {
    // 解析ini文件中，特定规则；并返回消耗的字节数；同时会写hook到的字段范围；
    char firstNoneSpace(const std::string& line, int& pos) {
        char first_none_space = '\0';
        while (pos < int (line.length())) {
            if (!std::isspace(line[pos])) {
                first_none_space = line[pos];
                break;
            }
            pos++;
        }
        return first_none_space;
    }

    void trimRange(const std::string& line, int &left, int &right) {
        while (left < right && std::isspace(line[left])) {
            left++;
        }
        while (left < right && std::isspace(line[right - 1])) {
            right--;
        }
    }

    int iniParseSection(const std::string& line, int& left, int& right, int pos) {
        if (pos >= int (line.length()) || pos < 0) {
            return 0;
        }
        int used_cnt = 0;

        int  last = -1;
        char ch = '\0';
        int cnt = -1;
        if (0 == sscanf(line.c_str() + pos, " [%n%*[^]]%n]%n", &left, &right, &last) &&
            last > 0 &&
            (cnt = sscanf(line.c_str() + pos + last, " %c", &ch), (cnt == 0 || cnt == EOF)))
        {
            used_cnt = right;
            left += pos;
            right += pos;
            return used_cnt;
        }
        else {
            return false;
        }
    }

    int iniParseKeyValue(const std::string& line,
                         int& key_left, int& key_right,
                         int& value_left, int & value_right,
                         int pos)
    {
        if (pos >= int (line.length()) || pos < 0) {
            return 0;
        }

        int last = -1;
        if (0 == sscanf(line.c_str() + pos, " %n%*[^=]%n=%n", &key_left, &key_right, &last) && last > 0) {
            key_left += pos;
            key_right += pos;
            trimRange(line, key_left, key_right);
            value_left = last + pos;
            value_right = line.size(); // 这里不是很准确；这个赋值，是以line仅包含一行文本的前提而定的；如果……
            trimRange(line, value_left, value_right);
            return value_right;
        }
        else {
            return false;
        }
    }
    // 注意，注释的判断有些不同；
    // dosini的规则是，注释行，必须';'开头！
    int iniParseComment(const std::string& line, int& left, int & right) {
        // sss::regex::simpleregex reg_ini_linecomment("^;\\(.+\\)$");
        if (sss::is_begin_with(line, ";")) {
            left = 1;
            sss::firstNoneSpace(line, left);
            right = line.length();
            return right;
        }
        else {
            return false;
        }
    }

    int iniParseEmptyLine(const std::string& line, int& left, int & right, int pos) {
        int pos_rs = pos;
        left = pos;
        firstNoneSpace(line, pos);
        right = pos;
        return pos - pos_rs;
    }
} // namespace sss
