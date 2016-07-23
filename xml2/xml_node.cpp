#include "xml_node.hpp"
#include "xml_doc.hpp"

#include <sss/iConvpp.hpp>
#include <sss/regex/simpleregex.hpp>
#include <sss/spliter.hpp>
#include <sss/utlstring.hpp>
#include <sss/Exception.hpp>
#include <sss/log.hpp>

#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <typeinfo>
#include <map>
#include <set>

#include <stdio.h>


namespace sss{
    namespace xml2{

        //! properties_t {{{1
        properties_t::properties_t() // {{{2
        {
        }

        properties_t::properties_t(const std::string& properties_str) // {{{2
        {
            this->parse(properties_str);
        }

        properties_t::~properties_t() // {{{2
        {
        }

        int         properties_t::parse(const std::string& properties_str) // {{{2
        {
            // 先考虑简单的情况：
            // key = "value"
            // \c\w+\s*=\s*"[^"]+"
            // 允许减号'-'，字母大小写，以及数字；但是数字和减号不能作为开头；
            // 减号不能作为结尾；
            static sss::regex::simpleregex
                key_value_pair_reg("^\\s*"      // 前导空白符
                                   "\\(\\<\\c[\\-a-zA-Z0-9]*\\>\\)" // key
                                   "\\s*=\\s*"  // =
                                   "\"\\([^\"]*\\)\""); // "value"

            std::string::const_iterator match_beg_it = properties_str.begin();
            std::string::const_iterator match_end_it = properties_str.end();

            std::string::const_iterator submatch_beg_it;
            std::string::const_iterator submatch_end_it;

            int cnt = 0;
            typedef std::set<std::string> validators_t;
            validators_t validators;
            while (key_value_pair_reg.match(match_beg_it, match_end_it,
                                            submatch_beg_it, submatch_end_it))
            {
                std::string key = key_value_pair_reg.get_submatch(1);
                std::string value = key_value_pair_reg.get_submatch(2);
                SSS_LOG_DEBUG("key = \"%s\"; value = \"%s\"\n",
                              key.c_str(), value.c_str());

                validators_t::iterator it = validators.find(key);
                if (it != validators.end()) {
                    throw std::runtime_error("encounter the same key `" + key + "`");
                }
                validators.insert(key);
                this->set(key, value);
                ++cnt;

                match_beg_it = submatch_end_it;
            }
            return cnt;
        }

        void        properties_t::print(std::ostream& o) const // {{{2
        {
            if (!this->empty()) {
                for (const_iterator it = this->begin(); it != this->end(); ++it) {
                    o << " " << it->first << "=\"" << it->second << "\"";
                }
                //o << " ";
            }
        }

        bool        properties_t::has_key(const std::string& key) const // {{{2
        {
            return this->find(key) != this->end();
        }

        properties_t::iterator properties_t::find(const std::string& key) // {{{2
        {
            for (iterator it = this->begin(); it != this->end(); ++it) {
                if (it->first == key) {
                    return it;
                }
            }
            return this->end();
        }

        properties_t::const_iterator properties_t::find(const std::string& key) const // {{{2
        {
            for (const_iterator it = this->begin(); it != this->end(); ++it) {
                if (it->first == key) {
                    return it;
                }
            }
            return this->end();
        }

        std::string properties_t::get(const std::string& key) const // {{{2
        {
            const_iterator it = this->find(key);

            if (it != this->end()) {
                return it->second;
            }
            else {
                // FIXME ……
                return "";
            }
        }

        void        properties_t::set(const std::string& key, const std::string& value) // {{{2
        {
            iterator it = this->find(key);
            if (it != this->end()) {
                it->second = value;
            }
            else {
                this->insert(it, std::make_pair(key, value));
            }
        }

        //! node {{{1
        bool        node::is_properties_able() const // {{{2
        {
            return (this->node_type & 2) == 2;
        }

        node *        node::set(const std::string& key, const std::string& value) // {{{2
        {
            if (!this->is_properties_able()) {
                std::string msg =
                    this->get_node_type_name()
                    + std::string("::set(key, value)");
                throw sss::ExceptionNotSupportMethod(msg);
            }
            if (this->get_doc() && key == "id") {
                this->get_doc()->id_add(value, this);
            }
            this->properties.set(key, value);
            return this;
        }

        std::string node::get(const std::string& key) const // {{{2
        {
            if (!this->is_properties_able()) {
                std::string msg =
                    this->get_node_type_name()
                    + std::string("::get(key)");
                throw sss::ExceptionNotSupportMethod(msg);
            }
            return this->properties.get(key);
        }

        bool        node::has_key(const std::string& key) const // {{{2
        {
            if (!this->is_properties_able()) {
                std::string msg =
                    this->get_node_type_name()
                    + std::string("::has_key(key)");
                throw sss::ExceptionNotSupportMethod(msg);
            }
            return this->properties.has_key(key);
        }

        node::node(const std::string& name, xml_doc * pdoc, node_type_t t) // {{{2
            : data(name), p_doc(pdoc), p_parent(0),
              node_type(t)
        {
        }

        node::~node() // {{{2
        {
            this->clear();
        }

        void node::clear() // {{{2
        {
            while (!node_list.empty()) {
                node * tmp = node_list.back();
                node_list.pop_back();
                delete tmp;
            }
        }

        size_t   node::size() const // {{{2
        {
            return this->node_list.size();
        }

        void node::print(std::ostream& out, const char * sep) const // {{{2
        {
            html_util::indent_wraper ind(sep);
            this->print_impl(out, ind);
        }

        void node::print_text(std::ostream& out, const char * sep) const // {{{2
        {
            html_util::indent_wraper ind(sep);
            this->print_text_impl(out, ind);
        }

        void node::print_text_impl(std::ostream& out, html_util::indent_wraper& ind) const // {{{2
        {
            switch (this->node_type) {
            case type_cdata:
                out << this->get_data();
                break;

            case type_node:
                for (size_t i = 0; i != this->node_list.size(); ++i) {
                    node_list[i]->print_text_impl(out, ind);
                }
                break;

            case type_text:
                out << this->get_data();
                break;

            default:
                break;
            // case type_info:
            // case type_comment:
            // case type_xmldoc:
            // case type_doctype:
            }
        }

        void node::print_impl(std::ostream& out, html_util::indent_wraper& ind) const // {{{2
        {
#if 0
            bool is_self_node = this->node_list.empty();

            if (!is_self_node) {
                bool is_inner_text = false;
                is_inner_text =
                    (this->node_list.size() == 1 &&
                     this->node_list[0]->is_node_text());

                out << ind.get() << "<" << this->get_data() << this->properties << ">";
                {
                    html_util::indent_auto ind_auto(ind);
                    if (!is_inner_text) {
                        out << std::endl;
                    }
                    for (size_t i = 0; i != this->node_list.size(); ++i) {
                        node_list[i]->print_impl(out, ind);
                        if (!is_inner_text && node_list[i]->node_type == type_text) {
                            out << std::endl;
                        }
                    }
                }
                if (!is_inner_text) {
                    out << ind.get();
                }
                out << "</" << this->get_data() << ">" << std::endl;
            }
            else {
                out << ind.get() << "<" << this->get_data() << this->properties << "/>"
                    << std::endl;
            }
#else
            // NOTE 如果内部只有一个节点（不管是text,comment,还是 node），都不额外输出回车符。
            bool is_self_node = this->node_list.empty();

            if (!is_self_node) {
                bool is_inner_text = (this->node_list.size() == 1);

                out << ind.get() << "<" << this->get_data() << this->properties << ">";
                {
                    html_util::indent_auto ind_auto(ind);
                    if (!is_inner_text) {
                        out << std::endl;
                    }
                    for (size_t i = 0; i != this->node_list.size(); ++i) {
                        if (is_inner_text) {
                            node_list[i]->print(out);
                        }
                        else {
                            node_list[i]->print_impl(out, ind);
                            if (!is_inner_text) {
                                out << std::endl;
                            }
                        }
                    }
                }
                if (!is_inner_text) {
                    out << ind.get();
                }
                out << "</" << this->get_data() << ">";
            }
            else {
                out << ind.get() << "<" << this->get_data() << this->properties << "/>";
            }
#endif
        }

        node * node::locate(const std::string& path) // {{{2
        {
            //    E
            //          元素名称(div, p);
            //    E#id
            //          使用id的元素(div#content, p#intro, span#error);
            //    E.class
            //          使用类的元素(div.header, p.error.critial). 你也可以联合
            //          使用class和idID: div#content.column.width;
            //    E>N
            //          子代元素(div>p, div#footer>p>span);
            //    E+N
            //          兄弟元素(h1+p, div#header+div#content+div#footer);
            //    E*N
            //          元素倍增(ul#nav>li*5>a);
            //    E$*N
            //          条目编号(ul#nav>li.item-$*5);
            //    E:N
            //          第N个子节点；
            node * tmp = this;
            sss::Spliter sp(path, '>');
            std::string stem;
            while (sp.fetch_next(stem)) {
                //static sss::regex::simpleregex
                //    reg_is_node_with_id("^\\w+#\\w+$");
                static sss::regex::simpleregex
                    reg_is_node_with_index("^\\w+:\\d+$");

                int index = 0;
                if (reg_is_node_with_index.match(stem)) {
                    int n;
                    sscanf(stem.c_str(), "%*[^:]%n:%d", &n, &index);
                    stem = stem.substr(0, n);
                }
                node* next = tmp->find_child(stem, index);
                if (next) {
                    tmp = next;
                }
                else {
                    tmp = 0;
                    break;
                }
            }
            return tmp;
        }

        node * node::verify_path(const std::string& path) // {{{2
        {
            node * tmp = this;
            sss::Spliter sp(path, '>');
            std::string stem;
            while (sp.fetch_next(stem)) {
                //static sss::regex::simpleregex
                //    reg_is_node_with_id("^\\w+#\\w+$");
                static sss::regex::simpleregex
                    reg_stem("^\\(\\c\\w*\\)$");
                    //reg_stem("^\\(\\c\\w+\\):\\(\\d+\\)$");

                if (reg_stem.match(stem)) {
                    stem = reg_stem.get_submatch(1);
                }
                node* next = tmp->find_child(stem, 0);
                if (next) {
                    tmp = next;
                }
                else {
                    next = tmp->p_doc->create_node(stem);
                    tmp = tmp->append_child(next);
                }
            }
            return tmp;
        }

        bool node::swap(node * that) // {{{2
        {
            if (!that) {
                return false;
            }
            if (that->get_doc() != this->get_doc()) {
                return false;
                //throw std::runtime_error("only swap node under same xml_doc.");
            }
            if (that == this->get_doc() || this == this->get_doc()) {
                return false;
                //throw std::runtime_error("can not swap xml_doc node.");
            }
            if (!this->p_parent && !that->p_parent) {
                throw std::runtime_error("this and that all has no parent node");
            }
            // swap p_parent
            if (this->p_parent) {
                node_list_t::iterator it_this = std::find(this->p_parent->node_list.begin(),
                                                          this->p_parent->node_list.end(),
                                                          this);
                assert(it_this != this->p_parent->node_list.end());
                *it_this = that;
            }
            if (that->p_parent) {
                node_list_t::iterator it_that = std::find(that->p_parent->node_list.begin(),
                                                          that->p_parent->node_list.end(),
                                                          that);
                assert(it_that != that->p_parent->node_list.end());
                *it_that = this;
            }
            std::swap(this->p_parent, that->p_parent);

            // swap node_list
            if (!this->node_list.empty()) {
                for (node_list_t::iterator it = this->node_list.begin();
                     it != this->node_list.end();
                     ++it)
                {
                    (*it)->p_parent = that;
                }
            }
            if (!that->node_list.empty()) {
                for (node_list_t::iterator it = that->node_list.begin();
                     it != that->node_list.end();
                     ++it)
                {
                    (*it)->p_parent = this;
                }
            }
            std::swap(this->node_list, that->node_list);

            return this;
        }

        const char * node::get_node_type_name() const // {{{2
        {
            static std::map<node_type_t, const char*> names;
            if (names.empty()) {
                // vim:'<,'>s/^\S\+$/\tnames[\0] = "\0";/ge
                names[type_node] = "type_node";
                names[type_text] = "type_text";
                names[type_info] = "type_info";
                names[type_doctype] = "type_doctype";
                names[type_comment] = "type_comment";
                names[type_cdata] = "type_cdata";
                names[type_xmldoc] = "type_xmldoc";
                names[type_xhtmldoc] = "type_xhtmldoc";
            }
            return names[this->node_type];
        }

        void node::enumrate_subnode() const // {{{2
        {
            std::cout
                << this->get_node_type_name()
                << "(" << this->get_data() << ")"
                << std::endl;

            printf(" at(%p)\n", this);

            std::cout
                << "sub cnt = " << this->node_list.size()
                << std::endl;
            for (size_t i = 0; i != this->node_list.size(); ++i) {
                std::cout
                    << i << ":" << this->node_list[i]->get_node_type_name()
                    << "(" << this->node_list[i]->get_data() << ")"
                    << std::endl;
            }
        }

        // 递归查找符合名字的所有子孙节点
        node_list_t node::find_all(const std::string& name) const // {{{2
        {
            node_list_t ret;
            this->find_all_impl(name, ret);
            return ret;
        }

        void node::find_all_impl(const std::string& name, node_list_t & ret) const // {{{2
        {
            for (node_list_t::const_iterator it = this->node_list.begin();
                 it != this->node_list.end();
                 ++it)
            {
                if ((*it)->is_node(name)) {
                    ret.push_back(*it);
                }

                // NOTE not else if!
                if ((*it)->type() == type_node) {
                    (*it)->find_all_impl(name, ret);
                }
            }
        }

        node_list_t node::find_all_empty_node() const // {{{2
        {
            node_list_t ret;
            this->find_all_empty_node_impl(ret);
            return ret;
        }

        void node::find_all_empty_node_impl(node_list_t & ret) const // {{{2
        {
            for (node_list_t::const_iterator it = this->node_list.begin();
                 it != this->node_list.end();
                 ++it)
            {
                if ((*it)->is_empty_node()) {
                    ret.push_back(*it);
                }
                else if ((*it)->type() == type_node) {
                    (*it)->find_all_empty_node_impl(ret);
                }
            }
        }

        bool node::is_empty_node() const // {{{2
        {
            return this->type() == type_node && this->node_list.empty();
        }

        bool node::is_leaf() const // {{{2
        {
            return this->is_empty_node() || this->type() != type_node;
        }

        node_list_t node::find_child(const std::string& name) const // {{{2
        {
            node_list_t ret;
            for (node_list_t::const_iterator it = this->node_list.begin();
                 it != this->node_list.end();
                 ++it)
            {
                if ((*it)->is_node(name)) {
                    ret.push_back(*it);
                }
            }
            return ret;
        }

        node * node::find_child(const std::string& name, int index) const // {{{2
        {
            // FIXME 如果 index 传入的值，就是 -1，那么本函数岂不是要死循环？
            node * ret = 0;
            if (0 <= index && index < int(this->node_list.size())) {
                for (node_list_t::const_iterator it = this->node_list.begin();
                     it != this->node_list.end();
                     ++it)
                {
                    if ((*it)->is_node(name)) {
                        --index;
                    }
                    if (index == -1) {
                        ret = (*it);
                        break;
                    }
                }
            }
            return ret;
        }

        node_list_t& node::get_node_list() // {{{2
        {
            return this->node_list;
        }

        node * node::first_child() const // {{{2
        {
            return *this->node_list.begin();
        }

        node * node::last_child() const // {{{2
        {
            return *this->node_list.rbegin();
        }

        node * node::next_sibling() const // {{{2
        {
            node * parent = get_parent();
            if (parent) {
                node_list_t::const_iterator it = std::find(parent->node_list.begin(),
                                                           parent->node_list.end(),
                                                           this);
                assert(it != parent->node_list.end());
                it++;
                if (it == parent->node_list.end()) {
                    return 0;
                }
                return *it;
            }
            return 0;
        }

        node * node::prev_sibling() const // {{{2
        {
            node * parent = get_parent();
            if (parent) {
                node_list_t::const_iterator it = std::find(parent->node_list.begin(),
                                                           parent->node_list.end(),
                                                           this);
                assert(it != parent->node_list.end());
                if (it == parent->node_list.begin()) {
                    return 0;
                }
                --it;
                return *it;
            }
            return 0;
        }

        // 返回被插入的节点；
        // refchild 必须是当前节点的儿子！
        // child 不能是当前节点的祖先！
        // 如果refchild 是 null；则插到第一个！
        node * node::insert_before(node* child, node* refchild) // {{{2
        {
            if (!this->is_node()) {
                std::string msg =
                    this->get_node_type_name()
                    + std::string("::insert_before(new_child, refchild)");
                throw sss::ExceptionNotSupportMethod(msg);
            }
            if (!refchild) {
                refchild = this->first_child();
            }
            // FIXME
            // 检查 refchild 的合法性！
            this->node_list.insert(std::find(this->node_list.begin(),
                                             this->node_list.end(),
                                             refchild),
                                   child);
            return refchild;
        }

        // 返回被插入的第一个节点
        node * node::insert_before(const node_list_t& childs, node* refchild) // {{{2
        {
            if (!this->is_node()) {
                std::string msg =
                    this->get_node_type_name()
                    + std::string("::insert_before(new_childs, refchild)");

                throw sss::ExceptionNotSupportMethod(msg);
            }
            if (childs.size()) {
                if (!refchild) {
                    refchild = this->first_child();
                }
                // FIXME
                // 检查 refchild 的合法性！
                this->node_list.insert(std::find(this->node_list.begin(),
                                                 this->node_list.end(),
                                                 refchild),
                                       childs.begin(), childs.end());
                return childs[0];
            }
            else {
                return 0;
            }
        }

        node * node::append_child(node* child) // {{{2
        {
            if (this->is_node()) {
                if (child) {
                    this->node_list.push_back(child);
                    child->join_family(this);
                }
                return child;
            }
            else {
                std::string msg =
                    this->get_node_type_name()
                    + std::string("::append_child()");
                throw sss::ExceptionNotSupportMethod(msg);
            }
        }

        node * node::append_child(const node_list_t& subs) // {{{2
        {
            if (this->is_node()) {
                for (size_t i = 0; i != subs.size(); ++i) {
                    this->append_child(subs[i]);
                }
                if (subs.empty()) {
                    return 0;
                }
                else {
                    return subs[0];
                }
            }
            else {
                std::string msg =
                    std::string(this->get_node_type_name())
                    + "::append_child(const::node_list_t&)";
                throw sss::ExceptionNotSupportMethod(msg);
            }
        }

        bool   node::is_text_container() const // {{{2
        {
            return
                this->node_type == type_node &&
                this->node_list.size() == 1 &&
                this->node_list[0]->is_node_text();
        }

        std::string node::text() const // {{{2
        {
            if (this->is_text_container()) {
                return this->node_list[0]->text();
            }
            else {
                return "";
            }
        }

        node * node::text(const std::string& str) // {{{2
        {
            if (this->is_text_container()) {
                this->node_list[0]->text(str);
            }
            else if (this->is_empty_node()) {
                this->append_child(this->p_doc->create_text(str));
            }
            return this;
        }

        std::string node::inner_text() const // {{{2
        {
            std::ostringstream oss;
            for (size_t i = 0; i != this->size(); ++i) {
                this->node_list[i]->print_text(oss);
            }
            return oss.str();
        }

        std::string node::inner_xml() const // {{{2
        {
            std::ostringstream oss;
            for (size_t i = 0; i != this->size(); ++i) {
                this->node_list[i]->print(oss);
            }
            return oss.str();
        }

        node * node::clone(bool is_deep) const // {{{2
        {
            node * ret = new node(this->get_data(), this->p_doc);
            if (is_deep) {
                for (size_t i = 0; i != this->node_list.size(); ++i) {
                    ret->append_child(this->node_list[i]->clone(true));
                }
            }
            ret->properties = this->properties;
            return ret;
        }

        void node::join_family(node* ref) // {{{2
        {
            if (ref) {
                this->p_doc = ref->p_doc;
                this->p_parent = ref;
            }
        }

        node_type_t node::type() const // {{{2
        {
            return node_type;
        }

        bool node::is_node_text() const // {{{2
        {
            return node_type == type_text;
        }

        bool node::is_node_cdata() const // {{{2
        {
            return node_type == type_cdata;
        }

        bool node::is_node(const std::string& name) const // {{{2
        {
            bool ret = this->is_node() && (name == "*" || this->get_data() == name);
            // bool ret =
            //     (this->is_node() && name != "*" && this->get_data() == name) ||
            //     (this->is_node() && name == "*");

            return ret;
        }

        // 是否允许有子节点？
        bool node::is_node() const // {{{2
        {
            return (this->node_type & 1) == 1;
        }

        // 解析xml的属性字符串，并返回解析数；
        int  node::parse_properties_str(const std::string& properties_str) // {{{2
        {
            return this->properties.parse(properties_str);
        }

        std::string node::get_data() const // {{{2
        {
            return this->data;
        }

        std::string node::set_data(const std::string& d) // {{{2
        {
            std::string ret = this->get_data();
            this->data = d;
            return ret;
        }

        xml_doc * node::get_doc() const // {{{2
        {
            return this->p_doc;
        }

        node *    node::get_parent() const // {{{2
        {
            return this->p_parent;
        }

        //! node_info {{{1
        node_info::node_info(const std::string& name, xml_doc * pdoc) // {{{2
            : node(name, pdoc)
        {
             this->node_type = type_info;
             // 默认值
             this->set("version", "1.0");
             this->set("encoding", "utf8");
        }

        node_info::~node_info() // {{{2
        {
        }

        // NOTE info 对象，必须先输出 version ，然后输出encoding，不然浏览器会无法识别……
        // 额，这算啥鸟门子的限制？
        // 应该先确定编码集，再决定版本吧？
        // 2015-07-29
        // 这是 xml3c 标准规定的
        void node_info::print_impl(std::ostream& out, html_util::indent_wraper& ind) const // {{{2
        {
            // <?xml version="1.0" encoding="utf-8" ?>
            out << ind.get()
                << "<?" << this->get_data() << this->properties << "?>";
        }

        node * node_info::clone(bool /* is_deep */) const // {{{2
        {
            node * ret = new node_info(this->get_data(), this->p_doc);
            ret->properties = this->properties;
            return ret;
        }

        //! node_doctype {{{1
        node_doctype::node_doctype(const std::string& data, xml_doc * pdoc) // {{{2
            : node(data, pdoc)
        {
            this->node_type = type_doctype;
        }

        node_doctype::~node_doctype() // {{{2
        {
        }

        void   node_doctype::print_impl(std::ostream& out, html_util::indent_wraper& ind) const // {{{2
        {
            out << ind.get() << this->get_data();
        }

        node * node_doctype::clone(bool ) const // {{{2
        {
            return new node_doctype(this->get_data(), this->p_doc);
        }

        //! node_comment {{{1
        node_comment::node_comment(const std::string& comment, xml_doc * pdoc) // {{{2
            : node(comment, pdoc)
        {
            this->node_type = type_comment;
        }

        node_comment::~node_comment() // {{{2
        {
        }

        void   node_comment::print_impl(std::ostream& out, html_util::indent_wraper& ind) const // {{{2
        {
            out << ind.get();
            out << "<!--" << this->get_data() << "-->";
        }

        node * node_comment::clone(bool ) const // {{{2
        {
            return new node_comment(this->get_data(), this->p_doc);
        }

        //! node_text {{{1
        node_text::node_text(const std::string& text, xml_doc * pdoc) // {{{2
            : node(text, pdoc)
        {
             this->node_type = type_text;
        }

        node_text::~node_text() // {{{2
        {
        }

        std::string node_text::text() const // {{{2
        {
            return this->get_data();
        }

        node * node_text::text(const std::string& str) // {{{2
        {
            this->data = xml_doc::safe_xml_entites(str);
            return this;
        }

        node * node_text::clone(bool /* is_deep */) const // {{{2
        {
            return new node_text(this->get_data(), this->p_doc);
        }

        void node_text::print_impl(std::ostream& out, // {{{2
                                   html_util::indent_wraper& ind) const
        {
            if (this->p_parent && this->p_parent->size() >= 2) {
                out << ind.get();
            }
            out << this->get_data();
        }

        //! node_cdata {{{1
        node_cdata::node_cdata(const std::string& data, xml_doc * pdoc) // {{{2
            : node(data, pdoc)
        {
            this->node_type = type_cdata;
        }

        node_cdata::~node_cdata() // {{{2
        {
        }

        std::string node_cdata::text() const // {{{2
        {
            return this->get_data();
        }

        node * node_cdata::text(const std::string& str) // {{{2
        {
            std::string ret = this->get_data();
            this->set_data(str);
            return this;
        }

        void node_cdata::print_impl(std::ostream& out, html_util::indent_wraper& ind) const // {{{2
        {
            out << ind.get();
            out << "<![CDATA[" << this->get_data() << "]]>";
        }

        node * node_cdata::clone(bool ) const // {{{2
        {
            return new node_cdata(this->get_data(), this->p_doc);
        }

        //! node_PI {{{1
        node_PI::node_PI(const std::string& pi, xml_doc *pdoc) // {{{2
            : node(pi, pdoc)
        {
            this->node_type = type_PI;
        }

        node_PI::~node_PI() // {{{2
        {
        }

        void node_PI::print_impl(std::ostream& out, html_util::indent_wraper& ind) const // {{{2
        {
            out << ind.get()
                << "<?" << this->get_data() << "?>" << std::endl;
        }
        node * node_PI::clone(bool ) const // {{{2
        {
            return new node_PI(this->get_data(), this->p_doc);
        }

    } // end of xml
} // end of sss
