#ifndef  __JSONPP_HPP_1407418756__
#define  __JSONPP_HPP_1407418756__

// string
// number
// object
// array
// true
// false
// null

#include <iosfwd>
#include <string>
#include <map>
#include <vector>

#include <stdexcept>

namespace sss {
namespace jsonpp
{
    class JValue;

    JValue *    parser(const std::string& json_str);
    int         parser(JValue& o, const std::string& json_str);

    enum jtype_t {
        JSON_VALUE = 0,         // 1
        JSON_STRING,            // 1
        JSON_NUMBER,            // 1
        JSON_OBJECT,            // 1
        JSON_ARRAY,
        JSON_TRUE,
        JSON_FALSE,
        JSON_NULL,

        JSON_BOOLEAN,           // 扩展
        JSON_INT,
        JSON_DOUBLE
    };

    // 对象是一个无序的“‘名称/值’对”集合。一个对象以“{”（左括号）开始，“
    // }”（右括号）结束。每个“名称”后跟一个“:”（冒号）；“‘名称/值’ 对”
    // 之间使用“,”（逗号）分隔。

    class JVisitor;

    class JValue
    {
        friend class JVisitor;
        friend class JVPrinter;
        friend class JVCounter;
    public:
        JValue(jtype_t t = JSON_VALUE);
        virtual ~JValue();

    public:
        virtual void accept(JVisitor& )                  const;

    public:
        virtual JValue& operator[](int)                 const;
        virtual JValue& at(int)                         const;

        virtual JValue& operator[](const std::string&)  const;
        virtual JValue& key(const std::string&)         const;
        virtual bool    has_key(const std::string&)     const;

        virtual int     size()                          const;

    public:
        virtual bool    add(JValue * val);               // JSON_ARRAY
        virtual bool    add(const std::string& name, JValue * val); // JSON_OBJECT

    public:
        // NOTE 因为不是使用的 handel + pimpl 策略，所以，这里的swap动作，只能
        // 针对同类的object
        virtual bool    swap(JValue& val);
        virtual JValue * clone()                        const;

    public: // get
        // 是否所有对象，都可以转换为 string（序列化） ？
        virtual std::string     get_string()            const;

        virtual int             get_int()               const;
        virtual double          get_double()            const;

        virtual bool            get_bool()              const;

        // 如何获取 null 值？
        virtual void *          get_null()              const;

    public:
        virtual std::string     to_str()                const;

    public:

        bool    is_string()     const;
        bool    is_number()     const;

        bool    is_object()     const;
        bool    is_array()      const;

        bool    is_true()       const;
        bool    is_false()      const;
        bool    is_null()       const;

        bool    is_boolean()    const;
        bool    is_int()        const;
        bool    is_double()     const;
        //----------------------------------------------------------------------

    public:
        int             get_type()      const;
        const char *    get_type_str()  const;

        int             node_count()    const;

    public:
        void            print(std::ostream& os) const;

    public:
        static const char * type_id2name(int id);

    private:
        jtype_t         jtype;
    };

    std::ostream& operator << (std::ostream& o, const JValue& jval);

    // NOTE
    // 类脚本语言，json对象默认值是null
    // 但是，就 BNF 范式来说，json 的构建用字符序列，是不允许为'空'的！
    // 至少都应该是 一个 "null"；

    // NOTE jsonpp_object 就定义上说，是 "名称/值" 对的无序组合；
    // 那就意味着，是属于 key-value 的，key无重复的形式；
    // 于是：
    class JObject : public JValue
    {
    public:
        friend class JVisitor;
        //friend class JVPrinter;
        friend class JVCounter;
        typedef std::map<std::string, JValue*> data_t;

    public:
        JObject();
        ~JObject();

    public:
        virtual JValue& operator[](const std::string&)  const;
        virtual JValue& key(const std::string&)         const;

        virtual bool    has_key(const std::string&)     const;
        virtual int     size()                          const;

        virtual bool    add(const std::string& name, JValue * val); // JSON_OBJECT
        virtual bool    swap(JValue& val);
        virtual JValue * clone()                        const;

        virtual std::string     to_str()                const;

        // FIXME TODO
        // 1. 如何枚举键值？N多动态语言，都有类似{}的结构；他们一般提供了返回
        // key数组的方法；有的甚至提供了返回 value数组的方法；
        //    或者说，相比xml，json缺少一个能描述其数据结构的方法；
        //
        //    解决办法：
        //       a. 使用迭代器；JObject::iterator。
        //       b. 提供以数字方式，访问key-value的办法；将"无序"，变"有序"。
        //       c. 提供一个keys(),values()的方法——虽然看起来有点傻；
        //
        //    比较：
        //       方法 a，实现起来稍微麻烦；为了使用方便，需要JHandle和JValue同
        //       时调整；
        //       方法 b，实现起来最简单，不用调整JHandle，但是显得没有效率；
        //       方法 c，实现起来也简单，不过使用者会需要维护额外的数据结构；除
        //       非JObject内部同时保存key的数组；
        //
        // 2. {} 貌似没有提供移除某一个 key-value 对的方法；
        //
        // 3. visiter呢？理论上visitor也可以制作迭代器！
        //
        // 通过visitor的话，可以提供一个统一的截面；所有json类型，都可以visitor
        // 的方式，进行迭代；
        //
        // 只不过 JNull，JDouble等类型，实际上只能迭代它自己。

    public:
        const data_t& get_data() const;

    private:
        data_t data;
    };

    // 数组是值（value）的有序集合。一个数组以“[”（左中括号）开始，“]”（右
    // 中括号）结束。值之间使用“,”（逗号）分隔。
    class JArray : public JValue
    {
    public:
        typedef std::vector<JValue*> data_t;
        friend class JVisitor;
        //friend class JVPrinter;
        friend class JVCounter;

    public:
        JArray();
        ~JArray();

    public:
        virtual JValue& operator[](int idx)             const;
        virtual JValue& at(int idx)                     const;

        virtual int     size()                          const;

        virtual bool    add(JValue * val);               // JSON_ARRAY

        virtual bool    swap(JValue& val);
        virtual JValue * clone()                        const;

        virtual std::string     to_str()                const;

        // TODO
        // 1. [] 貌似没有提供移除某一个 元素 的方法；

    public:
        const data_t& get_data() const;

    private:
        data_t data;
    };

    // 字符串（string）是由双引号包围的任意数量Unicode字符的集合，使用反斜线转
    // 义。一个字符（character）即一个单独的字符串（character string）。
    //
    // 字符串（string）与C或者Java的字符串非常相似。
    class JString : public JValue
    {
    public:
        typedef std::string data_t;
        friend class JVisitor;
        friend class JVPrinter;
        friend class JVCounter;
    public:
        JString(const std::string& val = "");
        ~JString();

    public:
        std::string     get_string()            const;
        virtual std::string     to_str()                const;
        virtual bool    swap(JValue& val);
        virtual JValue * clone()                        const;

    private:
        data_t data;
    };

    // 数值（number）也与C或者Java的数值非常相似。除去未曾使用的八进制与十六进
    // 制格式。除去一些编码细节。
    class JDouble : public JValue
    {
    public:
        typedef double data_t;
        friend class JVisitor;
        friend class JVPrinter;
        friend class JVCounter;
    public:
        JDouble(double val = 0.0);
        ~JDouble();

        double          get_double()                    const;
        virtual std::string     to_str()                const;
        virtual bool    swap(JValue& val);
        virtual JValue * clone()                        const;

    private:
        data_t data;
    };

    class JInt : public JValue
    {
    public:
        typedef int data_t;
        friend class JVisitor;
        friend class JVPrinter;
        friend class JVCounter;
    public:
        JInt(double val = 0);
        ~JInt();

        int             get_int()               const;
        virtual std::string     to_str()                const;
        virtual bool    swap(JValue& val);
        virtual JValue * clone()                        const;

    private:
        int data;
    };

    // 对应 boolean 类型，是否需要分开 json.true 和 json.false？
    // 如果真把他们看做是boolean对象值，那么他们应该可以进行bool运算。
    // 也就是说，json.true 与 json.false 之间，应该可以进行运算。即，他们应该是
    // 同一种子类型。
    // 要体现他们的不同，那就只有通过内部的"值"来判断了。
    class JBool : public JValue
    {
    public:
        typedef bool data_t;
        friend class JVisitor;
        friend class JVPrinter;
        friend class JVCounter;
    public:
        JBool(bool val = false);
        ~JBool();

    public:
        bool            get_bool()                           const;
        virtual std::string     to_str()                const;
        virtual bool    swap(JValue& val);
        virtual JValue * clone()                        const;

    private:
        data_t data;
    };

    class JNull : public JValue
    {
        friend class JVisitor;
        friend class JVPrinter;
        friend class JVCounter;
    public:
        JNull();
        ~JNull();

    public:
        void *          get_null()                      const;
        virtual std::string     to_str()                const;
        virtual bool    swap(JValue& val);
        virtual JValue * clone()                        const;
    };

    class JParser
    {
    public:
        JParser();
        ~JParser();

    public:
        typedef std::string::const_iterator     const_iterator;
        typedef std::string::iterator           iterator;

        JValue *  falldown(const std::string& str);
        JValue *  falldown(const_iterator it_b, const_iterator it_e);

        const_iterator skip_white_space(const_iterator it_b, const_iterator it_e);

        JValue*  parse_value(const_iterator& it_b, const_iterator it_e);

        JValue*  parse_string(const_iterator& it_b, const_iterator it_e);
        JValue*  parse_number(const_iterator& it_b, const_iterator it_e);
        JValue*  parse_array(const_iterator& it_b, const_iterator it_e);
        JValue*  parse_object(const_iterator& it_b, const_iterator it_e);
        JValue*  parse_bool(const_iterator& it_b, const_iterator it_e);
        JValue*  parse_null(const_iterator& it_b, const_iterator it_e);

        bool     parse_string_impl(const_iterator& it_b, const_iterator it_e);

        //bool parse_colon(); // :
        //bool parse_comma(); // ,
    };

    // 从 字符串 创建 json 对象
    JValue * tokener_parse(const char * str);
    // NOTE 如何知道解析完毕？
    // xml必须要有根节点；然后，一个节点有其对应的结束节点；
    // JSON不同于xml，她判断结束的标准是不同的。
    // JSON，只能是value！——即，JSON也相当于必须有根节点；并且，根节点必须是
    // object,array,string,...,null等其中一个。
    // 即，不存在形如："1,2,3,4,5" 这样的序列。JSON是不认的！

    // 值（value）可以是双引号括起来的字符串（string）、数值(number)、true、
    // false、 null、对象（object）或者数组（array）。这些结构可以嵌套。

    // 我觉得可以针对JValue指针进行特化；利用模板类……
    // 需要修改的是构造函数；
    class JHandle
    {
    public:
        explicit JHandle(const std::string& jsonstr);
        explicit JHandle(JValue * pval);
        JHandle();
        ~JHandle();

    public:
        //inline operator JValue*()
        //{
        //    return this->data;
        //}

        void swap(JHandle& ref);

        inline void validate() const throw(std::runtime_error)
        {
            // 注意，只有调用内部方法的动作，有必要检测内部指针的有效性！
            if (!this->data) {
                throw std::runtime_error("zero JValue * pointer");
            }
        }
        inline JValue * operator->() const
        {
            this->validate();
            return this->data;
        }

        // 获取内部指针
        inline JValue * get() const
        {
            return this->data;
        }

        inline JValue& operator*()
        {
            this->validate();
            return *this->data;
        }

        // NOTE assign应该返回自己的引用，而不是老指针！
        // 老指针应当销毁！
        JHandle& assign(const std::string& jsonstr);
        JHandle& assign(JValue*);

        // release()这个函数名，也有问题；会让人误以为是释放……
        // 这里重点在于所有权！
        JValue * release();

        // ...
        void clear();

    private:// 禁止拷贝与复制
        JHandle(const JHandle& );
        JHandle& operator = (const JHandle&);

    private:
        JValue * data;
    };

    class JVisitor
    {
    public:
        virtual void visit(const JValue * );

        virtual void visit(const JArray * )   = 0;
        virtual void visit(const JObject * )  = 0;

        virtual void visit(const JBool * )    = 0;
        virtual void visit(const JDouble * )  = 0;
        virtual void visit(const JInt * )     = 0;
        virtual void visit(const JNull * )    = 0;
        virtual void visit(const JString * )  = 0;
    };
}

}


#endif  /* __JSONPP_HPP_1407418756__ */
