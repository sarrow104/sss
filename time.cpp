#include <sss/time.hpp>

#include <time.h>

#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <assert.h>
#include <sstream>
#include <map>
#include <string>

#include <sss/util/Parser.hpp>
#include <sss/util/StringSlice.hpp>
#include <sss/log.hpp>

#ifdef __WIN32__

// http://stackoverflow.com/questions/667250/strptime-in-windows
const char * strp_weekdays[] =
{
    "sunday",   "monday",       "tuesday",      "wednesday",
    "thursday", "friday",       "saturday"
};

const char * strp_monthnames[] =
{
    "january",  "february",     "march",        "april",
    "may",       "june",        "july",         "august",
    "september", "october",     "november",     "december"
};

bool strp_atoi(const char * & s, int & result, int low, int high, int offset)
{
    bool worked = false;
    char * end;
    unsigned long num = strtoul(s, & end, 10);
    if (num >= (unsigned long)low && num <= (unsigned long)high)
    {
        result = (int)(num + offset);
        s = end;
        worked = true;
    }
    return worked;
}

char * strptime(const char *s, const char *format, struct tm *tm)
{
    bool working = true;
    while (working && *format && *s)
    {
        switch (*format)
        {
        case '%':
            {
                ++format;
                switch (*format)
                {
                case 'a':
                case 'A': // weekday name
                    tm->tm_wday = -1;
                    working = false;
                    for (size_t i = 0; i < 7; ++ i)
                    {
                        size_t len = strlen(strp_weekdays[i]);
                        if (!strnicmp(strp_weekdays[i], s, len))
                        {
                            tm->tm_wday = i;
                            s += len;
                            working = true;
                            break;
                        }
                        else if (!strnicmp(strp_weekdays[i], s, 3))
                        {
                            tm->tm_wday = i;
                            s += 3;
                            working = true;
                            break;
                        }
                    }
                    break;
                case 'b':
                case 'B':
                case 'h': // month name
                    tm->tm_mon = -1;
                    working = false;
                    for (size_t i = 0; i < 12; ++ i)
                    {
                        size_t len = strlen(strp_monthnames[i]);
                        if (!strnicmp(strp_monthnames[i], s, len))
                        {
                            tm->tm_mon = i;
                            s += len;
                            working = true;
                            break;
                        }
                        else if (!strnicmp(strp_monthnames[i], s, 3))
                        {
                            tm->tm_mon = i;
                            s += 3;
                            working = true;
                            break;
                        }
                    }
                    break;
                case 'd':
                case 'e': // day of month number
                    working = strp_atoi(s, tm->tm_mday, 1, 31, 0);
                    break;
                case 'D': // %m/%d/%y
                    {
                        const char * s_save = s;
                        working = strp_atoi(s, tm->tm_mon, 1, 12, -1);
                        if (working && *s == '/')
                        {
                            ++ s;
                            working = strp_atoi(s, tm->tm_mday, 1, 31, 0);
                            if (working && *s == '/')
                            {
                                ++ s;
                                working = strp_atoi(s, tm->tm_year, 0, 99, 0);
                                if (working && tm->tm_year < 69)
                                    tm->tm_year += 100;
                            }
                        }
                        if (!working)
                            s = s_save;
                    }
                    break;
                case 'H': // hour
                    working = strp_atoi(s, tm->tm_hour, 0, 23, 0);
                    break;
                case 'I': // hour 12-hour clock
                    working = strp_atoi(s, tm->tm_hour, 1, 12, 0);
                    break;
                case 'j': // day number of year
                    working = strp_atoi(s, tm->tm_yday, 1, 366, -1);
                    break;
                case 'm': // month number
                    working = strp_atoi(s, tm->tm_mon, 1, 12, -1);
                    break;
                case 'M': // minute
                    working = strp_atoi(s, tm->tm_min, 0, 59, 0);
                    break;
                case 'n': // arbitrary whitespace
                case 't':
                    while (isspace((int)*s))
                        ++s;
                    break;
                case 'p': // am / pm
                    if (!strnicmp(s, "am", 2))
                    { // the hour will be 1 -> 12 maps to 12 am, 1 am .. 11 am, 12 noon 12 pm .. 11 pm
                        if (tm->tm_hour == 12) // 12 am == 00 hours
                            tm->tm_hour = 0;
                    }
                    else if (!strnicmp(s, "pm", 2))
                    {
                        if (tm->tm_hour < 12) // 12 pm == 12 hours
                            tm->tm_hour += 12; // 1 pm -> 13 hours, 11 pm -> 23 hours
                    }
                    else
                        working = false;
                    break;
                case 'r': // 12 hour clock %I:%M:%S %p
                    {
                        const char * s_save = s;
                        working = strp_atoi(s, tm->tm_hour, 1, 12, 0);
                        if (working && *s == ':')
                        {
                            ++ s;
                            working = strp_atoi(s, tm->tm_min, 0, 59, 0);
                            if (working && *s == ':')
                            {
                                ++ s;
                                working = strp_atoi(s, tm->tm_sec, 0, 60, 0);
                                if (working && isspace((int)*s))
                                {
                                    ++ s;
                                    while (isspace((int)*s))
                                        ++s;
                                    if (!strnicmp(s, "am", 2))
                                    { // the hour will be 1 -> 12 maps to 12 am, 1 am .. 11 am, 12 noon 12 pm .. 11 pm
                                        if (tm->tm_hour == 12) // 12 am == 00 hours
                                            tm->tm_hour = 0;
                                    }
                                    else if (!strnicmp(s, "pm", 2))
                                    {
                                        if (tm->tm_hour < 12) // 12 pm == 12 hours
                                            tm->tm_hour += 12; // 1 pm -> 13 hours, 11 pm -> 23 hours
                                    }
                                    else
                                        working = false;
                                }
                            }
                        }
                        if (!working)
                            s = s_save;
                    }
                    break;
                case 'R': // %H:%M
                    {
                        const char * s_save = s;
                        working = strp_atoi(s, tm->tm_hour, 0, 23, 0);
                        if (working && *s == ':')
                        {
                            ++ s;
                            working = strp_atoi(s, tm->tm_min, 0, 59, 0);
                        }
                        if (!working)
                            s = s_save;
                    }
                    break;
                case 'S': // seconds
                    working = strp_atoi(s, tm->tm_sec, 0, 60, 0);
                    break;
                case 'T': // %H:%M:%S
                    {
                        const char * s_save = s;
                        working = strp_atoi(s, tm->tm_hour, 0, 23, 0);
                        if (working && *s == ':')
                        {
                            ++ s;
                            working = strp_atoi(s, tm->tm_min, 0, 59, 0);
                            if (working && *s == ':')
                            {
                                ++ s;
                                working = strp_atoi(s, tm->tm_sec, 0, 60, 0);
                            }
                        }
                        if (!working)
                            s = s_save;
                    }
                    break;
                case 'w': // weekday number 0->6 sunday->saturday
                    working = strp_atoi(s, tm->tm_wday, 0, 6, 0);
                    break;
                case 'Y': // year
                    working = strp_atoi(s, tm->tm_year, 1900, 65535, -1900);
                    break;
                case 'y': // 2-digit year
                    working = strp_atoi(s, tm->tm_year, 0, 99, 0);
                    if (working && tm->tm_year < 69)
                        tm->tm_year += 100;
                    break;
                case '%': // escaped
                    if (*s != '%')
                        working = false;
                    ++s;
                    break;
                default:
                    working = false;
                }
            }
            break;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case '\f':
        case '\v':
            // zero or more whitespaces:
            while (isspace((int)*s))
                ++ s;
            break;
        default:
            // match character
            if (*s != *format)
                working = false;
            else
                ++s;
            break;
        }
        ++format;
    }
    return (working?(char *)s:0);
}
#endif

static const char * week_full_name[7]={
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
};

static int month_day[12] = {
    0,
    31,
    31+28,
    31+28+31,
    31+28+31+30,
    31+28+31+30+31,
    31+28+31+30+31+30,
    31+28+31+30+31+30+31,
    31+28+31+30+31+30+31+31,
    31+28+31+30+31+30+31+31+30,
    31+28+31+30+31+30+31+31+30+31,
    31+28+31+30+31+30+31+31+30+31+30
};

namespace sss{
    namespace time {

        std::string strftime(const std::string& fmt_s, const tm & current_tm)
        {
            const char * fmt = fmt_s.c_str();
            const struct tm * c_tm = &current_tm;
            std::ostringstream oss;
            char buf[48];
            size_t sp = sizeof(buf);
            while (fmt[0]) {
                size_t cur_write = 0;
                size_t fmt_offset = 2;
                if (fmt[0] == '%') {
                    switch(fmt[1]) {
                    case 'a':
                        cur_write = std::strftime(buf, sp, "%a", c_tm);
                        break;

                    case 'A':
                        cur_write = std::strftime(buf, sp, "%A", c_tm);
                        break;

                    case 'b':
                        cur_write = std::strftime(buf, sp, "%b", c_tm);
                        break;

                    case 'c':
                        cur_write = std::strftime(buf, sp, "%c", c_tm);
                        break;

                    case 'C':
                        // NOTE check sp ！length
                        // std::cout << __func__ << ":" << __LINE__ << " " << c_tm->tm_year << std::endl;
                        cur_write = std::sprintf(buf, "%02d", ((c_tm->tm_year + 1900)/ 100) % 100);
                        break;

                    case 'd':
                        cur_write = std::strftime(buf, sp, "%d", c_tm);
                        break;

                    case 'D':
                        cur_write = std::strftime(buf, sp, "%m/%d/%y", c_tm);
                        break;

                    case 'e':
                        cur_write = std::sprintf(buf, "%02d", c_tm->tm_mday);
                        break;

                    case 'F':
                        cur_write = std::strftime(buf, sp, "%Y-%m-%d", c_tm);
                        break;

                    case 'g':
                        cur_write = std::sprintf(buf, "%02d", ((c_tm->tm_year + 1900) % 100));
                        break;

                    case 'G':
                        cur_write = std::sprintf(buf, "%04d", (c_tm->tm_year + 1900));
                        break;

                    case 'h':
                        cur_write = std::strftime(buf, sp, "%b", c_tm);
                        break;

                    case 'H':
                        cur_write = std::strftime(buf, sp, "%H", c_tm);
                        break;

                    case 'I':
                        cur_write = std::strftime(buf, sp, "%I", c_tm);
                        break;

                    case 'j':
                        cur_write = std::strftime(buf, sp, "%j", c_tm);
                        break;

                    case 'm':
                        cur_write = std::strftime(buf, sp, "%m", c_tm);
                        break;

                    case 'M':
                        cur_write = std::strftime(buf, sp, "%M", c_tm);
                        break;

                    case 'n':
                        cur_write = std::sprintf(buf, "%s", "\n");
                        break;

                    case 'p':
                        cur_write = std::strftime(buf, sp, "%p", c_tm);
                        break;

                    case 'P':
                        cur_write = std::strftime(buf, sp, "%p", c_tm);
                        buf[0] = std::tolower(buf[0]);
                        buf[1] = std::tolower(buf[1]);
                        break;

                    case 'r':
                        cur_write = std::strftime(buf, sp, "%H:%M:%S %p", c_tm);
                        break;

                    case 'R':
                        cur_write = std::strftime(buf, sp, "%H:%M", c_tm);
                        break;

                    case 'S':
                        cur_write = std::strftime(buf, sp, "%S", c_tm);
                        break;

                    case 't':
                        cur_write = std::sprintf(buf, "%s", "\t");
                        break;

                    case 'T':
                        cur_write = std::strftime(buf, sp, "%H:%M:%S", c_tm);
                        break;

                    case 'u':
                        // TODO check sp
                        cur_write = std::sprintf(buf, "%d", c_tm->tm_wday);
                        break;

                    case 'U':
                        cur_write = std::strftime(buf, sp, "%U", c_tm);
                        break;

                    case 'V':
                        // TODO 每年的第几周，使用基于周的年
                        cur_write = std::strftime(buf, sp, "%U", c_tm);
                        break;

                    case 'w':
                        cur_write = std::strftime(buf, sp, "%w", c_tm);
                        break;

                    case 'W':
                        cur_write = std::strftime(buf, sp, "%W", c_tm);
                        break;

                    case 'x':
                        cur_write = std::strftime(buf, sp, "%x", c_tm);
                        break;

                    case 'X':
                        cur_write = std::strftime(buf, sp, "%X", c_tm);
                        break;

                    case 'y':
                        cur_write = std::strftime(buf, sp, "%y", c_tm);
                        break;

                    case 'Y':
                        cur_write = std::strftime(buf, sp, "%Y", c_tm);
                        break;

                    case 'z':
                        cur_write = std::strftime(buf, sp, "%z", c_tm);
                        break;

                    case 'Z':
                        cur_write = std::strftime(buf, sp, "%Z", c_tm);
                        break;

                    case '+':
                        buf[0] = '%';
                        buf[1] = '+';
                        buf[2] = '\0';
                        cur_write = 2;
                        break;

                    case '%':
                        buf[0] = '%';
                        buf[2] = '\0';
                        cur_write = 1;
                        break;

                    default:
                        cur_write = 1;
                        buf[0] = fmt[0];
                        if (fmt[1]) {
                            buf[1] = fmt[1];
                            cur_write++;
                        }
                        buf[cur_write] = '\0';
                        fmt_offset = cur_write;
                        break;
                    }
                }
                else {
                    // while (fmt[] != '%') {
                    // }
                    cur_write = 1;
                    buf[0] = fmt[0];
                    buf[1] = '\0';
                    fmt_offset = 1;
                }

                oss << buf;
                fmt += fmt_offset;
            }
            return oss.str();
        }

        int weekday_of(const std::string& date)
        {
            int year, month, day;
            char s1, s2;
            //puts("Please give me a date(yyyy.mm.dd)?");
            sscanf(date.c_str(), "%d%c%d%c%d", &year, &s1, &month, &s2, &day);
            assert(s1 == s2);
            return weekday_of(year, month, day);
        }

        int weekday_of(int year, int month, int day)
        {
            long total_days = (year - 1900)*365 + (year-1900)/4 + month_day[month - 1] + day - 1;
            if ((year - 1900) % 4 == 0 && month < 3 && year != 1900)
                --total_days;
            return total_days % 7;
        }

        const char * weekday_name(int d)
        {
            assert(d >= 0 && d < 7);
            return ::week_full_name[d];
        }

        //----------------------------------------------------------------------
        //date.cpp

        Date::Date( create_method cm )
        {
            std::time_t t;
            std::time(&t);

            this->set_time_t(t);

            this->create_filter(cm);
        }

        void Date::create_filter( create_method cm )
        {
            switch (cm)
            {
            case DATE_PART:
                this->hour(0);
                this->minute(0);
                this->seconds(0);
                break;

            case TIME_PART:
                this->year(0);
                this->month(0);
                this->day(0);
                break;

            case WHOLE_PART:
                break;

            default:
                break;
            }
        }

        Date::Date( std::time_t t)
        {
            this->set_time_t(t);
        }

        // 如何从字符串读取时间？
        // 需要解析！
        // 当然，有默认的解析模式
        // {4Y}-{2M}-{2D}
        // 也可以自定义；
        // {2h}:{2m}:{2s}
        //
        // 这样，系统会根据长度，截取对应位置的字符串，并解析为int供程序构建时间
        //
        // 即，上述相当于 记名的正则字符串
        // \(\d\{4}:year\)-\(\d\{2}:month\)-\(\d\{2}:day\)
        // \(\d\{2}:hour\)-\(\d\{2}:minute\)-\(\d\{2}:seconds\)
        //
        // 解析结果，可以用预定义的名字 "year" 等来获取。

typedef std::string::const_iterator iterator;
typedef sss::util::Parser<iterator> Parser_t;

namespace {
    bool parseFieldHolder(std::string::const_iterator & it_beg, std::string::const_iterator it_end,
                          uint32_t& field_width, char & type)
    {
        //SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
        Parser_t::Rewinder r(it_beg);
        if (r.commit(Parser_t::parseChar(it_beg, it_end, '{') &&
                     Parser_t::parseUint32_t(it_beg, it_end, field_width, 1) &&
                     (field_width == 2 || field_width == 4) &&
                     Parser_t::parseAnyChar(it_beg, it_end, type) &&
                     (type == 'Y' || type == 'M' || type == 'D' ||
                      type == 'h' || type == 'm' || type == 's') &&
                     Parser_t::parseChar(it_beg, it_end, '}')))
        {
            return true;
        }
        return false;
    }
}

// TODO
// 分别用来解析年月日，以及时分秒的字符串，应该作为类的const static成员。
// FIXME
// 我的格式输出串，用的是系统的 % 方式；
//
// 而我这里却成了 {width-type} 逻辑；显然有冲突；
// 最好的办法是，在提供fmt的同时，设置显示逻辑；
// 这样，可以保证原样解析，和原样输出！
//
// 不需要去做！
// <time.h>，已经提供了strptime函数……
// 注意，该函数返回，指向余下字符的指针；如果失败，则返回nullptr；
Date::Date(const std::string& str_time, const std::string& fmt)
{
    std::memset(&this->_t, 0, sizeof(this->_t));
    if (!strptime(str_time.c_str(), fmt.c_str(), &_t)) {
        std::ostringstream oss;
        oss << __func__ << " strptime(\"" << str_time << "\", \"" << fmt << "\") error!";
        throw std::runtime_error(oss.str());
    }
    this->initFinal();
}

void Date::set_time_t(time_t t)
{
    std::time_t t_ = t;
    // FIXME 不可重入！
    this->_t = *std::localtime(&t_);
}

Date::Date(short year,  short mon,     short day)
{
    std::memset(&this->_t, 0, sizeof(this->_t));
    this->initDatePart(year, mon, day);

    this->initFinal();
}

Date::Date(short year,  short mon,     short day,
           short hours, short minutes, short seconds)
{
    std::memset(&this->_t, 0, sizeof(this->_t));

    this->initDatePart(year, mon, day);
    this->initDatePart(hours, minutes, seconds);

    this->initFinal();
}

void Date::initFinal()
{
    // SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
    //  夏令时标识符，实行夏令时的时候，tm_isdst为正。不实行夏令时的进候，
    //  tm_isdst为0；不了解情况时，tm_isdst()为负。
    //  夏令时：人为将时间拨快依一个小时——因为盛夏天亮得早。
    //this->_t.tm_isdst = 0;

    std::time_t t = std::mktime(&this->_t);
#ifdef __WIN32__
    struct tm* pt = std::localtime(&t);
#else
    struct tm lt;

    tzset(); // set timezone
    //! http://stackoverflow.com/questions/19170721/real-time-awareness-of-timezone-change-in-localtime-vs-localtime-r

    struct tm* pt = localtime_r(&t, &lt);
#endif
    assert(pt);
    this->_t = *pt;
}

void Date::initDatePart(short year,  short mon,     short day)
{
    assert(year > 1900 &&
           mon > 0 && mon < 13 &&
           day > 0 && day < 32);
    this->year(year);
    this->month(mon);
    this->day(day);
}

void Date::initTimePart(short hours, short minutes, short seconds)
{
    assert(hours >= 0  && hours < 24 &&
           minutes >= 0 && minutes < 60 &&
           seconds >= 0 && seconds < 60);
    this->hour(hours);
    this->minute(minutes);
    this->seconds(seconds);
}

Date::Date(const Date& other, create_method cm)
    : _t(other._t)
{
    this->create_filter(cm);
}

std::ostream& operator<<(std::ostream& os, const Date& d)
{
    os << d.year() << '-' << d.month() << '-' << d.day();
    return os;
}

Date& Date::operator=(const Date& other)
{
    if( this == &other)
        return *this;
    this->_t = other._t;
    return *this;
}

bool Date::less(const Date& d) const
{
    return this->to_seconds() < d.to_seconds();
}

int Date::year( void ) const
{
    return _t.tm_year + 1900;
}

int Date::year(int y)
{
    int Y = this->year();
    this->_t.tm_year = y - 1900;
    return Y;
}

int Date::month( void ) const
{
    return _t.tm_mon + 1;
}

int Date::month(int m)
{
    int M = this->month();
    this->_t.tm_mon = m - 1;
    return M;
}

int Date::day( void ) const
{
    return _t.tm_mday;
}

int Date::day(int d)
{
    int D = this->day();
    this->_t.tm_mday = d;
    return D;
}

int Date::week_day() const
{
    return this->_t.tm_wday;
}

int Date::hour() const
{
    return this->_t.tm_hour;
}

int Date::hour(int h)
{
    int H = this->hour();
    this->_t.tm_hour = h;
    return H;
}

int Date::minute() const
{
    return this->_t.tm_min;
}

int Date::minute(int m)
{
    int M = this->minute();
    this->_t.tm_min = m;
    return M;
}

int Date::seconds() const
{
    return this->_t.tm_sec;
}

int Date::seconds(int s)
{
    int S = this->seconds();
    this->_t.tm_sec = s;
    return S;
}

Date& Date::add_day(int day)
{
    std::time_t t = std::mktime(&this->_t);
    t += day * 24 * 3600;
    this->_t = *std::localtime(&t);
    return *this;
}

Date& Date::add_sec(int sec)
{
    std::time_t t = std::mktime(&this->_t);
    t += sec;
    this->_t = *std::localtime(&t);
    return *this;
}

bool Date::operator==(const Date& other)
{
    return (_t.tm_year == other._t.tm_year
            && _t.tm_mon == other._t.tm_mon
            && _t.tm_mday == other._t.tm_mday);
}

// NOTE not used
//static int Dtab[2][13] =
//{
//    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
//    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
//};

std::string Date::format(const std::string& fmt) const
{
    // NOTE sample "%Y-%m-%d %T"
    //char buf[256];
    //std::strftime(buf, sizeof(buf), fmt.c_str(), &this->_t);
    //return std::string(buf);
    return sss::time::strftime(fmt, this->_t);
}

int Date::interval_days(const Date& d) const
{
    Date now(*this, DATE_PART);
    Date before(d, DATE_PART);

    return now.interval_seconds(before) / (24 * 3600);
}

int Date::interval_seconds(const Date& d) const
{
    struct tm N = this->_t;
    struct tm B = d._t;
    std::time_t now = std::mktime(&N);
    std::time_t before = std::mktime(&B);
    return (now - before);
}

int Date::to_seconds() const
{
    struct tm Now = this->_t;
    return std::mktime(&Now);
}

} // namespace time

} // namespace sss
