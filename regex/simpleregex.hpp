#ifndef  __SIMPLEREGEX_HPP_1396661763__
#define  __SIMPLEREGEX_HPP_1396661763__

#include <algorithm>
#include <string>
#include <vector>

#include <map>
#include <list>

#include <iostream>

// NOTE
// 这是一个 simple regex 库；使用一个线性结构，以及附加的跳转map，来保存编译后
// 的正则表达式结构体；
//
// 为什么叫"simple"，是因为：
// 1. 只基于字符；
// 2. 用的是需要回溯的递归下降法；
// 3. 暂时未用专门的数据结构来保存匹配结果；即，匹配结果是保存在正则表达式结构
// 体中的；
//
// 这导致同一个正则表达式，不能应用于多线程的情况――只能一个match 动作地做完；

//----------------------------------------------------------------------

// TODO 2015-02-21
// 增加 支持 忽略大小写 比较；
//
// 首先，match的时候，是否大小写敏感，应该作为正则对象的一个属性存在――即，在
// match动作当中，不应该发生变化。
//
// 那么，这个信息，有两种方式：
// 1. 构造函数。
// 2. 提供额外的"方法"，以修改该属性。
//
// 当然，真正匹配的时候，还应该在内部自动完成判断（相应的转换）。
//
// 当然，还有一种模式是，创建另外的一种正则对象――忽略大小写；同时，两种对象可
// 以相互转换。
//
// "可能的问题："
//      [^] 与 [] 集合。

// TODO 2014-04-21
// 增加
// \i 标识符；
// 并且，用户可以设定标识符的字符集；比如C语言标识符字符集是
//
// [a-zA-Z][a-zA-Z0-9_]\+
//
//----------------------------
//
// 简单正则匹配
// 只提供简单的正则匹配判断；
// 没有分支，只有串联；
// '^','$',"[abc]",'.','*','+'
// 转义字符是 "\\"
//
// 递归；非贪婪
// 复杂度 0(n^2)
//
//1. TODO 支持分支
// 如果要支持 分支(|)，就需要支持\( \)；
// 形如：
// \(int|double|float\)
//
// 就数据结构来说，相当于有三条路，可以通往目的地；
//
// 其长度，分别是 3、6、5；路径分别是：
// 'i'->'n'->'t'
// 'd'->'o'->'u'->'b'->'l'->'e'
// 'f'->'l'->'o'->'a'->'t'
//
//! 如何实现？
// 最简单的方法，还是递归；即，把分支的，作为一个"子正则表达式"来处理。
//
// 2. TODO 支持返回子串
// 这个是相当重要的特性――因为前面的分支，可以通过拆解的方式解决；
//
// 要支持返回匹配的字串的话，
//
// 3. NOTE 支持贪婪搜索
//
// 注意，如果 支持了 单词边界 \<,\> 的话，那么，是否支持 贪婪 搜索，就问题不大
// 了。
//
// 因为，我觉得，贪婪搜索，效率会比较受影响；因为，每次匹配，都尽量往多的匹配；
//
// 如果，遇到一个很长的行，而实际我们真正需要获取的，可能只是前面几十个字符，那
// 不是完蛋了？
//
//! 如何支持贪婪搜索
// TODO
//
// 认证来说，很多时候，边界并不不能完全用\<,\>来代替；如果不支持贪婪搜索的时候
// ，本正则表达式，在实际使用中，有时候需要用多个表达式，多次match，才能达到预
// 定的效果。
//
// 那么，应该如何支持贪婪搜索呢？
//
// 这与方向有关。当前的模式匹配，默认都是从最近点开始，往后搜索。
//
// 那么，要实现贪婪，就有两种策略：
//
// 一、沿用当前的检索方向，但是每次分支，都需要保存当前已经选择好的"路径"；这样
// ，才能尽量找出最长的匹配串。
//    ――把正则表达式，想象成N个分段匹配；后一段的路径选择，都取决于之前的选择
//    。
//
// 二、逆向匹配；直接从最远的位置，反向寻找，直到找到合适的项目。
//
//-------------------------------------
//! 如何支持单词边界的检索
//
// 其实单词边界的检索，其本身是有冗余信息的。因为，实际使用的时候，\<,\>这种边
// 界信息，一般是不会单独使用，而会混合一些字符：
// \<word\>
//
// 即，\<告诉regex解析器，它需要左边是标点符号、空白符或者行开头；
// 而其右侧，则是字母或者数字。
//
// 而接下来的 word 则实际告诉解析器，它只能匹配含有word子串的字符串。
//
// 另外，这种边界，更像是“锚点”――它并不允许匹配的"平移"。实际的匹配中，它需
// 要向前，或者向后"预知"（或者"记忆"）单词，以进行匹配。
//
// 而其他类型的标记，则往往只需要读取当前带匹配字符即可；
//
// 当然，特殊的还有 '^','$','*','+' 等。
//
// 比如用 "a+" 来匹配 "aaaa" ―― 它的任意子串，都匹配"a+"――，就好像发生了平
// 移一样。
//
// 实现起来也很简单――谨记"递归"，和"锚点"两个词即可。
//
// 前者告诉我们，只需要解决“一点”；后者，则提示如何设计数据结构。
//
//--------------------------------------------------------

namespace sss{
    namespace regex {
        class simpleregex
        {
        public:
            enum match_case_t
            {
                case_sensitive = 0,     // 默认值
                case_insensitive = 1
            };

            enum match_t_base_t{
                MT_BASE_ZERO    = 0x0100,
                MT_BASE_ANY     = 0x0200,
                MT_BASE_MULTY   = 0x0400,
                MT_BASE_HOLDER  = 0x0800,
                MT_BASE_SINGLE  = 0x1000,
            };

            enum match_type_t
            {
//! 非法、默认类型
                NULL_MATCH      = 0,                 // 0 | MT_BASE_ZERO

//! 消耗字数不定
                REFER_MATCH     = 0 | MT_BASE_ANY,   // "\\[1-9][0-9]"

//! 多次匹配用
                TIMES_MATCH     = 0 | MT_BASE_MULTY, // "\\{min,max}"
                QUESTION_MATCH  = 1 | MT_BASE_MULTY, // "\\?" 0 or 1 times
                STAR_MATCH      = 2 | MT_BASE_MULTY, // "*" any; inlude 0
                PLUS_MATCH      = 3 | MT_BASE_MULTY, // "+" many; at least 1

//! 不消耗字符的关键字符：

                START_MATCH     = 0 | MT_BASE_HOLDER, // '^'
                END_MATCH       = 1 | MT_BASE_HOLDER, // '$'
                WORD_L_MATCH    = 2 | MT_BASE_HOLDER, // "\<"
                WORD_R_MATCH    = 3 | MT_BASE_HOLDER, // "\>"
                BRANCH_MATCH    = 4 | MT_BASE_HOLDER, // "\\|" 分支
                ANCHOR_L_MATCH  = 5 | MT_BASE_HOLDER, // "\(" 左锚点
                ANCHOR_R_MATCH  = 6 | MT_BASE_HOLDER, // "\)" 右锚点

//! 消耗单个字符：
                CHAR_MATCH      = 0 | MT_BASE_SINGLE, // 某单个字符
                DIGIT_MATCH     = 1 | MT_BASE_SINGLE, // "\d"
                ALPHA_MATCH     = 2 | MT_BASE_SINGLE, // "\c" -- means charachter
                ALNUM_MATCH     = 3 | MT_BASE_SINGLE, // "\w" | "\d"
                SPACE_MATCH     = 4 | MT_BASE_SINGLE, // "\s"
                NOTSPACE_MATCH  = 5 | MT_BASE_SINGLE, // "\S"
                SET_MATCH       = 6 | MT_BASE_SINGLE, // [abc]  包含'-'，请使用"\\"转义
                SET_VERSE_MATCH = 7 | MT_BASE_SINGLE, // [^abc]
                ANY_MATCH       = 8 | MT_BASE_SINGLE, // .

                // 关于 REFER_MATCH 内引用
                // NOTE
                // 1. 不能出现 "\\0"！
                // 参考 C语言结构体，自包含的情况：
                // struct A {
                //    A a;
                // };
                //
                // 想这种循环引用的情况，C语言是会报错的；因为这没法确定结构体大小；
                // 同样的，"\\0" 代表整个正则串，所匹配到的字符；而"\\0"如果要
                // 出现在正则串当中的时候，就意味着匹配动作还未完成，又如何去界
                // 定整个匹配到的字符串边界呢？
                // 2. 引用出现的位置，必然在该序列所框定的范围之外；
                // 理由，与"\\0"的讨论类似；也是防止循环引用。
                //
                //! 如何实现？
                // 要知道 match_token 类型，是不知道整个正则字符串的情况的！
                // 就是说，match_token::match 动作，没法完成！这意味着，我需
                // 要修改 match_token 的构造函数――使其知道所属的
                // simpleregex 对象地址！
                // 而且，我的实现有一个很大的问题，就是上层的逻辑，是由
                // simpleregex 对象，通过其递归下降的几个方法，来共同完成的
                // 。
                // 1. 是最终的比较，又 match_token::match 完成；并且比较的时
                // 候，没有涉及到被比较字符串的上下文，而只是一个单独的字符
                // 。
                // 2. 上述 match_token::match 的比较，没有涉及到消耗字符串的
                // 信息。
                //
                // 3. 是否允许 \2, \1 这样的引用？即，后确定的引用，先在前面
                // 显示。――这感觉有点像逻辑上，自己证明自己的感觉。
                // vim-error-msg-E65: Illegle back reference
                // 看样子，这种引用，不被支持。
                // 即，引用出现的位置，必须在 \(\) 成对的pair之后！
                // 不是说 "自引用" 就无法完成。此时，需要将该功能的实现（匹
                // 配）部分，放入 simpleregex 的一个新方法，比如 mach_selrefer 里面。
                //

                // TODO "\\zs" | "\\ze" 范围锚点
                // -- NOTE '\zs', '\ze' 这两个，虽然与前匹配，后缀匹配功能重
                // -- 叠了。但是，它不能提供与 前否定，后否定 相匹配的功能
                //
                // TODO "\\?" query mark 表示匹配一次或0次；
                // TODO 区分贪婪与非贪婪 ―― 利用构造函数？bool greed ？
                // \{n,m}
                // \x,\X
                // \D
                //
                // vim中，为了匹配标识符，
                // 他使用了：\<\h\w*
                // 其中
                //    \h 表示 [a-zA-Z_]
                //    \w 表示 [a-zA-Z_0-9]
            };

            //! 关于锚点；
            // \0 表示整个字符串
            // \1 表示内部的一组锚点
            // \2 以此类推
            //
            // "\\0" 因为 simpleregex::match() 会记录匹配开始与结束的位置，所以，这不是问题；
            // "\\1" ... "\\n" 这些，则需要两个vector 或者 一个vector<pair<int> > 来保存；
            // 如上，每一次match，就生成一次匹配；
            // 另外，如果遇到循环的情况，则只记录最后一次匹配！
            //

            struct match_token
            {
            public:
                // 一般匹配
                match_token(match_type_t t,
                            const std::string& str,
                            match_case_t m_case = case_sensitive);

                // 锚点
                match_token(match_type_t t,
                            const std::string& str,
                            int pos,
                            match_case_t m_case = case_sensitive);

            public:
                bool match(char ch) const;

                bool is_case_sensitive() const;
                bool enable_case_sensitive();
                bool enable_case_insensitive();

                bool is_set_type() const;

                bool next(int & times) const;           // not implied

                bool is_in_range(int times) const;      // not implied

                bool init();

                inline match_type_t get_type() const
                {
                    return this->type;
                }

                const char * get_type_name() const;

                inline bool is_type(match_type_t t) const
                {
                    return this->type == t;
                }

                inline bool is_type_base(match_t_base_t t) const
                {
                    return t == MT_BASE_ZERO ? !this->type : this->type & t;
                }

                inline bool is_greed() const
                {
                    return this->greed;
                }

                inline int get_min() const
                {
                    return this->t_min;
                }

                inline int get_max() const
                {
                    return this->t_max;
                }

            public:
                void print(std::ostream& o) const;

            protected:
                void sort_with_case();

            public:
                match_type_t  type;
                match_case_t  match_case;
                int         anchor_id;  // 锚点ID
                std::string data;
                bool greed;
                int  t_min;
                int  t_max;
            };

            typedef std::vector<match_token> match_tokens_t;
            typedef match_tokens_t::const_iterator const_iterator;
            typedef match_tokens_t::iterator iterator;

    public:
            inline const_iterator end() const
            {
                return this->match_tokens.end();
            }

            //inline iterator end() const
            //{
            //    return this->match_tokens.end();
            //}

            inline const_iterator begin() const
            {
                return this->match_tokens.begin();
            }

            //inline iterator begin() const
            //{
            //    return this->match_tokens.begin();
            //}

        public:
            explicit simpleregex(const std::string& reg_str,
                                 match_case_t match_case = simpleregex::case_sensitive);
            ~simpleregex();

        public:
            //! sugar
            //> 返回是否匹配
            bool match(const std::string& str);

            //> 返回是否匹配 并返回匹配的字串  v1
            bool match(const std::string& str,
                       std::string& matched_sub);

            //> 返回是否匹配 并返回匹配的字串，以及开始匹配的位置 v1
            bool match(const std::string& str,
                       std::string& matched_sub,
                       int& begin_pos);

            //> 返回是否匹配 并返回匹配的字串  v2
            bool match(std::string::const_iterator,
                       std::string::const_iterator,
                       std::string& matched_sub);

            //> 返回是否匹配 并返回匹配的字串，以及开始匹配的位置 v2
            bool match(std::string::const_iterator,
                       std::string::const_iterator,
                       std::string& matched_sub,
                       int& begin_pos);

            //> 返回是否匹配 并返回匹配的子串起始位置
            bool match(std::string::const_iterator,
                       std::string::const_iterator,
                       std::string::const_iterator&,
                       std::string::const_iterator&);

            //! 基础函数
            //> 返回是否匹配 并在对象内部记录本次匹配的位置信息
            bool match(std::string::const_iterator,
                       std::string::const_iterator);

            // NOTE 替换替换的模式
            // 正则表达式替换，如果以vim为参考，分为两部分。
            // s/parttern/subs/ge
            // 首先，subs也是一个可以解析的串；
            // 其中的\\1，被理解为字串。
            // 然后，整个匹配的部分，被用上面的替换掉；
            //
            // 返回替换后的字符串
            std::string substitute(const std::string&,
                                   const std::string&);

            std::string substitute(std::string::const_iterator,
                                   std::string::const_iterator,
                                   std::string::const_iterator,
                                   std::string::const_iterator);

            //! sugar
            //> "将多次匹配的信息，依次写入传入的容器"――利用后插迭代器
            //! 使用范例：
            // std::vector<std::string> submatches;
            // sss::regex::simpleregex("\\[sql\\d+\\]")
            //    .all_sub_matches("[sql1] [sql23] [sql13] [sql0]",
            //                      std::back_inserter(submatches));
            //
            //! 输出：
            // [sql1]
            // [sql23]
            // [sql13]
            // [sql0]
            //
            template <typename Out_iterator>
            int  all_sub_matches(const std::string& str,
                                 Out_iterator bit)
            {
                try {
                    std::string::const_iterator str_beg = str.begin();
                    std::string::const_iterator str_end = str.end();
                    int cnt = 0;
                    while (str_beg != str_end && match(str_beg, str_end)) {
                        std::advance(str_beg, start_pos);
                        bit = std::string(str_beg, str_beg + consumed_cnt);
                        std::advance(str_beg, consumed_cnt);
                        //++bit;
                        ++cnt;
                    }
                    return cnt;
                }
                catch (std::exception & e) {
                    std::cout << __FILE__ << ", " << __LINE__ << std::endl;
                    throw;
                }
            }

            int sub_match_count() const;

            void print_match_info(std::ostream& o);

            void reset(); // 重置状态值，以迎接下一次匹配；


        protected:
            const_iterator get_times_token(const_iterator tok_here);

            const_iterator get_anchor_l2r(const_iterator anchor_l);
            //const_iterator get_anchor_r2l(const_iterator anchor_r);

            const std::vector<int>& get_branches(const_iterator tok_l_anchor);

            bool has_branch(const_iterator anchor_l);

        public:
            static int consume_times_token(std::string::const_iterator regs_here,
                                           std::string::const_iterator regs_end,
                                           bool & p_greed,
                                           int & t_min,
                                           int & t_max);

        protected:
            //! 内部实现脚本：

            //  处理匹配结束、匹配当前
            bool match_here(const_iterator,
                            const_iterator,
                            std::string::const_iterator,
                            std::string::const_iterator);

            // 理论上，应用的匹配，只消耗 正则表达式串 的一个元素；
            // 而消耗的被匹配字符串的，则数量不定；
            bool match_refer(const_iterator,
                             const_iterator,
                             std::string::const_iterator,
                             std::string::const_iterator);

            bool match_word_left(const_iterator,
                                 const_iterator,
                                 std::string::const_iterator,
                                 std::string::const_iterator);

            bool match_word_right(const_iterator,
                                  const_iterator,
                                  std::string::const_iterator,
                                  std::string::const_iterator);

            bool match_branches(const_iterator branch_beg,
                                const_iterator branch_end,
                                const std::vector<int>&  branches,
                                std::string::const_iterator& str_beg,
                                std::string::const_iterator str_end,
                                int * p_consumed = 0);

            //! 关于rest匹配
            // 这其实是关于回溯的；
            // 为了减少回溯次数；当进入 rest匹配模式的时候，在遇到次数匹配的时候，
            // 只用做前面的n次匹配，而不用做后门的(n,m]次匹配；并且是非贪婪模式！
            //bool match_rest_testing(const_iterator,
            //                        const_iterator,
            //                        std::string::const_iterator,
            //                        std::string::const_iterator,
            //                        int * p_consumed = 0);

        public:
            std::string get_submatch(int id) const;
            int         get_submatch_count() const;
            int         get_submatch_start(int id) const;
            int         get_submatch_end(int id) const;
            int         get_submatch_consumed(int id) const;

        public:
            bool        is_case_sensitive() const;
            bool        enable_case_sensitive();
            bool        enable_case_insensitive();

        protected:
            void        sort_with_case();
            void        record_times();

        private:
            match_tokens_t match_tokens;

            std::string reg_string;
            std::string to_match;

        public:
            int         start_pos;              // 匹配状态！
            int         consumed_cnt;           // 匹配状态！
            match_case_t match_case;

            std::string::const_iterator    str_beg_bak; // 在match函数使用过程中不变
            const_iterator tok_beg_bak;         // 初始化后，不变

            std::vector<int> left_anchors;      // 匹配状态！
            std::vector<int> right_anchors;     // 匹配状态！

            // 记录branch标签下标，对应的最近右锚点标签下标；
            // 即，分支匹配的结束位置
            std::map<int, int>    branch2right_anchor; // 初始化后，不变

            std::map<int, int> anchorpairs;     // 初始化后，不变
            std::map<int, int> anchorpairs_rev; // 初始化后，不变

            std::map<int, int> timespos;        // 初始化后，不变


            // 左锚点对应的分支下标数组
            // 即，分支的开始位置
            // 如果某对锚点之间，没有直接分支，怎么办？
            // 应该是一个空的数组！
            std::map<int, std::vector<int> > left_anchor2branchs;// 初始化后，不变

            bool is_match_here_called;          // 控制初次 进入 match_here 函数的行为的
            bool least_match;                   // 最少匹配模式

        public:
            struct state_t
            {
                state_t(int pos,
                        int cnt,
                        const std::vector<int>& left,
                        const std::vector<int>& right)
                    : start_pos(pos), consumed_cnt(cnt), left_anchors(left), right_anchors(right)
                {
                }
                ~state_t()
                {
                }

                int         start_pos;              // 匹配状态！
                int         consumed_cnt;           // 匹配状态！
                std::vector<int> left_anchors;      // 匹配状态！
                std::vector<int> right_anchors;     // 匹配状态！
            };

            class states_t : private std::list<state_t>
        {
        public:
            using std::list<state_t>::empty;
            using std::list<state_t>::back;

        public:
            inline void push(const simpleregex& reg)
            {

                state_t tmp(reg.start_pos,
                            reg.consumed_cnt,
                            reg.left_anchors,
                            reg.right_anchors);
                if (this->empty()) {
                    this->std::list<state_t>::push_back(tmp);
                }
                else {
                    this->back().start_pos     = tmp.start_pos;
                    this->back().consumed_cnt  = tmp.consumed_cnt;
                    this->back().left_anchors  = tmp.left_anchors;
                    this->back().right_anchors = tmp.right_anchors;
                }
            }

            inline void swap_top(simpleregex& reg)
            {
                if (!this->empty()) {
                    state_t& state = this->back();
                    std::swap(reg.start_pos,     state.start_pos);
                    std::swap(reg.consumed_cnt,  state.consumed_cnt);
                    std::swap(reg.left_anchors,  state.left_anchors);
                    std::swap(reg.right_anchors, state.right_anchors);
                }
            }

            inline void pop(simpleregex& reg)
            {
                this->swap_top(reg);
                if (!this->empty()) {
                    this->pop_back();
                }
            }
        };

        };

    inline std::ostream& operator <<(std::ostream& o, const sss::regex::simpleregex::match_token& tok)
    {
        tok.print(o);
        return o;
    }


} // end-of namespace regex
}// end-of namespace sss


#endif  /* __SIMPLEREGEX_HPP_1396661763__ */
