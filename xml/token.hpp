#ifndef  __TOKEN_HPP_1401003567__
#define  __TOKEN_HPP_1401003567__

#include <string>
#include <vector>
#include <iostream>
#include <map>

namespace sss {
    namespace xml {
        struct token_t {
        public:
            enum token_type_t {
                token_xml_nill = 0,
                token_xml_info,                 // <? .{-} ?>
                token_xml_comment,              // <!-- .{-} -->
                token_xml_node_begin,           // <\w\+> ��ʱ��֧������
                token_xml_node_end,             // </\w\+>
                token_xml_node_self,            // </\w\+/>
                token_xml_line_text,            // text�Գ�һ�У�
                token_xml_node_cdata,           // <!CDATA[ ... ]]>
                token_xml_doctype,              // html: <!DOCTYPE ... >
                token_xml_space                 // �հ��ַ�-�б�Ҫ��
            };

        public:
            token_t(token_type_t, const std::string&, int, int);
            token_t();
            ~token_t();

        public:
            void print(std::ostream&) const;

            // ��ȡtoken�ַ��������ֲ����Ӵ�
            const std::string& get_name() const;

            // ��ȡtoken�ַ��������Բ����Ӵ�
            const std::string& get_properties_str() const;

        public:
            token_type_t        token_type;
            std::string         data;
            std::string         name;
            std::string         properties_str;
            int                 row;
            int                 col;
        };

        typedef std::vector<token_t> tokens_t;

        inline std::ostream& operator << (std::ostream& out, const token_t& token)
        {
            token.print(out);
            return out;
        }

        class tokenizer
        {
        public:
            tokenizer(const std::string&);
            tokenizer(std::string::const_iterator,
                      std::string::const_iterator);
            ~tokenizer();

        public:
            // ��������ɹ�������ȡ����tokenд�봫�����;
            // ������true;
            // ��֮������false;
            bool fetch(token_t&);

        protected:
            // ���Զ�ȡ���ɹ��Ļ�����true�����޸��ڲ����ԣ���֮����false;
            bool test_xml_info();       // <?xml .... ?>
            bool test_xml_doctype();    // <!DOCTYPE ... >
            bool test_xml_comment();    // <!-- ... -->
            bool test_xml_node_begin(); // <html xmlns="http://www.w3.org/1999/xhtml">
            bool test_xml_node_end();   // </html>
            bool test_xml_node_text();  // TEXT
            bool test_xml_node_cdata(); // <[CDATA[ TEXT ]]>

            bool test_xml_space();

            void move_next(std::string::const_iterator,
                           std::string::const_iterator);

        private:
            std::string::const_iterator ini;
            std::string::const_iterator fin;

            token_t::token_type_t token_type;
            std::string data;
            //properties_t properties;
            int  row;
            int  col;
        };
    }
}



#endif  /* __TOKEN_HPP_1401003567__ */
