* readme

date:2015-07-10

======================================================================

** DTD格式与使用简要说明 {{{1

参考：
http://www.iteye.com/topic/517520

DTD文件 是 w3公司，专门的用来校验XML文档正确性的工具；

	它当前对xml文档的结构进行规范，并对数据类型进行有限的控制；

----------------------------------------------------------------------

在xml文件上使用DTD文档规范，有两种方式；内部声明和外部声明：

内部声明

	<!DOCTYPE 根元素名 [元素声明]>

	注意，上述格式说明的方括号符号，不是表示可选！

范例：

	<?xmlversion="1.0"?>

	<!DOCTYPE note[
		<!ELEMENT note(to,from,heading,body)>
		<!ELEMENT to(#PCDATA)>
		<!ELEMENT from(#PCDATA)>
		<!ELEMENT heading(#PCDATA)>
		<!ELEMENT body(#PCDATA)>
	]>

	<note>
	 <to>Tove</to>
	 <from>Jani</from>
	 <heading>Reminder</heading>
	 <body>Don't forget me this weekend</body>
	</note>

解释：

	!DOCTYPE note
		定义此文档是 note 类型的文档。

	!ELEMENT note
		定义 note 元素有四个元素："to、from、heading、body"；
		注意，顺序也需要考虑！即，note以下，必须依次出现上述四个元素。

	!ELEMENT to
		定义 to 元素为 "#PCDATA" 类型

	!ELEMENT from
		定义 from 元素为 "#PCDATA" 类型

	!ELEMENT heading
		定义 heading 元素为 "#PCDATA" 类型

	!ELEMENT body
		定义 body 元素为 "#PCDATA" 类型

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

其中：

	PCDATA

	PCDATA 的意思是被解析的字符数据（Parsed Character DATA）。

	可把字符数据想象为 XML 元素的开始标签与结束标签之间的文本。

	PCDATA是会被解析器解析的文本。这些文本将被解析器检查实体以及标记。

	文本中的标签会被当作标记来处理，而实体会被展开。

	不过，被解析的字符数据不应当包含任何 &、< 或者 > 字符；需要使用 &amp; 、
	&lt; 以及 &gt; 实体来分别替换它们。

	CDATA

	CDATA 的意思是字符数据（Character DATA）。

	CDATA 是不会被解析器解析的文本。在这些文本中的标签不会被当作标记来对待，
	其中的实体也不会被展开。

元素的声明

	<!ELEMENT 元素名称 类别>

	or

	<!ELEMENT 元素名称 (元素内容)>

	<!ELEMENT root EMPTY> //EMPTY关键字表示元素是个空元素
        <!ELEMENT root ANY> //ANY关键字表示元素中可以出现任何内容,也可以为空
        //下面这个声明表示root中可以有文本,也可以是空
        <!ELEMENT root (#PCDATA)> //()表示一个分组,其中是放的允许在元素出现的内容,#PCDATA表示文本
        <!ELEMENT root (child)> //child是子元素的名称,这个声明表示root中必须且只能有一个child元素
        <!ELEMENT root (child1,child2)> //以逗号分隔,表示子元素依次出现
        <!ELEMENT root (child1|child2)> //竖线与"OR"的意思相近,表示root元素中只能出现child1或child2
        <!ELEMENT root (child?)> //root中child子元素可以出现一次,也可以不出现
        <!ELEMENT root (child+)> //root中child子元素至少出现一次
        <!ELEMENT root (child*)> //root中child子元素可以出现任意次数或不出现
        <!ELEMENT root (child,(a,b))> //()还可以嵌套,这里表示root元素中第一次子元素必须是child
        //紧接着是a或b
        <!ELEMENT root (child,(a,b)+)> //*,?,+这些量词可作用于分组,这里表示root元素中第一次子元素必须是child
        //紧接着是a或b出现一次或多次

	范例（xhtml1-strict.dtd）：

	<!ELEMENT table
	     (caption?, (col*|colgroup*), thead?, tfoot?, (tbody+|tr+))>

	表示，xhtml中，table的第一个子元素，要么是可省略的caption，接下来，要么N个
	col，或者N个colgroup；接着，是可省略的thead、tfoot；然后是不少于一个的
	tbody或者tr；

元素属性

	在 DTD 中，属性通过 ATTLIST 声明来进行声明。

	声明属性

	属性声明拥使用下列语法：

	<!ATTLIST 元素名称 属性名称 属性类型 默认值>

	范例：<!ATTLIST input type CDATA "text">

	表示：元素input的type属性值是文本,默认值是text;

	以下是属性类型表

	类型            描述
	--------------- --------------------------------------
	CDATA           值为字符数据 (character data)
	(en1|en2|..)    此值是枚举列表中的一个值
	ID              值为唯一的 id
	IDREF           值为另外一个元素的 id
	IDREFS          值为其他 id 的列表
	NMTOKEN         值为合法的 XML 名称
	NMTOKENS        值为合法的 XML 名称的列表
	ENTITY          值是一个实体
	ENTITIES        值是一个实体列表
	NOTATION        此值是符号的名称
	xml:            值是一个预定义的 XML 值

	默认值参数可使用下列值：

	值              解释
	--------------- --------------------------------------------------------
	值              属性的默认值.该属性可以出现,也可以不出现,
			当没有明确指定该属性时,属性值使用默认值
	#REQUIRED       属性值是必需的
	#IMPLIED        属性不是必需的,可以出现,可以不出现
	#FIXED value    属性值是固定的.属性可有可无,但有的时候,其值必须是value

	范例：

	<!ATTLIST img src CDATA #REQUIRED>		//img元素的src属性是必须的,值为字符串
	<!ATTLIST script type CDATA "text/javascript">	//script元素的type属性默认值是text/javascript
	<!ATTLIST div id ID #IMPLIED>			//div元素的id属性是唯一的ID标识,可有可无
	<!ATTLIST input type(text|radio|checkbox) "text"> //input元素的type属性是三个值中的一个,默认值是text
	<!ATTLIST label for IDREF #IMPLIED>		//label元素的for属性是页面中另一个元素的ID

	规定一个元素 square 的属性width，默认的值为“0”；DTD:

	<!ELEMENT square EMPTY>
	<!ATTLIST square width CDATA "0">

	合法的 XML:

	<square width="100" />

	在上面的例子中，"square" 被定义为带有 CDATA 类型的 "width" 属性的空元素
	。如果宽度没有被设定，其默认值为0 。

----------------------------------------------------------------------

	范例2（xhtml1-strict.dtd#table的属性）
	<!ATTLIST table
	  %attrs;
	  summary     %Text;         #IMPLIED
	  width       %Length;       #IMPLIED
	  border      %Pixels;       #IMPLIED
	  frame       %TFrame;       #IMPLIED
	  rules       %TRules;       #IMPLIED
	  cellspacing %Length;       #IMPLIED
	  cellpadding %Length;       #IMPLIED
	  >

	其中，形如 %attrs; 是DTD中的“宏”；宏定义，见<!ENTITY ...>

	<!ENTITY % attrs "%coreattrs; %i18n; %events;">

	注意，ENTITY宏在定义的时候，“%”后面的空格，是必须的！
	而使用宏的时候，则没有空格；并且，用“;”结尾；
	这写法，可以参考 & => &amp; 等xml实体的使用；

http://www.cnblogs.com/cyq1162/p/3273607.html|DTD语法，与 xsd 语法

----------------------------------------------------------------------

*** 实体声明

一般实体

	实体是用于定义用于定义引用普通文本或特殊字符的快捷方式的变量。实体引用是
	对实体的引用。实体可在内部或外部进行声明。

	一个内部实体是以<!ENTITY 实体名称 "实体的值">的形式声明的,一个外部私有实
	体是以<!ENTITY 实体名称 SYSTEM "URI/URL">格式声明 ,一个外部公共实体是以
	<!ENTITY 实体名称 PUBLIC "公共实体名称" "URI/URL"> ,其中公共实体名称和
	DOCTYPE中的公共DTD名称格式是一样的. 一个实体引用是&实体名称;格式

	//实体定义
	<!ENTITY abc "ABCabcABC">		//内部实体
	<!ENTITY abc SYSTEM "abc.ent">		//外部私有实体
	<!ENTITY test PUBLIC "-//AjaxLife//ENTITIES TEST 1 for XML//EN" "test.ent"> //外部公共实体

	//实体引用
	<abc>&abc;</abc>

----------------------------------------------------------------------

参数实体

	参数实体是只在DTD中使用的实体(并且参数实体只能在外部DTD中声明),它的声明
	语法与一般实体不同处在于其要在实体名称前加个百分号,而引用时则使用%实体名
	称;的形式

	<!ENTITY % abc "root">
	<!ELEMENT %abc; (child)>		//这句将声明元素root,具有一个子元素child

----------------------------------------------------------------------

*** XML 命名空间

XML 命名空间可提供避免元素命名冲突的方法。由于 XML 中的元素名是预定义的，当两个
不同的文档使用相同的元素名时，就会发生命名冲突。命名空间其实就是给这些标签名加个
前缀!

    <root>
        <svg:template />
        <xsl:template />
    </root>

现在,root下仍然是两个template元素,它们的节点名称仍然是template,但是它们的意义不
一样了,因为它们使用了不同的前缀!但是XML命名空间前缀需要声明才可以使用,如果不声明
,则被视为元素名称的一部分! XML 命名空间属性被放置于某个元素的开始标签之中，并使
用以下的语法：xmlns:namespace-prefix="namespaceURI" . 当一个命名空间被定义在某个
元素的开始标签中时，所有带有相同前缀的子元素都会与同一个命名空间相关联。

注意,用于标示命名空间的地址不会被解析器用于查找信息。其惟一的作用是赋予命名空间
一个惟一的名称。不过，很多公司常常会作为指针来使用命名空间指向某个实存的网页，这
个网页包含着有关命名空间的信息。

    <root xmlns:svg="http://www.svg.org" xmlns:xsl="http://www.xsl.org">
        <svg:template />
        <xsl:template />
    </root>

这样,为了区分那些名称相同而含义不同的元素,必须在每个元素名前面加前缀.其实还可以
在父级元素上声明默认命名空间,让所有没有前缀子元素的默认使用此命名空间.HTML的命名
空间便是一个例子.

    <html xmlns="http://www.w3.org/1999/xhtml">
    </html>

对于使用命名空间的XML文档，其DTD中对元素的声明也应该包含命名空间前缀(即应与文档
中所书写的一致).另外,命名空间不但作用于元素,还作用于属性

----------------------------------------------------------------------

*** DTD实例

	报纸文章

	01 <!DOCTYPE NEWSPAPER[
	02 <!ELEMENT NEWSPAPER(ARTICLE+)>
	03 <!ELEMENT ARTICLE(HEADLINE,BYLINE,LEAD,BODY,NOTES)>
	04 <!ELEMENT HEADLINE(#PCDATA)>
	05 <!ELEMENT BYLINE(#PCDATA)>
	06 <!ELEMENT LEAD(#PCDATA)>
	07 <!ELEMENT BODY(#PCDATA)>
	08 <!ELEMENT NOTES(#PCDATA)>
	09 <!ATTLIST ARTICLEAUTHOR CDATA #REQUIRED>
	10 <!ATTLIST ARTICLEEDITOR CDATA #IMPLIED>
	11 <!ATTLIST ARTICLEDATE   CDATA #IMPLIED>
	12 <!ATTLIST ARTICLEEDITION CDATA #IMPLIED>
	13 <!ENTITY NEWSPAPER       "VervetLogicTimes">
	14 <!ENTITY PUBLISHER       "VervetLogicPress">
	15 <!ENTITY COPYRIGHT       "Copyright 1998 VervetLogicPress">
	16 ]>

	产品目录

	01 <!DOCTYPE 	CATALOG[
	02 <!ENTITY  	AUTHOR	"JohnDoe">
	03 <!ENTITY  	COMPANY	"JDPowerTools,Inc.">
	04 <!ELEMENT	CATALOG(PRODUCT+)>
	05 <!ELEMENT	PRODUCT
	06 		(SPECIFICATIONS+,OPTIONS?,PRICE+,NOTES?)>
	07 <!ATTLIST	PRODUCT
	08 		NAME CDATA #IMPLIED
	09 		CATEGORY(HandTool|Table|Shop-Professional)"HandTool"
	10 		PARTNUM CDATA #IMPLIED
	11 		PLANT(Pittsburgh|Milwaukee|Chicago) "Chicago"
	12 		INVENTORY(InStock|Backordered|Discontinued) "InStock">
	13 <!ELEMENT	SPECIFICATIONS(#PCDATA)>
	14 <!ATTLIST	SPECIFICATIONS
	15 		WEIGHT	CDATA	#IMPLIED
	16 		POWER	CDATA	#IMPLIED>
	17 <!ELEMENT	OPTIONS(#PCDATA)>
	18 <!ATTLIST	OPTIONS
	19 		FINISH(Metal|Polished|Matte) "Matte"
	20 		ADAPTER(Included|Optional|NotApplicable) "Included"
	21 		CASE(HardShell|Soft|NotApplicable) "HardShell">
	22 <!ELEMENT	PRICE(#PCDATA)>
	23 <!ATTLIST	PRICE
	24 		MSRP	CDATA	#IMPLIED
	25 		WHOLESALE	CDATA	#IMPLIED
	26 		STREET	CDATA	#IMPLIED
	27 		SHIPPING	CDATA	#IMPLIED>
	28 <!ELEMENTNOTES(#PCDATA)>
	29 ]>

	图书馆

	01 <!DOCTYPE books[
	02 <!--公共标识符："-//LIBRARY//BDATA/DTD/BOOKDTD"-->
	03 <!ENTITY % attrib "(every|default)">
	04 <!--设定类型的临时实体-->
	05 <!ELEMENT books(book*)>
	06 <!--图书馆图书元素-->
	07 <!ELEMENT book(name,id,attrib+,description?,writer?,corp?,comment?)>
	08 <!--一本图书的元素-->
	09 <!ELEMENT name(#PCDATA)>
	10 <!--名字-->
	11 <!ELEMENT id EMPTY>
	12 <!--ID（空）-->
	13 <!ATTLIST id id ID #REQUIRED>
	14 <!--ID属性-->
	15 <!ELEMENT attrib EMPTY>
	16 <!--类型（空）-->
	17 <!ATTLIST attrib attrib %attrib; "default">
	18 <!--类型属性-->
	19 <!ELEMENT description (#PCDATA)>
	20 <!--描述-->
	21 <!ELEMENT writer(#PCDATA)>
	22 <!--作者-->
	23 <!ELEMENT corp(#PCDATA)>
	24 <!--出版社-->
	25 <!ELEMENT comment(#PCDATA)>
	26 <!--注释-->
	27 <!ENTITY library "OurLibrary">
	28 <!--定义图书馆产权实体-->
	29 ]>

DTD的缺陷

	利用DTD验证有效性的解析器，就能够立即对文档的完整性进行可靠的检查。DTD虽
	然比较实用，但DTD也有不少的缺陷。

	 - DTD有自己的特殊语法，其本身不是XML文档；

	 - DTD只提供了有限的数据类型，用户无法自定义类型；

	 - DTD不支持域名机制。

----------------------------------------------------------------------

实体ENTITY 允许循环定义吗？

** dtd解析库说明 {{{1

通过上面的说明，可以知道，DTD虽然不是标准xml文件，但它也是一个可用程序分析说明的
文本；

所以，它本身是可以解析的；

它有什么用？当然是在解析xml的时候，提供一个辅助验证；就是说，如果xml文件本身都是
用符合“标准”的程序生成的，并且传输的时候正常。那么xml文件，是没有必要进行验证
的！

DTD就是一个“然并卵”的东西。

估计，它唯一的作用，就是在手写xml（含xhtml）的时候，提供自动补全用……

鉴于DTD是在解析xml文件的同时，被引入xml解析器的。并且，可以不提供doctype；而且提
供的时候，还有内部与外部两种方式；比较两种引入方式，可以看到，两种方式，本质上是
一回事；

不过，xml解析器，需要为DTD的解析，提供一个入口！（即，解析出root名，以及DTD定义
的字符串）；

然后，DTD解析器，分析传入的字符串，是否是URL，来决定是从网络读取，还是直接解析传
入的字符串。

解析完成之后，就可以利用DTD数据结构，一面解析xml根节点，一边验证其正确性了。

貌似，DTD还可以提供默认值？

----------------------------------------------------------------------

当然，这个DTD解析器，可以以后完成——只要将内部DTD的描述部分隔离出来即可。

----------------------------------------------------------------------

** 用户接口与功能 {{{1

基本功能当然是提供xml结构、属性变化的时候，提供一个校验用户动态构造xml）；

还有就是xml文件的validate；

再高级一点的功能，就是作为xml文本编辑器的补全了。

----------------------------------------------------------------------

当前，就是与我的 sss::xml::xml_doc 结合，在动态构造的时候，提供一个校验。

那么，我就需要分析统计一下，我的xml_doc里面，那些方法，涉及到了，xml文件的变化，
以考虑我的dtd工具的外部接口，应该如何设计。

还有，现有我的xml库缺陷是否与dtd冲突，等等问题。

xml库，缺陷，那当然是 xml_doc对象类型，与node对象的不统一；导致，构造、复制，移
动上的困难；

比如，我很想做一个，可以直接解析xml字符串，生成一系列节点的parser，而不是，现在
这样，仅针对xml_doc的！

我发现，这竟然很困难！

因为，我的xml_doc，在使用的时候，不是用的指针，而是一个对象！

导致我的xml文档的解析结果，需要先提供一个xml_doc对象，然后在原地构建这个xml文件
的内存解析版！

我应当使用handle模式；

该handle，具体保存的是一个xml_doc，还是node，就需要动态检测；

——记得，我当初，采用对象版的设计方案，主要是为了避免手动析构……

看样子，最好的方案，还是 指针+handle！

怎么样，重新做一个xml2?

----------------------------------------------------------------------

首先，类型DTD，在提供给xml使用的时候，用的是指针（或者handle）；它应当仅属于
xml_doc对象；

往当前节点，添加内容、属性，是需要用DTD进行校验的；

我当前的策略，是没有校验；

xml_doc 根据参数、create_xxxx 方法等信息，来生成一定的节点（单个，或者vector包裹
）；至于是否合乎DTD标准，那是不管的。

----------------------------------------------------------------------

而如果要进行DTD校验，那肯定要与xml的生成相关；

我的xml节点树，是如何生成的呢？前导、属性、内部的PCDATA，再到闭合；如此往复递归
，就构成了一棵树；

那么，校验，应当发生在什么时间呢？

前导节点部分，决定了节点的标签（元素名）；此时，结合父节点，可以判定当前子节点是
否合理；

接着读取属性；此时可以判断提供的属性，是否在允许的范围之内；

接着，闭合前导标签；

可以判断必要的属性，是否提供完整；

接着，节点的内部的值；在构建完成后，可以判断，是否合乎要求；

最后，是闭合标签；此时，可以判断是否必要的子节点，都提供完备；

----------------------------------------------------------------------

就是说，在进行子节点校验的时候，需要提供父节点，以及当前欲添加的子节点的标签名；

在进行属性校验的时候，需要提供节点名，以及属性的key,value值对；

----------------------------------------------------------------------

如果要提供补全的话；则，补全子节点名：提供父节点名，插入位置序号；

补全属性：提供节点；

补全属性值，则提供节点，以及属性的名字（或者部分名字）；

----------------------------------------------------------------------

** DTD校验结构的数据结构以及构造方式 {{{1

DTD，合理子节点：

0 -> "node"
1 -> "abc|adc"
...
n -> ""

可以利用字符串查找，来判断是否合理；不对，次数，应当如何限定？

应该是类似一个链表的玩意儿……再查……

----------------------------------------------------------------------

送DTD文件描述部分，可以知道DTD文件的 <!ELEMENT note ...> 的note部分，都可以使用
实体！

这一点，严重影响了我关于如何解析DTD流的想法。我本来的想法，就是基于 note 元素名
，可以依靠来，作为树名字的id（或者属性名的一部分）。

但现在，我所能依靠的，只能是 ENTITY 的树了！

就是说，我在分别统计 ELEMENT 的序列、ATTLIST 的序列之前，需要先搞定 ENTITY 序列
。

然后用没有依赖关系的 ENTITY ，用来给 ELEMENT,ATTLIST 做替换。替换完成之后，再来
进行解析……

至于 ENTITY 之间，如何消除依赖，见：我的 dbclone 工具；

1. 顺序读取dtd流，将其转换为以下几片森林，分别是：
        ELEMENT-type
	ELEMENT-subnode
	node-ATTLIST
	node-ATTLIST-value_type
	ENTITY-entity_string

----------------------------------------------------------------------

