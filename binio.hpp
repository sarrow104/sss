#ifndef  __BINIO_HPP_1317660388__
#define  __BINIO_HPP_1317660388__

#include <iosfwd>
#include <cstdio>

namespace sss{

// 确认后续len个字节的内容，与pData所指向的内容相同
// 相同的话，移动文件指针
// 不同的话，文件指针归位；并抛出内容为msg的异常
void read_require(std::FILE * fp, const void * pData, size_t len, const std::string& msg);
// 确认已经到了文件末尾
// 确定的话，移动文件指针
// 不是的话，文件指针归位；并抛出内容为msg的异常
void require_fseek_at_end(std::FILE * fp, const std::string& msg);

// 读取流中的字符，直到遇到delimiter字节
// NOTE -- 2011-10-04 -- Sarrow:此版本中，虽然读取了末尾的字符，但是返回的串里面，并没有包含它。
std::string read_stop_at(std::FILE * fp, char delimiter);
// 读取流中的字符，直到遇到times次，以pat_len长度的pattern块；
// NOTE 所有的字节，都存放进返回值里面
std::string read_stop_at_times(std::FILE * fp, const void * pattern, int pat_len, int times);

long long file2string_c(const std::string& fname, std::string& buffer);

long long file2string_cpp1(const std::string& fname, std::string& buffer);

long long file2string_cpp2(const std::string& fname, std::string& buffer);

}// namespace sss

#endif  /* __BINIO_HPP_1317660388__ */

// std::ifstream
//      ::gcount()
//              返回read()动作读取的字节数
//      ::seekg(...)
//              文件指针挪
//      ::close()
//              关闭描述符
//      ::peek()
//              当前游标指向字节；游标不动
//
// 打开方式：
// ios::in	为输入(读)而打开文件
// ios::out	为输出(写)而打开文件
// ios::ate	初始位置：文件尾
// ios::app	所有输出附加在文件末尾
// ios::trunc	如果文件已存在则先删除该文件
// ios::binary	二进制方式
//
// ofstream, ifstream 和 fstream类的成员函数open的默认方式：
// 类	        参数的默认方式
// ofstream	ios::out | ios::trunc
// ifstream	ios::in
// fstream	ios::in | ios::out
//
//
// 流操作的其他的更详细说明，见：
//
//! http://blog.chinaunix.net/uid-25749806-id-345845.html
//
// 鉴于 windows,linux下，ifstream的文本打开文件，对于换行符的处理不同，强烈建议
// 全部用二进制方式打开文件；特别是基于行的内存模型，可以参考EmEditor的处理方式
// 。
