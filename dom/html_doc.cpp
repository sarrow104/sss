#include "html_doc.hpp"

#include <algorithm>

#include "html_tag_factory.hpp"

namespace sss {

namespace dom {

html_doc::html_doc(html_doc::document_type_t t) : root(0), p_current(0) {
    // ǿ��ϵͳ����ע�ắ����
    // ��Ȼ��ϵͳ���ܺ�������δ�õľ�̬����__regist�Ĺ��죻
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
    // ���ȣ������ڵ㶼������this->root���ӽڵ㣿����ʵ��������һ���ǣ�������Ҳ�����壩
    if (!is_sub_node_of(A, this->root) || !is_sub_node_of(B, this->root)) {
        return false;
    }
    // ��ζ�λ���ڵ���elements�е�iterator����index��
    // �Զ����ɵ��칹���࣬��Ȼ�кô�����д�ܶ���룻���ǣ�����˽ӿڵĻ��ң�
    // �����군�ˣ������������͵�tag������elements��Ա�ģ��ѵ�Ҫÿ��tag��
    // dynamic_castһ����
    //
    // �ٹ����м�һ�������𣿡���ֻҪ����м�������֧��elements��Ա��
    //
    // ��Ҫ����Ϊ��������������û���ģ�壬��������ģ��丸�࣬����html_tag����
    // ʵ��Ȼ�������ƣ�����ʵ����ģ�������һ���������ڱ��������������Ǻ��޹�ϵ
    // �Ĳ�ͬ���ͣ�
    //
    // ��Щhtml_tagsʵ���࣬��Ψһ��ѪԵ��ϵ�����ǻ���html_tags::html_tag �ˡ�
    //
    // ���⣬�ǲ������еı�ǩ���������Ապϣ�
    //
    // <p />
    // TODO
    //
    // ��ˣ�Ҫô��
    // 1. �ұ����ṩһ����Ӳ㣡���ṩһ��֧�� elementsԪ�ز����Ĺ��н��棡
    // 2. ����html_tag�£��ṩһ��swap���������У�html_tag element�Ĳ�ͬ����ͨ
    //    ���麯��vtab_ptr�����ֵģ����������������ݣ��Ҳ�����ԭ�ع��첻ͬ�Ķ�
    //    ������swap��������ȡ����õİ취�����ǽ����ϲ�ָ���ֵ��
    // ����Ҫ��¼�ϲ�ڵ�ָ�룬�����ڲ�����λ�õ�iterator��

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

    // ������ȣ������ڵ㣻
    do {
        // ��ǰ�ڵ㣬�Ƿ����Ҫ��
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

            // ����Ǹ��ڵ����һ�����ӣ�
            if (int(father->get_subnodes()->size()) <= index) {
                if (!calc_stack.empty()) {
                    // Ŀ���ǵ�����һ�����ڵ㣻
                    // ��������Ķ���ֻ�ǻص����׽ڵ㣻�������ġ���һ�����ڵ㣬
                    // ��ʵ��ָ���׵��ֵܣ����߸��׵ĸ��׵��ֵܣ�
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
