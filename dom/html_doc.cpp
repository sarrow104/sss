#include "html_doc.hpp"

#include <algorithm>

#include "html_tag_factory.hpp"

namespace sss {

namespace dom {

html_doc::html_doc(html_doc::document_type_t t) : root(0), p_current(0) {
    // 强制系统调用注册函数；
    // 不然，系统可能忽略这种未用的静态变量__regist的构造；
    if (sss::html_tags::html::__regist) {
        this->root = html_tag_factory::create("html");
        this->p_current = root;
        this->root->set_htmldocument(this);
        this->p_document_type = new dom::document_type(t);
    }
}

html_doc::~html_doc() {
    delete root;
    delete p_document_type;
}

html_tags::html_tag * html_doc::create(const std::string& tag_name) {
    return html_tag_factory::create(tag_name);
}

html_tags::html_tag * html_doc::create_sub(const std::string& tag_name) {
    html_tags::html_tag * tmp_node = html_tag_factory::create(tag_name);
    this->p_current->add(tmp_node);
    this->p_current = tmp_node;
    return p_current;
}

html_tags::html_tag * html_doc::create_sibling(const std::string& tag_name, bool) {
    html_tags::html_tag * tmp_node = html_tag_factory::create(tag_name);
    p_current->get_parent()->add(tmp_node);
    p_current = tmp_node;
    return p_current;
}

bool html_doc::is_sub_node_of(html_tags::html_tag * sub, html_tags::html_tag * r) {
    if (!sub || !r) {
        return false;
    }
    html_tags::html_tag *sub_s_root = sub->get_root_tag();
    while (sub && sub != sub_s_root) {
        sub = sub->get_parent();
        if (sub == r) {
            return true;
        }
    }
    return false;
}

bool html_doc::swap_node(html_tags::html_tag * A, html_tags::html_tag * B) {
    // 首先，两个节点都必须是this->root的子节点？（其实，仅其中一个是，本操作也有意义）
    if (!is_sub_node_of(A, this->root) || !is_sub_node_of(B, this->root)) {
        return false;
    }
    // 如何定位父节点中elements中的iterator或者index？
    // 自动生成的异构子类，虽然有好处，少写很多代码；但是，造成了接口的混乱；
    // 这下完蛋了！不是所有类型的tag，都有elements成员的！难道要每种tag都
    // dynamic_cast一下吗？
    //
    // 再构建中键一层类型吗？――只要这个中间类型能支持elements成员？
    //
    // 主要是因为，我这个类族是用基于模板，正交构造的；其父类，除了html_tag，其
    // 实虽然看似相似，但其实由于模板参数不一样，所有在编译器看来，都是毫无关系
    // 的不同类型；
    //
    // 这些html_tags实体类，其唯一的血缘关系，就是基类html_tags::html_tag 了。
    //
    // 另外，是不是所有的标签，都可以自闭合？
    //
    // <p />
    // TODO
    //
    // 因此，要么，
    // 1. 我必须提供一个间接层！以提供一个支持 elements元素操作的公有界面！
    // 2. 我在html_tag下，提供一个swap函数？不行；html_tag element的不同，是通
    //    过虚函数vtab_ptr来区分的，而不是其他的数据；我不可能原地构造不同的对
    //    象！所以swap方法不可取；最好的办法，还是交换上层指针的值；
    // 我需要记录上层节点指针，还有内部插入位置的iterator；

    bool is_still_go = true;
    // std::swap(a, b);
    html_tags::html_tag::subnodes_t& a_subs = *A->get_parent()->get_subnodes();
    html_tags::html_tag::subnodes_t::iterator it_a = std::find(a_subs.begin(), a_subs.end(), A);
    if (it_a == a_subs.end()) {
        is_still_go = false;
    }

    html_tags::html_tag::subnodes_t& b_subs = *B->get_parent()->get_subnodes();
    html_tags::html_tag::subnodes_t::iterator it_b = std::find(b_subs.begin(), b_subs.end(), B);
    if (it_b == b_subs.end()) {
        is_still_go = false;
    }

    if (is_still_go) {
        std::swap(*it_b, *it_a);
    }

    return true;
}

void html_doc::print(std::ostream& o, const char * sep ){
    this->p_document_type->print(o, sep);
    this->get_root()->print(o, sep);
}

class predictor {
public:
    virtual ~predictor() {}

    virtual bool operator() (html_tags::html_tag* ) = 0;

    virtual bool needed_next() const = 0;
};

class id_predictor : public predictor {
    std::string to_find;
    bool has_found;

public:
    explicit id_predictor(const std::string& id) : to_find(id), has_found(false) {
    }

    bool operator()(html_tags::html_tag* tag) {
        if (tag->get_id() == to_find) {
            has_found = true;
        }
        return has_found;
    }

    bool needed_next() const {
        return !has_found;
    }
};

class class_predictor : public predictor {
    std::string to_find;

public:
    explicit class_predictor(const std::string& class_name) : to_find(class_name) {
    }

    bool operator()(html_tags::html_tag* tag) {
        return tag->get_class() == to_find;
    }

    bool needed_next() const {
        return true;
    }
};

class tag_name_predictor : public predictor {
    std::string to_find;

public:
    explicit tag_name_predictor(const std::string& tag_name) : to_find(tag_name) {
    }

    bool operator() (html_tags::html_tag* tag) {
        return tag->get_tag_name() == to_find;
    }

    bool needed_next() const {
        return true;
    }
};

html_doc::subnodes_t html_doc::get_element_by_id(const std::string& class_name) {
    id_predictor pre(class_name);

    return get_element_by_predictor(pre);
}

html_doc::subnodes_t html_doc::get_element_by_class(const std::string& id_name) {
    class_predictor pre(id_name);

    return get_element_by_predictor(pre);
}

html_doc::subnodes_t html_doc::get_element_by_tag(const std::string& tag_name) {
    tag_name_predictor pre(tag_name);

    return get_element_by_predictor(pre);
}

html_doc::subnodes_t html_doc::get_element_by_predictor(predictor& pre) {
    html_doc::subnodes_t ret;

    html_doc::subnodes_t calc_stack;
    std::vector<int> calc_index_stack;

    html_tags::html_tag * current = this->get_root();
    int index = 0;

    // 深度优先，遍历节点；
    do {
        // 当前节点，是否符合要求？
        if (pre(current)) {
            ret.push_back(current);
        }

        if (current->get_subnodes()) {
            //std::cout << "has sub" << std::endl;
            calc_index_stack.push_back(index);
            calc_stack.push_back(current);

            index = 0;
            current = current->get_subnodes()->at(index);
        }
        else {
            //std::cout << "no sub" << std::endl;
            ++index;
            html_tags::html_tag * father = current->get_parent();

            //std::cout << "father = " << father->get_tag_name() << std::endl;
            //std::cout << "number of siblings = " << father->get_subnodes()->size() << std::endl;
            //std::cout << "index = " << index << std::endl;

            // 如果是父节点最后一个儿子！
            if (int(father->get_subnodes()->size()) <= index) {
                if (!calc_stack.empty()) {
                    // 目的是到“下一个”节点；
                    // 但是下面的动作只是回到父亲节点；而真正的”下一个“节点，
                    // 其实是指父亲的兄弟，或者父亲的父亲的兄弟！
                    current = calc_stack.back();
                    calc_stack.pop_back();
                    index = calc_index_stack.back();
                    calc_index_stack.pop_back();

                    father = current->get_parent();

                    while (father && int(father->get_subnodes()->size()) == index + 1) {
                        current = calc_stack.back();
                        calc_stack.pop_back();
                        index = calc_index_stack.back();
                        calc_index_stack.pop_back();

                        //std::cout << "back to " << current->get_tag_name() << std::endl;
                        father = current->get_parent();
                    }

                    if (father) {
                        ++index;
                        current = father->get_subnodes()->at(index);
                    }
                    else {
                        current = 0;
                        index = 0;
                    }
                }
                else {
                    current = 0;
                    index = 0;
                }
            }
            else {
                current = father->get_subnodes()->at(index);
            }

        }
    } while (!calc_stack.empty() && current && pre.needed_next());

    return ret;
}

} // end of namespace dom

} // end of namespace sss

//<html>
// <head>
//  <title>test</title>
//  <style type="text/css">
//   .data{font-weight:normal;}
//   .head{font-weight:bold;}
//  </style>
// </head>
// <body>
//  <table>
//   <tr>
//    <td class="data">abc</td>
//   </tr>
//   <tr>
//    <td class="data">123</td>
//   </tr>
//  </table>
// </body>
//</html>
