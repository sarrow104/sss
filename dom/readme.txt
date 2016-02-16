* readme.txt

date:2014-03-16

======================================================================

本模块 是 模仿 Document Object Module；

因为html tag 过多——上百个，并为了减少判断，我所以使用了模版构建的方式；

相关代码见：
.\dom\tag_element.hpp|314

另外，为了让其实例化，我使用了宏+静态注册“构建函数”的方式，见：

.\dom\tag_element.cpp|6
.\dom\factory_policy.hpp|49

----------------------------------------------------------------------

最开始，测试这个模块的时候，这些模版构建的html tag子类，都能正常注册“creater”
函数，工作得也不错；

但是，当我试着将模块，挪到namespace sss 下面，添加到libsss.a中时，再重新创建可执
行文件的时候，我发现，我的这些模版tag子类，都没有注册各自的 creater！工厂类是0注
册！

至于原因，我猜测是编译器优化的原因——__regist 这静态成员变量，因为并没有实际使
用；所以，链接后，估计相关代码就省略了……

所以，我干脆在最常使用html_doc::html_doc函数中原本的构造，语句：

        this->root = html_tag_factory::create("html");
        this->p_current = root;
        this->root->set_htmldocument(this);
        this->p_document_type = new dom::document_type;

用

    if (sss::html_tags::html::__regist) {
    }

包裹起来了；

这样，重新编译链接之后，exe文件再次工作正常。

----------------------------------------------------------------------

对比几种方式之后，我觉得，还是用同构类来实现html tag比较好。因为，本来html的行为
，就是多样的。

其行为，完全可以根据，当前html版本来限制（比如，html4.0，在输出的时候，就是大写
字母的标签；并且，部分标签可用，部分标签不可用；而xhtml,html5标准来说，输出的时
候，就是小写；并且，所有标签，都要求闭合；）

如果，针对不同的版本，再用模版构建不同的类族，也不见得有多轻松；而且，这样模版构
建的话，每个版本的html类族，基本都会含有上百个成员类；代码会膨胀的厉害；比如，当
前版本，基类html_tag有9个虚函数需要重载；html tag，当前是78个；对于模版构造来说
，这78个子类的这些虚函数，差不多要重载一半；由于这些模版构造虽然节省了代码编写，
但是生成的代码，却一点不少；相当于生成了78 * 9 / 2 = 351 个函数！

这样的话，还不如使用函数指针来表示当前版本（状态）的不同；

----------------------------------------------------------------------

比如，针对每个版本的html标准，都有一个与之对应的构造工程类（或者，内部多维数组）
与之对应；

比如， xhtml11 ，与之对应的结构就是类似：

sss::dom::html_tag_name_map::id_HTML, false, false

sss::dom::html_tag_name_map::id_HEAD, false, false
sss::dom::html_tag_name_map::id_LINK, true, true

sss::dom::html_tag_name_map::id_TITLE, false, true
sss::dom::html_tag_name_map::id_META, true, true
sss::dom::html_tag_name_map::id_IMG, true, true

sss::dom::html_tag_name_map::id_BODY, false, false
sss::dom::html_tag_name_map::id_DIV, false, false

sss::dom::html_tag_name_map::id_H1, false, true
sss::dom::html_tag_name_map::id_H2, false, true
sss::dom::html_tag_name_map::id_H3, false, true
sss::dom::html_tag_name_map::id_H4, false, true
sss::dom::html_tag_name_map::id_H5, false, true
sss::dom::html_tag_name_map::id_H6, false, true

sss::dom::html_tag_name_map::id_SPAN, false, true

sss::dom::html_tag_name_map::id_TABLE, false, false
sss::dom::html_tag_name_map::id_TBODY, false, false
sss::dom::html_tag_name_map::id_TR, false, false
sss::dom::html_tag_name_map::id_TH, false, true
sss::dom::html_tag_name_map::id_TD, false, true

sss::dom::html_tag_name_map::id_P, false, false

sss::dom::html_tag_name_map::id_HR, true, true
sss::dom::html_tag_name_map::id_BR, true, true

sss::dom::html_tag_name_map::id_STYLE, false, false

——其实，还少了一维，即名字；然后创建具体的标签的时候，就根据这些属性，选用合适
的成员函数，组成一个函数表，提供给对象使用；——相当于人工生成的虚函数表。

----------------------------------------------------------------------

或者，用 pimpl 方式？
