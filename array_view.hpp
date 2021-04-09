#ifndef __ARRAY_VIEW_HPP_1586081473__
#define __ARRAY_VIEW_HPP_1586081473__


// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4480.html#string.view
//! https://code.woboq.org/llvm/libcxx/include/string_view.html
//      #763line
//! https://searchcode.com/codesearch/view/49391615/
//      #488line
//! https://gcc.gnu.org/onlinedocs/gcc-6.2.0/libstdc++/api/a01649_source.html
//      #694line
//! http://open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3685.html
//

#include <algorithm>
#include <stdexcept>
#include <cstddef>
#include <cstring>
#include <limits>
#include <string>
#include <sstream>
#include <type_traits>
#include <vector>

#include <sss/config.hpp>

namespace sss {
// 7.2, Class template array_view
template <class ValueType>
class array_view;

// 7.10, Inserters and extractors
template <class ValueType>
std::basic_ostream<ValueType>& operator<<(
    std::basic_ostream<ValueType>& os, const array_view<ValueType>& view)
{
    os.put('[');
    bool is_first = true;
    for (const ValueType& v: view)
    {
        if (!is_first)
        {
            os << ", ";
        }
        os << v;
        is_first = false;
    }
    os.put(']');
    return os;
}

}  // namespace sss

namespace sss {

template <class ValueType>
class array_view {
public:
    // types
    typedef ValueType value_type;
    typedef const ValueType* pointer;
    typedef const ValueType* const_pointer;
    typedef const ValueType& reference;
    typedef const ValueType& const_reference;
    typedef const_pointer const_iterator;
    typedef pointer iterator;
    // 1) Because array_view refers to a constant sequence, iterator and
    // const_iterator are the same type.

    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef const_reverse_iterator reverse_iterator;
    // typedef typename traits::off_type difference_type;
    typedef ptrdiff_t difference_type;
    typedef typename std::basic_string<ValueType>::size_type size_type;
    static const size_type npos = size_type(-1);

    // 7.3, array_view constructors and assignment operators
    array_view() SSS_NOEXCEPT : data_(nullptr), size_(0){};
    array_view(const array_view&) SSS_NOEXCEPT = default;
    array_view& operator=(const array_view&) SSS_NOEXCEPT = default;

    template <class Allocator>
    array_view(
        const std::vector<ValueType, Allocator>& view) SSS_NOEXCEPT
        : data_(view.data()),
          size_(view.size())
    {
    }
    array_view(const ValueType* view, size_type len)
        : data_(view), size_(len){};

    template <size_t N>
    array_view(ValueType (&s)[N])
        : data_(s), size_(N)
    {
    }
    template <size_t N>
    array_view(const ValueType (&s)[N])
        : data_(s), size_(N)
    {
    }

    array_view(array_view&& right) SSS_NOEXCEPT
        : data_ (nullptr), size_ (0)
    {
        array_view& right_ref{right};
        right_ref.swap(*this);
    }

    template<typename IteratorType>
    array_view(IteratorType first_, IteratorType last_)
    : data_(&*first_),
      size_(std::distance(first_, last_))
    {}

    void clear()
    {
        data_ = nullptr;
        size_ = 0;
    }
    // 7.4, array_view iterator support
    const_iterator begin() const SSS_NOEXCEPT { return data_; }
    const_iterator end() const SSS_NOEXCEPT { return data_ + size_; }
    //const_iterator cbegin() const SSS_NOEXCEPT { return data_; };
    //const_iterator cend() const SSS_NOEXCEPT { return data_ + size_; }
    const_reverse_iterator rbegin() const SSS_NOEXCEPT
    {
        return data_ + size_ - 1;
    };
    const_reverse_iterator rend() const SSS_NOEXCEPT
    {
        return data_ - 1;
    };
    const_reverse_iterator crbegin() const SSS_NOEXCEPT
    {
        return data_ + size_ - 1;
    };
    const_reverse_iterator crend() const SSS_NOEXCEPT
    {
        return data_ - 1;
    };

    // 7.5, array_view capacity
    size_type size() const SSS_NOEXCEPT { return size_; };
    size_type length() const SSS_NOEXCEPT { return size_; };
    size_type max_size() const SSS_NOEXCEPT { return size_; };
    bool empty() const SSS_NOEXCEPT { return !size_; };
    // 7.6, array_view element access
    const_reference operator[](size_type pos) const { return data_[pos]; }
    const_reference at(size_type pos) const {
        if (pos >= this->size()) {
            std::ostringstream oss;
            oss << "query " << pos << "th element! but size only " << this->size();
            throw std::runtime_error(oss.str());
        }
        return data_[pos];
    };
    const_reference front() const { return data_[0]; };
    const_reference back() const { return data_[size_ - 1]; };
    const_pointer data() const SSS_NOEXCEPT { return data_; };
    value_type pop_back()
    {
        value_type __ret{0};
        if (!this->empty()) {
            __ret = this->data_[this->size_ - 1];
            this->size_--;
        }
        return __ret;
    }
    value_type pop_front()
    {
        value_type __ret{0};
        if (!this->empty()) {
            __ret = this->data_[0];
            this->data_++;
            this->size_--;
        }
        return __ret;
    }

    // 7.7, array_view modifiers
    void remove_prefix(size_type n)
    {
        if (!empty()) {
            n = std::min(n, size_);
            data_ += n;
            size_ -= n;
        }
    };
    void remove_suffix(size_type n)
    {
        if (!empty()) {
            n = std::min(n, size_);
            size_ -= n;
        }
    };
    void swap(array_view& s) SSS_NOEXCEPT
    {
        if (this != &s) {
            std::swap(data_, s.data_);
            std::swap(size_, s.size_);
        }
    }

    void set(const ValueType* view, size_type len)
    {
        data_ = view;
        size_ = len;
    }

    template<typename ContType>
    void copy_to(ContType& target) const
    {
        target.clear();
        std::copy(this->begin(),
                  this->end(),
                  std::back_inserter(target));
    }

    template<typename ContType>
    void append_to(ContType& target) const
    {
        std::copy(this->begin(),
                  this->end(),
                  std::back_inserter(target));
    }

    // 7.8, array_view string operations
    template <class Allocator>
    explicit operator std::vector<ValueType, Allocator>() const
    {
        return to_vec();
    }
    template <class Allocator = std::allocator<ValueType>>
    std::vector<ValueType, Allocator> to_vec(
        const Allocator& a = Allocator()) const
    {
        if (empty()) {
            return {};
        }
        else {
            return {data_, size_};
        }
    }

    size_type copy(ValueType* __str, size_type __n, size_type __pos = 0) const
    {
        size_type __ret = std::min(size_ - __pos, __n);
        std::memcpy(__str, data_ + __pos, __ret);
        return __ret;
    }

    array_view subarr(size_type __pos = 0, size_type __n = npos) const
    {
        if (__pos > size_) __pos = size_;
        if (__n > size_ - __pos) __n = size_ - __pos;
        return array_view(data_ + __pos, __n);
    }

private:
    const_pointer data_;  // exposition only
    size_type     size_;  // exposition only
};

}  // namespace sss


#endif /* __ARRAY_VIEW_HPP_1586081473__ */
