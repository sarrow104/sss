#include "query.hpp"

#include <sqldb.h>

#include <sss/utlstring.hpp>
#include <sss/log.hpp>

#define SSS_COLOG_TURNOFF
#include <sss/colorlog.hpp>

#include "DBLink.hpp"
#include "tdspp.hpp"

namespace sss {
namespace tdspp2 {

namespace detail {

const char * get_sybtype_prt_name(int type)
{
    static std::map<int, const char*> sybtype_map;
    if (sybtype_map.empty()) {
        sybtype_map[47]  = "SYBCHAR";
        sybtype_map[39]  = "SYBVARCHAR";
        sybtype_map[38]  = "SYBINTN";
        sybtype_map[48]  = "SYBINT1";
        sybtype_map[52]  = "SYBINT2";
        sybtype_map[56]  = "SYBINT4";
        sybtype_map[127] = "SYBINT8";
        sybtype_map[62]  = "SYBFLT8";
        sybtype_map[61]  = "SYBDATETIME";
        sybtype_map[50]  = "SYBBIT";
        sybtype_map[104] = "SYBBITN";
        sybtype_map[35]  = "SYBTEXT";
        sybtype_map[99]  = "SYBNTEXT";
        sybtype_map[34]  = "SYBIMAGE";
        sybtype_map[122] = "SYBMONEY4";
        sybtype_map[60]  = "SYBMONEY";
        sybtype_map[58]  = "SYBDATETIME4";
        sybtype_map[59]  = "SYBREAL";
        sybtype_map[45]  = "SYBBINARY";
        sybtype_map[31]  = "SYBVOID";
        sybtype_map[37]  = "SYBVARBINARY";
        sybtype_map[108] = "SYBNUMERIC";
        sybtype_map[106] = "SYBDECIMAL";
        sybtype_map[109] = "SYBFLTN";
        sybtype_map[110] = "SYBMONEYN";
        sybtype_map[111] = "SYBDATETIMN";
        sybtype_map[103] = "SYBNVARCHAR";
        sybtype_map[49]  = "SYBDATE";
        sybtype_map[51]  = "SYBTIME";
        sybtype_map[187] = "SYBBIGDATETIME";
        sybtype_map[188] = "SYBBIGTIME";
        sybtype_map[40]  = "SYBMSDATE";
        sybtype_map[41]  = "SYBMSTIME";
        sybtype_map[42]  = "SYBMSDATETIME2";
        sybtype_map[43]  = "SYBMSDATETIMEOFFSET";
    }
    auto it = sybtype_map.find(type);
    if (it != sybtype_map.end()) {
        return it->second;
    }
    else {
        return "NULL";
    }
}

} // namespace detail

Query::Query(DBLink* tds)
    : TDS(tds)
{
}

Query::~Query()
{
    // 2013-04-18
    // this->bind(0);
}

void Query::sql(const std::string& cmd)
{
    this->command = cmd;
}

// 多"结果集"如何处理？

void Query::execute(void)
{
    //! 功能：清空上次查询得到的数据集。
    //如果是一个句柄的话，每次重新执行select语句之前都要调用它清空结果，不然数据库会报错的。
    COLOG_DEBUG_STMT(dbcancel(this->TDS->dbprocess));

    // 获取受语句影响的行数 - update,delete 类语句，很有效。比如，某些语句我只想
    // 影响的行数为1；就可以用begin trans,commit trans包裹该语句。通过检查
    // dbcount是否为1，来决定是否commit。
    //
    // COLOG_DEBUG_STMT(dbcount(this->TDS->dbprocess));

    // 清空命令缓冲区 NOTE 其实dbcmd 是可以反复执行，逐行添加语句的。
    COLOG_DEBUG_STMT(dbfreebuf(this->TDS->dbprocess));

    // NOTE 通过dbcmd()，传递给服务器的脚本字符串，可以通过：
    // dbstrcpy(), dbgetchar(), dbstrlen()等函数，按照需要地取回来。
    COLOG_DEBUG_STMT(dbcmd(this->TDS->dbprocess, const_cast<char*>(this->command.c_str())));
    SSS_LOG_DEBUG("dbcmd(%p, \"%s\");\n",
                  this->TDS->dbprocess,
                  this->command.c_str());
    SSS_LOG_DEBUG("dbsqlexec(%p);\n", this->TDS->dbprocess);
    if (COLOG_DEBUG_STMT(dbsqlexec(this->TDS->dbprocess)) == FAIL)
    {
        throw Exception("dbsqlexec error(" + this->command + ")");
    }

    // FIXME 2017-08-20
    // 这里应该用状态机，来处理结果集！
    // 而不是越俎代庖！
    // 如果用户没有绑定"行buffer"，则代替用户完成循环
//     if (!this->current_row)
//     {
//         // NOTE 直接dbcancel 对于耗时的操作好像会导致执行取消？
// #if 0
//         COLOG_DEBUG_STMT(dbcancel(this->TDS->dbprocess));
// #else
//         while (this->getresult())
//             while (this->fetch());
// #endif
//     }
}


int  Query::affect_count()
{
    return COLOG_DEBUG_STMT(dbcount(this->TDS->dbprocess));
}

void Query::clear(void) {
    // if (this->current_row)
    //     this->current_row->clear();
    this->fl.clear();

    this->col_buf_len.clear();
    this->headers.clear();
    this->types.clear();
    this->presizes.clear();
    this->scales.clear();
}

// NOTE 包装::sql(), ::execute(); 返回受影响的行数
int  Query::do_update(const std::string& sql)
{
    return do_execute(sql);
}

// NOTE 包装::sql(), ::execute(); 返回受影响的行数
int  Query::do_delete(const std::string& sql)
{
    return do_execute(sql);
}

// NOTE 包装::sql(), ::execute(); 返回受影响的行数
int  Query::do_execute(const std::string& sql)
{
    // sss::tdspp2::FieldList * p_cur = this->current_row;
    this->sql(sql);
    // this->bind(0);
    this->execute();
    while (this->getresult());
    // this->bind(p_cur);
    return this->affect_count();
}

// TODO freetds api 并没有直接支持 transaction 动作；
// 于是……
bool Query::do_execute_rollback(const std::string& sql, int cnt)
{
    // FIXME 如何支持事务嵌套？
    this->do_execute("begin tran");
    int row_count = this->do_execute(sql);
    if (row_count != cnt) {
        this->do_execute("rollback tran");
        return false;
    }
    else {
        this->do_execute("commit tran");
        return true;
    }
    return false;
}

// 绑定缓冲行
// NOTE TODO
//      bind的效果，应该有作用范围——即，当this对象被销毁之后，自动bind到之前
//      一个值，或者干脆取消绑定！
// 因为，FieldList fl对象的作用域是不明的。
// 有可能存在fl对象已经被销毁了，但是current_row 指针还是原来的值；于是就产生非法访问！
// void Query::bind(FieldList * fl)
// {
//     SSS_LOG_DEBUG("Query::bind(%p);\n", fl);
//     this->current_row = fl;
// }

// TODO
// 绑定HeaderList
// void Query::bind_header(..);
//
// TODO
// 绑定结果集 —— 相当于一次性获取 HeaderList 以及 行集
// Query 会根据这几个指针的绑定情况，以决定如何保存各种数据。
//

bool Query::getresult(void)
{
    SSS_LOG_DEBUG("dbresults(%p);\n", this->TDS->dbprocess);
    int result_code = COLOG_DEBUG_STMT(dbresults(this->TDS->dbprocess));

    switch (result_code)
    {
        case NO_MORE_RESULTS:
            this->clear();
            return false;

        case SUCCEED:
            if (true)
            {
                this->clear();

                this->fieldcount = COLOG_DEBUG_STMT(dbnumcols(this->TDS->dbprocess));
                SSS_LOG_DEBUG("%d = dbnumcols(%p);\n",
                              this->fieldcount,
                              this->TDS->dbprocess);

                // NOTE headers 的获取，应该与是否 bind FieldList 无关。
                for (int i = 0; i < this->fieldcount; ++i)
                {
                    // FIXME 没有字段名，应该如何处理？
                    // 2013-04-21
                    // 方便通过字段名，访问字段值
                    // 如果遇到字段名重复，则取最后一个
                    header_index[COLOG_DEBUG_STMT(dbcolname(this->TDS->dbprocess, i + 1))] = i;

                    headers.push_back(COLOG_DEBUG_STMT(dbcolname(this->TDS->dbprocess, i + 1)));
                    SSS_LOG_DEBUG("%s = dbcolname(%p, %d);\n",
                                  headers.back().c_str(),
                                  this->TDS->dbprocess,
                                  i + 1);
                    int type = COLOG_DEBUG_STMT(dbcoltype(this->TDS->dbprocess, i + 1));
                    if (!type) {
                        // int dbcoltype (DBPROCESS * dbproc,
                        //                int     column)
                        //
                        // Get the datatype of a regular result set column.
                        //
                        // Parameters:
                        //     dbproc   contains all information needed by db-lib to manage communications with the server.
                        //     column   Nth in the result set, starting from 1.
                        //
                        // Returns:
                        //     SYB* datetype token value, or zero if column out of range
                        //
                        // See also:
                        //     dbcollen(), dbcolname(), dbdata(), dbdatlen(), dbnumcols(), dbprtype(), dbvarylen().
                        //
                    }
                    types.push_back(type);
                    // const char * ft_name =dbprtype(this->types.back());
                    // SSS_LOG_DEBUG("%d type: %s\n", ft_name ? ft_name : "NULL");

                    // FIXME
                    // dbcoltype 不管SQL语句是 char，还是vartype，返回的都是 SQLCHAR 类型！
                    // 但是，sql在处理他们的后缀空白符的时候，又是不一样的。那么，应该如何区分他们呢？
                    //switch (this->types.back()) {
                    //case SQLCHAR     :   printf("Char %s\n", dbprtype(this->types.back()));
                    //                     printf("Varchar %s\n", dbprtype(this->types.back())); break;
                    //case SQLBINARY   :   printf("Binary %s\n", dbprtype(this->types.back()));
                    //                     printf("Varbinary %s\n", dbprtype(this->types.back())); break;
                    //case SQLINT1     :   printf("Tinyint %s\n", dbprtype(this->types.back())); break;
                    //case SQLINT2     :   printf("Smallint %s\n", dbprtype(this->types.back())); break;
                    //case SQLINT4     :   printf("Int %s\n", dbprtype(this->types.back())); break;
                    //case SQLFLT4     :   printf("Real %s\n", dbprtype(this->types.back())); break;
                    //case SQLFLT8     :   printf("Float %s\n", dbprtype(this->types.back())); break;
                    //case SQLMONEY4   :   printf("Smallmoney %s\n", dbprtype(this->types.back())); break;
                    //case SQLMONEY    :   printf("money %s\n", dbprtype(this->types.back())); break;
                    //case SQLDECIMAL  :   printf("decimal %s\n", dbprtype(this->types.back())); break;
                    //case SQLNUMERIC  :   printf("numeric %s\n", dbprtype(this->types.back())); break;
                    //case SQLDATETIM4 :   printf("smalldatetime %s\n", dbprtype(this->types.back())); break;
                    //case SQLDATETIME :   printf("datetime %s\n", dbprtype(this->types.back())); break;
                    //case SQLIMAGE    :   printf("image %s\n", dbprtype(this->types.back())); break;
                    //case SQLTEXT     :   printf("text %s\n", dbprtype(this->types.back())); break;
                    //}

                    DBTYPEINFO * p_typeinfo = COLOG_DEBUG_STMT(dbcoltypeinfo(this->TDS->dbprocess, i + 1));
                    // NOTE p_typeinfo 每次得到的值，都是一个固定的，说明，这个是来
                    // 自栈的，而不是new来的。
                    // 所以，虽然获取上面的值，没有重入性，但是 live with it!
                    presizes.push_back(p_typeinfo->precision);
                    scales.push_back(p_typeinfo->scale);
                    //printf("%d %d\n", presizes.back(), scales.back());
                    SSS_LOG_DEBUG("%p = dbcoltypeinfo(%p, %d);\n",
                                  p_typeinfo,
                                  this->TDS->dbprocess,
                                  i + 1);

                    //DBTYPEINFO *  dbcoltypeinfo (DBPROCESS *dbproc, int column)
                    //      Get precision and scale information for a regular result column.
                    SSS_LOG_DEBUG("%d = dbcoltype(%p, %d);\n",
                                  types.back(),
                                  this->TDS->dbprocess,
                                  i + 1);

                    // convert all to char!
                    switch (this->types[i]) {
                        case SYBCHAR:
                            col_buf_len.push_back(COLOG_DEBUG_STMT(dbcollen(this->TDS->dbprocess, i + 1)));
                            SSS_LOG_DEBUG("%d = dbcollen(%p, %d);\n",
                                          col_buf_len.back(),
                                          this->TDS->dbprocess,
                                          i + 1);
                            // col data in native byte length
                            break;

                        case SYBBINARY:
                        case SYBIMAGE:
                        case SYBTEXT:
                        case SYBNTEXT:
                        case SYBVARBINARY:
                            col_buf_len.push_back(256);
                            break;

                        default:
#if 0
                            // NOTE 这里的长度，怎么和列无关呢？
                            col_buf_len.push_back(dbwillconvert(types[i], SYBCHAR));
                            SSS_LOG_DEBUG("%d = dbwillconvert(%s, %s);\n",
                                          col_buf_len.back(),
                                          sss::tdspp2::detail::get_sybtype_prt_name(types[i]),
                                          sss::tdspp2::detail::get_sybtype_prt_name(SYBCHAR));
#else
                            col_buf_len.push_back(std::max(32,
                                                           2 * COLOG_DEBUG_STMT(dbdatlen(this->TDS->dbprocess, i + 1))) + 2);
                            // convert to char
#endif
                    }

                    switch (this->types[i]) {
                        case SYBBINARY:
                        case SYBIMAGE:
                        case SYBTEXT:
                        case SYBNTEXT:
                        case SYBVARBINARY:
                            this->fl.push_back(new Field);
                            break;

                        default:
                            this->fl.push_back(new Field(col_buf_len[i]));
                            break;
                    }

                    SSS_LOG_DEBUG("(*this->current_row).push_back(%p)\n",
                                  this->fl.back());

                    SSS_LOG_DEBUG("dbbind(%p, %d, %d, %d, %p);\n",
                                  this->TDS->dbprocess,
                                  i + 1,
                                  NTBSTRINGBIND, this->fl[i]->get_buflen() + 1,
                                  this->fl[i]->get_buf());

                    switch (this->types[i]) {
                        case SYBBINARY:
                        case SYBIMAGE:
                        case SYBTEXT:
                        case SYBNTEXT:
                        case SYBVARBINARY:
                            break;

                        default:
                            // NOTE TEXT/IMAGE 列，不能 dbbind() ！因为列所需要的宽度不一！而
                            // 其他列，都有一个最大宽度；所以，对于数据库来说，增加一列的话
                            // ，相当方便——主要是方便利用便宜检索数据；
                            // 而 TEXT/IMAGE这种占用空间变化很大的列来说，不能和这些一般类型
                            // 的列存放在一起，只能单独存放；所以，这些列值的获取，只能延迟
                            // 到 dbnextrow()；然后分别用
                            //      dbdata(dbproc, c+1),
                            //      dbdatlen(dbproc, c+1);
                            // 获取该行结果集，该字段的值（返回的是本地指针，应该是从服务器
                            // 拷贝了一份到本地）；
                            //
                            // 当然，也不是说，不能使用 dbbind() 只不过这种方式，用户只能获
                            // 取定长数据——即，前面256个字节；
                            if (COLOG_DEBUG_STMT(dbbind(this->TDS->dbprocess, i + 1,
                                                        NTBSTRINGBIND, this->fl[i]->get_buflen() + 1,
                                                        reinterpret_cast<BYTE*>(this->fl[i]->get_buf()))) == FAIL)
                            {
                                throw Exception("dbbind error, the column number given isn't valid, "
                                                "or the vartype isn't compatible with the sqlserver "
                                                "data type being returned, or varaddr is null.");
                            }
                            // FIXME dbnullbind ，以处理 NULL 列的情况！
                            //
                            // 0 column bound successfully
                            // -1 column is NULL.
                            // >0 true length of data, had column not been truncated due to
                            // insufficient space in the columns bound host variable .
                            SSS_LOG_DEBUG("dbnullbind(%p, %d, %p);\n",
                                          this->TDS->dbprocess,
                                          i + 1,
                                          &this->fl[i]->status);
                            if (COLOG_DEBUG_STMT(dbnullbind(this->TDS->dbprocess, i + 1, &this->fl[i]->status)) == FAIL)
                            {
                                throw Exception("dbnullbind() faild at col.");
                            }
                    }
                }
            }
            return true;
    } // end of switch (result_code)
    return false;
}

// return
// true:
//      读取成功
// false:
//      没有数据
bool Query::fetch()
{
    //if (!this->current_row)
    //    return false;

    int row_code = COLOG_DEBUG_STMT(::dbnextrow(this->TDS->dbprocess));
    SSS_LOG_DEBUG("%d = dbnextrow(%p)\n", row_code, this->TDS->dbprocess);

    switch (row_code)
    {
        case NO_MORE_ROWS:
            SSS_LOG_DEBUG("NO_MORE_ROWS : %d\n", NO_MORE_ROWS);
            this->clear();
            return false;

        case REG_ROW:
            if (true)
            {
                //for (int i = 0; i < int(this->types.size()); ++i) {
                for (int i = 0; i < int(this->headers.size()); ++i) {
                    SSS_LOG_DEBUG("%s@%d : len = %d; datlen = %d\n",
                                  dbprtype(this->types[i]),
                                  i + 1,
                                  dbcollen(this->TDS->dbprocess, i + 1),
                                  dbdatlen(this->TDS->dbprocess, i + 1));
                    switch (this->types[i]) {
                        case SYBBINARY:
                        case SYBIMAGE:
                        case SYBTEXT:
                        case SYBNTEXT:
                        case SYBVARBINARY:
                            {
                                (*this->fl.at(i))= sss::tdspp2::Field(reinterpret_cast<char*>(COLOG_DEBUG_STMT(dbdata(this->TDS->dbprocess, i + 1))),
                                                                          COLOG_DEBUG_STMT(dbdatlen(this->TDS->dbprocess, i + 1)));
                                //std::cout << current_row->at(i)->tostr() << std::endl;
                            }
                            break;

                        case SYBCHAR:
                        case SYBVARCHAR:
                            this->fl.at(i)->set_bytelen(COLOG_DEBUG_STMT(dbdatlen(this->TDS->dbprocess, i + 1)), ' ');
                            break;

                        default:
                            // NOTE
                            // 因为dbbind 的时候，使用的是 dbbind NTBSTRINGBIND ;
                            // 而写入的时候，Field.data 并不知道长度（用的是二进制），所以需要在后面，告知其真实长度？
                            this->fl.at(i)->shrink();
                            break;
                    }
                }
                std::ostringstream oss;
                oss << this->fl;
                SSS_LOG_DEBUG("%p: %s\n", &this->fl, oss.str().c_str());
            }
            SSS_LOG_DEBUG("REG_ROW : %d\n", REG_ROW);
            return true;

        case BUF_FULL:
            SSS_LOG_DEBUG("BUF_FULL : %d\n", BUF_FULL);
            throw Exception("buffer not sufficient.");
            break;

        case FAIL:
            dbcancel(this->TDS->dbprocess);
            throw Exception("dbnextrow failed");
            break;

        default: // NOTE COMPUTE row id
            throw Exception("Data for computeid "
                            + sss::cast_string<int>(row_code)
                            + " ignored.");
            break;
        }
        return false;
    }

    bool Query::fetch(sss::tdspp2::FieldList& r_fl)
    {
        bool is_ok = this->fetch();
        if (is_ok) {
            r_fl = this->fl;
        }
        return is_ok;
    }

    bool Query::fetch(sss::tdspp2::ResultSet& rs)
    {
        return rs.load(*this);
    }

    int Query::next(void)
    {
        while (this->getresult())
        {
            if (this->fetch())
            {
                int col_count = this->headers.size();
                if (!col_count)
                    continue;
                else
                    return col_count;
            }
        }
        return 0;
    }

    Field* Query::fields(int i)
    {
        if (0 <= i && size_t(i) < this->fl.size()) {
            return this->fl[i];
        }
        return 0;
    }

    Field* Query::fields(const std::string& fld)
    {
        std::map<std::string, int>::iterator it = this->header_index.find(fld);
        if (it != this->header_index.end())
        {
            return this->fl[it->second];
        }
        return 0;
    }

    // TODO
    void Query::first(void)
    {
        // 游标定位到结果集首行
        // NOTE 通过 dbgetrow，可以将游标定位到当前结果集的任意一行——只要不超过缓冲区行数大小；
        // FreeTDS 会将结果集暂时存放在缓冲区里面；缓冲区通过
        // dbsetopt(dbproc, DBBUFFER, buflen);
        //  设置缓冲区可以缓存的最大行数
        // dbclrbuf()
        //  清空当前缓冲区。
        // 等函数管理。
        //
        // 获取具体某行、某列数据，通过dbgetrow,dbdata来获取。
        // if(dbgetrow(dbproc, num) == REG_ROW)
        // {
        //   //将该行的每一列的数据指针、长度信息存入对应的result数组的对应项中。
        //   for(int i=0; i< m_Cols ; i++)
        //   {
        //     //第i+1列的数据指针
        //     result[i].p = dbdata(dbproc, i+1);
        //     //第i+1列的数据的长度
        //     result[i].n = dbdatlen(dbproc, i+1);
        //   }
        //   return result;
        // }
    }

    // TODO
    bool Query::eof()
    {
        // 游标定位到结果集末行
        return false;
    }

    // FIXME
    std::string Query::operator[] (const char *s)
    {
        return this->fields(s)->tostr();
    }

    // FIXME
    std::string Query::operator[] (const std::string& fld)
    {
        return this->fields(fld)->tostr();
    }


    std::string Query::operator[](int index)
    {
        return this->fields(index)->tostr();
    }

    void Query::printheader(std::ostream& o, const std::string& sep)
    {
        for (size_t i = 0; i < this->headers.size(); ++i)
        {
            if (i == 0)
            {
                o << this->headers[i];
            }
            else
            {
                o << sep << this->headers[i];
            }
        }
    }

} // namespace tdspp2
} // namespace sss
