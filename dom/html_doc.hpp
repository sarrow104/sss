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

    // 首先，是一个包装类：
    // class html_doc {
    //    html_tags::html root;     // html文档对象根节点
    //    html_tag *p_current;      // 当前操作节点；
    //
    //    // 在当前节点下，创建并定位到特定类型的子节点；
    //    html_doc& create(const std::string& tag_name);
    //
    //    html_tag* locat_father(); // 父节点
    //    html_tag* locat_sub(const std::string& tag_name); // 特定类型子节点
    //    html_tag* locat_sub(int index);                   // 第index个子节点
    //    html_tag* locat_sibling(bool is_down);            // 向下、向上一个兄弟节点
    //
    //    html_tag* search_by_tag(const std::string& tag_name);
    //    html_tag* search_by_id(const std::string& id_name);
    //    html_tag* search_by_class(const std::string& class_name);
    //    // 这种搜索，更高级的，可以组合起来，比如class为"xxx"的，在那个深度的，
    //    // tag为"div"的节点下面的节点；
    //
    //    // 另外，搜索还需要注意搜索方向以及搜索方式（深度优先、广度优先）
    //
    // };
public:
    // 内部检索函数，返回的对象；用来支持iterator操作；
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
    html_tags::html_tag *root;                  // html文档对象根节点
    html_tags::html_tag *p_current;             // 当前操作节点；
    dom::document_type  *p_document_type;

public:
    html_doc(html_doc::document_type_t type = sss::dom::document_type::XHTML10_STRICT);

    ~html_doc();

protected:
    bool is_sub_node_of(html_tags::html_tag *, html_tags::html_tag *);

public:
    // 创建孤立的节点；
    html_tags::html_tag * create(const std::string& tag_name);

    // 在当前节点下面，创建子节点
    html_tags::html_tag * create_sub(const std::string& tag_name);

    // 创建兄弟节点；方向向下，默认
    // direction:
    //  true 向下;
    //  false 向上;
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

    //html_tags::html_tag* locate_father(); // 父节点
    //html_tags::html_tag* locate_sub(const std::string& tag_name); // 特定类型子节点
    //html_tags::html_tag* locate_sub(int index);                   // 第index个子节点
    //html_tags::html_tag* locate_sibling(bool is_down);            // 向下、向上一个兄弟节点

    // TODO
    // 一个 html_doc 中，一个id，肯定只能对应一个tag节点。在使用 search_by_id
    // 函数的时候，要么返回第一个遇到的，id属性的节点，要么返回最后一个；
    //
    // 当然，还有一种方式，就是截获对内部html节点的属性赋值操作；一旦发现某id值
    // 重复使用，就清掉之前一个赋值——以保证id值对应唯一html节点——当然，也可
    // 以反过来，让后续的赋值失效；
    //
    // 总之，html_doc 对象之中，需要保存一个map结构，以建立从id到具体的节点的一
    // 一映射；
    //html_tags::html_tag* search_by_id(const std::string& id_name);
    //
    // NOTE
    //
    // 以下两个检索函数都是，一对多的；我觉得，应该返回一个节点对象；比如subnodes_t
    //
    // subnodes_t search_by_tag(const std::string& tag_name);
    //
    // subnodes_t earch_by_class(const std::string& class_name);
    //
    // 返回 subnodes_t 意味着返回检索结果；另外一个选择是，返回一个延迟计算的对
    // 象；它记录了检索方式，以及当前枚举的位置；
    //
    // 前者的好处是，立即返回，编码方便；后者，者可以达到并行处理的效果，并且原
    // 始dom的修改，可以在体现在迭代这个延迟计算体的调用过程上；不过，编码就相
    // 对复杂些；
    //
    // 那么，同id处理方式类似，这两种检索方式需不需要也做一个缓存结构？当然，这
    // 个肯定是用multmap来缓存；
    //
    // 这样的话，只需要返回一个std::pair<iterator, iterator> 组合，就能反映结果
    // 集了。
    //
    // 这种搜索，更高级的，可以组合起来 后的定位器（用于js或者css），比如class
    // 为"xxx"的，在那个深度的，tag为"div"的节点下面的节点；
    //
    // 另外，搜索还需要注意搜索方向以及搜索方式（深度优先、广度优先）
    //
};

#if 0
void test_html_doc() {
    html_doc my_doc("my.htm");
    // 此时，会构建html_tag::html root; 对象；并可以修改其属性；
    // 并且，html_tag * p_current = & root;
    //
    // 关键在于方便的构建子节点；
    //
    // my_doc.create_sub("div");
    //
    // 相当于：
    //
    // > html_tag * tmp_node = html_factory::create("div");
    // > p_current->add(tmp_node);
    // > p_current = tmp_node;
    //
    // my_doc.create_sibling("table", dir = false);
    //
    // 相当于：
    //
    // > html_tag * tmp_node = html_factory::create("table");
    // > p_current->get_parent()->add(tmp_node);
    // > p_current = tmp_node;
    //
    // TODO
    // 如何交换节点？
    //
    // 最简单的情况是，两个节点是兄弟节点；
    //                  直接swap即可；
    //
    // 其次是，两个节点没有互为祖先关系；
    //                  还是swap即可；
    //
    // 最后是两个节点互为祖先；
    //                  首先，将子孙节点解除连接关系；然后把这个子孙节点，用
    //                  sibling插入的方式，插到另外一个节点的父节点下面；
    //                  最后，swap这两个节点；
    //
    // 综上，两个节点的交换，就变成了一种接口：
    //                  先检测如果存在互为子孙的关系，那么进行重构；使其变为兄
    //                  弟节点关系；
    //                  最后，进行swap动作，以完成交换；
    //
    // "支持交换动作的撤销吗？"
    //                  需要记录原始位置；——需要整个撤销系统的支持才行；
    my_doc;
}

#endif

} // end of namespace dom

} // end of namespace sss

#endif  /* __HTML_DOC_HPP_1391063635__ */
