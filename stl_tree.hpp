#ifndef  __STL_TREE_HPP_1331716578__
#define  __STL_TREE_HPP_1331716578__

//C++树的实现
//STL里面没有提供容器树的模板实现，从网上找到一个：
//
//本文来自CSDN博客，转载请标明出处：
//    http://blog.csdn.net/stephenxu111/archive/2008/05/14/2446382.aspx
//
//NOTE 2011-10-11
// Sarrow:
// 需要改进，不是POD版本。里面存放的就是一个简单的int

//tree.h 头文件

#include <list>
#include <algorithm>

namespace sss {


template <typename T> class Tree{
public:
    struct TreeNode;                                //定义一个结构体原形
    class  Iterator;                                //定义一个类原形
    typedef std::list<TreeNode*> List;              //重命名一个节点链表

    TreeNode* clone(TreeNode*, List&, TreeNode*);   //Clone复制函数

    struct TreeNode{
        T           _data;                          //数据
        TreeNode*   _parent;                        //父节点
        List        _children;                      //子节点

        TreeNode(const T & val, TreeNode* parent);  //构造函数
        TreeNode();  //构造函数

        void SetParent(TreeNode&);                  //设置父节点
        void InsertChildren(TreeNode&);             //插入子节点
    };

    //This is TreeSub Class Iterator
    class Iterator{
    private:
        Tree* _tree;                                //Tree data
        typename std::list<TreeNode*>::iterator _lit;        //List Iterator

    public:
        Iterator();                                 //默认构造函数
        Iterator(const Iterator&);                  //复制构造函数
        Iterator(Tree*, TreeNode*);                 //构造函数
        Iterator(Tree*, typename std::list<TreeNode *>::iterator it);  //构造函数
        //运算符重载
        void operator=(const Iterator&);            //赋值运算符重载
        bool operator==(const Iterator&);           //关系运算符重载
        bool operator!=(const Iterator&);           //关系运算符重载
        Iterator& operator++();                     //前缀++运算符
        Iterator  operator++(int);                  //后缀++运算符
        int  operator*()const;                      //获得节点信息
        bool operator!();                           //赋值运算符重载

        typedef typename std::list<TreeNode*>::iterator List;
        friend class Tree;
    };

public:

    //下面是构造器和运算符重载
    Tree();                                     // 默认构造函数
    Tree(const Tree&);                          // 复制构造函数
    Tree(const T & val );                       // 带参数构造函数
    Tree(const T & val, const std::list<Tree*>&);   // 带参数构造函数
    ~Tree();                                    // 析构函数

    Tree& operator=(const Tree&);               // =符号运算符重载
    bool operator==(const Tree&);               // ==符号运算符重载
    bool operator!=(const Tree&);               // !=符号运算符重载

    //下面是成员函数
    void        Clear();                        //清空
    bool        IsEmpty()const;                 //判断是否为空
    int         Size()const;                    //计算节点数目
    int         Leaves();                       //计算叶子数
    int         Root()const;                    //返回根元素
    int         Height();                       //计算树的高度

    //下面是静态成员函数
    static bool IsRoot(Iterator);               //判断是否是根
    static bool isLeaf(Iterator);               //判断是否是叶子
    static Iterator Parent(Iterator);           //返回其父节点
    static int  NumChildren(Iterator);          //返回其子节点数目

    //跌代器函数
    Iterator    begin();                        //Tree Begin
    Iterator    end();                          //Tree End
    friend class Iterator;                      //Iterator SubClass

private:
    std::list<TreeNode*>                 _nodes;                //节点数组
    typename std::list<TreeNode*>::iterator LIt;         //一个节点迭代器

private:
    int height(TreeNode*);
    int level(TreeNode*, Iterator);
};


} // end of namespace sss

namespace sss {
//***** 下面是对于TreeNode结构体的定义实现*****///

template<typename T>
Tree<T>::TreeNode::TreeNode(const T& val, Tree<T>::TreeNode* Parent)
    : _data(val), _parent(Parent)
{
}

template<typename T>
Tree<T>::TreeNode::TreeNode()
    : _parent(0)
{
}

template<typename T>
void Tree<T>::TreeNode::SetParent(Tree<T>::TreeNode& node)
{
    _parent = &node;
}

template<typename T>
void Tree<T>::TreeNode::InsertChildren(Tree<T>::TreeNode& node)
{
    _children.push_back(&node);
}


//***** 下面是对于Tree类的定义实现*****///
template<typename T>
Tree<T>::Tree()
{
}

template<typename T>
Tree<T>::Tree(const  T& val)
{
    _nodes.push_back(new Tree<T>::TreeNode(val));
}

template<typename T>
Tree<T>::Tree(const Tree& t)
{
    if(t._nodes.empty())return;
    clone(t._nodes.front(), _nodes, 0);
}

template<typename T>
Tree<T>::Tree(const T& val, const std::list<Tree<T> *>& lit)
{
    TreeNode* root = new TreeNode(val);        //建立根节点
    _nodes.push_back(root);                     //放入树中
    typename std::list<Tree*>::const_iterator it;
    for(it = lit.begin(); it!=lit.end(); it++)
    {
        if(!((*it)->_nodes.empty())){           //如果当前节点元素不为空
            Tree* tp = new Tree(**it);
            TreeNode* p = tp->_nodes.front();
            root->_children.push_back(p);       //设置根的子节点
            p->_parent = root;                  //设置节点的父节点为根
            typename std::list<TreeNode*>::iterator lit1 = tp->_nodes.begin();
            typename std::list<TreeNode*>::iterator lit2 = tp->_nodes.end();
            typename std::list<TreeNode*>::iterator lit3 = _nodes.end();
            _nodes.insert(lit3, lit1, lit2);
        }
    }
}

template<typename T>
Tree<T>::~Tree()
{
    for(typename std::list<typename Tree<T>::TreeNode*>::iterator it = _nodes.begin();
        it!=_nodes.end();
        it++)
    {
        delete (*it);
    }
    _nodes.clear();
}

template<typename T>
Tree<T>& Tree<T>::operator =(const Tree<T> & t)
{
    Clear();
    Tree<T>* p = new Tree<T>(t);
    _nodes = p->_nodes;
    return *this;
}

template<typename T>
bool Tree<T>::operator ==(const Tree<T>& t)
{
    if(_nodes.size()!=t._nodes.size())
    {
        return false;
    }
    typename std::list<typename Tree<T>::TreeNode*>::iterator it = _nodes.begin();
    typename std::list<typename Tree<T>::TreeNode*>::const_iterator _it = t._nodes.begin();
    while(it!=_nodes.end() && _it != t._nodes.end())
    {
        if((*it)->_data != (*_it)->_data)
        {
            return false;
        }
        it++;
        _it++;
    }
    return true;
}

template<typename T>
bool Tree<T>::operator !=(const Tree& t)
{
    if (this == &t)
    {
        return false;
    }
    else if(_nodes.size() != _nodes.size())
    {
        return true;
    }
    else
    {
        typename std::list<typename Tree<T>::TreeNode*>::iterator it = _nodes.begin();
        typename std::list<typename Tree<T>::TreeNode*>::const_iterator _it = t._nodes.begin();
        while(it!=_nodes.end() && _it != t._nodes.end())
        {
            if((*it)->_data != (*_it)->_data)
            {
                return true;
            }
            it++;
            _it++;
        }
        return false;
    }
}

template <typename T>
void Tree<T>::Clear()
{
    for(typename std::list<typename Tree<T>::TreeNode*>::iterator it = _nodes.begin(); it!=_nodes.end(); it++)
    {
        delete* it;
    }
    _nodes.clear();
}

template <typename T>
bool Tree<T>::IsEmpty()const{
    return _nodes.empty();
}

template <typename T>
int Tree<T>::Size()const{
    return static_cast<int>(_nodes.size());
}

// count leaves
template <typename T>
int Tree<T>::Leaves()
{
    int i = 0;
    for (typename std::list<typename Tree<T>::TreeNode*>::iterator it = _nodes.begin();
         it != _nodes.end();
         ++it)
    {
        if((*it)->_children.size()==0)
        {
            i++;
        }
    }
    //std::list<TreeNode*>::iterator it = _nodes.begin();
    //while(it!=_nodes.end()){
    //    if((*it)->_children.size()==0){
    //        i++;
    //    }
    //    it++;
    //}
    return i;
}

template <typename T>
int Tree<T>::Height()
{
    int h = 0;
    if(_nodes.size()!=0)
    {
        TreeNode* TNode = _nodes.front();
        h = this->height(TNode);
    }
    else{
        h = -1; //判断为空树
    }
    return h;
}

template <typename T>
int Tree<T>::height(typename Tree<T>::TreeNode* node)
{
    if(!node)
    {
        return -1;
    }
    else{
        std::list<TreeNode*> plist = node->_children;
        if(plist.size()==0)
        {
            return 0;
        }
        int hA = 0;
        for(typename std::list<typename Tree<T>::TreeNode*>::iterator it = plist.begin();
            it!=plist.end();
            it++)
        {
            int hB = height(*it);
            if(hB>hA)
            {
                hA = hB;
            }
        }
        return hA+1;
    }
}

template <typename T>
typename Tree<T>::Iterator Tree<T>::begin()
{
    return Iterator(this, _nodes.begin());
}

template <typename T>
typename Tree<T>::Iterator Tree<T>::end()
{
    return Iterator(this, _nodes.end());
}

template <typename T>
int Tree<T>::Root()const{
    return (*_nodes.begin())->_data;
}

template <typename T>
bool Tree<T>::IsRoot(Iterator it)
{
    TreeNode p(*it);
    return p._parent == 0;
}

template <typename T>
bool Tree<T>::isLeaf(Iterator it)
{
    TreeNode p(*it);
    return p._children.size() == 0;
}

template <typename T>
typename Tree<T>::Iterator Tree<T>::Parent(Iterator it)
{
    TreeNode p(*it);
    Tree* t = it._tree;
    Iterator Ite(t, p._parent);
    return Ite;
}

template <typename T>
int Tree<T>::NumChildren(Iterator it)
{
    TreeNode p(*it);
    return static_cast<int>(p._children.size());
}

//***** 下面是对于Tree::Iterator类的定义实现*****///
template <typename T>
Tree<T>::Iterator::Iterator()
{
}

template <typename T>
Tree<T>::Iterator::Iterator(const Iterator& it)
{
    _tree = it._tree;
    _lit = it._lit;
}

template <typename T>
Tree<T>::Iterator::Iterator(Tree* t, TreeNode* n)
{
    _tree = t;
    std::list<TreeNode*>& nodes = _tree->_nodes;
    _lit = std::find(nodes.begin(), nodes.end(), n);   //<algorithm> Members
}

template <typename T>
Tree<T>::Iterator::Iterator(Tree * t, typename std::list<TreeNode*>::iterator lt)
{
    _tree = t;
    _lit = lt;
}

template <typename T>
void Tree<T>::Iterator::operator =(const Tree<T>::Iterator& it)
{
    _tree = it._tree;
    _lit = it._lit;
}

template <typename T>
bool Tree<T>::Iterator::operator ==(const Tree<T>::Iterator & it)
{
    return _tree == it._tree && _lit == it._lit;
}

template <typename T>
bool Tree<T>::Iterator::operator !=(const Tree<T>::Iterator & it)
{
    return _tree != it._tree || _lit != it._lit;
}

template <typename T>
typename Tree<T>::Iterator& Tree<T>::Iterator::operator ++()
{
    ++_lit;
    return *this;
}

template <typename T>
typename Tree<T>::Iterator Tree<T>::Iterator::operator ++(int)
{
    Iterator it(*this);
    ++_lit;
    return it;
}

template <typename T>
int Tree<T>::Iterator::operator *() const{
    return ((*_lit)->_data);
}

template <typename T>
bool Tree<T>::Iterator::operator !()
{
    return _lit == _tree->_nodes.end();
}

//Clone函数 NOTE 非成员函数
template <typename T>
typename Tree<T>::TreeNode* clone(typename Tree<T>::TreeNode* node, typename Tree<T>::List& nodes, typename Tree<T>::TreeNode* nodep)
{
    typename Tree<T>::TreeNode* cp = new typename Tree<T>::TreeNode(node->_data, nodep);
    nodes.push_back(cp);
    typename Tree<T>::List& l = node->_children;
    typename Tree<T>::List& cl = cp->_children;
    for(typename std::list<typename Tree<T>::TreeNode*>::iterator lt = l.begin(); lt != l.end(); lt++)
    {
        cl.push_back(clone(*lt, nodes, cp));
    }
    return cp;
}

} // end of namespace sss
#endif  /* __STL_TREE_HPP_1331716578__ */
