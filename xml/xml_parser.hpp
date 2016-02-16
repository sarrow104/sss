#ifndef  __XML_PARSER_HPP_1400994344__
#define  __XML_PARSER_HPP_1400994344__

#include "xml_node.hpp"
#include "xml_doc.hpp"
#include "token.hpp"

#include <string>
#include <vector>
#include <iostream>

// 解析xml文件；并生成一个xml_doc对象或者xml::node * 对象；
namespace sss {
    namespace xml {

        class xml_parser {
            // 本质上，是将传入的字符串，解释、并分析，最后组建一个xml::node 对
            // 象；
            // NOTE 可以是xml_doc类型；
            // 那么，就需要进行分析；
            //
            // 首先，分析是否是bom；
            // 此时，需要预读3个字节；以判断bom；
            // 然后，尝试读取<?..?>标记；（以"<?"开头，并且以"?>"结尾，忽略空白字符）
            // 接着，就开始解析<...>标签；此处，使用递归完成即可；

        public:
            xml_parser();
            ~xml_parser();

        public:
            // 将传入的字符串，按xml方式解析，并生成到 传入的 xml_doc 对象上
            // 并返回该对象
            xml::xml_doc * parse(xml::xml_doc *,
                                 std::string::const_iterator s_ini,
                                 std::string::const_iterator s_fin);

            xml::xml_doc * parse(xml::xml_doc *,
                                 const std::string&);

        protected:
            // 解析bom文件头；返回消耗的字节数
            int    parse_bom(std::string::const_iterator,
                             std::string::const_iterator);

            // 将xml字串序列，解析为token序列
            int    tokens_assign(tokens_t&,
                                 std::string::const_iterator s_ini,
                                 std::string::const_iterator s_fin);

            // 避开无用的token
            tokens_t::const_iterator skip_useless_token(tokens_t::const_iterator,
                                                        tokens_t::const_iterator);

            // token pair check -- 使用 stack 配对测试：
            void   tokens_pair_node_check(); // throw

            // 分析标记序列，并生成节点序列
            tokens_t::const_iterator parse_impl(tokens_t::const_iterator,
                                                tokens_t::const_iterator);

        private:
            //xml::xml_doc *      tmp_doc;
            xml::node *         tmp_node;
            std::string         bom_str;
            std::string         charset_str;
            tokens_t            tokens;
            std::vector<std::string> level_stack;
        };
    }
}


#endif  /* __XML_PARSER_HPP_1400994344__ */
