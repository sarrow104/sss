#ifndef  __KEYWORDFILTER_HPP_1449451116__
#define  __KEYWORDFILTER_HPP_1449451116__

#include "glob_path.hpp"
#include "filter_t.hpp"

#include <vector>
#include <string>

namespace sss {

    namespace path {

    namespace inner {

        inline bool isSubstrOf(const std::string& inner, const std::string& outer)
        {
            return outer.end() != std::search(outer.begin(), outer.end(),
                                              inner.begin(), inner.end(),
                                              sss::char_equal_casei());
        }

        inline bool isEqual(const std::string& lhs, const std::string& rhs)
        {
            return !sss::stricmp_t()(lhs, rhs) && !sss::stricmp_t()(rhs, lhs);
        }

        class IncludeRules : private std::vector<std::string>
        {
        public:
            typedef std::vector<std::string> BaseT;
            using BaseT::iterator;
            using BaseT::const_iterator;
            using BaseT::begin;
            using BaseT::end;

        public:
            IncludeRules()
            {
            }

            ~IncludeRules()
            {
            }

        public:

            // 保留"父串"；抛弃"相等"和"真子串"
            bool insert(const std::string& rule)
            {
                // std::cout << __func__ << " " << rule << std::endl;
                // NOTE，首先，当前rule集合，互不为对方的真子串！
                // 那么，新添加进来的串，要么是其中一个或者多个的子串，要么就是一个或者
                // 多个的父串！
                // 前者，意味着直接抛弃即可；
                // 对于后者，则需要抛弃rule；
                iterator it = this->begin();
                while (it != this->end()) {
                    if (isEqual(*it, rule) || isSubstrOf(rule, *it)) {
                        //std::cout << "`" << *it << "` contains `" << rule << "`" << std::endl;
                        return false;
                    }
                    if (isSubstrOf(*it, rule)) {
                        // std::cout << "`" << rule << "` contains `" << *it << "`" << std::endl;
                        it = this->erase(it);
                        continue;
                    }
                    ++it;
                }
                this->BaseT::push_back(rule);
                return true;
            }

            bool match(const std::string& fname) const
            {
                for (const_iterator it = this->begin(); it != this->end(); ++it)
                {
                    if (!isSubstrOf(*it, fname)) {
                        return false;
                    }
                }
                return true;
            }
        };

        class ExcludeRules : private std::vector<std::string>
        {
        public:
            typedef std::vector<std::string> BaseT;
            using BaseT::iterator;
            using BaseT::const_iterator;
            using BaseT::begin;
            using BaseT::end;

        public:
            ExcludeRules()
            {
            }

            ~ExcludeRules()
            {
            }

        public:

            // 保留"真子串"；抛弃"相等"和"父串"
            bool insert(const std::string& rule)
            {
                // std::cout << __func__ << " " << rule << std::endl;

                // NOTE，首先，当前rule集合，互不为对方的真子串！
                // 那么，新添加进来的串，要么是其中一个或者多个的子串，要么就是一个或者
                // 多个的父串！
                //
                // 如果是原有的真子串，需要删除原有串（父串）；
                // 如果等于原有，或者是原有的父串，则抛弃该rule；
                iterator it = this->begin();
                while ( it != this->end()) {
                    if (isEqual(*it, rule) || isSubstrOf(*it, rule)) {
                        // std::cout << "`" << rule << "` contains `" << *it << "`" << std::endl;
                        return false;
                    }
                    if (isSubstrOf(rule, *it)) {
                        // std::cout << "`" << *it << "` contains `" << rule << "`" << std::endl;
                        it = this->erase(it);
                        continue;
                    }
                    ++it;
                }
                this->BaseT::push_back(rule);
                return true;
            }

            // "禁用"规则，到底应该返回true还是false表示"通过"呢？
            // 要不这样，用true，表示触犯了"禁用条例"？
            bool match(const std::string& fname) const
            {
                for (const_iterator it = this->begin(); it != this->end(); ++it)
                {
                    if (isSubstrOf(*it, fname)) {
                        return true;
                    }
                }
                return false;
            }

        };
    }

    class KeywordFilter : public sss::path::filter_t
    {
    public:
        KeywordFilter();
        KeywordFilter(int argc, char *argv[]);
        ~KeywordFilter();

    public:
        bool addWithPrefix(const char * kw);

        bool addLike(const char *);
        bool addHate(const char *);

        void terminalHighlightCout(std::ostream& o, const std::string& fname);

    protected:
        void checkConflict() const;

    public:
        virtual bool is_match(const sss::path::file_descriptor& fd);

    private:
        inner::IncludeRules _like_patterns;
        inner::ExcludeRules _hate_patterns;
    };

    }
}

#endif  /* __KEYWORDFILTER_HPP_1449451116__ */
