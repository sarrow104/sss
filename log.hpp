#ifndef  __LOG_HPP_1321928111__
#define  __LOG_HPP_1321928111__

// TODO
// �û����Ծ��������ּ������Ϣ��������ĸ�(���ļ���)���ڡ�
// ��ǰ��ֻ�ܶ���һ����Ϣ���ڣ������ļ�������stdout��
//
// ��������趨��
//
// ���ȣ�����һ��FILE ���б�Ȼ��ÿ���б��Ӧһ����Ϣ����
//
// ����������б�Ҫ�ṩ�б����ʽ��
//
// ��ʵ�û������Ҫ������������ļ��ģ�����Ļ�ģ��ٶ��������õ�����١�
// �������ǵĻ������ļ�����Ļ�ķֿ��趨����OK����
//
// ��������б����ʽ��Ҳ�򵥣���push_back��pop_back�ֱ���Ƽ��ɣ�������Ҫ�޸�
// �ض����ڵ���Ϣ����Ļ�����ʹ��vector�Ĳ���Ϊ�á�
//
// ��Щ����ȡ�ᰡ��
//
// ���⣬��Ϣ����Ӧ������Ϊ�����Ʊ�������ʽ�����������û����Ծ���������Ϣ�����
// ������������������ڵ�һ���򾡣�
// ��Ȼ��Ϊ�˷��㣬���Ҳ���ṩ"һ����"����ʽ��
//
// ��ε����أ��ṩ����ķ�����
//
// ��ʵҲ��ʵ�֣�����bit-operation���ɣ�
//
// �����������log�����趨���������û��������������Ļ����ͻ��˵Ĵ��벻����̫��
// ��Ӱ�졣Ҳ����ȥ�ж������С�
//
// ȱ���ǣ�����������ڼ䣬��Ȼ����ͨ���ı价�����������ı�log�����Ϊ��
//
// ֱ�ӣ�
// set sss::log::file=on
// set sss::log::stdout=on
// ���⣬����ļ��Ļ���������д�븽�ӵ�����
//
//----------------------------------------------------------------------
// TODO ֧�ֶ��߳�
//
// 1. ���ڵİ汾������֧�ֶ��̡߳�������˵������ڶ��߳�����ʹ�ñ�log���ܣ�������
// �����������ǰ�󲻷��������
//
// ��� SSS_LOG_FUNC_TRACE Ҳ����ˣ����ң�����ջ��ȵļ���Ҳ���ܲ�׼ȷ��
//
// ��Ϊ�������ϣ���ͬ�̣߳���������ջ��ȣ�Ӧ���ò�ͬ������ͳ�ơ����߳��ϣ��ܰ�
// ��ĳ�����߳��ڱ�����
//
// �����Ҫ "�ֲ߳̾��洢����" TLS
// ���壬�μ���
//
// 2. 2013-03-22; �����ʱ��ͬʱ�����ǰδ֪����Ϣ���𣻿��ء�������Ϣ�����ʱ
//    ��Ҳ��Ҫ�����Ӧ��Ϣ�����൱��sss::log�����log��
// 3. ��Ϣ�����ʽ�Ƚϻ��ҡ�����"��"SSS_LOG_EXPRESSION�������ǰ����
// SSS_LOG_EXPRESSION �����־����������û�С�
//
// http://www.cnblogs.com/gogly/articles/2416362.html
//----------------------------------------------------------------------
// TODO 2015-06-26
// ��ʵ��ʹ���У�����һ������Ƚ��鷳�������ҵ� SSS_LOG_FUNC_TRACE ���������ٺ�
// ���ĳ���ġ��������м���ȱ�ݣ���������ʹ�ã�
// 1. ����ԭʼ���룬�������ԡ�����Ȼ����ͨ�� DEBUG ��ȡ������
// 2. ���ڵ��÷�����λ�ã���ʵ����֪����
//    �������ȷʵ��Ҫ������֪���Ļ�����Ҫ�ں������ô����������滻��
//    ������Ȼ������һ�ְ취���ǣ���QT��modԤ����һ����Ҳ�����Ҫ��صĴ��룬��
//    һ������ǰ���滻��
// 3. ���������ʾ���ƣ�
//    һ�����򣬲�����C����C++�����ʵ���󣬶��Ǻ����ĵ��á������Եݹ飬Ҳ����
//    ��ӵݹ飻
//    ���Կ�����һ�õ����������������ߵ���֧��ͷ��ʱ����ͨ����ջ�ķ�ʽ������
//    ������
//    ��Щ��֧������ֻ��һ���ڵ���ȣ���Щ��������N�ڣ�����־�ļ�������һ������
//    �ṹ����ˣ��ܶ��֧�ĺ���ջ���ܺ����ƽ����־�ļ���Ҳ���ѷ�ӳ��һ�㡣
//    �������޸Ĳ��ԣ�
//    a. ������Ϣ���ֵ���ȡ�������ȷ����仯��ʱ������Ҫ��Դ������е�������
//    ��������־������֣���������һ����ȵĲ�������
//    ����ͨ���꣬�����ܰ쵽�ģ����Խ�����������趨Ϊһ����������֣�Ȼ������
//    ־����꣬���������ɡ�
//    Ȼ�����趨һ��ȫ�ֱ������������ƿ��������Ϣ����ȣ�
//    b. ��־�����ʱ��ʹ���ض��ı���ַ������Ǹ�vim���ģ�����vim���۵����ܣ�
//

#include <sss/bit_operation/bit_operation.h>

#include <string>
#include <vector>
#include <sstream>
#include <cstdio>

/* {{{1
����C��printf�������8����Ա��ԭ�����£�

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

ǰ�ĸ�����û��ʲô����ġ������ĸ�vϵ�п��Խ���va_list������ͨ�����ڶԿɱ����
����İ�װ������־��¼ϵͳ�н�Ϊ���á�����������룬

1 enum LogLevel { log_ERROR, log_WARN, log_INFO, log_DEBUG }
2 void log(LogLevel level, const char *fmt, ...)
3 {
4     va_list ap;
5     va_start(ap, fmt);
6     vsnprintf(buf, sizeof(buf), fmt, ap);
7     va_end(ap);
8     //~ write buf to file, or do something else
9 }

����vϵ�к������������va_end�꣬�������Щ�������غ���Ҫ���ú����Լ�����
va_end����Ҫ�ٴν�������б�����Ҫ����va_start, va_end��

����

�������˺�������C/C++�У����κ궨��Ҳ���Խ��ܱ�Σ�ʹ�÷����ͺ������ơ����磬��
�������log������ĳ���������־���붨��ɺ꣬

#define log_warn(fmt, ...) log(log_WARN, fmt, __VA_ARGS__)

����__VA_ARGS__ֻ�Ǳ�Ԥ�������򵥵�չ��Ϊ���ݸ���log_warn�ı���б��������ŷ�
����������ʹ�þ���������������֣�������ͳһ��__VA_ARGS__������������

#define log_warn(fmt, args...) log(log_WARN, fmt, args)

���������궨���У���һ������ֵ��ע�⣬���ǵ�����б�Ϊ��ʱ��log�������õĲ�����
�����һ����β�Ķ��ţ�����ĳЩ�������лᱻ���Ϊ���󣨾�˵MSVC���ᣩ���������
�¿��Խ�fmtҲ�������б�

#define log_warn(...) log(log_WARN, __VA_ARGS__)

------------------------------------------------------------------------------

�ᵽ�ɱ�����Ͳ��ò��ᵽѹջ��ʽ�����ڿɱ�����ĳ�Ա������ʼ��ʹ��__cdecl��ѹջ
��ʽ

�����֮ǰ��typedefһ���������ͣ�Ȼ����__stdcallѹջ��ʽ�ͻ�������ģ���Ϊ����
ѹջ�������෴�ģ�����stdcall����Ϻ����ɱ����÷������

�磺

typedef char* (_stdcall SERVERFUNC)(void *, void *, int);
typedef SERVERFUNC * PSERVERFUNC;// ���庯������(����Ǻ���ָ��)

}}}1*/

// NOTE �������õĺ꣬����ʹ�á�

// FIXME ���뺯�����˳�������
//#define SSS_LOG_IN_FUNC         \ -
//        sss::log::debug("step in < %s %s %d\n", __FILE__, __func__, __LINE__)
//
//#define SSS_LOG_OUT_FUNC        \ -
//        sss::log::debug("step out %s %s %d >\n", __FILE__, __func__, __LINE__)
// -- �������ڣ�
//    1. �޷��Զ���ӵ�ÿһ�������Ŀ�ʼ�����������ͼ����ˡ�
//    2. û�к�������ջ�����Ϣ
//
// ���⣬Ϊ�����Զ���ʾ���������˳���������Ϣ��ҲӦ���ö������������ǡ���������
// ���ܱ�֤�кܶ���ڣ����return�����throw���ĺ������棬Ҳ���������������ջ
// ����
//
// ���ԣ��ú꣬Ӧ��������ӣ�
//
#ifndef SSS_LOG_CLEAN
#ifndef SSS_LOG_FUNC_TRACE
#define SSS_LOG_FUNC_TRACE(level) \
        sss::log::tracer __sss_log_tracer__(level, __FILE__, __LINE__, __func__);
// NOTE �����꣬�Ƚ������ڵݹ麯�������⣬���ڹ̶��˱��������֣����ԣ�һ������
// �����棬����"��"ֻ��ʹ��һ�Ρ�
// FIXME
// ����ʹ�������ܼ��ߡ�����������ʾ����Ϣ�Ƚ��٣���ʵ���ڱȽ���ĺ������ã��û�
// ��֪������ʵ�Ƕ�ջ��Ϣ�������뺯�������������Щ���˳�������ʱ���Ǵ��ĸ�
// return��䷵�أ�
//
// ������� SSS_LOG_FUNC_TRACE ֻ���ļ�����������Ҳ�ܰ�����������������У�����
// �к��������Ϣ��
// �����Ҫ��¼����ʱ����Ϣ������Ҫ���ⶨ��꣺
// NOTE ��Ŀɱ���� ��__VA_ARGS__
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

// TRACE ��
#ifndef SSS_LOG_TRACE
#define SSS_LOG_TRACE(level) \
        sss::log::trace(level, "[%s:%d][%s]\n", __FILE__, __LINE__, __FUNCTION__);
#endif

// ���ʽ��ʾ -- NOTE ���ʽ��Ӧ���и����ã���Ϊ�Ժ�ú���ܱ� #ifdef _DEBUG ��
// ��������
// ���⣬��һ�䲻��д�ɶ��е���ʽ������Ϊʹ���� __FILE__, __LINE__ ���ڲ��ꡣ��
// ��д�ɶ��У���ô __LINE__ ��ֵ�ں�ʵ�廯֮�󣬾ͻᷢ���仯��
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
// ���¼����꣬Ӧ�ÿ����ù�����Ĳ����������Ƿ���ʾ�кš��ļ����ȡ�
// �����꿪�� ���� log ��ʾ���
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
        // ���ĳһ��Ϣ�����Ƿ���
        // �����ڿ����û��Զ������Ϣ������С�
        bool  is_level_on(log_level level);

        // ���ڿ�����־�����λ��
        void outto(const std::string& fname, bool append = false);
        void outto(const char * fname, bool append = false);
        // stdout ��stderr
        void outto(FILE * c_file);
        // ���ص�ǰ����־��Ϣ�������

        //! �������磺
        // --sss-log-level [debug |info |nomsg |error |warn]
        // --sss-log-outto log_file_name
        // �������в�����
        void parse_command_line(int & argc, char * argv[]);

        //log_level level();

        // �����µļ��𣻲�������ǰ���������
        // NOTE
        // ��Ϣ��־�����Ǵ�log_DEBUG���ϵġ��������Ϊlog_DEBUG����ô�����������������Ϣ�����ᱻ�����
        // �෴���������Ϊlog_ERROR����ô��ֻ����һ�������Ϣ�ᱻ�����
        // ���ڣ��������Ϊlog_NOMSG����ô��ʲô��Ϣ�����ᱻ�����
        log_level level(log_level level);

        // ������ʱ���뿪�ز��ִ�����������ʱ�䷶Χ֮�ڣ��ĵ��Դ��룬�����������£�
        // �����µ���Ϣ��־����
        void push_level(log_level level);
        void push_mask(int m);

        // ������ǰ��Ϣ��־���𣬶�ʹ��֮ǰ����Ϣ��־����
        void pop_level();
        void pop_mask();

        void mask(int);

        // ������Ϣ��ʱ���ʽ��������֮ǰ�ĸ�ʽ
        std::string timef(const std::string fmt);
        // ȡ�����ʱ��
        std::string timef();

        // ���ö�Ӧλ�õ���Ϣ����
        bool warn (const char * fmt, ...);
        bool info (const char * fmt, ...);
        bool debug(const char * fmt, ...);
        bool error(const char * fmt, ...);

        bool trace(log_level l, const char * fmt, ...);

        bool write(log_level l, const char * fmt, ...);
        bool write_no_time(log_level l, const char * fmt, ...);


        // "�Ƿ��ӡʱ��"
        // ���ʱ���ܼ���(��ʱ�䣬��ε���)��log�����Ӧ�ü���ms�����ʱ�䡣��
        // ������͵�ʹ��clockϵ�к����ˡ���strftime��ͨ��time�����õ��ģ�ֻ��
        // ��ľ��ȡ�
        bool time_stamp(bool status);
        bool time_stamp_status();

        // ����ĳģ�����
        void model_on(const std::string& model_name);

        // �ر�ĳģ�����
        void model_off(const std::string& model_name);

        // ����ĳģ�����(true:��; false ��)
        void model_trig(const std::string& model_name, bool trig);

        // ĳģ���Ƿ��Ѿ����ã�
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

            // ����״̬
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
