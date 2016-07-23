#include "token.hpp"

#include <stdexcept>
#include <vector>
#include <map>

#include <sss/regex/simpleregex.hpp>
#include <sss/log.hpp>
#include <sss/utlstring.hpp>

// FIXME
// 这个模块，还必须输出 SSS_LOG_ERROR ，不然执行出错？非法内存读取？
// 本来全局log::level 是 log_DEBUG，然后这里基本都是SSS_LOG_DEBUG；
// 不得已，全部修改为SSS_LOG_ERROR；就正常了；
//
// 如何排查？折半替换回去！

namespace sss {
    namespace xml {

        struct is_not_space{
            bool operator () (char ch)
            {
                return
                    // 空格、水平制表符、垂直制表符
                    ch != ' ' && ch != '\t' && ch != '\v' &&
                    // 回车、换行
                    ch != '\r' && ch != '\n';
            }
        };

        // FIXME
        // xml名字：
        //      [a-zA-Z_][\w\d]\+\([-.:]\w[\w\d]\+\)*
        // 即：
        //      必须以字母或下划线开头。中键可以添加"-",".",":"分割；
        //      但是，不能以xml或者XML开头——这是xml语法规定的保留字……
        token_t::token_t(token_type_t type,
                         const std::string& d,
                         int r, int c)
            : token_type(type), data(d), row(r), col(c)
        {
            std::string::const_iterator it = this->data.begin();

            switch (this->token_type) {
            case token_xml_node_end:
            case token_xml_node_begin:
            case token_xml_node_self:
                {
                    //  ++it;                           // 跳过'/'
                    //  ++it;                           // 跳过'<'
                    static sss::regex::simpleregex
                        node_data_reg("\\(\\<\\c\\w*\\>\\)");

                    std::string::const_iterator match_beg_it = data.begin();
                    std::string::const_iterator match_end_it = data.end();

                    std::string::const_iterator submatch_beg_it;
                    std::string::const_iterator submatch_end_it;

                    if (node_data_reg.match(match_beg_it,    match_end_it,
                                            submatch_beg_it, submatch_end_it)) {
                        this->name = node_data_reg.get_submatch(1);
                        this->properties_str=sss::trim_copy(std::string(submatch_end_it,
                                                                        match_end_it - 1));
                    }

                    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, this->name);
                    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, this->properties_str);
                }
                break;

            case token_xml_info: // 2014-07-09
                it += 2;    // 跳过 "<?"
                {
                    static sss::regex::simpleregex info_name_reg("\\<\\c\\w+\\>");
                    std::string::const_iterator match_s_begin;
                    std::string::const_iterator match_s_end;

                    info_name_reg.match(it, this->data.end(),
                                        match_s_begin, match_s_end);

                    this->name = std::string(match_s_begin, match_s_end);

                    std::string::const_iterator match_e_begin = match_s_end;
                    std::string::const_iterator match_e_end = this->data.end();
                    // 跳过末尾的 "?>"
                    std::advance(match_e_end, -2);

                    this->properties_str = std::string(match_e_begin, match_e_end);
                    sss::trim(this->properties_str);
                }
                break;

            case token_xml_comment:
                //this->data = sss::trim_copy(this->data.substr(4,
                //                            this->data.length() - 7));
                sss::trim(this->data);
                break;

            default:
                break;
            }
        }

        token_t::token_t()
            : token_type(token_xml_nill), row(0), col(0)
        {
        }

        token_t::~token_t()
        {
        }

        void token_t::print(std::ostream& out) const
        {
            static std::map<token_type_t, std::string> token_name;
            if (token_name.empty()) {
                token_name[token_xml_nill] = "token_xml_nill";
                token_name[token_xml_info] = "token_xml_info";
                token_name[token_xml_comment] = "token_xml_comment";
                token_name[token_xml_node_begin] = "token_xml_node_begin";
                token_name[token_xml_node_end] = "token_xml_node_end";
                token_name[token_xml_node_self] = "token_xml_node_self";
                token_name[token_xml_line_text] = "token_xml_line_text";
                token_name[token_xml_space] = "token_xml_space";
                token_name[token_xml_node_cdata] = "token_xml_node_cdata";
                token_name[token_xml_doctype] = "token_xml_doctype";
            }
            out << "[ name=\"" << token_name[this->token_type] << "\""
                << ", data=\"" << this->data << "\""
                << ", row=" << this->row
                << ", col=" << this->col << " ]";
        }

        const std::string& token_t::get_name() const
        {
            return this->name;
        }

        // 获取token字符串的属性部分子串
        const std::string& token_t::get_properties_str() const
        {
            return this->properties_str;
        }

        // tokenizer
        tokenizer::tokenizer(const std::string& xml_str)
            : ini(xml_str.begin()), fin(xml_str.end()), row(0), col(0)
        {
        }

        tokenizer::tokenizer(std::string::const_iterator beg,
                             std::string::const_iterator end)
            : ini(beg), fin(end), row(0), col(0)
        {
        }

        tokenizer::~tokenizer()
        {
        }


        bool tokenizer::fetch(token_t& token)
        {
            try {
                if (ini == fin) {
                    return false;
                }

                typedef bool (tokenizer::* finder_t) ();
                static std::vector<finder_t> finders;
                if (finders.empty()) {
                    finders.push_back(&tokenizer::test_xml_info);       // <?xml
                    finders.push_back(&tokenizer::test_xml_doctype);    // <!DOCTYPE
                    finders.push_back(&tokenizer::test_xml_comment);    // <!--
                    finders.push_back(&tokenizer::test_xml_node_begin); // <\w+
                    finders.push_back(&tokenizer::test_xml_node_end);   // </\w+
                    finders.push_back(&tokenizer::test_xml_node_cdata); // <[CDATA[
                }

                bool ret = false;

                switch (*ini) {
                case '<':
                    for (size_t i = 0; i != finders.size(); ++i) {
                        ret = (this->*(finders[i]))();
                        if (ret) {
                            break;
                        }
                    }
                    break;

                case '>':
                    throw std::logic_error("unexpected '>'");
                    break;

                default:
                    if (is_not_space()(*ini)) {
                        ret = this->test_xml_node_text();
                    }
                    else {
                        ret = this->test_xml_space();
                    }
                    break;
                }

                try {
                    if (ret) {
                        token = token_t(this->token_type, this->data, row, col);
                    }
                }
                catch (std::exception & e) {
                    std::cout
                        << __FILE__ << ", " << __LINE__
                        << this->data
                        << std::endl;
                    throw;
                }

                return ret;
            }
            catch (std::exception & e) {
                std::cout << __FILE__ << ", " << __LINE__ << std::endl;
                throw;
            }
        }

        bool tokenizer::test_xml_info()
        {
            //SSS_LOG_ERROR("\n");
            static sss::regex::simpleregex reg_begin("^<?");
            static sss::regex::simpleregex reg_end("?>");

            std::string::const_iterator match_s_begin;
            std::string::const_iterator match_s_end;

            std::string::const_iterator match_e_begin;
            std::string::const_iterator match_e_end;

            bool ret = false;
            if (reg_begin.match(this->ini, this->fin,
                                match_s_begin, match_s_end))
            {
                ret = reg_end.match(match_s_end, this->fin,
                                    match_e_begin, match_e_end);

                if (ret) {
                    // 根据类型，决定如何保存要截取的数据；
                    data = std::string(match_s_begin, match_e_end);
                    token_type = token_t::token_xml_info;
                    this->move_next(match_s_begin, match_e_end);
                }
            }
            return ret;
        }

        // <!DOCTYPE ... >
        bool tokenizer::test_xml_doctype()
        {
            //SSS_LOG_ERROR("\n");
            static sss::regex::simpleregex reg_begin("^<!DOCTYPE\\>");
            static sss::regex::simpleregex reg_end(">");

            std::string::const_iterator match_s_begin;
            std::string::const_iterator match_s_end;

            std::string::const_iterator match_e_begin;
            std::string::const_iterator match_e_end;

            bool ret = false;
            if (reg_begin.match(this->ini, this->fin,
                                match_s_begin, match_s_end))
            {
                ret = reg_end.match(match_s_end, this->fin,
                                    match_e_begin, match_e_end);

                if (ret) {
                    // 根据类型，决定如何保存要截取的数据；
                    data = std::string(match_s_begin, match_e_end);
                    token_type = token_t::token_xml_doctype;
                    this->move_next(match_s_begin, match_e_end);
                }
            }
            return ret;
        }

        bool tokenizer::test_xml_comment()
        {
            //SSS_LOG_ERROR("\n");
            static sss::regex::simpleregex reg_begin("^<!--");
            static sss::regex::simpleregex reg_end("-->");

            std::string::const_iterator match_s_begin;
            std::string::const_iterator match_s_end;

            std::string::const_iterator match_e_begin;
            std::string::const_iterator match_e_end;

            bool ret = false;
            if (reg_begin.match(this->ini, this->fin,
                                match_s_begin, match_s_end))
            {
                ret = reg_end.match(match_s_end, this->fin,
                                    match_e_begin, match_e_end);

                if (ret) {
                    // 根据类型，决定如何保存要截取的数据；
                    data = std::string(match_s_end, match_e_begin);
                    token_type = token_t::token_xml_comment;

                    this->move_next(match_s_begin, match_e_end);
                }
            }
            return ret;
        }

        bool tokenizer::test_xml_node_begin()
        {
            // FIXME
            // 这个匹配有缺陷；应该找：第一个是'<'但是第二个不是'/'的串；之后，
            // 允许空白符；接着才是标示节点名字的标识符。
            static sss::regex::simpleregex reg_begin("^<\\c\\w*\\>");
            static sss::regex::simpleregex reg_end(">");

            std::string::const_iterator match_s_begin;
            std::string::const_iterator match_s_end;

            std::string::const_iterator match_e_begin;
            std::string::const_iterator match_e_end;

            bool ret = false;
            if (reg_begin.match(this->ini, this->fin,
                                match_s_begin, match_s_end))
            {
                ret = reg_end.match(match_s_end, this->fin,
                                    match_e_begin, match_e_end);

                if (ret) {
                    // 根据类型，决定如何保存要截取的数据；
                    data = std::string(match_s_begin, match_e_end);
                    if (*(data.rbegin() + 1) == '/') {
                        token_type = token_t::token_xml_node_self;
                    }
                    else {
                        token_type = token_t::token_xml_node_begin;
                    }

                    this->move_next(match_s_begin, match_e_end);
                }
            }
            return ret;
        }

        bool tokenizer::test_xml_node_end()
        {
            try {
                static sss::regex::simpleregex reg_begin("^</\\c\\w*\\>");
                static sss::regex::simpleregex reg_end(">");

                std::string::const_iterator match_s_begin;
                std::string::const_iterator match_s_end;

                std::string::const_iterator match_e_begin;
                std::string::const_iterator match_e_end;

                bool ret = false;
                if (reg_begin.match(this->ini, this->fin,
                                    match_s_begin, match_s_end))
                {
                    ret = reg_end.match(match_s_end, this->fin,
                                        match_e_begin, match_e_end);

                    if (ret) {
                        // 根据类型，决定如何保存要截取的数据；
                        data = std::string(match_s_begin, match_e_end);
                        token_type = token_t::token_xml_node_end;

                        this->move_next(match_s_begin, match_e_end);
                    }
                }
                return ret;
            }
            catch (std::exception & e) {
                std::cout << __FILE__ << ", " << __LINE__ << std::endl;
                throw;
            }
        }

        bool tokenizer::test_xml_node_cdata()
        {
            static sss::regex::simpleregex reg_begin("^<!\\[CDATA\\[");
            static sss::regex::simpleregex reg_end("\\]\\]>");

            std::string::const_iterator match_s_begin;
            std::string::const_iterator match_s_end;

            std::string::const_iterator match_e_begin;
            std::string::const_iterator match_e_end;

            bool ret = false;
            if (reg_begin.match(this->ini, this->fin,
                                match_s_begin, match_s_end))
            {
                ret = reg_end.match(match_s_end, this->fin,
                                    match_e_begin, match_e_end);

                if (ret) {
                    // 根据类型，决定如何保存要截取的数据；
                    data = std::string(match_s_end, match_e_begin);
                    token_type = token_t::token_xml_node_cdata;

                    this->move_next(match_s_begin, match_e_end);
                }
            }
            return ret;
        }

        struct is_not_line_text{
            bool operator () (char ch)
            {
                return !(ch != '\n' && ch != '\r' && ch != '<' && ch != '>');
            }
        };

        bool tokenizer::test_xml_node_text()
        {
            char ch = *this->ini;

            if (is_not_line_text()(ch)) {
                return false;
            }
            // 非换行符、<> 的所有符号
            std::string::const_iterator match_end =
                std::find_if(this->ini, this->fin, is_not_line_text());

            data = std::string(this->ini, match_end);
            token_type = token_t::token_xml_line_text;
            this->move_next(this->ini, match_end);

            return true;
        }

        bool tokenizer::test_xml_space()
        {
            if (is_not_space()(*this->ini)) {
                return false;
            }
            std::string::const_iterator match_end =
                std::find_if(this->ini, this->fin, is_not_space());

            data = std::string(this->ini, match_end);
            token_type = token_t::token_xml_space;

            this->move_next(this->ini, match_end);
            return true;
        }

        void tokenizer::move_next(std::string::const_iterator match_begin,
                                  std::string::const_iterator match_end)
        {
            ini = match_end;
            int line_cnt = std::count(match_begin, match_end, '\n');
            row += line_cnt;
            if (line_cnt) {
                std::string next_line("\n");
                col = std::find_end(match_begin, match_end,
                                    next_line.begin(), next_line.end()) - match_begin;
            }
            else {
                col += match_end - match_begin;
            }
        }
    }
}
