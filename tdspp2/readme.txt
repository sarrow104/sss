* readme

======================================================================
** todo
date:2013-06-27

-- 限制提取

D:\project\oasis\sell_statics\SpaceTimeWrapper.cpp|77
D:\project\oasis\autofill\AutoFill.cpp|252

-- TRan
D:\project\oasis\yb_data_import\ybdata_import\v2\YBDataImportCtl.cpp|472
----------------------------------------------------------------------

date:2013-04-16

FIXME:
	同一个 fl 反复绑定，有时候会出现非法内存访问

----------------------------------------------------------------------

date:2013-04-07

----------------------------------------------------------------------

关于结果集容器的讨论。

之前，我看到一个封装MySQL Api的。他采用的结果集很奇葩：
std::vector<std::map<std::string, MySQLAny> > result_set;

其好处很容易看到，就是可以写成：

result_set[行号][列名]的形式，来检索具体的字段。

当然，这样的方式，还是有其缺点的：

1. 内存占用比较高。因为，结果集的各行，是可以共享列名的。所以稍微好点的方案是：

   std::map<std::string, std::vector<MySQLAny> > result_set;

2. 该代码编写人员，做了一个假设：sql脚本编写者，对检索结果的每一列，都取了不同的
   字段名！

   但是，实际情况不总是这样的；上述定义，在遇到了偷懒的sql编写者，就会碰壁，得到
   怪异的结果，甚至造成业务上的错误。

3. 容器本身不支持延迟计算。如果遇到结果集比较大的时候，内存占用会比较大。

当然，原作者的设计，在大多数情况下，都不会出问题，可以满足很多地方的需求。

而且还有一个好处是：相当于延迟计算——只有当需要提取结果集的时候，才提取他们。

----------------------------------------------------------------------
缺点2，可以用变通的办法来消除错误——使用multimap 而不是 map来存放数据！当然，最
好做一个封装，然后重载[]操作符，以允许用字段名，或者是字段顺序号来检索需要的字段
。

用字段名检索的时候，会返回std::pair<iterator, iterator>的半开半闭区间；这样，用
户就知道，返回结果的数量了。

不过，这种方式，会影响使用的体验；还不如抛出异常——一旦发现有一个字段名，对应多
组值的时候，就抛出异常。

----------------------------------------------------------------------

我觉得，结果集的封装，应该留给稍微上层一点的用户，底层么，只需要提供参数传递、获
取，结果集的一行的获取即可。

至于，上层用户按什么样的结构组织数据，那是他们自己的事情，与库无关。

----------------------------------------------------------------------

** 时空软件里面又是如何处理的呢？

其资料检索方案允许如下形式的语句：

if :spid is not null
  select a.hw,b.hwbh,b.huowname,a.pihao,a.sxrq,a.shl as 批号数量,
         a.shl as kcshl,a.gebjj as diaocdj,a.gebjj as diaordj,
         a.gebjj as chbdj,c.jwh,a.shpchd
    from sphwph a,huoweizl b,hwsp c
   where b.beactive='是' and a.spid=c.spid
     and a.hw=c.hw and a.hw=b.hw and a.shl<>0
     and a.spid=isnull( :spid ,'')
     and b.duifbsh=:bendian
   order by a.shl
else
  select a.hw,b.hwbh,b.huowname,a.pihao,a.sxrq,a.shl as 批号数量,
         a.shl as kcshl,a.gebjj as diaocdj,a.gebjj as diaordj,
         a.gebjj as chbdj,c.jwh,a.shpchd
    from sphwph a,huoweizl b,hwsp c
   where b.beactive='是' and a.spid=c.spid
     and a.hw=c.hw and a.hw=b.hw and a.shl<>0
     and b.hw not in (select hw from huoweizl where huowname like '%不合格%')
     and a.spid=isnull( :mx.spid ,'')
     and b.duifbsh=:bendian
   order by a.shl

见：:mx.spid，就是说，它允许重新获取在gui界面上，确定列名的字段值。——因为：当
你点击skdjrun对应某明细的输入格子的时候，你至少提供了“行号”、“列名”这两个信
息。

当然，汇总信息 :spid, :bendian 这些，时空软件也能自动替换，生成能工作的脚本。

这说明，时空软件还完成了变量名的作用域限制。

----------------------------------------------------------------------
** 多语句、多检出结果的支持

在实验类似

	select top 10 * into #spkfk from spkfk
	select spbh,spmch from #spkfk

的多语句同时执行的时候，我发现，检出的结果是空的！

修改为：

	select top 10 spbh,spmch from spkfk
	select top 10 * into #spkfk from spkfk
	select spbh,spmch from #spkfk

之后，检出结果，只有第一行语句的数据！

同时在profiler 中检查，发现语句都有执行，无遗漏。

怎么办？那是配置上的问题了。

----------------------------------------------------------------------
查看：

./多结果获取-论文.txt|94

后发现，他是让整个检索语句都执行结束之后，再利用dbprocess内部指向的结果集指针，
再返回来使用 dbgetrow()，以获取特定行的结果集。

此时，作者只能返回批处理sql语句的最后一个结果集——因为之前的结果集指针，都被冲
掉了。

上面肯定是有缺陷的——如果检索结果使用了临时表；那么最后一个语句常常是 drop
table #tmp。

那么，获取有效结果集的想法，就悲剧了！

我应该怎么做？继续brain fuck，让单循环支持多结果集输出吗？

	while(q.fetch())
	    do_sth();

还是将接口修改为双循环？

	while(q.getresult())
            while(q.fetch())
	        do_sth();

从语义完整性来说，当然是双重循环好。但，很多时候，用户并不想获取并输出结果集。他
们只是简单想执行该语句而已。

----------------------------------------------------------------------
另外，其他sql封装，使用了很多级别的类，这是为什么呢？我想，这是为了限制状态与操
作。

sql解析，本身是一个巨大的状态机，我们这样看是没有问题的——比如，freetds的API，
很多都是dbxxxxx(dbprocess)这样的函数接口。这就可以验证上述说法了。

为什么要封装，还不是因为上述的函数接口，都处于一个平面，适当分层，划分功能，是很
容易理解的事情。

如果用C++语言封装，自然用类来划分各种方法，也是很正常的事情。

比如：

DBresult * result = q.getresult();
while()
     result->fetch();

这样操作起来，就不容易出错。你不可能先调用fetch，再getresult，因为程序限制了你。

而，按照我开始的涉及，getresult(),fetch()都是Query的方法，那用户就可能混用这些
不能混用的方法。

当然，也可以还是沿用单循环的方式，不过，需要做少许修改：q.fetch()不再返回bool，
而是一个int，用来区别返回的类型；可以有：

1. header.
2. data
3. 统计行

----------------------------------------------------------------------

很多时候，多语句的执行，只需要取得一个结果集——因为只有一句是检索语句。如何判断
呢？客户端自己分句是不可能的——你还得做一个解析器。

最好是freetds自己完成。

一个是依赖dbrows,还有一个是获取headers列表。如果这两个数据有一个是0，那么，就忽
略该结果集。

----------------------------------------------------------------------

// 创建登录信息
ftds::login * login = ftds::create_login("","","","");

// 根据登录信息获取操作进程
dbprocess = login->create_process();
dbprocess->sql("");
while (ftds::resultset rs = dbprocess->get_resultset())
    while (row = rs->fetch())
    {
       row->col("");
       row->col(0);
    }

// 指向内部一个虚拟的resultset对象，所以，不存在释放内存的问题。

算了，还是不要用指针的形式。还是用对象。结果集的获取，用直接创建的方法不方便，就
用外部参数的办法。然后检查返回值，以确定状态是否OK。

就好比：std::getline(std::istream &, std::string&)函数一样。

ftds::dbprocess;
login.create_process(dbprocess);

or

ftds::dbprocess process(login);

dbprocess.sql("");
result_set rs;
while (dbprocess.get_resultset(rs))
{
    row;
    while (rs.fetch(row))
    {
       row.col("");
       row.col(0);
    }
}

----------------------------------------------------------------------
class DBInit:
	-DBInit()
	-~DBInit()
	+...

class Login:
	管理登录信息
	整理用户输入，生成合适的登录信息，绑定到LOGINREC * 对象上。
	内部是一个LOGINREC*指针，如何管理这个资源呢？

	首先，需要考虑客户程序的扩展能力——客户端很可能需要同时链接不同的数据库
	；就是说，需要同时存在多个登录信息。

	sybdb的使用前，需要先调用 dbinit() 函数；程序结束前，还需要调用dbexit()函
	数。这两个函数都只能调用一次。

	而且，这两个函数都不返回、或需要传入资源句柄这样东西。

	怎么办？Login 是 LOGINREC* 的封装，这就是，不能允许拷贝构造函数的存在！
	因为没有意义——对于用户来说，最好连dbinit()的存在都不需要知道！利用静态
	函数里面的静态变量，来实现dbinit()和dbexit()的自动调用，是最好的选择了。

class DBProcess:
	管理数据库链接进程。

	数据库可以有多个工作进程。客户程序也可以有多个线程（进程）；每个客户线程
	都需要一个数据库工作线程才能进行数据的交互。

	内部是一个DBPROCESS*指针；那么，如何管理这资源呢？

	允许拷贝构造函数——即，客户端与同一个数据库，有多个线程相连。

class ResultSet:
	结果集类。
	如果要使用逻辑上的结果集，就需要，实地保存检索出来的数据，而不是按需要，
	就从DBPROCESS * 的缓存、游标上面取。
	但是这样的话，就和DBLink类的语义冲突了。
	DBLink设计目的是管理程序与数据库之间交互进程的。就是说，SQL的语句，只能
	顺序执行；结果集的获取，同样也只能顺序获取。如果ResultSet只是简单的
	DBPROCESS * 封装，而又运行从DBLink构建多个ResultSet的话，然后这个多个
	ResultSet本来目的是按需检索数据，就会造成混乱。

	解决办法有两个：

	1. 采用实地存储的办法。——结果集就可以类一般对象一样，到处传递、存放。
	   坏处，有点过度涉及——如何检出数据、如何保存数据是客户代码的事情。这
	   种结构，没有必要作为基础容器，提供给客户代码。另外，遇到大量的检索，
	   有可能超出内存使用。

	2. ResultSet类型，就是简单的DBPROCESS * 封装。但是对于DBProcess * 指针，
	   只有使用的分，不能进行资源管理——就是说，获取、拷贝若干个ResultSet结
	   构，其实都是一个对象。

	   那么，本类存在的目的，就只是一个方法归类作用而已。
	   但是，如何管理ResultSet生存期呢？比方说，我已经在处理下一个ResultSet
	   了，前一个如何让其失效呢？

	   没办法。

	   因为ResultSet 不是逻辑意义上的对象。

	我觉得，还是让Query来处理算了。ResultSet只是上层应用对象而已，方便客户代
	码限制结果集。

	让Query内部维护一个状态机；分别表示：遇到新的ResultSet；处理ResultSet中
	；解析到Header；解析row data；解析compute row；等等。当然，还有错误消息
	。

	此时，可以使用WaitMessage，来确保消息已经获取完毕。——就是将本来的异步
	操作，变为顺序执行。

class Query:
	结论，类ResultSet对象；
	也是对DBPROCESS *的封装，目的只是将一类型的方法，归纳到一个类型里面使用
	。

	另外一个作用是记录当前执行的sql语句。虽然dbxxxx api本身可以管理传递进来
	的sql语句（附加、清空）——另外，freeTDS本身有缓存当前执行sql语句，也提
	供了获取他们的API。

	Login login;
	login.set_properties("127.0.0.1",....);
	DBLink dblink(login);
	Query q(login, "select * from ...");
	q.append("union select * from ...");

	while (ResultSet rs(q))
	{
	    if (rs.rowcount() == 0)
	        continue;
	    else // NOTE 受影响的行数非0，其结果集仍然可能是空的！
	    {
	        int num_col = rs.colcount();
		if (num_col == 0)
		    continue;
		// 这里需要商榷一下；如果用户不定义FieldList，是不是就不能检索结
		// 果呢？
		// 就是说，客户程序，如何简单、无返回地执行语句呢？
		while (FieldList fl(rs))
		{
		}
	    }
	}

class ResSetRule:
	定义结果获取规则；主要是用来限制执行性的，属于上层结构。
	比如：
	1. 限制当前update 语句，只能影响一条记录。超过的，自动roll back；
	2. 限制结果集数目。比如，只能有一个可用的结果集。

class Err_and_Msg:
	数据库返回的错误消息、一般消息，是否应该直接用C++里面的异常模型来处理呢
	？首先需要明白的一点，数据库相当于一个不太可能死机的连续服务员——随时等
	等客户端指令；解析执行该指令；如果有结果，则将结果返还给用户。
	就是说，就算数据库返回的是“错误消息”，在大多数情况下，都还是“正常”的
	；除非遇到死锁、服务器当机等极端少数情况外，客户，都可以继续向服务器发送
	各种指令。
	就好比，使用isqlw，来调试语句一样。这次的语句错了，你稍作修改，选择，再
	按F5，还可以继续执行——不是上次错了，这次就不提供服务了。
	当然，流程还是相当于被打断了。

	那么异常呢？异常其实和这个也类似，部分异常，不是说，客户端程序只能被系统
	杀死，然后让用户重来什么的。当然，另外一些异常，只能中断程序执行，比如内
	存资源获取失败什么的。
	而且，发生了异常，程序的执行也就被打乱了——这和调试数据库脚本是一回事。
	不同的是，调试sql脚本，遇到错误的时候，是调试人员即时处理。
	而对付程序异常的策略，已经写进代码里面了。
	db-library，采用的是回调函数，来从服务器获取各种消息；这就意味着，和sql
	执行流，可能是异步的！

	怎么办？通过获取错误代码的方式。当提示有消息，则等待若干秒钟，以便获取到
	数据？还是在回调函数里面申请一个互斥体？

	然后客户端程序，就循环访问这个互斥体保护的内存对象；当这个对象的状态发生
	改变之后，就说明信息全部到位，可以现实给用户了。

	当前，还是采用最简单的策略，直接输出到log里面即可——如果是Err信息，则终
	止程序执行。

----------------------------------------------------------------------
** SQL语句的批量执行与控制

class BatchSQL:
	BatchSQL bsql;
	bsql.add("", ONLY_ONE_ROW_AFFECT);
	bsql.add("", ONLY_ONE_ROW_AFFECT);

	...

	特点是，如果其中一句执行错误，就会发生回滚——保证没有不完成的数据库修改
	。

** bug

生成的可执行文件，不支持中文名！

$ ./提成目录导出.exe
FreeTDS: db-lib: exiting because client error handler returned INT_EXIT for msgno 20109

$ mv ./提成目录导出.exe tichen_export.exe
再次执行，就OK了。

见：

sss\include\sss\tdspp2\Login.cpp|64

    // NOTE APPname 不能包含特殊字符！比如斜杠
    DBSETLAPP(this->loginrec, this->appname.c_str());

----------------------------------------------------------------------

** dbdata 与 4096
http://www.sommarskog.se/MSSQL/mssql-dblib.html

Text/image functions
The functions for inserting and updating text/image columns in MSSQL::DBlib have a different interface from dbwritetext and dbupdatetext in DB-Library.

There is a simple example of using dbwritetext in eg\wtext.pl. For example of all functions, please see the test script dblib\t\3_text.t.

Notice that by default, DB-Library and SQL Server have a limit on the maximum
size of a text/image values you can recieve of mere 4096 bytes. You can use
these calls to remove all limits:

   $d->dbsetopt(DBTEXTSIZE, "2147483647");
   $d->dbsetopt(DBTEXTLIMIT, "0");
   $d->dbsqlexec;

   while ($d->dbresults != NO_MORE_RESULTS) {}

You cannot use these functions if you have enabled the table option text in row (available in SQL2000 an on).

General caveat: about the only time I play with text/image columns is when I work with the MSSQL::DBlib.

----------------------------------------------------------------------

** TODO

date:2014-08-12

Query::bind ，增加 参数为 ResultSet 的版本；

即，每次循环，获得一个结果集。

