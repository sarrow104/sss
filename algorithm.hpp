#ifndef  __ALGORITHM_HPP_1448760163__
#define  __ALGORITHM_HPP_1448760163__

#include <cstdlib>
#include <iterator>
#include <algorithm>
#include <cassert>

#include <iostream>

namespace sss
{
    // 从容器或者数组，获取其迭代器或者指针类型
    // template<typename T, int N>struct iterator_type
    // {
    //     typedef 
    // };
    template<typename T, int N> size_t size(T (&)[N])
    {
        return N;
    }

    template<typename T, int N> size_t size(const T (&)[N])
    {
        return N;
    }

    template<typename T, int N> T* begin(T (&a)[N])
    {
        return static_cast<T*>(a);
    }

    template<typename T, int N> T* end(T (&a)[N])
    {
        return static_cast<T*>(a) + N;
    }

    template<typename T, int N> const T* begin(const T (&a)[N])
    {
        return static_cast<const T*>(a);
    }

    template<typename T, int N> const T* end(const T (&a)[N])
    {
        return static_cast<const T*>(a) + N;
    }

    template<typename Container> size_t size(const Container & c)
    {
        return c.size();
    }

    template<typename Container> typename Container::iterator begin(Container & c)
    {
        return c.begin();
    }

    template<typename Container> typename Container::iterator end(Container & c)
    {
        return c.end();
    }

    template<typename Container> typename Container::const_iterator begin(const Container & c)
    {
        return c.cbegin();
    }

    template<typename Container> typename Container::const_iterator end(const Container & c)
    {
        return c.cend();
    }
    template<typename _InputIterator1,
             typename _InputIterator2>

    std::pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
             _InputIterator2 __first2, _InputIterator2 __last2)
    {
        // // concept requirements
        // __glibcxx_function_requires(_InputIteratorConcept<_InputIterator1>)
        // __glibcxx_function_requires(_InputIteratorConcept<_InputIterator2>)
        // __glibcxx_requires_valid_range(__first1, __last1);

        while (__first1 != __last1 &&
               __first2 != __last2 &&
               *__first1 == *__first2)
        {
            ++__first1;
            ++__first2;
        }
        return std::pair<_InputIterator1, _InputIterator2>(__first1, __first2);
    }

    template<typename _InputIterator1,
             typename _InputIterator2,
             typename _BinaryPredicate>

    std::pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
             _InputIterator2 __first2, _InputIterator2 __last2,
             _BinaryPredicate __binary_pred)
    {
        // // concept requirements
        // __glibcxx_function_requires(_InputIteratorConcept<_InputIterator1>)
        // __glibcxx_function_requires(_InputIteratorConcept<_InputIterator2>)
        // __glibcxx_requires_valid_range(__first1, __last1);

        while (__first1 != __last1 &&
               __first2 != __last2 &&
               bool(__binary_pred(*__first1, *__first2)))
        {
            ++__first1;
            ++__first2;
        }
        return std::pair<_InputIterator1, _InputIterator2>(__first1, __first2);
    }

    template<typename _InputIterator1,
             typename _InputIterator2>
    inline int max_match(_InputIterator1 __first1, _InputIterator1 __last1,
                          _InputIterator2 __first2, _InputIterator2 __last2)
    {
        int cnt = 0;
        while (__first1 != __last1 &&
               __first2 != __last2 &&
               bool(*__first1 == *__first2))
        {
            ++__first1;
            ++__first2;
            ++cnt;
        }

        return cnt;
    }

    template<typename _InputIterator1,
             typename _InputIterator2,
             typename _BinaryPredicate>
    inline int max_match(_InputIterator1 __first1, _InputIterator1 __last1,
                          _InputIterator2 __first2, _InputIterator2 __last2,
                          _BinaryPredicate __binary_pred)
    {
        int cnt = 0;
        while (__first1 != __last1 &&
               __first2 != __last2 &&
               bool(__binary_pred(*__first1, *__first2)))
        {
            ++__first1;
            ++__first2;
            ++cnt;
        }

        return cnt;
    }

    template<typename _InputIterator1,
             typename _InputIterator2>
    inline int is_equal(_InputIterator1 __first1, _InputIterator1 __last1,
                        _InputIterator2 __first2, _InputIterator2 __last2)
    {
        int cnt = max_match(__first1, __last1, __first2, __last2);
        return cnt == std::distance(__first1, __last1) && cnt == std::distance(__first2, __last2);
    }

    template<typename _InputIterator1,
             typename _InputIterator2,
             typename _BinaryPredicate>
    inline int is_equal(_InputIterator1 __first1, _InputIterator1 __last1,
                        _InputIterator2 __first2, _InputIterator2 __last2,
                        _BinaryPredicate __binary_pred)
    {
        int cnt = max_match(__first1, __last1, __first2, __last2, __binary_pred);
        return cnt == std::distance(__first1, __last1) && cnt == std::distance(__first2, __last2);
    }
}

#endif  /* __ALGORITHM_HPP_1448760163__ */
