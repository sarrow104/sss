#ifndef  __DLIST_HPP_1437622911__
#define  __DLIST_HPP_1437622911__

#include <algorithm>
#include <iostream>

namespace sss {
namespace can {

template<typename T>
class dlist
{
public:
    dlist() = default;
    dlist(const dlist&) = delete;
    dlist(dlist&&) noexcept = default;

    dlist& operator = (const dlist&) = delete;
    dlist& operator = (dlist&&) noexcept = default;

    ~dlist() = default;

//public:
    inline T * nextSibling() const
    {
        return _next;
    }

    inline T * prevSibling() const
    {
        return _prev;
    }

    bool append(T * node)
    {
        if (node && static_cast<T*>(this) != node) {
            node->unlinkBefore();
            T * last_node = node->lastSibling();

            if (this->_next) {
                this->_next->_prev = last_node;
                last_node->_next = this->_next;
            }
            node->_prev = static_cast<T*>(this);
            this->_next = node;
            return true;
        }
        return false;
    }

    bool insert(T * node)
    {
        if (node && static_cast<T*>(this) != node) {
            node->unlinkBefore();
            T * last_node = node->lastSibling();

            if (this->_prev) {
                this->_prev->_next = node;
                node->_prev = this->_prev;
            }
            last_node->_next = static_cast<T*>(this);
            this->_prev = last_node;
            return true;
        }
        return false;
    }

    void unlink()
    {
        if (this->_next) {
            this->_next->_prev = this->_prev;
        }
        if (this->_prev) {
            this->_prev->_next = this->_next;
        }

        this->_next = 0;
        this->_prev = 0;
    }

    void unlinkBefore()
    {
        if (this->_prev) {
            this->_prev->_next = 0;
        }
        this->_prev = 0;
    }

    void unlinkAfter()
    {
        if (this->_next) {
            this->_next->_prev = 0;
        }
        this->_next = 0;
    }

    T * firstSibling() const
    {
        const T * prev_node = static_cast<const T*>(this);
        while (prev_node && prev_node->_prev) {
            prev_node = prev_node->_prev;
        }
        return const_cast<T*>(prev_node);
    }

    T * lastSibling() const
    {
        const T * next_node = static_cast<const T*>(this);
        while (next_node && next_node->_next) {
            next_node = next_node->_next;
        }
        return const_cast<T*>(next_node);
    }

    // 不包括自己 TODO check
    T * nextListLength() const
    {
        T * node = this;
        T * fast_node = this;
        int cnt = 0;
        // 快慢指针，检查循环链表
        while (fast_node) {
            node = node->nextSibling();
            fast_node = fast_node->nextSibling();
            if (!fast_node) {
                return cnt * 2;
            }
            fast_node = fast_node->nextSibling();
            if (fast_node == node) {
                throw std::runtime_error("loop in next");
            }
            ++cnt;
        }
        return cnt * 2 - 1;
    }

    // 不包括自己 TODO check
    T * prevListLength() const
    {
        T * node = this;
        T * fast_node = this;
        int cnt = 0;
        // 快慢指针，检查循环链表
        while (fast_node) {
            node = node->prevSibling();
            fast_node = fast_node->prevSibling();
            if (!fast_node) {
                return cnt * 2;
            }
            fast_node = fast_node->prevSibling();
            if (fast_node == node) {
                throw std::runtime_error("loop in prev");
            }
            ++cnt;
        }
        return cnt * 2 - 1;
    }

    bool reverse()
    {
        T * prev_list = (this->_prev ? this->firstSibling() : 0);
        this->unlinkBefore();
        T * node = static_cast<T*>(this);
        while (node) {
            T * left = node->_next;
            std::swap(node->_prev, node->_next);
            if (!left) {
                break;
            }
            node = left;
        }

        if (prev_list) {
            T * prev_list_end = prev_list->lastSibling();
            prev_list_end->_next = node;
            node->_prev = prev_list_end;
        }
        return true;
    }

protected:
    T * _prev;
    T * _next;
};

} // namespace can
} // namespace sss


#endif  /* __DLIST_HPP_1437622911__ */
