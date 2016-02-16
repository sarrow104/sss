#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include "heaper.h"

typedef std::vector<size_t> iv;
typedef heaper<iv::iterator> heap_t;

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

int main(int argc, const char * argv[])
{
    iv v(1000*1000*50,0);
    time_t s=time(NULL);
    f1(v);
    f2(v);
    time_t s1=time(NULL);
    std::cout << s1-s << std::endl;
    return 0;
}
