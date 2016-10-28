#ifndef  __XML_DOC_HPP_1401552796__
#define  __XML_DOC_HPP_1401552796__

#include "xml_node.hpp"

#include <string>

#include <sss/dom/html_util.hpp>
#include <sss/iConvpp.hpp>

#include <stdexcept>
#include <assert.h>
#include <iostream>
#include <stdio.h>
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
    namespace xml3{

        typedef std::map<std::string, std::string> bom_str_table_t;

        extern bom_str_table_t bom_str_table;

        class xml_parser;
        class DocHandle;
        class xml_visitor;

        class xml_doc : public node {

            friend class xml_parser;
            friend class DocHandle;
            friend class node;

        public:
            // 创建空的xml文档对象
            xml_doc();

            // 创建字符集为charset的xml文档对象
            // 同时设置是否打印bom——NOTE bom只在打印的时候有效。
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

            node * append_child(node* child);

        protected:
            node * root(node * p)
            {
                if (this->p_root) {
                    printf("doc%p root%p\n",
                           this,
                           this->p_root);
                    std::cout
                        << "xxx" << *p_root << "xxx"
                        << std::endl;
                }
                assert(!this->p_root);
                std::swap(this->p_root, p);
                return p;
            }

        public:
            node * info() const
            {
                return this->p_info;
            }

        protected:
            node * info(node * p)
            {
                assert(!this->p_info);
                std::swap(this->p_info, p);
                return p;
            }

        public:
            node * doctype() const
            {
                return this->p_doctype;
            }

        protected:
            node * doctype(node * p)
            {
                assert(!this->p_doctype);
                std::swap(this->p_doctype, p);
                return p;
            }

        protected:
            // 确保描述charset的节点存在于根节点之前！
            node      * verify_charset_node();

        public:
            // NOTE
            // 克隆的节点，貌似只能用在本地xml-document中！
            virtual node * clone(bool is_deep) const;
            virtual void accept(sss::xml3::xml_visitor& v);

        public:
            // 输出到外部文件；
            // 此时，会调用编码转换
            void write(const std::string&, const char * sep = "\t");

            // 从外部文件载入
            //void load(const std::string& );

            // 写到外部文件中
            void save(const std::string& );

            void clear();

            // 节点生成；
            // is_raw_data
            // 用来控制是否进行 safe_xml_entites 替换的；
            // false 替换； true 原样放行；
            //
            // 为什么？
            // 因为html下的script内容，本质上也是text；但是，却不能进行转义；
            // 只不过，其内部，不能带有</script>文本，就算是字符串中，也不行！
            // <html>
            // <head>
            // <title>显示啥？</title>
            // <script>alert("</script>");</script>
            // </head>
            // <body></body>
            // </html>
            //
            // 被浏览器理解为：
            //
            // <html>
            //   <head>
            //     <title>鏄剧ず鍟ワ紵</title>
            //     <script>alert("</script>
            //   </head>
            //   <body>");
            //
            //
            //   </body>
            // </html>
            //
            // 可见，"容错性"高的浏览器，为了让其"自洽"，还特意对源码进行了修改；
            // 即，浏览器，对于html的元素的解析，特别是script，是按照内部是文本
            // 的方式解析的；之后，再将这个文本，放进特殊的环境中解释——js解释
            // 器。
            //
            // 当然，其显示风格是hide……
            //
            node_text * create_text(const std::string& text, bool is_raw_data = false);
            node *      create_node(const std::string& name);
            node *      create_comment(const std::string& data);
            node *      create_info(const std::string& data);
            node *      create_doctype(const std::string& data);
            node *      create_cdata(const std::string& data);
            node *      create_PI(const std::string& xml_str);
            node *      create_subtree(const std::string& name);
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
            // replace all '&' '<' '>' in raw_string in copies
            static std::string safe_xml_entites(const std::string& raw_string);

            node * id_get(const std::string& id)
            {
                return this->id_map.get(id);
            }

            void id_add(const std::string& id, node * p)
            {
                try {
                    this->id_map.set(id, p);
                }
                catch (std::runtime_error& e) {
                    this->get_doc()->print(std::cout);
                    if (p) {
                        std::cout << "new:";
                        p->print(std::cout);
                    }
                    if (this->id_get(id)) {
                        std::cout << "old:";
                        this->id_get(id)->print(std::cout);
                    }
                    throw;
                }
            }

            node * id_remove(const std::string& id)
            {
                node * ret = this->id_map.get(id);
                if (ret) {
                    this->id_map.erase(id);
                }
                return ret;
            }

        public:
            class IDmap_t : private std::map<std::string, node *>
            {
            public:
                typedef std::map<std::string, node *> Base_t;
                typedef Base_t::const_iterator const_iterator;
                typedef Base_t::iterator       iterator;

                IDmap_t()
                {
                }
                explicit IDmap_t(xml_doc * pdoc)
                {
                    this->init(pdoc);
                }
                ~IDmap_t()
                {
                }

            public:
                // TODO 2015-11-08
                // node * 需要通过set 来保证唯一性！
                void print(std::ostream& o) const
                {
                    o << "{";
                    for (Base_t::const_iterator it = this->Base_t::begin();
                         it != this->Base_t::end();
                         ++it)
                    {
                        o << it->first << ":`" << *it->second << "`" << std::endl;
                    }
                    o << "}";
                }

                node * get(const std::string & id)
                {
                    iterator it = this->Base_t::find(id);
                    if (it != this->Base_t::end()) {
                        return it->second;
                    }
                    else {
                        return 0;
                    }
                }

                void set(const std::string & id, node * p)
                {
                    if (!p) {
                        throw std::runtime_error("try to add NULL node with id `" + id + "`");
                    }
                    iterator it = this->Base_t::find(id);
                    if (it != this->Base_t::end()) {
                        throw std::runtime_error("duplicate xml3::node id `" + id + "`");
                    }
                    this->Base_t::insert(std::make_pair(id, p));
                }

                void erase(const std::string & id)
                {
                    iterator it = this->Base_t::find(id);
                    if (it != this->Base_t::end()) {
                        this->Base_t::erase(it);
                    }
                }

                void swap(IDmap_t & ref)
                {
                    if (this != &ref)
                    {
                        std::swap(static_cast<Base_t&>(*this), static_cast<Base_t&>(ref));
                    }
                }

                void init(xml_doc * pdoc)
                {
                    if (pdoc) {
                        // TODO
                        // 清空当前数据；然后从pdoc 的所有节点中，记录id属性以及对应节点位置；
                        this->Base_t::clear();
                        this->init_impl(pdoc->root());
                    }
                }

                void init_impl(node * cur)
                {
                    if (cur && cur->is_node() && cur->has_key("id")) {
                        // FIXME NOTE xml dtd 中，属性的id值，是一种特殊的值！
                        this->set(cur->get("id"), cur);
                        for (sss::xml3::node * i = cur->firstChild(); i; i = i->nextSibling()) {
                            this->init_impl(i);
                        }
                    }
                }

                // NOTE FIXME 这个数据结构，不支持复制！
                // 这个数据结构，应该需要重建的方式才行啊！
            private:
                IDmap_t(const IDmap_t& ref);
            };

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

            node *                p_info;         // info节点
            node *                p_doctype;      // doctype节点
            node *                p_root;         // 根节点

        private:
            IDmap_t               id_map;
        };
    }
}


// TODO 2015-07-30
// node::set(), get() 也应该通过p_doc监管起来；以便将设计id的赋值、取消（unset)
// ；节点的删除，也体现在其管理的数据结构上。
//
// FIXME xml_doc 对象本身，不能支持 set() ，get() 操作！


#endif  /* __XML_DOC_HPP_1401552796__ */
