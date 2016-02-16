#ifndef  __TIME_HPP_1320158433__
#define  __TIME_HPP_1320158433__

#include <string>
#include <ctime>
#include <iostream>

#ifdef __WIN32__
#       include <windows.h>
#else
#       include <unistd.h>
#endif

//把日期转化为星期几的C程序 {{{1
//2007-12-03
// http://blog.163.com/dingmaotu@126/blog/static/21484302007113101533164/

// 老师布置了这样一个作业，日期范围是1900年1月1日至2099年12月31日。我看很多程序
// 都很低效，于是写了一个高效一点的，编译时速度较慢，运行绝对快，参考了linux
// 0.11 mktime.c中的函数。有时研究一下内核还是能学到很多东西的。以下是源程序：

/*
* 文件：datetoweek.c
*
* 查询1900.1.1至2099.12.31的日期为星期几。
* 1900.1.1为星期一
*/

// yyyy.mm.dd
// start from Monday
// 0, "Monday",
// 1, "Tuesday",
// 2, "Wednesday",
// 3, "Thursday",
// 4, "Friday",
// 5, "Saturday",
// 6, "Sunday"
// }}}

// struct tm {
//     int tm_sec;     /* seconds after the minute - [0,59] */
//     int tm_min;     /* minutes after the hour - [0,59] */
//     int tm_hour;    /* hours since midnight - [0,23] */
//     int tm_mday;    /* day of the month - [1,31] */
//     int tm_mon;     /* months since January - [0,11] */
//     int tm_year;    /* years since 1900 */
//     int tm_wday;    /* days since Sunday - [0,6] */
//     int tm_yday;    /* days since January 1 - [0,365] */
//     int tm_isdst;   /* daylight savings time flag */
// };

namespace sss{

    namespace time {
inline void sleep(int msec)
{
#ifdef __WIN32__
    ::Sleep(msec);
#else
    ::usleep(msec*1000);
#endif
}

int weekday_of(const std::string& date);

int weekday_of(int year, int month, int day);

// 返回程序已运行的毫秒数
inline int time_passed()
{
    return clock();
}

// 返回日历时间——从1970 年1月1日0时0分0秒到此时，经过的秒数
// time_t 实际是long 的typdef
inline time_t time()
{
    return std::time(NULL);
}

// 格式说明{{{1
// %a 星期几的简写
// %A 星期几的全称
// %b 月分的简写
// %B 月份的全称
// %c 标准的日期的时间串(locale)
// %C 年份的后两位数字 -- NOTE 不建议读取时使用
// %d 十进制表示的每月的第几天(range 01 to 31)
// %D 月/天/年(%m/%d/%y)
// %e 在两字符域中，十进制表示的每月的第几天(用空格作为fillchar)
// %F 年-月-日(YYYY-MM-DD)(%Y-%m-%d)
// %g 年份的后两位数字，使用基于周的年(2-digit year (00-99))-- NOTE 不建议读取时使用
// %G 年分，使用基于周的年
//    The ISO 8601 week-based year (see NOTES) with century as a
//    decimal number.  The 4-digit year corresponding to the ISO
//    week number  (see %V).  This has the same format and value as
//    %Y, except that if the ISO week number belongs to the
//    previous or next year, that year is used instead. (TZ)
// %h 简写的月份名(equal_to %b)
// %H 24小时制的小时(range 00 to 23)
// %I 12小时制的小时(range 01 to 12)
// %j 十进制表示的每年的第几天(range 001 to 366)
// %m 十进制表示的月份(range 01 to 12)
// %M 十时制表示的分钟数(range 00 to 59)
// %n 新行符(A newline character)
// %p 本地的AM或PM的等价显示(locale) Noon is treated as "PM" and midnight as "AM"
// %P Like %p but in lowercase: "am" or "pm" or a corresponding string for the current locale.
// %r 12小时的时间
//    The time in a.m. or p.m. notation.
//    In the POSIX locale this is equivalent to %I:%M:%S %p
// %R 显示小时和分钟：hh:mm(%H:%M)
// %S 十进制的秒数(range 00 to 60).
//    (The range is up to 60 to allow for occasional leap seconds.)
// %t 水平制表符(A tab character)
// %T 显示时分秒：hh:mm:ss (%H:%M:%S)
// %u 每周的第几天，星期一为第一天 （值从0到6，星期一为0）
// %U 第年的第几周，把星期日做为第一天 01。（值从0到53）
// %V 每年的第几周，使用基于周的年
//    The ISO 8601 week number (see NOTES) of the current year as a
//    decimal number, range 01 to 53, where week 1  is  the  first
//    week that has at least 4 days in the new year.  See also %U
//    and %W.
// %w 十进制表示的星期几（值从0到6，星期天为0）
// %W 每年的第几周，把星期一做为第一天（值从0到53）
// %x 标准的日期串 (locale)
// %X 标准的时间串 (locale)
//NOTE %y 不带世纪的十进制年份（值从0到99）
//NOTE %Y 带世纪部分的十制年份
// %z The +hhmm or -hhmm numeric timezone (that is, the hour and minute offset from UTC)
// %Z The timezone name or abbreviation
// %+ The date and time in date(1) format. (TZ) (Not supported in glibc2.)
// %% 百分号
// }}}

inline std::string strftime(const std::string& fmt, const tm & c_tm)
{
    char buf[256];
    std::strftime(buf, sizeof(buf), fmt.c_str(), &c_tm);
    return std::string(buf);
}

inline std::string strftime(const std::string& fmt)
{
    time_t lt = std::time(NULL);
    // localtime 返回的tm结构，是从1900年开始算起。
    struct tm c_tm = *std::localtime(&lt);

    return strftime(fmt, c_tm);
}

// 如何对日期进行运算？
class Date
{
public:
    enum create_method {DATE_PART = 1, TIME_PART = 2, WHOLE_PART = 3};

public:
    Date( create_method cm = WHOLE_PART );

    Date( std::time_t );

    // 解析字符串形式的时间，并生成对象
    // %% 表示一个 % 号！
    Date( const std::string& str_time , const std::string& fmt = "%F");

    Date(short year,  short mon,     short day);

    Date(short year,  short mon,     short day,
         short hours, short minutes, short seconds);

    Date(const Date& other, create_method cm = WHOLE_PART);

public:
    Date& operator=(const Date& other);

protected:
    void create_filter( create_method cm );

public:
    void set_time_t(time_t t);

    int year( void ) const;
    int year( int y );
    int month( void ) const;
    int month( int m );
    int day( void ) const;
    int day( int d );

    int hour() const;
    int hour(int h);
    int minute() const;
    int minute(int m);
    int seconds() const;
    int seconds(int s);

    int week_day() const;

    bool operator==(const Date& other);
    inline bool is_leap_year() const
    {
        return Date::is_leap_year(this->year());
    }

    static
    inline bool is_leap_year(int y)
    {
        return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    }

    Date& add_day(int day);

    Date& add_sec(int sec);

    int interval_days(const Date& d) const;
    int interval_seconds(const Date& d) const;

    int to_seconds() const;

    inline Date& operator+=(int days)
    {
        this->add_day(days);
        return *this;
    }

    bool less(const Date& d) const;

    operator struct tm() const
    {
        return this->_t;
    }

public:
    friend std::ostream& operator<<(std::ostream& os, const Date& d);
    std::string format(const std::string& fmt) const;

protected:
    void initFinal();
    void initDatePart(short year,  short mon,     short day);
    void initTimePart(short hours, short minutes, short seconds);

private:
    tm _t;
};

inline
bool operator<(const Date& left, const Date& right)
{
    return left.less(right);
}

inline
int operator-(const Date& left, const Date& right)
{
    return left.interval_days(right);
}

inline
Date operator+(const Date& left, int days)
{
    sss::time::Date next(left);
    next.add_day(days);
    return next;
}

inline
Date operator-(const Date& left, int days)
{
    sss::time::Date next(left);
    next.add_day(-days);
    return next;
}

inline bool operator != (const Date& left, const Date& right)
{
    return left.to_seconds() != right.to_seconds();
}

// TODO Timer
// 形如 ： SSS_TIME_TIMER(command, stream, exception_msg);
// 生成：
// {
//   try{
//    sss::time::timer __t__;
//    command;
//   }
//   catch(std::exception& e) {
//      stream << exception_msg;
//      throw;
//   }
// }

#define SSS_TIME_TIMER(command, msg, stream) \
{ \
    sss::time::Timer __timer__(msg, stream); \
    command; \
}

class Timer
{
public:
    Timer(const std::string& msg, std::ostream& o)
        : _runing(false), _msg(msg), _o(o)
    {
        this->start();
    }

    ~Timer()
    {
        if (this->_runing) {
            this->end();
        }
    }

public:

    void start()
    {
        sss::time::Date now;
        this->_start = now;
        this->_runing = true;
    }

    void end()
    {
        sss::time::Date _end;
        this->_o
            << this->_msg << " : "
            << _end.interval_seconds(this->_start) << "s"
            << std::endl;
        this->_runing = false;
    }

private:
    bool                _runing;
    sss::time::Date     _start;
    std::string         _msg;
    std::ostream&       _o;
};

} // namespace time

} // namespace sss

#endif  /* __TIME_HPP_1320158433__ */
