#ifndef  __FIELD_HPP_1364391291__
#define  __FIELD_HPP_1364391291__

#include <sss/utlstring.hpp>

#include <string>
#include <vector>

#include <iostream>

//#define SSS_TDSPP2_FIELD_USE_CPP

namespace sss{
namespace tdspp2{

bool sqltype_is_numeric(int type);

bool sqltype_is_binary(int type);

bool sqltype_is_datetime(int type);

bool sqltype_is_char(int type);

bool sqltype_is_bit(int type);

bool sqltype_is_text(int type);

/** Field class. AKA column. */
class Field // {{{1
{
    //friend class Query;

public:
    int         status; // is null or sth.

public:
    /** Constructor */
    Field();
    /** Constructor */
    explicit Field(int size);

    Field(char * byte_data, int len);

    Field(const Field& ref);
    /** Destructor */
    ~Field();

public:
    Field & operator=(const Field& ref);
    /** Return string value of a field. */
    std::string tostr(void);

    std::string tostr_trim(void) {
        return sss::trim_copy(this->tostr());
    }

    std::string tobinary(void);

    const char * c_str(void);

    int length();

    /** Return integer value of a field. */
    long        toint(void);
    /** Return double value of a field. */
    double      todouble(void);

    bool is_null() const
    {
        return this->status == -1;
    }

    bool is_valid() const
    {
        return this->status == -1 || this->status == 0;
    }

    char at(int i) const
    {
#ifdef SSS_TDSPP2_FIELD_USE_CPP
        return this->data.at(i);
#else
        return this->data[i];
#endif
    }

    size_t size() const
    {
#ifdef SSS_TDSPP2_FIELD_USE_CPP
        return this->data.size();
#else
        return this->bytelen;
#endif
    }

    void swap(Field& ref);

    char * get_buf();

    int    get_buflen();

    void shrink();

    void set_bytelen(int , char padding = ' ');

private:
    void assert_data_init();

    void assert_not_null();

private:
    // 到底是用 C++ 的 std::string 还是C 的char* 来管理字段内存呢？
    // dbbind本质上是C风格；bind之后，可以方便库用户，在同一个内存地址检索刚刚
    // fetch到的数据；缺点嘛，是可能的异常处理不太方便，以及类型转换等等不方便；
    //
    // std::string的话，麻烦也大；上面说了，dbbind是C风格的，只要bind后，它都会
    // 往同一个位置写数据；
    //
    // 但是，就算是对同一个字段，每次fetch，可能有效数据占用的空间都不一样大；
    //
    // 减小的时候，还好说；如果，遇到扩大（减小、扩大指的先后两次fetch、同一字
    // 段，占用空间大小关系）；
    //
    // std::string 自己是不知道有数据写入的；如果遇到扩大——比如，某字段长度是10；
    // 第一次，fetch到5个byte；第二次fetch到8个byte；
    // 那么，在resize()的时候，第六个byte，很可能被string内部函数，给覆盖成'\0'了！
    //
    // 再用c_str()等方式，检出的数据就不对了；
    //
    // 但是，IMAGE/TEXT等字段，最好用std::string来存放，因为std::cout输出的时候
    // ，它是看字节数，而不是靠检测'\0'来判断结束。
    //
    // 综合考虑的话，我觉得这样不错：
    //
    // 1. 即：行 buffer采用C风格管理；
    // 2. 结果集管理，采用C++ 风格——因为，这涉及到对象的内存管理；C++比较合适；
    //
    // 这样的话，就需要额外为buffer创建一个对象了；
    //
    // 要不这样，Field 还是采用C风格；不过多一个变量，用来描述长度？

#ifdef SSS_TDSPP2_FIELD_USE_CPP
    std::string   data;
#else
    char *        data;
    int           bytelen;      // 有效字节长度
#endif
    int           buflen;       // 记录最开始的大小需求；当然，也可以换用 std::string::capacity()
};

class FieldList : public std::vector<Field*> // {{{1
{
    friend class Query;

public:
    typedef std::vector<Field*>::iterator iterator;
    typedef std::vector<Field*>::const_iterator const_iterator;

    FieldList();
    FieldList(const FieldList& ref);
    ~FieldList();

public:
    FieldList& operator = (const FieldList& ref);

    void print(std::ostream& o, const std::string& sep = "\t") const;
    void clear();

    void erase(int);
};

inline std::ostream& operator << (std::ostream& o, FieldList & ref) // {{{1
{
    ref.print(o);
    return o;
}

/** 字段属性列表 */
class FieldInfo // {{{1
{
public:
    FieldInfo();
    explicit FieldInfo(const sss::tdspp2::Query& q);
    ~FieldInfo();

public:
    std::vector<std::string>    names;
    std::vector<int>            types;
    std::vector<int>            presizes;
    std::vector<int>            bytes;
    std::vector<int>            maxlens;

public:
    size_t  size() const;
    void assign(const sss::tdspp2::Query& q);

    void erase(int);
    void erase(const std::string& col_name);

public:
    void swap(FieldInfo& ref);
};

/** Row class */
class ResRows : public std::vector<FieldList> // {{{1
{
    friend class Query;

public:
    typedef std::vector<FieldList>::iterator iterator;
    typedef std::vector<FieldList>::const_iterator const_iterator;

    /** Constructor */
    ResRows();
    /** Destructor */
    ~ResRows();

public:
    /** Prints a row to stdout. */
    void print(std::ostream& o, const std::string& sep = "\t") const;

    void del_col(int col_id);

    void del_row(int row_id);

    //ResRows(const ResRows& ref);    // NOTE 使用默认的拷贝构造函数即可
    //ResRows& operator = (const ResRows& ref); // 使用默认的赋值函数即可
};

inline std::ostream& operator << (std::ostream& o, const ResRows& ref) // {{{1
{
    ref.print(o);
    return o;
}

class fdname_index_t : private std::map<std::string, int> // {{{1
{
public:
    fdname_index_t() {}

public:
    typedef std::map<std::string, int> father_t;
    using father_t::iterator;
    using father_t::clear;
    using father_t::const_iterator;
    using father_t::find;
    using father_t::begin;
    using father_t::end;
    using father_t::operator[];

    template <typename IT> fdname_index_t& assign(IT ini, IT fin) {
        fdname_index_t tmp;
        int i = 0;
        for (IT it = ini; it != fin; ++it, ++i) {
            tmp[sss::cast_string(*it)] = i;
        }
        tmp.swap(*this);
        return *this;
    }

    void swap(fdname_index_t& ref) {
        ((father_t&)*this).swap((father_t&)(ref));
    }
};

/** 结果集 */
class ResultSet // {{{1
{
private:
    FieldInfo   infos;                          // 字段属性列表
    ResRows     rows;                           // 结果集数据
    std::vector<ResultSet*> compute_sets;       // 计算列集合
    fdname_index_t fdname_index;

public:
    ResultSet();
    ~ResultSet();

public:
    // TODO 或许，应该在输出字段名的同时，还输出字段类型，长度等信息；
    //
    // 为了不增加空间，可以考虑简写；
    // 比如c(10)，表示char(10); s(100)，表示 varchar(100)
    // 其他：
    //  i => int
    //  d(14,4) => decimal(14,4)
    //  B       => blob
    //  b       => bit
    //  m       => image
    //  t       => text
    void pretty_print(std::ostream& o);

    void set_fieldinfo(const sss::tdspp2::Query& q);

public:
    // 按列名删除某列
    void del_col(const std::string& col_name);

    // 按列数删除某列
    void del_col(int col_id);

    // 删除某行
    void del_row(int row_id);

    // TODO 如果结果集之间，也可以使用集合运算，那世界……
    // res_out = res1.left_join(res2).on(res1_col1, res2_col1).on(res1_col2, res2_col2)
    //
    // 应该如何实现这种写法呢？
    // 利用延迟计算的办法？
    //
    // 让上述.left_join,.on动作，生成一个操作序列；
    // 再由操作序列，生成集合？
    //
    // 怎么样，难道还模拟主键？索引？——NOTE 所谓的主键，其实是索引的一种。主
    // 键是没有重复键的索引；
    // 如果将索引比作"排序的桶"，那么主键相当于每个桶，只能装一瓶酒。

public:

    void calc_fdname_index();

    bool has_field(const std::string& fdname) const;

    std::vector<std::string> & names();
    int  width() const;
    int  height() const;

    Field & get(int row, int col);
    Field & get(int row, const std::string& fdname);

    std::string& name(int col);
    int  presize(int col) const;
    int  type(int col) const;
    int  byte(int col) const;
    int  maxlen(int col) const;

    bool is_numeric(int col) const;
    bool is_binary(int col) const;
    bool is_datetime(int col) const;
    bool is_char(int col) const;
    bool is_bit(int col) const;
    bool is_text(int col) const;

    bool empty() const;
    // 载入一个结果集；
    // 如果，成功，返回true; 否则，返回false
    // NOTE 必须先调用 bind(FieldList&); 然后才能使用本函数获取结果集；
    // 不然，只能返回空的结果；
#ifdef OLD_ResultSet__load
    // 内部，封装了 Query::getresult();
    // 所以
    // while(res.load(q)) {
    //   即可
    // }
    bool load(sss::tdspp2::Query& q, bool donot_execute = false);
#else
    bool load(sss::tdspp2::Query& q);
#endif
    bool append(sss::tdspp2::FieldList& fl);
    bool fetch(sss::tdspp2::Query& q);
    void swap(ResultSet& ref);
};

/** "结果集"集合 */
class ResultSetList : public std::vector<ResultSet> // {{{1
{
};

} // namespace tdspp2
} // namespace sss

#endif  /* __FIELD_HPP_1364391291__ */
