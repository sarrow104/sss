#include <cstdlib>
#include <ctime>

#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include "../heapify/heapify.hpp"

using iv = std::vector<size_t>;
using heap_t = heaper<iv::iterator>;

void f1(iv &v) {
    heap_t heap=make_heap(v);
    for (size_t i=0; i<v.size(); ++i) {
        // ÕâÀï¾ÍËæ±ã¸ã¸ã
        heap.push(i%10000);
    }
}

void f2(iv &v) {
    heap_t heap=make_heap(v);
    for (size_t i=0; i<v.size(); ++i) {
        heap.pop();
    }
}

TEST(heapify, basic)
{
    iv v(1000*1000*50,0);
    time_t s=time(nullptr);
    f1(v);
    f2(v);
    time_t s1=time(nullptr);
    std::cout << s1-s << std::endl;
}
