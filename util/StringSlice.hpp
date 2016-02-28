#ifndef  __STRINGSLICE_HPP_1450096289__
#define  __STRINGSLICE_HPP_1450096289__

#include <string>
#include <iostream>

// TODO
// StringSlice 如何假如内存管理？
//
// 之前，它就是引用别人的数据，比如来自std::string，或者来自上层函数传递来的c风
// 格字符串参数；在函数内部，只是临时使用；
//
// 如果，要假如内存管理，那肯定得加入指针；
//
// 对于 std::string，那我不是需要 std::string* ？
//
// 对于 char * ，又怎么办？
//
// 两者根本没办法协调其内存管理:
// new std::string(value);
// new char[std::strlen(value)];
//
// 析构的时候：
//
// delete   _ptr;
// delete[] _ptr;
//
// 结论，没办法，除非再套一个type_traits

namespace sss {
    namespace util {

        template<typename Iterator>
        class StringSlice
        {
            typedef typename std::iterator_traits<Iterator>::value_type value_type;
            // typedef std::string::const_iterator iterator;
        public:
            typedef Iterator iterator;
            StringSlice(Iterator it_beg, Iterator it_end)
                : m_beg(it_beg), m_end(it_end)
            {
            }
            ~StringSlice()
            {
            }

        public:

            bool operator == (const char* cstr)
            {
                Iterator it = this->m_beg;
                for (;
                     it != this->m_end && cstr && *cstr;
                     ++it, ++cstr)
                {
                    if (*it != *cstr) {
                        return false;
                    }
                }
                return it == this->m_end && (!cstr || !*cstr);
            }

            value_type operator[](int idx)
            {
                if (0 <= idx && idx < int(this->length())) {
                    return this->at(idx);
                }
                else {
                    return value_type('\0');
                }
            }

            value_type at(int idx)
            {
                return *(this->m_beg + idx);
            }

            bool operator == (const std::string& s)
            {
                Iterator it = this->m_beg;
                std::string::const_iterator it2 = s.begin();
                for (;
                     it != this->m_end && it2 != s.end();
                     ++it, ++it2)
                {
                    if (*it != *it2) {
                        return false;
                    }
                }
                return it == this->m_end && (it2 == s.end());
            }

            StringSlice& assign(Iterator it_beg, Iterator it_end)
            {
                this->m_beg = it_beg;
                this->m_end = it_end;
            }

            StringSlice& assignTo(std::string& out)
            {
                out.assign(this->m_beg, this->m_end);
            }

            void print(std::ostream& o) const {
                for (Iterator it = m_beg; it != m_end; ++it) {
                    o << *it;
                }
            }

            bool shrink(size_t left_offset, size_t right_offset)
            {
                if (this->length() >= left_offset + right_offset) {
                    std::advance(this->m_beg, left_offset);
                    std::advance(this->m_end, - right_offset);
                    return true;
                }
                return false;
            }

            void clear()
            {
                if (this->length()) {
                    this->m_beg = this->m_end;
                }
            }

            char pop_back()
            {
                char ret = '\0';
                if (this->length()) {
                    ret = *(this->m_end - 1);
                    std::advance(this->m_end, -1);
                }
                return ret;
            }

            char pop_front()
            {
                char ret = '\0';
                if (this->length()) {
                    ret = *(this->m_beg);
                    std::advance(this->m_beg, 1);
                }
                return ret;
            }

            StringSlice substr(int start_pos)
            {
                return StringSlice(this->m_beg + start_pos, this->m_end);
            }

            StringSlice substr(int start_pos, int len)
            {
                return StringSlice(this->m_beg + start_pos, this->m_beg + start_pos + len);
            }

            size_t length() const {
                return std::distance(m_beg, m_end);
            }

            std::string str() const {
                return std::string(m_beg, m_end);
            }

            Iterator begin() const {
                return m_beg;
            }

            Iterator end() const {
                return m_end;
            }

            void rtrim() {
                while (m_beg != m_end && std::isspace(*(m_end-1))) {
                    m_end--;
                }
            }

            void ltrim() {
                while (m_beg != m_end && std::isspace(*m_beg)) {
                    m_beg++;
                }
            }

            void trim() {
                ltrim();
                rtrim();
            }

            char back() const {
                if (m_beg != m_end) {
                    return *(m_end - 1);
                }
                return '\0';
            }

            char front() const {
                if (m_beg != m_end) {
                    return *m_beg;
                }
                return '\0';
            }

        private:
            Iterator m_beg;
            Iterator m_end;
        };

        template<typename Iterator>
        inline StringSlice<Iterator> make_slice(Iterator beg, Iterator end)
        {
            return StringSlice<Iterator>(beg, end);
        }

        template<typename Iterator>
        inline StringSlice<Iterator> make_slice(const std::pair<Iterator, Iterator>& range)
        {
            return StringSlice<Iterator>(range.first, range.second);
        }

        template<typename Iterator>
        inline std::ostream& operator << (std::ostream& o, const StringSlice<Iterator>& s)
        {
            s.print(o);
            return o;
        }

    }
}


#endif  /* __STRINGSLICE_HPP_1450096289__ */
