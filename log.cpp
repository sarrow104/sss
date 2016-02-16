#include <sss/log.hpp>

#include <cstdio>
#include <cstdarg>
#include <string>
#include <vector>
#include <cassert>
#include <map>

#include <sss/time.hpp>
#include <sss/CMLParser.hpp>

namespace sss{
    namespace log {
        // NOTE
        // 慎用全局变量
        // 最好用函数内的static对象包裹；
        // 以便控制构造时机；
        // static const char * fcall_src  = "NULL";
        // static const char * fcall_func = "NULL";
        // static int          fcall_line = 0;
        // static std::string  fcall_msg = "";

        void func_call_msg(const char * src, const char * func, int line, const std::string& msg)
        {
            tracer::func_call_msg(src, func, line, msg);
        }

        class LogSetting
        {
        public:
            LogSetting()
                : _time_fmt("%Y-%m-%d %H:%M:%S"),
                  // _level(log_ERROR),
                  _cfile(stdout),
                  _time_stamp_status(true)
            {
                //this->_levels.push_back(log_ERROR);
                this->level(log_ERROR);
            }
            ~LogSetting() {}

        protected:
            inline
            static int make_mask(log_level level)
            {
                // NOTE 当level = log_NOMSG （即0）的时候，下面的算式计算结果，也是0！
                // TODO 另外，level 的取值范围也应该检测一下：level应该是能被2整除的数！
                return (1u << sss::bit::highest_bit_pos(level)) - 1;
            }
            inline
            static int get_default_mask()
            {
                return LogSetting::make_mask(sss::log::log_ERROR);
            }

            int & get_last_mask()
            {
                if (!this->_masks.size())
                    this->_masks.push_back(LogSetting::get_default_mask());
                return *(this->_masks.rbegin());
            }

        public:
            void set_mask(int m)
            {
                this->get_last_mask() = (m & sss::log::log_level_mask);
                //this->_mask = (m & sss::log::log_level_mask);
            }

            log_level level(log_level level)
            {
                assert(level >= log_NOMSG && level <= log_level_max);
                this->set_mask(LogSetting::make_mask(level));
                return level;
            }

            void push_mask(int m)
            {
                this->_masks.push_back(m & sss::log::log_level_mask);
            }
            void push_level(log_level level)
            {
                this->push_mask(LogSetting::make_mask(level));
            }

            // 放弃当前消息日志级别
            void pop_mask()
            {
                if (this->_masks.size() > 1)
                {
                    this->_masks.pop_back();
                }
            }
            void pop_level()
            {
                this->pop_mask();
            }

            bool is_level_on(log_level level)
            {
                return this->get_last_mask() & level;
            }

            FILE * c_file() const
            {
                return this->_cfile;
            }
            FILE * c_file(FILE * c_file)
            {
                FILE * prev_c_file = this->_cfile;
                this->_cfile = c_file;
                return prev_c_file;
            }
            //log_level level() const
            //{
            //    return this->_level;
            //}

            std::string timef() const
            {
                return this->_time_fmt;
            }

            std::string timef(const std::string& time_fmt)
            {
                std::string prev_time_fmt = this->_time_fmt;
                this->_time_fmt = time_fmt;
                return prev_time_fmt;
            }

            bool time_stamp(bool status)
            {
                bool prev_status = this->_time_stamp_status;
                this->_time_stamp_status = status;
                return prev_status;
            }

            bool time_stamp() const
            {
                return this->_time_stamp_status;
            }

            bool is_timf_valid() const
            {
                return this->_time_fmt.length();
            }
            void write(log_level level, const char * fmt, va_list ap)
            {
                assert(level >= log_ERROR && level <= log_level_max);
                if (this->_time_stamp_status && this->is_level_on(level))
                {
                    if (this->is_timf_valid())
                    {
                        fprintf(this->_cfile, "%s ", sss::time::strftime(this->_time_fmt).c_str());
                    }
                    //va_list ap;
                    //va_start(ap, fmt);
                    vfprintf(this->_cfile, fmt, ap);
                    //va_end(ap);
                    fflush(this->_cfile);
                }
            }

            void write_no_time(log_level level, const char * fmt, va_list ap)
            {
                assert(level >= log_ERROR && level <= log_level_max);
                if (this->is_level_on(level))
                {
                    //va_list ap;
                    //va_start(ap, fmt);
                    vfprintf(this->_cfile, fmt, ap);
                    //va_end(ap);
                    fflush(this->_cfile);
                }
            }

            void model_trig(const std::string& model_name, bool trig)
            {
                this->_model_status[model_name] = trig;
            }

            bool model(const std::string& model_name) const
            {
                std::map<std::string, bool>::const_iterator it =
                    this->_model_status.find(model_name);
                if (it != this->_model_status.end()) {
                    return it->second;
                }
                else {
                    return false;
                }
            }

        private:
            std::string _time_fmt;
            //log_level   _level;
            std::vector<int>       _masks;
            std::FILE * _cfile;
            bool        _time_stamp_status;
            std::map<std::string, bool> _model_status;
        };

        static LogSetting& get_log_setting()
        {
            static LogSetting log_setting;
            return log_setting;
        }

        bool  is_level_on(log_level level)
        {
            return get_log_setting().is_level_on(level);
        }

        // 用于控制日志输出的位置
        void outto(const std::string& fname, bool append)
        {
            outto(fname.c_str(), append);
        }
        void outto(const char * fname, bool append)
        {
            const char * property = "w";
            if (append)
            {
                property = "a";
            }
            get_log_setting().c_file(fopen(fname, property));
        }
        // STDOUT 、STDERR
        void outto(FILE * c_file)
        {
            get_log_setting().c_file(c_file);
        }

        void parse_command_line(int & argc, char * argv[])
        {
            // 2013-04-04
            sss::CMLParser::RuleSingleValue lvl;
            lvl.add_option("debug", sss::log::log_DEBUG);
            lvl.add_option("info", sss::log::log_INFO);
            lvl.add_option("nomsg", sss::log::log_NOMSG);
            lvl.add_option("error", sss::log::log_ERROR);
            lvl.add_option("warn", sss::log::log_WARN);

            sss::CMLParser::RuleSingleValue out;

            sss::CMLParser::Exclude parser;
            parser.add_rule("--sss-log-level", sss::CMLParser::ParseBase::r_parameter, lvl);
            parser.add_rule("--sss-log-outto", sss::CMLParser::ParseBase::r_parameter, out);

            parser.parse(argc, argv);
            if (lvl.size())
            {
                sss::log::level(lvl.get_id<sss::log::log_level>(0));
            }

            SSS_LOG_DEBUG("after lvl.size() %d %s\n", lvl.size(), lvl.get(0) ? lvl.get(0) : "NULL");

            if (out.size())
            {
                const char * file = out.get(0);
                if (file && file[0])
                {
                    sss::log::outto(file);
                }
            }
        }

        // 设置新的级别；并返回新的级别
        log_level level(log_level level)
        {
            return get_log_setting().level(level);
        }
        void mask(int m)
        {
            get_log_setting().set_mask(m);
        }
        void push_level(log_level level)
        {
            get_log_setting().push_level(level);
        }
        void push_mask(int m)
        {
            get_log_setting().push_mask(m);
        }

        // 放弃当前消息日志级别
        void pop_level()
        {
            get_log_setting().pop_level();
        }
        void pop_mask()
        {
            get_log_setting().pop_mask();
        }

        // 设置消息的时间格式；并返回之前的格式
        std::string timef(const std::string fmt)
        {
            return get_log_setting().timef(fmt);
        }
        // 取消输出时间
        std::string timef()
        {
            return get_log_setting().timef("");
        }

        bool time_stamp(bool status)
        {
            return get_log_setting().time_stamp(status);
        }

        bool time_stamp_status()
        {
            return get_log_setting().time_stamp();
        }

        // 开启某模块输出
        void model_on(const std::string& model_name)
        {
            get_log_setting().model_trig(model_name, true);
        }

        // 关闭某模块输出
        void model_off(const std::string& model_name)
        {
            get_log_setting().model_trig(model_name, false);
        }

        // 开关某模块输出(true:开; false 关)
        void model_trig(const std::string& model_name, bool trig)
        {
            get_log_setting().model_trig(model_name, trig);
        }

        // 某模块是否已经启用？
        bool model(const std::string& model_name)
        {
            return get_log_setting().model(model_name);
        }

        // 设置对应位置的消息级别
        bool warn (const char * fmt, ...)
        {
            va_list ap;
            va_start(ap, fmt);
            get_log_setting().write(log_WARN, fmt, ap);
            va_end(ap);
            return true;
        }
        bool info (const char * fmt, ...)
        {
            va_list ap;
            va_start(ap, fmt);
            get_log_setting().write(log_INFO, fmt, ap);
            va_end(ap);
            return true;
        }
        bool debug(const char * fmt, ...)
        {
            va_list ap;
            va_start(ap, fmt);
            get_log_setting().write(log_DEBUG, fmt, ap);
            va_end(ap);
            return true;
        }
        bool error(const char * fmt, ...)
        {
            va_list ap;
            va_start(ap, fmt);
            get_log_setting().write(log_ERROR, fmt, ap);
            va_end(ap);
            return true;
        }
        bool trace(log_level l, const char * fmt, ...)
        {
            va_list ap;
            va_start(ap, fmt);
            get_log_setting().write(l, fmt, ap);
            va_end(ap);
            return true;
        }

        bool write(log_level l, const char * fmt, ...)
        {
            va_list ap;
            va_start(ap, fmt);
            get_log_setting().write(l, fmt, ap);
            va_end(ap);
            return true;
        }

        bool write_no_time(log_level l, const char * fmt, ...)
        {
            va_list ap;
            va_start(ap, fmt);
            get_log_setting().write_no_time(l, fmt, ap);
            va_end(ap);
            return true;
        }

        const char * level2name(log_level l)
        {
            const char * ret = "NULL";
            switch (l) {
            case log_NOMSG: ret = "nomsg"; break;
            case log_ERROR: ret = "error"; break;
            case log_WARN : ret = "warn";  break;
            case log_INFO : ret = "info";  break;
            case log_DEBUG: ret = "debug"; break;
            default: break;
            }
            return ret;
        }

        tracer::tracer(sss::log::log_level l,
                       const char * sf_name,
                       int line_num,
                       const char * func_name)
                : level(l), file(sf_name), func(func_name), line(line_num)
            {
                //int pre_depth = get_caller_stack_ref().get_depth();
                const char * pre_func = get_caller_stack_ref().step_in(func_name);
                pre_func = pre_func ? pre_func : "/";
                int depth = get_caller_stack_ref().get_depth();
                sss::log::trace(this->level,
                                "[%s][%s:%d]; FUNC_TRACE.depth [%d -> %d]; %s() -> %s() {{{%d\n",
                                level2name(level),
                                this->file, this->line,
                                depth - 1, depth,
                                pre_func, this->func,
                                depth);
                if (!get_caller_info().fcall_msg.empty()) {
                    sss::log::trace(this->level,
                                    "\t\t\t from [%s:%d];(%s)\n",
                                    get_caller_info().fcall_src ? get_caller_info().fcall_src : "NULL",
                                    get_caller_info().fcall_line,
                                    get_caller_info().fcall_msg.c_str());
                    get_caller_info().fcall_msg.clear();
                }
            }

        tracer::~tracer()
        {
            const char * pre_func = get_caller_stack_ref().step_out();
            pre_func = pre_func ? pre_func : "NULL";
            int depth = get_caller_stack_ref().get_depth();
            sss::log::trace(this->level,
                            "[%s][%s:%d]; FUNC_TRACE.depth [%d <- %d]; %s() <- %s() }}}%d\n",
                            level2name(level),
                            this->file, this->line,
                            depth, depth + 1,
                            pre_func, this->func,
                            depth + 1);
        }

        tracer::caller_stack_depth_recorder & tracer::get_caller_stack_ref()
        {
            static caller_stack_depth_recorder recorder;
            return recorder;
        }

        // 报告状态
        void tracer::report()
        {
            // TODO
        }

        const char * tracer::caller_stack_depth_recorder::step_in(const char * func_name)
        {
            const char * top = 0;
            if (this->func_names.size())
            {
                top = *this->func_names.rbegin();
            }
            this->func_names.push_back(func_name);
            return top;
        }

        const char * tracer::caller_stack_depth_recorder::step_out()
        {
            const char * top = 0;
            if (this->func_names.size())
            {
                this->func_names.pop_back();
                if (!this->func_names.empty()) {
                    top = *this->func_names.rbegin();
                }
                else {
                    top = "/";
                }
            }
            return top;
        }

    } // namespace log
} // namespace sss

