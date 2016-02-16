#ifndef  __XML_HANDLE_HPP_1438236519__
#define  __XML_HANDLE_HPP_1438236519__

#include "xml_node.hpp"
#include "xml_doc.hpp"
#include "selector_parser.hpp"
#include "xml_selector.hpp"

#include <sss/utlstring.hpp>
#include <sss/util/Memory.hpp>
#include <sss/log.hpp>

#include <memory>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

#include <assert.h>

namespace sss{
    namespace xml3 {
        class xml_doc;
        class node;
        class EleHandle {
            static const char * D_PREFIX;
        public:
            EleHandle()
                : _data(0)
            {
            }
            explicit EleHandle(node * p)
                : _data(p)
            {
            }
            EleHandle(const EleHandle & ref)
                : _data(ref._data)
            {
            }

        public:
            EleHandle& operator=(const EleHandle & ref)
            {
                if (this != &ref)
                {
                    this->_data = ref._data;
                }
                return *this;
            }

            sss::xml3::xml_doc * getDocument()
            {
                return this->_data ? this->_data->get_doc() : 0;
            }

            EleHandle set(const std::string& path, sss::xml3::node_type_t type, const std::string& value)
            {
                if (!this->_data) {
                    return EleHandle(0);
                }
                else {
                    try {
                        // FIXME
                        EleHandle ret(this->_data->verify_path(path));
                        if (ret.get()) {
                            ret->clear();
                            ret.appendChild(type, value);
                            return ret;
                        }
                        return EleHandle(0);
                    }
                    catch (...) {
                        return EleHandle(0);
                    }
                }
            }

            EleHandle setCData(const std::string& path, const std::string& value)
            {
                return set(path, sss::xml3::type_cdata, value);
            }

            EleHandle setInfo(const std::string& path, const std::string& value)
            {
                return set(path, sss::xml3::type_info, value);
            }

            EleHandle setComment(const std::string& path, const std::string& value)
            {
                return set(path, sss::xml3::type_comment, value);
            }

            // 设置一个空白的节点……
            EleHandle setNode(const std::string& path, const std::string& value)
            {
                return set(path, sss::xml3::type_node, value);
            }

            EleHandle setText(const std::string& path, const std::string& value)
            {
                return set(path, sss::xml3::type_text, value);
            }

            EleHandle setPI(const std::string& path, const std::string& value)
            {
                return set(path, sss::xml3::type_PI, value);
            }

            EleHandle add(const std::string& path, sss::xml3::node_type_t type, const std::string& value)
            {
                if (!this->_data) {
                    return EleHandle(0);
                }
                else {
                    try {
                        // FIXME
                        EleHandle ret(this->_data->verify_path(path));
                        if (ret.get()) {
                            ret.appendChild(type, value);
                            return ret;
                        }
                        return EleHandle(0);
                    }
                    catch (...) {
                        return EleHandle(0);
                    }
                }
            }

            EleHandle addCData(const std::string& path, const std::string& value)
            {
                return add(path, sss::xml3::type_cdata, value);
            }

            EleHandle addInfo(const std::string& path, const std::string& value)
            {
                return add(path, sss::xml3::type_info, value);
            }

            EleHandle addComment(const std::string& path, const std::string& value)
            {
                return add(path, sss::xml3::type_comment, value);
            }

            // 设置一个空白的节点……
            EleHandle addNode(const std::string& path, const std::string& value)
            {
                return add(path, sss::xml3::type_node, value);
            }

            EleHandle addText(const std::string& path, const std::string& value)
            {
                return add(path, sss::xml3::type_text, value);
            }

            // 如何删除节点？
            //
            // 我的xml树形结构，内部相当于有6个指针
            //   -parent
            //   -next
            //   -prev
            //   -first_child
            //   -lastt_child
            //
            //   -document
            //
            // 已知条件：
            //    待删除节点的指针
            //
            // 效果：
            //    首先，隔离这个节点
            //     如果有parent；
            //     则调用 con::tree::unlinkTree()
            //
            //    从docment中，删除id隐射
            //
            //    delete 指针；
            void remove()
            {
                if (this->_data) {
                    delete this->_data;
                    this->_data = 0;
                }
            }

            // EleHandle addPI(const std::string& path, const std::string& value)
            // {
            //     return add(path, sss::xml3::type_PI, value);
            // }

            // EleHandle addPI(const std::string& path, const std::string& value)
            // {
            //     return add(path, sss::xml3::type_doctype, value);
            // }

            template<typename T>
                EleHandle set(const std::string& path, const T& value)
                {
                    if (!this->_data) {
                        return EleHandle(0);
                    }
                    return this->setText(path, sss::cast_string(value));
                }

            EleHandle set(const std::string& path, const std::string& value)
            {
                return this->setText(path, value);
            }

            std::string toString() const
            {
                return this->get("");
            }

            std::string get(const std::string& path) const
            {
                if (!this->_data) {
                    return "";
                }
                sss::xml3::node * n =  this->_data->locate(path);
                if (!n) {
                    return "";
                }
                return n->text();
            }

            EleHandle prevSibling()
            {
                return EleHandle(this->_data ? this->_data->prev_sibling() : 0);
            }

            EleHandle nextSibling()
            {
                return EleHandle(this->_data ? this->_data->next_sibling() : 0);
            }
            EleHandle firstChild()
            {
                return EleHandle(this->_data ? this->_data->first_child() : 0);
            }
            EleHandle lastChild()
            {
                return EleHandle(this->_data ? this->_data->last_child() : 0);
            }

            EleHandle findElementByID(const std::string & id)
            {
                return EleHandle(this->getDocument() ? this->getDocument()->id_get(id) : 0);
            }

            EleHandle findElementByTag(const std::string & tag)
            {
                return EleHandle(this->_data ? this->_data->find_child(tag, 0) : 0);
            }

            EleHandle& setAttr(const std::string& key, const std::string& value)
            {
                if (this->_data && this->_data->is_node()) {
                    this->_data->set(key, value);
                }
                return *this;
            }

            EleHandle& removeAttr(const std::string& key)
            {
                if (this->_data && this->_data->is_node()) {
                    this->_data->remove_key(key);
                }
                return *this;
            }

            bool hasAttr(const std::string& key)
            {
                if (this->_data && this->_data->is_node()) {
                    return this->_data->has_key(key);
                }
                else {
                    return false;
                }
            }

            EleHandle operator++(int)
            {
                EleHandle tmp(*this);
                this->_data = this->nextSibling().get();
                return tmp;
            }

            EleHandle& operator++()
            {
                this->_data = this->nextSibling().get();
                return *this;
            }

            EleHandle operator--(int)
            {
                EleHandle tmp(*this);
                this->_data = this->prevSibling().get();
                return tmp;
            }

            EleHandle& operator--()
            {
                this->_data = this->prevSibling().get();
                return *this;
            }

            EleHandle& toFistChild()
            {
                this->_data = this->firstChild().get();
                return *this;
            }

            EleHandle& toLastChild()
            {
                this->_data = this->lastChild().get();
                return *this;
            }

            sss::xml3::node * nodeCreator(sss::xml3::node_type_t type, const std::string& data);

            // 在当前元素后面插入——返回的是新插入的节点
            // EleHandle append(sss::xml3::node_type_t type, const std::string& data)
            // {
            //     if (!this->_data) {
            //         return EleHandle(0);
            //     }
            //     sss::scoped_ptr<sss::xml3::node> p(this->nodeCreator(type, data));
            //     if (p.get()) {
            //         this->_data->append(p.get());
            //         EleHandle ret(p.get());
            //         p.release();
            //         return ret;
            //     }
            //     return EleHandle(0);
            // }

            // 在当前元素前面插入
            // EleHandle insert(sss::xml3::node_type_t type, const std::string& data)
            // {
            //     if (!this->_data) {
            //         return EleHandle(0);
            //     }
            //     sss::scoped_ptr<sss::xml3::node> p(this->nodeCreator(type, data));
            //     if (p.get()) {
            //         this->_data->ininsert(p.get());
            //         EleHandle ret(p.get());
            //         p.release();
            //         return ret;
            //     }
            //     return EleHandle(0);
            // }

            EleHandle toParent()
            {
                return EleHandle(this->_data ? this->_data->get_parent() : 0);
            }

            EleHandle appendChild(const std::string& data)
            {
                return this->appendChild(sss::xml3::type_text, data);
            }

            EleHandle appendChildText(const std::string& data)
            {
                return this->appendChild(sss::xml3::type_text, data);
            }

            EleHandle appendChildPI(const std::string& data)
            {
                return this->appendChild(sss::xml3::type_PI, data);
            }

            EleHandle locate(const std::string& path)
            {
                return EleHandle(this->_data ? this->_data->locate(path) : 0);
            }

            EleHandle appendChildInfo(const std::string& data)
            {
                return this->appendChild(sss::xml3::type_info, data);
            }

            EleHandle appendChildCData(const std::string& data)
            {
                return this->appendChild(sss::xml3::type_cdata, data);
            }

            EleHandle appendChildComment(const std::string& data)
            {
                return this->appendChild(sss::xml3::type_comment, data);
            }

            EleHandle appendChildDoctype(const std::string& data)
            {
                return this->appendChild(sss::xml3::type_doctype, data);
            }

            EleHandle appendChildNode(const std::string& data)
            {
                return this->appendChild(sss::xml3::type_node, data);
            }

            // NOTE
            // sss::xml3::node 只提供了两组安全的添加子节点的方法：
            // insert_before(node* child, node* refchild)
            // append_child(node* child)
            EleHandle appendChild(sss::xml3::node_type_t type, const std::string& data)
            {
                // SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
                if (!this->_data) {
                    return EleHandle(0);
                }

                sss::scoped_ptr<sss::xml3::node> p(this->nodeCreator(type, data));
                if (p.get()) {
                    this->_data->append_child(p.get());
                    EleHandle ret(p.get());
                    p.release();
                    return ret;
                }
                return EleHandle(0);
            }

            // 在当前元素前面插入
            EleHandle insertChild(sss::xml3::node_type_t type, const std::string& data)
            {
                if (!this->_data) {
                    return EleHandle(0);
                }
                sss::scoped_ptr<sss::xml3::node> p(this->nodeCreator(type, data));
                if (p.get()) {
                    this->_data->insert_before(p.get(), 0);
                    EleHandle ret(p.get());
                    p.release();
                    return ret;
                }
                return EleHandle(0);
            }

            EleHandle insertChild(sss::xml3::node_type_t type, const std::string& data, EleHandle& ref)
            {
                if (!this->_data) {
                    return EleHandle(0);
                }
                sss::scoped_ptr<sss::xml3::node> p(this->nodeCreator(type, data));
                if (p.get()) {
                    this->_data->insert_before(p.get(), ref.get());
                    EleHandle ret(p.get());
                    p.release();
                    return ret;
                }
                return EleHandle(0);
            }

            std::vector<EleHandle> query(const std::string& query_str) const
            {
                std::vector<EleHandle> ret;
                if (this->_data) {
                    sss::scoped_ptr<Selector> sel(sss::xml3::SelectorParser::create(query_str));
                    node_list_t matched = sel->matchAll(this->_data);
                    for (size_t i = 0; i != matched.size(); ++i) {
                        ret.push_back(EleHandle(matched[i]));
                    }
                }
                return ret;
            }

            void print(std::ostream& o) const {
                if (this->_data) {
                    this->_data->print(o);
                }
            }

            node * operator->()
            {
                if (!this->_data) {
                    std::ostringstream oss;
                    oss << D_PREFIX << "::" << __func__ << " dereference null sss::xml3::node*";
                    throw std::runtime_error(oss.str());
                }
                return this->_data;
            }

            node * get()
            {
                return this->_data;
            }

        private:
            node * _data;
        };

        inline std::ostream& operator << (std::ostream& o, const EleHandle& e)
        {
            e.print(o);
            return o;
        }

        class DocHandle {
            friend class EleHandle;

        public:
            DocHandle();
            explicit DocHandle(const std::string& fname,
                               std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);

            // NOTE 这个是有析构函数的
            // 所以……
            // 要么禁止从sss::xml3::xml_doc*构建、禁止拷贝函数；
            // 要么再增加一个参数，用来指定当前handle对象，到底有没有管理内存；
            // 或者，强制要求，传入的sss::xml3::xml_doc*，必须来自解析器！
            // 即，是new来的；
            ~DocHandle();

        protected:
            explicit DocHandle(sss::xml3::xml_doc * p)
                : _pdoc(p), _openmode(std::ios_base::in | std::ios_base::out)
            {
            }

        public:

#ifndef __WIN32__
            DocHandle& loadFromGumbo(const std::string& fname);
#endif

        public:

            void swap(DocHandle& rhs)
            {
                std::swap(this->_pdoc, rhs._pdoc);
                std::swap(this->_fname, rhs._fname);
                std::swap(this->_openmode, rhs._openmode);
            }

            DocHandle& parse(const std::string& xml_str);

            xml3::node * parseSubtree(const std::string& xml_str);

        public:
            DocHandle& load(const std::string& fname);

            DocHandle& save(const std::string& fname);

            DocHandle& save();

            void  clear();

            const DocHandle& write(const std::string& fname, const char * sep = "\t") const;

            const DocHandle& print(std::ostream& o) const;

            EleHandle toNode()
            {
                return EleHandle(this->_pdoc);
            }

            EleHandle locate(const std::string& path) const;

            EleHandle root() const;

            DocHandle& root(const std::string& data);

            std::vector<EleHandle> query(const std::string& query_str) const
            {
                return this->root().query(query_str);
            }

            xml_doc * operator->()
            {
                assert(this->_pdoc);
                return this->_pdoc;
            }

            xml_doc * get() const
            {
                return this->_pdoc;
            }

        public:
            DocHandle(const DocHandle& ref)
                : _pdoc(0), _openmode(std::ios_base::in | std::ios_base::out)
            {
                if (ref.get()) {
                    // NOTE FIXME
                    // 克隆，可能会有问题——因为doc_ptr，应该等于新的……
                    this->_pdoc = dynamic_cast<sss::xml3::xml_doc*>(ref.get()->clone(true));
                    this->_fname = ref._fname;
                    this->_openmode = ref._openmode;
                }
            }

            sss::xml3::xml_doc * release()
            {
                sss::xml3::xml_doc * ret = 0;
                std::swap(ret, this->_pdoc);
                return ret;
            }

            DocHandle& operator = (const DocHandle& ref) {
                DocHandle tmp(ref);
                this->swap(tmp);
                return *this;
            }

        private:
            sss::xml3::xml_doc *    _pdoc;
            std::string             _fname;
            std::ios_base::openmode _openmode;
        };

        inline std::ostream& operator << (std::ostream& o, const DocHandle& doc)
        {
            doc.print(o);
            return o;
        }
    }
}


#endif  /* __XML_HANDLE_HPP_1438236519__ */
