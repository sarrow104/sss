#ifndef  __SKIPPER_HPP_1435830830__
#define  __SKIPPER_HPP_1435830830__

#include <algorithm>
#include <string>
#include <iterator>

#include <sss/log.hpp>

namespace sss {
namespace stream {

// NOTE 关于 Rewinder 的begin,commit,rollback关系
// bool begin()
//      用当前的迭代器值，作为初始值
// bool commit()
//      标记内部状态，使得其析构函数，不会重置迭代器；
// bool rollback()
//      当内部状态为false的时候，立即重置迭代器
//------------------------------------------------
// "策略讨论"
// 1. 仅当 commit 之后，才不会 rewind;
// 2. 手动 rollback 之后，不会再次 rewind；
// 3. begin 动作发生与否，与 析构的关系？
// 4. 没有commit的话，析构会触发 rollback;
//
// 我想得到如下效果：
//
// Rewinder_t r;
// Rewinder_t r1;
//
// if (rule1()) {
//    r2.begin() && r2.commit(rule2);
//    r.commit(rule3());
// }
// if (!r.ok()) {
//    r.commit(rule4());
// }
//
// 注意，在第二个分支 rul4() ，并没有触发r2的begin()动作；此时，我也不
// 希望其在析构的时候，自动rollback();
//
// 貌似，必须在构造的时候，就提供一个激活信息才行。不然，就需要更改策略
// ——必须手动rollback调用！
//
// 即，取消析构-rewind概念——但，这与异常抛出的需求不符！
//
// 要么，就在外层成功的时候，手动commit所有的Rewinder_t对象——这不现实
// ，这要求，同时增加三个地方！(定义、使用、还有最后的on_commit);
//
// 还有办法，就是让begin更改实现——内部增加一个s_beg的栈；然后必须搭配
// commit(is_success)动作使用；
// 不管子匹配，是否成功，都触发pop栈动作；
// 应该是可行的；不过若子匹配内部有异常抛出，将无法截获——于是，只有顶
// 层的析构能够包含。
//
// 当然问题不大；
//
// 优点：
//    可行；编程难度不大——特别是没有嵌套的子匹配的话，栈长度有限；于
//    是，不用考虑动态内存分布；
//
//    Rewinder_t对象的数量也得到了控制；
//
//    不过，有些嵌套是
//    while (true) {
//       r2.begin();
//       if (rulex()) {
//          r2.commit();
//       }
//       else {
//          r2.rollback();
//          break;
//       }
//    }
//
//    形式，且 rulex() 表达式复杂，不方便轻易表示成：
//    while (r2.begin() && r2.commit(rulex()));
// =====================
// 或者，不用栈，用"步进"的方式？
// 首先，记录构造函数发生时，迭代器的起始位置；然后，设置一个临时起点，
// 记录begin()动作发生时的位置；
// 。。。
//
// 没意义，相当于高度为2的栈。只不过，不断重用栈顶而已。
// =====================
// 还有一个办法！
// 原本的commit是不做动作的！
// 可以如此，既然commit肯定是手动触发，那么可以额外做一个标记，记录
// commit的时候，it_cur到了那里！
//
// 之后，在析构的时候，将it_cur重设置为其记录的值即可！
// ---------------------
// 另外，是关于ok的判断！
// 有两种策略：
// 1. 根据是否有手动调用commit来判断；
// 2. 根据是否有消耗字符来判断。
template<typename chiter>
    class Rewinder {
    public:
        explicit Rewinder(chiter & it)
            : is_ok(false),
            it_cur(it), it_bak(it), it_commit(it)
        {
#if _DEBUG_
            SSS_LOG_DEBUG("%s@%p\n",
                          __func__, this);
#endif
        }

        ~Rewinder()
        {
#if _DEBUG_
            SSS_LOG_DEBUG("%s@%p\n",
                          __func__, this);
#endif
            this->rollback();
        }

    public:
        inline bool begin()
        {
            if (!this->ok()) {
                this->it_bak = it_cur;
                this->it_commit = it_cur;
            }
            return !this->ok();
        }

        inline bool commit()
        {
            this->is_ok = true;
            this->it_commit = this->it_cur;
            return this->is_ok;
        }

        inline bool ok() const
        {
#if 1
            // 默认，it_cur 与 it_bak 差多少，就
            return this->is_ok;
#else
            return this->count();
#endif
        }

        inline operator void * ()
        {
            return reinterpret_cast<void *>(this->ok());
        }

        chiter get_it_cur() const
        {
            return this->it_cur;
        }

        chiter get_it_beg() const
        {
            return this->it_bak;
        }

        chiter get_it_end() const
        {
            return this->it_commit;
        }

        inline int count() const {
            return std::distance(it_bak, it_cur);
        }

        inline bool commit(bool is_success) {
            if (!is_success) {
                this->rollback();
            }
            else {
                this->commit();
            }
            return is_success;
        }

        inline void rollback()
        {
#if 0
            if (!is_ok && this->it_cur != it_commit) {
                //this->commit();
            }
#else
            if (this->it_cur != it_commit) {
#if _DEBUG_
                SSS_LOG_DEBUG("%s@%p offset = %d\n",
                              __func__, this,
                              std::distance(this->it_cur, this->it_commit));
#endif
                this->it_cur = it_commit;
            }
#endif
        }

        std::string get_consume()
        {
            return std::string(it_bak, it_cur);
        }

        std::string get_consume_ok()
        {
            if (this->ok()) {
                return std::string(this->it_bak, this->it_commit);
            }
            else {
                return "";
            }
        }

        void consume(int cnt)
        {
            this->it_cur = this->it_bak;
            std::advance(this->it_cur, cnt);
        }

    private:
        bool     is_ok;
        chiter & it_cur;
        chiter   it_bak;
        chiter   it_commit;
    };

template<typename chiter>
    class skipper {
    public:
        typedef chiter iterator;

    public:
        // 根据传入的引用、指针；
        // 在初始化的时候，记录一个值；
        // 再在结束的时候，记录一个值；
        class Recorder {
        public:
            Recorder(skipper& _sk, chiter& _it)
                : sk(_sk), it(_it)
            {
                this->it_b = it;
            }
            ~Recorder()
            {
                sk.set_beg(it_b);
                sk.set_end(it);
            }

        private:
            skipper& sk;
            chiter&  it;
            chiter   it_b;
        };

        typedef typename std::iterator_traits<chiter>::value_type value_type;
        typedef Rewinder<chiter> Rewinder_t;

        class Exception : public std::exception
        {
        public:
            Exception(const std::string& _msg) throw()
                : msg(_msg)
            {}
            virtual ~Exception() throw()
            {}

        public:
            virtual const char * what() const throw()
            {
                return this->msg.c_str();
            }
            std::string msg;
        };

    public:
        void clear()
        {
            this->it_beg_store = *this->p_cur;
        }

    public:
        inline bool skip_char(chiter & it, chiter end, char byte, const std::string& msg = "")
        {
            {
                Recorder d(*this, it);
                // FIXME 这里需要考虑负号bit位吗？
                if (it != end && *it == byte) {
                    ++it;
                }
            }

            return throw_on_false(msg);
        }

        inline bool skip_char_any(chiter & it, chiter end, const std::string& msg = "")
        {
            {
                Recorder d(*this, it);
                if (it != end) {
                    ++it;
                }
            }

            return throw_on_false(msg);
        }

        inline bool skip_char(chiter & it, chiter end, std::string valid, const std::string& msg = "")
        {
            {
                Recorder d(*this, it);
                std::sort(valid.begin(), valid.end());
                if (it != end && std::binary_search(valid.begin(), valid.end(), *it))
                {
                    ++it;
                }
            }

            return throw_on_false(msg);
        }

        inline bool skip_char_verse(chiter & it, chiter end, std::string valid, const std::string& msg = "")
        {
            {
                Recorder d(*this, it);
                std::sort(valid.begin(), valid.end());
                if (it != end && !std::binary_search(valid.begin(), valid.end(), *it))
                {
                    ++it;
                }
            }

            return throw_on_false(msg);
        }

        inline bool skip_char_range(chiter & it, chiter end, value_type ini, value_type fin, const std::string& msg = "")
        {
            {
                Recorder d(*this, it);
                unsigned int i_ini(ini);
                unsigned int i_fin(fin);
                if (i_ini > i_fin) {
                    std::swap(ini, fin);
                }
                if (it != end && (unsigned int)(*it) >= i_ini && (unsigned int)(*it) <= i_fin)
                {
                    ++it;
                }
            }
            return throw_on_false(msg);
        }

        // NOTE 不能直接生成 skip_int,skip_short等函数；因为，一旦考虑多字
        // 节的情况，就需要字节序问题了。
        // 最好让库用户自己决定转化规则；

        inline bool skip_set(chiter & it, chiter end, std::string valid, const std::string& msg = "")
        {
            {
                Recorder d(*this, it);
                std::sort(valid.begin(), valid.end());
                while (it != end && std::binary_search(valid.begin(), valid.end(), *it))
                {
                    ++it;
                }
            }

            return throw_on_false(msg);
        }

        inline bool skip_set_verse(chiter & it, chiter end, std::string valid, const std::string& msg = "")
        {
            {
                Recorder d(*this, it);
                this->it_beg_store = it;
                std::sort(valid.begin(), valid.end());
                while (it != end && !std::binary_search(valid.begin(), valid.end(), *it))
                {
                    ++it;
                }
            }

            return throw_on_false(msg);
        }

        template<typename Funct>
            inline bool skip_if(chiter & it, chiter end, Funct f, const std::string& msg = "")
            {
                {
                    Recorder d(*this, it);
                    while (it != end && f(*it)) {
                        ++it;
                    }
                }

                return throw_on_false(msg);
            }

        template<typename Funct>
            inline int skip_if_times(chiter & it, chiter end, Funct f, int cnt, const std::string& msg = "")
            {
                bool ret = false;
                {
                    Recorder d(*this, it);
                    Rewinder_t r(it);
                    while (cnt && it != end && f(*it)) {
                        --cnt;
                        ++it;
                    }
                    ret = !cnt;
                    r.commit(ret);
                }

                return throw_on_false(ret, msg);
            }

        template<typename Funct>
            inline int skip_if_range_times(chiter & it, chiter end, Funct f, int t_min, int t_max, const std::string& msg = "")
            {
                if (t_min < 0 || t_min >= t_max) {
                    return 0;
                }
                bool ret = false;
                {
                    Recorder d(*this, it);
                    Rewinder_t r(it);
                    int cnt = t_max - t_min;
                    while (cnt && it != end && f(*it)) {
                        --cnt;
                        ++it;
                    }
                    ret = !cnt;
                    r.commit(ret);
                }

                return throw_on_false(ret, msg);
            }

        template<typename Funct>
            inline int skip_if_any_times(chiter & it, chiter end, Funct f, const std::string& msg = "")
            {
                bool ret = false;
                {
                    Recorder d(*this, it);
                    Rewinder_t r(it);
                    int cnt = 0;
                    while (it != end && f(*it)) {
                        cnt++;
                        ++it;
                    }
                    ret = cnt;
                    r.commit(ret);
                }

                return throw_on_false(ret, msg);
            }


        inline bool skip_dec(chiter & it, chiter end, const std::string& msg = "")
        {
            return skip_if(it, end, ::isdigit, msg);
        }

        //int a = 1.;
        // NOTE 数字前面的正负号，由用户自己处理；这里假设，已经遇到数字了？
        // FIXME '.' 能作为浮点数的开头吗？
        // C语言的浮点数，'.'可以是开头；而json则不行！
        // json的浮点数，必须以数字开头（并且0不在开头多次使用；要么是单独
        // 的一个数字0，要么就是"1-9"开头的长度大于1的数字串；
        // 接着是小数部分；小数部分则是 "(.\d\+)?"；接着是e指数：
        // "([eE](+|-)?\d\+)?"
        //
        // 而C风格，则不同，小数点可以作为小数部分的开头或者结尾；指数部分，还是一致的；
        // 并且，使用前导数字0，也没关系；08.1e2 照样被理解为810；
        //
        // 于是有：
        // \d*(\.\d+)?
        //
        // 貌似需要多种标准了……
        // 由于，我是用C/C++编译器来解析，并且C/C++风格比json风格要宽松，所
        // 以，完全可以用本函数来规范输入，再用Crt函数，将字符串转化为double；
        // (\d+\(\.\d*\)\?|\.\d+)
        // 写为bnf的话：
        //
        // frac // json-style
        //   -> '0'           ('.' [0-9]+)?
        //   ->  [1-9] [0-9]* ('.' [0-9]+)?
        //
        // frac // c-style
        //   -> [0-9]+ ('.' [0-9]*)?
        //   -> '.' [0-9]+
        //
        // e
        //   -> [eE] ('+'|'-')? [0-9]+

        enum double_style_t {ds_C, ds_JSON};

    protected:
        // e
        //   -> [eE] ('+'|'-')? [0-9]+
        inline bool skip_double_e(chiter& it_b, chiter it_e)
        {
            Rewinder_t r(it_b);
            return r.commit((skip_char(it_b, it_e, 'e') || skip_char(it_b, it_e, 'E')) &&
                            (skip_char(it_b, it_e, '+') || skip_char(it_b, it_e, '-') || true) &&
                            skip_dec(it_b, it_e));
        }

    protected:
        // frac // json-style
        //   -> '0'           ('.' [0-9]+)?
        //   ->  [1-9] [0-9]* ('.' [0-9]+)?
        inline bool skip_frac_json(chiter& it_b, chiter it_e)
        {
            //Recorder d(*this, it);
            if (it_b == it_e) {
                return false;
            }
            switch(*it_b) {
                case '0':
                    it_b ++;
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
                    this->skip_dec(it_b, it_e);
                    break;

                default:
                    return false;
            }
            if (this->is_begin_with(it_b, it_e, '.')) {
                Rewinder_t r(it_b);
                it_b++;
                r.commit(this->skip_dec(it_b, it_e));
            }
            return true;
        }

    protected:

        // frac // c-style
        //   -> [0-9]+ ('.' [0-9]*)?
        //   -> '.' [0-9]+
        inline bool skip_frac_c(chiter& it_b, chiter it_e)
        {
            //Recorder d(*this, it);
            Rewinder_t r(it_b);
            return
                r.commit(this->skip_dec(it_b, it_e) &&
                         ((this->skip_char(it_b, it_e, '.') && this->skip_dec(it_b, it_e) >= 0) || true)) ||
                r.commit(this->skip_char(it_b, it_e, '.') && this->skip_dec(it_b, it_e));
        }

    public:
        inline bool skip_double(chiter& it_b, chiter it_e, double_style_t style = ds_JSON, const std::string& msg = "")
        {
            Recorder d(*this, it_b);
            Rewinder_t r(it_b);
            bool ret = (style == ds_JSON ? this->skip_frac_json(it_b, it_e) : this->skip_frac_c(it_b, it_e));
            skip_double_e(it_b, it_e);
            r.commit(ret);
            return throw_on_false(ret, msg);
        }

        inline bool skip_word(chiter & it, chiter end, const std::string& msg = "")
        {
            bool ret = false;
            {
                Recorder d(*this, it);
                Rewinder_t r(it);
                ret =
                    (this->is_begin_if(it, end, ::isalpha) || this->is_begin_with(it, end, '_')) &&
                    (it++, true);

                while (it != end && (::isalnum(*it) || *it == '_') ){
                    ++it;
                }

                r.commit(ret);
            }

            return throw_on_false(ret, msg);
        }

        inline bool skip_word_specify(chiter & it, chiter end, const std::string& word, const std::string& msg = "")
        {
#if 0
            Rewinder_t r(it);
            r.commit(this->skip_word(it, end) && this->last_matchs() == word);
            return throw_on_false(r.count(), msg);
#else
            if (word.empty()) {
                return false;
            }
            std::string::const_iterator it_word = word.begin();
            bool ret = false;
            {
                Recorder d(*this, it);
                Rewinder_t r(it);
                ret =
                    it != end && *it == *it_word &&
                    (::isalpha(*it) || *it == '_') &&
                    (it++, it_word++, true);

                while (ret && it != end && (::isalnum(*it) || *it == '_')) {
                    if (it_word != word.end() && *it == *it_word ) {
                        ++it;
                        ++it_word;
                    }
                    else {
                        ret = false;
                        break;
                    }
                }

                ret = ret && it_word == word.end();
                r.commit(ret);
            }

            return throw_on_false(ret, msg);
#endif
        }

        inline bool skip_bom_utf8(chiter & it, chiter end, const std::string& msg = "")
        {
            const char * bom_str = "\xef\xbb\xbf";
            return skip_bin(it, end, bom_str, bom_str + 3, msg);
        }

        bool skip_oct(chiter & it, chiter end, const std::string& msg = "")
        {
            {
                Recorder d(*this, it);
                while (it != end && '0' <= *it && *it <= '7') {
                    ++it;
                }
            }

            return throw_on_false(msg);
        }

        bool skip_hex(chiter & it, chiter end, const std::string& msg = "")
        {
            return skip_if(it, end, ::isxdigit, msg);
        }

        // NOTE 采用下面的参数策略，主要是为了防备编译器，选择函数出现二义性；
        template<typename It2>
            bool skip_str(chiter & it, chiter end, It2 tar_beg, size_t len, const std::string& msg = "")
            {
                It2 tar_end = tar_beg;
                std::advance(tar_end, len);
                return skip_bin(it, end, tar_beg, tar_end, msg);
            }

        bool skip_str(chiter & it, chiter end, const std::string& tar, const std::string& msg = "")
        {
            return skip_bin(it, end, tar.begin(), tar.end(), msg);
        }

        bool skip_str_before(chiter & it, chiter end, const std::string& tar, const std::string& msg = "")
        {
            return skip_bin_before(it, end, tar.begin(), tar.end(), msg);
        }

        bool skip_str_after(chiter & it, chiter end, const std::string& tar, const std::string& msg = "")
        {
            return skip_bin_after(it, end, tar.begin(), tar.end(), msg);
        }

        // TODO
        // 读取一行——包括换行符
        // 注意：windows下是 "\r\n" ；linux 下是 "\n"; mac 下是 "\r"
        //! http://blog.csdn.net/tskyfree/article/details/8121951
        bool skip_line_nl(chiter & it, chiter end, const std::string& msg = "")
        {
            Recorder d(*this, it);
            Rewinder_t r(it);
            {
                while (it != end && (*it != '\n' && *it != '\r')) {
                    ++it;
                }
                if (it != end && *it == '\r') {
                    ++it;
                }
                if (it != end && *it == '\n') {
                    ++it;
                }
            }
            r.commit();
            return throw_on_false(r.count(), msg);
        }

        bool skip_line(chiter & it, chiter end, const std::string& msg = "")
        {
            Recorder d(*this, it);
            Rewinder_t r(it);
            {
                while (it != end && (*it != '\n' && *it != '\r')) {
                    ++it;
                }
            }
            r.commit();
            return throw_on_false(r.count(), msg);
        }

        template<typename It2>
            bool skip_bin_before(chiter & it, chiter end, It2 tar_beg, It2 tar_end, const std::string& msg = "")
            {
                bool ret = false;
                {
                    Recorder d(*this, it);
                    Rewinder_t r(it);

                    while (true) {
                        if (is_begin_with(it, end, tar_beg, tar_end)) {
                            ret = true;
                            break;
                        }
                        if (it == end) {
                            break;
                        }
                        ++it;
                    }

                    r.commit(ret);
                }

                return throw_on_false(ret, msg);
            }

        template<typename It2>
            bool skip_bin_after(chiter & it, chiter end, It2 tar_beg, It2 tar_end, const std::string& msg = "")
            {
                bool ret = false;
                {
                    Recorder d(*this, it);
                    Rewinder_t r(it);
                    // 注意，skip_bin_before可能只会消耗0个字符，但是，返回true，表示同样匹配成功！
                    false =
                        skip_bin_before(it, end, tar_beg, tar_end) &&
                        skip_bin(it, end, tar_beg, tar_end);

                    r.commit(ret);
                }
                return throw_on_false(ret, msg);
            }

        // 掉过二进制
        template<typename It2>
            bool skip_bin(chiter & it, chiter end, It2 tar_beg, It2 tar_end, const std::string& msg = "")
            {
                bool ret = false;
                {
                    Recorder d(*this, it);
                    Rewinder_t r(it);

                    if (is_begin_with(it, end, tar_beg, tar_end)) {
                        std::advance(it, std::distance(tar_beg, tar_end));
                        ret = true;
                    }

                    r.commit(ret);
                }

                return throw_on_false(ret, msg);
            }

        // 匹配二进制
        template<typename It2>
            bool is_begin_with(chiter it, chiter end, It2 tar_beg, It2 tar_end)
            {
                while (it != end && tar_beg != tar_end) {
                    if (*it++ != *tar_beg++) {
                        return false;
                    }
                }
                return tar_beg == tar_end;
            }

        template<typename Func>
            bool is_begin_if(chiter it, chiter end, Func f)
            {
                return (it != end && f(*it));
            }

        template<typename Func>
            bool is_begin_if_not(chiter it, chiter end, Func f)
            {
                return (it != end && !f(*it));
            }

        bool is_begin_with(chiter it, chiter end, const std::string& tar)
        {
            return is_begin_with(it, end, tar.begin(), tar.end());
        }

        bool is_begin_with(chiter it, chiter end, char val)
        {
            return (it != end && *it == val);
        }

        // 跳过空白字符
        bool skip_ws(chiter & it, chiter end, const std::string& msg = "")
        {
            {
                Recorder d(*this, it);
                while (it != end && std::isspace(*it) && *it != '\n' && *it != '\r') {
                    ++it;
                }
            }

            return throw_on_false(msg);
        }

        // 跳过换行符
        // FIXME 应该一次跳过一个！
        bool skip_nl(chiter & it, chiter end, const std::string& msg = "")
        {
            {
                Recorder d(*this, it);
#if 0
                while (it != end (*it == '\n' || *it == '\r')) {
                    ++it;
                }
#else
                if (it != end && (*it == '\r')) {
                    ++it;
                }

                if (it != end && (*it == '\n')) {
                    ++it;
                }
#endif
            }

            return throw_on_false(msg);
        }

        // 跳过空白加换行符
        bool skip_ws_nl(chiter & it, chiter end, const std::string& msg = "")
        {
            return skip_if(it, end, ::isspace, msg);
        }

        // 工作策略，参考:
        // sss\BDEAlias.cpp|read_require273
        // 简述：
        //   读取定长串；并与目标串比较；如果不符，则退回指针；

    protected:
        inline void set_beg(chiter it)
        {
            this->it_beg_store = it;
        }

        inline void set_end(chiter it)
        {
            this->it_cur = it;
        }

        // void * 0 -> const std::string& ?
        // 还是
        // const char * ？
        inline bool throw_on_false(const std::string& msg)
        {
            if (!msg.empty() && !this->last_consume()) {
                throw Exception(msg);
            }
            return this->last_consume();
        }

        inline bool throw_on_false(bool is_ok, const std::string& msg)
        {
            if (!is_ok && !msg.empty()) {
                throw Exception(msg);
            }
            return is_ok;
        }

    public:
        inline int last_consume() const
        {
            return std::distance(this->it_beg_store, this->it_cur);
        }

        int last_matchs(std::string& ret) const
        {
            if (this->last_consume()) {
                ret.assign(this->it_beg_store, this->it_cur);
                return true;
            }
            return this->last_consume();
        }

        std::string last_matchs() const
        {
            return std::string(this->it_beg_store, this->it_cur);
        }

        int last_matchs(char * buf, int len, int& copy_len) const
        {
            if (this->last_consume()) {
                size_t max2cp = std::min(size_t(len), this->last_consume());
                chiter cur = this->it_beg_store;
                for (size_t i = 0; i != max2cp; ++i) {
                    buf[i] = *cur++;
                }
                copy_len = max2cp;
            }
            return this->last_consume();
        }
        //void rewind() {
        //    *this->p_cur = this->it_beg_store;
        //}

    private:
        chiter   it_beg_store;
        chiter   it_cur;
    };
}
}


#endif  /* __SKIPPER_HPP_1435830830__ */
