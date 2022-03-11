#ifndef __CONFIG_FILE_H__
#define __CONFIG_FILE_H__

/* 乱序的 配置文件读取器
 * http://www.adp-gmbh.ch/cpp/config_file.html
 * -- 只能读取；不能写入。
 *
 * 我认为，将配置文件的属性值，用非std::string类型的Chameleon来保存，有点过头了。
 * 应该由用户来决定如何辨认这些值。
 *
 * change-log:
 * 2012-02-19
 * 完成配置文件的磁盘保存。
 * 可以在内存中完成配置文件，然后写入到外存中。
 *
 * 缺点：
 * 1. 不能保证写入时，配置的顺序与读取时候的一致。
 * 2. 注释会丢失
 *
 * TODO
 *
 * 完成配置文件的顺序读取与顺序写入——与如何编写与读取有关
 *
 * TODO
 * 再创建一个子类，专门处理包含 [sec] xxx, [sec] xxx_mix 这样密码对处理过程；
 * 这是基于配置文件手动修改的形式；
 * 还有一种，是基于'密码控件输入框'的，就不需要上面这种操蛋的方法了。
 *
 * TODO 2013-03-29
 * 建立 block 的 map结构，方便用户枚举block名，而不需要
 * select distinct dbo.block_name(block_key_pair) from ini_data
 */
#include <sss/utlstring.hpp>

#include <exception>
#include <map>
#include <string>

namespace sss {
class ConfigFile
{
public:// member types
    using container_t = std::map<std::string, std::string>;
    //template <typename T> class option_t // {{{1
    //{
    //    typedef T iterator_t;
    //    iterator_t it;
    //public:
    //    explicit option_t(iterator_t i)
    //        : it(i) {}
    //    const std::string section() const;
    //    const std::string key() const;
    //    const std::string& value() const
    //    {
    //        return this->it->second;
    //    }
    //    option_t<T>& operator ++()
    //    {
    //        ++this->it;
    //        return *this;
    //    }
    //    option_t<T> operator ++(int)
    //    {
    //        option_t ret(*this);
    //        ++this->it;
    //        return ret;
    //    }
    //    operator iterator_t ()
    //    {
    //        return this->it;
    //    }
    //    bool operator == (const option_t<T>& right)
    //    {
    //        return this->it == right.it;
    //    }

    //    bool operator != (const option_t<T>& right)
    //    {
    //        return this->it != right.it;
    //    }
    //}
    class option_t      // sub-class option_t as iterator {{{1
    {
        container_t::iterator it;
    public:
        explicit option_t(container_t::iterator i)
            : it(i) {}
        std::string section() const;
        std::string key() const;
        const std::string& value() const
        {
            return this->it->second;
        }
        option_t& operator ++()
        {
            ++this->it;
            return *this;
        }
        option_t operator ++(int)
        {
            option_t ret(*this);
            ++this->it;
            return ret;
        }
        operator container_t::iterator ()
        {
            return this->it;
        }
        bool operator == (const option_t& right)
        {
            return this->it == right.it;
        }

        bool operator != (const option_t& right)
        {
            return this->it != right.it;
        }
    };

    class const_option_t      // sub-class option_t as iterator {{{1
    {
        container_t::const_iterator it;
    public:
        explicit const_option_t(container_t::const_iterator i)
            : it(i) {}
        explicit const_option_t(container_t::iterator i)
            : it(i) {}
        std::string section() const;
        std::string key() const;
        const std::string& value() const
        {
            return this->it->second;
        }
        const_option_t& operator ++()
        {
            ++this->it;
            return *this;
        }
        const_option_t operator ++(int)
        {
            const_option_t ret(*this);
            ++this->it;
            return ret;
        }
        operator container_t::const_iterator ()
        {
            return this->it;
        }
        bool operator == (const const_option_t& right)
        {
            return this->it == right.it;
        }

        bool operator != (const const_option_t& right)
        {
            return this->it != right.it;
        }
    };
    // endof sub-class option_t }}}1

    //typedef std::map<std::string, std::string>::const_iterator const_iterator;
    //typedef std::map<std::string, std::string>::iterator iterator;
    using value_type = std::map<std::string, std::string>::value_type;
    using iterator = option_t;
    using const_iterator = const_option_t;
    //typedef option_t<container_t::iterator> iterator;
    //typedef const_option_t<container_t::const_iterator> const_iterator;
    class Exception : std::exception
    {
    public:
        explicit Exception(const std::string& m) noexcept;
        Exception(const Exception& ) noexcept = default;
        ~Exception() noexcept override;
        const char * what() const noexcept override;

    private:
        std::string msg;
    };

public:                                         // members
    container_t content_;
    std::string dosini_fname;
    bool is_opened;
    bool is_modified;

public:                                         // ctr | dtr
    ConfigFile();
    explicit ConfigFile(std::string const& configFile);

    ~ConfigFile();

public:
    // FIXME TODO
    // 因为上面涉及了关于update的操作。肯定需要在内部记录一个是否经历了修改，但
    // 还没有将修改保存在外部文件中的，状态变量，比如上面的is_modified。有了这
    // 个变量，设计拷贝构造函数，已经复制函数的时候，就需要好好考虑了——通俗的
    // 做法是使用pimpl设计模式。
    // 在这里，为了简化，干脆就不支持类型的拷贝以及赋值，这两个操作。

    ConfigFile(const ConfigFile&);

private:
    ConfigFile& operator = (const ConfigFile&);

public:                                         // status
    bool is_open() const
    {
        return this->is_opened;
    }

public:
    void swap(sss::ConfigFile& cf);             // 交换数据空间；主要是用于容器的使用；

public:                                         // read | write | update
    bool read(const std::string& fname);
    bool write(const std::string& fname) const;
    bool update();

    bool clear();

public:                                         // iterator
    ConfigFile::const_iterator block_begin_const(std::string const& section);
    ConfigFile::const_iterator block_end_const(std::string const& section);
    ConfigFile::const_iterator block_end_const(std::string const& section, ConfigFile::const_iterator ini);

    ConfigFile::iterator       block_begin(std::string const& section);
    ConfigFile::iterator       block_end(std::string const& section);
    ConfigFile::iterator       block_end(std::string const& section, ConfigFile::iterator ini);

public:                                         // set | get
    // 取值（不存在的话，抛出 const char * 异常）
    const std::string& value(std::string const& section, std::string const& entry) const noexcept(false);
          std::string& value(std::string const& section, std::string const& entry) noexcept(false);

    // 取值（不存在的话，返回""）
    std::string value_no_throw(std::string const& section, std::string const& entry) const;
    std::string value_no_throw(std::string const& section, std::string const& entry) ;

    // 赋值 或 插入新值
    template <typename T>
    std::string const& value(std::string const& section, std::string const& entry, const T & val)
    {
        return this->value(section, entry, sss::cast_string(val));
    }
    std::string const& value(std::string const& section, std::string const& entry, std::string const& value);

    // TODO 删除值；
    void del_entry(std::string const& section, std::string const& entry);
    // TODO 删除区；
    void del_section(std::string const& section);

protected:
    // 当内存有修改，运行本函数
    void mark_modified();
    // 内存修改，更新到外存后，运行本函数
    void mark_updated();
};

namespace enc {
class EncBase;
}

namespace utl {

// ini配置文件的部分字段需要加密、解密处理；于是有了下面这个sugar类

class ConfigEncValued
{
public:
    ConfigEncValued(sss::enc::EncBase& enc, sss::ConfigFile& cfg);
    ~ConfigEncValued();

public:
    inline
    std::string get_value(const std::string& block,
                          const std::string& name,
                          const std::string& alias)
    {
        return this->get_value(block, name, block, alias);
    }

    // 加密前：block.name
    // 加密后：block_alias.name_alias
    std::string get_value(const std::string& block,
                          const std::string& name,
                          const std::string& block_alias,
                          const std::string& name_alias);

private:
    sss::enc::EncBase& enc;
    sss::ConfigFile& cfg;
};

} // namespace utl
} // namespace sss

#endif
