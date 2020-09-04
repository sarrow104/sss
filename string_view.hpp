#ifndef __STRING_VIEW_HPP_1476871268__
#define __STRING_VIEW_HPP_1476871268__

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

#include <sss/config.hpp>

namespace sss {
// 7.2, Class template basic_string_view
template <class charT, class traits = std::char_traits<charT> >
class basic_string_view;

// 7.9, basic_string_view non-member comparison functions
template <class charT, class traits>
bool operator==(basic_string_view<charT, traits> __x,
                basic_string_view<charT, traits> __y) SSS_NOEXCEPT
{
    return __x.compare(__y) == 0;
}
template <class charT, class traits>
bool operator!=(basic_string_view<charT, traits> __x,
                basic_string_view<charT, traits> __y) SSS_NOEXCEPT

{
    return __x.compare(__y) != 0;
}
template <class charT, class traits>
bool operator<(basic_string_view<charT, traits> __x,
               basic_string_view<charT, traits> __y) SSS_NOEXCEPT
{
    return __x.compare(__y) < 0;
}
template <class charT, class traits>
bool operator>(basic_string_view<charT, traits> __x,
               basic_string_view<charT, traits> __y) SSS_NOEXCEPT
{
    return __x.compare(__y) > 0;
}
template <class charT, class traits>
bool operator<=(basic_string_view<charT, traits> __x,
                basic_string_view<charT, traits> __y) SSS_NOEXCEPT
{
    return __x.compare(__y) <= 0;
}
template <class charT, class traits>
bool operator>=(basic_string_view<charT, traits> __x,
                basic_string_view<charT, traits> __y) SSS_NOEXCEPT
{
    return __x.compare(__y) >= 0;
}
// see below, sufficient additional overloads of comparison functions

// 7.10, Inserters and extractors
template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(
    std::basic_ostream<charT, traits>& os, basic_string_view<charT, traits> str)
{
    os.write(str.data(), str.length());
    return os;
}

// basic_string_view typedef names
typedef basic_string_view<char> string_view;
typedef basic_string_view<char16_t> u16string_view;
typedef basic_string_view<char32_t> u32string_view;
typedef basic_string_view<wchar_t> wstring_view;

// // 7.11, Hash support
// template <class T> struct hash;
// template <> struct hash<experimental::string_view>;
// template <> struct hash<experimental::u16string_view>;
// template <> struct hash<experimental::u32string_view>;
// template <> struct hash<experimental::wstring_view>;

}  // namespace std

namespace sss {

template <class charT, class traits>
class basic_string_view {
public:
    // types
    typedef traits traits_type;
    typedef charT value_type;
    typedef const charT* pointer;
    typedef const charT* const_pointer;
    typedef const charT& reference;
    typedef const charT& const_reference;
    typedef const typename traits::char_type* const_iterator;  // See 7.4
    typedef const_iterator iterator;
    // 1) Because basic_string_view refers to a constant sequence, iterator and
    // const_iterator are the same type.

    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef const_reverse_iterator reverse_iterator;
    // typedef typename traits::off_type difference_type;
    typedef ptrdiff_t difference_type;
    typedef typename std::basic_string<charT, traits>::size_type size_type;
    static const size_type npos = size_type(-1);

    // 7.3, basic_string_view constructors and assignment operators
    basic_string_view() SSS_NOEXCEPT : data_(nullptr), size_(0){};
    basic_string_view(const basic_string_view&) SSS_NOEXCEPT = default;
    basic_string_view& operator=(const basic_string_view&) SSS_NOEXCEPT = default;
    template <class Allocator>
    basic_string_view(
        const std::basic_string<charT, traits, Allocator>& str) SSS_NOEXCEPT
        : data_(str.data()),
          size_(str.size())
    {
    }
    basic_string_view(const charT* str) : data_(str), size_(str ? traits::length(str) : 0)
    {
    }
    basic_string_view(const charT* str, size_type len)
        : data_(str), size_(len){};

    template <size_t N>
    basic_string_view(charT (&s)[N])
        : data_(s), size_(s[N - 1] ? N : traits::length(s))
    {
    }
    template <size_t N>
    basic_string_view(const charT (&s)[N])
        : data_(s), size_((s && s[N - 1]) ? N : traits::length(s))
    {
    }
    //basic_string_view(basic_string_view&& right) SSS_NOEXCEPT
    //    : data_ (nullptr), size_ (0)
    //{
    //    basic_string_view& right_ref{right};
    //    right_ref.swap(*this);
    //}

    void clear()
    {
        data_ = nullptr;
        size_ = 0;
    }
    // 7.4, basic_string_view iterator support
    const_iterator begin() const SSS_NOEXCEPT { return data_ ? data_ : ""; }
    const_iterator end() const SSS_NOEXCEPT { return data_ ? data_ + size_ : ""; }
    const_iterator cbegin() const SSS_NOEXCEPT { return data_ ? data_ : ""; };
    const_iterator cend() const SSS_NOEXCEPT { return data_ ? data_ + size_ : ""; }
    const_reverse_iterator rbegin() const SSS_NOEXCEPT
    {
        return const_reverse_iterator(end());
    };
    const_reverse_iterator rend() const SSS_NOEXCEPT
    {
        return const_reverse_iterator(begin());
    };
    const_reverse_iterator crbegin() const SSS_NOEXCEPT
    {
        return const_reverse_iterator(end());
    };
    const_reverse_iterator crend() const SSS_NOEXCEPT
    {
        return const_reverse_iterator(begin());
    };

    // 7.5, basic_string_view capacity
    size_type size() const SSS_NOEXCEPT { return size_; };
    size_type length() const SSS_NOEXCEPT { return size_; };
    size_type max_size() const SSS_NOEXCEPT { return size_; };
    bool empty() const SSS_NOEXCEPT { return !size_; };
    // 7.6, basic_string_view element access
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

    // 7.7, basic_string_view modifiers
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
    void swap(basic_string_view& s) SSS_NOEXCEPT
    {
        if (this != &s) {
            std::swap(data_, s.data_);
            std::swap(size_, s.size_);
        }
    }

    void set(const charT* str)
    {
        data_ = str;
        size_ = (str ? 0u : traits::length(str));
    }

    void set(const charT* str, size_type len)
    {
        data_ = str;
        size_ = len;
    }

    void copy_to_string(std::basic_string<charT, traits>& target) const
    {
        target.assign(data_, size_);
    }

    void append_to_string(std::basic_string<charT, traits>& target) const
    {
        target.append(data_, size_);
    }

    // 7.8, basic_string_view string operations
    template <class Allocator>
    explicit operator std::basic_string<charT, traits, Allocator>() const
    {
        return to_string();
    }
    template <class Allocator = std::allocator<charT>>
    std::basic_string<charT, traits, Allocator> to_string(
        const Allocator& a = Allocator()) const
    {
        if (empty()) {
            return {};
        }
        else {
            return {data_, size_};
        }
    }

    size_type copy(charT* __str, size_type __n, size_type __pos = 0) const
    {
        size_type __ret = std::min(size_ - __pos, __n);
        std::memcpy(__str, data_ + __pos, __ret);
        return __ret;
    }

    basic_string_view substr(size_type __pos = 0, size_type __n = npos) const
    {
        if (__pos > size_) __pos = size_;
        if (__n > size_ - __pos) __n = size_ - __pos;
        return basic_string_view(data_ + __pos, __n);
    }
    int compare(basic_string_view __str) const SSS_NOEXCEPT
    {
        int __r = std::memcmp(data_, __str.data_, std::min(size_, __str.size_));
        if (__r == 0) {
            if (size_ < __str.size_)
                __r = -1;
            else if (size_ > __str.size_)
                __r = +1;
        }
        return __r;
    }
    int compare(size_type __pos1, size_type __n1, basic_string_view __str) const
    {
        return this->substr(__pos1, __n1).compare(__str);
    }
    int compare(size_type __pos1, size_type __n1, basic_string_view __str,
                size_type __pos2, size_type __n2) const
    {
        return this->substr(__pos1, __n1).compare(__str.substr(__pos2, __n2));
    }
    int compare(const charT* __str) const
    {
        return this->compare(basic_string_view{__str});
    }
    int compare(size_type __pos1, size_type __n1, const charT* __str) const
    {
        return this->substr(__pos1, __n1).compare(basic_string_view{__str});
    }
    int compare(size_type __pos1, size_type __n1, const charT* __str,
                size_type __n2) const
    {
        return this->substr(__pos1, __n1)
            .compare(basic_string_view(__str, __n2));
    }
    size_type find(basic_string_view __str, size_type __pos = 0) const SSS_NOEXCEPT
    {
        return this->find(__str.data_, __pos, __str.size_);
    }
    size_type find(charT __c, size_type __pos = 0) const SSS_NOEXCEPT;
    size_type find(const charT* __str, size_type __pos, size_type __n) const;
    size_type find(const charT* __str, size_type __pos = 0) const
    {
        return this->find(__str, __pos, traits_type::length(__str));
    }
    size_type rfind(basic_string_view __str, size_type __pos = npos) const
        SSS_NOEXCEPT
    {
        return this->rfind(__str.data_, __pos, __str.size_);
    }
    size_type rfind(charT __c, size_type __pos = npos) const SSS_NOEXCEPT;
    size_type rfind(const charT* __str, size_type __pos, size_type __n) const;
    size_type rfind(const charT* __str, size_type __pos = npos) const
    {
        return this->rfind(__str, __pos, traits_type::length(__str));
    }

    size_type find_first_of(basic_string_view __str, size_type __pos = 0) const
        SSS_NOEXCEPT
    {
        return this->find_first_of(__str.data_, __pos, __str.size_);
    }
    size_type find_first_of(charT __c, size_type __pos = 0) const SSS_NOEXCEPT
    {
        return this->find(__c, __pos);
    }
    size_type find_first_of(const charT* __str, size_type __pos,
                            size_type __n) const;
    size_type find_first_of(const charT* __str, size_type __pos = 0) const
    {
        return this->find_first_of(__str, __pos, traits_type::length(__str));
    }
    size_type find_last_of(basic_string_view __str,
                           size_type __pos = npos) const SSS_NOEXCEPT
    {
        return this->find_last_of(__str.data_, __pos, __str.size_);
    }
    size_type find_last_of(charT __c, size_type __pos = npos) const SSS_NOEXCEPT
    {
        return this->rfind(__c, __pos);
    }
    size_type find_last_of(const charT* __str, size_type __pos,
                           size_type __n) const;
    size_type find_last_of(const charT* __str, size_type __pos = npos) const
    {
        return this->find_last_of(__str, __pos, traits_type::length(__str));
    }
    size_type find_first_not_of(basic_string_view __str,
                                size_type __pos = 0) const SSS_NOEXCEPT
    {
        return this->find_first_not_of(__str.data_, __pos, __str.size_);
    }
    size_type find_first_not_of(charT __c, size_type __pos = 0) const SSS_NOEXCEPT;
    size_type find_first_not_of(const charT* __str, size_type __pos,
                                size_type __n) const;
    size_type find_first_not_of(const charT* __str, size_type __pos = 0) const
    {
        return this->find_first_not_of(__str, __pos,
                                       traits_type::length(__str));
    }
    size_type find_last_not_of(basic_string_view __str,
                               size_type __pos = npos) const SSS_NOEXCEPT
    {
        return this->find_last_not_of(__str.data_, __pos, __str.size_);
    }
    size_type find_last_not_of(charT __c, size_type __pos = npos) const
        SSS_NOEXCEPT;
    size_type find_last_not_of(const charT* __str, size_type __pos,
                               size_type __n) const;
    size_type find_last_not_of(const charT* __str, size_type __pos = npos) const
    {
        return this->find_last_not_of(__str, __pos, traits_type::length(__str));
    }

    bool is_begin_with(const basic_string_view& __x) const
    {
        return !__x.size_
            || (size_ >= __x.size_ &&
                std::memcmp(data_, __x.data_, __x.size_) == 0);
    }

    bool is_end_with(const basic_string_view& __x) const
    {
        return !__x.size_
            || (size_ >= __x.size_ &&
                std::memcmp(data_ + (size_ - __x.size_), __x.data_, __x.size_) == 0);
    }

    bool is_contains(const basic_string_view& __x) const
    {
        return find(__x) != npos;
    }

private:
    const_pointer data_;  // exposition only
    size_type size_;      // exposition only
};

template <typename charT, typename traits>
inline typename basic_string_view<charT, traits>::size_type
basic_string_view<charT, traits>::find(const charT* __str, size_type __pos,
                                       size_type __size) const
{
    if (__pos > size_) return npos;
    const_pointer result =
        std::search(data_ + __pos, data_ + size_, __str, __str + __size);
    size_type xpos = result - data_;
    return xpos + __size <= size_ ? xpos : npos;
}

template <typename charT, typename traits>
inline typename basic_string_view<charT, traits>::size_type
basic_string_view<charT, traits>::find(charT __c, size_type __pos) const
    SSS_NOEXCEPT
{
    if (size_ <= 0 || __pos >= size_) return npos;
    const_pointer result = std::find(data_ + __pos, data_ + size_, __c);
    return result != data_ + size_ ? result - data_ : npos;
}

template <typename charT, typename traits>
inline typename basic_string_view<charT, traits>::size_type
basic_string_view<charT, traits>::rfind(charT __c, size_type __pos) const
    SSS_NOEXCEPT
{
    if (size_ <= 0) return npos;
    for (size_t i = std::min(__pos + 1, size_); i != 0;) {
        if (data_[--i] == __c) return i;
    }
    return npos;
}
template <typename charT, typename traits>
inline typename basic_string_view<charT, traits>::size_type
basic_string_view<charT, traits>::rfind(const charT* __str, size_type __pos,
                                        size_type __n) const
{
    if (size_ < __n) return npos;
    if (__n == 0) return std::min(size_, __pos);
    const_pointer last = data_ + std::min(size_ - __n, __pos) + __n;
    const_pointer result = std::find_end(data_, last, __str, __str + __n);
    return result != last ? result - data_ : npos;
}

template<typename CharT, typename TraitsT>
inline typename basic_string_view<CharT, TraitsT>::size_type
basic_string_view<CharT, TraitsT>::find_first_of(const CharT* __str, size_type __pos, size_type __n) const
{
    for (; __n && __pos < this->size_; ++__pos)
    {
        const CharT* __p = traits_type::find(__str, __n,
                                             this->data_[__pos]);
        if (__p)
            return __pos;
    }
    return npos;
}

template<typename CharT, typename TraitsT>
inline typename basic_string_view<CharT, TraitsT>::size_type
basic_string_view<CharT, TraitsT>::find_first_not_of(const CharT* __str, size_type __pos, size_type __n) const
{
    for (; __n && __pos < this->size_; ++__pos)
    {
        const CharT* __p = traits_type::find(__str, __n,
                                             this->data_[__pos]);
        if (!__p)
            return __pos;
    }
    return npos;
}

template<typename CharT, typename TraitsT>
inline typename basic_string_view<CharT, TraitsT>::size_type
basic_string_view<CharT, TraitsT>::find_last_of(const CharT* __str, size_type __pos, size_type __n) const
{
    for (__pos = (__pos == npos ? this->size_ : __pos); __n && __pos > 0; --__pos)
    {
        const CharT* __p = traits_type::find(__str, __n,
                                             this->data_[__pos - 1]);
        if (__p)
            return __pos - 1;
    }
    return npos;
}

template<typename CharT, typename TraitsT>
inline typename basic_string_view<CharT, TraitsT>::size_type
basic_string_view<CharT, TraitsT>::find_last_not_of(const CharT* __str, size_type __pos, size_type __n) const
{
    for (__pos = (__pos == npos? this->size_ : __pos); __n && __pos > 0; --__pos)
    {
        const CharT* __p = traits_type::find(__str, __n,
                                             this->data_[__pos - 1]);
        if (!__p)
            return __pos - 1;
    }
    return npos;
}

inline void trim(sss::string_view& s)
{
    while(!s.empty() && std::isspace(s.front()))
    {
        s.pop_front();
    }
    while(!s.empty() && std::isspace(s.back()))
    {
        s.pop_back();
    }
}

}  // namespace sss

#endif /* __STRING_VIEW_HPP_1476871268__ */
