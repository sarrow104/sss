#ifndef  __PARSER_HPP_1450184979__
#define  __PARSER_HPP_1450184979__

#include <sss/util/StringSlice.hpp>
#include <sss/util/utf8.hpp>
#include <sss/utlstring.hpp>
#include <sss/algorithm.hpp>
#include <sss/log.hpp>

#include <limits>
#include <iterator>

#include <cstring>
//#ifndef __WIN32__
//#       include <cstdint>
//#endif
#include <cstdlib>
#include <cctype>
#include <stdint.h>

namespace sss {
    namespace util {
        template<typename Iterator>
        class Parser{
        public:
            typedef typename std::iterator_traits<Iterator>::value_type value_type;

            // 最开始，我的想法是让函数生成一个Rewinder对象，使得在判断式失败的
            // 时候，这个对象，会返回创建的时刻，Iterator开始的位置——
            // 但，这在C、C++语言中，是不可能的！
            // 因为，函数的参数传递顺序……
            // 要保证类函数的用法，只有一个办法，就是使用宏；
            //
            // #define rewinder_helper(r, it_beg, operations) \_
            // Rewinder r(Iterator& it_beg); \_
            // r.commit(.....);
            //
            // 不过，还是有少许问题；这样的宏，没法写到&&、||串式子里面；
            // 因为使用了分号——这是不同的语句了！串接的式子，只能允许逗号表达
            // 式；还有括号表达式——其内部，同样不能有分号；
            //
            // 就是说，我回到了老路！
            class Rewinder {
            public:
                Rewinder(Iterator & it_beg)
                    : m_beg(it_beg), m_beg_sv(it_beg), m_end(it_beg),
                    m_succeed(false), m_active(false)
                {
                    SSS_LOG_DEBUG("r(%p) %c\n", this, *m_beg_sv);
                }

                // 默认回到原位
                ~Rewinder()
                {
                    if (this->m_active) {
                        this->commit(m_succeed);
                    }
                }

            public:
                // the begin - rollbak-on-false - commit rule
                bool begin()
                {
                    SSS_LOG_DEBUG("r(%p) %c\n", this, *m_beg);
                    this->m_active = true;
                    this->m_beg_sv = this->m_beg;
                    this->m_end = this->m_beg;
                    this->m_succeed = false;
                    return true;
                }

                bool commit(bool is_success)
                {
                    SSS_LOG_DEBUG("r(%p) %s\n", this, is_success ? "succeed" : "failed");
                    if (!is_success && m_beg != m_beg_sv) {
                        SSS_LOG_DEBUG("r(%p) rewinde to %c\n", this, *m_beg_sv);
                        m_beg = m_beg_sv;
                    }
                    if (is_success) {
                        this->end();
                    }
                    this->m_active = false;
                    return is_success;
                }

                int distance() const
                {
                    return std::distance(this->m_beg_sv, this->m_beg);
                }

                bool end()
                {
                    this->m_succeed = true;
                    this->m_end = m_beg;
                    this->m_active = false;
                    return true;
                }

                bool is_commited() const
                {
                    return this->m_succeed;
                }

                sss::util::StringSlice<Iterator> getSlice() const {
                    return sss::util::StringSlice<Iterator>(m_beg_sv, m_succeed ? m_end : m_beg_sv);
                }

            private:
                Iterator & m_beg;
                Iterator   m_beg_sv;
                Iterator   m_end;
                bool       m_succeed;
                bool       m_active;
            };

            static
            inline bool parseUint32_t(Iterator & it_beg, Iterator it_end,
                                      uint32_t & value, int width = 0)
            {
                if (width <= 0) {
                    width = 0;
                }
                if (isDigit(it_beg, it_end)) {
                    value = *it_beg - '0';
                    it_beg++;
                    width--;
                    // 如果width本来是0，那么此时就是-1；
                    // 如果width本来是正数n，那么此时是n-1；当n-1==0的时候，不会
                    // 进入循环；
                    // 当然n-1 > 0的时候，则会在n - 1 == 0的时候，停止循环；
                    while (isDigit(it_beg, it_end) && width != 0 &&
                           (value < std::numeric_limits<uint32_t>::max()/10u ||
                            (value == std::numeric_limits<uint32_t>::max()/10u &&
                             uint32_t(*it_beg - '0') <= std::numeric_limits<uint32_t>::max()%10u)))
                    {
                        value *= 10;
                        value += *it_beg - '0';
                        it_beg++;
                        width--;
                    }
                    return true;
                }
                return false;
                // return width <= 0 && (!isDigit(it_beg, it_end) || isEnd(it_beg, it_end))
            }

            static
            inline bool parseChar(Iterator & it_beg, Iterator it_end, char ch)
            {
                if (it_beg != it_end && *it_beg == ch) {
                    it_beg++;
                    // SSS_LOG_DEBUG("%c\n", ch);
                    return true;
                }
                return false;
            }

            static
            inline bool parseAnyChar(Iterator & it_beg, Iterator it_end, char& ch)
            {
                if (it_beg != it_end) {
                    ch = *it_beg++;
                    return true;
                }
                return false;
            }

            // 流程：
            // 1. 先开head字节
            // 2. 通过head字节，判断必须获取的长度；
            // 3. 如果需要读取后续字节，预取他们，然后拼装；
            static
            inline bool parseUtf8Any(Iterator& it_beg, Iterator it_end, uint32_t& code_page)
            {
                std::pair<uint32_t, int> st = sss::util::utf8::peek(it_beg, it_end);
                if (st.second) {
                    code_page = st.first;
                    std::advance(it_beg, st.second);
                }
                return st.second;
            }

            static inline bool parseUtf8(Iterator & it_beg, Iterator it_end, const char * s)
            {
                bool ret = false;
                const char * u8_beg = s;
                const char * u8_end = std::strchr(s, '\0');
                uint32_t u8_tar = 0u;
                if (parseUtf8Any(u8_beg, u8_end, u8_tar)) {
                    if (sss::is_begin_with(it_beg, it_end, s, u8_beg)) {
                        std::advance(it_beg, std::distance(s, u8_beg));
                        ret = true;
                    }
                }
                return ret;
            }

            static inline bool parseUtf8(Iterator & it_beg, Iterator it_end, const std::string& s)
            {
                bool ret = false;
                std::string::const_iterator u8_beg = s.begin();
                std::string::const_iterator u8_end = s.end();
                uint32_t u8_tar = 0u;
                if (parseUtf8Any(u8_beg, u8_end, u8_tar)) {
#ifdef __WIN32__
                    std::string::const_iterator s_beg = s.begin();
                    if (sss::is_begin_with(it_beg, it_end, s_beg, u8_beg)) {
                        std::advance(it_beg, std::distance(s_beg, u8_beg));
                        ret = true;
                    }
#else
                    if (sss::is_begin_with(it_beg, it_end, s.cbegin(), u8_beg)) {
                        std::advance(it_beg, std::distance(s.cbegin(), u8_beg));
                        ret = true;
                    }
#endif
                }
                return ret;
            }

            static inline bool parseUtf8(Iterator & it_beg, Iterator it_end, uint32_t ch_tar)
            {
                bool ret = true;
                uint32_t ch = 0u;
                Iterator it_beg_sv = it_beg;
                if (!parseUtf8Any(it_beg, it_end, ch) || ch != ch_tar) {
                    it_beg = it_beg_sv;
                    ret = false;
                }
                return ret;
            }

            static inline bool parseHexChar(Iterator & it_beg, Iterator it_end, char& ch)
            {
                if (it_beg != it_end && std::isxdigit(*it_beg)) {
                    ch = *it_beg++;
                    return true;
                }
                return false;
            }

            static inline bool parseSetChar(Iterator & it_beg, Iterator it_end, const char * to_match, char& ch)
            {
                // SSS_LOG_DEBUG("%c\n", *it_beg);
                if (it_beg != it_end && to_match && std::strchr(to_match, *it_beg)) {
                    ch = *it_beg++;
                    return true;
                }
                return false;
            }

            static inline bool parseSetUtf8(Iterator & it_beg, Iterator it_end, const std::string& to_match, uint32_t& ch)
            {
                std::pair<uint32_t, int> st = sss::util::utf8::peek(it_beg, it_end);
                // if (st.second && to_match.find(it_beg, it_beg + st.second) != std::string::npos) {
                if (st.second) {
                    std::string u8ch(it_beg, it_beg + st.second);
                    if (to_match.find(u8ch) != std::string::npos) {
                        ch = st.first;
                        std::advance(it_beg, st.second);
                        return true;
                    }
                }
                return false;
            }

            static inline bool parseSetUtf8(Iterator & it_beg, Iterator it_end, const char * to_match, uint32_t& ch)
            {
                return parseSetUtf8(it_beg, it_end, std::string(to_match), ch);
            }

            static inline bool parseSetChar(Iterator & it_beg, Iterator it_end, const std::string& to_match, char& ch)
            {
                // SSS_LOG_DEBUG("%c\n", *it_beg);
                if (it_beg != it_end && to_match.find(*it_beg) != std::string::npos) {
                    ch = *it_beg++;
                    // SSS_LOG_DEBUG("%c\n", ch);
                    return true;
                }
                return false;
            }

            template <typename Iterator2>
                static inline bool parseSetChar(Iterator & it_beg, Iterator it_end, Iterator2 range_beg, Iterator2 range_end, char& ch)
                {
                    if (it_beg != it_end && std::find(range_beg, range_end, *it_beg) != range_end) {
                        ch = *it_beg++;
                        return true;
                    }
                    return false;
                }


            template<typename Func>
                static inline bool parseIfChar(Iterator & it_beg, Iterator it_end, Func f, char& ch)
                {
                    if (it_beg != it_end && f(*it_beg)) {
                        ch = *it_beg++;
                        // SSS_LOG_DEBUG("%c\n", ch);
                        return true;
                    }
                    return false;
                }

            // [_a-zA-Z][_a-zA-Z0-9]*
            static inline bool parseCIdentifier(Iterator & it_beg, Iterator it_end, sss::util::StringSlice<Iterator>& id)
            {
                char holder = '\0';
                // SSS_LOG_DEBUG("%c\n", *it_beg);
                Iterator it_beg_bak = it_beg;
                if (parseIfChar(it_beg, it_end, ::isalpha, holder) || parseChar(it_beg, it_end, '_')) {
                    while (parseIfChar(it_beg, it_end, ::isalnum, holder) || parseChar(it_beg, it_end, '_'));
                    id.assign(it_beg_bak, it_beg);
                    // SSS_LOG_DEBUG("%s\n", id.str().c_str());
                    return true;
                }
                return false;
            }

            static inline bool parseRangeChar(Iterator & it_beg, Iterator it_end, char range_beg, char range_end, char& ch)
            {
                if (it_beg != it_end && range_beg <= *it_beg && range_end >= *it_beg) {
                    ch = *it_beg++;
                    return true;
                }
                return false;
            }

            static inline bool parseRangeUtf8(Iterator & it_beg, Iterator it_end,
                                              uint32_t range_beg, uint32_t range_end, uint32_t& ch)
            {
                std::pair<uint32_t, int> ret = sss::util::utf8::peek(it_beg, it_end);
                if (ret.second && ret.first >= range_beg && ret.first <= range_end) {
                    ch = ret.first;
                    std::advance(it_beg, ret.second);
                    return true;
                }
                return false;
            }

            template <typename Iterator2>
                static inline bool parseSequence(Iterator & it_beg, Iterator it_end, Iterator2 seq_beg, Iterator2 seq_end)
                {
                    if (sss::is_begin_with(it_beg, it_end, seq_beg, seq_end)) {
                        std::advance(it_beg, std::distance(seq_beg, seq_end));
                        return true;
                    }
                    return false;
                }

            static inline bool parseSequence(Iterator & it_beg, Iterator it_end, const char * seq)
            {
                if (sss::is_begin_with(it_beg, it_end, seq)) {
                    SSS_LOG_DEBUG("%s with %s succeed\n", std::string(it_beg, it_end).c_str(), seq);
                    std::advance(it_beg, std::strlen(seq));
                    return true;
                }
                SSS_LOG_DEBUG("%s with %s failed\n", std::string(it_beg, it_end).c_str(), seq);
                return false;
            }

            static inline bool parseSequence(Iterator & it_beg, Iterator it_end, const std::string& seq)
            {
                if (sss::is_begin_with(it_beg, it_end, seq)) {
                    std::advance(it_beg, seq.length());
                    return true;
                }
                return false;
            }

            static inline bool parseAfterSequence(Iterator & it_beg, Iterator it_end, const char * seq)
            {
                if (parseBeforeSequence(it_beg, it_end, seq)) {
                    SSS_LOG_DEBUG("%s with %s succeed\n", std::string(it_beg, it_end).c_str(), seq);
                    if (seq && *seq) {
                        it_beg += std::strlen(seq);
                    }
                    return true;
                }
                SSS_LOG_DEBUG("%s with %s failed\n", std::string(it_beg, it_end).c_str(), seq);
                return false;
            }

            // .(`ls *`) with .(`
            static inline bool parseBeforeSequence(Iterator & it_beg, Iterator it_end, const char * seq)
            {
                // 空串，匹配任意
                if (!seq || !*seq) {
                    return true;
                }
                Iterator it_beg_bak = it_beg;
                size_t offset = std::strlen(seq);
                if (std::distance(it_beg, it_end) < offset) {
                    SSS_LOG_DEBUG("%s with %s failed\n", std::string(it_beg, it_end).c_str(), seq);
                    return false;
                }

                while (it_beg != it_end - offset + 1) {
                    SSS_LOG_DEBUG("test %s with %s\n", std::string(it_beg, it_end).c_str(), seq);
                    if (sss::is_begin_with(it_beg, it_end, seq)) {
                        return true;
                    }
                    it_beg ++;
                }
                it_beg = it_beg_bak;
                return false;
            }

            template<typename ContainerT>
                static inline bool parseSequenceList(Iterator & it_beg, Iterator it_end, const ContainerT& t, size_t & id)
                {
                    for (size_t i = 0; i != sss::size(t); ++i)
                    {
                        if (sss::is_begin_with(it_beg, it_end, sss::begin(t[i]), sss::end(t[i]))) {
                            std::advance(it_beg, std::distance(sss::begin(t[i]), sss::end(t[i])));
                            id = i;
                            return true;
                        }
                    }
                    return false;
                }

            template<typename Iterator2>
                static inline bool parseSequenceList(Iterator & it_beg, Iterator it_end, Iterator2 list_beg, Iterator2 list_end, Iterator2& id)
                {
                    for (Iterator2 it = list_beg; it != list_end; ++it, ++id)
                    {
                        if (sss::is_begin_with(it_beg, it_end, sss::begin(*it), sss::end(*it))) {
                            std::advance(it_beg, std::distance(sss::begin(*it), sss::end(*it)));
                            id = it;
                            return true;
                        }
                    }
                    return false;
                }

            static inline bool parseWsChar(Iterator & it_beg, Iterator it_end, char & ch)
            {
                return parseIfChar(it_beg, it_end, ::isspace, ch);
            }

            // FIXME
            static inline bool parseNlChar(Iterator & it_beg, Iterator it_end, char & ch)
            {
                return parseIfChar(it_beg, it_end, ::isspace, ch);
            }

            static
                inline bool isDigit(Iterator it_beg, Iterator it_end)
                {
                    return it_beg != it_end && std::isdigit(*it_beg);
                }

            static
                inline bool isEnd(Iterator it_beg, Iterator it_end)
                {
                    return it_beg == it_end;
                }

            static int countIfEqualChar(Iterator it_beg, Iterator it_end, char ch)
            {
                int cnt = 0;
                while (it_beg != it_end && *it_beg == ch) {
                    it_beg++;
                    cnt++;
                }
                return cnt;
            }
        };
    }
}


#endif  /* __PARSER_HPP_1450184979__ */
