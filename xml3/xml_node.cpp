#include "xml_node.hpp"
#include "xml_doc.hpp"
#include "xml_visitor.hpp"
#include "xml_vprinter.hpp"

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
    namespace xml3{

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

        bool        properties_t::remove_key(const std::string& key) // {{{2
        {
            iterator it = this->find(key);
            bool ret = (it != this->end());
            if (ret) {
                this->erase(it);
            }
            return ret;
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
        node::node(const std::string& name, xml_doc * pdoc, node_type_t t) // {{{2
            : Base_findtype(t),
              data(name), p_doc(pdoc)
        {
        }

        node::~node() // {{{2
        {
            this->clear();
            this->Base_tree::unlinkTree();
        }

        bool        node::is_properties_able() const // {{{2
        {
            return (this->Base_findtype::type() & 2) == 2;
        }

        node *        node::set(const std::string& key, const std::string& value) // {{{2
        {
            if (!this->is_properties_able()) {
                std::string msg =
                    this->get_node_type_name()
                    + std::string("::set(key, value)");
                throw sss::ExceptionNotSupportMethod(msg);
            }
            // NOTE
            // 游离的节点，可以随意修改id；
            //
            // 但是，已经加入某document的节点，则不能！
            if (this->get_doc() && this->parent() && key == "id") {
                std::string old_id;
                if (this->has_key("id")) {
                    old_id = this->get("id");
                }
                // FIXME value 有可能为空串""
                if (!old_id.empty() && old_id != value) {
                    this->get_doc()->id_remove(old_id);
                }
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

        bool        node::remove_key(const std::string& key) // {{{2
        {
            if (!this->is_properties_able()) {
                std::string msg =
                    this->get_node_type_name()
                    + std::string("::remove_key(key)");
                throw sss::ExceptionNotSupportMethod(msg);
            }

            if (key == "id" && this->get_doc() && this->parent()) {
                if (this->has_key("id")) {
                    std::string id = this->get("id");
                    sss::xml3::node * maped_node = this->get_doc()->id_get(id);
                    if (maped_node != this) {
                        std::ostringstream oss;
                        oss << "maped node @" << maped_node << " not equal to this @" << this << std::endl;
                        throw std::runtime_error(oss.str());
                    }
                    this->get_doc()->id_remove(id);
                }
            }
            return this->properties.remove_key(key);
        }

        void node::clear() // {{{2
        {
            if (this->is_properties_able()) {
                this->remove_key("id");
            }

            this->Base_tree::clear();
        }

        size_t   node::size() const // {{{2
        {
            return this->Base_tree::size();
        }

        void node::print(std::ostream& out, const char * sep) const // {{{2
        {
            sss::xml3::xml_vprinter jv(out, sep);
            sss::xml3::node * p = const_cast<sss::xml3::node *>(this);
            p->accept(jv);
        }

        void node::print_text(std::ostream& out, const char * sep) const // {{{2
        {
            html_util::indent_wraper ind(sep);
            this->print_text_impl(out, ind);
        }

        void node::print_text_impl(std::ostream& out, html_util::indent_wraper& ind) const // {{{2
        {
            switch (this->Base_findtype::type()) {
            case type_cdata:
                out << this->get_data();
                break;

            case type_node:
                for (node * it = this->firstChild(); it; it = it->nextSibling()) {
                    it->print_text_impl(out, ind);
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
                    reg_is_node_with_index("^\\w+:-\\?\\d+$");

                int index = 0;
                if (reg_is_node_with_index.match(stem)) {
                    int n;
                    sscanf(stem.c_str(), "%*[^:]%n:%d", &n, &index);
                    // SSS_LOG_EXPRESSION(sss::log::log_ERROR, index);
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
            if (!this->parent() && !that->parent()) {
                throw std::runtime_error("this and that all has no parent node");
            }

            node * this_prev = this->prevSibling();
            node * that_prev = that->prevSibling();

            node * this_parent = this->parent();
            node * that_parent = that->parent();

            this->unlink();
            that->unlink();

            // swap p_parent
            if (this_parent) {
                this_parent->appendChild(this_prev, that);
                that->join_family(this_parent);
            }
            if (that_parent) {
                that_parent->appendChild(that_prev, this);
                this->join_family(that_parent);
            }

            //node * old_parent = this->parent(that->parent());
            //that->parent(old_parent);
            ////std::swap(this->_parent, that->_parent);

            // swap node_list
            if (this->hasChildren()) {
                for (node * it = this->firstChild(); it; it = it->nextSibling())
                {
                    it->parent(that);
                }
            }
            if (that->hasChildren()) {
                for (node * it = that->firstChild(); it; it = it->nextSibling())
                {
                    it->parent(this);
                }
            }
            std::swap(this->_firstChild, that->_firstChild);
            std::swap(this->_lastChild,  that->_lastChild);

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
            return names[node_type_t(this->Base_findtype::type())];
        }

        void node::enumrate_subnode() const // {{{2
        {
            std::cout
                << this->get_node_type_name()
                << "(" << this->get_data() << ")"
                << std::endl;

            printf(" at(%p)\n", this);

            std::cout
                << "sub cnt = " << this->size()
                << std::endl;
            int i = 0;
            for (node * it = this->firstChild(); it; it = it->nextSibling(), ++i) {
                std::cout
                    << i << ":" << it->get_node_type_name()
                    << "(" << it->get_data() << ")"
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
            for (node * it = this->firstChild();
                 it;
                 it = it->nextSibling())
            {
                if (it->is_node(name)) {
                    ret.push_back(it);
                }

                // NOTE not else if!
                if (it->is_type(type_node)) {
                    it->find_all_impl(name, ret);
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
            for (node * it = this->firstChild();
                 it;
                 it = it->nextSibling())
            {
                if (it->is_empty_node()) {
                    ret.push_back(it);
                }
                else if (it->is_type(type_node)) {
                    it->find_all_empty_node_impl(ret);
                }
            }
        }

        bool node::is_empty_node() const // {{{2
        {
            return this->is_type(type_node) && !this->hasChildren();
        }

        bool node::is_leaf() const // {{{2
        {
            return this->is_empty_node() || this->type() != type_node;
        }

        node_list_t node::find_child(const std::string& name) const // {{{2
        {
            node_list_t ret;
            for (node * it = this->firstChild();
                 it;
                 it = it->nextSibling())
            {
                if (it->is_node(name)) {
                    ret.push_back(it);
                }
            }
            return ret;
        }

        // 0或正，正向查找；
        // 负数，逆向查找
        node * node::find_child(const std::string& name, int index) const // {{{2
        {
            node * ret = 0;
            if (0 <= index && index < int(this->size())) {
                for (node* it = this->firstChild();
                     it;
                     it = it->nextSibling())
                {
                    if (it->is_node(name)) {
                        --index;
                    }
                    if (index == -1) {
                        ret = it;
                        break;
                    }
                }
            }
            else if (-int(this->size()) <= index && index < 0) {
                for (node* it = this->lastChild();
                     it;
                     it = it->prevSibling())
                {
                    if (it->is_node(name)) {
                        ++index;
                    }
                    if (index == 0) {
                        ret = it;
                        break;
                    }
                }
            }
            return ret;
        }

        //node_list_t& node::get_node_list() // {{{2
        //{
        //    return this->node_list;
        //}

        node * node::first_child() const // {{{2
        {
            return this->Base_tree::firstChild();
        }

        node * node::last_child() const // {{{2
        {
            return this->Base_tree::lastChild();
        }

        node * node::next_sibling() const // {{{2
        {
            return this->Base_tree::nextSibling();
        }

        node * node::prev_sibling() const // {{{2
        {
            return this->Base_tree::prevSibling();
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
            this->Base_tree::insertChild(child, refchild);
            return refchild;
        }

        // 返回被插入的第一个节点
        // 当前refchild != 0的时候，
        // 依次，往这个之前，插一个点即可；
        // 如果refchild == 0，说明，是往子节点头部插入
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
                for (node_list_t::const_iterator it = childs.begin();
                     it != childs.end();
                     ++it)
                {
                    this->Base_tree::insertChild(*it, refchild);
                }
                return childs[0];
            }
            else {
                return 0;
            }
        }

        node * node::append_child(node* child) // {{{2
        {
            if (!this->is_node()) {
                std::string msg =
                    this->get_node_type_name()
                    + std::string("::append_child()");
                throw sss::ExceptionNotSupportMethod(msg);
            }
            if (child) {
                std::string id;
                if (child->is_node()) {
                    id = child->get("id");
                }
                if (!id.empty() && this->get_doc()) {
                    this->get_doc()->id_add(id, child);
                }
                this->Base_tree::appendChild(child);
                child->join_family(this);
            }
            return child;
        }

        node * node::append_child(const node_list_t& subs) // {{{2
        {
            if (this->is_node()) {
                std::string msg =
                    std::string(this->get_node_type_name())
                    + "::append_child(const::node_list_t&)";
                throw sss::ExceptionNotSupportMethod(msg);
            }
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

        bool   node::is_text_container() const // {{{2
        {
            return
                this->Base_findtype::type() == type_node &&
                this->size() == 1 &&
                this->firstChild()->is_node_text();
        }

        std::string node::text() const // {{{2
        {
            if (this->is_text_container()) {
                return this->firstChild()->text();
            }
            else {
                return "";
            }
        }

        node * node::text(const std::string& str) // {{{2
        {
            if (this->is_text_container()) {
                this->firstChild()->text(str);
            }
            else if (this->is_empty_node()) {
                this->append_child(this->p_doc->create_text(str));
            }
            return this;
        }

        std::string node::inner_text() const // {{{2
        {
            std::ostringstream oss;
            for (node * it = this->firstChild();
                 it;
                 it = it->nextSibling())
            {
                it->print_text(oss);
            }
            return oss.str();
        }

        std::string node::inner_xml() const // {{{2
        {
            std::ostringstream oss;
            for (node * it = this->firstChild();
                 it;
                 it = it->nextSibling())
            {
                it->print(oss);
            }
            return oss.str();
        }

        void node::accept(xml_visitor& v)
        {
            v.visit(this);
        }

        node * node::clone(bool is_deep) const // {{{2
        {
            node * ret = new node(this->get_data(), this->p_doc);
            if (is_deep) {
                for (node * it = this->firstChild();
                     it;
                     it = it->nextSibling())
                {
                    ret->append_child(it->clone(true));
                }
            }
            ret->properties = this->properties;
            return ret;
        }

        void node::join_family(node* ref) // {{{2
        {
            if (ref) {
                this->p_doc = ref->p_doc;
                this->parent(ref);
                if (this->p_doc && this->is_type(type_doctype)) {
                    if (!this->p_doc->doctype()) {
                        this->p_doc->doctype(this);
                    }
                    else if (this->p_doc->doctype() != this) {
                        throw std::runtime_error("re assignment doctype node!");
                    }
                }
            }
        }

        node_type_t node::type() const // {{{2
        {
            return node_type_t(Base_findtype::type());
        }

        bool node::is_node_text() const // {{{2
        {
            return this->Base_findtype::is_type(type_text);
        }

        bool node::is_node_cdata() const // {{{2
        {
            return this->Base_findtype::is_type(type_cdata);
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
            return (this->Base_findtype::type() & 1) == 1;
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
            return this->Base_tree::parent();
        }

        //! node_info {{{1
        node_info::node_info(const std::string& name, xml_doc * pdoc) // {{{2
            : node(name, pdoc, type_info)
        {
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

        node * node_info::clone(bool /* is_deep */) const // {{{2
        {
            node * ret = new node_info(this->get_data(), this->p_doc);
            ret->properties = this->properties;
            return ret;
        }

        void node_info::accept(xml_visitor& v)
        {
            v.visit(this);
        }

        //! node_doctype {{{1
        node_doctype::node_doctype(const std::string& data, xml_doc * pdoc) // {{{2
            : node(data, pdoc, type_doctype)
        {
        }

        node_doctype::~node_doctype() // {{{2
        {
        }

        node * node_doctype::clone(bool ) const // {{{2
        {
            return new node_doctype(this->get_data(), this->p_doc);
        }

        void node_doctype::accept(xml_visitor& v)
        {
            v.visit(this);
        }

        //! node_comment {{{1
        node_comment::node_comment(const std::string& comment, xml_doc * pdoc) // {{{2
            : node(comment, pdoc, type_comment)
        {
        }

        node_comment::~node_comment() // {{{2
        {
        }

        node * node_comment::clone(bool ) const // {{{2
        {
            return new node_comment(this->get_data(), this->p_doc);
        }

        void node_comment::accept(xml_visitor& v)
        {
            v.visit(this);
        }

        //! node_text {{{1
        node_text::node_text(const std::string& text, xml_doc * pdoc) // {{{2
            : node(text, pdoc, type_text)
        {
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

        void node_text::accept(xml_visitor& v)
        {
            v.visit(this);
        }

        node * node_text::clone(bool /* is_deep */) const // {{{2
        {
            return new node_text(this->get_data(), this->p_doc);
        }

        //! node_cdata {{{1
        node_cdata::node_cdata(const std::string& data, xml_doc * pdoc) // {{{2
            : node(data, pdoc, type_cdata)
        {
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

        node * node_cdata::clone(bool ) const // {{{2
        {
            return new node_cdata(this->get_data(), this->p_doc);
        }

        void node_cdata::accept(xml_visitor& v)
        {
            v.visit(this);
        }

        //! node_PI {{{1
        node_PI::node_PI(const std::string& pi, xml_doc *pdoc) // {{{2
            : node(pi, pdoc, type_PI)
        {
        }

        node_PI::~node_PI() // {{{2
        {
        }

        node * node_PI::clone(bool ) const // {{{2
        {
            return new node_PI(this->get_data(), this->p_doc);
        }

        void node_PI::accept(xml_visitor& v)
        {
            v.visit(this);
        }

    } // end of xml
} // end of sss
