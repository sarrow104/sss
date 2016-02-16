# std::cout and STDIN_FILENO

http://stdcxx.apache.org/doc/stdlibug/34-2.html

http://stackoverflow.com/questions/10681134/c-daemon-silence-mode

http://stackoverflow.com/questions/16168391/c-closing-cout-properly

http://www.cplusplus.com/forum/unices/426/

http://stackoverflow.com/questions/25055826/stdcout-overhead-if-closed-stdin-fileno

http://www.cplusplus.com/forum/unices/28134/

https://www.coursehero.com/file/p2g28l28/isattySTDOUTFILENO-coloredout-coloroutput-if-iscolored/

http://www.netbsd.org/~jmmv/process/boost_process/platform_specific_usage.html


---------------------------------------

注意：

   C++内部，安全的 两个流，绑定同一个输出对象的方式，只有：

> std::ofstream out;
> if (argc > 1)
>   out.open(argv[1]);
> else {
>   out.copyfmt(std::cout);                                  //1
>   out.clear(std::cout.rdstate());                          //2
>   out.basic_ios<char>::rdbuf(std::cout.rdbuf());           //3
> }
> // output to out, for example
> out << "Hello world!" << std::endl;

另外，C++中，是不允许拷贝流对象的；

> out.copyfmt(std::cout);                                  //1
> out.clear(std::cout.rdstate());                          //2
> out.basic_ios<char>::rdbuf(std::cout.rdbuf());

要模拟拷贝的话，除了之前的rdbuf函数，还需要额外拷贝格式、状态；具体代码，类上！

> std::streambuf * old = std::cout.rdbuf(mStdOut.rdbuf())

---------------------------------------

首先，考虑库的使用风格问题：

> std::cout << s::T::moveXY(x, y) << "sometext" << std:endl;

或者：

> s::T::Cursor c(std::cout);
>
> c.moveXY(x, y);
>
> std::cout << "sometext" << std:endl;

---------------------------------------

前者，将会在每次输出的时候，都将全局变量<file_no-rdbuf()>值对，
比对一下，看是不是标准输入输出；然后再决定是否真的调用对应的操作（输出转义流，或者调用api函数）

后者，则没有判断。

当然，潜在的问题还有，std::cout 是基于线程的吗？它内部呢？
肯定存在一个序列化的东西，关键这个东西，是在file_no之上，还是之下？

---------------------------------------

只能先假设了；

---------------------------------------

## 接着是类结构的问题

像这些对象，都有类似的操作——即，检测流对象-操作符，然后决定动作；

也有区别，那就是调用的函数，或转义流，略有不同；

---------------------------------------

类linux下，很简单，弄一个类族，真可以解决问题，而且不用引入虚函数；

windows模式，就不行了，必须要有“虚”的东西存在！

---------------------------------------

还是有共同点：

那就是moveXY()所生成的转义流，或者函数，其本身是不用存储中间结果的；基本都是用完就扔！

一个临时对象，就可以了；

颜色则不同；一般是预定义风格——以便通过配置文件，动态修改；

---------------------------------------

从这个语义上看，moveXY()确实需要用对象来包裹，不管是风格1还是风格2；

风格1，是static函数的样式，它需要返回上述，描述移动的对象；

风格2，对象+成员函数的形式——它自己就是我说的这种对象；

不过，看起来有些割裂而已；

我先选择风格1；

---------------------------------------

看样子，之前的风格选择不太正确

因为相关的操作太多了；为了保持上述风格，我不得不编写2×N套类；类里面还得重复的函数；

屏幕部分，就有清屏、删除行；上移光标、下移光标；
而且，上述这些操作对于linux终端来说，就是一个数字（可选），加一个字母就完成了；

而windows来说，就对应着一个函数调用（一个函数指针，一个参数——忽略console-handle）；

而且，windows可以有好几个console——用户可以自己创建console，并获取其handle然后操作之；

ok，这些不论，那么，当我：

sss::Terminal::console cons(std::cout);

后，我希望cons对象，绑定到std::cout所代表的控制台上，应该如何操作呢？

GetStdHandle(STD_OUTPUT_HANDLE)

https://msdn.microsoft.com/en-us/library/windows/desktop/ms683231%28v=vs.85%29.aspx

STD_INPUT_HANDLE
(DWORD)-10
	The standard input device. Initially, this is the console input buffer, CONIN$.

STD_OUTPUT_HANDLE
(DWORD)-11
	The standard output device. Initially, this is the active console screen buffer, CONOUT$.

STD_ERROR_HANDLE
(DWORD)-12
	The standard error device. Initially, this is the active console screen buffer, CONOUT$.

---------------------------------------

鉴于win::console下，isatty函数是可用的，就是说，

> hHandle - STD_OUTPUT_HANDLE - STDOUT_FILENO - std::cout

可以建立类似上面这种一一对应的关系。

那么，从std::cout到hHandle是可以判断的了！

---------------------------------------

先考虑最windows下，最简单的情况，console输出，STD_OUTPUT_HANDLE；

> sss::Terminal::isatty(std::cout);

在没有重定向，和被重定向，分别返回什么？

关键语句是：

> return ::getenv("COLORTERM") || ::isatty(i);

前者，会返回非零；

后者，会返回0；

但是，并不能得到一个handle！即，还区分handle的对应关系；
