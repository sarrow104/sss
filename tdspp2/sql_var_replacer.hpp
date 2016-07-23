#ifndef  __SQL_VAR_REPLACER_HPP_1393478517__
#define  __SQL_VAR_REPLACER_HPP_1393478517__

#include <sstream>
#include <string>
#include <map>
#include <vector>

namespace sss {
namespace tdspp2 {

// 分词，找到 : 开头的短语位置
// 用户提供替换字符串；
// 最后获取，生成的脚本；
//
// 将用户提供的字符串，以 "串1" :名字 "串2" ... 的方式，进行分割；
//
// 冒号开头的短语，会在每次"生成"的时候，进行动态替换，以生成变化的脚本。
//
// 另外，引入了模板机制，所以，只要能输出到"流"的各种对象，都可以作为变量来绑定
// ！
//
// 另外，生成器内部，使用了状态机，对字符串进行分割；

class sql_var_replacer{
    class built_in {
        friend class sql_var_replacer;

    protected:
        built_in() {}
        virtual ~built_in() {}

    public:
        virtual std::string to_string() const = 0;
    };

    class ValueVar : public built_in {
        friend class sql_var_replacer;

    private:
        explicit ValueVar(const std::string& val) : str(val) {
        }

    public:
        virtual std::string to_string() const {
            return str;
        }

    private:
        std::string str;
    };

    template<typename T> class ReferVar : public built_in{
        friend class sql_var_replacer;

    public:
        typedef T value_type;

    private:
        T& value_ref;

    private:
        explicit ReferVar(T& ref) : value_ref(ref) {
        }
        ~ReferVar() {
        }

    public:
        std::string to_string() const {
            std::ostringstream oss;
            oss << value_ref;
            return oss.str();
        }
    };

public:
    explicit sql_var_replacer(const std::string& sql);
    ~sql_var_replacer();

public:
    void set_var(const std::string& name, const std::string& value);

    template<typename T> void set_ref(const std::string& name, T& ref) {
        colon_vars_t::iterator it = colon_vars.find(name);
        if (it == colon_vars.end()) {
            this->colon_vars.insert(it,
                                    std::make_pair(name, new ReferVar<T>(ref)));
        }
        else {
            delete it->second;
            it->second = new ReferVar<T>(ref);
        }
    }

    std::string generate() const;

    typedef void (sql_var_replacer::*feeder_t) (char ch);
    feeder_t feeder;

protected:
    void add_stem(const std::string& s);

protected:
    void do_s_colon(char ch);
    void do_d_colon(char ch);
    void do_other(char ch);

    bool is_id_char(char ch) const;

private:
    typedef std::vector<std::string> stems_t;
    stems_t stems;

    typedef std::map<std::string, built_in*> colon_vars_t;
    colon_vars_t colon_vars;

    std::string stem;
};

}
}

#endif  /* __SQL_VAR_REPLACER_HPP_1393478517__ */
