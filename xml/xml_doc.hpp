#ifndef  __XML_DOC_HPP_1401552796__
#define  __XML_DOC_HPP_1401552796__

#include "xml_node.hpp"

#include <string>

#include <sss/dom/html_util.hpp>
#include <sss/iConvpp.hpp>

#include <iostream>
#include <map>

// NOTE xml_doc ����Ϊ���󲿷֣��� xml::node ��ͬ��
// ����Ҫ��Ҫ���֣�xml_doc �ڵ� ����ͨ�ڵ㣿
//
// TODO list
// 1. ����ת�������vim���ڲ����룬�ļ����룻
// 2. BOM �Ƿ���Ҫ�ṩ��
//
// ZenCoding ����
//
// html:xt>div#header>div#logo+ul#nav>li.item-$*5>a
//
// <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
//                       "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
// <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
// <head>
//      <title></title>
//      <meta http-equiv="Content-Type" content="text/html;charset=UTF-8" />
// </head>
// <body>
//      <div id="header">
//              <div id="logo"></div>
//              <ul id="nav">
//                      <li class="item-1"><a href=""></a></li>
//                      <li class="item-2"><a href=""></a></li>
//                      <li class="item-3"><a href=""></a></li>
//                      <li class="item-4"><a href=""></a></li>
//                      <li class="item-5"><a href=""></a></li>
//              </ul>
//      </div>
// </body>
// </html>
//
// NOTE ���� ZenCoding ��չ��֮��������һ��xhtml ͷ��Ȼ���ڲ��ڵ㶼������
// html>body���棻
// ������ div#header��Ȼ�������ڲ��������ֵܽڵ㣺div#logo �� ul#nav��
// �����ul#nav���棬����5��class���֣����˱仯��liԪ�أ�����ÿ��li�У�������
// һ��a�ڵ㣻
//
// ֧�����ԣ�
//
// html>head+body>div.test_div>ul>li.list-item*10>a[href="#"]>p>span[style="color:red;"]

//
//<?xml version="1.0" encoding="utf-8" ?>
//<UPLOAD>
//  <ENTERPRISE>
//    <CODE>JY33030002</CODE>
//    <FILETYPENUMBER>AddDevice-JY33030002-20140519103136</FILETYPENUMBER>
//    <FILECREATTIME>2014-05-19 10:31:36</FILECREATTIME>
//  </ENTERPRISE>
//  <RECORD>
//    <ID>JY33030002-4</ID>
//    <WAREHOUSEID>JY33030002-02</WAREHOUSEID>
//    <LOCATION>������</LOCATION>
//  </RECORD>
//</UPLOAD>

//NOTE ��δ����ı��е�������ţ�
//
// ò�ƣ��ڵ�����ԡ��ڵ����ݣ�����

namespace sss{
    namespace xml{

        typedef std::map<std::string, std::string> bom_str_table_t;

        extern bom_str_table_t bom_str_table;

        class xml_doc : public node {

        public:
            // �����յ�xml�ĵ�����
            xml_doc();

            // �����ַ���Ϊcharset��xml�ĵ�����
            // ͬʱ�����Ƿ��ӡbom����NOTE bomֻ�ڴ�ӡ��ʱ����Ч��
            xml_doc(const std::string& charset,
                    int has_bom = false);

            // �������ڵ�Ϊroot_name��xml�ĵ�����
            // ����ͬ�ϣ�
            xml_doc(const std::string& root_name,
                    const std::string& charset,
                    int has_bom = false);

            // ����
            virtual ~xml_doc();

        public:
            virtual node * root() const;
            virtual node * root(const std::string&);

            // ��ȡ���ڵ�����
            virtual std::string root_name() const;

            virtual std::string set_charset(const std::string& );

            virtual std::string get_charset();

            virtual bool        validate() const;

        protected:

            // ȷ������charset�Ľڵ�����ڸ��ڵ�֮ǰ��
            node      * verify_charset_node();

        public:
            void print(std::ostream&, const char * sep = "\t") const;

            // ������ⲿ�ļ���
            // ��ʱ������ñ���ת��
            void write(const std::string&, const char * sep = "\t");

            // ���ⲿ�ļ�����
            void load(const std::string& );

            // д���ⲿ�ļ���
            void save(const std::string& );

            void clear();

            // �ڵ����ɣ�
            node_text * create_text(const std::string& text);
            node *      create_node(const std::string& name);
            node *      create_comment(const std::string& data);
            node *      create_info(const std::string& data);
            node *      create_doctype(const std::string& data);
            node *      create_cdata(const std::string& data);
            // TODO ���Լ���ڵ��ֵ����Բ���������棻
            node_list_t create_zencoding(const std::string& script);

            // ���ı����У���������ĵ��У�ĳ�ڵ����µ�Ҷ�ӽڵ㣻
            template<typename IT>
            int text_fill_empty_node(node * here, IT ini, IT fin)
            {
                node_list_t empty_nodes = here->find_all_empty_node();
                int count = 0;
                for (node_list_t::iterator it = empty_nodes.begin();
                     it != empty_nodes.end() && ini != fin;
                     ++it, ++ini)
                {
                    std::string text(*ini);
                    (*it)->append_child(this->create_text(text));
                    ++count;
                }
                return count;
            }

        public:
            static std::string safe_xml_entites(const std::string& raw_string);

        private:
            //NOTE "���ڵ�"
            // xml_doc ���������Ǹ��ڵ㣻���ڵ�����ֱ�ӵģ�Ψһһ��type_node
            // ���͵��ӽڵ㣻�����������type_comment�ڵ㣩

            bool is_bom_enable;
            // �ڴ���룻Ϊ�˷������������ֽ�������Ĭ��ֵΪcp936����gb2312
            std::string encoding;

            // �ļ����룻������Ϊutf8
            std::string fencoding;

            sss::iConv icv;
        };
    }
}


#endif  /* __XML_DOC_HPP_1401552796__ */
