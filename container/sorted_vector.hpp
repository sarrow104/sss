// sorted_vector.hpp
#pragma once

#include <iterator>
#include <cstdlib>
#include <algorithm>

namespace sss {

// equal_range place_holder for __cplusplus < 201104L

template<typename ContType>
void sort(ContType& vec)
{
    std::sort(vec.begin(), vec.end());
}

template<typename ContType, typename Pred>
void sort(ContType& vec, Pred pred)
{
    std::sort(vec.begin(), vec.end(), pred);
}

template<typename ContType, typename T>
typename ContType::iterator
insert_sorted(ContType& vec, const T& value)
{
    return
        vec.insert(
            std::upper_bound(
                std::begin(vec),
                std::end(vec),
                value),
            value);
}

template<typename ContType, typename T, typename Pred>
typename ContType::iterator
insert_sorted(ContType& vec, const T& value, Pred pred)
{
    return
        vec.insert(
            std::upper_bound(
                std::begin(vec),
                std::end(vec),
                value,
                pred),
            value);
}

template<typename ContType, typename T>
bool
insert_set_sorted(ContType& vec, const T& value)
{
    typedef typename ContType::iterator ForwardIt;
    std::pair<ForwardIt, ForwardIt> pr
        = std::equal_range(
            std::begin(vec),
            std::end(vec),
            value);

    return (pr.first == pr.second)
        ? (pr.first = vec.insert(
                pr.second,
                value),
            pr.first != std::end(vec))
        : false;
}

template<typename ContType, typename T, typename Pred>
bool
insert_set_sorted(ContType& vec, const T& value, Pred pred)
{
    typedef typename ContType::iterator ForwardIt;
    std::pair<ForwardIt, ForwardIt> pr
        = std::equal_range(
            std::begin(vec),
            std::end(vec),
            value,
            pred);

    return (pr.first == pr.second)
        ? (pr.first = vec.insert(
                pr.second,
                value),
            pr.first != std::end(vec))
        : false;
}

template<typename ContType, typename T>
void
erase_sorted(ContType& vec, const T& value)
{
    typedef typename ContType::iterator ForwardIt;
    std::pair<ForwardIt, ForwardIt> pr
        = std::equal_range(
            std::begin(vec),
            std::end(vec),
            value);

    vec.erase(
        pr.first,
        pr.second);
}

template<typename ContType, typename T, typename Pred>
void
erase_sorted(ContType& vec, const T& value, Pred pred)
{
    typedef typename ContType::iterator ForwardIt;
    std::pair<ForwardIt, ForwardIt> pr
        = std::equal_range(
            std::begin(vec),
            std::end(vec),
            value,
            pred);

    vec.erase(
        pr.first,
        pr.second);
}

template<typename ContType, typename T>
typename ContType::const_iterator
find_sorted(const ContType& vec, const T& value)
{
    typedef typename ContType::const_iterator ForwardIt;
    ForwardIt end = std::end(vec);
    std::pair<ForwardIt, ForwardIt> pr
        = std::equal_range(
            std::begin(vec),
            end,
            value);

    return pr.first != pr.second
        ? pr.first
        : end;
}

template<typename ContType, typename T, typename Pred>
typename ContType::const_iterator
find_sorted(const ContType& vec, const T& value, Pred pred)
{
    typedef typename ContType::const_iterator ForwardIt;
    ForwardIt end = std::end(vec);
    std::pair<ForwardIt, ForwardIt> pr
        = std::equal_range(
            std::begin(vec),
            end,
            value,
            pred);

    return pr.first != pr.second
        ? pr.first
        : end;
}

template<typename ContType, typename T>
typename ContType::iterator
find_sorted(ContType& vec, const T& value)
{
    typedef typename ContType::iterator ForwardIt;
    ForwardIt end = std::end(vec);
    std::pair<ForwardIt, ForwardIt> pr
        = std::equal_range(
            std::begin(vec),
            end,
            value);

    return pr.first != pr.second
        ? pr.first
        : end;
}

template<typename ContType, typename T, typename Pred>
typename ContType::iterator
find_sorted(ContType& vec, const T& value, Pred pred)
{
    typedef typename ContType::iterator ForwardIt;
    ForwardIt end = std::end(vec);
    std::pair<ForwardIt, ForwardIt> pr
        = std::equal_range(
            std::begin(vec),
            end,
            value,
            pred);

    return pr.first != pr.second
        ? pr.first
        : end;
}


template<typename ContType, typename T>
std::size_t
count_sorted(const ContType& vec, const T& value)
{
    typedef typename ContType::const_iterator ForwardIt;
    std::pair<ForwardIt, ForwardIt> pr
        = std::equal_range(
            std::begin(vec),
            std::end(vec),
            value);

    return std::distance(pr.first, pr.second);
}

template<typename ContType, typename T, typename Pred>
std::size_t
count_sorted(const ContType& vec, const T& value, Pred pred)
{
    typedef typename ContType::const_iterator ForwardIt;
    std::pair<ForwardIt, ForwardIt> pr
        = std::equal_range(
            std::begin(vec),
            std::end(vec),
            value,
            pred);

    return std::distance(pr.first, pr.second);
}


template<typename ContType, typename T>
bool
has_sorted(const ContType& vec, const T& value)
{
    return sss::count_sorted(vec, value) != 0u;
}

template<typename ContType, typename T, typename Pred>
bool
has_sorted(const ContType& vec, const T& value, Pred pred)
{
    return sss::count_sorted(vec, value, pred) != 0u;
}

} // namespace sss
