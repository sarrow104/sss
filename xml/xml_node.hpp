#ifndef  __XML_GEN_HPP_1400471032__
#define  __XML_GEN_HPP_1400471032__

#include <sss/dom/html_util.hpp>

#include <vector>
#include <string>
#include <map>
#include <iostream>

namespace sss{
    namespace xml{

        // 这些节点有各种属性；最简单的，有是否允许子节点！
        // 如下：奇数标识允许子节点；偶数，则不允许；
        // 还有，是否支持设定属性
        enum node_type_t {                              // 子节点       属性
            type_node           = 0 * 4 + 1 + 2,        // Y            Y
            type_text           = 1 * 4 + 0 + 0,        // N            N
            type_info           = 2 * 4 + 0 + 2,        // N            Y
            type_doctype        = 3 * 4 + 0 + 0,        // N            N
            type_comment        = 4 * 4 + 0 + 0,        // N            N
            type_cdata          = 5 * 4 + 0 + 0,        // N            N
            type_xmldoc         = 6 * 4 + 1 + 2,        // Y            Y
            type_xhtmldoc       = 7 * 4 + 1 + 2         // Y            Y
        };

        //typedef std::map<std::string, std::string> properties_t;

        class properties_t : public std::map<std::string, std::string> {
        public:
            properties_t();
            properties_t(const std::string& properties_str);
            ~properties_t();

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

        class node{
            friend class xml_doc;
            friend class xmlcfg_file;

        protected:
            node(const std::string&, xml_doc *, node_type_t = type_node);
            virtual ~node();

        public:
            virtual node * first_child() const;
            virtual node * last_child() const;

            virtual node * insert_before(node*, node* = 0);
            virtual node * insert_before(const node_list_t& , node* = 0);

            virtual node * append_child(node* );

            // 连续添加多个子节点
            virtual node * append_child(const node_list_t& );

            virtual void clear();

            virtual std::string text() const;
            virtual std::string text(const std::string&);

            // 获取内部文本标识
            virtual std::string inner_text() const;
            // 获取内部xml文本标识
            virtual std::string inner_xml() const;

        protected:
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

            virtual void        set(const std::string& key, const std::string& value);
            virtual std::string get(const std::string& key) const;

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
            virtual std::string text(const std::string&);

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
            virtual std::string text(const std::string&);

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
