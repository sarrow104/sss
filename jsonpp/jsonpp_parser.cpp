#include "jsonpp.hpp"

#include <cctype>

#include <sss/log.hpp>
#include <sss/regex/simpleregex.hpp>
#include <sss/utlstring.hpp>

// TODO ����throw! �Է��ش�����Ϣ��

namespace sss {
namespace jsonpp
{
    JParser::JParser()  // {{{1
    {
    }

    JParser::~JParser() // {{{1
    {
    }

    JValue * JParser::falldown(const std::string& str)     // {{{1
    {
        return this->falldown(str.begin(), str.end());
    }

    JValue * JParser::falldown(const_iterator it_b, const_iterator it_e) // {{{1
    {
        jsonpp::JValue * p_jval = 0;
        it_b = this->skip_white_space(it_b, it_e);
        if (*it_b == '[') {
            p_jval = this->parse_array(it_b, it_e);
        }
        else if (*it_b == '{') {
            p_jval = this->parse_object(it_b, it_e);
        }
        else {
            SSS_LOG_INFO("not JSON_ARRAY or JSON_OBJECT\n");
            throw "not JSON_ARRAY or JSON_OBJECT\n";
        }

        return p_jval;
    }

    JValue * JParser::parse_value(const_iterator& it_b, const_iterator it_e) // {{{1
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b, it_e));

        it_b = this->skip_white_space(it_b, it_e);
        switch (*it_b) {
        case '"':
            return this->parse_string(it_b, it_e);
            break;

        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return this->parse_number(it_b, it_e);
            break;

        case '[':
            return this->parse_array(it_b, it_e);
            break;

        case '{':
            return this->parse_object(it_b, it_e);
            break;

        case 't':
        case 'f':
            return this->parse_bool(it_b, it_e);
            break;

        case 'n':
            return this->parse_null(it_b, it_e);
            break;
        }
        // NOTE ������һ��ֵ�Ľ���֮�������л���ʣ����ֽڣ���ô��δ���
        return 0;
    }

    // FIXME ����Ӧ�����Խ�������жϣ�
    // ���Խ���հ׷�֮�󣬾͵��˱߽磬��ô���׳��쳣��
    JParser::const_iterator JParser::skip_white_space(const_iterator it_b, const_iterator it_e) // {{{1
    {
        while (it_b != it_e && std::isspace(*it_b)) {
            ++it_b;
        }
        return it_b;
    }

    JValue *  JParser::parse_string(const_iterator& it_b, const_iterator it_e) // {{{1
    {
        // "���� �ַ���ֵ �Ĵ洢������չʾ"
        // �������ڣ�
        // json����Ϊһ�����ݽ�����׼�����ڵģ�
        // ����˵��������������������web���������ⲿ�ļ��У��ַ�������Ӧ���ϸ�
        // ������˫������������ת���ַ��ȵȱ�׼����������֣�
        // ���ǣ����ڲ����и����жϵ�ʱ�򣬱����ѯĳһ��json.object�Ƿ���ĳһ
        // �����Ե�ʱ����Ӧ���Ƚ���β��""��
        // ���ǣ���\\t�����ı�Ƿ��ţ�������жϺʹ洢�أ�
        const_iterator it_b_saved = it_b;
        if (this->parse_string_impl(it_b, it_e)) {
            return new jsonpp::JString(std::string(it_b_saved+1, it_b-1));
        }
        else {
            return 0;
        }
    }

    bool      JParser::parse_string_impl(const_iterator& it_b, const_iterator it_e) // {{{1
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        const_iterator it_b_saved = it_b;

        if (it_b == it_e || *it_b != '"') {
            return false;
        }

        ++it_b; // skip front '"' mark
        while (it_b != it_e && *it_b != '"') {
//            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b, it_e));
            switch (*it_b) {
            case '\\':
                {
                    if (std::distance(it_b, it_e) < 2) {
                        return 0;
                    }
                    const_iterator it_b2 = it_b;
                    ++it_b2;
                    switch (*it_b2) {
                    case '"':   // quotation mark
                    case '\\':  // reverse solidus
                    case '/':   // solidus
                    case 'b':   // backspace
                    case 'f':   // formfeed
                    case 'n':   // newline
                    case 'r':   // carriage return
                    case 't':   // horizontal tab
                        it_b +=2;
                        break;

                    case 'u':   // 4hexadecimal digits
                        {
                            if (std::distance(it_b, it_e) < 6) {
                                return false;
                            }
                            const_iterator it_hex = it_b + 2;
                            for (int i = 0; i < 4; ++i, ++it_hex) {
                                if (!std::isxdigit(*it_hex)) {
                                    SSS_LOG_INFO("return false %c\n", *it_hex);
                                    return false;
                                }
                            }
                            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b, it_hex));
                            it_b = it_hex;
                        }
                        break;

                    default:
                        SSS_LOG_INFO("return false %c\n", *it_b);
                        return false;
                        break;
                    }
                }
                break;

            default:
                if (std::iscntrl(*it_b)) {
                    return false;
                }
                else {
                    ++it_b;
                }
                break;
            }
        }
        if (it_b == it_e || *it_b != '"') {
            SSS_LOG_INFO("return false %c\n", *it_b);
            return false;
        }
        ++it_b; // skip end '"' mark
        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b_saved, it_b));
        return true;
    }

    JValue *  JParser::parse_number(const_iterator& it_b, const_iterator it_e) // {{{1
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        const_iterator it_b_saved = it_b;
        switch (*it_b) {
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            break;

        default:
            return 0;
        }
        // ����������
        if (*it_b == '-') {
            ++it_b;
            if (it_b == it_e) {
                return 0;
            }
        }

        // ��������
        if (*it_b == '0' && (it_b + 1 != it_e && !std::isdigit(*(it_b + 1)))) {
            ++it_b;
        }
        else if ('1' <= *it_b && *it_b <= '9') {
            while (it_b != it_e && std::isdigit(*it_b)) {
                ++it_b;
            }
        }
        else {
            return 0;
        }

        // С������
        if (it_b != it_e && *it_b == '.' &&
            it_b + 1 != it_e && std::isdigit(*(it_b + 1)))
        {
            it_b += 2;
            while (it_b != it_e && std::isdigit(*it_b)) {
                ++it_b;
            }
        }

        // eָ������
        if (it_b != it_e && std::toupper(*it_b) == 'E') {
            ++it_b;
            if (it_b != it_e && (*it_b == '+' || *it_b == '-')) {
                ++it_b;
            }
            int e_cnt = 0;
            while (it_b != it_e && std::isdigit(*it_b)) {
                ++it_b;
                ++e_cnt;
            }
            if (!e_cnt) {
                return 0;
            }
        }

        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b_saved, it_b));
        if (std::find(it_b_saved, it_b, '.') != it_b ||
            std::find(it_b_saved, it_b, 'e') != it_b ||
            std::find(it_b_saved, it_b, 'E') != it_b)
        {
            return new jsonpp::JDouble(sss::string_cast<double>(std::string(it_b_saved, it_b)));
        }
        else {
            return new jsonpp::JInt(sss::string_cast<int>(std::string(it_b_saved, it_b)));
        }
    }

    JValue *  JParser::parse_array(const_iterator& it_b, const_iterator it_e) // {{{1
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        const_iterator it_b_saved = it_b;
        if (it_b == it_e || *it_b != '[') {
            return 0;
        }
        ++it_b; // skip front '[' mark
        jsonpp::JArray tmp_ja;
        bool is_first = true;
        while ((it_b = this->skip_white_space(it_b, it_e)) != it_e && *it_b != ']') {
            if (!is_first) {
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b, it_e));
                if (*it_b == ',') {
                    ++it_b;
                }
            }
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b, it_e));
            const_iterator it_b_cur = it_b;
            jsonpp::JValue * tmp_val = this->parse_value(it_b, it_e);
            if (!tmp_val) {
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b_cur, it_e));
                return 0;
            }
            tmp_ja.add(tmp_val);
            is_first = false;
        }
        if (it_b == it_e || *it_b != ']') {
            SSS_LOG_INFO("return false %c\n", *it_b);
            return 0;
        }
        ++it_b; // skip end ']' mark
        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b_saved, it_b));
        jsonpp::JArray * p_ret = new jsonpp::JArray;
        p_ret->swap(tmp_ja);
        return p_ret;
    }

    JValue *  JParser::parse_object(const_iterator& it_b, const_iterator it_e) // {{{1
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        const_iterator it_b_saved = it_b;
        if (it_b == it_e || *it_b != '{') {
            SSS_LOG_INFO("return false %c\n", *it_b);
            return 0;
        }
        ++it_b; // skip front '{' mark
        bool is_first = true;
        jsonpp::JObject tmp_jo;
        while ((it_b = this->skip_white_space(it_b, it_e)) != it_e && *it_b != '}') {
            if (!is_first) {
                if (it_b == it_e) {
                    SSS_LOG_INFO("return false %c\n", *it_b);
                    return 0;
                }
                if (*it_b == ',') {
                    ++it_b;
                    continue;
                }
            }

            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b, it_e));

            // read name part
            it_b = this->skip_white_space(it_b, it_e);
            const_iterator it_name_beg = it_b;
            if (!this->parse_string_impl(it_b, it_e)) {
                SSS_LOG_INFO("return false %c\n", *it_b);
                return 0;
            }
            std::string name(it_name_beg+1, it_b-1);

            if (tmp_jo.has_key(name)) {
                return 0;
            }

            // read value part
            it_b = this->skip_white_space(it_b, it_e);
            if (it_b == it_e) {
                SSS_LOG_INFO("return false %c\n", *it_b);
                return 0;
            }
            if (*it_b == ':') {
                ++it_b;
                jsonpp::JValue * p_value = this->parse_value(it_b, it_e);
                if (!p_value) {
                    SSS_LOG_INFO("return false %c\n", *it_b);
                    return 0;
                }
                tmp_jo.add(name, p_value);
            }
            is_first = false;
        }
        if (it_b == it_e || *it_b != '}') {
            SSS_LOG_INFO("return false %c\n", *it_b);
            return 0;
        }
        ++it_b; // skip end '}' mark
        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b_saved, it_b));
        jsonpp::JObject * p_ret = new jsonpp::JObject;
        p_ret->swap(tmp_jo);
        return p_ret;
    }

    JValue *  JParser::parse_bool(const_iterator& it_b, const_iterator it_e) // {{{1
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        const_iterator it_b_saved = it_b;
        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b, it_e));

        if (it_b == it_e || (*it_b != 't' && *it_b != 'f')) {
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b, it_e));
            return 0;
        }
        static sss::regex::simpleregex reg_true("^\\<true\\>");
        static sss::regex::simpleregex reg_false("^\\<false\\>");
        if (reg_true.match(it_b, it_e)) {
            it_b += 4;
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b_saved, it_b));
            return new jsonpp::JBool(true);
        }
        if (reg_false.match(it_b, it_e)) {
            it_b += 5;
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b_saved, it_b));
            return new jsonpp::JBool(false);
        }
        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b_saved, it_e));
        return 0;
    }

    JValue *  JParser::parse_null(const_iterator& it_b, const_iterator it_e) // {{{1
    {
        SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
        const_iterator it_b_saved = it_b;

        if (it_b == it_e || *it_b != 'n') {
            return 0;
        }
        static sss::regex::simpleregex reg_null("^\\<null\\>");
        if (reg_null.match(it_b, it_e)) {
            it_b += 4;
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(it_b_saved, it_b));
            return new jsonpp::JNull;
        }
        return 0;
    }
}

}
