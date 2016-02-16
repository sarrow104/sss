#ifndef  __HTML_UTIL_HPP_1392003663__
#define  __HTML_UTIL_HPP_1392003663__

#include <string>
#include <cctype>

namespace sss{
namespace html_util {
    class indent_wraper { // {{{1
        std::string     item;
        int             depth;

    public:
        explicit indent_wraper(const std::string& in) : item(in), depth(0) {
            int i = 0;
            // ÌÞ³ý item ÖÐ ·Ç¿Õ°×·û
            while (i < int(this->item.length())) {
                if (!std::isspace(this->item[i])) {
                    break;
                }
                else {
                    ++i;
                }
            }
            this->item.resize(i);
        }

        std::string get() {
            std::string ret;
            for (int i = 0; i < this->depth; ++i) {
                ret.append(this->item);
            }
            return ret;
        }

        void pop() {
            this->depth--;
        }
        void push() {
            this->depth++;
        }
    };

    class indent_auto { // {{{1
        indent_wraper &ind;

    public:
        explicit indent_auto(indent_wraper& in) : ind(in) {
            ind.push();
        }
        ~indent_auto() {
            ind.pop();
        }
    };
// }}}1
} // end of namespace html_util

} // end of namespace sss

#endif  /* __HTML_UTIL_HPP_1392003663__ */
