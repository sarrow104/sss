#include <sss/regex/simpleregex.hpp>

//#include <cstdint>
#include <limits>

#include <sstream>
#include <stdexcept>
#include <map>
#include <set>
#include <list>

#include <sss/utlstring.hpp>
#include <sss/log.hpp>

#define _MODEL_NAME_ "sss::regex::simpleregex"
#define SSS_REG_FUNC_TRACE

namespace sss{
    namespace regex {

        inline bool isword(char ch) // {{{1
        {
            return std::isalpha(ch) || ch == '_';
        }

        inline bool isdigitword(char ch) // {{{1
        {
            return std::isalnum(ch) || ch == '_';
        }

        // 截取串前20字符并返回；超过的话，附上"..."代表未完
        static const char * get_match_type_name(simpleregex::match_type_t t) // {{{1
        {
            static std::map<simpleregex::match_type_t, const char *> names;

            // vim: '<,'>s/^\S\+$/names[simpleregex::\0] = "\0";/ge

            if (names.empty()) {
                names[simpleregex::NULL_MATCH] = "NULL_MATCH";
                names[simpleregex::START_MATCH] = "START_MATCH";
                names[simpleregex::END_MATCH] = "END_MATCH";
                names[simpleregex::WORD_L_MATCH] = "WORD_L_MATCH";
                names[simpleregex::WORD_R_MATCH] = "WORD_R_MATCH";
                names[simpleregex::CHAR_MATCH] = "CHAR_MATCH";
                names[simpleregex::SET_MATCH] = "SET_MATCH";
                names[simpleregex::SET_VERSE_MATCH] = "SET_VERSE_MATCH";
                names[simpleregex::DIGIT_MATCH] = "DIGIT_MATCH";
                names[simpleregex::ALPHA_MATCH] = "ALPHA_MATCH";
                names[simpleregex::ALNUM_MATCH] = "ALNUM_MATCH";
                names[simpleregex::SPACE_MATCH] = "SPACE_MATCH";
                names[simpleregex::NOTSPACE_MATCH] = "NOTSPACE_MATCH";
                names[simpleregex::ANY_MATCH] = "ANY_MATCH";
                names[simpleregex::ANCHOR_L_MATCH] = "ANCHOR_L_MATCH";
                names[simpleregex::ANCHOR_R_MATCH] = "ANCHOR_R_MATCH";
                names[simpleregex::STAR_MATCH] = "STAR_MATCH";
                names[simpleregex::PLUS_MATCH] = "PLUS_MATCH";
                names[simpleregex::REFER_MATCH] = "REFER_MATCH";
                names[simpleregex::BRANCH_MATCH] = "BRANCH_MATCH";
                names[simpleregex::TIMES_MATCH] = "TIMES_MATCH";
                names[simpleregex::QUESTION_MATCH] = "QUESTION_MATCH";
            }
            return names[t];
        }

        const char * simpleregex::match_token::get_type_name() const
        {
            return get_match_type_name(this->get_type());
        }

        simpleregex::match_token::match_token(match_type_t t, const std::string& str, match_case_t m_case) // {{{1
            : type(t), match_case(m_case), anchor_id(0), data(str)
        {
            SSS_LOG_DEBUG("%s %s\n",
                          this->get_type_name(),
                          data.c_str());
            this->init();
        }

        simpleregex::match_token::match_token(match_type_t t, const std::string& str, int pos, match_case_t m_case) // {{{1
            : type(t), match_case(m_case), anchor_id(pos), data(str)
        {
            SSS_LOG_DEBUG("%s %s %d\n",
                          this->get_type_name(),
                          data.c_str(),
                          anchor_id);
            this->init();
        }

        bool simpleregex::match_token::init()   // {{{1
        {
            this->sort_with_case();

            // 关于比较次数，以及是否贪婪。
            // vim的做法，与其他的一些正则表达式引擎不同；
            // js里面，
            // + = {1,}
            // * = {}, {0,}
            // ? = {0,1}
            // {n} = {n,n}
            //
            // 此外，? 可以作为次数的后缀，以表示是否开启非贪婪模式。

            // 而 vim 中，没有 ? 后缀模式。而是用 {} 的负号，来表示是否贪婪。
            //
            // \{n,m}	Matches n to m of the preceding atom, as many as possible
            // \{n}	Matches n of the preceding atom
            // \{n,}	Matches at least n of the preceding atom, as many as possible
            // \{,m}	Matches 0 to m of the preceding atom, as many as possible
            // \{}	Matches 0 or more of the preceding atom, as many as possible (like *)
            //
            // \{-n,m}	matches n to m of the preceding atom, as few as possible
            // \{-n}	matches n of the preceding atom
            // \{-n,}	matches at least n of the preceding atom, as few as possible
            // \{-,m}	matches 0 to m of the preceding atom, as few as possible
            // \{-}	matches 0 or more of the preceding atom, as few as possible
            //
            // 就是说，vim 模式，优先使用 \{} 加 - 即可。
            //
            // 这下郁闷了。我还是先实现 vim 模式吧!

            switch (this->get_type()) {
            case TIMES_MATCH:
                {
                    int consume = simpleregex::consume_times_token(data.begin() + 1,
                                                                   data.end(),
                                                                   this->greed,
                                                                   this->t_min,
                                                                   this->t_max);
                    if (!consume) {
                        throw std::invalid_argument("invalid TIMES_MATCH: \"" + data + "\".");
                    }
                }
                break;

            case STAR_MATCH:
                this->greed = true;
                this->t_min = 0;
                this->t_max = -1; // means no limits.
                break;

            case PLUS_MATCH:
                this->greed = true;
                this->t_min = 1;
                this->t_max = -1; // means no limits.
                break;

            case QUESTION_MATCH:
                this->greed = true;
                this->t_min = 0;
                this->t_max = 1;
                break;

            default:
                break;
            }

            return true;
        }

        void simpleregex::match_token::sort_with_case() // {{{1
        {
            // NOTE 本函数的目的在于，构造set\set_inverse集合的时候，先进行排序
            // ？
            if (!this->is_case_sensitive()) {
                std::sort(this->data.begin(), this->data.end(), sss::char_less_casei());
            }
        }

        bool simpleregex::match_token::is_case_sensitive() const // {{{1
        {
            return this->match_case == case_sensitive;
        }

        bool simpleregex::match_token::enable_case_sensitive() // {{{1
        {
            if (this->is_case_sensitive()) {
                return false;
            }
            // case_insensitive -> case_sensitive
            switch (this->get_type()) {
            case SET_MATCH:
            case SET_VERSE_MATCH:
                std::sort(this->data.begin(), this->data.end());
                break;

            default:
                break;
            }
            this->match_case = case_sensitive;
            return true;
        }

        bool simpleregex::match_token::enable_case_insensitive() // {{{1
        {
            if (!this->is_case_sensitive()) {
                return false;
            }

            // case_sensitive -> case_insensitive
            if (this->is_set_type()) {
            }
            switch (this->get_type()) {
            case SET_MATCH:
            case SET_VERSE_MATCH:
                std::sort(this->data.begin(), this->data.end(), sss::char_less_casei());
                break;

            default:
                break;
            }

            this->match_case = case_insensitive;
            return true;
        }

        bool simpleregex::match_token::is_set_type() const // {{{1
        {
            switch (this->get_type())
            {
            case SET_MATCH:
            case SET_VERSE_MATCH:
                return true;

            default:
                return false;
            }
            return false;
        }

        // TODO
        bool simpleregex::match_token::next(int & times) const
        {
            (void)times;
            return true;
        }

        // TODO
        bool simpleregex::match_token::is_in_range(int times) const
        {
            (void)times;
            return true;
        }

        bool simpleregex::match_token::match(char ch) const // {{{1
        {
            try {
                // NOTE 如何支持忽略大小写的比较？
                // 可以在一开始创建的时候，就体现这种不同。特别是 SET_MATCH
                // ，可以使用大堆；还有 set<std::string> ，传入比较函数。
                // 还有一种办法是使用 vector堆；只不过，在最终find比较的时候
                // ，使用忽略大小写的比较函数（或者对象）
                // 还有，就是 std::set 的 find 插入插入策略。这就限制了变化
                // ……即，simpleregex 在一开始，就要从内部数据上，体现忽略
                // 大小写的不同。
                // 当然，原始数据，还是需要用类set来过滤重复的数据。
                // 不过，就算是vector 模拟的 堆排序即可，在是否忽略大小写的
                // 情况下，顺序还是不同的！
                // 以最大堆为例：
                // std::make_heap(v.begin(), v.end(), std::less)
                // 将生成以根节点为最大值，其左右子树根节点，不大于本根节点
                // 的二叉树。
                // 即，为了体现效率，在切换比较行为时（是否大小写敏感），需
                // 要对各个match成员数据，进行重排。
                // 当然，也可以不管效率，直接用std::find……
                // 另外，排序后，应该用 二分查找 好一点。
                // lower_bound
                // upper_bound
                //
                // 于是，对应是否大小写敏感的修改，将分为两步进行：
                // 1. 修改内部数据结构――set->vector，+ lower_bound or
                // upper_bound
                // 2. 根据属性，来判断大小写敏感。
                //
                // 还有一个办法，可以用空间换时间――即，同时存储两种风格的
                // 内存比较序列。
                // 不过，上述变化真心很少出现……

                bool ret = false;

                if (!this->is_case_sensitive()) {
                    ch = std::toupper(ch);
                }
                switch (this->get_type()) {
                case NULL_MATCH:
                    throw std::invalid_argument("NULL_MATCH token matched!");
                    break;

                case CHAR_MATCH:
                    if (this->is_case_sensitive()) {
                        ret = this->data[0] == ch;
                    }
                    else {
                        ret = std::toupper(this->data[0]) == ch;
                    }
                    break;

                case SET_MATCH:
                    if (this->is_case_sensitive()) {
                        ret = std::binary_search(this->data.begin(), this->data.end(), ch);
                    }
                    else {
                        ret = std::binary_search(this->data.begin(), this->data.end(), ch, sss::char_less_casei());
                    }
                    break;

                case SET_VERSE_MATCH:
                    if (this->is_case_sensitive()) {
                        ret = !std::binary_search(this->data.begin(), this->data.end(), ch);
                    }
                    else {
                        ret = !std::binary_search(this->data.begin(), this->data.end(), ch, sss::char_less_casei());
                    }
                    break;

                case DIGIT_MATCH:
                    if (std::isdigit(ch)) {
                        ret = true;
                    }
                    break;

                case ALPHA_MATCH:
                    if (isword(ch)) {
                        ret = true;
                    }
                    break;

                case ALNUM_MATCH:
                    if (isdigitword(ch)) {
                        ret = true;
                    }
                    break;

                case SPACE_MATCH:
                    if (std::isspace(ch)) {
                        ret = true;
                    }
                    break;

                case NOTSPACE_MATCH:
                    if (!std::isspace(ch)) {
                        ret = true;
                    }
                    break;

                case ANY_MATCH:
                    ret = true;
                    break;

                default:
                    ret = false;
                    break;
                }

                SSS_LOG_DEBUG("%s [%s : %c] %c (%s)\n",
                              this->get_type_name(),
                              this->data.c_str(),
                              ch,
                              ret ? 'T' : 'F',
                              this->is_case_sensitive() ? "case_sensitive" : "case_insensitive");
                return ret;
            }
            catch (std::exception & e) {
                std::cout << __FILE__ << ", " << __LINE__ << std::endl;
                throw;
            }
        }

        void simpleregex::match_token::print(std::ostream& o) const // {{{1
        {
            o << this->get_type_name() << "(" << this->data << ")";
        }

        simpleregex::simpleregex(const std::string& reg_str, match_case_t _match_case) // {{{1
            : reg_string(reg_str)
              , start_pos(-1)
              , consumed_cnt(0)
              , match_case(_match_case)
              , least_match(false)
        {
            SSS_LOG_DEBUG("(\"%s\", %s)\n", reg_str.c_str(), this->is_case_sensitive() ? "case_sensitive" : "case_insensitive");
            // 这里，应该对 reg_string 进行"分词"
            // 比如，将'^\[sql\d\] --$' 分割为：
            // START_MATCH  "^"
            // CHAR_MATCH   "["
            // CHAR_MATCH   "s"
            // CHAR_MATCH   "q"
            // CHAR_MATCH   "l"
            // DIGIT_MATCH  "\d"
            // CHAR_MATCH   "]"
            // CHAR_MATCH   " "
            // CHAR_MATCH   "-"
            // CHAR_MATCH   "-"
            // END_MATCH    "$"
            //
            // 其他：
            // ALNUM_MATCH "\w"

            // NOTE，默认"分支"，针对整个传――可以没有显式的\(\)
            // ――当然，也可以让正则表达式串，默认就用一组\(和\)包裹起来。
            // FIXME 这里应该改为stack结构！
            std::list<int> last_left_anchor;
            last_left_anchor.push_back(-1);

            for (size_t i = 0; i != reg_string.length(); ++i) {
                switch (reg_string[i]) {
                case '^':
                    match_tokens.push_back(match_token(START_MATCH, "^"));
                    break;

                case '$':
                    match_tokens.push_back(match_token(END_MATCH, "$"));
                    break;

                case '*':
                    if (match_tokens.empty() ||
                        match_tokens.back().get_type() == START_MATCH)
                    {
                        throw std::invalid_argument("STAR_MATCH token cannot at first!");
                    }
                    match_tokens.push_back(match_token(STAR_MATCH, "*"));
                    SSS_LOG_FUNC_CALL("match_token(STAR_MATCH, \"*\")");
                    this->record_times();

                    break;

                case '+':
                    if (match_tokens.empty() ||
                        match_tokens.back().get_type() == START_MATCH)
                    {
                        throw std::invalid_argument("PLUS_MATCH token cannot at first!");
                    }
                    match_tokens.push_back(match_token(PLUS_MATCH, "+"));
                    SSS_LOG_FUNC_CALL("match_token(PLUS_MATCH, \"+\")");
                    this->record_times();
                    break;

                case '[':       // SET_MATCH or SET_VERSE_MATCH
                    // NOTE 如果需要包含减号'-'，使用"\\-" 即可！
                    {
                        bool is_set_end = false;
                        typedef std::set<char> sub_set_t;
                        sub_set_t sub_set;
                        match_type_t current_type = SET_MATCH;
                        bool is_range = false;
                        char range_beg = '\0';
                        for (size_t j = i + 1; j != reg_string.length(); ++j) {
                            if (j == i + 1 && reg_string[j] == '^') {
                                current_type = SET_VERSE_MATCH;
                            }
                            else if (reg_string[j] == '\\' &&
                                     j + 1 != reg_string.length())
                            {
                                range_beg = reg_string[++j];
                                is_range = false;
                                sub_set.insert(range_beg);
                            }
                            else if (reg_string[j] == ']') {
                                i = j;
                                is_set_end = true;
                                break;
                            }
                            else if (reg_string[j] == '-') {
                                if (range_beg != '\0') {
                                    is_range = true;
                                }
                                else {
                                    throw std::invalid_argument("reg set encouter : [-]");
                                }
                                // NOTE 如果一开始，就是一个'-'；那么……
                                // 使用 "\\-" 转义！
                            }
                            else {
                                if (is_range) {
                                    char range_end = reg_string[j];
                                    for (char c = range_beg; c <= range_end; ++c) {
                                        sub_set.insert(c);
                                    }
                                    is_range = false;

                                    // 重新取为'\0'；以防止[a-d-k]这种形式！
                                    range_beg = '\0';
                                }
                                else {
                                    range_beg = reg_string[j];
                                    sub_set.insert(reg_string[j]);
                                }
                            }
                        }

                        if (!is_set_end) {
                            throw std::invalid_argument("reg set unmatched \"[\"");
                        }
                        std::string string_set;
                        for (sub_set_t::iterator it = sub_set.begin();
                             it != sub_set.end();
                             ++it)
                        {
                            string_set += *it;
                        }
                        match_tokens.push_back(match_token(current_type, string_set));
                    }
                    break;

                case '\\':
                    {
                        if (i + 1 == reg_string.length()) {
                            throw std::invalid_argument("转义字符不完整");
                        }
                        switch (reg_string[++i]) {
                        case '[':
                            match_tokens.push_back(match_token(CHAR_MATCH, "["));
                            break;

                        case ']':
                            match_tokens.push_back(match_token(CHAR_MATCH, "]"));
                            break;

                        case '^':
                            match_tokens.push_back(match_token(CHAR_MATCH, "^"));
                            break;

                        case '$':
                            match_tokens.push_back(match_token(CHAR_MATCH, "$"));
                            break;

                        case '.': // 2014-08-19 漏了一个 \\. 小数点！
                            match_tokens.push_back(match_token(CHAR_MATCH, "."));
                            break;

                        case '*':
                            match_tokens.push_back(match_token(CHAR_MATCH, "*"));
                            break;

                        case '+':
                            match_tokens.push_back(match_token(CHAR_MATCH, "+"));
                            break;

                        case 's':
                            match_tokens.push_back(match_token(SPACE_MATCH, "\\s"));
                            break;

                        case 'S':
                            match_tokens.push_back(match_token(NOTSPACE_MATCH, "\\S"));
                            break;

                        case 'w':
                            match_tokens.push_back(match_token(ALNUM_MATCH, "\\w"));
                            break;

                        case 'c':
                            match_tokens.push_back(match_token(ALPHA_MATCH, "\\c"));
                            break;

                        case 'd':
                            match_tokens.push_back(match_token(DIGIT_MATCH, "\\d"));
                            break;

                        case '<':
                            match_tokens.push_back(match_token(WORD_L_MATCH, "\\<"));
                            break;

                        case '>':
                            match_tokens.push_back(match_token(WORD_R_MATCH, "\\>"));
                            break;

                        //! 如何进行左右锚点匹配？
                        // NOTE 允许嵌套！但是，不允许未完成的锚点。
                        // 也就是说，是否"合格"的要求，与带括号的运算式相同！
                        // 即，
                        // 1. 左括号，可以随便出现；
                        // 2. 右括号出现的时候，必须在左侧找到空闲的"左括号"
                        // 3. 解析完成的时候，没有孤立(未配对)的括号。
                        case '(':
                            match_tokens.push_back(match_token(ANCHOR_L_MATCH,
                                                               "\\(",
                                                               left_anchors.size()));
                            left_anchors.push_back(0);

                            last_left_anchor.push_back(match_tokens.size() - 1);

                            break;

                        case ')':
                            if (left_anchors.size() <= right_anchors.size()) {
                                throw std::invalid_argument("single right anchor!");
                            }
                            match_tokens.push_back(match_token(ANCHOR_R_MATCH,
                                                               "\\)",
                                                               right_anchors.size()));
                            anchorpairs[last_left_anchor.back()] = match_tokens.size() - 1;
                            anchorpairs_rev[match_tokens.size() - 1] = last_left_anchor.back();

                            if (this->left_anchor2branchs.find(last_left_anchor.back())
                                == this->left_anchor2branchs.end())
                            {
                                this->left_anchor2branchs[last_left_anchor.back()] = std::vector<int>();
                            }

                            right_anchors.push_back(0);
                            (void)last_left_anchor.pop_back();

                            break;

                        case '\\':
                            match_tokens.push_back(match_token(CHAR_MATCH, "\\"));
                            break;

                        case '|':
                            match_tokens.push_back(match_token(BRANCH_MATCH, "\\|"));
                            //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, last_left_anchor.back());

                            if (left_anchor2branchs.find(last_left_anchor.back()) == left_anchor2branchs.end()) {
                                // NOTE 分支是一分为二！
                                // 如果有一个分支标记，那么说明是两个分支。
                                // 而一个分支的话，相当于没有分支――即，不用额外处理。
                                left_anchor2branchs[last_left_anchor.back()] = std::vector<int>();
                            }
                            left_anchor2branchs[last_left_anchor.back()].push_back(match_tokens.size() - 1);
                            break;

                        case '{':
                            {
                                bool greed = false;
                                int t_min = 0;
                                int t_max = 0;
                                int consume = this->consume_times_token(reg_string.begin() + i,
                                                                        reg_string.end(),
                                                                        greed,
                                                                        t_min,
                                                                        t_max);
                                if (consume == 0) {
                                    throw std::invalid_argument(std::string("invalid times :\"\\")
                                                                + reg_string.substr(i - 1, 5)+"\"");
                                }

                                match_tokens.push_back(match_token(TIMES_MATCH,
                                                                   reg_string.substr(i - 1,
                                                                                     consume + 1)));
                                i += consume - 1;
                                SSS_LOG_FUNC_CALL("match_token(TIMES_MATCH, \""
                                                  + reg_string.substr(i - 1, consume + 1)
                                                  + "\")");
                                this->record_times();
                            }
                            break;

                        case '?':
                            // 因为 问号 在一般的文字中，还算常见；而匹配0或1次，在正则里面却不多见
                            // 故，这里设计为添加前缀；
                            match_tokens.push_back(match_token(QUESTION_MATCH, "\\?"));
                            SSS_LOG_FUNC_CALL("match_token(QUESTION_MATCH, \"\\?\")");
                            this->record_times();
                            break;

                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            {
                                int refer_id = reg_string[i] - '0';
                                if (int(right_anchors.size()) < refer_id) {
                                    throw std::invalid_argument(std::string("invalid reference id :\"\\")
                                                                + reg_string[i] + "\"");
                                }
                                // NOTE 没必要支持多位数……；于是……
                                match_tokens.push_back(match_token(REFER_MATCH,
                                                                   std::string("\\") + reg_string[i],
                                                                   reg_string[i] - '0'));
                            }
                            break;

                        default:
                            throw std::invalid_argument(std::string("unsupport escape string:\"\\")
                                                        + reg_string[i] + "\"");
                        }
                    }
                    break;

                case '.':
                    match_tokens.push_back(match_token(ANY_MATCH, "."));
                    break;

                default:
                    match_tokens.push_back(match_token(CHAR_MATCH,
                                                       std::string(1, reg_string[i])));
                }
            }

            if (left_anchors.size() != right_anchors.size()) {
                throw std::invalid_argument("anchor numbers not balance.");
            }

            if (!this->is_case_sensitive()) {
                this->sort_with_case();
            }

            for (std::map<int, std::vector<int> >::const_iterator it = left_anchor2branchs.begin();
                 it != left_anchor2branchs.end();
                 ++it)
            {
                for (std::vector<int>::const_iterator b_it = it->second.begin();
                     b_it != it->second.end();
                     ++b_it)
                {
                    branch2right_anchor[*b_it] =
                        (it->first == -1 ? match_tokens.size() : anchorpairs[it->first]);
                }
            }

            //if (this->left_anchor2branchs.find(-1) == this->left_anchor2branchs.end()) {
            //    this->left_anchor2branchs[-1] = std::vector<int>();
            //}

            this->tok_beg_bak = this->match_tokens.begin();

            //for (std::map<int, int>::const_iterator it = anchorpairs_rev.begin();
            //     it != anchorpairs_rev.end();
            //     ++it)
            //{
            //    SSS_LOG_DEBUG("[%d, %d)\n", it->first, it->second);
            //}

            for (std::map<int, int>::const_iterator it = anchorpairs.begin();
                 it != anchorpairs.end();
                 ++it)
            {
                SSS_LOG_DEBUG("[%d, %d)\n", it->first, it->second);
            }

            char s_format[] = "[%02d] = %s\n";
            s_format[3] = sss::cast_string(this->match_tokens.size() - 1).length() + '0';
            for (const_iterator it = this->begin();
                 it != this->end();
                 ++it)
            {
                SSS_LOG_DEBUG(s_format,
                              std::distance(this->tok_beg_bak, it),
                              sss::cast_string(*it).c_str());
            }
        }

        simpleregex::~simpleregex() // {{{1
        {
        }

        bool simpleregex::is_case_sensitive() const // {{{1
        {
            return this->match_case == simpleregex::case_sensitive;
        }

        bool simpleregex::enable_case_sensitive() // {{{1
        {
            if (this->is_case_sensitive()) {
                return false;
            }

            this->sort_with_case();
            this->match_case = case_sensitive;
            //for (simpleregex::iterator it = this->match_tokens.begin();
            //     it != this->match_tokens.end();
            //     ++it)
            //{
            //    match_token & match_cur = *it;
            //    match_cur.enable_case_sensitive();
            //}
            return true;
        }

        void simpleregex::sort_with_case() // {{{1
        {
            if (this->is_case_sensitive()) {
                std::for_each(this->match_tokens.begin(),
                              this->match_tokens.end(),
                              std::mem_fun_ref(&simpleregex::match_token::enable_case_sensitive));
            } else {
                std::for_each(this->match_tokens.begin(),
                              this->match_tokens.end(),
                              std::mem_fun_ref(&simpleregex::match_token::enable_case_insensitive));
            }
        }

        bool simpleregex::enable_case_insensitive() // {{{1
        {
            if (!this->is_case_sensitive()) {
                return false;
            }

            this->sort_with_case();

            this->match_case = case_insensitive;
            //for (simpleregex::iterator it = this->match_tokens.begin();
            //     it != this->match_tokens.end();
            //     ++it)
            //{
            //    match_token & match_cur = *it;
            //    match_cur.enable_case_insensitive();
            //}
            return true;
        }

        std::string simpleregex::get_submatch(int id) const // {{{1
        {
            if (id == 0) {
                return this->to_match.substr(this->start_pos,
                                             this->consumed_cnt);
            }
            else if (id > 0 && id <= this->get_submatch_count()) {
                return this->to_match.substr(this->get_submatch_start(id),
                                             this->get_submatch_consumed(id));
            }
            else {
                return "";
            }
        }

        int         simpleregex::get_submatch_count() const
        {
            return this->left_anchors.size();
        }

        int         simpleregex::get_submatch_start(int id) const
        {
            return this->left_anchors[id - 1];
        }

        int         simpleregex::get_submatch_end(int id) const
        {
            return this->right_anchors[id - 1];
        }

        int         simpleregex::get_submatch_consumed(int id) const
        {
            if (id == 0) {
                return this->consumed_cnt;
            }
            else {
                return this->get_submatch_end(id) - this->get_submatch_start(id);
            }
        }

        bool simpleregex::match(const std::string& str) // {{{1
        {
            return match(str.begin(), str.end());
        }

        bool simpleregex::match(const std::string& str, std::string& matched_sub) // {{{1
        {
            bool ret = match(str.begin(), str.end());
            if (ret) {
                matched_sub = str.substr(this->start_pos, this->consumed_cnt);
            }
            return ret;
        }

        bool simpleregex::match(const std::string& str, std::string& matched_sub, int& begin_pos) // {{{1
        {
            bool ret = this->match(str, matched_sub);
            if (ret) {
                begin_pos = this->start_pos;
            }
            return ret;
        }

        bool simpleregex::match(std::string::const_iterator str_beg, std::string::const_iterator str_end) // {{{1
        {
#ifdef SSS_REG_FUNC_TRACE
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
#endif

            // 备份匹配的起始位置
            this->to_match = std::string(str_beg, str_end);
            this->str_beg_bak = str_beg;
            this->start_pos = -1;
            this->consumed_cnt = 0;

            SSS_LOG_DEBUG("\"%s\" =~ simpleregex(\"%s\")\n",
                          sss::utlstr::sample_string(str_beg, str_end).c_str(),
                          this->reg_string.c_str());
            // 空的regex，能匹配所有的字符串
            if (match_tokens.empty()) {
                this->consumed_cnt = 0;
                return true;
            }

            // FIXME 2014-07-04 这里好像有问题！
            // START_MATCH 只出现在 match(,) 函数中！
            // 是不是应该在match_here 里面处理它？
            if (this->begin()->is_type(START_MATCH)) {
                SSS_LOG_FUNC_CALL("patch to match_here START_MATCH at [0] : START_MATCH");
                this->start_pos = 0;
                return match_here(this->begin() + 1,
                                  this->end(),
                                  str_beg,
                                  str_end);
            }

            do {
                std::ostringstream oss;
                oss << "patch to match_here loop at str["
                    << std::distance(str_beg_bak, str_beg)
                    << "]";

                SSS_LOG_FUNC_CALL(oss.str());

                // 每次循环开始，consumed_cnt 都应该清零；
                // 因为，就是因为没有匹配到，所以，才需要下一次循环；
                // 所以，consumed_cnt 肯定是 为0 的；
                this->consumed_cnt = 0;
                this->is_match_here_called = false; // 是否调用过 match_here 函数
                this->start_pos = std::distance(str_beg_bak, str_beg);

                if (sss::log::model(_MODEL_NAME_)) {
                    SSS_LOG_DEBUG("consumed_cnt = %d; start_pos = %d\n",
                                  this->consumed_cnt,
                                  this->start_pos);
                }

                if (match_here(this->begin(),
                               this->end(),
                               str_beg,
                               str_end))
                {
                    return true;
                }
            } while (++str_beg != str_end);
            return false;
        }

        bool simpleregex::match(std::string::const_iterator str_beg, std::string::const_iterator str_end, std::string& matched_sub) // {{{1
        {
            std::string::const_iterator match_beg;
            std::string::const_iterator match_end;

            bool ret = match(str_beg, str_end, match_beg, match_end);
            if (ret) {
                matched_sub = std::string(match_beg, match_end);
            }
            return ret;
        }

        // bool simpleregex::match(str_beg, str_end, match_beg, match_end) // {{{1
        bool simpleregex::match(std::string::const_iterator str_beg,
                                std::string::const_iterator str_end,
                                std::string::const_iterator& match_beg,
                                std::string::const_iterator& match_end)
        {
            bool ret = match(str_beg, str_end);
            if (ret) {
                match_beg = str_beg;
                std::advance(match_beg, this->start_pos);

                match_end = match_beg;
                std::advance(match_end, this->consumed_cnt);
            }
            return ret;
        }

        // bool simpleregex::match(str_beg, str_end, matched_sub, begin_pos) // {{{1
        bool simpleregex::match(std::string::const_iterator str_beg,
                                std::string::const_iterator str_end,
                                std::string& matched_sub,
                                int& begin_pos)
        {
            bool ret = match(str_beg, str_end, matched_sub);
            if (ret) {
                begin_pos = this->start_pos;
            }
            return ret;
        }

        // 替换
        struct subs_stem_t // {{{1
        {
            // NOTE 解析替换用字符串，将其拆分为替换的id或者字符串
            // TODO 以后可能支持内部脚本，以便进行复杂的表达式计算后替换。
            enum enum_subs_stem_t { ID = 0, STRING};

        private:
            enum_subs_stem_t type;
            int         subs_id;
            std::string str_val;

        public:
            explicit subs_stem_t(const std::string& s)
                : type(STRING), subs_id(0), str_val(s)
            {
            }

            explicit subs_stem_t(int id)
                : type(ID), subs_id(id)
            {
            }

        public:
            int get_id() const
            {
                return this->subs_id;
            }

            std::string get_str() const
            {
                return this->str_val;
            }

        public:
            static void parse_subs_str(const std::string& subs, std::vector<subs_stem_t>& out)
            {
                parse_subs_str(subs.begin(), subs.end(), out);
            }

            static void parse_subs_str(std::string::const_iterator subs_beg,
                                       std::string::const_iterator subs_end,
                                       std::vector<subs_stem_t>& out)
            {
                // NOTE，C++字符串，两个反斜杠，表示一个反斜杠！所以下面是6个反
                // 斜杠！
                sss::regex::simpleregex reg_subs("\\\\\\d\\>");

                // NOTE 需要特殊处理 "\\" 即可；
                std::string::const_iterator match_beg_it = subs_beg;
                std::string::const_iterator match_end_it = subs_end;

                std::string::const_iterator submatch_beg_it;
                std::string::const_iterator submatch_end_it;

                while (match_beg_it < match_end_it)
                {
                    if (reg_subs.match(match_beg_it, match_end_it,
                                       submatch_beg_it, submatch_end_it))
                    {
                        int id = sss::string_cast<int>(std::string(submatch_beg_it + 1, submatch_end_it));
                        if (match_beg_it != submatch_beg_it) {
                            std::string stem = std::string(match_beg_it, submatch_beg_it);
                            out.push_back(subs_stem_t(stem));
                        }
                        out.push_back(subs_stem_t(id));
                        match_beg_it = submatch_end_it;
                    }
                    else {
                        std::string stem = std::string(match_beg_it, match_end_it);
                        out.push_back(subs_stem_t(stem));
                        match_beg_it = match_end_it;
                    }
                }
            }

            static void print_subs_str(sss::regex::simpleregex& reg, const std::vector<subs_stem_t>& v_subs, std::ostream& oss)
            {
                for (size_t i = 0; i != v_subs.size(); ++i) {
                    const subs_stem_t& sub(v_subs[i]);
                    switch (sub.type)
                    {
                    case subs_stem_t::ID:
                        oss << reg.get_submatch(sub.get_id());

                    case subs_stem_t::STRING:
                        oss << sub.get_str();
                    }
                }
            }
        };

        std::string simpleregex::substitute(const std::string& s, const std::string& subs) // {{{1
        {
            return this->substitute(s.begin(), s.end(), subs.begin(), subs.end());
        }

        // bool simpleregex::substitute {{{1
        std::string simpleregex::substitute(std::string::const_iterator tar_beg,
                                            std::string::const_iterator tar_end,
                                            std::string::const_iterator subs_beg,
                                            std::string::const_iterator subs_end)
        {
            std::ostringstream oss;
            std::vector<subs_stem_t> v_subs;
            subs_stem_t::parse_subs_str(subs_beg, subs_end, v_subs);

            std::string::const_iterator match_beg_it = tar_beg;
            std::string::const_iterator match_end_it = tar_end;

            std::string::const_iterator submatch_beg_it;
            std::string::const_iterator submatch_end_it;

            while (match_beg_it < match_end_it)
            {
                if (this->match(match_beg_it, match_end_it,
                                submatch_beg_it, submatch_end_it))
                {
                    oss << std::string(match_beg_it, submatch_beg_it);
                    subs_stem_t::print_subs_str(*this, v_subs, oss);
                    match_beg_it = submatch_end_it;
                }
                else {
                    oss << std::string(match_beg_it, match_end_it);
                    match_beg_it = match_end_it;
                }
            }
            return oss.str();
        }

        int  simpleregex::sub_match_count() const
        {
            // 不算 "\\0"
            return left_anchors.size() - 1;
        }

        void simpleregex::print_match_info(std::ostream& out) // {{{1
        {
            out << "start_pos = " << start_pos
                << ", consumed_cnt = " << consumed_cnt << std::endl;
        }

        // bool simpleregex::match_here(tok_beg, tok_end, str_beg, str_end) {{{1

        //if( regex[0]=='\0' )
        //    return 1;
        //if( regex[1]=='*' )
        //    return matchStar(regex[0], regex+2, text);
        //if( strcmp(regexp, "$") == 0 )
        //    return *text=='\0';
        //if( *text!='\0' && (regex[0]=='.' || regex[0]==*text))
        //    return matchHere(regex+1, text+1);
        //return 0;
        bool simpleregex::match_here(simpleregex::const_iterator tok_beg,
                                     simpleregex::const_iterator tok_end,
                                     std::string::const_iterator str_beg,
                                     std::string::const_iterator str_end)
        {
#ifdef SSS_REG_FUNC_TRACE
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
#endif

            SSS_LOG_DEBUG("<=======================\n");
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::distance(this->tok_beg_bak, tok_beg));
            for (const_iterator it = tok_beg; it != tok_end; ++it) {
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *it);
            }
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::distance(str_beg_bak, str_beg));
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, sss::utlstr::sample_string(str_beg, str_end));

            simpleregex::states_t state_old;
            state_old.push(*this);

            try {
                bool ret = false;

                // 0 长的 regex，可以匹配任何字符串
                if (tok_beg == tok_end) { // {{{2
                    SSS_LOG_FUNC_CALL("empty reg-string.");
                    throw true;
                }

                // TODO 到底应该如何同时解决 范围(引用)与分支的问题？注意，用的是递归下降法
                // 焦点在于，这个循环的处理。我是用递归来模拟循环，还是用一个实际的循环呢？
                // 而且，还需要能复用 match_here。
                // 实际的循环的话，需要先分析同层的"分支"位置；比如，先创建一个分支index的数组；
                // 我觉得循环加复用match_here的话，需要让match_here自己完成后续的判
                // 断――跳出最内层的引用，再进行后续匹配。
                //
                // 递归模拟的话，就是在分支这里，做完后续匹配之后，进行判断，看是否
                // 能匹配。不匹配的话，就再往下一个分支；
                //
                // 就编程难度来说，还是"递归模拟"循环的方式简单一点。

                // 如果在最外层有分支：
                //   在match函数处，做一个调用match_here 的标记，然后在 match_here 这里，完成分支：
                if (this->left_anchor2branchs.find(-1) != this->left_anchor2branchs.end() && // {{{2
                    !this->is_match_here_called)
                {
                    SSS_LOG_FUNC_CALL("NONE anchor - branch.");
                    this->is_match_here_called = true;

                    // simpleregex::const_iterator branch_beg = tok_beg;

                    const std::vector<int>& branches = this->left_anchor2branchs[-1];

                    std::string::const_iterator str_beg_store = str_beg;

                    ret = this->match_branches(tok_beg,
                                               tok_end,
                                               branches,
                                               str_beg_store,
                                               str_end);

                    throw ret;
                }


                // 因为 *, {} 这种，允许0长匹配的存在
                // 因此，需要一个额外的数组或者函数，用来表示，当前节点，是否有对应的次数表示？
                // 即，在第一次处理次数动作的时候，就需要特殊处理，而不是等循环过后再做。
                // 如果允许0长匹配，那么，直接匹配 TIMES_MATCH 之后的节点；并记录下是否能匹配。
                // 其他情况，类似处理。
                // 总之，贪婪搜索，必须：
                // 1. 记录成功匹配；
                // 2. 每次匹配，需要走完整个正则表达式结构体；
                //
                // 至于非贪婪搜索，则是遇到第一次成功的匹配，就退出。
                //
                // 如何实现？
                //
                // 策略也简单：
                //  将 TIMES_MATCH 所引用的正则表达式，看作用整体，用 [begin, end)
                //  的方式，来定位，并进行比较。然后，将 TIMES_MATCH 之后的，作为后
                //  续串，再做一次匹配。
                //
                //  唯一需要注意的是，此处，不允许平移！必须马上得到匹配！――当然
                //  ，这是 match_here 函数能够保证的事情
                //
                //  这样的话，我必须额外，再定义一个数组，用来描述他是否被 TIMES_MATCH 节点控制着。
                //
                //  其他问题：
                //  1. 需要排除 多种 TIMES_MATCH 节点，串在一起的情况；
                //     从逻辑上，这种串接，是可以合并的。
                //
                //  2. 如何处理 分支 与 TIMES_MATCH ?
                //     我当前处理分支的模式，是记录了，分支的位置――依赖于串的开头，或者左锚点的位置；
                //     而对于 TIMES_MATCH 的处理，我又想在类似左锚点的位置，额外记
                //     录是否有对应的 TIMES_MATCH 节点；这就照成了冲突；
                //     理论上，我应该优先分析 TIMES_MATCH ，然后再处理分支；
                //
                //  3. 锚点嵌套如何处理？锚点应该记录最后一次匹配吗？
                //  'aa aba abba abbba abbbba '
                //  /\(a\(b*\)a \)\+
                //  s/\(a\(b*\)a \)\+/\1\2/ge
                //  'abbbba bbbb '
                //  根据实验，vim内部嵌套，锚点，记录的是最后一次匹配的位置。

                SSS_LOG_FUNC_CALL("check times_tok exists.");
                const_iterator times_tok = this->get_times_token(tok_beg);

                if (times_tok != this->end() && times_tok < tok_end) { // {{{2
                    SSS_LOG_DEBUG("in times match.\n");

                    const_iterator range_beg = tok_beg;
                    const_iterator range_end = times_tok;

                    // 在范围 {n,m} 之内，先完成 前面的 n 次匹配；
                    //
                    // 然后，在向 m 次的匹配过程中，进行逐步测试。

                    bool is_prev_n_ok = true;
                    std::string::const_iterator str_beg_store = str_beg;

                    // NOTE 前 get_min() 次匹配，与是否 is_greed() 无关！
                    // 并且，由于这些次数的匹配，必须成功，所以，也没有必要测试
                    // 性匹配――即，中途记录上次成功值！
                    for (int i = 0; i < times_tok->get_min(); ++i) {
                        std::ostringstream oss;
                        oss << "in times_tok->get_min() " << i;
                        SSS_LOG_FUNC_CALL(oss.str());

                        //std::string to_test =
                        //    sss::utlstr::sample_string(str_beg_store, str_end);

                        //SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::distance(this->tok_beg_bak, range_beg));
                        //for (const_iterator it = range_beg; it != range_end; ++it) {
                        //    SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *it);
                        //}

                        int consumed_cnt_store = this->consumed_cnt;

                        if (!this->match_here(range_beg,
                                              range_end,
                                              str_beg,
                                              str_end))
                        {
                            is_prev_n_ok = false;
                            break;
                        }
                        else {
                            std::advance(str_beg, this->consumed_cnt - consumed_cnt_store);
                        }
                    }

                    bool is_rest_ok = false;

                    // 之前 t_min 次匹配正常；此时需要测试最后这次t_min匹配之后，
                    // 余下正则串，是否也恰好能匹配余下的待匹配字符；
                    // 如果成功，记录该次匹配；
                    // 否则……
                    simpleregex::states_t states_last_succ;
                    simpleregex::states_t states_prev_rest;

                    // 马上进行 n 次后的 余下匹配！
                    if (is_prev_n_ok) {
                        states_prev_rest.push(*this);

                        SSS_LOG_FUNC_CALL("in 'is_prev_n_ok' rest; " + sss::cast_string(times_tok->get_min()));
                        is_rest_ok = this->match_here(times_tok + 1,
                                                      tok_end,
                                                      str_beg,
                                                      str_end);

                        if (is_rest_ok) {
                            states_last_succ.push(*this); // record the last success one
                        }
                        states_prev_rest.pop(*this); // restore state ; no matter TRUE of FALSE.
                    }
                    else {
                        throw false;
                    }

                    // NOTE 非贪婪模式的话，如果前N次已经有成功的匹配，那么后面的就不用进行了；
                    // 也就是说，只有贪婪模式，或者前面N词都没有成功的匹配，则进行后面的匹配
                    if (times_tok->is_greed() || !is_rest_ok) {
                        // 测试 后面的 m - n 次
                        int range_max =
                            times_tok->get_max() == -1 ?
                            std::numeric_limits<int32_t>::max() :
                            times_tok->get_max() - times_tok->get_min();

                        // 从当前，划分为 : range 和 rest 两部分；
                        // 先尝试匹配 range + rest；
                        // 如果range 匹配成功，则记录此处成功位置；并"尝试"匹配 rest
                        //        如果 rest 匹配成功，则记录一次；并将状态，退回到range成功之后；
                        // 如果 range 匹配失败；则退出循环――也不必记录状态――这由 match_here 行为决定；
                        //
                        // 最后，检查 range{n,m} + rest 都成功的匹配，是否存在？

                        for (int i = 0; i < range_max; ++i) {
                            simpleregex::states_t states_cur;
                            states_cur.push(*this);

                            SSS_LOG_DEBUG("range_max at (i = %d) start\n", i);
                            //SSS_LOG_EXPRESSION(sss::log::log_DEBUG,
                            //                   sss::utlstr::sample_string(str_beg_store, str_end));

                            std::ostringstream oss;
                            oss << "range_max at (i = " << i << ") start";
                            SSS_LOG_FUNC_CALL(oss.str());
                            bool is_branch_ok =
                                this->match_here(range_beg,
                                                 range_end,
                                                 str_beg,
                                                 str_end);

                            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, is_branch_ok);

                            if (!is_branch_ok)
                            {
                                break;
                            }

                            std::advance(str_beg, this->consumed_cnt - states_cur.back().consumed_cnt);

                            states_prev_rest.push(*this); // record the last success one

                            SSS_LOG_FUNC_CALL("after i times match. check the rest.");
                            bool is_rest_ok = this->match_here(times_tok + 1,
                                                               tok_end,
                                                               str_beg,
                                                               str_end);

                            if (is_rest_ok) {
                                states_last_succ.push(*this);

                                states_prev_rest.pop(*this); // restore state ; no matter TRUE of FALSE.
                                if (!times_tok->is_greed()) {
                                    break;
                                }
                            }

                            SSS_LOG_DEBUG("range_max at (i = %d) end\n", i);
                        }
                    }

                    if (!states_last_succ.empty()) {
                        states_last_succ.pop(*this);
                        ret = true;
                    }

                    throw ret;
                }

                // 如果是"左锚点"
                if (tok_beg->get_type() == ANCHOR_L_MATCH) { // {{{2
                    SSS_LOG_DEBUG("left_anchors[anchor_id=%d, str_ini_pos=%d]\n",
                                  tok_beg->anchor_id,
                                  std::distance(str_beg_bak, str_beg));

                    // TODO 记录出入位置
                    const_iterator range_beg = tok_beg + 1;
                    SSS_LOG_FUNC_CALL("tok_beg->is_type(ANCHOR_L_MATCH)");
                    const_iterator range_end = this->get_anchor_l2r(tok_beg);

                    assert(range_end != this->end());

                    // TODO 循环控制！
                    SSS_LOG_FUNC_CALL("tok_beg->is_type(ANCHOR_L_MATCH)");
                    const std::vector<int>& branches = this->get_branches(tok_beg);

                    std::string::const_iterator str_beg_store = str_beg;

                    int consumed;
                    SSS_LOG_FUNC_CALL("tok_beg->is_type(ANCHOR_L_MATCH)");
                    ret = this->match_branches(range_beg,
                                               range_end,
                                               branches,
                                               str_beg_store,
                                               str_end,
                                               &consumed);

                    if (ret) {
                        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, range_end->anchor_id);
                        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, tok_beg->anchor_id);
                        right_anchors[range_end->anchor_id] =
                            std::distance(this->str_beg_bak, str_beg_store);

                        left_anchors[tok_beg->anchor_id] =
                            std::distance(this->str_beg_bak, str_beg_store) - consumed;

                        SSS_LOG_DEBUG("right_anchors[%d, %d]\n",
                                      range_end->anchor_id,
                                      std::distance(this->str_beg_bak, str_beg_store));

                        SSS_LOG_DEBUG("==== in ===\n");
                        throw match_here(range_end + 1,
                                          tok_end,
                                          str_beg_store,
                                          str_end);
                        SSS_LOG_DEBUG("==== out ===\n");
                    }
                    throw false;
                }

                if (std::distance(tok_beg, tok_end) == 1 // {{{2
                    && tok_beg->get_type() == END_MATCH)
                {
                    SSS_LOG_FUNC_CALL("end with $");
                    throw str_beg == str_end;
                }

                // NOTE WORD_L_MATCH, WORD_R_MATCH
                if (tok_beg->get_type() == WORD_L_MATCH) { // {{{2
                    SSS_LOG_FUNC_CALL("tok_beg->get_type() == WORD_L_MATCH");
                    throw match_word_left(tok_beg,
                                          tok_end,
                                          str_beg,
                                          str_end);

                }
                // NOTE
                // WORD_R_MATCH 经常出现在正则表达式结尾；而此时，待匹配字符串，也
                // 常常恰好走到结尾
                if (tok_beg->get_type() == WORD_R_MATCH) { // {{{2
                    SSS_LOG_FUNC_CALL("tok_beg->get_type() == WORD_R_MATCH");
                    throw match_word_right(tok_beg,
                                            tok_end,
                                            str_beg,
                                            str_end);
                }

                // NOTE 引用匹配
                if (tok_beg->get_type() == REFER_MATCH) { // {{{2
                    SSS_LOG_FUNC_CALL("tok_beg->get_type() == REFER_MATCH");
                    throw match_refer(tok_beg,
                                       tok_end,
                                       str_beg,
                                       str_end);
                }

                if (str_beg != str_end) { // {{{2
                    // 还有未匹配的字符；并且当前字符匹配成功
                    if (tok_beg->match(*str_beg)) {
                        SSS_LOG_FUNC_CALL("str_beg != str_end && tok_beg->match(*str_beg)");
                        ret = match_here(tok_beg + 1,
                                         tok_end,
                                         str_beg + 1,
                                         str_end);
                        this->consumed_cnt += ret ? 1 : 0;
                        throw ret;
                    }
                }

                throw false;
            }
            catch (bool ret) { // {{{2
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, this->consumed_cnt - state_old.back().consumed_cnt);
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, ret);
                SSS_LOG_DEBUG("=======================>\n");

                if (!ret) {
                    state_old.pop(*this);
                }
                return ret;
            }
        }

        // bool simpleregex::match_word_left(tok_beg, tok_end, str_beg, str_end) {{{1
        // prev is null or !isalnum(prev)
        //  and
        // isalnum(next)
        bool simpleregex::match_word_left(simpleregex::const_iterator tok_beg,
                                          simpleregex::const_iterator tok_end,
                                          std::string::const_iterator str_beg,
                                          std::string::const_iterator str_end)
        {
#ifdef SSS_REG_FUNC_TRACE
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
#endif
            if ((str_beg_bak == str_beg || ! isdigitword(*(str_beg - 1)))
                &&
                isdigitword(*str_beg))
            {
                return this->match_here(tok_beg + 1, tok_end, str_beg, str_end);
            }
            return false;
        }

        //bool simpleregex::match_word_right(tok_beg, tok_end, str_beg, str_end) {{{1
        // isalnum(prev)
        //   and
        // next is last or !isalnum(next)
        bool simpleregex::match_word_right(simpleregex::const_iterator tok_beg,
                                           simpleregex::const_iterator tok_end,
                                           std::string::const_iterator str_beg,
                                           std::string::const_iterator str_end)
        {
#ifdef SSS_REG_FUNC_TRACE
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
#endif
            SSS_LOG_DEBUG("(%d, ..., %c, %c)\n",
                          std::distance(tok_beg_bak, tok_beg),
                          (str_beg==str_end) ? '\0' : *str_beg,
                          '\0');
            bool ret = false;
            if ((str_beg_bak != str_beg && isdigitword(*(str_beg - 1)))
                &&
                (str_beg == str_end || ! isdigitword(*str_beg) ))
            {
                ret = this->match_here(tok_beg + 1, tok_end, str_beg, str_end);
            }

            SSS_LOG_DEBUG("ret = %d\n", ret);

            return ret;
        }

        //bool simpleregex::match_refer(tok_beg, tok_end, str_beg, str_end) {{{1
        // sss::is_begin_with( std::string(str_beg, str_end), pref_refer_substring )
        bool simpleregex::match_refer(simpleregex::const_iterator tok_beg,
                                      simpleregex::const_iterator tok_end,
                                      std::string::const_iterator str_beg,
                                      std::string::const_iterator str_end)
        {
#ifdef SSS_REG_FUNC_TRACE
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
#endif
            SSS_LOG_DEBUG("(%d, ..., %c, %c)\n",
                          std::distance(tok_beg_bak, tok_beg),
                          (str_beg==str_end) ? '\0' : *str_beg,
                          '\0');

            size_t refer_id = tok_beg->anchor_id;
            if (this->left_anchors.size() < refer_id) {
                throw std::invalid_argument("invalid refer_id.");
            }
            std::string refer_string = this->get_submatch(refer_id);

            bool ret = false;

            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::string(str_beg, str_end));
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, refer_string);

            // FIXME 这里应该还需要考虑是否大小写敏感
            if (sss::is_begin_with(str_beg, str_end, refer_string.begin(), refer_string.end())) {
                ret = this->match_here(tok_beg + 1,
                                       tok_end,
                                       str_beg + this->get_submatch_consumed(refer_id),
                                       str_end);

                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, ret);
                if (ret) {
                    this->consumed_cnt += this->get_submatch_consumed(refer_id);
                }
            }
            return ret;
        }

        const std::vector<int>& simpleregex::get_branches(simpleregex::const_iterator tok_l_anchor)
        {
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::distance(tok_beg_bak, tok_l_anchor));
            if (tok_l_anchor == this->end()) {
                return left_anchor2branchs[-1];
            }
            else {
                return left_anchor2branchs[std::distance(tok_beg_bak, tok_l_anchor)];
            }
        }

        // simpleregex::const_iterator get_times_token(simpleregex::const_iterator tok_here) // {{{1
        simpleregex::const_iterator simpleregex::get_times_token(simpleregex::const_iterator tok_here)
        {
#ifdef SSS_REG_FUNC_TRACE
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
#endif
            int offset = std::distance(tok_beg_bak, tok_here);
            if (this->timespos.find(offset) != timespos.end()) {
                const_iterator times_tok = tok_beg_bak;
                std::advance(times_tok, timespos[offset]);
                return times_tok;
            }
            else {
                return this->match_tokens.end();
            }
        }

        // int simpleregex::consume_times_token(regs_here, regs_end, & p_greed, & t_min, & t_max) {{{1
        // NOTE 与以前处理 BDE 类似，我需要若干函数，能进行字符串解析：预看下一个字符、几个字符，分支
        // 等操作的函数。
        // 见：d:\program\msys\extra\sss\include\sss\BDEAlias.cpp|291
        // 为了不用写 if 可以用抛出异常的方式：
        //
        // start_with_require(regs_here, regs_end, '{', "");
        int simpleregex::consume_times_token(std::string::const_iterator regs_here,
                                             std::string::const_iterator regs_end,
                                             bool & greed,
                                             int & t_min,
                                             int & t_max)
        {
#ifdef SSS_REG_FUNC_TRACE
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
#endif
            std::string::const_iterator regs_here_bak = regs_here;
            // 从 { 开始 到 } 结束；
            // {} 至少2个字符
            int ret = 0;

            // '{'
            if (*regs_here++ != '{') {
                return ret;
            }

            // bool greed
            greed = true;
            if (*regs_here == '-') {
                greed = false;
                regs_here++;
            }

            // times min default = 0
            t_min = 0;
            bool has_t_min = false;
            if (std::isdigit(*regs_here)) {
                has_t_min = true;
                t_min = 0;
                do  {
                    t_min += 10 * t_min + (*regs_here - '0');
                } while (regs_here + 1 != regs_end && std::isdigit(*++regs_here));
            }

            bool has_common = false;
            // common
            if (*regs_here == ',') {
                regs_here++;
                has_common = true;
            }

            // times max default = -1
            t_max = has_common || !has_t_min ? -1 : t_min;
            if (std::isdigit(*regs_here)) {
                t_max = 0;
                do  {
                    t_max *= 10;
                    t_max += (*regs_here - '0');
                } while (regs_here + 1 != regs_end && std::isdigit(*++regs_here));
            }

            // '}'
            if (*regs_here++ != '}') {
                return ret;
            }

            ret = std::distance(regs_here_bak, regs_here);

            // NOTE vim中，允许 {10,5}这种存在！
            if (t_max >= 0 && t_max < t_min) {
                std::swap(t_max, t_min);
            }

            return ret;
        }

        // TODO 重置状态值，以迎接下一次匹配；
        void simpleregex::reset() // {{{1
        {
            // TODO
        }

        void simpleregex::record_times() // {{{1
        {
#ifdef SSS_REG_FUNC_TRACE
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
#endif
            if (match_tokens.size() < 2) {
                return;
            }
            switch (match_tokens.back().get_type())
            {
            case PLUS_MATCH:
            case STAR_MATCH:
            case TIMES_MATCH:
            case QUESTION_MATCH:
                if (match_tokens[match_tokens.size() - 2].get_type() == ANCHOR_R_MATCH) {
                    if (anchorpairs_rev.find(match_tokens.size() - 2) == anchorpairs_rev.end()) {
                        throw std::invalid_argument("anchorpairs_rev.not find("
                                                    + sss::cast_string(match_tokens.size() - 2)
                                                    + ")");
                    }
                    timespos[anchorpairs_rev[match_tokens.size() - 2]] = match_tokens.size() - 1;
                }
                else {
                    timespos[match_tokens.size() - 2] = match_tokens.size() - 1;
                }
                break;

            default:
                break;
            }
        }

        simpleregex::const_iterator simpleregex::get_anchor_l2r(simpleregex::const_iterator anchor_l) // {{{1
        {
#ifdef SSS_REG_FUNC_TRACE
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
#endif
            int offset = std::distance(this->begin(), anchor_l);
            const_iterator anchor_r = this->end();
            if (this->anchorpairs.find(offset) != this->anchorpairs.end()) {
                anchor_r = this->begin();
                std::advance(anchor_r, this->anchorpairs[offset]);
            }
            return anchor_r;
        }

        //simpleregex::const_iterator simpleregex::get_anchor_r2l(simpleregex::const_iterator anchor_r) // {{{1
        //{
        //    int offset = std::distance(this->begin(), anchor_r);
        //    const_iterator anchor_l = this->end();
        //    if (this->anchorpairs_rev.find(offset) != this->anchorpairs_rev.end()) {
        //        anchor_l = this->begin();
        //        std::advance(anchor_l, this->anchorpairs_rev[offset]);
        //    }
        //    return anchor_l;
        //}

        // bool simpleregex::match_branches(branch_beg, branch_end,  branches, str_beg, str_end); // {{{1

        // 完成分支循环的判断：
        bool simpleregex::match_branches(const_iterator branch_beg,
                                         const_iterator branch_end,
                                         const std::vector<int>&  branches,
                                         std::string::const_iterator& str_beg,
                                         std::string::const_iterator str_end,
                                         int * p_consumed)
        {
#ifdef SSS_REG_FUNC_TRACE
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
#endif
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::distance(this->tok_beg_bak, branch_beg));
            for (const_iterator it = branch_beg; it != branch_end; ++it) {
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, *it);
            }

            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, std::distance(this->str_beg_bak, str_beg));
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, sss::utlstr::sample_string(str_beg, str_end));

            if (branch_beg == branch_end) {
                if (p_consumed) {
                    *p_consumed = 0;
                }
                return true;
            }

            simpleregex::states_t states_prev;

            const_iterator beg = branch_beg;
            const_iterator end = this->end();
            std::vector<int>::const_iterator b_it = branches.begin();

            bool ret = false;

            do {
                states_prev.push(*this);

                if (b_it != branches.end()) {
                    beg = end == this->end() ? beg : end + 1;
                    end = this->begin() + *b_it;
                }
                else {
                    beg = end == this->end() ? beg : end + 1;
                    end = branch_end;
                }

                std::ostringstream oss;
                oss << "loop (" << std::distance(branches.begin(), b_it) << ")";
                SSS_LOG_FUNC_CALL(oss.str());

                ret = match_here(beg,
                                 end,
                                 str_beg,
                                 str_end);
                if (ret) {
                    str_beg += (this->consumed_cnt - states_prev.back().consumed_cnt);
                    if (p_consumed) {
                        *p_consumed = this->consumed_cnt - states_prev.back().consumed_cnt;
                        SSS_LOG_DEBUG("consumed = %d\n", *p_consumed);
                    }
                    break;
                }
                else {
                    states_prev.pop(*this);
                }
            }
            while (b_it++ != branches.end());

            return ret;
        }

        // NOTE 未被使用
        bool simpleregex::has_branch(const_iterator anchor_l)  // {{{1
        {
#ifdef SSS_REG_FUNC_TRACE
            SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
#endif
            int offset = std::distance(this->begin(), anchor_l);
            if (this->left_anchor2branchs.find(offset) != this->left_anchor2branchs.end() ) {
                return this->left_anchor2branchs[offset].size();
            }
            else {
                return false;
            }
        }


        //// bool simpleregex::match_rest_testing(tok_beg, tok_end, str_beg, str_end, int * p_consumed) // {{{1
        //bool simpleregex::match_rest_testing(simpleregex::const_iterator tok_beg,
        //                                     simpleregex::const_iterator tok_end,
        //                                     std::string::const_iterator str_beg,
        //                                     std::string::const_iterator str_end,
        //                                     int * p_consumed)
        //{
        //    int consumed_cnt_store = this->consumed_cnt;
        //    bool ret = this->match_here(tok_beg, tok_end, str_beg, str_end);

        //    std::vector<int> left_anchors_bak(this->left_anchors);
        //    std::vector<int> right_anchors_bak(this->right_anchors);

        //    if (ret && p_consumed) {
        //        *p_consumed = this->consumed_cnt - consumed_cnt_store;
        //    }

        //    std::swap(left_anchors_bak, this->left_anchors);
        //    std::swap(right_anchors_bak, this->right_anchors);
        //    std::swap(consumed_cnt_store, this->consumed_cnt);

        //    return ret;
        //}
    } // end-of namespace regex
}// end-of namespace sss

