#ifndef  __XMLCFG_FILE_HPP_1414586840__
#define  __XMLCFG_FILE_HPP_1414586840__

#include "xml_node.hpp"
#include "xml_doc.hpp"
#include "xml_handle.hpp"

#include <sss/util/Memory.hpp>
#include <sss/utlstring.hpp>

#include <string>
#include <sstream>
#include <memory>


// TODO
// xmlcfg_file 应该作为sss::xml3::node * 的一个handle来使用；并且，提供了一些适用的sugar 方法：
// xmlkey root("some_xml_file");
//
// xmlkey stem = root.locate(path_stem);        // 定位到某一个具体的节点；
// int size = stem.size();                      // 获取其大小；
// for (xmlkey::iterator it = stem.begin(); it != stem.end(); ++it) {
//   ...
// }
// root.get<std::string>("some>path>to>some_node")
//
// 要提供上面的语法糖，就意味着，我需要处理资源管理上的问题；选项有2：
//
//   1. 支持引用计数的智能指针；
//
//   2. 放弃资源管理，xmlkey仅作为一个包裹类存在；
//
// TODO 支持，修改xml元素吗？

namespace sss {
    namespace xml3 {
        // NOTE 自定义迭代器，可以查看：
        // http://bbs.csdn.net/topics/100134636
        class xmlkey    // NOTE 这实际上是一个指针的wrapper；并且，他并不管理资源；
        {
        public:
            explicit xmlkey(sss::xml3::node * p)
                : p_node(p)
            {
            }
            ~xmlkey()
            {
            }

            operator void*()
            {
                return p_node;
            }

            operator sss::xml3::node& ()
            {
                return *p_node;
            }

            std::string inner_text() const
            {
                return p_node->inner_text();
            }

            std::string inner_xml() const
            {
                return p_node->inner_xml();
            }

            std::string key() const
            {
                return p_node->get_data();
            }

            std::string value() const
            {
                return p_node->inner_text();
            }

            xmlkey locate(const std::string& path)
            {
                return xmlkey(this->p_node->locate(path));
            }

        public:
            template<typename T>
                T get(const std::string& path)
                {
                    try {
                        T value;
                        sss::xml3::node * p_path = p_node->locate(path);
                        if (p_path) {
                            std::istringstream iss(p_path->inner_text());
                            iss >> value;
                            return value;
                        }
                        return T();
                    }
                    catch (...) {
                        return T();
                    }
                }

                std::string get(const std::string& path)
                {
                    try {
                        sss::xml3::node * p_path = p_node->locate(path);
                        if (p_path) {
                            return p_path->inner_text();
                        }
                        return "";
                    }
                    catch (...) {
                        return "";
                    }
                }

        public:
            class iterator
            {
            private:
                sss::xml3::node * index;
                sss::xml3::node * p_node;

            public:
                iterator(const iterator& ref_key)
                    : index(ref_key.index), p_node(ref_key.p_node)
                {
                }

                iterator(sss::xml3::node * p, bool is_begin)
                    : p_node(p)
                {
                    if (is_begin) {
                        index = p_node->firstChild();
                    }
                    else {
                        index = 0;
                    }
                }

                ~iterator()
                {
                }

            public:
                iterator& operator--()
                {
                    if (index) {
                        index = index->prevSibling();
                    }
                    return *this;
                }
                iterator operator--(int)
                {
                    iterator ret(*this);
                    if (index) {
                        index = index->prevSibling();
                    }
                    return ret;
                }

                iterator& operator++()
                {
                    if (index) {
                        index = index->nextSibling();
                    }
                    return *this;
                }
                iterator operator++(int)
                {
                    iterator ret(*this);
                    if (index) {
                        index = index->nextSibling();
                    }
                    return ret;
                }

                xmlkey operator*() const
                {
                    return xmlkey(index);
                }

                bool operator ==(const iterator& i) const
                {
                    return this->p_node == i.p_node && this->index == i.index;
                }

                bool operator !=(const iterator& i) const
                {
                    return !(*this == i);
                }
            };

            iterator begin() const
            {
                return iterator(this->p_node, true);
            }

            iterator end() const
            {
                return iterator(this->p_node, false);
            }

        private:
            sss::xml3::node * p_node;
        };

        class xmlcfg_file
        {
        public:
            typedef sss::xml3::xmlkey::iterator iterator;

        public:
            xmlcfg_file();
            explicit xmlcfg_file(const std::string&);
            ~xmlcfg_file();

        public:
            template<typename T> T get(const std::string& path)
            {
                try {
                    T value;
                    sss::xml3::node * p_node = my_doc->locate(path);
                    if (p_node) {
                        std::istringstream iss(p_node->inner_text());
                        iss >> value;
                        return value;
                    }
                    return T();
                }
                catch (...) {
                    return T();
                }
            }

            inline std::string get(const std::string& path)
            {
                try {
                    sss::xml3::node * p_node = my_doc->locate(path);
                    if (p_node) {
                        return p_node->inner_text();
                    }
                    return "";
                }
                catch (...) {
                    return "";
                }
            }

            inline std::string get_string(const std::string& path)
            {
                try {
                    sss::xml3::node * p_node = my_doc->locate(path);
                    if (p_node) {
                        return p_node->inner_text();
                    }
                    return "";
                }
                catch (...) {
                    return "";
                }
            }

            inline std::string set_string(const std::string& path, const std::string& value)
            {
                try {
                    std::string ret;
                    sss::xml3::node * p_node = my_doc->verify_path(path);
                    if (p_node) {
                        ret = p_node->inner_text();
                        p_node->text(value);
                        this->mark_modified();
                    }
                    return ret;
                }
                catch (...) {
                    return "";
                }
            }

            inline std::string add_string(const std::string& path, const std::string& value)
            {
                try {
                    sss::xml3::node * p_node = my_doc->verify_path(path);
                    if (p_node) {
                        sss::scoped_ptr<sss::xml3::node> text(this->my_doc->create_text(value));
                        if (text.get()) {
                            // NOTE
                            // 对于我这种依赖于指针的树形结构，除非添加的位置或者id有冲突，不然
                            // append_child动作肯定会成功——其中没有内存的分配
                            // 就是说，虽然没有异常，但是，我还是应当检测一下返
                            // 回值，以便确定指针对象，正确移动过去了——
                            //
                            // 然后，保证在失败的时候，被release掉！
                            //
                            // 当然，最完美的方案，还是使用 operator new，让
                            // xml-doc对象，直接接管create的元素！
                            //
                            // 简单来说，直接relase，是不可取的……
                            // 指针丢失了……
                            //
                            // 当然，补救的办法是，一旦失败，直接释放该对象——
                            // 但是，这样就无法debug了。
                            //
                            // 因为对象丢失。
                            p_node->append_child(text.get());
                            text.release();
                            this->mark_modified();
                        }
                    }
                    return value;
                }
                catch (...) {
                    return "";
                }
            }

            template<typename T> std::string set(const std::string& path, const T& value)
            {
                try {
                    std::string ret;
                    sss::xml3::node * p_node = my_doc->verify_path(path);
                    if (p_node) {
                        ret = p_node->inner_text();
                        p_node->text(sss::cast_string(value));
                        this->mark_modified();
                    }
                    return ret;
                }
                catch (...) {
                    return "";
                }
            }

            inline std::string set_text(const std::string& path, const std::string& value)
            {
                return this->set_string(path, value);
            }

            inline std::string add_text(const std::string& path, const std::string& value)
            {
                return this->add_string(path, value);
            }

            inline std::string set_cdata(const std::string& path, const std::string& value)
            {
                try {
                    std::string ret;
                    sss::xml3::node * p_node = my_doc->verify_path(path);
                    if (p_node) {
                        ret = p_node->inner_text();
                        sss::scoped_ptr<sss::xml3::node> cdata(this->my_doc->create_cdata(value));
                        if (cdata.get()) {
                            p_node->clear();
                            p_node->append_child(cdata.get());
                            cdata.release();
                        }
                        this->mark_modified();
                    }
                    return ret;
                }
                catch (...) {
                    return "";
                }
            }

            inline std::string add_cdata(const std::string& path, const std::string& value)
            {
                try {
                    sss::xml3::node * p_node = my_doc->verify_path(path);
                    if (p_node) {
                        sss::scoped_ptr<sss::xml3::node> cdata(this->my_doc->create_cdata(value));
                        if (cdata.get()) {
                            p_node->append_child(cdata.get());
                            cdata.release();
                        }
                        this->mark_modified();
                    }
                    return value;
                }
                catch (...) {
                    return "";
                }
            }

            // NOTE 本函数的目的是，解析xml_str字符串，并生成在path下面；
            // 但是，我的xml_parser对象的工作模式，与这里的需求相悖；
            // 它是解析一个字符串，然后替换掉传入xml_doc对象；
            // 最好，给 sss::xml3::node 再添加一个parser方法，让能够想
            // create_zencoding函数一样，生成一系列节点。
            //
            inline std::string set_subxml(const std::string& path, const std::string& xml_str)
            {
                try {
                    std::string ret;
                    sss::xml3::node * p_node = my_doc->verify_path(path);
                    if (p_node) {
                        ret = p_node->inner_text();
                        sss::scoped_ptr<sss::xml3::node> sub(my_doc->create_subtree(xml_str));
                        if (sub.get()) {
                            p_node->clear();
                            p_node->append_child(sub.get());
                            sub.release();
                        }

                        this->mark_modified();
                    }
                    return ret;
                }
                catch (...) {
                    return "";
                }
            }

            inline std::string add_subxml(const std::string& path, const std::string& xml_str)
            {
                try {
                    sss::xml3::node * p_node = my_doc->verify_path(path);
                    if (p_node) {
                        sss::scoped_ptr<sss::xml3::node> sub(my_doc->create_subtree(xml_str));
                        if (sub.get()) {
                            p_node->append_child(sub.get());
                            sub.release();
                        }

                        this->mark_modified();
                    }
                    return xml_str;
                }
                catch (...) {
                    return "";
                }
            }

            int subnode_count(const std::string& path);

            iterator sub_begin(const std::string& path);
            iterator sub_end(const std::string& path);

            void print(std::ostream&);

            void update();

            void write(const std::string& name);

            sss::xml3::xml_doc& get_xmldoc();

            inline void mark_modified()
            {
                this->is_modified = true;
            }

        private:
            sss::xml3::DocHandle my_doc;
            std::string fname;
            bool is_modified;
        };
    }
}

#endif  /* __XMLCFG_FILE_HPP_1414586840__ */
