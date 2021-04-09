// poly_holder.hpp
#pragma once

#include <boost/functional/hash.hpp>
#include <boost/type_traits.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>
#include <boost/preprocessor.hpp>

namespace sss {
namespace can {

class poly_holder
{
public:
    typedef poly_holder this_type;
    typedef void (*deleter_t)(void*);

    poly_holder()
        :
            _hash_code (0),
            _ptr       (nullptr),
            _deleter   (nullptr)
    {}

    poly_holder(this_type&& right)
        :
            _hash_code (0),
            _ptr       (nullptr),
            _deleter   (nullptr)
    {
        this_type& right_ref = right;
        this->swap(right_ref);
    }
    ~poly_holder()
    {
        this->clear();
    }

    poly_holder& operator=(this_type&& right)
    {
        this_type& right_ref = right;
        if (this != &right_ref)
        {
            this->swap(right_ref);
        }
        return *this;
    }

private:
    poly_holder(const poly_holder&) = delete;
    poly_holder& operator=(const poly_holder&) = delete;

public:
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
        // s/\(\w\+\);\?/std::swap(\1, right.\1);/ge
        std::swap(_hash_code, right._hash_code);
        std::swap(_ptr,       right._ptr);
        std::swap(_deleter,   right._deleter);
    }

protected:
    template<typename T>
    static void wrap_deleter(void * ptr)
    {
        if (ptr) {
            T* tmp = reinterpret_cast<T*>(ptr);
            delete tmp;
        }
    }

public:
    template<typename T>
    bool init()
    {
        if (!_ptr)
        {
            _hash_code = boost::hash_value(typeid(T));
            _ptr       = new T();
            _deleter   = &wrap_deleter<T>;
            return true;
        }
        return false;
    }

    template<typename T, typename Arg0, typename ...Args>
    bool init(Arg0&& arg0, Args&&... args)
    {
        if (!_ptr)
        {
            _hash_code = boost::hash_value(typeid(T));
            _ptr       = new T(std::forward<Arg0>(arg0), std::forward<Args>(args)...);
            _deleter   = &wrap_deleter<T>;
            return true;
        }
        return false;
    }

    template<typename T>
    T* get() const
    {
        assert(this_type::is_type<T>());
        return _ptr
            ? reinterpret_cast<T*>(_ptr)
            : nullptr;
    }

    void * ptr() const
    {
       return _ptr;
    }

    template<typename T>
    bool is_type() const
    {
        return _hash_code == boost::hash_value(typeid(T));
    }

    void clear()
    {
        if (!_ptr)
        {
            assert(_deleter);
            _deleter(_ptr);
            _hash_code = 0;
            _deleter   = nullptr;
            _ptr       = nullptr;
        }
    }

    bool empty() const
    {
        return !_ptr;
    }

private:
    size_t    _hash_code;
    void*     _ptr;
    deleter_t _deleter;
};

} // namespace can
} // namespace sss
