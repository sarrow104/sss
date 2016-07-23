* C语言，基于libc的枚举版本

date:2014-08-02

trackback:http://stackoverflow.com/questions/612097/how-can-i-get-a-list-of-files-in-a-directory-using-c-or-c?rq=1

======================================================================

   len = strlen(name);
   dirp = opendir(".");
   while ((dp = readdir(dirp)) != NULL)
           if (dp->d_namlen == len && !strcmp(dp->d_name, name)) {
                   (void)closedir(dirp);
                   return FOUND;
           }
   (void)closedir(dirp);
   return NOT_FOUND;

----------------------------------------------------------------------

int list_dir (const char *path)
{
    struct dirent *entry;
    int ret = 1;
    DIR *dir;
    dir = opendir (path);

    while ((entry = readdir (dir)) != NULL) {
        printf("\n%s",entry->d_name);
    }
}

Following is the structure of the struct dirent

struct dirent {
    ino_t d_ino; /* inode number */
    off_t d_off; /* offset to the next dirent */
    unsigned short d_reclen; /* length of this record */
    unsigned char d_type; /* type of file */
    char d_name[256]; /* filename */
};

----------------------------------------------------------------------

/*
 * File: isdir.c
 * Date: 2008-03-14
 * Auth: mymtom
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>

#ifndef DT_DIR
#error "DT_DIR not defined, maybe d_type not a mumber of struct dirent!"
#endif

int
main(int argc, char *argv[])
{
        static char dot[] = ".", dotdot[] = "..";
        const char *name;
        DIR *dirp;
        struct dirent *dp;

        if (argc == 2)
                name = argv[1];
        else
                name = dot;

        dirp = opendir(name);
        if (dirp == NULL) {
                (void)fprintf(stderr, "%s: opendir(): %s: %s\n",
                    argv[0], name, strerror(errno));
                exit(errno);
        }

        while ((dp = readdir(dirp)) != NULL) {
                if (dp->d_type == DT_DIR)
                        if ( strcmp(dp->d_name, dot)
                            && strcmp(dp->d_name, dotdot) )
                                (void)printf("%s/\n", dp->d_name);
        }
        (void)closedir(dirp);

        return (0);
}

----------------------------------------------------------------------

* 获得Windows系统中文件属性——GetFileAttributes详解

trackback:http://blog.csdn.net/fivedoumi/article/details/6933989

在MSDN中，文件总共有15种属性，根据磁盘的分区格式不同，文件的属性也会不同。
现在针对 GetFileAttributes 函数的返回值做以下整理
返回字段
返回值
属性类型
FILE_ATTRIBUTE_READONLY 1	只读	- 橙色
FILE_ATTRIBUTE_HIDDEN 2	隐藏	- 橙色
FILE_ATTRIBUTE_SYSTEM 4	系统	- 橙色
FILE_ATTRIBUTE_DIRECTORY 16	目录	- 橙色
FILE_ATTRIBUTE_ARCHIVE 32	存档
FILE_ATTRIBUTE_DEVICE 64	保留	- 橙色
FILE_ATTRIBUTE_NORMAL 128	正常
FILE_ATTRIBUTE_TEMPORARY 256	临时
FILE_ATTRIBUTE_SPARSE_FILE 512	稀疏文件
FILE_ATTRIBUTE_REPARSE_POINT 1024	超链接或快捷方式
FILE_ATTRIBUTE_COMPRESSED 2048	压缩	- 蓝色
FILE_ATTRIBUTE_OFFLINE 4096	脱机
FILE_ATTRIBUTE_NOT_CONTENT_INDEXED 8192	索引
FILE_ATTRIBUTE_ENCRYPTED 16384	加密	- 绿色
FILE_ATTRIBUTE_VIRTUAL 65536	虚拟

橙色标记的属性为Windows系统中文件的公有属性，其中“只读”、“隐藏”、“系统”、
“存档”为文件的四种基本属性。compressed,content_indexed,encrypted只存在于NTFS分
区中。

文件去掉全部属性后（四种基本属性），将自动标记为normal。同时具有system和hidden属
性的文件会在系统中彻底隐形，这也是病毒常用的伎俩。

commpressed和encrypted不能共存。默认情况下文件都有content_indexed属性

======================================================================

** 关于用户接口问题

这filter参数，应该何时起作用？

win32api的FindFirstFile,FindNextFile，可以有虚拟的，过滤器；此时，它过滤文件夹否？如果我
想找的东西，可能在子文件夹下？……

不行的；FindxxxxFile函数的特点是统一应用过滤器！不管是否是文件夹；

而一个gui的可以浏览某些类型文件的软件来说，其打开窗口，通常是这样的逻辑：

显示文件夹和它所关心的后缀的文件！

就是说，对于文件应用过滤器，而文件夹不应用过滤器；

我的 path_glob 类，为了保持FindFirstFile语义的一致，所以，filter也是应用在所有的
fd上面；

对于 path_glob_recursive 来说，就有些困扰了。

首先 path_glob_recursive 一般来说，其目的就是为了检出所有的文件——对于这个枚举
器来说，大部分情况下，它要枚举的都是具体的文件（当然，也有偏门的，它就想枚举目录
——此时可以用来目录结构重建）；

简单说，对于path_glob_recursive，如果只是要文件，那么过滤器应当只作用于文件，而
避开目录；

反之，我们可能像避开某些目录；

就是说，我们需要两组过滤器！

----------------------------------------------------------------------

** 关键字过滤器

其实，我已经有正则表达式过滤器了；但问题是，关键字过滤器也很有用；

一个初级的实现，见：

/home/sarrow/project/FindDocument/src/main.cpp|5

该findDocument工具，当前的特点是，支持通过命令行，提供多个关键字；并且，以是否以
“-”，作为禁用类型的关键字；

关键字这种匹配方式，特别是禁用列表，貌似直接用正则表达式，是比较难实现的——当前
，我可以把 IncludeRule和ExcludeRule，这两种匹配，融合到一个过滤器里面；

而正则表达式来说，貌似没法表示ExcludeRule这种策略；只能在外面套一个否定判断来实
现。

而且，更重要的是，正则表达式，严重依赖于用户输入——一个表达不当的正则表达式，为
何效率低……

还有，我这个版本的 findDocument 中用到的技术，还考虑了关键字之间的包含的剔除相关
的逻辑（就是一种优化）——这要是换成等效的正则表达式，就比较难优化了（假设用户提
供的，就是正则表达式的形式，然后考虑我说的覆盖性优化，就比较难做了。）

还有，如果考虑关键字过滤器的“并集”运算，形如：

    abc xyz -abcxyz | love

换成文字描述就是，同时包含子串 “abc”，“xyz”，但是不包含“abcxyz”；或者仅包
含一个“love”；

而这个，要直接用正则表达式来描述，是没办法的；特别是-abcxyz，需要单独处理……

呵呵，想死的心都有了。

----------------------------------------------------------------------

