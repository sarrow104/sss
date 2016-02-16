* xhtml\readme.txt

date:2014-06-30

======================================================================

目的：模仿、继承等等，方式，构建一个(x)html的解析修改器。

需求：

现有的xml解析工具基本可用，但是属性的解析有缺失——另外，好像php文本不是严格意义
上的xml文本（<?php ... ?>）。

xhml在使用的时候，会遇到很多重复的属性定义——可能是php、jsp等生成的。最好的情况
，是遇到 class="..." 这样，预定义的css声明。

这样，可以减少数据的内存耗用。

----------------------------------------------------------------------

在什么时候解析？如何解析？

xml::tokenizer 是将整个标签<xxxx> or </xxxx>，看作是一个整体；而属性是头标签的一
部分。

就是说，属性的解析，应该延后到……

            bool test_xml_info();
            bool test_xml_comment();
            bool test_xml_node_begin();
            bool test_xml_node_end();
            bool test_xml_node_text();
            bool test_xml_node_cdata();

对，就是这里！

说白了，就是 tokenizer 在生成每个token_t对象的时候，就为该对象附加上属性！（当然
，也可以偷懒，就用一个字符串标识；或者用key-value键值对表示）

不过，关键在于，比如，xhtml的style属性，它本身还可以看作是一个key-value键值对！
就是说，至少会遇到双层嵌套的情况。

而且，针对xhtml，其元素的属性有一个特点，那就是，子元素css风格的继承性。

要描述这种继承性，肯定不能基于字符串来讨论。

更好的方式是，类似支持类的编程语言的属性来表达。

就是说，这个css字符串，我们必须解析出来，并用专门的表达方式进行表达。

这样，在视图获取某个元素的属性的时候，我们就可以用上述方式（也可以参考受限于“生
命周期”的变量），来获取相应的属性。

还有，xhtml的解析要点，也与xml不同；后者，需要有描述字符集的<?...?>结构，并且位
于序列开始；而后者，没有特别要求这个类似的结构，并且，位置也不一样。

形如：
<meta http-equiv="content-type" content="text/html; charset=UTF-8" />

一般位于：

html>head

另外，常用字符：

下面是常用的编码示例

    Arabic (ISO-8859-6)
    Catalan (ISO-8859-1)
    Chinese (Simplified) (GB2312)
    Chinese (Traditional) (BIG5)
    Danish (ISO-8859-1)
    Dutch (ISO-8859-1)
    English (ISO-8859-1)
    Esperanto (ISO-8859-3)
    Finnish (ISO-8859-1)
    French (ISO-8859-1)
    Georgian (UTF-8)
    German (ISO-8859-1)
    Hebrew (ISO-8859-8-I)
    Hungarian (ISO-8859-2)
    Irish Gaelic (ISO-8859-1)
    Italian (ISO-8859-1)
    Japanese (SHIFT_JIS)
    Korean (EUC-KR)
    Norwegian (Bokm?l) (ISO-8859-1)
    Norwegian (Nynorsk) (ISO-8859-1)
    Occitan (ISO-8859-1)
    Portuguese (Brazil) (ISO-8859-1)
    Portuguese (Portugal) (ISO-8859-1)
    Romanian (ISO-8859-2)
    Russian (ISO-8859-5)
    Slovenian (ISO-8859-2)
    Spanish (ISO-8859-1)
    Swedish (ISO-8859-1)
    Yiddish (UTF-8)

** 怎么办？

如何处理上述差异？并且，需要注意的是，浏览器，可以在读取html文本之后，再决定如何
理解内部的文字编码！

就是说，浏览器，貌似是用byte数组来存放原始读取到的数据的！之后，再根据用户选择的
编码，来解析这些原始字符数据！

并且，貌似容错性还不错！就算不能正常解析，也有显示……

于是，我应当xml_doc用到的基础函数，额外包装起来，以便xhtml能随意组合它们，按照需
要的顺序，解析输入数据……

当然，为了方便使用 xhtml_doc 对象，还需要设计一些 candy 函数。

xml有不合规的info结构

	<?xml .... ?>

xhtml 也有不合规的DOCTYPE结构：

	<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
		 "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

还有，xml的tag以及属性，都支持名字空间！

形如：

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.sothink.com/product/flashdecompiler/index.htm">
  <xsl:template match="/">
  <html>
  <body>
    <h2>My CD Collection</h2>
    <table>
    <tr><th>Title</th><th>Artist</th></tr>
    <xsl:for-each select="catalog/cd">
    <tr><td><xsl:value-of select="title"/></td><td><xsl:value-of select="artist"/></td></tr>
    </xsl:for-each>
    </table>
  </body>
  </html>
  </xsl:template>
</xsl:stylesheet>

其中，名字空间是 http://www.sothink.com/product/flashdecompiler/index.htm （W3C
建议用不容易“撞衫”的域名作为名字空间的名字，所以……）

前缀（或者，“别名”的说法，对C++程序员来说，更亲切）：
	xsl

具体的使用建议，见：

	http://blog.csdn.net/larntin2002/article/details/5652618

简单说：


	1. 把所有的名称空间声明都写在根元素中
	2. 同一名称空间，只声明一个前缀。

	如下面的 Xml 文本：

<memo xmlns='http://www.sothink.com' xmlns:html='http://www.sothink.com/product/flashdecompiler/index.htm'>
  <html:body>
    Now hear <html:i>this</html:i>
  </html:body>
</memo>

当然作为一般的 Xml 文件，彻底不用名称空间 (Namespace) 可能是更好的建议。

----------------------------------------------------------------------

** 如何同时解决xml与xhtml解析问题

1. bom问题，它们都会遇到
2. 不合规的< ... >结构，它们也都会遇到，并且各不相同。

解决方案1：

	创建子类，分别先处理bom与不合规；然后余下的部分，都按照xml的方式进行处理
	——不同的只是，xhtml所允许的tag名，有限制。
	然后属性有继承性。

解决方案2：

	将现有的xml_parser部分，就成员函数，大部分其实都可以公用；于是独立出来。
	至于具体的子类，就管理自己的成员变量，并管理如何构造与析构即可。

当然，还有方案3，那就是拷贝复制大法：

----------------------------------------------------------------------

这对解析，还有一种策略：

比如MIME文件类型，可以先判断bom之后的字符，是<?xml 还是 <!DOCTYPE 字样，以决定后
续解析，是按照xml，还是(x)html……

----------------------------------------------------------------------

让现有的xml_doc:: 作为基类，可以处理所有的<... >节点；

然后具体的子类，负责构建时候的审核以及顺序。然后调整xml_doc的顺序，将 node_info
不再作为其特殊成员变量；

（当然，也可以作为特殊的成员变量，名字叫info，或者备注什么的；针对xml，那就是
node_info类型；针对xhtml，那就是doc_type类型；）

总之，让所谓的 token能容纳所有的节点，然后xml_parser 能处理所有的构造、节点组合
；

然后让，具体的子类 xml_doc构造函数，对获取到的结果进行分析，比如针对xml，看是否
有node_info节点，并且是否唯一，位置等等。

另外，其实有一个问题需要注意，我们如何在解析之前，就知道我们处理的是xml文档，还
是xhtml文档呢？

需要有一个先验的“认知”么？

----------------------------------------------------------------------

额，我觉得还是把两个解析组件分开的好；因为，如果要兼容html的解析，那么<br>的识别
，就会成为问题；当然，还可能遇到<p>，以及其他一些不满足xml闭合规则的节点标签。

即，token 和 tokenizer 都可以共用；但是构造器parser需要另外提供；

当然，最大的问题是，只有在开始解析之后，才能知道，被解析对象所参照的具体标准版本
，才能进一步决定解析的时候，判断正误的标准！

----------------------------------------------------------------------

另外，charset应该作为xml文档和html文档的一个，独立于node_type外的属性存在——当
基类的构造完成之后，由xml_doc或者html_doc这个实体子类，完成validate；并从合适的
节点中，提取出charset信息；以便输出、取值的时候使用。

----------------------------------------------------------------------

已经将 xml_doc 对象修改为可以容纳xhtml文件对象（支持解析）；问题在于，对输出的支
持，不太友好；比如a等节点，默认是inline属性，允许在其他标签内部打印，而不用特意
换行。

类上，需要一个“先验”的对象，用于管理各种标签的打印输出属性。

----------------------------------------------------------------------

* html标签的block、inline分类明细

date:2013-06-25

trackback:http://aaagu1234.blog.163.com/blog/static/4009371520135257422224/

======================================================================

块元素(block element)m]
◎ address - 地址
◎ blockquote - 块引用
◎ center - 居中对齐块
◎ dir - 目录列表
◎ div - 常用块级容易，也是css layout的主要标签
◎ dl - 定义列表
◎ fieldset - form控制组More...
◎ form - 交互表单
◎ h1 - 大标题
◎ h2 - 副标题
◎ h3 - 3级标题
◎ h4 - 4级标题
◎ h5 - 5级标题
◎ h6 - 6级标题
◎ hr - 水平分隔线
◎ isindex - input prompt
◎ menu - 菜单列表
◎ noframes - frames可选内容，（对于不支持frame的浏览器显示此区块内容
◎ noscript - 可选脚本内容（对于不支持script的浏览器显示此内容）
◎ ol - 排序表单
◎ p - 段落
◎ pre - 格式化文本
◎ table - 表格
◎ ul - 非排序列表

内联元素(inline element)
◎ a - 锚点
◎ abbr - 缩写
◎ acronym - 首字
◎ b - 粗体(不推荐)
◎ bdo - bidi override
◎ big - 大字体
◎ br - 换行
◎ cite - 引用
◎ code - 计算机代码(在引用源码的时候需要)
◎ dfn - 定义字段
◎ em - 强调
◎ font - 字体设定(不推荐)
◎ i - 斜体
◎ img - 图片
◎ input - 输入框
◎ kbd - 定义键盘文本
◎ label - 表格标签
◎ q - 短引用
◎ s - 中划线(不推荐)
◎ samp - 定义范例计算机代码
◎ select - 项目选择
◎ small - 小字体文本
◎ span - 常用内联容器，定义文本内区块
◎ strike - 中划线
◎ strong - 粗体强调
◎ sub - 下标
◎ sup - 上标
◎ textarea - 多行文本输入框
◎ tt - 电传文本
◎ u - 下划线
◎ var - 定义变量

可变元素
可变元素为根据上下文语境决定该元素为块元素或者内联元素。
◎ applet - java applet
◎ button - 按钮
◎ del - 删除文本
◎ iframe - inline frame
◎ ins - 插入的文本
◎ map - 图片区块(map)
◎ object - object对象
◎ script - 客户端脚本

----------------------------------------------------------------------

如何处理，使用 visitor 设计模式吗？

将输出交给 visitor ？

----------------------------------------------------------------------

class Expression;
class NumberExpression;
class BinaryExpression;
class FunctionExpression;

class Expression : public ParsingTreeCustomBase
{
public:
    class IVisitor : public Interface
    {
    public:
        virtual void Visit(NumberExpression* node)=0;
        virtual void Visit(BinaryExpression* node)=0;
        virtual void Visit(FunctionExpression* node)=0;
    };

    virtual void Accept(IVisitor* visitor)=0;
};

class NumberExpression : public Expression
{
public:
    TokenValue value;

    void Accept(IVisitor* visitor){visitor->Visit(this);}
};

class BinaryExpression : public Expression
{
public:
    enum BinaryOperator
    {
        Add, Sub, Mul, Div,
    };
    Ptr<Expression> firstOperator;
    Ptr<Expression> secondOperator;
    BinaryOperator binaryOperator;

    void Accept(IVisitor* visitor){visitor->Visit(this);}
};

class FunctionExpression : public Expression
{
public:
    TokenValue functionName;
    List<Ptr<Expression>> arguments;

    void Accept(IVisitor* visitor){visitor->Visit(this);}
};

----------------------------------------------------------------------

class FunctionExpression : public Expression
{
public:
    TokenValue functionName;
    List<Ptr<Expression>> arguments;

    void Accept(IVisitor* visitor){visitor->Visit(this);}
};

class ExpressionPrinter : public Expression::IVisitor
{
public:
    WString result;

    void Visit(NumberExpression* node)
    {
        result+=node->value.stringValue;
    }

    void Visit(BinaryExpression* node)
    {
        result+=L"(";
        node->firstOperand->Accept(this);
        switch(binaryOperator)
        {
        case Add: result+=L" + "; break;
        case Sub: result+=L" - "; break;
        case Mul: result+=L" * "; break;
        case Div: result+=L" / "; break;
        }
        node->secondOperand->Accept(this);
        result+=L")";
    }

    void Visit(FunctionExpression* node)
    {
        result+=node->functionName.stringValue+L"(";
        for(int i=0;i<arguments.Count();i++)
        {
            if(i>0) result+=L", ";
            arguments[i]->Accept(this);
        }
        result+=L")";
    }
};

WString PrintExpression(Ptr<Expression> expression)
{
    ExpressionPrinter printer;
    expression->Accept(&printer);
    return printer.result;
}

----------------------------------------------------------------------

class A;
class B;
class C;

class xhtml::node {
    class Visitor {
    public:
        visit(c *
    };
};

----------------------------------------------------------------------

利用visitor设计模式，输出样式，就可以依赖于Printer具体定义——即，动态控制样式，
成为可能——就算基于同样的数据！

----------------------------------------------------------------------

** xhtml与xml

文档上说，xhtml是xml；区别在于DTD；不过，xml的xml prolog是必须要提供的，可以省略
doctype；而xhtml则可以省略xml prolog，必须提供doctype（虽然允许错误发生）。
