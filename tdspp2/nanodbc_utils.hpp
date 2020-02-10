#pragma once

#include <cstring>
#include <mutex>

#include <nanodbc/nanodbc.h>

#if defined(__GNUC__) && __GNUC__ < 5
#include <cwchar>
#else
#include <codecvt>
#endif
#include <locale>
#include <string>

#ifdef NANODBC_USE_IODBC_WIDE_STRINGS
#error Examples do not support the iODBC wide strings
#endif

namespace odbc {
namespace util {
struct row_dumper_t{
    explicit row_dumper_t(nanodbc::result& row, const std::string& sep = "\t")
        : row_(row), sep_(sep)
        {
        }
    nanodbc::result& row_;
    std::string sep_ = "\t";
    void print(std::ostream& o) const
    {
        bool is_first = true;
        for (int i = 0; i < row_.columns(); ++i) {
            if (is_first) {
                o << sep_;
            }
            o << row_.get<std::string>(i);
            is_first = false;
        }
    }
    row_dumper_t& operator=(const row_dumper_t& ref) = default;
};

inline row_dumper_t dump_row(nanodbc::result& row, const std::string& sep = "\t")
{
    return row_dumper_t{row, sep};
}

inline std::ostream& operator<< (std::ostream& o, const row_dumper_t& r)
{
    r.print(o);
    return o;
}

bool sqltype_is_numeric(int type);
bool sqltype_is_binary(int type);

inline std::string find_first_matched_driver(const std::vector<std::string>& drivers, const std::string& pattern)
{
    for (std::string driver : drivers) {
        // std::string::find
        if (driver.find(pattern) != std::string::npos) {
            return driver;
        }
    }
    return "";
}

const std::vector<std::string>& get_driver_list();

// 参考：http://blog.csdn.net/fanwenbo/article/details/38535573
// https://msdn.microsoft.com/zh-cn/library/system.data.odbc.odbcconnection.connectionstring.aspx
// "Driver={SQL Server};Server=(local);Trusted_Connection=Yes;Database=AdventureWorks;"
// "Driver={Microsoft ODBC for Oracle};Server=ORACLE8i7;Persist Security Info=False;Trusted_Connection=Yes"
// "Driver={Microsoft Access Driver (*.mdb)};DBQ=c:\bin\Northwind.mdb"
// "Driver={Microsoft Excel Driver (*.xls)};DBQ=c:\bin\book1.xls"
// "Driver={Microsoft Text Driver (*.txt; *.csv)};DBQ=c:\bin"
// "DSN=dsnname"
inline std::string gen_dsn_mssql(const std::string& server, const std::string& dbname)
{
    std::string driver = find_first_matched_driver(get_driver_list(), "SQL Server");
    if (driver.empty()) {
        return "";
    }
    return "Driver={" + driver + "};Server={" + server + ";Trusted_Connection=Yes;Database={" + dbname + "};";;
}

inline std::string gen_dsn_mssql(const std::string& server, const std::string& dbname, const std::string& uname, const std::string& pwd)
{
    // cat /etc/odbcinst.ini
    //! https://blog.csdn.net/code_my_life/article/details/45097051
    // DRIVER={ODBC Driver 11 for SQL Server};SERVER={111.111.111.111,1433};UID={sa};PWD={xxxxxxxx};
    std::string driver = find_first_matched_driver(get_driver_list(), "SQL Server");
    if (driver.empty()) {
        return "";
    }
    return "Driver={"+driver+"};Server={"+server+"};Trusted_Connection=No;Database={"+dbname+"};Uid={"+uname+"};Pwd={"+pwd+"};";
}

// TODO: These convert utils need to be extracted to a private
//       internal library to share with tests
#ifdef NANODBC_ENABLE_UNICODE
inline nanodbc::string convert(std::string const& in)
{
    static_assert(
        sizeof(nanodbc::string::value_type) > 1,
        "NANODBC_ENABLE_UNICODE mode requires wide string");
    nanodbc::string out;
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
    // Workaround for confirmed bug in VS2015 and VS2017 too
    // See: https://connect.microsoft.com/VisualStudio/Feedback/Details/1403302
    using wide_char_t = nanodbc::string::value_type;
    auto s =
        std::wstring_convert<std::codecvt_utf8_utf16<wide_char_t>, wide_char_t>().from_bytes(in);
    auto p = reinterpret_cast<wide_char_t const*>(s.data());
    out.assign(p, p + s.size());
#else
    out = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().from_bytes(in);
#endif
    return out;
}

inline std::string convert(nanodbc::string const& in)
{
    static_assert(sizeof(nanodbc::string::value_type) > 1, "string must be wide");
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
    // Workaround for confirmed bug in VS2015 and VS2017 too
    // See: https://connect.microsoft.com/VisualStudio/Feedback/Details/1403302
    using wide_char_t = nanodbc::string::value_type;
    std::wstring_convert<std::codecvt_utf8_utf16<wide_char_t>, wide_char_t> convert;
    auto p = reinterpret_cast<const wide_char_t*>(in.data());
    out = convert.to_bytes(p, p + in.size());
#else
    out = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().to_bytes(in);
#endif
    return out;
}
#else
inline nanodbc::string convert(std::string const& in)
{
    return in;
}
#endif

template <typename T>
inline std::string any_to_string(T const& t)
{
    return std::to_string(t);
}

template <>
inline std::string any_to_string<nanodbc::string>(nanodbc::string const& t)
{
    return convert(t);
}

} // namespace util
} // namespace odbc
