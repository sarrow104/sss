#ifndef  __ARGSSTRINGBUILDER_HPP_1450076657__
#define  __ARGSSTRINGBUILDER_HPP_1450076657__

// 模拟 Qt::QString().arg()的功能；
// 并消除其使用上的歧义性；
// 问题，参见：
//
//! http://blog.csdn.net/dbzhang800/article/details/6433061
//
// 简言之，每次arg()动作，都会让QString重新解析；
//
// 我这个版本，为了分离不变与变化，将其分为多个对象；
// 分别用来：
// 1. 保存用户的"%1%2...%3"这样的列表——并解析为中间结果；
// 2. 保存arg()数组；
// 3. 最用获取替换结果
// 参考：
//! /home/sarrow/extra/sss/include/sss/tdspp2/sql_var_replacer.hpp|42
//
// 其工作流程是，先对输入的字符串进行分析，分解为 串、对象引用、串，这样的组合；
//
// 只有当调用 generate() 方法的时候，才会往流里面，按其字符形式，写入数据；
//
// 这样，数据就可以多次使用；
//
// 当然，字符串解析后的形式，并没有单独保存，这使得拷贝不太可能。
//
// 另外，还有一个问题是，没有通用的解析器……
//
// 最好能设定解析器；即，由用户继承——
// 比如 sql_var_replacer 的解析器是冒号开头，后面跟字符；
//  ":\w\+\>"
// 而时空软件的变量定义，则是 ":\w\+\(.\w\+\>\)?" 即，允许带一个 ".suffix" 这样
// 的东西；
//
// 当然，还有就是转义字符的问题；
// 是否需要考虑转义字符？
// 考虑的话，如何设定？
// 必须要解决这些，问题，才能制作出一个真正方便的工具；
//
// 而我原版里面，为了处理冒号，甚至用了多个函数：
//  void do_s_colon(char ch);
//  void do_d_colon(char ch);
//  void do_other(char ch);
//  这些名字难看的函数，就是专门用来处理所谓的冒号的；一个冒号，怎么办？两个呢？
//  其他情况呢？
//
// sql_var_replacer的构造函数是将传入的字符串，根据冒号变量，来划分成字符串数组
// ；
//
// 在最后 generate 的时候，每次还会根据这个数组每条记录的匹配情况，进行生成；
//
// 简言之，不会判断，是否有"变量"没有被替换——没有提供的变量，会保留冒号变量的
// 形式，原样输出；
//
// 用哪种变量的形式？
// 仅类Qt的前缀式吗？
//
// 如果，仅考虑 %1 的形式；
//     单个的%表示它自己；
//     两个%%也表示它自己；
//     %\d\+表示变量；
//     不过，还是有问题——如何分割呢？比如我在变量%1之后，需要马上跟着其他的数
//     字，比如2——注意，不是变量！
//     那么，应当如何操作？
//     没办法的；如果写成%12，那么会被当做变量的！
//     所以，这种前缀式，是有漏洞的；
//
//     需要再加上一个后缀，用来分割，不然容易有歧义；
//
// 综上，我选择{\d\+}的形式；
//
// 同时，在对原始格式进行切分之后，如何马上区别普通字符串，还是变量呢？
// 有两种方式；
//
//  1. struct { type .., union {range_t ...} } 的形式；不过，遗憾的是，这里的
//  range_t相当于是拥有构造函数的类，所以，不能直接使用union；
//
//  2. 采用"奇偶数"法；
//
//     规定，变量前面，必须是一个字符串rang_t；
//     也就是说，字符串在奇数位；变量在偶数位；
//
//  带有前缀、后缀的形式，难道就没有问题了吗？
//
//  类似前缀式，如果我要生成的字符串中，带有 "{1}" 子串，应该怎么办？
//  还是得靠转义！
//  此时，我需要将第一个"{"字符，转义掉；

#include <sss/util/StringSlice.hpp>
#include <sss/utlstring.hpp>
#include <sss/util/Memory.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <iosfwd>
#include <vector>

namespace sss {
    namespace util {

        /**
         * @brief 将输入字符串，拆分问用raw_string间隔的变量列表；
         */
        class ArgsStringBuilder
        {
            friend class ArgsList;

        public:
            explicit ArgsStringBuilder(const std::string& format);
            virtual ~ArgsStringBuilder();

        private:
            /**
             * @brief 由于m_stems内部的iterator，本质上不是标量，所以暂时禁用赋值与拷贝构造
             */
            ArgsStringBuilder& operator=(const ArgsStringBuilder& );
            ArgsStringBuilder(const ArgsStringBuilder& );

        private:
            /**
             * @brief 保存字符串部分
             */
            std::vector<StringSlice<std::string::const_iterator> > m_stems;   // 
            /**
             * @brief NOTE 变量是{\d\+}形式！这里保存的是对应的序号；
             * 0 表示不存在；
             */
            std::vector<int>    m_varibles;     // 变量部分；
            std::string m_format;               // 原始字符串；
        };

        /**
         * @brief 变量列表
         */
        class ArgsList;
        /**
         * @brief 变量基类
         */
        class ArgHolder
        {
            friend class ArgsList;
            friend class sss::scoped_ptr<ArgHolder>;

        protected:
            ArgHolder() {}
            virtual ~ArgHolder() {}

        public:
            virtual void print(std::ostream& o) const = 0;
        };

        inline std::ostream& operator << (std::ostream& o, const ArgHolder& ah)
        {
            ah.print(o);
            return o;
        }

        /**
         * @brief 纯字符串变量 <- 变量基类
         */
        class StringHolder : public ArgHolder
        {
            friend class ArgsList;
        protected:
            explicit StringHolder(const std::string& s)
                : m_data(s)
            {
            }
            template<typename T>
            explicit StringHolder(const T& val)
            {
                this->reset(val);
            }
            ~StringHolder()
            {
            }

            void reset(const std::string& s)
            {
                this->m_data = s;
            }

            template<typename T> void reset(const T& val)
            {
                this->m_data = sss::cast_string(val);
            }

        public:
            void print(std::ostream& o) const {
                o << this->m_data;
            }

        private:
            std::string m_data;
        };

        const char * ignore = "";

        /**
         * @brief 引用变量 <- 变量基类
         *
         * @tparam T 任意类型；内部会建立该对象的引用
         */
        template<typename T> class RefHolder : public ArgHolder
        {
            friend class ArgsList;
        protected:
            explicit RefHolder(T& val)
                : m_data(val)
            {
            }
            ~RefHolder()
            {
            }

        public:
            void print(std::ostream& o) const {
                o << this->m_data;
            }

        private:
            T& m_data;
        };

        /**
         * @brief 变量列表；以指针的数组方式进行管理；并管理内存；
         *        用于ArgsStringBuilder的构建时候的替换；
         */
        class ArgsList : private std::vector<ArgHolder *>
        {
        public:
            ArgsList(ArgsStringBuilder& as)
                : m_pbuilder(&as)
            {
            }

            ArgsList() : m_pbuilder(0) {
            }

            ~ArgsList() {
                this->clear();
            }

        public:
            ArgsStringBuilder * bind(ArgsStringBuilder * asb)
            {
                std::swap(this->m_pbuilder, asb);
                return asb;
            }

        public:
            template<typename T> void argRef(T& val, int idx = 0) {
                idx = indexToSlot(idx);

//                std::cout << __func__ << "(" << val << ", " << idx << ")" << std::endl;

                delete this->at(idx);
                sss::scoped_ptr<ArgHolder> ref(new RefHolder<T>(val));
                this->operator[](idx) = ref.get();
                ref.release();
            }

            template<typename T> void argCon(const T& val, int idx = 0) {
                idx = indexToSlot(idx);

//                std::cout << __func__ << "(" << val << ", " << idx << ")" << std::endl;
                delete this->at(idx);
                sss::scoped_ptr<ArgHolder> ref(new StringHolder(val));
                this->operator[](idx) = ref.get();
                ref.release();
            }

        public:
            void print(std::ostream& ) const;

            void clear() {
                while (!this->empty()) {
                    delete this->back();
                    this->pop_back();
                }
            }

        protected:
            // index = base from 1 to base from 0
            int indexToSlot(int idx)
            {
                if (idx <= 0) {
                    iterator it = std::find(this->begin(), this->end(), reinterpret_cast<void*>(0));
                    if (it == this->end()) {
                        this->reserve(this->size() + 1);
                        idx = this->size();
                    }
                    // 找最后一个0，然后赋值；如果找不到，则push_back()
                }
                else {
                    this->reserve(idx);
                }
                return idx - 1;
            }

        protected:
            void reserve(int n) {
//                std::cout << __func__ << "(" << n << ")" << std::endl;
                while (int(this->size()) < n) {
                    this->push_back(0);
                }
            }

        private:
            ArgsStringBuilder * m_pbuilder;
        };

        inline std::ostream& operator << (std::ostream& o, const ArgsList& al)
        {
            al.print(o);
            return o;
        }

#if __cplusplus >= 201103
        inline void printf(const char * s)
        {
            while(*s) {
                if (*s == '%' && *(++s) != '%') {
                    throw std::runtime_error("invalid format string: missing arguments");
                }
                std::cout << *s++;
            }
        }

        template<typename T, typename... Args> void printf(const char * s, T value, Args... args)
        {
            while(*s) {
                if (*s == '%' && *(++s) != '%') {
                    std::cout << value;
                    printf(*s ? ++s : s, args...); // 即便当 *s == 0 也会产生调用，以检测更多的类型参数
                    return;
                }
                std::cout << *s++;
            }
            throw std::runtime_error("extra arguments provided to printf");
        }

        template<typename... Args>
        std::string ConstArgsStringBuilder(const std::string& format, Args... args)
        {
            std::ostringstream oss;
            ArgsStringBuilder asb(format);
            ArgsList al(asb);
            ConstArgsBuilderHelper(al, args...);
            oss << al;
            return oss.str();
        }

        template<typename... Args>
        std::string RefArgsStringBuilder(const std::string& format, Args... args)
        {
            std::ostringstream oss;
            ArgsStringBuilder asb(format);
            ArgsList al(asb);
            RefArgsBuilderHelper(al, args...);
            oss << al;
            return oss.str();
        }

        template<typename T, typename... Args>
        void RefArgsBuilderHelper(ArgsList & al, T& value, Args... args)
        {
            al.argRef(value);
            RefArgsBuilderHelper(al, args...);
        }

        inline void RefArgsBuilderHelper(ArgsList & )
        {
            // nothing here;
        }

        template<typename T, typename... Args>
        void ConstArgsBuilderHelper(ArgsList & al, const T& value, Args... args)
        {
            al.argCon(value);
            ConstArgsBuilderHelper(al, args...);
        }

        inline void ConstArgsBuilderHelper(ArgsList & )
        {
            // nothing here;
        }
#endif
    }
}


#endif  /* __ARGSSTRINGBUILDER_HPP_1450076657__ */
