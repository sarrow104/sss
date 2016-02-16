/*
 * $Id: field.cc,v 2.7 2005/10/20 11:24:12 martin Exp $
 * Copyright (c) 2004, 2005, Voidsoft AB
 */

#include <sss/log.hpp>

#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include "tdspp.hpp"

namespace sss{
namespace tdspp2{

//! type check {{{1
bool sqltype_is_numeric(int type)       // {{{2
{
    switch (type) {
    case SYBINTN:
    case SYBINT1:
    case SYBINT2:
    case SYBINT4:
    case SYBINT8:
    case SYBFLT8:
    case SYBMONEY4:
    case SYBMONEY:
    case SYBMONEYN:
    case SYBREAL:
    case SYBBINARY:
    case SYBNUMERIC:
    case SYBDECIMAL:
    case SYBFLTN:
        return true;

    default:
        return false;
    }
}

bool sqltype_is_binary(int type)        // {{{2
{
    switch (type) {
    case SYBIMAGE:
    case SYBVOID:
    case SYBVARBINARY:
        return true;

    default:
        return false;
    }
}

bool sqltype_is_datetime(int type)      // {{{2
{
    switch (type) {
    case SYBDATETIME:
    case SYBDATETIME4:
    case SYBDATETIMN:
        return true;

    default:
        return false;
    }
}

bool sqltype_is_char(int type)  // {{{2
{
    switch (type) {
    case SYBCHAR:
    case SYBVARCHAR:
    case SYBNVARCHAR:
        return true;

    default:
        return false;
    }
}

bool sqltype_is_bit(int type)   // {{{2
{
    switch (type) {
    case SYBBIT:
    case SYBBITN:
        return true;

    default:
        return false;
    }
}

bool sqltype_is_text(int type)  // {{{2
{
    switch (type) {
    case SYBTEXT:
    case SYBNTEXT:
        return true;

    default:
        return false;
    }
}


/** Class Field */      // {{{1
Field::Field()          // {{{2
    : status(-1), // status == -1 means NULL
#ifdef SSS_TDSPP2_FIELD_USE_CPP
#else
      data(0),
      bytelen(0),
#endif
      buflen(0)
{
    SSS_LOG_DEBUG("Field::Field() at %p\n", this);
}

Field::Field(int size)          // {{{2
    : status(-1),
#ifdef SSS_TDSPP2_FIELD_USE_CPP
#else
      data(new char[size + 1]),
      bytelen(0),
#endif
      buflen(size)
{
    SSS_LOG_DEBUG("Field::Field(%d) at %p\n", size, this);
#ifdef SSS_TDSPP2_FIELD_USE_CPP
    this->data.resize(buflen, '\0');
#else
#endif
}

Field::Field(char * byte_data, int len)         // {{{2
    : status(-1),
#ifdef SSS_TDSPP2_FIELD_USE_CPP
#else
      data(0),
      bytelen(0),
#endif
      buflen(len)
{
    SSS_LOG_DEBUG("Field::Field(char %p, %d) at %p\n", byte_data, len, this);
    if (len > 0) {
#ifdef SSS_TDSPP2_FIELD_USE_CPP
        this->data.reserve(len);
        this->data.assign(byte_data, byte_data + len);
#else
        this->data = new char[len + 1];
        std::memcpy(this->data, byte_data, len);
        this->bytelen = len;
#endif
        this->status = 0;
    }
}

Field::Field(const Field& ref)          // {{{2
    : status(ref.status),
#ifdef SSS_TDSPP2_FIELD_USE_CPP
#else
      data(0),
      bytelen(0),
#endif
      buflen(ref.buflen)
{
    SSS_LOG_DEBUG("Field::Field(const Field& %p) at %p; ref.is_null() = %d; ref.is_valid() = %d\n",
                  &ref, this,
                  ref.is_null(), ref.is_valid());
#ifdef SSS_TDSPP2_FIELD_USE_CPP
    // 强制备份，而不是共享内存
    this->data.resize(ref.data.length());
    this->data.assign(ref.data.begin(), ref.data.end());
#else
    if (ref.data) {
        this->data = new char[this->buflen + 1];
        std::memcpy(this->data, ref.data, ref.bytelen + 1);
    }
    this->bytelen = ref.bytelen;
#endif
}

Field::~Field()                 // {{{2
{
    SSS_LOG_DEBUG("Field::~Field() at %p\n", this);
#ifdef SSS_TDSPP2_FIELD_USE_CPP
#else
    delete[] this->data;
    this->bytelen = 0;
#endif
    this->status = -1;
}

void Field::swap(Field& ref)    // {{{2
{
    SSS_LOG_DEBUG("Field::swap(Field & %p) at %p\n", &ref, this);
#ifdef SSS_TDSPP2_FIELD_USE_CPP
    std::swap(status, ref.status);
    std::swap(data,   ref.data);
    std::swap(buflen, ref.buflen);
#else
    std::swap(status, ref.status);
    std::swap(data,   ref.data);
    std::swap(bytelen, ref.bytelen);
    std::swap(buflen, ref.buflen);
#endif
}

Field & Field::operator=(const Field& ref)                      // {{{2
{
    SSS_LOG_DEBUG("Field::operator=(const Field& %p) at %p\n", &ref, this);
    if (this != &ref)
    {
        Field tmp(ref);
        this->swap(tmp);
    }
    return *this;
}

char * Field::get_buf()         // {{{2
{
    return &this->data[0];
}

int    Field::get_buflen()      // {{{2
{
    return this->buflen;
}

void Field::set_bytelen(int len, char padding)    // {{{{2
{
#ifdef SSS_TDSPP2_FIELD_USE_CPP
    this->shrink();
    this->data.resize(len, padding);
#else
    for (bytelen = std::strlen(this->data); bytelen < len; bytelen++ ) {
        this->data[bytelen] = padding;
    }
#endif
}

void Field::shrink()            // {{{2
{
    SSS_LOG_DEBUG("Field::shrink() at %p\n", this);

#ifdef SSS_TDSPP2_FIELD_USE_CPP
    //this->data.assign(this->data.c_str(), std::strlen(this->data.c_str()));
    this->data.resize(std::strlen(this->data.c_str()));
#else
    bytelen = std::strlen(this->data);
    //if (bytelen != std::strlen(this->data))
    //{
    //    SSS_LOG_ERROR("bytelen(%d), strlen(%d)\n", bytelen, std::strlen(this->data));
    //}
#endif
}

void Field::assert_data_init()  // {{{2
{
#ifdef SSS_TDSPP2_FIELD_USE_CPP
#else
    if (!data)
    {
        char buffer[10];
        sprintf(buffer, "%p", this);

        SSS_LOG_ERROR("Field::assert_data_init() faild at %p : Data not initialized.\n", this);
        throw Exception(std::string("Field (") + buffer + "): Data not initialized");
    }
#endif
}

void Field::assert_not_null()   // {{{2
{
    if (this->is_null())
        throw Exception("NULL field.");
}

std::string Field::tostr()      // {{{2
{
    try {
        SSS_LOG_DEBUG("Field::tostr() at %p\n", this);
        if (this->is_null())
            return "NULL";
        this->assert_data_init();
#ifdef SSS_TDSPP2_FIELD_USE_CPP
        return this->data;
#else
        return std::string(this->data, this->data + this->bytelen);
#endif
    }
    catch (...) {
        SSS_LOG_ERROR("Exception at %p\n", this);
        throw;
    }
}

std::string Field::tobinary(void)
{
    if (this->is_null()) {
        return "NULL";
    }
    if (!this->size()) {
        return "";
    }
    else {
        std::ostringstream oss;
        oss << "0x";
        for (size_t i = 0; i != this->size(); ++i ) {
            oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(this->at(i) & 0x00ffu);
        }
        return oss.str();
    }
}

const char * Field::c_str(void)	// {{{2
{
    SSS_LOG_DEBUG("Field::%s() at %p\n", __func__, this);
    if (this->is_null())
        return "NULL";
    this->assert_data_init();
#ifdef SSS_TDSPP2_FIELD_USE_CPP
    return this->data.c_str();
#else
    return this->data;
#endif
}

// NOTE FIXME
// 关于 tostr(), tostr_trim(), length()
// tdspp2 之前的一个稳定版的特点是 tostr() 只返回有效字符——这与 sql 的直觉不是
// 很一直；sql 直觉，char(10) 是按10字节返回，而varchar(10)则忽略后面的空白符；
// sql len(char_fd) 则会忽略后面的空白符。
//
// 那么，我这里 Field::length()，应该如何返回？
//
// 一种设计是，按照最简存储，然后按需要附加空白符再返回。
// 还有一种，就是保留空白符，但是计算长度的时候忽略末尾空白符。
//
// 还有呢，就是是提供字段类型，以方便其判断。
// 另外需要注意的是，除了二进制数据，其他数据类型都是转化为字符再保存的。

int Field::length()	// {{{2
{
    SSS_LOG_DEBUG("Field::%s() at %p\n", __func__, this);
    if (this->is_null())
        return 0;
    this->assert_data_init();
#ifdef SSS_TDSPP2_FIELD_USE_CPP
    return this->data.length();
#else
    return this->bytelen;
#endif
}

/* FIXME */
long Field::toint()	// {{{2
{
    this->assert_not_null();
    this->assert_data_init();
#ifdef SSS_TDSPP2_FIELD_USE_CPP
    return atoi(this->data.c_str());
#else

#if 1
    this->data[bytelen] = '\0';
    return atoi(this->data);
#else
    int ret2 = sss::string_cast<int>(this->data);
    int ret3 = sss::string_cast<int>(this->tostr());
    if (ret1 != ret2 || ret1 != ret3) {
        SSS_LOG_ERROR("ret1(%d), ret2(%d), ret3(%d)\n", ret1, ret2, ret3);
        SSS_LOG_ERROR("bytelen(%d), strlen(%d)\n", this->bytelen, std::strlen(this->data));
    }
    return ret3;
#endif
    //return atoi(this->data);
#endif
}

/* FIXME */
double Field::todouble()        // {{{2
{
    this->assert_not_null();
    this->assert_data_init();
#ifdef SSS_TDSPP2_FIELD_USE_CPP
    return atof(this->data.c_str());
#else
    return atof(this->data);
#endif
}

/** class FieldList */ // {{{1
FieldList::FieldList()  // {{{2
{
    SSS_LOG_DEBUG("FieldList::FieldList() at %p\n", this);
}

FieldList::FieldList(const FieldList& ref) : std::vector<Field*>()      // {{{2
{
    SSS_LOG_DEBUG("FieldList::FieldList(const FieldList& %p) at %p\n", &ref, this);
    this->reserve(ref.size());
    for (int i = 0; i < int(ref.size()); ++i)
    {
        this->push_back(new Field(*ref[i]));
    }
}

FieldList::~FieldList() // {{{2
{
    SSS_LOG_DEBUG("FieldList::~FieldList() at %p\n", this);
    this->clear();
}

//template<typename T> class AssignableByCopy
//{
//public:
//    T& operator = (const T& ref)
//    {
//        if (this != &ref)
//        {
//            T tmp(ref);
//            std::swap(*this, tmp);
//        }
//        return *this;
//    }
//};
//
// NOTE usage:
// class MyClass : public AssignableByCopy<MyClass>
// {
// };

FieldList& FieldList::operator = (const FieldList& ref) // {{{2
{
    if (this != &ref)
    {
        FieldList tmp(ref);
        std::swap(*this, tmp);
    }
    return *this;
}

void FieldList::clear() // {{{2
{
    SSS_LOG_DEBUG("FieldList::%s() at %p\n", __func__, this);
    while (this->size())
    {
        delete this->back();
        this->pop_back();
    }
}

void FieldList::print(std::ostream& o, const std::string& sep) const    // {{{2
{
    for (const_iterator it = this->begin(); it != this->end(); ++it)
    {
        if (it != this->begin())
        {
            o << sep;
        }
        o << (*it)->tostr();
    }
}

void FieldList::erase(int col_id)       // {{{2
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    if ( col_id >= 0 && col_id < int(this->size()) ) {
        iterator it = this->begin();
        std::advance(it, col_id);
        delete *it;
        std::vector<Field*>::erase(it);
    }
}

//! FieldInfo {{{1

FieldInfo::FieldInfo()
{
}

FieldInfo::FieldInfo(const sss::tdspp2::Query& q)
{
    this->names    = q.headers;
    this->types    = q.types;
    this->bytes    = q.col_buf_len;
    this->presizes = q.presizes;

    this->maxlens.reserve(q.headers.size());
    // NOTE 我觉得，不应该记录head的长度
    for (size_t i = 0; i != q.headers.size(); ++i) {
        //this->maxlens.push_back(q.headers[i].length());
        this->maxlens.push_back(0);
    }
}

FieldInfo::~FieldInfo()
{
}

void FieldInfo::assign(const sss::tdspp2::Query& q)
{
    FieldInfo tmp_info(q);
    this->swap(tmp_info);
}


size_t  FieldInfo::size() const
{
    size_t len = this->names.size();
    assert(len == this->types.size());
    assert(len == this->presizes.size());
    assert(len == this->bytes.size());
    assert(len == this->maxlens.size());
    return len;
}

void FieldInfo::erase(int pos)  // {{{2
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    if (pos >= 0 && pos < int(this->names.size())) {
        std::vector<std::string>::iterator is_iter = names.begin();
        std::advance(is_iter, pos);
        this->names.erase(is_iter);

        SSS_LOG_DEBUG("after names\n");

        std::vector<int>::iterator it_iter;

        it_iter = types.begin();
        std::advance(it_iter, pos);
        this->types.erase(it_iter);

        SSS_LOG_DEBUG("after types\n");

        it_iter = presizes.begin();
        std::advance(it_iter, pos);
        SSS_LOG_DEBUG("presizes.size = %d\n", this->presizes.size());
        this->presizes.erase(it_iter);
        SSS_LOG_DEBUG("after presizes\n");

        it_iter = bytes.begin();
        std::advance(it_iter, pos);
        SSS_LOG_DEBUG("presizes.size = %d\n", this->bytes.size());
        this->bytes.erase(it_iter);
        SSS_LOG_DEBUG("after bytes\n");

        it_iter = maxlens.begin();
        std::advance(it_iter, pos);
        this->maxlens.erase(it_iter);

        SSS_LOG_DEBUG("after maxlens\n");
    }
}

void FieldInfo::erase(const std::string& col_name)      // {{{2
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    // NOTE 仅删除第一次出现的 col_name 字段
    size_t i = 0;
    for (; i != this->names.size(); ++i) {
        if (this->names[i] == col_name) {
            break;
        }
    }

    if (i != this->names.size()) {
        this->erase(i);
    }
}


void FieldInfo::swap(FieldInfo& ref)    // {{{2
{
    names.swap(ref.names);
    types.swap(ref.types);
    presizes.swap(ref.presizes);
    bytes.swap(ref.bytes);
    maxlens.swap(ref.maxlens);
}

/** Class ResRows */ // {{{1
/** Constructor */
ResRows::ResRows()      // {{{2
{
    // empty;
}

/** Destructor */
ResRows::~ResRows()     // {{{2
{
    // empty;
}

void ResRows::print(std::ostream& o, const std::string& sep) const      // {{{2
{
    for (const_iterator it = this->begin(); it != this->end(); ++it) {
        (*it).print(o, sep);
        o << std::endl;
    }
}

void ResRows::del_col(int col_id)       // {{{2
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    for (iterator it = this->begin(); it != this->end(); ++it) {
        FieldList& row = (*it);
        row.erase(col_id);
    }
}

void ResRows::del_row(int row_id)       // {{{2
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    //std::remove(begin, end, match);       -- 移动
    //this->rows.erase(pos, end);           -- 删除一个范围
    iterator row = this->begin();
    std::advance(row, row_id);
    this->erase(row);
}

// fdname_index_t // {{{1

// Class ResultSet // {{{1
ResultSet::ResultSet()  // {{{2
{
}

ResultSet::~ResultSet() // {{{2
{
}

bool ResultSet::has_field(const std::string& fdname) const      // {{{2
{
    return fdname_index.find(fdname) != fdname_index.end();
}

std::vector<std::string> & ResultSet::names()   // {{{2
{
    return infos.names;
}

void ResultSet::swap(ResultSet& ref)    // {{{2
{
    infos.swap(ref.infos);
    rows.swap(ref.rows);
    compute_sets.swap(ref.compute_sets);
    fdname_index.swap(ref.fdname_index);
}

bool ResultSet::is_numeric(int col) const       // {{{2
{
    return sqltype_is_numeric(infos.types[col]);
}

bool ResultSet::is_binary(int col) const        // {{{2
{
    return sqltype_is_binary(infos.types[col]);
}

bool ResultSet::is_datetime(int col) const      // {{{2
{
    return sqltype_is_datetime(infos.types[col]);
}

bool ResultSet::is_char(int col) const  // {{{2
{
    return sqltype_is_char(infos.types[col]);
}

bool ResultSet::is_bit(int col) const   // {{{2
{
    return sqltype_is_bit(infos.types[col]);
}

bool ResultSet::is_text(int col) const  // {{{2
{
    return sqltype_is_text(infos.types[col]);
}

void ResultSet::pretty_print(std::ostream& o)   // {{{2
{
    try {
        if (this->width()) {

            std::vector<int> fld_max_lens;
            {
                std::ostringstream oss;
                for (int i = 0; i < int(this->width()); ++i) {
                    int fld_len = std::max(this->maxlen(i), int(this->name(i).length()));
                    oss << ' ' << this->name(i) << std::string(fld_len - this->name(i).length(), ' ');
                    fld_max_lens.push_back(fld_len);
                }
                o << sss::rtrim_copy(oss.str()) << std::endl;
            }

            for (int i = 0; i < int(this->width()); ++i) {
                o << ' ' << std::string(fld_max_lens[i], '-');
            }
            o << std::endl;

            for (int r = 0; r < this->height(); ++r) {
                std::ostringstream oss;
                for (int c = 0; c < int(this->width()); ++c) {
                    // 因为数字类数据的大小比较，是从低位比较高位；
                    // 所以，这部分列类型，使用右对齐；反之，使用左对齐；
                    oss << ' ';
                    std::string fld = this->get(r, c).tostr_trim();
                    if (this->is_numeric(c)) {
                        oss << std::string(fld_max_lens[c] - fld.length(), ' ')
                            << fld;
                    }
                    else {
                        oss << fld
                            << std::string(fld_max_lens[c] - fld.length(), ' ');
                    }
                }
                o << sss::rtrim_copy(oss.str()) << std::endl;
            }
        }
    }
    catch (std::exception& e) {
        SSS_LOG_ERROR("catch from %s\n", __func__);
        throw;
    }
}

void ResultSet::set_fieldinfo(const sss::tdspp2::Query& q)
{
    this->infos.assign(q);
}

// 按列名删除某列
void ResultSet::del_col(const std::string& col_name)    // {{{2
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    fdname_index_t::iterator it = this->fdname_index.find(col_name);
    if (it != this->fdname_index.end()) {
        this->del_col(it->second);
    }
}

// 按列数删除某列
void ResultSet::del_col(int col_id)     // {{{2
{
    SSS_LOG_FUNC_TRACE(sss::log::log_DEBUG);
    if (col_id >= 0 && col_id < this->width()) {
        this->infos.erase(col_id);
        this->rows.del_col(col_id);
        this->calc_fdname_index();
    }
}

// 删除某行
void ResultSet::del_row(int row_id)     // {{{2
{
    if (row_id >= 0 and row_id < this->height()) {
        this->rows.del_row(row_id);
    }
}

void ResultSet::calc_fdname_index()     // {{{2
{
    fdname_index.clear();
    for (size_t i = 0; i != infos.names.size(); ++i) {
        fdname_index[infos.names[i]] = i;
    }
    //fdname_index.assign(infos.names.begin(), infos.names.end());
}

// example:
//      Quer q;
//      q.sql("some sql");
//      q.execute();
//      ResultSet rs;
//      while (rs.fetch(q)) {
//         rs.pretty_print(std::cout);
//      }
bool ResultSet::fetch(sss::tdspp2::Query& q)    // {{{2
{
    // 注意，execute 在外！
    return this->load(q, true);
}

// 载入一个结果集；
// 如果，成功，返回true; 否则，返回false
// NOTE 必须先调用 bind(FieldList&); 然后才能使用本函数获取结果集；
// 不然，只能返回空的结果；
bool ResultSet::load(sss::tdspp2::Query& q, bool donot_execute)     // {{{2
{
    try {
        bool is_ok = false;
        sss::tdspp2::FieldList * p_fl = q.current_row;
        sss::tdspp2::FieldList fl;
        q.bind(&fl);
        if (!donot_execute) {
            q.execute();
        }
        //if (!p_fl) {
        //    return false;
        //}
        if (q.getresult()) {
            ResultSet tmp;
            is_ok = true;
            if (!q.headers.empty()) {
                tmp.infos.assign(q);
                //tmp.infos.names = q.headers;
                //tmp.infos.types = q.types;
                //tmp.infos.bytes = q.col_buf_len;

                //tmp.infos.maxlens.reserve(q.headers.size());

                //tmp.infos.presizes = q.presizes;

                //// NOTE 我觉得，不应该记录head的长度
                //for (size_t i = 0; i != q.headers.size(); ++i) {
                //    //tmp.infos.maxlens.push_back(q.headers[i].length());
                //    tmp.infos.maxlens.push_back(0);
                //}

                tmp.calc_fdname_index();

                while (q.fetch()) {
                    tmp.append(fl);
                }
                tmp.swap(*this);
            }
        }

        q.bind(p_fl);
        return is_ok;
    }
    catch (...) {
        SSS_LOG_ERROR("Exception: when query at %p\n", &q);
        throw;
    }
}

bool ResultSet::append(sss::tdspp2::FieldList& fl) // {{{2
{
    this->rows.push_back(fl);
    if (this->rows.back().size() != fl.size()) {
        std::ostringstream oss;
        oss << "size changed from " << fl.size()
            << " to " << this->rows.back().size();
        throw std::logic_error(oss.str());
    }
    for (size_t i = 0; i < this->infos.size(); ++i) {
        int pre_maxlen = this->infos.maxlens[i];
        this->infos.maxlens[i] = std::max(this->infos.maxlens[i],
                                          int(fl[i]->tostr_trim().length()));
        int aft_maxlen = this->infos.maxlens[i];
        SSS_LOG_INFO("col %d maxlens %d => %d; cur_len = %d; '%s'\n",
                     i, pre_maxlen, aft_maxlen,
                     fl[i]->length(), fl[i]->tostr_trim().c_str());
    }
    return true;
}

bool ResultSet::empty() const   // {{{2
{
    return infos.names.empty();
}

int  ResultSet::width() const   // {{{2
{
    try {
        return infos.names.size();
    }
    catch (...) {
        SSS_LOG_ERROR("Exception: this at %p\n", this);
        throw;
    }
}

int  ResultSet::height() const  // {{{2
{
    return rows.size();
}

Field & ResultSet::get(int row, int col)        // {{{2
{
    try {
        return *rows.at(row).at(col);
    }
    catch (...) {
        SSS_LOG_ERROR("Exception: this(%p), row(%d), col(%d); rows(%d)\n",
                      this, row, col, rows.size());
        throw;
    }
}

Field & ResultSet::get(int row, const std::string& fdname)      // {{{2
{
    try {
        return *rows.at(row).at(fdname_index[fdname]);
    }
    catch (...) {
        SSS_LOG_ERROR("Exception: this(%p), row(%d), fdname(%s)\n",
                      this, row, fdname.c_str());
        throw;
    }
}

std::string& ResultSet::name(int col)   // {{{2
{
    try {
        return infos.names[col];
    }
    catch (...) {
        SSS_LOG_ERROR("Exception: this(%p) col(%d)\n", this, col);
        throw;
    }
}

int  ResultSet::presize(int col) const  // {{{2
{
    return infos.presizes[col];
}

int  ResultSet::type(int col) const     // {{{2
{
    return infos.types[col];
}

int  ResultSet::byte(int col) const     // {{{2
{
    return infos.bytes[col];
}

int  ResultSet::maxlen(int col) const   // {{{2
{
    return infos.maxlens[col];
}


} // namespace tdspp2
} // namespace sss
