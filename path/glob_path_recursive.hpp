#ifndef  __GLOB_RECURSIVE_HPP_1440646821__
#define  __GLOB_RECURSIVE_HPP_1440646821__

#include "glob_path.hpp"

#include <string>
#include <vector>

//! 递归枚举如何实现？
//  内部，肯定需要多次 opendir，来获取 dirent 对象——注意，这个对象是指针，不
//  能简单地复制拷贝！
//  hFind 也是一样；
//  这意味着，我的 glob_path 对象，不适用于stl容器！
//  要知道，我需要一个类似 std::vector<glob_path>的容器！
//  当然，还有一个办法，就是让glob_path本身，可以串接——他们本身就是链表的节点；
//  最后，还有一个傻办法，就是使用std::vector<glob_path*> ……
//
//  关于 file_descriptor 生命周期问题
//  file_descriptor 本质上，是一个 handle；其内存资源，来自于 glob_path ——
//  我并没有拷贝出来；
//  这意味着，我一旦delete 某一个 opendir 得来的对象，我的file_descriptor就会失效；
//  我考虑一下，我的glob_path_recursive::fetch()动作流程，看看，是否会发生上述问题；
//  我应当保证，在下一次fetch之前，file_descriptor 对象是有效的！

//extern filter file_filter;

// TODO 过滤器
// 过滤器的设计，主要有两个方面：
// 1. 使用风格；是stl的方式，还是动态一个用户函数？
// 2. 需要考虑那些过滤的条件；
// 另外，过滤器的话，同命令行参数类似，也有两种工作模式，一种是保留，一种是去除
// ；
// 即，exclude 和 include 两种方式；
// 至于风格的话，
// 如果写成 stl 风格的话，意味着，我需要提供类似 begin,end 的函数；这样，就可以
// 利用 algorithm 里面的方法。
// 最强的，则是利用 C++ 的 lambda 方法；
//
// 接着，则是继续沿着现有的思路走，额外创建一个filter对象；再分别设定其各个属性
// ：比如文件的access，大小，是否文件夹；更改时间创建时间等等；
// 然后，不同系统下，过滤条件，略有区别；比如windows下，多了一个创建之间；
// 而linux下，则多了几种文件类型，比如硬连接文件（不过，貌似windows下，也开始
// 支持了）、socket文件、进程文件、USB文件等等；
//
// 当然，最终使用起来，就比较蛋疼了——需要定义三个对象……
//
//class file_finder{
//public:
//    virtual bool operator()(const file_descriptor& )
//    virtual ~file_finder()
//};
//
//void test_file_finder() {
//    std::string root_path = "h:\\oasis";
//    file_finder ff(root_path);
//    ff.add_filter();
//    // ...
//}
//
//! 关于过滤器；
// 它的创建，应该尽量简单；需要注意的是，纯粹的 | 模式，是不足的；
// 因为过滤条件，用简单的数字，是不能完全表示的。文件名的模式，修改记录、所属用
// 户等等，都可以用作过滤条件。
//
// 因此，应当将如何过滤，完全交给用户；
// 于是，无非两种办法：
// 1. 侵入式；让用户编写过滤(仿)函数，再传递给枚举工具，我们的 glob_path；
// 接着，让 glob_path 在枚举的同时，完成过滤；
// 这样，fetch的时候，就得到符合用户条件的对象了——注意，此时，应当能返回全路
// 径；
// 2. 非侵入式：说白了，就是当前这种模式，要么根本没有过滤器，要么在循环内部，
// 用户自己决定是抛弃当前，还是接受当前；
//
// NOTE 貌似上述两种方式，都无法处理递归的情况；
// boost::filesystem::path 的递归迭代器，是如何实现的呢？
//
// 好了，先采取方案1；让过滤器，可以const传递；
//
// 仿函数的形式呢？
// 这就要求glob_path 是一个模板类；
//
// 影响不大，可以接受；
//
// 使用固定类型的filter；这就要求，其内部，通过设置"钩子"函数，以完成需要的判断
// ；
// 或者说，有任意多个，可以访问 file_descriptor 对象的(仿)函数，组合在一起；
//
// 如果，glob_path 是依赖于传入仿函数含税的 模板类，那么 file_descriptor 不应
// 该也依赖于这个类
//
// 但是，之前，我需要让 glob_path 是 file_descriptor 的友元类，于是悲剧发生。
// 应该这样:
// a. 让glob_path 和 file_descriptor 都是实体类；只不过 glob_path::fetch() ，还
// 额外接受一个函数动作；
// b. 让glob_path 构造的时候，接受一个过滤器——过滤器；使用的时候，你要么继承
// 自该过滤器，再构造一个对象；要么直接使用该过滤器的实体；
// c. 不再提供任何接口；让用户挨个处理枚举到的file_descriptor对象；
// 注意，方案b和方案c，两个并不冲突！
//! http://blog.csdn.net/tang08/article/details/5775086
// basic_recursive_directory_iterator
//
// 递归遍历目录的迭代器，它的构造参数与basic_directory_iterator相同，当调用 operator++时，如果当前值是一个目录，则进入下一级目录。
// 它有三个成员函数：
// 函数名	作用
// int level() const;  得到当前搜索深度
// void pop();  调用pop()后，下一次递增就会直接返回上一级目录
// void no_push();  调用no_push()后，即便下一个元素是目录类型也不进入
//
//
// 示例代码，得到指定目录下的所有文件名（和上例作用相同）：
//
//     void find_file2( const fs::path & dir_path )
//     {
//         fs::recursive_directory_iterator end_itr; // 缺省构造生成一个结束迭代器
//         for ( fs::recursive_directory_iterator itr( dir_path );
//             itr != end_itr;
//             ++itr )
//         {
//             std::cout << itr.level() << *itr << std::endl;
//         }
//     }
//
//

namespace sss {
    namespace path {

class glob_path_recursive
{
public:
    glob_path_recursive(const std::string& root, sss::path::file_descriptor & fd);

    glob_path_recursive(const std::string& root,
                        sss::path::file_descriptor & fd,
                        sss::path::filter_t * file_filter_, bool is_exclude);

    ~glob_path_recursive();

protected:
    bool is_first()
    {
        return this->_stack.empty();
    }

    bool fetch_no_filter();

    // 这个函数，本意是在用在dpush的时候，判断该目录，是否需要枚举；
    bool is_enum_current();

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

    bool dpush( const std::string & dir);

    bool dpop ();

    // 控制最大深度；0，无限制；
    void max_depth(int depth);

public:
    int level() const                           // 得到当前搜索深度
    {
        return this->_stack.size();
    }
    void pop()                                  // 调用pop()后，下一次递增就会直接返回上一级目录
    {
        this->_next_pop = true;
    }
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
        return int(this->_stack.size());
    }

    // 越过当前——如果当前是目录，则skip掉；如果当前是文件，则越过下一个？
    void jump()
    {
        // TODO
        if (this->_pfd->is_normal_dir()) {
            //std::cout << "jump at `" << this->_pfd->get_path() << "`" << std::endl;
            this->_jump_cur_dir = true;
        }
    }

protected:
    bool cur_next() const;
    bool can_dpush() const;

private:
    glob_path_recursive(const glob_path_recursive&);
    glob_path_recursive& operator=(const glob_path_recursive&);

protected:
    bool                        _next_pop;
    bool                        _no_push;
    mutable bool                _jump_cur_dir;
    sss::path::file_descriptor* _pfd;

    sss::path::filter_t *       _file_filter;
    sss::path::filter_t *       _dir_filter;
    bool                        _file_filter_not;
    bool                        _dir_filter_not;

    int                         _max_depth;

    std::string                 _root;
    std::vector<glob_path*>     _stack;         // 递归变循环 关键栈结构
    std::vector<bool>           _stack_exclude; // 目录过滤器-匹配情况 对应栈结构
};
    }
}


#endif  /* __GLOB_RECURSIVE_HPP_1440646821__ */
