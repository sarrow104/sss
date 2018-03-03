#ifndef  __UTF8_HPP_1439694946__
#define  __UTF8_HPP_1439694946__

// NOTE
// 容错能力；
// 网页编辑 敏感 个人信息，防止机器抓取 email地址；插入<200b>的字符；
//
// 因为utf8序列模式有限，单独的<200b>字节是不合法的；但是浏览器，仍然会正确识别
// 、显示文本。
//
// 我的这个utf8字符串wrapper，可以，自动过滤这些非法字节；
//
//  > 127 的 ascii
//  utf8头
//  utf8尾

#include <string>

namespace sss {
namespace stream {
// /home/sarrow/project/MyProject.Tool/ChineseConvert/readme.unconvert.txt|137

template<typename chiter>
    class Utf8{
    public:
        bool   validate     ( chiter s_beg, chiter s_end ) ; // 检查字符串是否符合utf8标准；
        chiter find_invalid ( chiter s_beg, chiter s_end ) ; // 定位到最近一个非法的utf8序列开头
        int    text_width   ( chiter s_beg, chiter s_end ) ; // 计算一定范围内，utf8序列的显示长度；
        int    count        ( chiter s_beg, chiter s_end ) ; // 计算一定范围内，utf8字符个数；

        // NOTE 貌似，名字不应该是 next，应该是fetch……
        uint32_t next      ( chiter &s_beg, chiter s_end ) ; // 从流里面抽取一个utf8字符，并让指针指向下一个utf8字符开头；
        uint32_t prev      ( chiter s_beg,  chiter &s_it ) ; // 让指针指向上一个utf8字符开头；并返回该utf8字符值
        uint32_t get_cur   ( chiter s_beg,  chiter s_end ) ; // 获取当前位置的utf8字符；
        uint32_t check_cur ( chiter s_beg,  chiter s_end ) ; // 当前位置是否是一个utf8字符；

    };
}
}
#endif  /* __UTF8_HPP_1439694946__ */
