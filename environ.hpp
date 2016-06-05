#ifndef __ENVIRON_HPP_1464842641__
#define __ENVIRON_HPP_1464842641__

#include <cstdlib>
#include <cstring>
#include <iterator>

#include <sss/utlstring.hpp>
#include <memory>

extern char ** environ;

namespace sss {
    namespace env {
        size_t count(char ** environ);
        inline size_t count()
        {
            return sss::env::count(environ);
        }
        template<typename Container>
            size_t dump(Container& out, char ** env = environ)
            {
                char ** p_env = env;
                while (p_env && p_env[0]) {
                    int eq_pos = std::strchr(p_env[0], '=') - p_env[0];
                    out[std::string(p_env[0], eq_pos)] = p_env[0] + eq_pos + 1;
                    p_env++;
                }
            }
        bool contain(const std::string& name, char ** env = environ);
        char * get(const std::string& name, char ** env = environ);

        // NOTE
        // 貌似 new [] 和 malloc() 并不兼容；
        // new [] 返回值前面，还附带了一个数量信息！
        // 关键在于，这个信息，是数组的长度；还是内存的字节数！
        // 如果是数组长度，那么就有兼容性问题。这里只能使用C风格；
        // 如果是字节数，那么可以换着用。
        template<typename Container>
        char ** alloc_c_str(const Container& c) {
            size_t size = (1 + c.size()) * sizeof(char *);
            for (typename Container::const_iterator it = c.begin();
                 it != c.end();
                 ++it)
            {
                size += it->first.length() + it->second.length() + 2;
            }
            // char * buf = reinterpret_cast<char*>(std::malloc(size)); // new char[size];
            char * buf = new char[size]; // ;
            // NOTE 实际的 guard 数字，比分配值，多了16个字节。
            // std::cout << __func__ << " size = " << size << std::endl;
            // std::cout << __func__ << " size = " << *reinterpret_cast<long*>(buf - sizeof(long)) << std::endl;
            char ** ret = reinterpret_cast<char**>(buf);
            char * content = buf + (1 + c.size()) * sizeof(char *);
            int i = 0;
            for (typename Container::const_iterator it = c.begin();
                 it != c.end();
                 ++it, ++i)
            {
                ret[i] = content;
                size_t cnt = std::sprintf(content, "%s=%s", it->first.c_str(), it->second.c_str());
                content += cnt + 1;
            }
            ret[c.size()] = 0;

            return ret;
        }
        char ** dup_c_str(char ** env);
        // set(const std::string& name)
        // NOTE 这个函数，还是交给系统的好。
        // 修改传入的environ，始终是不好的事情。
    } // namespace env
} // namespace sss


#endif /* __ENVIRON_HPP_1464842641__ */
