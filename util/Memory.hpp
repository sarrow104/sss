#ifndef  __MEMORY_HPP_1450786791__
#define  __MEMORY_HPP_1450786791__

#include <algorithm>
#include <cassert>

namespace sss {

template <typename T>
class scoped_ptr
{
public:
    scoped_ptr(T *p = 0)
        :m_px(p)
    {
    }

    //从auto_ptr获得指针的管理权
    // scoped_ptr(std::auto_ptr<T> p) :m_px(p.release())
    // {}

    ~scoped_ptr()
    {
        delete m_px;
    }

public:
    // 删除原来的指针，保存新的指针
    void reset(T * p = 0)
    {
        assert(p == 0 || p != m_px);
        scoped_ptr<T>(p).swap(*this);
    }

    T& operator*()const
    {
        assert(m_px != 0);
        return *m_px;
    }

    T* operator->()const
    {
        assert(m_px != 0);
        return m_px;
    }

    operator void * () const
    {
        return m_px;
    }

    T* get() const
    {
        return m_px;
    }

    T* release()
    {
        T * tmp = 0;
        std::swap(tmp, this->m_px);
        return tmp;
    }

    void swap(scoped_ptr & b)
    {
        T *tmp = b.m_px;
        b.m_px = m_px;
        m_px = tmp;
    }

private:
    scoped_ptr(scoped_ptr const &);
    scoped_ptr& operator=(scoped_ptr const &);

    void operator==(scoped_ptr const &)const;
    void operator!=(scoped_ptr const &)const;

private:
    T * m_px;
};

}


#endif  /* __MEMORY_HPP_1450786791__ */
