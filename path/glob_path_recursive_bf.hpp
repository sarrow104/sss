#ifndef __GLOB_PATH_RECURSIVE_BF_HPP_1455858834__
#define __GLOB_PATH_RECURSIVE_BF_HPP_1455858834__

#include "glob_path.hpp"

#include <string>
#include <vector>

#include <sss/util/Memory.hpp>

// BFs: breadth-first search

// 广度优先的递归搜索；
// NOTE
// 本质上是添加后续任务；
// 即，有可能增加太多的任务，如果一直没有找到，而用户没有终止循环的话。
// f1
// d1/
// d2/
//   f1
// d3/
//   f1
//   f2
// f2
// f3
// 如上，根目录有6个节点，分别是3个文件，3个文件夹；三个文件夹中，分别有0、1、2个文件；
//
// 按照广度优先；流程如下：
//
// 1. open .
// 2. visit f1
// 3. visit d1; push d1
// 4. visit d2; push d2
// 5. visit d3; push d3
// 6. visit f2;
// 7. visit f3
// 8. open d3
// 9. visit d3/f1
//10. visit d3/f2
//11. open d2
//12. visit d2/f1
//13. open d1
//
// 同一时间，室友一个 文件夹处于打开状态——其余的待处理的目录，都是字符串形式；
// 另外，skip动作的理解，也需要变化；深度优先模式，skip可以马上起效；而广度优先
// ，skip某一个文件夹的话，需要到下一次广度遍历……
//
// 广度优先，不再需要 glob_path 的栈结构，以保存递归状态；
// 反之，它需要两个 std::vecter<std::string>，分别用来标记，当前深度，还需要扫
// 描哪些目录(路径)；以及下一层深度，需要扫描哪些目录；
//
// 当，当前深度没有任务，并且下一层任务深度为0的时候，说明没有文件夹需要扫描了
// ，退出循环。
//
// 另外，深度优先，可以用来模拟广度优先吗？
//
// 假设，我需要查找符合某个规则的文件，有一个初始目录，接着，我按照最大深度1来遍历；
// 如果找到，则退出；没有找到，则扩大深度——暂停，这个扩大动作，如果，还是从当
// 前目录开始，就重复打开文件夹了！
//
// 还不如栈结构+循环glob_path；
//
//! dpush()与dpop()
//
// 这两个函数，貌似没用了；
// 深度优先，需要一个目录树分支一个目录树分支地扫描完毕，所以需要目录进出的操作
// ；而广度优先则不需要；
//
// 当然，这两个函数还是可以保留的——语义发生变化了；暂时供内部使用；
//
// 简言之，基础的，有三个状态需要保存：
//
// 1. 当前深度的目录列表；
// 2. 下一层深度的目录列表；
// 3. 当前打开的glob_path对象；
// 4. 深度

namespace sss {
    namespace path {

class glob_path_recursive_bf
{
public:
    glob_path_recursive_bf(const std::string& root, sss::path::file_descriptor & fd);

    glob_path_recursive_bf(const std::string& root,
                           sss::path::file_descriptor & fd,
                           sss::path::filter_t * file_filter_, bool is_exclude);

    ~glob_path_recursive_bf();

protected:
    // bool is_first()
    // {
    //     return this->_stack.empty();
    // }

    bool fetch_no_filter();

public:

    // 符合名字条件的文件；is_exclude 表示取反；即枚举不符合该条件的文件；
    bool file_filter(sss::path::filter_t * filter, bool is_exclude);

    // NOTE 文件夹的处理有些特殊；
    // 在进入某一个文件夹的时候，就需要处理这个信息——它需要与
    //   std::vector<glob_path*>     _stack
    // 同时变化；
    // 只有 不提供这个过滤器；或者匹配该过滤器的文件夹，才枚举内部的文件；
    // 但是其子文件夹，不受这个限制；
    bool dir_filter(sss::path::filter_t * filter, bool is_exclude);

    sss::path::filter_t * file_filter();

    sss::path::filter_t * dir_filter();

public:

    bool fetch();

protected:

    bool dpush( const std::string & dir);

public:
    // 控制最大深度；0，无限制；
    void max_depth(int depth);

    void no_push()                              // 调用no_push()后，即便下一个元素是目录类型也不进入
    {
        this->_no_push = true;
    }
    void auto_push()                            // 调用auto_push()后，下一个元素是目录类型，就会进入
    {
        this->_no_push = false;
    }

    void auto_push(bool status)
    {
        this->_no_push = !status;
    }

    int depth() const
    {
        return this->m_depth;
    }

    // 越过当前——如果当前是目录，则skip掉；如果当前是文件，则越过下一个？
    void jump()
    {
        // TODO
        if (this->m_pfd->is_normal_dir()) {
            //std::cout << "jump at `" << this->m_pfd->get_path() << "`" << std::endl;
            this->_jump_cur_dir = true;
        }
    }

protected:
    /**
     * @brief 获取当前目录的下一个节点
     *
     * @return 是否成功
     */
    bool can_dpush() const;

private:
    glob_path_recursive_bf(const glob_path_recursive_bf&);
    glob_path_recursive_bf& operator=(const glob_path_recursive_bf&);

protected:
    bool                        _no_push;
    mutable bool                _jump_cur_dir;
    sss::path::file_descriptor* m_pfd;

    sss::path::filter_t *       _file_filter;
    sss::path::filter_t *       _dir_filter;
    bool                        _file_filter_not;
    bool                        _dir_filter_not;

    int                         m_max_depth;
    int                         m_depth;

    std::string                 m_root;
    sss::scoped_ptr<glob_path>  m_p_gp;            // 递归变循环 关键栈结构
    std::vector<std::string>    m_this_dirs;          // 递归变循环 关键栈结构
    std::vector<std::string>    m_next_dirs;         // 递归变循环 关键栈结构
};
    }
}


#endif /* __GLOB_PATH_RECURSIVE_BF_HPP_1455858834__ */
