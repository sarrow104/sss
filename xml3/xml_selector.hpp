#ifndef  __XML_SELECTOR_HPP_1445767680__
#define  __XML_SELECTOR_HPP_1445767680__

// xml节点选择器；支持CSS风格的选择；
//

// node_list_t 作为选区；它继承自 std::vector<node*>; 并且只有引用语义；
// 支持node_list_t find(const std::string& CSS_selector);
// node_list_t node::find();
// gumbo-query 的选择器，是在 CSelection,和 CNode的内部使用；用完就扔的那种；
// 而我可能需要数个、几十上百个(比如同构的小说网页)，都是同样的选择；
//
// 此时，就没有必要重复创建这个检索用树形结构；
//
// 这种需求下，最好是外部传入、控制……
//
// 另外，gumbo-query在根据id检索的时候，并没有优化；我的xml结构，专门针对id有使
// 用map；此时，就没有必要再从所有节点中顺序查找；
//
// TODO 父选择器——包含某子元素的父元素
// 语法：
// E! > F
//
// 参考：
//! http://www.cnblogs.com/ziyunfei/archive/2012/10/05/2710631.html
//! https://github.com/Idered/cssParentSelector
//
//E > F - selects an F element, child of E
//
//E! > F - selects an E element, parent of F
//
//注意，"!" 是与前面的E结合；并且，可以是任意的tag，用'*'表示；


#include "xml_node.hpp"

#include <string>
#include <vector>

#include <memory>

namespace sss {
    namespace xml3 {

/**
 * 选择器-基类
 */
class Selector
{

public:
    typedef enum
    {
        /**
         * place holer
         */
        EDummy,
        /**
         * match none element node which has no sub-text or sub-elements!
         */
        EEmpty,
        /**
         * the only leaf child of parent
         */
        EOnlyChild,
        /**
         * the Nth child [mA, mB)
         */
        ENthChild,
        /**
         * math node by exactly tag-name
         */
        ETag,
    } TOperator;
public:

    Selector(TOperator aOp = EDummy)
    {
        init();
        mOp = aOp;
    }

    explicit Selector(bool aOfType)
    {
        init();
        mOp = EOnlyChild;
        mOfType = aOfType;
    }

    Selector(unsigned int aA, unsigned int aB, bool aLast, bool aOfType)
    {
        init();
        mOp = ENthChild;
        mA = aA;
        mB = aB;
        mLast = aLast;
        mOfType = aOfType;
    }

    explicit Selector(std::string aTag)
    {
        init();
        mOp = ETag;
        mTag = aTag;
    }

    virtual ~Selector()
    {
    }

public:

    virtual bool match(node* apNode);

    node_list_t filter(node_list_t nodes);

    node_list_t matchAll(node * apNode);

private:

    void init()
    {
        mOfType = false;
        mA = 0;
        mB = 0;
        mLast = false;
    }

    void matchAllInto(node * apNode, node_list_t& nodes);

private:

    TOperator mOp;

    bool mOfType;

    unsigned int mA;

    unsigned int mB;

    bool mLast;

    std::string mTag;
};

/**
 * 一元选择器
 */
class UnarySelector: public Selector
{
public:
    typedef enum
    {
        /**
         * 不满足选择条件的节点；取反；
         */
        ENot,
        /**
         * 含有满足条件的任意子孙节点的节点
         */
        EHasDescendant,
        /**
         * 含有满足条件孩子节点的节点
         */
        EHasChild,
    } TOperator;

public:

    UnarySelector(TOperator aOp, Selector* apS);

    virtual ~UnarySelector();

public:

    virtual bool match(node* apNode);

private:

    bool hasDescendantMatch(node* apNode, Selector* apS);

    bool hasChildMatch(node* apNode, Selector* apS);

private:

    Selector* mpS;

    TOperator mOp;
};

/**
 * 二元选择器
 */
class BinarySelector: public Selector
{
public:
    typedef enum
    {
        /**
         * || 二选一，或者，操作符
         */
        EUnion,
        /**
         * && 交集，同时符合两个条件 操作符
         */
        EIntersection,
        /**
         * 父子关系同时满足 操作符
         */
        EChild,
        /**
         * 某祖先满足，当前也满足 操作符
         */
        EDescendant,
        /**
         * 兄长节点以及当前，同时满足 操作符<br />
         *  mAdjacent 用来决定是否挨在一起
         */
        EAdjacent,
    } TOperator;

public:

    BinarySelector(TOperator aOp, Selector* apS1, Selector* apS2);

    BinarySelector(Selector* apS1, Selector* apS2, bool aAdjacent);

    ~BinarySelector();

public:

    virtual bool match(node* apNode);

private:

    Selector* mpS1;

    Selector* mpS2;

    TOperator mOp;

    bool mAdjacent;
};

/**
 * 属性选择器<br />
 * 某名字的属性，是否符合值的的特点
 */
class AttributeSelector: public Selector
{
public:
    typedef enum
    {
        /**
         * 是否存在任何属性
         */
        EExists,
        /**
         * 属性是否与特征串相等
         */
        EEquals,
        /**
         * 属性值 是否包含 特征串
         */
        EIncludes,
        /**
         * 属性值是否形如 "特征串-"
         */
        EDashMatch,
        /**
         * 是否以特征串为前缀
         */
        EPrefix,
        /**
         * 是否以特征串为后缀
         */
        ESuffix,
        /**
         * 是否以子串包含特征串
         */
        ESubString,
    } TOperator;

public:

    AttributeSelector(TOperator aOp, std::string aKey, std::string aValue = "");

public:

    virtual bool match(node* apNode);

private:

    std::string mKey;

    std::string mValue;

    TOperator mOp;
};

/**
 * 按内部文本是否符合提供值，检索；
 */
class TextSelector: public Selector
{
public:
    typedef enum
    {
        /**
         * 仅当前文本符合条件
         */
        EOwnContains,
        /**
         * 内部所有文本，满足条件
         */
        EContains,
    } TOperator;

public:
    TextSelector(TOperator aOp, std::string aValue)
    {
        mValue = aValue;
        mOp = aOp;
    }

    ~TextSelector()
    {
    }

public:

    virtual bool match(node* apNode);

private:

private:

    std::string mValue;

    TOperator mOp;
};

    }
}

#endif  /* __XML_SELECTOR_HPP_1445767680__ */

