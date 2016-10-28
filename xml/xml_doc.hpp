#ifndef  __XML_DOC_HPP_1401552796__
#define  __XML_DOC_HPP_1401552796__

#include "xml_node.hpp"

#include <string>

#include <sss/dom/html_util.hpp>
#include <sss/iConvpp.hpp>

#include <iostream>
#include <map>

// NOTE xml_doc 的行为，大部分，与 xml::node 相同！
// 到底要不要区分？xml_doc 节点 与普通节点？
//
// TODO list
// 1. 编码转换；类比vim；内部编码，文件编码；
// 2. BOM 是否需要提供？
//
// ZenCoding 范例
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
// NOTE 上述 ZenCoding 在展开之后，先生成一个xhtml 头；然后内部节点都生成在
// html>body下面；
// 首先是 div#header；然后是其内部的两个兄弟节点：div#logo 和 ul#nav；
// 最后，在ul#nav里面，塞了5个class名字，依此变化的li元素；并且每个li中，都含有
// 一个a节点；
//
// 支持属性：
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
//    <LOCATION>阴凉库</LOCATION>
//  </RECORD>
//</UPLOAD>

//NOTE 如何处理文本中的特殊符号；
//
// 貌似，节点的属性、节点内容，里面

namespace sss{
    namespace xml{

        typedef std::map<std::string, std::string> bom_str_table_t;

        extern bom_str_table_t bom_str_table;

        class xml_doc : public node {

        public:
            // 创建空的xml文档对象
            xml_doc();

            // 创建字符集为charset的xml文档对象
            // 同时设置是否打印bom――NOTE bom只在打印的时候有效。
            xml_doc(const std::string& charset,
                    int has_bom = false);

            // 创建根节点为root_name的xml文档对象
            // 其余同上；
            xml_doc(const std::string& root_name,
                    const std::string& charset,
                    int has_bom = false);

            // ……
            virtual ~xml_doc();

        public:
            virtual node * root() const;
            virtual node * root(const std::string&);

            // 获取根节点名字
            virtual std::string root_name() const;

            virtual std::string set_charset(const std::string& );

            virtual std::string get_charset();

            virtual bool        validate() const;

        protected:

            // 确保描述charset的节点存在于根节点之前！
            node      * verify_charset_node();

        public:
            void print(std::ostream&, const char * sep = "\t") const;

            // 输出到外部文件；
            // 此时，会调用编码转换
            void write(const std::string&, const char * sep = "\t");

            // 从外部文件载入
            void load(const std::string& );

            // 写到外部文件中
            void save(const std::string& );

            void clear();

            // 节点生成；
            node_text * create_text(const std::string& text);
            node *      create_node(const std::string& name);
            node *      create_comment(const std::string& data);
            node *      create_info(const std::string& data);
            node *      create_doctype(const std::string& data);
            node *      create_cdata(const std::string& data);
            // TODO 可以加入节点的值；用圆括号来代替；
            node_list_t create_zencoding(const std::string& script);

            // 用文本序列，挨个填充文档中，某节点以下的叶子节点；
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
            //NOTE "根节点"
            // xml_doc 对象本身并不是根节点；根节点是其直接的，唯一一个type_node
            // 类型的子节点；（允许额外有type_comment节点）

            bool is_bom_enable;
            // 内存编码；为了方便与其他部分交换，其默认值为cp936，即gb2312
            std::string encoding;

            // 文件编码；常见的为utf8
            std::string fencoding;

            sss::iConv icv;
        };
    }
}


#endif  /* __XML_DOC_HPP_1401552796__ */
