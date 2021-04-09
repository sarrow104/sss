#include "Terminal.hpp"

#ifdef SSS_PLT_WINDOWS
#       include <sss/bit_operation/bit_operation.h>
#       include <windows.h>
#       include <unistd.h>             // isatty(STDOUT_FILENO)
#       include <fcntl.h>              // mingw O_CREAT
#else
#       include <unistd.h>             // isatty(STDOUT_FILENO)
#endif

#include <cstring>
#include <stdexcept>

#include <sss/log.hpp>

namespace inner { // {{{

std::streambuf * fn_streambuf[3];

// 将STDxxx_FILENO和std::streambuf*捆绑在一起；
// 对于windows的console来说，我是通过检测环境变量，来判断该程序，是运行在模
// 拟终端下面，还是在普通的console下面；
// 至于有没有将输出重定向，就没有检测；
// 根据实测，mingw确实有isatty函数；不过O_CREAT宏定义的位置不同；
// linux：sys/types.h
// mingw：fcntl.h
// 不过在msys下运行的话，isatty(0-3)都是0；
// 而奇怪的是，在cmd窗口下运行，则都返回64；效果如：
// $ ./console.exe
// 2016-01-01 12:11:09 [error][main.cpp:104][testIsatty] ::isatty(STDIN_FILENO) = `0`
// 2016-01-01 12:11:09 [error][main.cpp:105][testIsatty] ::isatty(STDOUT_FILENO) = `0`
// 2016-01-01 12:11:09 [error][main.cpp:106][testIsatty] ::isatty(STDERR_FILENO) = `0`
// 2016-01-01 12:11:09 [error][main.cpp:109][testIsatty] fno = `3`
// 2016-01-01 12:11:09 [error][main.cpp:110][testIsatty] ::isatty(fno) = `0`
//
// E:\project\sss_test\console\win32>console.exe
// 2016-01-01 12:11:17 [error][main.cpp:104][testIsatty] ::isatty(STDIN_FILENO) = `64`
// 2016-01-01 12:11:17 [error][main.cpp:105][testIsatty] ::isatty(STDOUT_FILENO) = `64`
// 2016-01-01 12:11:17 [error][main.cpp:106][testIsatty] ::isatty(STDERR_FILENO) = `64`
// 2016-01-01 12:11:17 [error][main.cpp:109][testIsatty] fno = `3`
// 2016-01-01 12:11:17 [error][main.cpp:110][testIsatty] ::isatty(fno) = `0`
//
// 这意味着什么？
//
// isatty()确实有作用；而且msys下，应该是处理为管道——一边读取，一边显示所
// 以isatty的判断结果会有些怪；
//
// 重定向之后呢？
//
// $ ./console.exe > out
//
// $ cat out
// 2016-01-01 12:15:44 [error][main.cpp:104][testIsatty] ::isatty(STDIN_FILENO) = `0`
// 2016-01-01 12:15:44 [error][main.cpp:105][testIsatty] ::isatty(STDOUT_FILENO) = `0`
// 2016-01-01 12:15:44 [error][main.cpp:106][testIsatty] ::isatty(STDERR_FILENO) = `0`
// 2016-01-01 12:15:44 [error][main.cpp:109][testIsatty] fno = `3`
// 2016-01-01 12:15:44 [error][main.cpp:110][testIsatty] ::isatty(fno) = `0`
//
// E:\project\sss_test\console\win32>console.exe > out
//
// E:\project\sss_test\console\win32>type out
// 2016-01-01 12:15:27 [error][main.cpp:104][testIsatty] ::isatty(STDIN_FILENO) = `64`
// 2016-01-01 12:15:27 [error][main.cpp:105][testIsatty] ::isatty(STDOUT_FILENO) = `0`
// 2016-01-01 12:15:27 [error][main.cpp:106][testIsatty] ::isatty(STDERR_FILENO) = `64`
// 2016-01-01 12:15:27 [error][main.cpp:109][testIsatty] fno = `3`
// 2016-01-01 12:15:27 [error][main.cpp:110][testIsatty] ::isatty(fno) = `0`
//
// 怎么做？
//
// windows下
//
//   如果是console运行(通过环境变量检测)，那么isatty可用；
//   如果是模拟终端，isatty就不可用了；只能选择输出转义序列——因为标准输出
//   输入流，在虚拟终端这里的重定向，是被虚拟终端完全接管了的。用户程序是不
//   可能判断出来，输出的这些字符，到底是被虚拟终端显示出来，还是说，导入到
//   外部文件中，甚至丢弃。
bool init_fn_streambuf() // {{{
{
    fn_streambuf[STDIN_FILENO] = std::cin.rdbuf();
    fn_streambuf[STDOUT_FILENO] = std::cout.rdbuf();
    fn_streambuf[STDERR_FILENO] = std::cerr.rdbuf();
    return true;
}

bool is_init_fn_streambuf = init_fn_streambuf();

struct fn_stream_table_t { // {{{
    int m_fno;
    std::streambuf * m_streambuf;
};

#ifdef SSS_PLT_WINDOWS
namespace win32 { // {{{

HANDLE fn_handle[3];

bool init_fn_handle()
{
    fn_handle[STDIN_FILENO]     = ::GetStdHandle(STD_INPUT_HANDLE);
    fn_handle[STDOUT_FILENO]    = ::GetStdHandle(STD_OUTPUT_HANDLE);
    fn_handle[STDERR_FILENO]    = ::GetStdHandle(STD_ERROR_HANDLE);
    return true;
}

bool is_init_hd_streambuf = init_fn_handle();

HANDLE getHandleFromStdStream(std::ostream& o)
{
    for (int i = 0; i< 3; ++i) {
        if (fn_streambuf[i] == o.rdbuf()) {
            return fn_handle[i];
        }
    }
    return INVALID_HANDLE_VALUE;
}

int getConsoleAttribute() // {{{
{
    CONSOLE_SCREEN_BUFFER_INFO csi;
    std::memset(&csi, 0, sizeof(csi));
    HANDLE h = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) {
        return 0;
    }
    if (::GetConsoleScreenBufferInfo(h, &csi)) {
        return csi.wAttributes;
    }
    else {
        return 0;
    }
}
}
#endif
enum terminal_mode_t { // {{{
    TERMINAL_MODE_OFF = 0,
    TERMINAL_MODE_TTY = 1,
#ifdef SSS_PLT_WINDOWS
    TERMINAL_MODE_CMD = 2
#endif
};

terminal_mode_t getDefaultTerminalMode() // {{{
{
#ifdef SSS_PLT_WINDOWS
    return ::getenv("COLORTERM") ? TERMINAL_MODE_TTY : TERMINAL_MODE_CMD;
#else
    return isatty(STDOUT_FILENO) ? TERMINAL_MODE_TTY : TERMINAL_MODE_OFF;
#endif
}

#ifdef SSS_PLT_WINDOWS
namespace win32 {
bool applyTextAttr(int attr) // {{{
{
    HANDLE h = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }
    ::SetConsoleTextAttribute(h, attr);
    return true;
}
bool applyMoveXY(int row, int col) // {{{
{
    HANDLE h = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }
    COORD coord;
    coord.Y = row;
    coord.X = col;
    ::SetConsoleCursorPosition(h, coord);
    return true;
}

bool applyCurorMoveBy(int row, int col) // {{{
{
    HANDLE h = ::GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        return false;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if( !::GetConsoleScreenBufferInfo( hConsole, &csbi ))
    {
        return false;
    }

    csbi.dwCursorPosition.X += col;
    csbi.dwCursorPosition.Y += row;
    ::SetConsoleCursorPosition(hConsole, csbi.dwCursorPosition);
    return true;
}

bool applyEraseBehind() // {{{
{
    // 流程，类似 applyClearScreen
    // 1. 获取当前窗口属性
    // 2. 光标当前位置
    // 3. 默认风格；
    // 4. 覆盖重写；
    HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        return false;
    }
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    // Get the number of character cells in the current buffer.

    if( !::GetConsoleScreenBufferInfo( hConsole, &csbi ))
    {
        return false;
    }

    DWORD dwConSize = - csbi.dwCursorPosition.X + csbi.dwSize.X;
    COORD coordScreen = csbi.dwCursorPosition;

    DWORD cCharsWritten;
    if( !::FillConsoleOutputCharacter( hConsole,        // Handle to console screen buffer
                                      (TCHAR) ' ',     // Character to write to the buffer
                                      dwConSize,       // Number of cells to write
                                      coordScreen,     // Coordinates of first cell
                                      &cCharsWritten ))// Receive number of characters written
    {
        return false;
    }

    // Get the current text attribute.

    if( !::GetConsoleScreenBufferInfo( hConsole, &csbi ))
    {
        return false;
    }

    // Set the buffer's attributes accordingly.

    if( !::FillConsoleOutputAttribute( hConsole,         // Handle to console screen buffer
                                      csbi.wAttributes, // Character attributes to use
                                      dwConSize,        // Number of cells to set attribute
                                      coordScreen,      // Coordinates of first cell
                                      &cCharsWritten )) // Receive number of characters written
    {
        return false;
    }

    // Put the cursor at its home coordinates.

    ::SetConsoleCursorPosition( hConsole, coordScreen );
    return true;
}

bool applyClearScreen() // {{{
{
    HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        return false;
    }
    COORD coordScreen = { 0, 0 };    // home for the cursor
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    // Get the number of character cells in the current buffer.

    if( !::GetConsoleScreenBufferInfo( hConsole, &csbi ))
    {
        return false;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // Fill the entire screen with blanks.

    if( !::FillConsoleOutputCharacter( hConsole,        // Handle to console screen buffer
                                      (TCHAR) ' ',     // Character to write to the buffer
                                      dwConSize,       // Number of cells to write
                                      coordScreen,     // Coordinates of first cell
                                      &cCharsWritten ))// Receive number of characters written
    {
        return false;
    }

    // Get the current text attribute.

    if( !::GetConsoleScreenBufferInfo( hConsole, &csbi ))
    {
        return false;
    }

    // Set the buffer's attributes accordingly.

    if( !::FillConsoleOutputAttribute( hConsole,         // Handle to console screen buffer
                                      csbi.wAttributes, // Character attributes to use
                                      dwConSize,        // Number of cells to set attribute
                                      coordScreen,      // Coordinates of first cell
                                      &cCharsWritten )) // Receive number of characters written
    {
        return false;
    }

    // Put the cursor at its home coordinates.

    ::SetConsoleCursorPosition( hConsole, coordScreen );
    return true;
}
}
#endif

// TODO 通过环境变量，判断是否在msys下运行；
static int terminal_mode = getDefaultTerminalMode();
} // namespace inner

namespace sss { // {{{

namespace Terminal {// {{{

void enable_tty_mode() // {{{
{
    inner::terminal_mode = inner::TERMINAL_MODE_TTY;
}

#ifdef SSS_PLT_WINDOWS
void enable_cmd_mode() // {{{
{
    inner::terminal_mode = inner::TERMINAL_MODE_CMD;
}
#endif

void disable_style() // {{{
{
    inner::terminal_mode = inner::TERMINAL_MODE_OFF;
}

bool is_tty_mode() // {{{
{
    return inner::terminal_mode == inner::TERMINAL_MODE_TTY;
}

// 其实没办法正确判断是否是tty……
// 因为有reopen？
bool isatty(std::ostream& o) // {{{
{
#ifdef  SSS_PLT_WINDOWS
    return ::getenv("COLORTERM") || inner::win32::getHandleFromStdStream(o) != INVALID_HANDLE_VALUE;
#else
    if (!inner::is_init_fn_streambuf) {
        return false;
    }
    for (int i = 0; i < 3; ++i) {
        if (inner::fn_streambuf[i] == o.rdbuf()) {
            return ::isatty(i);
        }
    }
    return false;
#endif
}

#ifdef SSS_PLT_WINDOWS
bool is_cmd_mode() // {{{
{
    return inner::terminal_mode == inner::TERMINAL_MODE_CMD;
}
#endif

bool is_style_off() // {{{
{
    return inner::terminal_mode == inner::TERMINAL_MODE_OFF;
}

#ifdef SSS_PLT_WINDOWS
int getTextDefaultAttr() // {{{
{
    static const int default_attr = inner::win32::getConsoleAttribute();
    return default_attr;
}
#endif

namespace style { // {{{
begin::begin(font_mode_t mode)
{
#ifdef SSS_PLT_WINDOWS
    this->_attr = 0;
#endif
    // "\x1b[31m",          // normal | red
    // "\x1b[1;34m",        // bold   | blue
    // "\x1b[1;32;40m",     // bold   | green | background_black
    std::memset(_data, 0, sizeof(_data));
    std::strcat(_data, "\x1b[");

    char * prender = std::strchr(_data, '\0');

    bool is_first = true;

    if (mode & 0x7u) {
        is_first = false;
        *prender++ = (mode & 0x7u) - 1 + '1';
    }

    int foreground_color = (mode >> 5u) & 0x0Fu;
    if (foreground_color) {
        foreground_color >>= 1; // discard black color
        if (!is_first) {
            *prender++ = ';';
        }
        std::sprintf(prender, "%d", foreground_color + 30);
        is_first = false;
        prender = std::strchr(prender, '\0');
#ifdef SSS_PLT_WINDOWS
        this->_attr |= sss::bit::swapbit(foreground_color & 0x7u, 0, 2);
#endif
    }

    int background_color = (mode >> 10u) & 0x0Fu;
    if (background_color) {
        background_color >>= 1; // discard black color
        if (!is_first) {
            *prender++ = ';';
        }
        std::sprintf(prender, "%d", background_color + 40);
        is_first = false;
        prender = std::strchr(prender, '\0');
#ifdef SSS_PLT_WINDOWS
        this->_attr |= (sss::bit::swapbit(background_color & 0x7u, 0, 2) << 4);
#endif
    }
    *prender++ = 'm';
    this->_data_len = prender - this->_data;

#ifdef SSS_PLT_WINDOWS
    if (mode == sss::Terminal::style::FONT_RESET) {
        this->_attr = sss::Terminal::getTextDefaultAttr();
    }
    else if (mode & sss::Terminal::style::FONT_BOLD) {
        if (this->_attr & 0x7u) {
            this->_attr |= FOREGROUND_INTENSITY;
        }
        if (this->_attr & (0x7u << 4)) {
            this->_attr |= BACKGROUND_INTENSITY;
        }
    }
#endif
}

const char * begin::data() const {
    return sss::Terminal::is_tty_mode() ? this->_data : "";
}

int   begin::data_len() const {
    return sss::Terminal::is_tty_mode() ? this->_data_len : 0;
}

void begin::print(std::ostream& o) const
{
#ifdef SSS_PLT_WINDOWS
    if (sss::Terminal::is_tty_mode()) {
        o << this->data();
    }
    else if (sss::Terminal::is_cmd_mode() && sss::Terminal::isatty(o)) {
        o.flush();
        inner::win32::applyTextAttr(this->_attr);
    }
#else
    if (sss::Terminal::is_tty_mode() && sss::Terminal::isatty(o)) {
        o << this->data();
    }
#endif
}

style::begin    end(sss::Terminal::style::FONT_RESET);
}

style::begin        error(sss::Terminal::style::FONT_BOLD | sss::Terminal::style::FONT_F_RED);
style::begin        warning(sss::Terminal::style::FONT_BOLD | sss::Terminal::style::FONT_F_YELLOW);
style::begin        debug(sss::Terminal::style::FONT_BOLD | sss::Terminal::style::FONT_F_BLUE);
style::begin        info(sss::Terminal::style::FONT_BOLD | sss::Terminal::style::FONT_F_GREEN);
style::begin        strong(sss::Terminal::style::FONT_BOLD | sss::Terminal::style::FONT_F_WHITE);
style::begin        normal(sss::Terminal::style::FONT_F_WHITE);
style::begin        dark(sss::Terminal::style::FONT_DARK | sss::Terminal::style::FONT_F_WHITE);

// 关闭所有键盘指示灯
const char * turnOffKBLights = "\x1b[0q";
// 设置“滚动锁定”指示灯 (Scroll Lock)
const char * setKBScrollLock = "\x1b[1q";
// 设置“数值锁定”指示灯 (Num Lock)
const char * setKBNumLock = "\x1b[2q";
// 设置“大写锁定”指示灯 (Caps Lock)
const char * setKBCapsLock = "\x1b[3q";
// 发蜂鸣声beep
const char * beep = "\x07";

Screen::Screen(std::ostream& o)
    : m_o(o)
{
}

Screen::~Screen()
{
}


void Screen::clear()
{
    const char * op_str = "\x1b[2J";
#ifdef  SSS_PLT_WINDOWS
    if (sss::Terminal::is_tty_mode()) {
        m_o << op_str;
    }
    else if (sss::Terminal::is_cmd_mode() && sss::Terminal::isatty(m_o)) {
        m_o.flush();
        inner::win32::applyClearScreen();
    }
#else
    if (sss::Terminal::is_tty_mode() && sss::Terminal::isatty(m_o)) {
        m_o << op_str;
    }
#endif
}

void Screen::cursorMoveTo(int row, int col)
{
    char buf[24];
    std::sprintf(buf, "\x1b[%d;%dH", row, col);
#ifdef  SSS_PLT_WINDOWS
    if (sss::Terminal::is_tty_mode()) {
        m_o << buf;
    }
    else if (sss::Terminal::is_cmd_mode() && sss::Terminal::isatty(m_o)) {
        m_o.flush();
        inner::win32::applyMoveXY(row, col);
    }
#else
    if (sss::Terminal::is_tty_mode() && sss::Terminal::isatty(m_o)) {
        m_o << buf;
    }
#endif
}

void Screen::cursorMoveBy(int row, int col)
{
    char buf[24] = {'\0'};
    char * p = buf;
    if (row) {
        int offset = 0;
        std::sprintf(p, "\x1b[%d%c%n", std::abs(row), row > 0 ? 'B' : 'A', &offset);
        p += offset;
    }
    if (col) {
        std::sprintf(p, "\x1b[%d%c", std::abs(col), col > 0 ? 'C' : 'D');
    }
#ifdef  SSS_PLT_WINDOWS
    if (sss::Terminal::is_tty_mode()) {
        m_o << buf;
    }
    else if (sss::Terminal::is_cmd_mode() && sss::Terminal::isatty(m_o)) {
        m_o.flush();
        inner::win32::applyCurorMoveBy(row, col);
    }
#else
    if (sss::Terminal::is_tty_mode() && sss::Terminal::isatty(m_o)) {
        m_o << buf;
    }
#endif
}

void Screen::eraseBehind()
{
    const char * op_str = "\x1b[K";
#ifdef  SSS_PLT_WINDOWS
    if (sss::Terminal::is_tty_mode()) {
        m_o << op_str;
    }
    else if (sss::Terminal::is_cmd_mode() && sss::Terminal::isatty(m_o)) {
        m_o.flush();
        inner::win32::applyEraseBehind();
    }
#else
    if (sss::Terminal::is_tty_mode() && sss::Terminal::isatty(m_o)) {
        m_o << op_str;
    }
#endif
}

} // namespace Terminal
} // namespace sss

