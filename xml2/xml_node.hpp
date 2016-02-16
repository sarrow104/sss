#ifndef  __XML_GEN_HPP_1400471032__
#define  __XML_GEN_HPP_1400471032__

#include <sss/dom/html_util.hpp>
#include <sss/util/Memory.hpp>

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <memory>

namespace sss{
    namespace xml2{

        // 这些节点有各种属性；最简单的，有是否允许子节点！
        // 如下：奇数标识允许子节点；偶数，则不允许；
        // 还有，是否支持设定属性
        enum node_type_t {                  // 子节点 属性 Sample|Type
            type_node      = 0 * 4 + 1 + 2, // Y      Y    <a></a> | element 附带……
            type_text      = 1 * 4 + 0 + 0, // N      N    #PCDATA
            type_info      = 2 * 4 + 0 + 2, // N      Y    <?xml ... ?> | XMLDeclaration
            type_doctype   = 3 * 4 + 0 + 0, // N      N    <!DOCTYPE ...>
            type_comment   = 4 * 4 + 0 + 0, // N      N    <!-- -->
            type_cdata     = 5 * 4 + 0 + 0, // N      N    <[CDATA[ ...]]>
            type_xmldoc    = 6 * 4 + 1 + 2, // Y      Y    --取消 ——并非实际节点，而是一种包装
            type_xhtmldoc  = 7 * 4 + 1 + 2, // Y      Y    --取消 xhtml 本质是xml，所以没有存在必要
            type_PI        = 8 * 4 + 0 + 0, // N      N
            // NOTE DTD树形结构，只存在于type_document下面，所以……
            //
            // PI         ::=   '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
            // 所谓的PI，就是
            //   <?page render multiple authors ?>
            //   <?xml-stylesheet type="text/css" href="http://gnosis.cx/publish/programming/dW.css" ?>
            // 这种东西。是专门给xml解析器看的东西；
            //
            // 详细讨论，见："http://blog.csdn.net/dagouaofei/article/details/5636019"
            // NOTE 鉴于对xml解析器来说 <br/> == <br></br> 所以，并没有单独的
            // EmptyElemTag类型；

            // type_prolog NOTE 貌似允许不定数量的prolog语句！比如：
            //
            // <?xml-stylesheet href="http://www.w3.org/StyleSheets/TR/W3C-REC.css" type="text/css"?>
            // <?xml-stylesheet href="#internalStyle" type="text/css"?>
            // <!DOCTYPE html
            //      PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
            //      "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
            // <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
            // <head>
            // <title>An internal stylesheet example</title>
            // <style type="text/css" id="internalStyle">
            //   code {
            //     color: green;
            //     font-family: monospace;
            //     font-weight: bold;
            //   }
            // </style>
            // </head>
            // <body>
            // <p>
            //   This is text that uses our
            //   <code>internal stylesheet</code>.
            // </p>
            // </body>
            // </html>
            //
            // NOTE 另外，doctypedecl 等东西，需要原样保存——dtd数据结构虽然可
            // 以以之生成，但是没有必要将dtd树，与xml树合并起来表示。
            // 所以 doctypedecl 还是原样保存的好。
            //
            // 另外，就是 node_document 与 node_element 的共存问题。
            // sss::xml1 和 TinyXML2 ，都是 node_document 包裹 node_element 的方式；
            // 而我的新想法是，从node_element开始——此时，就必须用一个树节点来
            // 表示xml——需要注意的是，可以看做是链表。并且，表示root的指针，
            // 不一定指向链表头！然后，每一个节点，都有一个指针，指向node_document。
            //
            // 然后 使用 Seg2XML 函数，完成转化细节；至于递归部分，可以写在
            // SegmentXML::下面。
            //
            // 与之类似，就算是 node_document 作为包裹，root根节点，也不一定是
            // 其第一个child！
        };

        //typedef std::map<std::string, std::string> properties_t;

        class properties_t : private std::vector<std::pair<std::string, std::string> > {
        public:
            typedef std::vector<std::pair<std::string, std::string> > Base_t;
            typedef Base_t::const_iterator const_iterator;
            typedef Base_t::iterator       iterator;
            using Base_t::empty;
            using Base_t::begin;
            using Base_t::end;

        public:
            properties_t();
            properties_t(const std::string& properties_str);
            ~properties_t();

        protected:
            iterator find(const std::string& key);
            const_iterator find(const std::string& key) const;

        public:
            int         parse(const std::string& properties_str);
            void        print(std::ostream& o)  const;
            bool        has_key(const std::string& key) const;
            std::string get(const std::string& key) const;
            void        set(const std::string& key, const std::string& value);
        };

        inline std::ostream& operator<<(std::ostream& o, const properties_t& p)
        {
            p.print(o);
            return o;
        }

        class node;
        typedef std::vector<node*> node_list_t;

        class xml_doc;
        class xmlcfg_file;
        class xml_parser;

        class node{
            friend class xml_doc;
            friend class xmlcfg_file;
            friend class xml_parser;
            friend class sss::scoped_ptr<node>;

        protected:
            node(const std::string&, xml_doc *, node_type_t = type_node);
            virtual ~node();

        public:
            virtual node * first_child() const;
            virtual node * last_child() const;

            virtual node * next_sibling() const;
            virtual node * prev_sibling() const;

            virtual node * insert_before(node*, node* = 0);
            virtual node * insert_before(const node_list_t& , node* = 0);

            virtual node * append_child(node* );

            // 连续添加多个子节点
            virtual node * append_child(const node_list_t& );

            virtual void clear();

            virtual std::string text() const;
            virtual node * text(const std::string&);

            // 获取内部文本标识
            virtual std::string inner_text() const;
            // 获取内部xml文本标识
            virtual std::string inner_xml() const;

        public:
            // 克隆当前节点并返回——
            // 参数：是否深度拷贝
            virtual node * clone(bool is_deep) const;

        public:
            void print_text(std::ostream&, const char * sep = "\t") const;

            void print(std::ostream&, const char * sep = "\t") const;

            std::string get_data() const;

            std::string set_data(const std::string&);

            xml_doc * get_doc() const;
            node *    get_parent() const;

            // 子节点数量
            size_t   size() const;

            bool   is_text_container() const;

            // 按照"仿CSS定位器-路径"定位节点；
            // 不存在，则返回0；
            node * locate(const std::string& path);

            node * verify_path(const std::string& path);

            // 交换两个节点各自的父亲、孩子；要求，
            // 必须属于同一个doc！
            //
            // 至于两个节点是否是各自的祖先，没有关系；
            // 注意，虽然没有限制，但是其中一个节点，最好不要是doc对象！
            bool swap(node * p);

            void enumrate_subnode() const;

            const char * get_node_type_name() const;

            // 返回节点名符合要求的节点列表
            // 如果name == "*" 则查找所有名字的节点
            //
            // TODO
            //
            // 1. 根据class、id名作为查找条件；
            //
            // 2. 也应该引入"仿CSS定位器"的概念
            //    不过，加入定位器，意义不大；因为大部分的需求，都可以用locate()
            //    动作和 find_all() 两者协同完成；
            node_list_t find_all(const std::string& name) const;

            node_list_t find_all_leaves() const;

            node_list_t find_all_empty_node() const;

            // 在子节点中，查找名字符合要求的节点；
            // 类上
            node_list_t find_child(const std::string& name) const;

            // 查找子节点中，第index个，名字符合要求的节点
            // NOTE *index* start from Zero 0 !
            node * find_child(const std::string& name,
                              int index) const;

            node_list_t& get_node_list();

            // 返回当前节点类型
            node_type_t type() const;

            // 当前节点，是否是 node_text;
            bool is_node_text() const;

            bool is_node_cdata() const;

            bool is_empty_node() const;

            bool is_leaf() const;

            // 当前节点，是否是名为name的普通节点
            bool is_node(const std::string& name) const;

            // 当前节点，是否是普通节点
            bool is_node() const;

            bool is_properties_able() const;

            int  parse_properties_str(const std::string& properties_str);

            virtual node *      set(const std::string& key, const std::string& value);
            virtual std::string get(const std::string& key) const;
            virtual bool        has_key(const std::string& key) const;

        protected:
            // 打印-缩进控制
            virtual void print_impl(std::ostream&,
                                    html_util::indent_wraper&) const;

            virtual void print_text_impl(std::ostream&,
                                         html_util::indent_wraper&) const;
        protected:
            // 以子节点身份，加入对方节点下面
            void join_family(node*);

            // implementation for "find_all"
            void find_all_impl(const std::string& name, node_list_t & ret) const;

            void find_all_leaves_impl(node_list_t & ret) const;

            void find_all_empty_node_impl(node_list_t & ret) const;

        public:
            properties_t properties;    // 属性对象

        protected:
            node_list_t node_list;      // 子节点
            std::string data;           // 内部数据；根据节点类型不同，而不同
            xml_doc *   p_doc;          // 指向节点所属的xml_doc对象
            node *      p_parent;       // 指向父节点——根节点该属性，应该为0
            node_type_t node_type;      // 节点类型；
        };

        inline std::ostream & operator << (std::ostream & o, const node& xml)
        {
            xml.print(o);
            return o;
        }

        class node_info : public node {
            friend class xml_doc;

        protected:
            node_info(const std::string& name, xml_doc *);
            ~node_info();

        public:
            void print_impl(std::ostream&, html_util::indent_wraper&) const;
            node * clone(bool ) const;

        private:
        };

        class node_doctype : public node {
            friend class xml_doc;

        protected:
            node_doctype(const std::string& data, xml_doc *);
            ~node_doctype();

        public:
            void print_impl(std::ostream&, html_util::indent_wraper&) const;
            node * clone(bool ) const;

        private:
        };

        // 注释 - 行为与node_text 类似
        class node_comment : public node {
            friend class xml_doc;

        protected:
            node_comment(const std::string&, xml_doc *);
            ~node_comment();

        protected:

        public:
            void print_impl(std::ostream& out,
                            html_util::indent_wraper&) const;
            node * clone(bool ) const;
        };

        class node_text : public node{
            friend class xml_doc;

        protected:
            node_text(const std::string&, xml_doc *);
            ~node_text();

            virtual std::string text() const;
            virtual node * text(const std::string&);

        protected:

        public:

            void print_impl(std::ostream& out,
                            html_util::indent_wraper&) const;
            node * clone(bool ) const;

        private:
        };

        class node_cdata : public node {
            friend class xml_doc;

        protected:
            node_cdata(const std::string&, xml_doc *);
            ~node_cdata();

            virtual std::string text() const;
            virtual node * text(const std::string&);

        protected:

        public:
            void print_impl(std::ostream& out,
                            html_util::indent_wraper&) const;
            node * clone(bool ) const;

        private:
        };

        // xml:Processing Instruction
        // 根据产生式：
        // PI      ::=   '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
        // 它分为两部分，PITarget，以及具体的指令；指令部分允许为空；
        // 或者说，指令内容部分，要么没有，要么以空白符开头。
        // 我将必要的PITarget 存放到 node::data下面；指令部分，则自己存放；
        // 算了，还是整个都交给根存放吧！除了 <? 和 ?>
        class node_PI : public node {
            friend class xml_doc;

        protected:
            node_PI(const std::string&, xml_doc *);
            ~node_PI();

        protected:

        public:
            void print_impl(std::ostream& out,
                            html_util::indent_wraper&) const;
            node * clone(bool ) const;

        private:
        };
    }   // end of xml
} // end of sss

#endif  /* __XML_GEN_HPP_1400471032__ */
