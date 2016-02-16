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

        // ���� xml_str_data �������� tmp_doc ����
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

        // �ȴ����ֽ�����bom
        // Ȼ���ֽ�����ת����token��
        //
        // ��һ��ѭ���������ڵ��Ӧ��ϵ���Ƿ���ȷ
        // ��� info.charset ���� �ṩĬ��ֵ
        // �ڶ���ѭ���������ڵ㣻
        // ��� root �Ƿ�Ψһ��
        //
        //! �����ǽ���xml�����̣���ʵ����xhtml�Ļ���Ҳ��������������̣�
        // ������info �ڵ�ļ�⣬��Ҫ��С�޸ģ�Ӧ��������ж�����ڵ�Ĵ�����
        // ���Լ����ֵ�λ�úϲ����ʡ�
        //
        // �磺
        // �ȴ����ֽ�����bom
        // Ȼ���ֽ�����ת����token��
        //
        // ��һ��ѭ���������ڵ��Ӧ��ϵ���Ƿ���ȷ
        // ��� doctype
        // �ڶ���ѭ���������ڵ㣻
        // ��� root �Ƿ�Ψһ��
        // ���doctype �����ڣ����ṩһ��Ĭ��ֵ��
        // ���doctype λ�ò��ԣ�����Ų����ʼ��
        //
        //! �����˵�������Ϸ��Լ�⣬Ӧ��Ų��ʵ�����Ĺ��캯�������
        //����Ȼ��Ҳ���Խ���һ����validata�ĳ�Ա������
        // �����Ļ���parser�����������ԵñȽϼ�ࣻ

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
                // FIXME ��� text ���ֵ�λ�õĺϷ��ԣ����磬��������ڵ�ͬ������
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

        // ���� bom�������ض�ȡ�����ֽ�����
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
