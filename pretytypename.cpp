#include "pretytypename.hpp"

#ifdef __GNUG__

#if __cplusplus >= 201103L
#   include <type_traits>
#   include <memory>
#else
#   include <cstdlib>
#endif

#include <cxxabi.h>

namespace sss {
namespace util {


#if __cplusplus < 201103L
struct handle {
    char* p;
    handle(char* ptr) : p(ptr) { }
    ~handle() { std::free(p); }
};
#endif

std::string demangle(const char* name)
{
    int status = -4; // some arbitrary value to eliminate the compiler warning
#if __cplusplus < 201103L
    handle result( abi::__cxa_demangle(name, NULL, NULL, &status) );
    return (status==0) ? result.p : name ;
#else
    // TODO 可以固定一个栈大小，供 __cxa_demangle 使用，以避免动态申请――再通
    // 过std::string 传出去的话，相当于构造两次！
    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
            std::free
    };

    return (status==0) ? res.get() : name;
#endif

}

#else // ifdef __GNUG__

// does nothing if not g++
std::string demangle(const char* name) {
    return name;
}

#endif

} // namespace util
} // namespace sss
