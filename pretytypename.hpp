#ifndef __PRETYTYPENAME_HPP_1468490669__
#define __PRETYTYPENAME_HPP_1468490669__

#include <string>
#include <typeinfo>


namespace sss {
namespace util {

std::string demangle(const char* name);

template <class T>
std::string pretytypename(const T& t)
{
    return demangle(typeid(t).name());
}

template <class T>
std::string pretytypename_t()
{
    return demangle(typeid(T).name());
}

} // namespace util
} // namespace sss
// https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html


#endif /* __PRETYTYPENAME_HPP_1468490669__ */

