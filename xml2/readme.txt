* readme.txt

======================================================================

** xml文件格式说明

*** 1. BOM yes or not ?

一般，windows下文本文件头部都会带有BOM字节，以表明其字节顺序；但是，xml文件作为
一种数据交换文件（不是一般意义上的文本文件），其头部，有xml prolog语句：

	<?xml version="1.0" encoding="utf-8"?>

内嵌的 charset 属性，将如何理解文本的信息——文本编码，告知给了解析器；

于是，BOM，就没有意义了。

相关论述，见：http://www.cnblogs.com/ini_always/archive/2011/12/24/2300601.html

*** 2. XML prolog

见 上一节的：

	<?xml version="1.0" encoding="utf-8"?>

关键在于 <?xml ...?>的属性顺序。这两个属性的输出顺序，不能颠倒！只能是 version
在前，encoding 在后。

*** DOCTYPE 与 DTD解析

XML文件的“文档类型”，docType是xml的一个非必须属性；如果为空，活取的时候，返回
null即可；

——理论上，每个xml节点，都可以有自己的docType；

	格式：

	私有DTD

		<!DOCTYPE root SYSTEM "URL">

	共有DTD

		<!DOCTYPE root PUBLIC DTD名称 "URL">

	其中 DTD名称 格式为

	"注册//组织//类型 标签//语言",

		"注册" + | -
			指示组织是否由国际标准化组织(ISO)注册,+表示是,-表示不是.

		"组织"
			即组织名称,如:W3C;

		"类型"
			一般是DTD,

		"标签"
			是指定公开文本描述，即对所引用的公开文本的唯一描述性名称
			,后面可附带版本号。

		"语言"
			是DTD语言的ISO 639语言标识符,如:EN表示英文,ZH表示中文,

			完整的ISO 639语言标识符列表:
			http://ftp.ics.uci.edu/pub/ietf/http/related/iso639.txt

例如：

<!DOCTYPE note SYSTEM "note.dtd">

<!DOCTYPE hibernate-mapping PUBLIC
    "-//Hibernate/Hibernate Mapping DTD 3.0//EN"
    "http://hibernate.sourceforge.net/hibernate-mapping-3.0.dtd">

<!DOCTYPE ncx PUBLIC
     "-//NISO//DTD ncx 2005-1//EN"
     "http://www.daisy.org/z3986/2005/ncx-2005-1.dtd">

----------------------------------------------------------------------
说明：

...

DOCTYPE 描述了xml文件节点的说明、属性、可以包含那些节点等信息；即，可以对xml文件
主题部分进行校验；

w3公司，有专门的校验工具；

该行其实有两种格式；内部声明和外部声明：

内部声明

	<!DOCTYPE 根元素名 [元素声明]>

范例：

	<?xml version="1.0"?>

	<!DOCTYPE note[
		<!ELEMENT note (to,from,heading,body)>
		<!ELEMENT to (#PCDATA)>
		<!ELEMENT from (#PCDATA)>
		<!ELEMENT heading (#PCDATA)>
		<!ELEMENT body (#PCDATA)>
	]>

	<note>
	 <to>Tove</to>
	 <from>Jani</from>
	 <heading>Reminder</heading>
	 <body>Don't forget me this weekend</body>
	</note>

外部声明
假如 DTD 位于 XML 源文件的外部，那么它应通过下面的语法被封装在一个 DOCTYPE 定义中：
<!DOCTYPE 根元素 SYSTEM "文件名">
这个 XML 文档和上面的 XML 文档相同，但是拥有一个外部的 DTD:

1 <?xmlversion="1.0"?>
2 <!DOCTYPE note SYSTEM "note.dtd">
3 <note>
4  <to>Tove</to>
5  <from>Jani</from>
6  <heading>Reminder</heading>
7  <body>Don'tforgetmethisweekend!</body>
8 </note>

这是包含DTD的"note.dtd"文件：

1 <!ELEMENT note(to,from,heading,body)>
2 <!ELEMENT to(#PCDATA)>
3 <!ELEMENT from(#PCDATA)>
4 <!ELEMENT heading(#PCDATA)>
5 <!ELEMENT body(#PCDATA)>

DTD的具体解释，见：

/home/sarrow/extra/sss/include/sss/xml/dtd/readme.txt

======================================================================

** xml文件的构成

    - 元素 , 元素即所说的自定义标签,它是 XML 以及 HTML 文档的主要构建模块。
    - 属性 , 属性可提供有关元素的额外信息。属性总是被置于某元素的开始标签中。属
      性总是以名称/值的形式成对出现的。
    - 实体 , 实体是用来定义普通文本的变量。实体引用是对实体的引用。如HTML文档中
      的&nbsp;即是一个实体引用当文档被 XML 解析器解析时，实体就会被展开。
    - PCDATA , PCDATA 的意思是被解析的字符数据（parsed character data）。可把字
      符数据想象为 XML 元素的开始标签与结束标签之间的文本。PDATA 是会被解析器解
      析的文本。这些文本将被解析器检查实体以及标记。文本中的标签会被当作标记来处
      理，而实体会被展开。不过，被解析的字符数据不应当包含任 何&、< 或者 > 字符
      ；需要使用 &amp;、&lt; 以及 &gt; 实体来分别替换它们。
    - CDATA , CDATA 的意思是字符数据（character data）。CDATA 是不会被解析器解析
      的文本。在这些文本中的标签不会被当作标记来对待，其中的实体也不会被展开。

	CDATA 区段开始于 "<![CDATA["，结束于 "]]>",CDATA段中可以包含除CDATA限定
	符之外的任何字符

** xml文件的解析与xml对象的构建说明：

	首先， xml_parser 将 xml字符串序列拆分为两部分：
		a. bom 字节序列
		b. 利用 tokenizer 对象将 bom 外的字符串流，划分为 token 序列；

	之后，xml_parser 用递归的形式，分析 token 序列顺序，将其理解为各种xml节
	点对象，并组装起来。

	然后根据需要，生成 xml 对象，或者 xhtml 对象；

	在xml对象的构建过程中，都使用了指针的形式。即，构造过程中，基本没有xml节
	点对象的拷贝动作发生（除了zen-code语法糖）。

	另外，各种xml节点对象是一个类族。

----------------------------------------------------------------------

	另外，为了构造方便，还额外仿照 zen-code 的语法，创建了 create_zencoding
	的构建方式；

	为了检索方便，还额外仿照 css 定位器，创建了按位置、id、name等定位方式。


----------------------------------------------------------------------

** 如何处理属性？

打印的话，很方便；主要是解析；

此时，必须将解析进行拆分；

< \w+ key="value" />

的方式；

还有，单节点的处理；

属性可以用空格分开key与value；value必须用引号括起来（如果内部含有双引号，则外部
用单引号；默认用双引号括起来）；但是，如果同时允许单引号、双引号，就会引入多重标
准，这对于程序处理来说，不方便；

更通用的办法是，直接用转义的方式——这样，写法就唯一了。

6个预定义的转义序列（实体）：

	&lt;	<	小于号
	&gt;	>	大于号
	&amp;	&	和
	&apos;	'	单引号
	&quot;	"	双引号
	&nbsp;	' '     空格

需要注意的是，上述标记，不能嵌套使用——可类比C语言的/* */注释。

——另外，其实只有“<”和“&”两个字符是绝对禁止使用的。

每个属性之间，必须用空白符分割开。

** 字符数据

另外，如果字符数据是其他语言的脚本，可以采用 CDATA 序列；序列用 “<![CDATA[” 和
“]]>” 包裹；此时，内部的字符表示它们自己。

[14]  CharData ::=  [^<&]* - ([^<&]* ']]>' [^<&]*)

----------------------------------------------------------------------

** 注意

xml规定，根节点，只能有一个！

----------------------------------------------------------------------

** xml entities 的处理

如何理解用户的输入？

如果用户的输入含有",',<,>等等符号怎么办？含有 &lt;怎么办？同时含有这些，又怎么办
？

可以肯定的一点是，如果同时含有非法符号和xml实体，那么这个输入肯定是错误的！

当然，我觉得比较好的办法，是让库用户决定输入的内容到底是原意，还是……

不管我们采取哪个方案，都必须先实现两个函数：

escape

和

unescape

----------------------------------------------------------------------

当然，不管用户输入的意图如何，一旦输入到解析器之后，就只能有一种含义！

----------------------------------------------------------------------

** 内部编码与外部编码

xml文件中，其实有三种编码：

1. bom编码 —— 这是系统最先遇到的编码

2. <? charset ?> 编码说明 —— 这应该作为后续脚本解析标准；如果后续字符有不在标
   示的字符集里面，那么应当报错。

3. xml文本编码 —— 这是xml文件正文编码。应该有其唯一性；如果不满足，就应当报错
   。

基于使用场景的分析——无非就两种使用场景：1. 创建（数据源、手工等等）；2. 从外部
文件读取并解析——然后允许用户修改其中节点，并保存。

----------------------------------------------------------------------

* 现有的问题

** 解析与再现的割裂

xml的解析，标记化，是tokenizer + token 实现的；将元素构建成xml树形，是xml_parser
完成的。

输出是xml_node对象族完成的；

比如cdata块，其识别部分是tokenizer；输出在node_cdata::print_impl；即<[CDATA[ 和
]]>分别在两处，以不同的形式出现……

这就造成了一种信息的割裂……

问题不大，却让人感觉“如鲠在喉”。

* bug

** 标签名不支持下划线‘_’
date:2015-02-15

问题描述：

// <healskpos>
//  <dblogin>
//   <host>192.168.8.10</host>
//   <user>lzdyf</user>
//   <password_enc></password_enc>
//   <password>ws65635588</password>
//   <dbname>st_ccerp</dbname>
//  </dblogin>
// </healskpos>

结果，在使用 xmlcfg_file 来检索 值的时候，

如果<password_enc>在<password>前，那么

cfg.get_string("healskpos>dblogin>password")

会得到 ""；

如果<password_enc>在<password>后，那么

cfg.get_string("healskpos>dblogin>password")

会得到 "ws65635588"；

更改：

part1:

// D:\Program\MSYS\extra\sss\include\sss\regex\simpleregex.cpp|101
//
//! new
//                case ALPHA_MATCH:
//                    if (std::isalpha(ch) || ch == '_') {
//                        return true;
//                    }
//                    break;
//
//                case ALNUM_MATCH:
//                    if (std::isalnum(ch) || ch == '_') {
//                        return true;
//                    }
//                    break;
//! old
//                case ALPHA_MATCH:
//                    if (std::isalpha(ch)) {
//                        return true;
//                    }
//                    break;
//
//                case ALNUM_MATCH:
//                    if (std::isalnum(ch)) {
//                        return true;
//                    }
//                    break;

part2

// 从 DEBUG 的输出可见，tokenizer 已经支持'_'；但是，xml的输出，还是不支持 '_'
//
// 即，有可能在构造器这里出问题了。

part3
d:\program\msys\extra\sss\include\sss\xml\token.cpp|49
原始版本，还手动对 <password_enc> 这样进行 <, password_enc, properties_str, > 的
拆分；

新版本，则使用了 sss::regex::simpleregex；于是，在对'-'的处理上，有了一致性；问
题解决。

