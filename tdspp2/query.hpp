#ifndef  __QUERY_HPP_1364391271__
#define  __QUERY_HPP_1364391271__

#include "Login.hpp"
#include "field.hpp"
#include "tdspp.hpp"

#include <sss/utlstring.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <iterator>

namespace sss{
namespace tdspp2 {

class DBLink;

/** Query class */
class Query
{
public:
    /** SQL command */
    std::string command;

    // * Rows */
    // 如果用户需要保存整个 result-set，需要自己处理

    // 如果用户需要提取数据项，需要传入 FieldList 指针！
    FieldList * current_row;
    //ResultSet * current_res;

    // 如何"规整"地打印结果集？
    // 需要找出结果集中，“显示”长度最长的一列，或者说保存。（当然，如果确认是
    // 用gbk来保存数据，那么，字节数，与显示长度是一致的。）
    //
    // 这意味着，需要将整个结果集，保存在内存里面……
    typedef std::vector<std::string> headers_t;
    headers_t headers;
    std::vector<int> types;
    std::vector<int> col_buf_len;
    std::vector<int> presizes;
    std::vector<int> scales;

    /** Number of fields per row. */
    int fieldcount;
    /** Number of rows in a result set. */
    int rowcount;

private:
    std::map<std::string, int> header_index;
    DBLink* TDS;

public:
    /** Constructor */
    explicit Query(DBLink* tds);
    /** Destructor */
    ~Query();

public:
    int  affect_count();
    int  do_execute(const std::string& sql);
    int  do_update(const std::string& sql);
    int  do_delete(const std::string& sql);

    // TODO 当发生错误、或者 受影响函数与需求不符时；回滚；
    bool  do_execute_rollback(const std::string& sql, int cnt);

    // 获取单个值
    template <typename T>
        int  do_select(const std::string& sql, T& val)
        {
            sss::tdspp2::FieldList fl;
            sss::tdspp2::FieldList * p_cur = this->current_row;
            this->sql(sql);
            this->bind(&fl);
            this->execute();
            bool has_stored = false;
            while (this->getresult()) {
                if (this->fetch()) {
                    val = sss::string_cast<T>(fl[0]->tostr());
                    has_stored = true;
                    break;
                }
            }
            this->bind(p_cur);
            return has_stored;
        }

    // 获取单列，多行
    // NOTE 2013-05-09 使用back_inserter
    template <typename T>
        int  do_select_multy(const std::string& sql, T it)
        {
            sss::tdspp2::FieldList fl;
            sss::tdspp2::FieldList * p_cur = this->current_row;
            this->sql(sql);
            this->bind(&fl);
            this->execute();
            int cnt = 0;
            if (this->getresult())
            {
                while (this->fetch())
                {
                    *it = sss::string_cast<typename std::iterator_traits<T>::value_type>(fl[0]->tostr());
                    ++it;
                    ++cnt;
                }
            }
            this->bind(p_cur);
            return cnt;
        }

    template <typename T>
        int  do_select_multy(const std::string& sql, T ini, T fin)
        {
            sss::tdspp2::FieldList fl;
            sss::tdspp2::FieldList * p_cur = this->current_row;
            this->sql(sql);
            this->bind(&fl);
            this->execute();
            int cnt = 0;
            if (this->getresult())
            {
                while (ini != fin && this->fetch())
                {
                    *ini = sss::string_cast<typename std::iterator_traits<T>::value_type>(fl[0]->tostr());
                    ++ini;
                    ++cnt;
                }
            }
            this->bind(p_cur);
            return cnt;
        }

    // "修改语义"
    // NOTE 2013-05-09
    //  先一句 q.sql("..."); 再附加一句 q.execute();
    //  这种方式不爽。最好是调用一个方法，就完成一句的执行；
    //  do_sql("...");
    //  并且，每次执行后，当前执行语句将清空；
    //  另外，可以考虑，sql语句的附加；
    void sql(const std::string& cmd);

    /** Execute an SQL query where a result is expected. */
    void execute();

    // 传入指针0，表示取消绑定。
    void bind(FieldList * fl);

    //void bind(ResultSet * rs);

    // clear buffer for new result set
    void clear();

    /** fetch one row */
    bool fetch();

    /** Iterate to next row in a result set. */
    // 定位到下一个结果集，有效的首"行"；
    // 如果有绑定 FieldList ，就可以通过Query对象，或
    // 者 FieldList 对象，获取需要的字段；
    // 返回值，表示可用的字段数
    // 为0，表示已经枚举完可用的结果集；
    int next(void);

    /** Go to the first row in a result set. */
    void first();
    /** Return the columnvalue i for the current row. */
    Field *fields(int i);
    /** Return the columnvalue s for the current row. */
    Field *fields(const std::string&);
    /** Check if we've reached the end of the result set. */
    bool eof();

    /** Overloaded [] operator. Used to specify fld name in a row. */
    std::string operator[](const char *);
    /** Overloaded [] operator. Used to specify fld name in a row. */
    std::string operator[](const std::string&);
    /** Overloaded [] operator. Used to specify index in a row. */
    std::string operator[](int );

    void printheader(std::ostream& o, const std::string& sep = "\t");

public:
    // 获取结果集的基本信息：
    //  表头、字段类型、数量等等
    //  如果设置buf，则将buf绑定到结果集的各个字段上
    bool getresult();

};

template<> inline int Query::do_select<sss::tdspp2::ResultSet>(const std::string& sql, sss::tdspp2::ResultSet& rs)
    {
        this->sql(sql);
        rs.load(*this);
        return rs.height();
    }

template<> inline int Query::do_select<sss::tdspp2::ResultSetList>(const std::string& sql, sss::tdspp2::ResultSetList& rs_list)
    {
        sss::tdspp2::ResultSet rs;
        this->sql(sql);
        this->execute();
        int size = rs_list.size();
        while (rs.load(*this, true) && rs.width()) {
            rs_list.push_back(rs);
        }
        return rs_list.size() - size;
    }


} // namespace tdspp2
} // namespace sss

#endif  /* __QUERY_HPP_1364391271__ */
