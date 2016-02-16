#include "KeywordFilter.hpp"

#include <stdexcept>

#include <sss/Terminal.hpp>
#include <sss/utlstring.hpp>

namespace {
    bool isSubstrOf(const std::string& inner, const std::string& outer)
    {
        return outer.end() != std::search(outer.begin(), outer.end(),
                                          inner.begin(), inner.end(),
                                          sss::char_equal_casei());
    }
}

namespace sss {

    namespace path {

        KeywordFilter::KeywordFilter()
        {
        }

        KeywordFilter::KeywordFilter(int argc, char *argv[])
        {
            for (int i = 0; i < argc; ++i) {
                this->addWithPrefix(argv[i]);
            }
        }

        KeywordFilter::~KeywordFilter()
        {
        }

        //    for (int i = 1; i < argc; ++i) {
        //        if (argv[i] && argv[i][0] == '-') {
        //            if (argv[i][1]) {
        //            ...
        //            }
        //        }
        //        else {
        //        ...
        //        }
        //    }
        // NOTE 所谓冲突，当某一个hate规则，是like规则的"相等"，或者"真子串"关系的时候，
        // 就意味着规则冲突了——此时，不可能有条目，被该过滤器匹配到！
        //
        // 首先，当前没有冲突——规则数，都是0条；
        // 此时，任意一条like规则还是hate规则，同样也不会有冲突；
        // 假设，n条添加之后，没有冲突项；
        // 就是说，这n条，满足如下关系：
        //   1. like条目互相不是对方的父串；(允许串之间有重叠区域)
        //   2. hate条目，互相不是对方的子串；(同样，也允许串之间有重叠)
        //   3. 任何hate条目，都不是like条目的子串；
        //
        // 那么，在n条的基础上，再增加一条hate规则，会怎么样？
        //   new_hate，如果是原有like规则的某一条的子串——或者相等，那么冲突！
        //   如果没有冲突，那么，就可以进入添加hate规则的流程——添加之后，同样不会影响
        //   之前的冲突条件；
        //   三种情况：a. 真子串；因不和like冲突，那么删除已有的hate条目，也没有关系；
        //   b. 父串，同样，也没有关系；c. 完全不重叠，或者部分重叠，同样没有关系；
        //
        // 那么，在n条的基础上，再增加一条like规则，会怎么样？
        //   new_like，如果是原有的某条hate规则的父串——或者相对，那么冲突！
        //   接着，添加的时候，三种情况：
        //     a. 真子串；因不和like冲突，抛弃，也没有关系；
        //     b. 父串，需要删除原有子串；也没有关系；
        //     c. 完全不重叠，或者部分重叠，同样没有关系；
        void KeywordFilter::checkConflict() const
        {
            for (inner::IncludeRules::const_iterator it_like = _like_patterns.begin();
                 it_like != _like_patterns.end();
                 it_like++)
            {
                for (inner::ExcludeRules::const_iterator it_hate = _hate_patterns.begin();
                     it_hate != _hate_patterns.end();
                     it_hate++)
                {
                    if (isSubstrOf(*it_hate, *it_like)) {
                        std::ostringstream oss;
                        oss << "`" << "-" << *it_hate << "` conflict with `" << *it_like << "`" << std::endl;
                        throw std::logic_error(oss.str());
                    }
                }
            }
        }

        bool KeywordFilter::addWithPrefix(const char * kw)
        {
            if (!kw) {
                return false;
            }
            if (kw[0] == '-') {
                return this->addHate(kw + 1);
            }
            else {
                return this->addLike(kw);
            }
        }

        // 保留"父串"；抛弃"相等"和"真子串"
        bool KeywordFilter::addLike(const char * kw)
        {
            for (inner::ExcludeRules::const_iterator it_hate = _hate_patterns.begin();
                 it_hate != _hate_patterns.end();
                 it_hate++)
            {
                if (isSubstrOf(*it_hate, kw)) {
                    std::ostringstream oss;
                    oss << "`" << "-" << *it_hate << "` conflict with `" << kw << "`" << std::endl;
                    throw std::logic_error(oss.str());
                }
            }

            return _like_patterns.insert(kw);
        }

        // 保留"真子串"；抛弃"相等"和"父串"
        bool KeywordFilter::addHate(const char * kw)
        {
            for (inner::IncludeRules::const_iterator it_like = _like_patterns.begin();
                 it_like != _like_patterns.end();
                 it_like++)
            {
                if (isSubstrOf(kw, *it_like)) {
                    std::ostringstream oss;
                    oss << "`" << "-" << kw << "` conflict with `" << *it_like << "`" << std::endl;
                    throw std::logic_error(oss.str());
                }
            }

            return _hate_patterns.insert(kw);
        }

        bool KeywordFilter::is_match(const sss::path::file_descriptor& fd)
        {
            return _like_patterns.match(fd.get_name()) && !_hate_patterns.match(fd.get_name());
        }

        sss::Terminal::style::begin font_style_list[] = {
            sss::Terminal::style::begin(sss::Terminal::style::FONT_BOLD | sss::Terminal::style::FONT_F_RED),
            sss::Terminal::style::begin(sss::Terminal::style::FONT_DARK |
                                        sss::Terminal::style::FONT_F_RED | sss::Terminal::style::FONT_F_GREEN),
            sss::Terminal::style::begin(sss::Terminal::style::FONT_BOLD | sss::Terminal::style::FONT_F_GREEN),
            sss::Terminal::style::begin(sss::Terminal::style::FONT_DARK | sss::Terminal::style::FONT_F_CYAN),
            sss::Terminal::style::begin(sss::Terminal::style::FONT_BOLD | sss::Terminal::style::FONT_F_BLUE),
            sss::Terminal::style::begin(sss::Terminal::style::FONT_DARK | sss::Terminal::style::FONT_F_PURPLE)
        };

        // FIXME TODO
        // 这种高亮输出，应该自己检查 isatty(STDOUT_FILENO)，然后得出结论，到底是否……
        void KeywordFilter::terminalHighlightCout(std::ostream& o, const std::string& fname)
        {
            std::string out(fname);

            int color_idx = 0;
            for (inner::IncludeRules::const_iterator it = _like_patterns.begin();
                 it != _like_patterns.end();
                 ++it)
            {
                sss::wrapper_replace_all(out, *it,
                                         font_style_list[color_idx].data(),
                                         sss::Terminal::style::end.data(),
                                         true);
                if (++color_idx == sizeof(font_style_list) / sizeof(font_style_list[0])) {
                    color_idx = 0;
                }
            }
            o << out;
        }

    }
}
