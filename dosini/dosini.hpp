#ifndef  __DOSINI_HPP_1396739445__
#define  __DOSINI_HPP_1396739445__

#include <sss/utlstring.hpp>
#include <sss/log.hpp>

#include <string>
#include <vector>
#include <list>
#include <map>

namespace sss {

    // FIXME
    // 对 空block 取值，不成功！ []!

// NOTE
// gap_file_t 应该是一种容器；
// 即，对外来说，外界不需要知道内部实现；
// 对外界来说，就是一个连续的内存好了！
class gap_file_t
{
public:
    gap_file_t();
    explicit gap_file_t(const std::string& cont);
    ~gap_file_t();

public:
    void read(const std::string& fname);
    void write(const std::string& fname);
    void update();

public:
    std::string get_slice(int pos, int len);
    bool get_slice(int pos, int len, std::string& slice);

    // 将内部 pos开始，到之后len字符，替换为传如的 slice 串
    bool insert(int pos, int len, const std::string& slice);

    // 注意，如果使用 pos,len 的方式来插入，那么意味着，如果用上述信息来管理切片
    // ，那么需要重算 pos,len 等信息！
    // 而我不想重算！
    // 于是，我还只能使用gap_f_indexer_t 来表达数据在 gap_file_t 中的位置！

private:
    bool is_modified;
    typedef std::vector<std::string> data_t;
    data_t data;

    struct gap_f_indexer_t
    {
    public:
        gap_f_indexer_t(int id, int off, int l);
        ~gap_f_indexer_t();

    public:
        int source_id;
        int offset;
        int len;
    };
    typedef std::list<gap_f_indexer_t> view_t;
    view_t view;
};

class dosini
{
public:
    typedef std::vector<std::string> padding_file_t;
    struct value_t
    {
        int lineno;
        int pos;
        int len;
        value_t()
            : lineno(0), pos(0), len(0)
        {
        }
        value_t( int r, int p, int l)
            : lineno(r), pos(p), len(l)
        {
        }
        ~value_t()
        {
        }
    };
    typedef std::map<std::string, value_t>      block_t;
    typedef std::map<std::string, block_t>      blocks_t;
    typedef std::list<int> linenos_t;

public:
    explicit dosini(const std::string& fname);
    dosini();
    ~dosini();

public:
    bool load();
    bool save() const;
    bool write(const std::string& fpath) const;

    void print(std::ostream& out) const;
    void print_clean(std::ostream& out) const;

public:
    template<typename T> T get_defult(const std::string& block, const std::string& key, const T& default_value = T())
    {
        T ret = default_value;
        this->get(block, key, ret);
        return ret;
    }

    template <typename T> bool get(const std::string& block, const std::string& key, T& val) const
    {
        blocks_t::const_iterator b_it = this->blocks.find(block);
        if (b_it != this->blocks.end()) {
            const block_t& b = b_it->second;
            block_t::const_iterator key_it = b.find(key);
            if (key_it != b.end()) {
                val =  sss::string_cast<T>(this->get_data_slice(key_it->second));
                return true;
            }
        }

        return false;
    }

    // NOTE TODO
    // 对 get<bool>() 进行特化。
    // 比如，"true" 也翻译为 true;

    template <typename T> T get(const std::string& block, const std::string& key) const
    {
        T ret;
        blocks_t::const_iterator b_it = this->blocks.find(block);
        if (b_it != this->blocks.end()) {
            const block_t& b = b_it->second;
            block_t::const_iterator key_it = b.find(key);
            if (key_it != b.end()) {
                ret =  sss::string_cast<T>(this->get_data_slice(key_it->second));
            }
        }

        return ret;
    }

    template <typename T> bool set(const std::string& block, const std::string& key, const T& val)
    {
        std::string str_val = sss::cast_string(val);
        blocks_t::iterator b_it = this->blocks.find(block);
        if (b_it != this->blocks.end()) {
            block_t& b = b_it->second;
            block_t::iterator key_it = b.find(key);
            if (key_it != b.end() && this->get_data_slice(key_it->second) != str_val) {
                linenos_t::iterator it = std::find(this->linenos.begin(), this->linenos.end(), key_it->second.lineno);
                if (it == this->linenos.end()) {
                    SSS_LOG_ERROR("value_t index not find.\n");
                    exit(EXIT_FAILURE);
                }
                key_it->second = this->append_key(block, key, str_val);
                *it = key_it->second.lineno;

                // 因为是更改原值，所以去掉新添加进去的一行；
                this->linenos.pop_back();
            }
            else {
                key_it->second = this->append_key(block, key, str_val);
            }
            return true;
        }
        else {
            this->append_block(block);
            this->append_key(block, key, str_val);
            return true;
        }

        return false;
    }

    dosini::value_t append_line(const std::string& line);

    dosini::value_t append_block(const std::string& block);

    dosini::value_t append_block(const std::string& line, int bk_start, int bk_end);

    dosini::value_t append_key(const std::string& block, const std::string& key, const std::string& val);

    dosini::value_t append_key(const std::string& block,
                               const std::string& line,
                               int key_start, int key_end,
                               int val_start, int val_end);

protected:
    std::string get_data_slice(const value_t& index) const;

protected:
    static bool getline(std::istream& ifs, std::string& line);

private:

    // 用来顺序存放用到的行号
    // 将 padding_file_t 中的行，顺序输出，就得到ini文件了。
    linenos_t linenos;

    std::string fname;
    padding_file_t  data;
    blocks_t        blocks;
    //std::string
};

} // end of namespace sss

#endif  /* __DOSINI_HPP_1396739445__ */
