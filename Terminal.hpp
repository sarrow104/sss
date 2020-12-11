#ifndef  __TERMINAL_HPP_1449483182__
#define  __TERMINAL_HPP_1449483182__

#include <string>
#include <cstdlib>
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <iostream>

#include <tuple>

#include <sss/macro/platform.hpp>

// NOTE
// linux 下，常用的终端tools是ncurses；甚至可以支持256种颜色；
//! http://www.gnu.org/software/ncurses/

// see also:
//! http://www.tldp.org/HOWTO/Bash-Prompt-HOWTO/x361.html
// ######################################################
// 文本终端的颜色可以使用“ANSI非常规字符序列”来生成。举例：
// 　　echo -e "\033[44;37;5m ME \033[0m COOL"
// 以上命令设置背景成为蓝色，前景白色，闪烁光标，输出字符“ME”，然后重新设置屏
// 幕到缺省设置，输出字符 “COOL”。“e”是命令 echo 的一个可选项，它用于激活特
// 殊字符的解析器。“\033”引导非常规字符序列。“m”意味着设置属性然后结束非常
// 规字符序列，这个例子里真正有效的字符是 “44;37;5” 和“0”。
//
// 修改“44;37;5”可以生成不同颜色的组合，数值和编码的前后顺序没有关系。可以选
// 择的编码如下所示：
//
// 编码    颜色/动作
// 0       重新设置属性到缺省设置
// 1       设置粗体
// 2       设置一半亮度（模拟彩色显示器的颜色）
// 4       设置下划线（模拟彩色显示器的颜色）
// 5       设置闪烁
// 7       设置反向图象
//
// 22      设置一般密度
// 24      关闭下划线
// 25      关闭闪烁
// 27      关闭反向图象
//
// 30      设置黑色前景
// 31      设置红色前景  --
// 32      设置绿色前景  --
// 33      设置黄色前景
// 34      设置蓝色前景  --
// 35      设置紫色前景
// 36      设置青色前景
// 37      设置白色前景
//
// 38      在缺省的前景颜色上设置下划线
// 39      在缺省的前景颜色上关闭下划线
//
// 40      设置黑色背景
// 41      设置红色背景
// 42      设置绿色背景
// 43      设置黄色背景
// 44      设置蓝色背景
// 45      设置紫色背景
// 46      设置青色背景
// 47      设置白色背景
//
// 49      设置缺省黑色背景

// NOTE windows console 文本属性值列表
// FOREGROUND_BLUE	1       Text color contains blue.
// FOREGROUND_GREEN	2       Text color contains green.
// FOREGROUND_RED	4       Text color contains red.
// FOREGROUND_INTENSITY	8       Text color is intensified.
// BACKGROUND_BLUE	16      Background color contains blue.
// BACKGROUND_GREEN	32      Background color contains green.
// BACKGROUND_RED	64      Background color contains red.
// BACKGROUND_INTENSITY	128     Background color is intensified.
//
// COMMON_LVB_LEADING_BYTE	Leading byte.    // 以下属性，用于Double-byte Character Sets
// COMMON_LVB_TRAILING_BYTE	Trailing byte.
// COMMON_LVB_GRID_HORIZONTAL	Top horizontal.
// COMMON_LVB_GRID_LVERTICAL	Left vertical.
// COMMON_LVB_GRID_RVERTICAL	Right vertical.
// COMMON_LVB_REVERSE_VIDEO	Reverse foreground and background attributes.
// COMMON_LVB_UNDERSCORE	Underscore.

namespace sss {

namespace Terminal{
void enable_tty_mode();
#ifdef SSS_PLT_WINDOWS
void enable_cmd_mode();
#endif
void disable_style();

bool is_tty_mode();
#ifdef SSS_PLT_WINDOWS
bool is_cmd_mode();
#endif
bool is_style_off();

#ifdef SSS_PLT_WINDOWS
int getTextDefaultAttr();
#endif

bool isatty(std::ostream& o);

namespace style {

enum font_mode_t {
    FONT_RESET      = 0,
    FONT_BOLD       = 1,
    FONT_DARK       = 2,
    FONT_UNDERLINE  = 4,
    FONT_BLINK      = 5,
    FONT_REVERSE    = 7,

    // 前景色；8种；
    // 可看做：
    // C(0,3) + C(1,3) + C(2,3) + C(3,3) = 1 + 3 + 3 + 1 = 8
    // 不过，需要把黑色单独处理——
    // 因为，黑色，在上述表示方法中，是默认值==0；
    // 就算用户不显示写FONT_F_BLACK的话，还是会应用该颜色设置——这会用
    // 户的想法相悖；
    //
    // 黑色，与其他颜色 | 运算，都得其他颜色；
    //    只有1. 其他颜色，在二进制上，都包含黑色；
    //        2. 黑色，就用0表示；
    //
    // 所以……
    FONT_F_BLACK    = 1 << 5, // 30
    FONT_F_RED      = (1 << 6) | FONT_F_BLACK,
    FONT_F_GREEN    = (2 << 6) | FONT_F_BLACK,
    FONT_F_YELLOW   = (3 << 6) | FONT_F_BLACK, // 黄色：红色+绿色
    FONT_F_BLUE     = (4 << 6) | FONT_F_BLACK,
    FONT_F_PURPLE   = (5 << 6) | FONT_F_BLACK, // 紫色：红色+蓝色
    FONT_F_CYAN     = (6 << 6) | FONT_F_BLACK, // 青色：绿色+蓝色
    FONT_F_WHITE    = (7 << 6) | FONT_F_BLACK, // 37

    // 背景色
    FONT_G_BLACK    = 1 << 10, // 40
    FONT_G_RED      = (1 << 11) | FONT_G_BLACK,
    FONT_G_GREEN    = (2 << 11) | FONT_G_BLACK,
    FONT_G_YELLOW   = (3 << 11) | FONT_G_BLACK,
    FONT_G_BLUE     = (4 << 11) | FONT_G_BLACK,
    FONT_G_PURPLE   = (5 << 11) | FONT_G_BLACK,
    FONT_G_CYAN     = (6 << 11) | FONT_G_BLACK,
    FONT_G_WHITE    = (7 << 11) | FONT_G_BLACK, // 47
};

inline font_mode_t operator | (font_mode_t lhs, font_mode_t rhs)
{
    return font_mode_t(uint32_t(lhs) | uint32_t(rhs));
}

// NOTE
// 支持形如：
// std::cout << Terminal::sytle::bigin(Terminal::sytle::FONT_F_RED)
//           << arg1 << arg2
//           << Terminal::sytle::end
//           << std::endl;
// 的链式高亮方式；
template<typename T> struct highlight_t;
// 高亮对象？

// 注意，高亮风格的嵌套使用，不是那么简单的；
// terminal的高亮，本质上都是状态改变的开关，是一种线式操作；
// 若要模拟嵌套，就需要使用一个栈结构；
// style_out(data1, style_inner(data2))
//
// 要么，在显示风格改变之后，先转回默认状态；然后转到目标风格状态；

class begin{
public:
    explicit begin(font_mode_t mode);
    ~begin()
    {
    }

public:
    const char * data() const;
    int          data_len() const;

    void print(std::ostream& o) const;

    template<typename T> highlight_t<T> operator()(const T& value);

    // TODO ArgsT -> std::tuple()
    // 注意：构成 tuple 参数的，可能来自临时变量——这可能导致在最后实际 operator << 输出动作调用的时候，
    // 该部分内存空间不再有效。因此，最好还是：
    //   std::cout << Terminal::begin << va1 << va2 << v3 << Terminal::end << ... 这样的使用方式。
    // template<typename... Args>
    //     highlight_t<std::tuple<typename __decay_and_strip<Args>::__type...>> operator()(Args&&... args)
    //     {
    //         return {*this, std::make_tuple(args...)};
    //     }
    //相关知识：
    //  saved-args
    //! http://stackoverflow.com/questions/15537817/c-how-to-store-a-parameter-pack-as-a-variable/15537864
    //  使用timax::bind， 来获取一个std::function对象
    //! http://purecpp.org/?p=952

private:
#ifdef SSS_PLT_WINDOWS
    int     _attr;
#endif
    char    _data[12];
    int     _data_len;
};

template<typename T> struct highlight_t
{
    highlight_t(const Terminal::style::begin& s, const T& val)
        : style(s), value(val)
    {
    }

    const Terminal::style::begin& style;
    const T& value;

    void print(std::ostream& o) const;
};

inline std::ostream& operator << (std::ostream& o, const Terminal::style::begin& beg)
{
    beg.print(o);
    return o;
};

template<typename T> inline std::ostream& operator << (std::ostream& o, const highlight_t<T>& h)
{
    h.print(o);
    return o;
}

// NOTE
// reset to defalt-font setting
extern begin    end;

template<typename T> inline void highlight_t<T>::print(std::ostream& o) const
{
    o << this->style << this->value << sss::Terminal::style::end;
}

template<typename T>
inline highlight_t<T> begin::operator() (const T& value) {
    return highlight_t<T>(*this, value);
}
} // namespace style
using style::end;

extern style::begin        error;
extern style::begin        warning;
extern style::begin        debug;
extern style::begin        info;
extern style::begin        strong;
extern style::begin        normal;
extern style::begin        dark;

// 关闭所有键盘指示灯
extern const char * turnOffKBLights;
// 设置“滚动锁定”指示灯 (Scroll Lock)
extern const char * setKBScrollLock;
// 设置“数值锁定”指示灯 (Num Lock)
extern const char * setKBNumLock;
// 设置“大写锁定”指示灯 (Caps Lock)
extern const char * setKBCapsLock;
// 发蜂鸣声beep
extern const char * beep;

// NOTE
// 关于光标移动；
// 类颜色高亮处理方式的不同，光标移动，调用api与终端解释字符流的区别；
//
// 对于调用api来说，问题比较严重；如果输入输出流，没有绑定到标准窗口，
// 那么就麻烦了，可能这边的光标操作，会影响实际console窗口的显示！
//
// 这绝对是引入bug的麻烦。
//
// 怎么办？
//
// 需要使用全局变量；
//
// 在初始化的时候，就知道三个标准输入输出流(以及其文件描述符)，到底是怎
// 么绑定的；
//
// http://www.tldp.org/HOWTO/Bash-Prompt-HOWTO/x361.html
//- Position the Cursor:
//  \033[<L>;<C>H
//     Or
//  \033[<L>;<C>f
//  puts the cursor at line L and column C.
//- Move the cursor up N lines:
//  \033[<N>A
//- Move the cursor down N lines:
//  \033[<N>B
//- Move the cursor forward N columns:
//  \033[<N>C
//- Move the cursor backward N columns:
//  \033[<N>D
//
//- Clear the screen, move to (0,0):
//  \033[2J
//- Erase to end of line:
//  \033[K
//
//- Save cursor position:
//  \033[s
//- Restore cursor position:
//  \033[u
//
// "\x1b[15;40H"     把光标移动到第15行，40列

class Screen
{
public:
    Screen(std::ostream& o);
    ~Screen();

public:
    void clear();
    void cursorMoveTo(int row, int col);
    // NOTE
    // 屏幕的位移动作，并不会考虑已经输出到终端屏幕上，这些字符串，末尾的位置；
    // 即，可以任务，所谓的行尾就是屏幕宽度；
    // 也就是下面这个函数，相当于按"向量"来进行位移；
    void cursorMoveBy(int row, int col);
    void eraseBehind();

private:
    std::ostream& m_o;
};

// syntax sugar
struct _MoveTo
{
    _MoveTo(int row, int col)
        : m_row(row), m_col(col)
    {
    }
    int m_row;
    int m_col;
};

inline _MoveTo moveTo(int row, int col)
{
    return _MoveTo(row, col);
}

inline std::ostream& operator << (std::ostream& o, const _MoveTo& m)
{
    sss::Terminal::Screen s(o);
    s.cursorMoveTo(m.m_row, m.m_col);
    return o;
}

struct _MoveBy
{
    _MoveBy(int row, int col)
        : m_row(row), m_col(col)
    {
    }
    int m_row;
    int m_col;
};

inline _MoveBy moveBy(int row, int col)
{
    return _MoveBy(row, col);
}

inline std::ostream& operator << (std::ostream& o, const _MoveBy& m)
{
    sss::Terminal::Screen s(o);
    s.cursorMoveBy(m.m_row, m.m_col);
    return o;
}

inline std::ostream& clearScreen(std::ostream& o)
{
    sss::Terminal::Screen s(o);
    s.clear();
    return o;
}

inline std::ostream& eraseBehind(std::ostream& o)
{
    sss::Terminal::Screen s(o);
    s.eraseBehind();
    return o;
}

inline std::ostream& operator<< (std::ostream& o, std::ostream& (*func_oper)(std::ostream&) )
{
    return func_oper(o);;
}
} // namespace Terminal
} // namespace sss



#endif  /* __TERMINAL_HPP_1449483182__ */

