#include "glob_path_recursive_bf.hpp"
// #define _DEBUG_GPR_

#include <assert.h>

#include <iostream>
#include <memory>
#include <stdexcept>

#include "filter_t.hpp"

#include <sss/util/Memory.hpp>
#include <sss/log.hpp>


namespace sss {
    namespace path {

        glob_path_recursive_bf::glob_path_recursive_bf(const std::string& root,
                                                       sss::path::file_descriptor & fd) :
            _no_push(false), _jump_cur_dir(false),
            m_pfd(&fd),
            _file_filter(0), _dir_filter(0),
            _file_filter_not(false), _dir_filter_not(false),
            m_max_depth(0),
            m_root(root)
        {
            sss::path::full_of(m_root);
            this->dpush(m_root);
        }

        glob_path_recursive_bf::glob_path_recursive_bf(const std::string& root,
                                                       sss::path::file_descriptor & fd,
                                                       sss::path::filter_t * file_filter_,
                                                       bool is_exclude) :
            _no_push(false), _jump_cur_dir(false),
            m_pfd(&fd),
            _file_filter(file_filter_), _dir_filter(0),
            _file_filter_not(is_exclude), _dir_filter_not(false),
            m_max_depth(0),
            m_root(root)
        {
            sss::path::full_of(m_root);
            this->dpush(m_root);
        }

        glob_path_recursive_bf::~glob_path_recursive_bf()
        {
#ifdef _DEBUG_GPR_
            SSS_LOG_FUNC_TRACE(sss::log::log_ERROR);
#endif
        }

        bool glob_path_recursive_bf::file_filter(sss::path::filter_t * filter, bool is_exclude)
        {
            this->_file_filter = filter;
            this->_file_filter_not = is_exclude;
            return true;
        }

        bool glob_path_recursive_bf::dir_filter(sss::path::filter_t * filter, bool is_exclude)
        {
            // FIXME 在开始枚举之后
            // 不应当修改文件夹的过滤条件！
            // if (!is_first()) {
            // }
            this->_dir_filter = filter;
            this->_dir_filter_not = is_exclude;
            return true;
        }

        sss::path::filter_t * glob_path_recursive_bf::file_filter()
        {
            return this->_file_filter;
        }

        sss::path::filter_t * glob_path_recursive_bf::dir_filter()
        {
            return this->_dir_filter;
        }

        // 某一个 _stack 对象，的任何一次循环 fetch 动作，都有可能返回false——
        bool glob_path_recursive_bf::fetch()
        {
            bool ret = false;
            while (true) {
                ret = fetch_no_filter();
                if (ret) {
                    if (this->m_pfd->is_normal_file() &&
                        (!this->_file_filter || this->_file_filter_not != this->_file_filter->is_match(*m_pfd)))
                    {
                        return ret;
                    }
                    else if (this->m_pfd->is_normal_dir() &&
                             (!this->_dir_filter || this->_dir_filter_not != this->_dir_filter->is_match(*m_pfd)))
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

        bool glob_path_recursive_bf::fetch_no_filter()
        {
            if (!m_pfd->is_ok()) {
                if (this->m_this_dirs.empty()) {
                    if (this->m_next_dirs.empty()) {
                        return false;
                    }
                    std::swap(this->m_this_dirs, this->m_next_dirs);
                    this->m_depth++;
                }
                this->m_p_gp.reset(new sss::path::glob_path(m_this_dirs.back(), *m_pfd));
                this->m_this_dirs.pop_back();
            }

            if (this->m_p_gp->fetch()) {
                if (m_pfd->is_normal_dir() && this->can_dpush()) {
                    this->dpush(m_pfd->get_path());
                }
                return true;
            }
            else {
                this->m_p_gp.reset(0);
                return this->fetch_no_filter();
            }
        }

        bool glob_path_recursive_bf::dpush( const std::string & dir)
        {
            this->m_next_dirs.push_back(dir);
            return true;
        }

        void glob_path_recursive_bf::max_depth(int depth)
        {
            this->m_max_depth = depth;
        }

        bool glob_path_recursive_bf::can_dpush() const
        {
            bool ret = !_no_push && !_jump_cur_dir && (this->m_max_depth <=0 || this->depth() < this->m_max_depth);
            if (_jump_cur_dir) {
                _jump_cur_dir = false;
            }
            return ret;
        }
    }
}
