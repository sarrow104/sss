#ifndef  __SIMPLEREGEX_HPP_1396661763__
#define  __SIMPLEREGEX_HPP_1396661763__

#include <algorithm>
#include <string>
#include <vector>

#include <map>
#include <list>

#include <iostream>

// NOTE
// ����һ�� simple regex �⣻ʹ��һ�����Խṹ���Լ����ӵ���תmap������������
// ��������ʽ�ṹ�壻
//
// Ϊʲô��"simple"������Ϊ��
// 1. ֻ�����ַ���
// 2. �õ�����Ҫ���ݵĵݹ��½�����
// 3. ��ʱδ��ר�ŵ����ݽṹ������ƥ����������ƥ�����Ǳ�����������ʽ�ṹ
// ���еģ�
//
// �⵼��ͬһ��������ʽ������Ӧ���ڶ��̵߳��������ֻ��һ��match ���������ꣻ

//----------------------------------------------------------------------

// TODO 2015-02-21
// ���� ֧�� ���Դ�Сд �Ƚϣ�
//
// ���ȣ�match��ʱ���Ƿ��Сд���У�Ӧ����Ϊ��������һ�����Դ��ڡ���������
// match�������У���Ӧ�÷����仯��
//
// ��ô�������Ϣ�������ַ�ʽ��
// 1. ���캯����
// 2. �ṩ�����"����"�����޸ĸ����ԡ�
//
// ��Ȼ������ƥ���ʱ�򣬻�Ӧ�����ڲ��Զ�����жϣ���Ӧ��ת������
//
// ��Ȼ������һ��ģʽ�ǣ����������һ��������󡪡����Դ�Сд��ͬʱ�����ֶ����
// ���໥ת����
//
// "���ܵ����⣺"
//      [^] �� [] ���ϡ�

// TODO 2014-04-21
// ����
// \i ��ʶ����
// ���ң��û������趨��ʶ�����ַ���������C���Ա�ʶ���ַ�����
//
// [a-zA-Z][a-zA-Z0-9_]\+
//
//----------------------------
//
// ������ƥ��
// ֻ�ṩ�򵥵�����ƥ���жϣ�
// û�з�֧��ֻ�д�����
// '^','$',"[abc]",'.','*','+'
// ת���ַ��� "\\"
//
// �ݹ飻��̰��
// ���Ӷ� 0(n^2)
//
//1. TODO ֧�ַ�֧
// ���Ҫ֧�� ��֧(|)������Ҫ֧��\( \)��
// ���磺
// \(int|double|float\)
//
// �����ݽṹ��˵���൱��������·������ͨ��Ŀ�ĵأ�
//
// �䳤�ȣ��ֱ��� 3��6��5��·���ֱ��ǣ�
// 'i'->'n'->'t'
// 'd'->'o'->'u'->'b'->'l'->'e'
// 'f'->'l'->'o'->'a'->'t'
//
//! ���ʵ�֣�
// ��򵥵ķ��������ǵݹ飻�����ѷ�֧�ģ���Ϊһ��"��������ʽ"������
//
// 2. TODO ֧�ַ����Ӵ�
// ������൱��Ҫ�����ԡ�����Ϊǰ��ķ�֧������ͨ�����ķ�ʽ�����
//
// Ҫ֧�ַ���ƥ����ִ��Ļ���
//
// 3. NOTE ֧��̰������
//
// ע�⣬��� ֧���� ���ʱ߽� \<,\> �Ļ�����ô���Ƿ�֧�� ̰�� �����������ⲻ��
// �ˡ�
//
// ��Ϊ���Ҿ��ã�̰��������Ч�ʻ�Ƚ���Ӱ�죻��Ϊ��ÿ��ƥ�䣬�����������ƥ�䣻
//
// ���������һ���ܳ����У���ʵ������������Ҫ��ȡ�ģ�����ֻ��ǰ�漸ʮ���ַ�����
// �����군�ˣ�
//
//! ���֧��̰������
// TODO
//
// ��֤��˵���ܶ�ʱ�򣬱߽粢��������ȫ��\<,\>�����棻�����֧��̰��������ʱ��
// ����������ʽ����ʵ��ʹ���У���ʱ����Ҫ�ö�����ʽ�����match�����ܴﵽԤ
// ����Ч����
//
// ��ô��Ӧ�����֧��̰�������أ�
//
// ���뷽���йء���ǰ��ģʽƥ�䣬Ĭ�϶��Ǵ�����㿪ʼ������������
//
// ��ô��Ҫʵ��̰�����������ֲ��ԣ�
//
// һ�����õ�ǰ�ļ������򣬵���ÿ�η�֧������Ҫ���浱ǰ�Ѿ�ѡ��õ�"·��"������
// �����ܾ����ҳ����ƥ�䴮��
//    ������������ʽ�������N���ֶ�ƥ�䣻��һ�ε�·��ѡ�񣬶�ȡ����֮ǰ��ѡ��
//    ��
//
// ��������ƥ�䣻ֱ�Ӵ���Զ��λ�ã�����Ѱ�ң�ֱ���ҵ����ʵ���Ŀ��
//
//-------------------------------------
//! ���֧�ֵ��ʱ߽�ļ���
//
// ��ʵ���ʱ߽�ļ������䱾������������Ϣ�ġ���Ϊ��ʵ��ʹ�õ�ʱ��\<,\>���ֱ�
// ����Ϣ��һ���ǲ��ᵥ��ʹ�ã�������һЩ�ַ���
// \<word\>
//
// ����\<����regex������������Ҫ����Ǳ����š��հ׷������п�ͷ��
// �����Ҳ࣬������ĸ�������֡�
//
// ���������� word ��ʵ�ʸ��߽���������ֻ��ƥ�京��word�Ӵ����ַ�����
//
// ���⣬���ֱ߽磬�����ǡ�ê�㡱��������������ƥ���"ƽ��"��ʵ�ʵ�ƥ���У�����
// Ҫ��ǰ���������"Ԥ֪"������"����"�����ʣ��Խ���ƥ�䡣
//
// ���������͵ı�ǣ�������ֻ��Ҫ��ȡ��ǰ��ƥ���ַ����ɣ�
//
// ��Ȼ������Ļ��� '^','$','*','+' �ȡ�
//
// ������ "a+" ��ƥ�� "aaaa" ���� ���������Ӵ�����ƥ��"a+"�������ͺ�������ƽ
// ��һ����
//
// ʵ������Ҳ�ܼ򵥡�������"�ݹ�"����"ê��"�����ʼ��ɡ�
//
// ǰ�߸������ǣ�ֻ��Ҫ�����һ�㡱�����ߣ�����ʾ���������ݽṹ��
//
//--------------------------------------------------------

namespace sss{
    namespace regex {
        class simpleregex
        {
        public:
            enum match_case_t
            {
                case_sensitive = 0,     // Ĭ��ֵ
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
//! �Ƿ���Ĭ������
                NULL_MATCH      = 0,                 // 0 | MT_BASE_ZERO

//! ������������
                REFER_MATCH     = 0 | MT_BASE_ANY,   // "\\[1-9][0-9]"

//! ���ƥ����
                TIMES_MATCH     = 0 | MT_BASE_MULTY, // "\\{min,max}"
                QUESTION_MATCH  = 1 | MT_BASE_MULTY, // "\\?" 0 or 1 times
                STAR_MATCH      = 2 | MT_BASE_MULTY, // "*" any; inlude 0
                PLUS_MATCH      = 3 | MT_BASE_MULTY, // "+" many; at least 1

//! �������ַ��Ĺؼ��ַ���

                START_MATCH     = 0 | MT_BASE_HOLDER, // '^'
                END_MATCH       = 1 | MT_BASE_HOLDER, // '$'
                WORD_L_MATCH    = 2 | MT_BASE_HOLDER, // "\<"
                WORD_R_MATCH    = 3 | MT_BASE_HOLDER, // "\>"
                BRANCH_MATCH    = 4 | MT_BASE_HOLDER, // "\\|" ��֧
                ANCHOR_L_MATCH  = 5 | MT_BASE_HOLDER, // "\(" ��ê��
                ANCHOR_R_MATCH  = 6 | MT_BASE_HOLDER, // "\)" ��ê��

//! ���ĵ����ַ���
                CHAR_MATCH      = 0 | MT_BASE_SINGLE, // ĳ�����ַ�
                DIGIT_MATCH     = 1 | MT_BASE_SINGLE, // "\d"
                ALPHA_MATCH     = 2 | MT_BASE_SINGLE, // "\c" -- means charachter
                ALNUM_MATCH     = 3 | MT_BASE_SINGLE, // "\w" | "\d"
                SPACE_MATCH     = 4 | MT_BASE_SINGLE, // "\s"
                NOTSPACE_MATCH  = 5 | MT_BASE_SINGLE, // "\S"
                SET_MATCH       = 6 | MT_BASE_SINGLE, // [abc]  ����'-'����ʹ��"\\"ת��
                SET_VERSE_MATCH = 7 | MT_BASE_SINGLE, // [^abc]
                ANY_MATCH       = 8 | MT_BASE_SINGLE, // .

                // ���� REFER_MATCH ������
                // NOTE
                // 1. ���ܳ��� "\\0"��
                // �ο� C���Խṹ�壬�԰����������
                // struct A {
                //    A a;
                // };
                //
                // ������ѭ�����õ������C�����ǻᱨ��ģ���Ϊ��û��ȷ���ṹ���С��
                // ͬ���ģ�"\\0" �����������򴮣���ƥ�䵽���ַ�����"\\0"���Ҫ
                // ���������򴮵��е�ʱ�򣬾���ζ��ƥ�䶯����δ��ɣ������ȥ��
                // ������ƥ�䵽���ַ����߽��أ�
                // 2. ���ó��ֵ�λ�ã���Ȼ�ڸ��������򶨵ķ�Χ֮�⣻
                // ���ɣ���"\\0"���������ƣ�Ҳ�Ƿ�ֹѭ�����á�
                //
                //! ���ʵ�֣�
                // Ҫ֪�� match_token ���ͣ��ǲ�֪�����������ַ���������ģ�
                // ����˵��match_token::match ������û����ɣ�����ζ�ţ�����
                // Ҫ�޸� match_token �Ĺ��캯������ʹ��֪��������
                // simpleregex �����ַ��
                // ���ң��ҵ�ʵ����һ���ܴ�����⣬�����ϲ���߼�������
                // simpleregex ����ͨ����ݹ��½��ļ�������������ͬ��ɵ�
                // ��
                // 1. �����յıȽϣ��� match_token::match ��ɣ����ұȽϵ�ʱ
                // ��û���漰�����Ƚ��ַ����������ģ���ֻ��һ���������ַ�
                // ��
                // 2. ���� match_token::match �ıȽϣ�û���漰�������ַ�����
                // ��Ϣ��
                //
                // 3. �Ƿ����� \2, \1 ���������ã�������ȷ�������ã�����ǰ��
                // ��ʾ��������о��е����߼��ϣ��Լ�֤���Լ��ĸо���
                // vim-error-msg-E65: Illegle back reference
                // �����ӣ��������ã�����֧�֡�
                // �������ó��ֵ�λ�ã������� \(\) �ɶԵ�pair֮��
                // ����˵ "������" ���޷���ɡ���ʱ����Ҫ���ù��ܵ�ʵ�֣�ƥ
                // �䣩���֣����� simpleregex ��һ���·��������� mach_selrefer ���档
                //

                // TODO "\\zs" | "\\ze" ��Χê��
                // -- NOTE '\zs', '\ze' ����������Ȼ��ǰƥ�䣬��׺ƥ�书����
                // -- ���ˡ����ǣ��������ṩ�� ǰ�񶨣���� ��ƥ��Ĺ���
                //
                // TODO "\\?" query mark ��ʾƥ��һ�λ�0�Σ�
                // TODO ����̰�����̰�� ���� ���ù��캯����bool greed ��
                // \{n,m}
                // \x,\X
                // \D
                //
                // vim�У�Ϊ��ƥ���ʶ����
                // ��ʹ���ˣ�\<\h\w*
                // ����
                //    \h ��ʾ [a-zA-Z_]
                //    \w ��ʾ [a-zA-Z_0-9]
            };

            //! ����ê�㣻
            // \0 ��ʾ�����ַ���
            // \1 ��ʾ�ڲ���һ��ê��
            // \2 �Դ�����
            //
            // "\\0" ��Ϊ simpleregex::match() ���¼ƥ�俪ʼ�������λ�ã����ԣ��ⲻ�����⣻
            // "\\1" ... "\\n" ��Щ������Ҫ����vector ���� һ��vector<pair<int> > �����棻
            // ���ϣ�ÿһ��match��������һ��ƥ�䣻
            // ���⣬�������ѭ�����������ֻ��¼���һ��ƥ�䣡
            //

            struct match_token
            {
            public:
                // һ��ƥ��
                match_token(match_type_t t,
                            const std::string& str,
                            match_case_t m_case = case_sensitive);

                // ê��
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
                int         anchor_id;  // ê��ID
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
            //> �����Ƿ�ƥ��
            bool match(const std::string& str);

            //> �����Ƿ�ƥ�� ������ƥ����ִ�  v1
            bool match(const std::string& str,
                       std::string& matched_sub);

            //> �����Ƿ�ƥ�� ������ƥ����ִ����Լ���ʼƥ���λ�� v1
            bool match(const std::string& str,
                       std::string& matched_sub,
                       int& begin_pos);

            //> �����Ƿ�ƥ�� ������ƥ����ִ�  v2
            bool match(std::string::const_iterator,
                       std::string::const_iterator,
                       std::string& matched_sub);

            //> �����Ƿ�ƥ�� ������ƥ����ִ����Լ���ʼƥ���λ�� v2
            bool match(std::string::const_iterator,
                       std::string::const_iterator,
                       std::string& matched_sub,
                       int& begin_pos);

            //> �����Ƿ�ƥ�� ������ƥ����Ӵ���ʼλ��
            bool match(std::string::const_iterator,
                       std::string::const_iterator,
                       std::string::const_iterator&,
                       std::string::const_iterator&);

            //! ��������
            //> �����Ƿ�ƥ�� ���ڶ����ڲ���¼����ƥ���λ����Ϣ
            bool match(std::string::const_iterator,
                       std::string::const_iterator);

            // NOTE �滻�滻��ģʽ
            // ������ʽ�滻�������vimΪ�ο�����Ϊ�����֡�
            // s/parttern/subs/ge
            // ���ȣ�subsҲ��һ�����Խ����Ĵ���
            // ���е�\\1�������Ϊ�ִ���
            // Ȼ������ƥ��Ĳ��֣�����������滻����
            //
            // �����滻����ַ���
            std::string substitute(const std::string&,
                                   const std::string&);

            std::string substitute(std::string::const_iterator,
                                   std::string::const_iterator,
                                   std::string::const_iterator,
                                   std::string::const_iterator);

            //! sugar
            //> "�����ƥ�����Ϣ������д�봫�������"�������ú�������
            //! ʹ�÷�����
            // std::vector<std::string> submatches;
            // sss::regex::simpleregex("\\[sql\\d+\\]")
            //    .all_sub_matches("[sql1] [sql23] [sql13] [sql0]",
            //                      std::back_inserter(submatches));
            //
            //! �����
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

            void reset(); // ����״ֵ̬����ӭ����һ��ƥ�䣻


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
            //! �ڲ�ʵ�ֽű���

            //  ����ƥ�������ƥ�䵱ǰ
            bool match_here(const_iterator,
                            const_iterator,
                            std::string::const_iterator,
                            std::string::const_iterator);

            // �����ϣ�Ӧ�õ�ƥ�䣬ֻ���� ������ʽ�� ��һ��Ԫ�أ�
            // �����ĵı�ƥ���ַ����ģ�������������
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

            //! ����restƥ��
            // ����ʵ�ǹ��ڻ��ݵģ�
            // Ϊ�˼��ٻ��ݴ����������� restƥ��ģʽ��ʱ������������ƥ���ʱ��
            // ֻ����ǰ���n��ƥ�䣬�����������ŵ�(n,m]��ƥ�䣻�����Ƿ�̰��ģʽ��
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
            int         start_pos;              // ƥ��״̬��
            int         consumed_cnt;           // ƥ��״̬��
            match_case_t match_case;

            std::string::const_iterator    str_beg_bak; // ��match����ʹ�ù����в���
            const_iterator tok_beg_bak;         // ��ʼ���󣬲���

            std::vector<int> left_anchors;      // ƥ��״̬��
            std::vector<int> right_anchors;     // ƥ��״̬��

            // ��¼branch��ǩ�±꣬��Ӧ�������ê���ǩ�±ꣻ
            // ������֧ƥ��Ľ���λ��
            std::map<int, int>    branch2right_anchor; // ��ʼ���󣬲���

            std::map<int, int> anchorpairs;     // ��ʼ���󣬲���
            std::map<int, int> anchorpairs_rev; // ��ʼ���󣬲���

            std::map<int, int> timespos;        // ��ʼ���󣬲���


            // ��ê���Ӧ�ķ�֧�±�����
            // ������֧�Ŀ�ʼλ��
            // ���ĳ��ê��֮�䣬û��ֱ�ӷ�֧����ô�죿
            // Ӧ����һ���յ����飡
            std::map<int, std::vector<int> > left_anchor2branchs;// ��ʼ���󣬲���

            bool is_match_here_called;          // ���Ƴ��� ���� match_here ��������Ϊ��
            bool least_match;                   // ����ƥ��ģʽ

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

                int         start_pos;              // ƥ��״̬��
                int         consumed_cnt;           // ƥ��״̬��
                std::vector<int> left_anchors;      // ƥ��״̬��
                std::vector<int> right_anchors;     // ƥ��״̬��
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
