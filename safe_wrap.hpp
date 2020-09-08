#pragma once

#include <mutex>
#include <memory>

namespace sss {

template
    <
        typename Value,
        typename Mutex = std::mutex,
        typename SharedPtr = std::shared_ptr<Value>
    >
class safe_wrap
{
public:
    typedef Value     value_type;
    typedef SharedPtr value_ptr_type;
    typedef Mutex     mutex_type;
    typedef std::lock_guard<Mutex> write_lock_type;

private:
    typedef safe_wrap this_type;

public:
    safe_wrap(void)
        :
            _ptr()
    {}

    safe_wrap(value_type* pval)
        :
            _ptr (std::make_shared<value_type>(pval))
    {}

    safe_wrap(value_type&& val)
        :
            _ptr (new value_type(std::forward<value_type>(val)))
    {}

    safe_wrap(const value_type& val)
        :
            _ptr (new value_type(val))
    {}

    safe_wrap(const value_ptr_type& base)
        :
            _ptr (base)
    {}

    safe_wrap(this_type&& right)
    {
        this_type& right_ref = right;
        right_ref.swap(_ptr);
    }

    safe_wrap(const this_type& right)
    {
        value_ptr_type tmp = std::make_shared<value_type>(right.load());
        _ptr.swap(tmp);
    }

    ~safe_wrap()
    {}

    this_type& operator=(value_type&& val)
    {
        this_type::store(std::forward<value_type>(val));
        return *this;
    }

    this_type& operator=(const value_type& val)
    {
        this_type::store(val);
        return *this;
    }

    this_type& operator=(const value_ptr_type& val)
    {
        this_type::store(val);
        return *this;
    }

    this_type& operator=(this_type&& right)
    {
        this_type::swap(right);
        return *this;
    }

    this_type& operator=(const this_type& right)
    {
        if (this == &right)
        {
            return *this;
        }

        write_lock_type lk(_mutex);
        value_ptr_type tmp = std::make_shared<value_type>(std::move(right.load()));
        _ptr.swap(tmp);

        return *this;
    }

    void swap(value_type&& val)
    {
        write_lock_type lk(_mutex);
        if (_ptr)
        {
            std::swap(*_ptr, val);
            return;
        }

        value_ptr_type tmp = std::make_shared<value_type>(std::move(val));
        value_type tmp_val;
        std::swap(tmp_val, val);
        _ptr.swap(tmp);
    }

    void swap(value_ptr_type&& right)
    {
        value_ptr_type& right_ref = right;
        this_type::swap(right_ref);
    }

    void swap(value_ptr_type& base)
    {
        write_lock_type lk(_mutex);
        if (_ptr == base)
        {
            return;
        }
        _ptr.swap(base);
    }

    void swap(this_type&& right)
    {
        this_type& right_ref = right;
        this_type::swap(right_ref);
    }

    void swap(this_type& right)
    {
        if (this == &right)
        {
            return;
        }

        write_lock_type lk(_mutex);
        right.swap(_ptr);
    }

    operator bool(void) const
    {
        write_lock_type lk(_mutex);
        return !_ptr;
    }

    value_ptr_type get_shared_ptr() const
    {
        write_lock_type lk(_mutex);
        return _ptr;
    }

    void store(value_type&& val)
    {
        value_ptr_type tmp = std::make_shared<value_type>(std::move(val));
        {
            write_lock_type lk(_mutex);
            _ptr.swap(tmp);
        }
    }

    void store(const value_type& val)
    {
        value_ptr_type tmp = std::make_shared<value_type>(val);
        {
            write_lock_type lk(_mutex);
            _ptr.swap(tmp);
        }
    }

    void store(const value_ptr_type& ptr)
    {
        value_ptr_type tmp(ptr);
        {
            write_lock_type lk(_mutex);
            if (_ptr != ptr)
            {
                _ptr.swap(tmp);
            }
        }
    }

    value_type load() const
    {
        write_lock_type lk(_mutex);
        if (!_ptr)
        {
            throw std::runtime_error("");
        }
        return *_ptr;
    }

    bool empty() const
    {
        write_lock_type lk(_mutex);
        return !_ptr;
    }

    void clear()
    {
        value_ptr_type tmp;
        {
            write_lock_type lk(_mutex);
            tmp.swap(_ptr);
        }
    }

    template<typename result_type, typename Foo, typename ...Args>
    result_type call_function(Foo foo, Args&&... args)
    {
        write_lock_type lk(_mutex);
        if (!_ptr)
        {
            throw std::runtime_error("");
        }
        return ((_ptr.get()->*foo)(std::forward<Args>(args)...));
    }

    template<typename Handler>
    typename Handler::result_type using_handler(const Handler& handler)
    {
        write_lock_type lk(_mutex);
        if (!_ptr)
        {
            throw std::runtime_error("");
        }
        return handler(_ptr);
    }

private:
    mutable mutex_type _mutex;
    value_ptr_type _ptr;
};

} // namespace sss
