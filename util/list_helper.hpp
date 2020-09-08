// ${sss}/include/util/list_helper.hpp
#pragma once

#include <cstdlib>
#include <algorithm>

namespace sss {

namespace utility {

template<typename ContType>
inline bool is_all(const ContType& l, const typename ContType::value_type& t)
{
    bool is_ok = true;
    for (const typename ContType::value_type& v: l)
    {
        if (v != t) {
            is_ok = false;
            break;
        }
    }
    return is_ok;
}

template<typename ContType>
inline bool is_all_zero(const ContType& l)
{
    return utility::is_all(l, typename ContType::value_type());
}

template<typename ContType>
inline bool is_any(const ContType& l, const typename ContType::value_type& t)
{
    bool is_ok = false;
    for (const typename ContType::value_type& v: l)
    {
        if (v == t) {
            is_ok = true;
            break;
        }
    }
    return is_ok;
}

template<typename ContType>
inline bool is_any_not(const ContType& l, const typename ContType::value_type& t)
{
    bool is_ok = false;
    for (const typename ContType::value_type& v: l)
    {
        if (v != t) {
            is_ok = true;
            break;
        }
    }
    return is_ok;
}

template<typename InContType, typename OutContType>
inline std::size_t copy(const InContType& i, OutContType& o)
{
    typename OutContType::iterator it = std::copy(std::begin(i), std::end(i), std::begin(o));
    return std::distance(std::begin(o), it);
}

template<typename ContType, typename BinaryOp, typename OutputIt>
inline void binary_apply(const ContType& c1, const ContType& c2, BinaryOp op, OutputIt outIt)
{
    //typedef typename ContType::iterator iterator;
    typedef typename ContType::const_iterator const_iterator;

    const_iterator first1 = std::begin(c1);
    const_iterator last1 = std::end(c1);

    const_iterator first2 = std::begin(c2);
    const_iterator last2 = std::end(c2);

    for (; first1 != last1 && first2 != last2; ++first1, ++first2)
    {
        *outIt++ = op(*first1, *first2);
    }
}

template<typename ContType, typename TestOp>
inline void remove_if(ContType& c, TestOp op)
{
    c.erase(
        std::remove_if(
            std::begin(c),
            std::end(c),
            op),
        std::end(c));
}

template<typename ContType1, typename ContType2>
inline void append(ContType1& c1, const ContType2& c2)
{
    std::copy(std::begin(c2), std::end(c2), std::back_inserter(c1));
}

template<typename ContType, typename ValueType>
inline void init_list(ContType& l, ValueType v)
{
    for( typename ContType::value_type& val: l)
    {
        val = typename ContType::value_type(std::move(v));
    }
}

template<typename ContType>
inline void init_list(ContType& l)
{
    for( typename ContType::value_type& val: l)
    {
        val = typename ContType::value_type();
    }
}

} // namespace utility

} // namespace sss
