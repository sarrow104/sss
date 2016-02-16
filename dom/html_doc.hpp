#ifndef  __HTML_DOC_HPP_1391063635__
#define  __HTML_DOC_HPP_1391063635__

#include "html_tag.hpp"
#include "tag_element.hpp"
#include "dom_node.hpp"

#include <string>

namespace sss {

namespace dom {

class predictor;

class html_doc {
public:
    typedef html_tags::html_tag::subnodes_t subnodes_t;
    typedef sss::dom::document_type::document_type_t document_type_t;

    // ���ȣ���һ����װ�ࣺ
    // class html_doc {
    //    html_tags::html root;     // html�ĵ�������ڵ�
    //    html_tag *p_current;      // ��ǰ�����ڵ㣻
    //
    //    // �ڵ�ǰ�ڵ��£���������λ���ض����͵��ӽڵ㣻
    //    html_doc& create(const std::string& tag_name);
    //
    //    html_tag* locat_father(); // ���ڵ�
    //    html_tag* locat_sub(const std::string& tag_name); // �ض������ӽڵ�
    //    html_tag* locat_sub(int index);                   // ��index���ӽڵ�
    //    html_tag* locat_sibling(bool is_down);            // ���¡�����һ���ֵܽڵ�
    //
    //    html_tag* search_by_tag(const std::string& tag_name);
    //    html_tag* search_by_id(const std::string& id_name);
    //    html_tag* search_by_class(const std::string& class_name);
    //    // �������������߼��ģ������������������classΪ"xxx"�ģ����Ǹ���ȵģ�
    //    // tagΪ"div"�Ľڵ�����Ľڵ㣻
    //
    //    // ���⣬��������Ҫע�����������Լ�������ʽ��������ȡ�������ȣ�
    //
    // };
public:
    // �ڲ��������������صĶ�������֧��iterator������
    class tag_locator {
    public:
        enum locate_method_t {
            locate_by_tag = 0,
            locate_by_id,
            locate_by_class
        };

    public:
        tag_locator(const std::string& key, locate_method_t method, html_tags::html_tag * root);
    };

private:
    html_tags::html_tag *root;                  // html�ĵ�������ڵ�
    html_tags::html_tag *p_current;             // ��ǰ�����ڵ㣻
    dom::document_type  *p_document_type;

public:
    html_doc(html_doc::document_type_t type = sss::dom::document_type::XHTML10_STRICT);

    ~html_doc();

protected:
    bool is_sub_node_of(html_tags::html_tag *, html_tags::html_tag *);

public:
    // ���������Ľڵ㣻
    html_tags::html_tag * create(const std::string& tag_name);

    // �ڵ�ǰ�ڵ����棬�����ӽڵ�
    html_tags::html_tag * create_sub(const std::string& tag_name);

    // �����ֵܽڵ㣻�������£�Ĭ��
    // direction:
    //  true ����;
    //  false ����;
    html_tags::html_tag * create_sibling(const std::string& tag_name, bool direction = true);

    bool swap_node(html_tags::html_tag *, html_tags::html_tag *);

    html_tags::html_tag * get_root() const {
        return this->root;
    }

    html_tags::html_tag * get_current() const {
        return this->p_current;
    }

    html_tags::html_tag * to_parent() {
        html_tags::html_tag * tmp = this->p_current->get_parent();
        if (tmp) {
            this->p_current = tmp;
        }
        return tmp;
    }

    void print(std::ostream& o, const char * sep = "\t");

    html_tags::html_tag * locate_sub(int index) {
        this->p_current = this->p_current->get_subnodes()->at(index);
        return p_current;
    }

    subnodes_t get_element_by_tag(const std::string& tag_name);
    subnodes_t get_element_by_id(const std::string& class_name);
    subnodes_t get_element_by_class(const std::string& id_name);

protected:
    subnodes_t get_element_by_predictor(predictor& pre);

    //html_tags::html_tag* locate_father(); // ���ڵ�
    //html_tags::html_tag* locate_sub(const std::string& tag_name); // �ض������ӽڵ�
    //html_tags::html_tag* locate_sub(int index);                   // ��index���ӽڵ�
    //html_tags::html_tag* locate_sibling(bool is_down);            // ���¡�����һ���ֵܽڵ�

    // TODO
    // һ�� html_doc �У�һ��id���϶�ֻ�ܶ�Ӧһ��tag�ڵ㡣��ʹ�� search_by_id
    // ������ʱ��Ҫô���ص�һ�������ģ�id���ԵĽڵ㣬Ҫô�������һ����
    //
    // ��Ȼ������һ�ַ�ʽ�����ǽػ���ڲ�html�ڵ�����Ը�ֵ������һ������ĳidֵ
    // �ظ�ʹ�ã������֮ǰһ����ֵ�����Ա�֤idֵ��ӦΨһhtml�ڵ㡪����Ȼ��Ҳ��
    // �Է��������ú����ĸ�ֵʧЧ��
    //
    // ��֮��html_doc ����֮�У���Ҫ����һ��map�ṹ���Խ�����id������Ľڵ��һ
    // һӳ�䣻
    //html_tags::html_tag* search_by_id(const std::string& id_name);
    //
    // NOTE
    //
    // �������������������ǣ�һ�Զ�ģ��Ҿ��ã�Ӧ�÷���һ���ڵ���󣻱���subnodes_t
    //
    // subnodes_t search_by_tag(const std::string& tag_name);
    //
    // subnodes_t earch_by_class(const std::string& class_name);
    //
    // ���� subnodes_t ��ζ�ŷ��ؼ������������һ��ѡ���ǣ�����һ���ӳټ���Ķ�
    // ������¼�˼�����ʽ���Լ���ǰö�ٵ�λ�ã�
    //
    // ǰ�ߵĺô��ǣ��������أ����뷽�㣻���ߣ��߿��Դﵽ���д����Ч��������ԭ
    // ʼdom���޸ģ������������ڵ�������ӳټ�����ĵ��ù����ϣ��������������
    // �Ը���Щ��
    //
    // ��ô��ͬid����ʽ���ƣ������ּ�����ʽ�費��ҪҲ��һ������ṹ����Ȼ����
    // ���϶�����multmap�����棻
    //
    // �����Ļ���ֻ��Ҫ����һ��std::pair<iterator, iterator> ��ϣ����ܷ�ӳ���
    // ���ˡ�
    //
    // �������������߼��ģ������������ ��Ķ�λ��������js����css��������class
    // Ϊ"xxx"�ģ����Ǹ���ȵģ�tagΪ"div"�Ľڵ�����Ľڵ㣻
    //
    // ���⣬��������Ҫע�����������Լ�������ʽ��������ȡ�������ȣ�
    //
};

#if 0
void test_html_doc() {
    html_doc my_doc("my.htm");
    // ��ʱ���ṹ��html_tag::html root; ���󣻲������޸������ԣ�
    // ���ң�html_tag * p_current = & root;
    //
    // �ؼ����ڷ���Ĺ����ӽڵ㣻
    //
    // my_doc.create_sub("div");
    //
    // �൱�ڣ�
    //
    // > html_tag * tmp_node = html_factory::create("div");
    // > p_current->add(tmp_node);
    // > p_current = tmp_node;
    //
    // my_doc.create_sibling("table", dir = false);
    //
    // �൱�ڣ�
    //
    // > html_tag * tmp_node = html_factory::create("table");
    // > p_current->get_parent()->add(tmp_node);
    // > p_current = tmp_node;
    //
    // TODO
    // ��ν����ڵ㣿
    //
    // ��򵥵�����ǣ������ڵ����ֵܽڵ㣻
    //                  ֱ��swap���ɣ�
    //
    // ����ǣ������ڵ�û�л�Ϊ���ȹ�ϵ��
    //                  ����swap���ɣ�
    //
    // ����������ڵ㻥Ϊ���ȣ�
    //                  ���ȣ�������ڵ������ӹ�ϵ��Ȼ����������ڵ㣬��
    //                  sibling����ķ�ʽ���嵽����һ���ڵ�ĸ��ڵ����棻
    //                  ���swap�������ڵ㣻
    //
    // ���ϣ������ڵ�Ľ������ͱ����һ�ֽӿڣ�
    //                  �ȼ��������ڻ�Ϊ����Ĺ�ϵ����ô�����ع���ʹ���Ϊ��
    //                  �ܽڵ��ϵ��
    //                  ��󣬽���swap����������ɽ�����
    //
    // "֧�ֽ��������ĳ�����"
    //                  ��Ҫ��¼ԭʼλ�ã�������Ҫ��������ϵͳ��֧�ֲ��У�
    my_doc;
}

#endif

} // end of namespace dom

} // end of namespace sss

#endif  /* __HTML_DOC_HPP_1391063635__ */
