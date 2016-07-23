#ifndef  __LOG_HPP_1321928111__
#define  __LOG_HPP_1321928111__

// TODO
// 用户可以决定，那种级别的信息，输出到哪个(或哪几个)出口。
// 当前，只能定义一个信息出口，不是文件，就是stdout。
//
// 可以如此设定：
//
// 首先，定义一个FILE 的列表；然后每个列表对应一个消息级别。
//
// 不过，真的有必要提供列表的形式吗？
//
// 其实用户最多需要两种输出——文件的，和屏幕的；再多的情况，用的真很少。
// 这样考虑的话，将文件和屏幕的分开设定不就OK了吗？
//
// 如果采用列表的形式，也简单；用push_back，pop_back分别控制即可；不过，要修改
// 特定出口的消息级别的话，则使用vector的策略为好。
//
// 有些难于取舍啊！
//
// 另外，消息级别应该设置为二进制倍增的形式——这样，用户可以决定具体消息级别的
// 开关情况；而不是现在的一网打尽；
// 当然，为了方便，最好也能提供"一网打尽"的样式。
//
// 如何调和呢？提供额外的方法？
//
// 其实也好实现，利用bit-operation即可；
//
// 不过，总体的log开关设定，可以利用环境变量；这样的话，客户端的代码不会有太大
// 的影响。也不用去判断命令行。
//
// 缺点是，软件在运行期间，仍然不能通过改变环境变量，而改变log输出行为；
//
// 直接：
// set sss::log::file=on
// set sss::log::stdout=on
// 另外，针对文件的话，还有重写与附加的区别。
//
//----------------------------------------------------------------------
// TODO 支持多线程
//
// 1. 现在的版本，并不支持多线程——就是说，如果在多线程里面使用本log功能，可能遇
// 到调用与输出前后不符的情况。
//
// 针对 SSS_LOG_FUNC_TRACE 也是如此；并且，函数栈深度的计算也可能不准确。
//
// 因为，理论上，不同线程，函数调用栈深度，应该用不同的量来统计——线程上，能绑
// 定某量吗？线程内变量？
//
// 这个需要 "线程局部存储技术" TLS
// 具体，参见：
//
// 2. 2013-03-22; 输出的时候，同时输出当前未知的消息级别；开关、调整消息级别的时
//    候，也需要输出相应信息——相当于sss::log本身的log！
// 3. 消息输出格式比较混乱。比如"宏"SSS_LOG_EXPRESSION的输出，前面有
// SSS_LOG_EXPRESSION 这个标志。其他的则没有。
//
// http://www.cnblogs.com/gogly/articles/2416362.html
//----------------------------------------------------------------------
// TODO 2015-06-26
// 在实际使用中，还有一个问题比较麻烦：本来我的 SSS_LOG_FUNC_TRACE 是用来跟踪函
// 数的出入的。但是它有几大缺陷，限制了其使用：
// 1. 对于原始代码，有侵入性——虽然可以通过 DEBUG 宏取消掉；
// 2. 对于调用发生的位置，其实并不知晓。
//    ——如果确实需要完整的知道的话，需要在函数调用处就做代码替换。
//    ——当然，还有一种办法就是，像QT的mod预处理一样，也针对需要监控的代码，做
//    一个编译前的替换。
// 3. 调用深度显示控制；
//    一个程序，不关是C还是C++，本质到最后，都是函数的调用——可以递归，也可以
//    间接递归；
//    可以看作是一棵调用树——当进程走到分支尽头的时候，又通过回栈的方式，逆推
//    回来。
//    有些分支，可能只有一两节的深度；有些，可能有N节；而日志文件，则是一个线性
//    结构；如此，很多分支的函数栈可能很深；扁平的日志文件，也很难反映这一点。
//    有两个修改策略：
//    a. 控制消息出现的深度——当深度发生变化的时候（又需要对源代码进行调整——
//    即，让日志输出部分，额外增加一个深度的参数。）
//    ——通过宏，还是能办到的；可以将这个产生，设定为一个特殊的名字；然后让日
//    志输出宏，访问它即可。
//    然后再设定一个全局变量，用来控制可以输出消息的深度；
//    b. 日志输出的时候，使用特定的标记字符——是给vim看的；利用vim的折叠功能；
//

#include <sss/bit_operation/bit_operation.h>

#include <string>
#include <vector>
#include <sstream>
#include <cstdio>

/* {{{1
　　C的printf家族包含8个成员，原型如下，

1  #include <stdio.h>
2  int printf(const char *fmt, ...);
3  int fprintf(FILE *stream, const char *fmt, ...);
4  int sprintf(char *str, const char *fmt, ...);
5  int snprintf(char *str, size_t size, const char *fmt, ...);
6
7  #include <stdarg.h>
8  int vprintf(const char *fmt, va_list ap);
9  int vfprintf(FILE *stream, const char *fmt, va_list ap);
10 int vsprintf(char *str, const char *fmt, va_list ap);
11 int vsnprintf(char *str, size_t size, const char *fmt, va_list ap);

前四个函数没有什么特殊的。后面四个v系列可以接受va_list变量，通常用在对可变参数
输出的包装，在日志记录系统中较为常用。比如下面代码，

1 enum LogLevel { log_ERROR, log_WARN, log_INFO, log_DEBUG }
2 void log(LogLevel level, const char *fmt, ...)
3 {
4     va_list ap;
5     va_start(ap, fmt);
6     vsnprintf(buf, sizeof(buf), fmt, ap);
7     va_end(ap);
8     //~ write buf to file, or do something else
9 }

　　v系列函数并不会调用va_end宏，因此在这些函数返回后，需要调用函数自己进行
va_end。若要再次解析变参列表，就需要重新va_start, va_end。

宏变参

　　除了函数，在C/C++中，带参宏定义也可以接受变参，使用方法和函数类似。比如，若
将上面的log函数的某个级别的日志输入定义成宏，

#define log_warn(fmt, ...) log(log_WARN, fmt, __VA_ARGS__)

　　__VA_ARGS__只是被预处理器简单的展开为传递给宏log_warn的变参列表，包括逗号分
隔符。若想使用具有鲜明意义的名字，而不是统一的__VA_ARGS__，可以这样，

#define log_warn(fmt, args...) log(log_WARN, fmt, args)

　　上述宏定义中，有一个问题值得注意，就是当变参列表为空时，log函数调用的参数列
表会有一个结尾的逗号，这在某些编译器中会被诊断为错误（据说MSVC不会），这种情况
下可以将fmt也纳入变参列表，

#define log_warn(...) log(log_WARN, __VA_ARGS__)

------------------------------------------------------------------------------

提到可变参数就不得不提到压栈方式，对于可变参数的成员函数，始终使用__cdecl的压栈
方式

如果你之前用typedef一个函数类型，然后用__stdcall压栈方式就会有问题的，因为两者
压栈方向是相反的，而且stdcall是完毕后是由被调用方清理的

如：

typedef char* (_stdcall SERVERFUNC)(void *, void *, int);
typedef SERVERFUNC * PSERVERFUNC;// 定义函数类型(这个是函数指针)

}}}1*/

// NOTE 几个常用的宏，方便使用。

// FIXME 进入函数，退出函数：
//#define SSS_LOG_IN_FUNC         \ -
//        sss::log::debug("step in < %s %s %d\n", __FILE__, __func__, __LINE__)
//
//#define SSS_LOG_OUT_FUNC        \ -
//        sss::log::debug("step out %s %s %d >\n", __FILE__, __func__, __LINE__)
// -- 问题在于：
//    1. 无法自动添加到每一个函数的开始与结束——这就鸡肋了。
//    2. 没有函数调用栈深度信息
//
// 另外，为了能自动显示，哪怕是退出函数的信息，也应该用对象来包裹他们——这样，
// 才能保证有很多出口（多个return，多个throw）的函数里面，也能正常输出“调用栈
// ”。
//
// 所以，该宏，应该这个样子：
//
#ifndef SSS_LOG_CLEAN
#ifndef SSS_LOG_FUNC_TRACE
#define SSS_LOG_FUNC_TRACE(level) \
        sss::log::tracer __sss_log_tracer__(level, __FILE__, __LINE__, __func__);
// NOTE 上述宏，比较适用于递归函数。另外，由于固定了变量的名字，所以，一个作用
// 域里面，这种"宏"只能使用一次。
// FIXME
// 本宏使用起来很鸡肋——它本身显示的信息比较少；其实对于比较深的函数调用，用户
// 想知道的其实是堆栈信息——进入函数，其参数是哪些？退出函数的时候，是从哪个
// return语句返回？
//
// 而我这个 SSS_LOG_FUNC_TRACE 只有文件名、函数（也能包括创建对象的所在行）。还
// 有函数深度信息。
// 如果需要记录返回时的信息，则需要额外定义宏：
// NOTE 宏的可变参数 ：__VA_ARGS__
#endif

#ifdef _SSS_LOG_MODULE_NAME_
#       ifndef SSS_LOG_IF_MODULE
#       define SSS_LOG_IF_MODULE sss::log::model(_SSS_LOG_MODULE_NAME_)
#       endif
#else
#       ifdef SSS_LOG_IF_MODULE
#       undef SSS_LOG_IF_MODULE
#       endif
#       define SSS_LOG_IF_MODULE  (true)
#endif


#ifndef SSS_LOG_FUNC_RETURN
#define SSS_LOG_FUNC_RETURN(val, ...) \
        __sss_log_tracer__.return_at(__LINE__, ##__VA_ARGS__);
#endif

#ifndef SSS_LOG_FUNC_CALL
#define SSS_LOG_FUNC_CALL(msg) \
    sss::log::func_call_msg(__FILE__, __func__, __LINE__, msg);
#endif

// TRACE 宏
#ifndef SSS_LOG_TRACE
#define SSS_LOG_TRACE(level) \
        sss::log::trace(level, "[%s:%d][%s]\n", __FILE__, __LINE__, __FUNCTION__);
#endif

// 表达式显示 -- NOTE 表达式不应该有副作用！因为以后该宏可能被 #ifdef _DEBUG 包
// 裹起来！
// 另外，下一句不能写成多行的形式——因为使用了 __FILE__, __LINE__ 等内部宏。如
// 果写成多行，那么 __LINE__ 的值在宏实体化之后，就会发生变化。
#ifndef SSS_LOG_EXPRESSION
#define SSS_LOG_EXPRESSION(level, e)                                    \
        {                                                               \
            std::ostringstream oss;                                     \
            oss << (e);                                                 \
            sss::log::trace(level, "[%s][%s:%d][%s] %s = `%s`\n",       \
                            level2name(level),                          \
                            __FILE__, __LINE__,                         \
                            __FUNCTION__, #e, oss.str().c_str());       \
        }
#endif

// TODO
// 以下几个宏，应该可以用过额外的参数来控制是否显示行号、文件名等。
// 即：宏开关 控制 log 显示风格
#ifndef SSS_LOG_ERROR
#define SSS_LOG_ERROR(fmt, args...) \
    sss::log::error("[%s][%s:%d][%s]", level2name(sss::log::log_ERROR), __FILE__, __LINE__,__FUNCTION__); \
    sss::log::write_no_time(sss::log::log_ERROR, fmt, ##args)
#endif

#ifndef SSS_LOG_WARN
#define SSS_LOG_WARN(fmt, args...) \
    sss::log::warn("[%s][%s:%d][%s]", level2name(sss::log::log_WARN), __FILE__, __LINE__,__FUNCTION__); \
    sss::log::write_no_time(sss::log::log_WARN, fmt, ##args)
#endif

#ifndef SSS_LOG_INFO
#define SSS_LOG_INFO(fmt, args...) \
    sss::log::info("[%s][%s:%d][%s]", level2name(sss::log::log_INFO), __FILE__, __LINE__,__FUNCTION__); \
    sss::log::write_no_time(sss::log::log_INFO, fmt, ##args)
#endif

#ifndef SSS_LOG_DEBUG
#define SSS_LOG_DEBUG(fmt, args...) \
    sss::log::debug("[%s][%s:%d][%s]", level2name(sss::log::log_DEBUG), __FILE__, __LINE__,__FUNCTION__); \
    sss::log::write_no_time(sss::log::log_DEBUG, fmt, ##args)
#endif

#else
#       undef SSS_LOG_FUNC_TRACE
#       define SSS_LOG_FUNC_TRACE(level)

#       undef SSS_LOG_TRACE
#       define SSS_LOG_TRACE(level)

#       undef SSS_LOG_EXPRESSION
#       define SSS_LOG_EXPRESSION(level, e)

#       undef SSS_LOG_ERROR
#       define SSS_LOG_ERROR(fmt, args...)

#       undef SSS_LOG_WARN
#       define SSS_LOG_WARN(fmt, args...)

#       undef SSS_LOG_INFO
#       define SSS_LOG_INFO(fmt, args...)

#       undef SSS_LOG_DEBUG
#       define SSS_LOG_DEBUG(fmt, args...)
#endif

namespace sss{
    namespace log {
        enum log_level {
            log_NOMSG = 0,                      // 0x00
            log_ERROR = 1,                      // 0x01
            log_WARN  = (log_ERROR << 1),       // 0x02
            log_INFO  = (log_ERROR << 2),       // 0x04
            log_DEBUG = (log_ERROR << 3)        // 0x08
        };
        const log_level log_level_min = log_NOMSG;
        const log_level log_level_max = log_DEBUG;
        const uint32_t  log_level_mask = (1u << sss::bit::highest_bit_pos(log_level_max)) - 1;

        const char * level2name(log_level);

        // 2013-02-20
        // 检测某一消息级别是否开启
        // 可用于开关用户自定义的消息输出序列。
        bool  is_level_on(log_level level);

        // 用于控制日志输出的位置
        void outto(const std::string& fname, bool append = false);
        void outto(const char * fname, bool append = false);
        // stdout 、stderr
        void outto(FILE * c_file);
        // 返回当前的日志信息输出级别

        //! 过滤形如：
        // --sss-log-level [debug |info |nomsg |error |warn]
        // --sss-log-outto log_file_name
        // 的命令行参数。
        void parse_command_line(int & argc, char * argv[]);

        //log_level level();

        // 设置新的级别；并返回以前的输出级别
        // NOTE
        // 消息日志级别，是从log_DEBUG往上的。如果设置为log_DEBUG，那么，其他所有种类的信息都将会被输出！
        // 相反，如果设置为log_ERROR，那么，只有这一级别的信息会被输出。
        // 至于，如果设置为log_NOMSG，那么，什么消息都不会被输出。
        log_level level(log_level level);

        // 程序有时候想开关部分代码区域（流程时间范围之内）的调试代码，于是有了如下：
        // 设置新的消息日志级别
        void push_level(log_level level);
        void push_mask(int m);

        // 放弃当前消息日志级别，而使用之前的消息日志级别
        void pop_level();
        void pop_mask();

        void mask(int);

        // 设置消息的时间格式；并返回之前的格式
        std::string timef(const std::string fmt);
        // 取消输出时间
        std::string timef();

        // 设置对应位置的消息级别
        bool warn (const char * fmt, ...);
        bool info (const char * fmt, ...);
        bool debug(const char * fmt, ...);
        bool error(const char * fmt, ...);

        bool trace(log_level l, const char * fmt, ...);

        bool write(log_level l, const char * fmt, ...);
        bool write_no_time(log_level l, const char * fmt, ...);


        // "是否打印时间"
        // 针对时间密集型(短时间，多次调用)的log输出，应该加上ms级别的时间。不
        // 过，这就得使用clock系列函数了——strftime是通过time函数得到的，只有
        // 秒的精度。
        bool time_stamp(bool status);
        bool time_stamp_status();

        // 开启某模块输出
        void model_on(const std::string& model_name);

        // 关闭某模块输出
        void model_off(const std::string& model_name);

        // 开关某模块输出(true:开; false 关)
        void model_trig(const std::string& model_name, bool trig);

        // 某模块是否已经启用？
        bool model(const std::string& model_name);

        void func_call_msg(const char * src, const char * func, int line, const std::string& msg);

        // 2012-11-06
        class tracer
        {
            class caller_stack_depth_recorder
            {
            public:
                caller_stack_depth_recorder() {}
                ~caller_stack_depth_recorder() {}

            public:
                inline int get_depth()
                {
                    return this->func_names.size();
                }

                const char * step_in(const char * func_name);

                const char * step_out();

            private:
                std::vector<const char*> func_names;
            };

            struct caller_info_t
            {
                caller_info_t()
                {
                    this->fcall_src = 0;
                    this->fcall_func = 0;
                    this->fcall_line = 0;
                }
                const char * fcall_src;
                const char * fcall_func;
                int          fcall_line;
                std::string  fcall_msg;
            };

        public:
            tracer(sss::log::log_level l,
                   const char * sf_name = "NULL",
                   int line_num = 0,
                   const char * func_name = "NULL");

            ~tracer();

            static caller_stack_depth_recorder & get_caller_stack_ref();

            static caller_info_t & get_caller_info()
            {
                static caller_info_t caller_info;
                return caller_info;
            }

            static void func_call_msg(const char * src, const char * func, int line, const std::string& msg)
            {
                caller_info_t& info(get_caller_info());
                info.fcall_src = src;
                info.fcall_func = func;
                info.fcall_line = line;
                info.fcall_msg = msg;
            }

        protected:

            // 报告状态
            void report();

        private:
            sss::log::log_level level;
            const char * file;
            const char * func;
            int          line;
        };
    }
} // namespace sss

#ifdef _SSS_LOG_TEST_
int main(int argc, char *argv[])
{
    sss::log::level(sss::log::log_WARN);
    sss::log::timef();
    sss::log::error("%s func %s, line %d, level %d log_ERROR\n", __FILE__, __func__, __LINE__, sss::log::log_ERROR);
    sss::log::warn("%s line %d %d log_WARN\n", __func__, __LINE__, sss::log::log_WARN);
    sss::log::info("%s line %d %d log_INFO\n", __func__, __LINE__, sss::log::log_INFO);
    sss::log::debug("%s line %d %d log_DEBUG\n", __func__, __LINE__, sss::log::log_DEBUG);
}
#endif

#endif  /* __LOG_HPP_1321928111__ */
