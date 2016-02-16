* 删除到回收站

** windows

windows，貌似用的是 SHFileOperation函数。

trackback:http://www.cnblogs.com/wind-net/archive/2012/11/09/2761976.html

在使用该函数删除文件时必须设置SHFILEOPSTRUCT结构中的神秘FOF_ALLOWUNDO标志，这样
才能将待删除的文件拷到Recycle Bin，从而使用户可以撤销删除操作。需要注意的是，如
果pFrom设置为某个文件名，用FO_DELETE标志删除这个文件并不会将它移到Recycle Bin，
甚至设置FOF_ALLOWUNDO标志也不行，在这里你必须使用全路径名，这样SHFileOperation才
会将删除的文件移到Recycle Bin。

// 拷贝文件
 1     CString strSrc("D:\\test1");
 2     CString strDes("D:\\test2");
 3     strSrc += '\0';//必须加'\0'
 4     strDes += '\0';
 5     SHFILEOPSTRUCT FileOp;
 6     FileOp.fFlags = FOF_ALLOWUNDO|FOF_MULTIDESTFILES|FOF_SIMPLEPROGRESS;//
 7     CString str("正在进行中");
 8     str += '\0';
 9     FileOp.lpszProgressTitle = str; // 无模式操作对话框标题
10     FileOp.hwnd = m_hWnd;
11     FileOp.hNameMappings = NULL;
12     FileOp.pFrom = strSrc;   // 操作目标
13     FileOp.pTo = strDes;	// 目的文件夹
14     FileOp.wFunc = FO_COPY;  // 动作
15
16     BOOL bOk;
17     bOk = SHFileOperation(&FileOp);
18     if (!FileOp.fAnyOperationsAborted)//终止
19     {
20         if(bOk)
21             MessageBox(_T("操作出现错误！"));
22         else
23             MessageBox(_T("操作完成！"));
24     }
25     else
26     {
27         MessageBox(_T("操作已终止！");
28     }


----------------------------------------------------------------------

http://stackoverflow.com/questions/6776469/moving-files-to-trash-can-in-linux-using-c
http://freedesktop.org/wiki/Specifications/trash-spec/
http://www.hardcoded.net/articles/send-files-to-trash-on-all-platforms.htm
https://github.com/andreafrancia/trash-cli|py工具
https://github.com/hsoft/send2trash|py工具

** ubuntu

ubuntu下是，将需要删除的文件，mv到
/home/$USER/.local/share/Trash/files/ 下

如果已经有同名的文件，则在后缀前面，加一个数字，以示区别；数字从2开始；

例如：
a.txt
a.2.txt

同时，往
/home/$USER/.local/share/Trash/info/ 下面写一个，附加了后缀为 .trashinfo 的ini格式文件；

形如：

a.txt.trashinfo

内容为：

[Trash Info]
Path=/home/$USER/path/to/delete/position/a.txt
DeletionDate=2015-08-23T22:01:36

----------------------------------------------------------------------
注意，如果路径有中文，则用类似url的方式进行encode；

http://www.linuxquestions.org/questions/programming-9/bash-script-how-to-move-files-to-trash-instead-of-deleting-819103/

----------------------------------------------------------------------

trackback:http://superuser.com/questions/199731/linux-command-to-empty-recycle-bin

Except for the ~/.local/share/Trash trash files for other file systems may be stored in <mount-point>/.Trash-$(id -u) directories. If your're running ubuntu there is a helper package to clean all locations

sudo aptitude install trash-cli

To clean the trash in all locations just run:

empty-trash

It should work with any linux desktop environment compliant with FreeDesktop.org Trash Specification. On newer versions, the command may be:

trash-empty

shareimprove this answer

edited Nov 16 '12 at 11:16
Lee Boynton
1054

answered Oct 15 '10 at 9:45
Paweł Nadolski
76139



Good point. More info here: manpages.ubuntu.com/manpages/lucid/man1/empty-trash.1.html –  Linker3000 Oct 15 '10 at 9:50
add a comment
up vote
5
down vote


rm -rf ~/.local/share/Trash/files

If not under .local/share, it may be under ~/.Trash
shareimprove this answer

edited Jun 26 '13 at 3:00
Community♦
1

answered Oct 15 '10 at 9:27
Linker3000
18.4k23052



Thanks for your answer, but in .local/share/ I have just "applications" and "mime", no Trash to be found :(. –  Rob Oct 15 '10 at 9:35


OK - have a look for /home/YOURUSERNAME/.Trash –  Linker3000 Oct 15 '10 at 9:39


Many thanks, that was it ;) –  Rob Oct 15 '10 at 10:05
add a comment
up vote
2
down vote


In case you would like to empty the trash of the currently logged in user:

rm -rf ~/.local/share/Trash/files/*
