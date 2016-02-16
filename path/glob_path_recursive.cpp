#include "glob_path_recursive.hpp"
// #define _DEBUG_GPR_

#include <assert.h>

#include <iostream>
#include <memory>

#include "filter_t.hpp"

#include <sss/util/Memory.hpp>
#include <sss/log.hpp>


namespace sss {
    namespace path {

        glob_path_recursive::glob_path_recursive(const std::string& root,
                                                 sss::path::file_descriptor & fd)
            : _next_pop(false), _no_push(false), _jump_cur_dir(false),
            _pfd(&fd),
            _file_filter(0), _dir_filter(0),
            _file_filter_not(false), _dir_filter_not(false),
            _max_depth(0),
            _root(root)
        {
            sss::path::full_of(_root);
            this->dpush(_root);
        }

        glob_path_recursive::glob_path_recursive(const std::string& root,
                                                 sss::path::file_descriptor & fd,
                                                 sss::path::filter_t * file_filter_, bool is_exclude)
            : _next_pop(false), _no_push(false), _jump_cur_dir(false),
            _pfd(&fd),
            _file_filter(file_filter_), _dir_filter(0),
            _file_filter_not(is_exclude), _dir_filter_not(false),
            _max_depth(0),
            _root(root)
        {
            sss::path::full_of(_root);
            this->dpush(_root);
        }

        glob_path_recursive::~glob_path_recursive()
        {
#ifdef _DEBUG_GPR_
            SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
#endif
            while (!this->_stack.empty()) {
                this->dpop();
            }
        }

        bool glob_path_recursive::file_filter(sss::path::filter_t * filter, bool is_exclude)
        {
            this->_file_filter = filter;
            this->_file_filter_not = is_exclude;
            return true;
        }

        bool glob_path_recursive::dir_filter(sss::path::filter_t * filter, bool is_exclude)
        {
            // FIXME 在开始枚举之后
            // 不应当修改文件夹的过滤条件！
            // if (!is_first()) {
            // }
            this->_dir_filter = filter;
            this->_dir_filter_not = is_exclude;
            return true;
        }

        sss::path::filter_t * glob_path_recursive::file_filter()
        {
            return this->_file_filter;
        }

        sss::path::filter_t * glob_path_recursive::dir_filter()
        {
            return this->_dir_filter;
        }

        bool glob_path_recursive::is_enum_current()
        {
            //std::string dirname = sss::path::basename(path);
            return
                !this->_dir_filter ||
                (this->_dir_filter_not != this->_dir_filter->is_match(*_pfd));
        }

        // 某一个 _stack 对象，的任何一次循环 fetch 动作，都有可能返回false——
        // 此时，栈还非空！
        // 而本来，我这个fetch_no_filter，目的是始终获取到一个合法的
        // file_descriptor对象——除非，_stack 空了！
        // 从逻辑上来说，每一个 cur_next() 调用，都要进行上述判断——fetch动作
        // ，使得当前 !file_descriptor::is_ok() ，那么，就需要判断是否 当前
        // !glob_path::good() 也是如此；
        bool glob_path_recursive::fetch()
        {
#ifdef _DEBUG_GPR_
            SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
#endif
            bool ret = false;
            while (true) {
                while (!this->_stack.empty()) {
                    ret = fetch_no_filter();
                    if (this->_stack.empty() || this->_stack.back()->good()) {
                        break;
                    }
                    this->dpop();
                }
                if (ret) {
                    //std::cout
                    //    << _pfd->get_name()
                    //    << "|E:" << this->_stack_exclude.back() // 1
                    //    << "|F:" << this->_pfd->is_normal_file() // 1
                    //    << "|D:" << this->_pfd->is_normal_dir()  // 0
                    //    << "|f:" << bool(this->_file_filter)   // 1
                    //    << "|n:" << this->_file_filter_not     // 0
                    //    << "|():" << (!this->_file_filter || this->_file_filter_not != this->_file_filter->is_match(*_pfd)) // 1
                    //    << "|:" << (this->_stack_exclude.back() && this->_pfd->is_normal_dir())
                    //    << "|Out:" << (this->_stack_exclude.back() && this->_pfd->is_normal_dir() &&
                    //                  (!this->_file_filter || this->_file_filter_not != this->_file_filter->is_match(*_pfd)))
                    //    << std::endl;
                    if (!this->depth()) {
                        throw std::runtime_error("empty stack");
                    }
                    if (this->_stack_exclude.back() && this->_pfd->is_normal_file() &&
                        (!this->_file_filter || this->_file_filter_not != this->_file_filter->is_match(*_pfd)))
                    {
                        return ret;
                    }
                    else if (this->_pfd->is_normal_dir() &&
                             (!this->_dir_filter || this->_dir_filter_not != this->_dir_filter->is_match(*_pfd)))
                    {
                        return ret;
                    }
                    continue;
                }
                else {
                    return ret;
                }
            }
        }

        bool glob_path_recursive::fetch_no_filter()
        {
#ifdef _DEBUG_GPR_
            SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
#endif
            // SSS_LOG_ERROR("here!\n");
            if (!_pfd->is_ok()) {
                return this->cur_next();
            }
            else {
                if (this->_next_pop) {
#ifdef _DEBUG_GPR_
                    std::cout << "dpop from this->_next_pop " << __FILE__ << " " << __LINE__ << std::endl;
#endif
                    this->dpop();
                    return this->cur_next();
                }
                if (_pfd->is_normal_dir() && this->can_dpush()) {
                    if (this->dpush(_pfd->get_path())) {
                        return this->cur_next();
                    }
                }

                bool is_valid = false;

                do {
                    if ((is_valid = this->cur_next())) {
                        break;
                    }
#ifdef _DEBUG_GPR_
                    std::cout << "dpop from while loop " << __FILE__ << " " << __LINE__ << std::endl;
#endif
                    this->dpop();
                } while (this->depth());

                assert (is_valid == this->_pfd->is_ok());
                return is_valid;
            }
        }

        bool glob_path_recursive::dpush( const std::string & dir)
        {
#ifdef _DEBUG_GPR_
            //SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
            std::cout << __func__ << "(" << dir << ")";
#endif
            this->_jump_cur_dir = false;
            bool tmp_is_enum = is_enum_current();
            sss::scoped_ptr<glob_path> tmp_gp(new glob_path(dir, *_pfd));
            if (tmp_gp.get() && tmp_gp->good()) {
                this->_stack_exclude.push_back(tmp_is_enum);
                this->_stack.push_back(tmp_gp.release());
#ifdef _DEBUG_GPR_
                std::cout << " succeed." << std::endl;
#endif
                return true;
            }
            else {
#ifdef _DEBUG_GPR_
                std::cout << " failed." << std::endl;
#endif
                return false;
            }
        }

        bool glob_path_recursive::dpop ()
        {
#ifdef _DEBUG_GPR_
            SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
#endif
            if (this->_stack.empty()) {
                return false;
            }

            this->_next_pop = false;
            //SSS_LOG_EXPRESSION(sss::log::log_ERROR, this->_stack.back()->basedir());

            delete this->_stack.back();
            this->_stack.pop_back();
            this->_stack_exclude.pop_back();
            if (!this->_stack.empty()) {
                this->_pfd->bind(*this->_stack.back());
            }

            return true;
        }

        void glob_path_recursive::max_depth(int depth)
        {
            this->_max_depth = depth;
        }

        bool glob_path_recursive::cur_next() const
        {
            bool is_valid = !this->_stack.empty() && this->_stack.back()->fetch();
            return is_valid;
        }

        bool glob_path_recursive::can_dpush() const
        {
            bool ret = !_no_push && !_jump_cur_dir && (this->_max_depth <=0 || this->depth() < this->_max_depth);
            if (_jump_cur_dir) {
                // std::cout << "jump reverse at `" << this->_pfd->get_path() << "`" << std::endl;
                _jump_cur_dir = false;
            }
            return ret;
        }
    }
}
