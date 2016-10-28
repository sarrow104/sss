#include "ConfigFile.hpp"

#include <fstream>
#include <string>
#include <algorithm>

#include <sss/enc/encbase.hpp>
#include <sss/utlstring.hpp>
#include <sss/path.hpp>
#include <sss/log.hpp>

namespace sss{
static const char INI_FILE_SEC_OPT_SEP = '\n';
static const char * INI_FILE_MULLTY_LINE_VALUE_MARKER = ">>>";
static const char * INI_FILE_NEW_LINE_MARKER = "\r\n";

ConfigFile::Exception::Exception(const std::string& m) throw ()
    : msg("sss::Exception(" + m + ")")
{
}

ConfigFile::Exception::~Exception() throw ()
{
}

const char * ConfigFile::Exception::what() const throw()
{
    return this->msg.c_str();
}

const std::string ConfigFile::option_t::section() const
{
    return this->it->first.substr(0, this->it->first.find(INI_FILE_SEC_OPT_SEP));
}

const std::string ConfigFile::option_t::key() const
{
    return this->it->first.substr(this->it->first.find(INI_FILE_SEC_OPT_SEP) + 1);
}

const std::string ConfigFile::const_option_t::section() const
{
    return this->it->first.substr(0, this->it->first.find(INI_FILE_SEC_OPT_SEP));
}

const std::string ConfigFile::const_option_t::key() const
{
    return this->it->first.substr(this->it->first.find(INI_FILE_SEC_OPT_SEP) + 1);
}

ConfigFile::ConfigFile(std::string const& configFile)
    : is_opened(false), is_modified(false)
{
    SSS_LOG_DEBUG("ConfigFile::ConfigFile(std::string const& configFile) at %p\n", this);
    this->dosini_fname = configFile;
    if (sss::path::filereadable(configFile))
    {
        this->is_opened = this->read(this->dosini_fname);
    }
}

ConfigFile::ConfigFile()
    : is_opened(false), is_modified(false)
{
    SSS_LOG_DEBUG("ConfigFile::ConfigFile() at %p\n", this);
}

ConfigFile::ConfigFile(const ConfigFile& ref)
    : is_opened(false), is_modified(false)
{
    SSS_LOG_ERROR("ConfigFile::ConfigFile(const ConfigFile& ref) at %p\n", this);
    SSS_LOG_ERROR("ref at %p\n", &ref);
    if (ref.is_open())
        throw ConfigFile::Exception("ConfigFile 不允许，打开构造");
}

ConfigFile::~ConfigFile()
{
}

// 交换数据空间；主要是用于容器的使用；
// 2012-06-10
void ConfigFile::swap(sss::ConfigFile& cf)
{
    std::swap(this->content_,cf.content_);
    std::swap(this->dosini_fname,cf.dosini_fname);
    std::swap(this->is_modified, cf.is_modified);
    std::swap(this->is_opened, cf.is_opened);
}

bool ConfigFile::clear()        // clear buffer for next reading
{
    if (this->is_opened)
    {
        this->content_.clear();
        this->dosini_fname.resize(0);
        this->is_opened = false;
        this->is_modified = false;

        return true;
    }
    return false;
}

bool ConfigFile::read(const std::string& configFile)
{
    std::ifstream file(configFile.c_str(), std::ios::binary);

    if (file.is_open())
    {
        std::string line;
        std::string name;
        std::string value;
        std::string inSection;
        int posEqual;
        while (std::getline(file,line))
        {
            // 2012-02-19
            // trim white spaces
            sss::trim(line);

            // 空行
            if (! line.length())
                continue;

            // 注释行
            if (line[0] == '#')
                continue;
            if (line[0] == ';')
                continue;

            // Section
            if (line[0] == '[') {
                inSection = sss::trim_copy(line.substr(1,line.find(']') - 1));
                continue;
            }

            // 这里没有做'='存在与否的检查
            posEqual = line.find('=');
            name  = sss::trim_copy(line.substr(0, posEqual));
            // 2013-02-14 用户自行决定是否 sss::trim_copy()
            value = line.substr(posEqual + 1);
            SSS_LOG_EXPRESSION(sss::log::log_DEBUG, value);
            {   // 获取"多行"值：
                std::string mulline_mk = sss::trim_copy(value);
                SSS_LOG_EXPRESSION(sss::log::log_DEBUG, mulline_mk);
                if (mulline_mk.length() == 3 && mulline_mk[0] != '=' && ispunct(mulline_mk[0]) &&
                    mulline_mk[0] == mulline_mk[1] && mulline_mk[1] == mulline_mk[2])
                {
                    int pos = file.tellg();
                    bool is_fin_mk_found = false;
                    std::string tmp_value;
                    while (std::getline(file, line))
                    {
                        SSS_LOG_EXPRESSION(sss::log::log_DEBUG, sss::trim_copy(line));
                        if (sss::trim_copy(line) == mulline_mk)
                        {
                            is_fin_mk_found = true;
                            break;
                        }
                        // 这里最好不要 sss::trim(); 因为如果这里保存的是没有语
                        // 句结束符号的脚本，比如 python 等，会发生错误！
                        if (tmp_value.length())
                            tmp_value += INI_FILE_NEW_LINE_MARKER;
                        // 用 std::ios::binary 方式读取数据的时候，std::getline
                        // 不会把 "\r" 当作"回车换行"的一部分，而会保留。
                        if (line.length() && *line.rbegin() == '\r')
                        {
                            tmp_value.append(line, 0, line.length() - 1);
                            // tmp_value += line.substr(0, line.length() - 1);
                        }
                        else
                        {
                            tmp_value += line;
                        }

                    }
                    if (is_fin_mk_found)
                    {
                        value = tmp_value;
                    }
                    else
                    {
                        file.seekg(pos, std::ios::beg);
                    }
                }
            }

            // 这不是一个好方法――因为，这限制了不能用'/'作为名字。
            // 这里，应该使用，绝对不会出现 属性名 里面的 '\t' 或者 '\n' 等等
            //content_[inSection+'/'+name]=Chameleon(value);
            // Sarrow: 2011-11-04
            this->content_[inSection + INI_FILE_SEC_OPT_SEP + name] = value;
        }

        // log file status
        this->is_opened = true;
        this->dosini_fname = configFile;
        return true;
    }
    return false;
}

bool ConfigFile::write(const std::string& configFile) const
{
    std::ofstream file(configFile.c_str(), std::ios::binary);
    if (file.is_open())
    {
        // FIXME
        // dosini配置文件，分为块名和当前'块'对应的各种属性值。
        // 默认，有一个全局'块'；实际人工书写ini配置文件的时候，全局'块'，其实
        // 就是空的'块'名。
        //
        // 而当前功能库里面，全局'块'，写作"[]"。这个行为，需不需要修改呢？如果
        // 要修改的话，全局'块'，只能出现在配置文件的开头。而一下的方式，则可以
        // 出现在任何位置。
        //
        // 问题是：
        // 1. 需要修改这个行为吗？
        // 2. 如果修改，该如何实现？当前是用的排序的方式，那么就必须要考虑
        // 'INI_FILE_SEC_OPT_SEP'在排序中的"大小比较"问题。当前采用的是不可能出现在键值对的"\n"字符。
        //  有比他更小的可打印字符吗？如果没有的话，就可以保证这个出现在排序的第一个位置！
        //  当然，有一个更好的方案就是，将INI_FILE_SEC_OPT_SEP 定义为 "\0"――这就能保证它最小了！
        //
        std::string section(1, INI_FILE_SEC_OPT_SEP);// impossible section value
        bool is_first_line = true;
        for (container_t::const_iterator it = this->content_.begin(); it != this->content_.end(); ++it)
        {
            const_option_t opt(it);
            if (opt.section() != section)
            {
                section = opt.section();
                file << (is_first_line ? "" : INI_FILE_NEW_LINE_MARKER) // for an empty line at none 1st section begin
                     << "[" << section << "]" << INI_FILE_NEW_LINE_MARKER;
                is_first_line = false;
            }
            if (opt.value().find(INI_FILE_NEW_LINE_MARKER) != std::string::npos)
            {
                file << opt.key() << "=" << INI_FILE_MULLTY_LINE_VALUE_MARKER << INI_FILE_NEW_LINE_MARKER
                    << opt.value() << INI_FILE_NEW_LINE_MARKER << INI_FILE_MULLTY_LINE_VALUE_MARKER << INI_FILE_NEW_LINE_MARKER;
            }
            else
            {
                file << opt.key() << "=" << opt.value() << INI_FILE_NEW_LINE_MARKER;
            }
        }
        return true;
    }
    return false;
}

bool ConfigFile::update()
{
    if (this->is_modified)
    {
        this->write(this->dosini_fname);
        this->mark_updated();
        return true;
    }
    return false;
}

// helper class
class is_in_section
{
public:
    std::string section;
    explicit is_in_section(std::string const & sec)
        : section(sec + INI_FILE_SEC_OPT_SEP)
    {
    }
    bool operator() (const sss::ConfigFile::value_type & option)
    {
        return (option.first.substr(0, this->section.length()) == this->section);
    }
};

class is_not_in_section
{
public:
    std::string section;
    explicit is_not_in_section(std::string const & sec)
        : section(sec + INI_FILE_SEC_OPT_SEP)
    {
    }
    bool operator() (const sss::ConfigFile::value_type & option)
    {
        return (option.first.substr(0, this->section.length()) != this->section);
    }
};

ConfigFile::const_iterator ConfigFile::block_begin_const(std::string const& section)
{
    return const_iterator(std::find_if(this->content_.begin(), this->content_.end(), is_in_section(section)));
}

ConfigFile::const_iterator ConfigFile::block_end_const(std::string const& section)
{
    ConfigFile::iterator ini(std::find_if(this->content_.begin(), this->content_.end(), is_in_section(section)));
    return ConfigFile::const_iterator(this->block_end(section, ini).operator container_t::iterator());
}

ConfigFile::iterator ConfigFile::block_end(std::string const& section)
{
    ConfigFile::iterator ini(std::find_if(this->content_.begin(), this->content_.end(), is_in_section(section)));
    return ConfigFile::iterator(this->block_end(section, ini).operator container_t::iterator());
}

ConfigFile::iterator ConfigFile::block_end(std::string const& section, ConfigFile::iterator ini)
{
    if (ini.operator container_t::iterator() == this->content_.end())
        return ini; //ConfigFile::const_iterator(ini.operator container_t::iterator());
    return iterator(std::find_if(ini.operator container_t::iterator(), this->content_.end(), is_not_in_section(section)));
}

ConfigFile::iterator ConfigFile::block_begin(std::string const& section)
{
    return iterator(std::find_if(this->content_.begin(), this->content_.end(), is_in_section(section)));
}

//----------------------------------------------------------------------

const std::string& ConfigFile::value(std::string const& section, std::string const& entry) const throw(ConfigFile::Exception)
{
    container_t::const_iterator ci = this->content_.find(section + INI_FILE_SEC_OPT_SEP + entry);

    if (ci == content_.end())
        throw ConfigFile::Exception("["+ section + "]" + entry + " does not exist");

    return ci->second;
}

// fetch value or throw
std::string& ConfigFile::value(std::string const& section, std::string const& entry) throw(ConfigFile::Exception)
{
    container_t::iterator ci = this->content_.find(section + INI_FILE_SEC_OPT_SEP + entry);

    if (ci == content_.end())
        throw ConfigFile::Exception("["+ section + "]" + entry + " does not exist");

    return ci->second;
}

const std::string ConfigFile::value_no_throw(std::string const& section, std::string const& entry) const
{
    try {
        return this->value(section, entry);
    }
    catch (ConfigFile::Exception& e)
    {
        return "";
    }
}

std::string ConfigFile::value_no_throw(std::string const& section, std::string const& entry)
{
    try {
        return this->value(section, entry);
    }
    catch (ConfigFile::Exception& e)
    {
        return "";
    }
}

void ConfigFile::mark_modified()
{
    this->is_modified = true;
}

void ConfigFile::mark_updated()
{
    this->is_modified = false;
}

std::string const& ConfigFile::value(std::string const& section, std::string const& entry, std::string const& value)
{
    this->content_[section + INI_FILE_SEC_OPT_SEP + entry] = value;
    this->mark_modified();
    return value;
}

// TODO 删除值；
void ConfigFile::del_entry(std::string const& section, std::string const& entry)
{
    container_t::iterator ci = this->content_.find(section + INI_FILE_SEC_OPT_SEP + entry);
    if (ci != this->content_.end())
    {
        this->content_.erase(ci);
        this->mark_modified();
    }
}

// TODO 删除区；
void ConfigFile::del_section(std::string const& section)
{
    container_t::iterator ini = this->block_begin(section).operator container_t::iterator();
    if (ini != this->content_.end())
    {
        container_t::iterator fin = this->block_end(section).operator container_t::iterator();
        this->content_.erase(ini, fin);
        this->mark_modified();
    }
}

//----------------------------------------------------------------------
namespace utl {

ConfigEncValued::ConfigEncValued(sss::enc::EncBase& en, sss::ConfigFile& config)
    : enc(en),
      cfg(config)
{
}

ConfigEncValued::~ConfigEncValued()
{
}

std::string ConfigEncValued::get_value(const std::string& block,
                                       const std::string& name,
                                       const std::string& block_alias,
                                       const std::string& name_alias)
{
    //sss::enc::Hexblowfish hbf(this->magic_string);
    std::string value = this->cfg.value(block, name);
    if (value.length())
    {
        // 更新加密后的密码
        this->cfg.value(block_alias, name_alias, enc.encode(value));
        // 删除明文密码
        this->cfg.value(block, name, "");
        // 更新配置文件
        this->cfg.update();
    }
    else
    {
        value = enc.decode(this->cfg.value(block_alias, name_alias));
    }

    return value;
}

} // namespace utl;

} // namespace sss;

//----------------------------------------------------------------------

#ifdef CONFIGFILE_TEST
#include "ConfigFile.h"

#include <iostream>

int main()
{
    ConfigFile cf("config.txt");

    std::string foo;
    std::string water;
    double      four;

    foo   = cf.value("section_1", "foo"  );
    water = cf.value("section_2", "water");
    four  = cf.value("section_2", "four" );

    std::cout << foo   << INI_FILE_NEW_LINE_MARKER;
    std::cout << water << INI_FILE_NEW_LINE_MARKER;
    std::cout << four  << INI_FILE_NEW_LINE_MARKER;

    return 0;
}
/*
[section_1]
foo  = bar
water= h2o

[section_2]
foo  = foo
water= wet
four = 4.2
*/

#endif
