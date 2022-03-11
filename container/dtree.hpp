#ifndef  __DTREE_HPP_1437623293__
#define  __DTREE_HPP_1437623293__

#include <assert.h>

#include <sss/container/dlist.hpp>

// NOTE
// 使用者，创建的实体类，其方法，如果与本基类方法同名，会将其覆盖掉；
// 此时，若一定要使用基类的方法，就算使用 sss::can::dtree<T>::func() 这样的绝对
// 定位，都不好使！

namespace sss {
namespace can{

template <typename T>
class Handle
{
public:
    explicit Handle(T * val) : data(val)
    {
    }

    explicit Handle(const T& val) : data(&val)
    {
    }

    Handle(const Handle & ref)
        : data(ref.data) {}

    Handle & operator = (const Handle & ref)
    {
        if (this != &ref) {
            this->data = ref.data;
        }
    }

    ~Handle() = default;

    Handle firstChild()
    {
        return Handle(data ? data->firstChild() : 0);
    }

    Handle lastChild()
    {
        return Handle(data ? data->lastChild() : 0);
    }

    Handle nextSibling()
    {
        return Handle(data ? data->nextSibling() : 0);
    }

    Handle prevSibling()
    {
        return Handle(data ? data->prevSibling() : 0);
    }

    Handle parent()
    {
        return Handle(data ? data->parent() : 0);
    }

    Handle findChild(int type, int times = 0)
    {
        return Handle(data ? data->findChild(type, times) : 0);
    }

    Handle findSibling(int type, int times = 0)
    {
        return Handle(data ? data->findSibling(type, times) : 0);
    }

    T * get()
    {
        return data;
    }

public:
    T * data;
};

template<typename IntType, typename T>
class findbytype
{
public:
    findbytype(IntType type)
        : _type(type) {}

    ~findbytype() = default;

public:
    bool is_type(IntType type) const
    {
        return this->_type == type;
    }

    IntType  type() const
    {
        return this->_type;
    }

    // 参见：
    // /home/sarrow/project/epub/src/simpledoc.cpp|92
    inline T * findChild(IntType type, int times = 0)
    {
        if (times >= 0) {
            for (T * ele = static_cast<T*>(this)->firstChild();
                 ele;
                 ele = ele->nextSibling())
            {
                if (ele->is_type(type))
                {
                    if ((times--) == 0) {
                        return ele;
                    }
                }
            }
        }
        else {
            for (T * ele = static_cast<T*>(this)->lastChild();
                 ele;
                 ele = ele->prevSibling())
            {
                if (ele->is_type(type))
                {
                    if (++times == 0) {
                        return ele;
                    }
                }
            }
        }
        return 0;
    }

    inline T * findSibling(IntType type, int times = 0)
    {
        if (times >= 0) {
            for (T * ele = static_cast<T*>(this)->nextSibling();
                 ele;
                 ele = ele->nextSibling())
            {
                if (ele->is_type(type))
                {
                    if ((times--) == 0) {
                        return ele;
                    }
                }
            }
        }
        else {
            for (T * ele = static_cast<T*>(this)->prevSibling();
                 ele;
                 ele = ele->prevSibling())
            {
                if (ele->is_type(type))
                {
                    if (++times == 0) {
                        return ele;
                    }
                }
            }
        }
        return 0;
    }

protected:
    IntType _type;
};

template<typename T>
class dtree : public sss::can::dlist<T>
{
public:
    using Base_t = sss::can::dlist<T>;

public:
    dtree()
        : _parent(0),
          _firstChild(0),
          _lastChild(0)
    {}

    explicit dtree(T * p_parent)
        : _parent(p_parent),
          _firstChild(0),
          _lastChild(0)
    {}

    virtual ~dtree()
    {
        this->clear();
    }

public:
    void clear() {
        while (this->lastChild()) {
            this->removeChild(this->lastChild());
        }
    }

public:

    inline T * parent() const
    {
        return static_cast<T*>(_parent);
    }

    inline T * parent(T * p)
    {
        std::swap(p, this->_parent);
        return p;
    }

    inline T * nextSibling() const
    {
        return static_cast<T*>(this->Base_t::nextSibling());
    }

    inline T * prevSibling() const
    {
        return static_cast<T*>(this->Base_t::prevSibling());
    }

    inline T * firstSibling() const
    {
        return static_cast<T* >(this->Base_t::firstSibling());
    }

    inline T * lastSibling() const
    {
        return static_cast<T* >(this->Base_t::lastSibling());
    }

    inline T * firstChild() const
    {
        return static_cast<T*>(this->_firstChild);
    }

    inline T * lastChild() const
    {
        return static_cast<T*>(this->_lastChild);
    }

    bool hasChildren() const {
        return this->_firstChild;
    }

    ssize_t size() const {
        int cnt = 0;
        T * node = this->firstChild();
        while (node) {
            node = node->nextSibling();
            ++cnt;
        }
        return cnt;
    }

    bool reverse()
    {
        bool ret = this->Base_t::reverse();
        if (this->_parent) {
            std::swap(this->_parent->_firstChild,
                      this->_parent->_lastChild);
        }
        return ret;
    }

    bool insertChild(T * node) {
        return this->insertChild(node, 0);
    }

    virtual bool insertChild(T * node, T * beforeThis) {
        if (!node) {
            throw std::runtime_error("null node.");
        }
        if (node->nextSibling() || node->prevSibling() ) {
            throw std::runtime_error("not signle node");
        }

        beforeThis = beforeThis ? beforeThis : this->_firstChild;
        if (beforeThis) {
            if (beforeThis->parent() != static_cast<T*>(this)) {
                //beforeThis->print(std::cout, beforeThis->s_beg);
                throw std::runtime_error("different parent.");
            }
            beforeThis->Base_t::insert(node);
            this->_firstChild = this->_firstChild->firstSibling();
        }
        else {
            this->_firstChild = node;
            this->_lastChild = node;
        }
        node->parent(static_cast<T*>(this));
        return true;
    }

    bool appendChild(T* node) {
        return this->appendChild(node, 0);
    }

    virtual bool appendChild(T * node, T * afterThis) {
        if (!node) {
            throw std::runtime_error("null node.");
        }
        if (node->nextSibling() || node->prevSibling() ) {
            throw std::runtime_error("not signle node");
        }

        afterThis = afterThis ? afterThis : this->_lastChild;
        if (afterThis) {
            if ( afterThis->parent() != static_cast<T*>(this)) {
                //assert(afterThis->parent());
                //std::cout << "afterThis=";
                //afterThis->print(std::cout, afterThis->s_beg);
                ////afterThis->parent()->print(std::cout, afterThis->parent()->s_beg);
                //std::cout << "this=";
                //static_cast<T*>(this)->print(std::cout, static_cast<T*>(this)->s_beg);
                //std::cout << "node=";
                //node->print(std::cout, node->s_beg);
                throw std::runtime_error("different parent.");
            }
            afterThis->Base_t::append(node);
            this->_lastChild = this->_lastChild->lastSibling();
        }
        else {
            this->_firstChild = node;
            this->_lastChild = node;
        }
        node->parent(static_cast<T*>(this));
        return true;
    }

    T * unlinkTree()
    {
        // TODO
        // 是否需要将孤立子树和孤立当前子树根节点，然后将子树节点，往上挪呢？
        // 操作无非是整链的插入，还有循环parent(T*)操作即可；
        return this->unlink();
    }

    bool removeChild(T * node)
    {
        if (!node) {
            throw std::runtime_error("null node.");
        }
        if (node->parent() != this ) {
            throw std::runtime_error("not child!");
        }

        delete node->unlink();

        return true;
    }

    bool isChild(T * node) {
        return node->parent() == this;
    }

    T * unlink() {
        if (this->parent()) {
            if (this->parent()->firstChild() == static_cast<T*>(this)) {
                this->parent()->_firstChild = this->parent()->_firstChild->nextSibling();
            }
            if (this->parent()->lastChild() == static_cast<T*>(this)) {
                this->parent()->_lastChild  = this->parent()->_lastChild->prevSibling();
            }
        }
        this->Base_t::unlink();
        this->parent(0);
        return static_cast<T*>(this);
    }

protected:
    T * _parent;
    T * _firstChild;
    T * _lastChild;
};

} // namespace can
} // namespace sss


#endif  /* __DTREE_HPP_1437623293__ */
