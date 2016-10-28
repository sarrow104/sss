#include "xml_parser.hpp"

#include <assert.h>

#include <algorithm>
#include <stdexcept>

#include <sss/log.hpp>

#include "xml_doc.hpp"

namespace sss {

    namespace xml {
        xml_parser::xml_parser()
            : tmp_node(0)
        {
        }

        xml_parser::~xml_parser()
        {
        }

        // 解析 xml_str_data ，构建于 tmp_doc 对象
        xml::xml_doc * xml_parser::parse(xml::xml_doc * tmp_doc,
                                         const std::string& xml_str_data)
        {
            return this->parse(tmp_doc,
                               xml_str_data.begin(),
                               xml_str_data.end());
        }

        int    xml_parser::tokens_assign(tokens_t& tks,
                                         std::string::const_iterator s_ini,
                                         std::string::const_iterator s_fin)
        {
            int size_cnt = 0;
            token_t token;

            try {
                tks.clear();
                tokenizer tkn(s_ini, s_fin);

                while (tkn.fetch(token)) {
                    size_cnt ++;
                    tks.push_back(token);
                }

                return size_cnt;
            }
            catch (std::exception & e) {
                std::cout
                    << __FILE__ << ", " << __LINE__
                    << " size_cnt = " << size_cnt
                    << token
                    << std::endl;
                throw;
            }
        }

        // 先处理字节流的bom
        // 然后将字节流，转换成token流
        //
        // 第一遍循环，分析节点对应关系，是否正确
        // 检测 info.charset ―― 提供默认值
        // 第二遍循环，构建节点；
        // 检查 root 是否唯一；
        //
        //! 上面是解析xml的流程；其实解析xhtml的话，也可以套用这个流程；
        // 不过，info 节点的检测，需要做小修改：应该最后来判断这个节点的存在性
        // ，以及出现的位置合不合适。
        //
        // 如：
        // 先处理字节流的bom
        // 然后将字节流，转换成token流
        //
        // 第一遍循环，分析节点对应关系，是否正确
        // 检测 doctype
        // 第二遍循环，构建节点；
        // 检查 root 是否唯一；
        // 如果doctype 不存在，则提供一个默认值；
        // 如果doctype 位置不对，则将其挪至开始。
        //
        //! 额，或者说，上述合法性检测，应该挪到实体对象的构造函数中完成
        //（当然，也可以借助一个叫validata的成员函数）
        // 那样的话，parser函数，就能显得比较简洁；

        void   xml_parser::tokens_pair_node_check() // throw
        {
            // "stack check"
            for (size_t i = 0; i != tokens.size(); ++i) {
                switch (tokens[i].token_type) {
                case token_t::token_xml_nill:
                case token_t::token_xml_comment:
                case token_t::token_xml_space:
                case token_t::token_xml_info:
                case token_t::token_xml_doctype:
                case token_t::token_xml_node_cdata:

                case token_t::token_xml_node_self:
                    break;

                case token_t::token_xml_node_begin:
                    this->level_stack.push_back(tokens[i].get_name());
                    break;

                case token_t::token_xml_node_end:
                    if (this->level_stack.back() == tokens[i].get_name()) {
                        this->level_stack.pop_back();
                    }
                    else {
                        throw std::logic_error("unexpected node "
                                               + tokens[i].get_name());
                    }
                    break;

                case token_t::token_xml_line_text:
                    break;
                }
            }
            //std::cout
            //  << "tokens cnt = " << tokens.size()
            //  << " stack checked" << std::endl;

            if (!this->level_stack.empty()) {
                throw std::logic_error("unfinished xml node stack");
            }

        }

        xml::xml_doc * xml_parser::parse(xml::xml_doc * tmp_doc,
                                         std::string::const_iterator s_ini,
                                         std::string::const_iterator s_fin)
        {
            try {
                int bom_len = this->parse_bom(s_ini, s_fin);
                std::advance(s_ini, bom_len);

                this->tokens_assign(this->tokens, s_ini, s_fin);

                this->tokens_pair_node_check();

                tokens_t::const_iterator tk_ini = tokens.begin();
                tokens_t::const_iterator tk_fin = tokens.end();

                this->tmp_node = tmp_doc;

                while (tk_ini != tk_fin) {
                    tk_ini = parse_impl(tk_ini, tk_fin);
                }

                return tmp_doc;
            }
            catch (std::exception & e) {
                std::cout << __FILE__ << ", " << __LINE__ << std::endl;
                throw;
            }
        }

        tokens_t::const_iterator
            xml_parser::skip_useless_token(tokens_t::const_iterator tk_ini,
                                           tokens_t::const_iterator tk_fin)
        {
            bool skip = true;
            while (skip && tk_ini != tk_fin) {
                switch ((*tk_ini).token_type) {
                case token_t::token_xml_nill:
                //case token_t::token_xml_comment:
                case token_t::token_xml_space:
                //case token_t::token_xml_info:
                    tk_ini++;
                    break;

                default:
                    skip = false;
                    break;
                }
            }
            return tk_ini;
        }

        tokens_t::const_iterator xml_parser::parse_impl(tokens_t::const_iterator tk_ini,
                                                        tokens_t::const_iterator tk_fin)
        {
            tk_ini = skip_useless_token(tk_ini, tk_fin);
            if (tk_ini == tk_fin) {
                return tk_ini;
            }
            xml_doc * tmp_doc = tmp_node->get_doc();

            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *tk_ini);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, tk_ini->get_name());
            const std::string& name = tk_ini->get_name();
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, name);
            const std::string& properties_str = tk_ini->get_properties_str();

            switch (tk_ini->token_type) {
            case token_t::token_xml_doctype:
                tmp_node->append_child(tmp_doc->create_doctype(tk_ini->data));
                tk_ini++;
                break;

            case token_t::token_xml_info:
                {
                    xml::node * p_current_node =
                        tmp_node->append_child(tmp_doc->create_info(name));
                    p_current_node->parse_properties_str(properties_str);
                    tk_ini++;
                }
                break;

            case token_t::token_xml_comment:
                tmp_node->append_child(tmp_doc->create_comment(tk_ini->data));
                tk_ini++;
                break;

            case token_t::token_xml_node_self:
                {
                    xml::node * p_current_node =
                        tmp_node->append_child(tmp_doc->create_node(name));
                    p_current_node->parse_properties_str(properties_str);
                    tk_ini++;
                }
                break;

            case token_t::token_xml_node_begin:
                this->level_stack.push_back(name);
                tmp_node = tmp_node->append_child(tmp_doc->create_node(name));
                tmp_node->parse_properties_str(properties_str);
                tk_ini++;
                break;

            case token_t::token_xml_node_end:
                if (this->level_stack.back() == tk_ini->get_name()) {
                    this->level_stack.pop_back();
                    assert(tmp_node);
                    tmp_node = tmp_node->get_parent();
                    tk_ini++;
                }
                else {
                    throw std::logic_error("unexpected node " +
                                           tk_ini->get_name());
                }
                break;

            case token_t::token_xml_line_text:
                // FIXME 检测 text 出现的位置的合法性（比如，不能与根节点同级）！
                //printf("%p %p\n", tmp_node, tmp_doc);
                //assert(tmp_node);
                //assert(tmp_doc);
                this->tmp_node->append_child(tmp_doc->create_text(tk_ini->data));
                tk_ini++;
                break;

            case token_t::token_xml_node_cdata:
                this->tmp_node->append_child(tmp_doc->create_cdata(tk_ini->data));
                tk_ini++;
                break;

            default:
                break;
            }

            return tk_ini;
        }

        // 解析 bom，并返回读取过的字节数；
        int    xml_parser::parse_bom(std::string::const_iterator ini,
                                     std::string::const_iterator fin)
        {
            for (sss::xml::bom_str_table_t::iterator it = sss::xml::bom_str_table.begin();
                 it != sss::xml::bom_str_table.end();
                 ++it)
            {
                int bom_len = it->second.length();
                if (fin >= ini + bom_len) {
                    if (it->second == std::string(ini, ini + bom_len)) {
                        this->bom_str = it->first;
                        //std::cout << "bom name:" << it->first << std::endl;
                        return bom_len;
                    }
                }
                else {
                    continue;
                }
            }
            //std::cout << "bom find failed" << std::endl;
            return 0;
        }
    }
}
