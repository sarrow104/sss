* C++版的JSON-c

date:2014-08-07

======================================================================

** 关于json对象的解析与构建

----------------------------------------------------------------------

** 某C风格的json解析库（JSON-c）

类型 enum json_type

	json_type_object
	json_type_boolean
	json_type_double
	json_type_int
	json_type_array
	json_type_string

基类json对象

	json_object

json-array操作函数：
	json_object_array_add
	json_object_array_get_idx
	json_object_array_length
	json_object_array_put_idx

对象增加引用计数
	json_object_get

对象减少引用计数
	json_object_put

获取值——类型错误，怎么办？

	json_object_get_boolean
	json_object_get_double
	json_object_get_int
	json_object_get_string
	json_object_get_type

判断类型
	json_object_is_type

创建 json_object 对象
	json_object_new_array
	json_object_new_boolean
	json_object_new_double
	json_object_new_int
	json_object_new_object
	json_object_new_string

获取对象键值
	json_object_object_get

添加、删除键值
	json_object_object_add
	json_object_object_del

循环
	json_object_object_foreach

序列化输出 -- 所有对象，都（输出）为字符串。
	json_object_to_json_string

解析：
	json_tokener
	json_tokener_free
	json_tokener_new

从字符串，创建 json 对象（注意，这个字符串，是C语言风格的！使用双引号、转义字符
等）
	json_tokener_parse
	json_tokener_parse_ex

----------------------------------------------------------------------
** json的BNF文法说明

object
    {}
    { members }

members
    pair
    pair , members

pair
    string : value

array
    []
    [ elements ]

elements
    value
    value , elements

value
    string
    number
    object
    array
    true
    false
    null

string
    ""
    " chars "

chars
    char
    char chars

char
    any-Unicode-character-
        except-"-or-\-or-
        control-character
    \"
    \\
    \/
    \b
    \f
    \n
    \r
    \t
    \u four-hex-digits

number
    int
    int frac
    int exp
    int frac exp

int
    digit
    digit1-9 digits
    - digit
    - digit1-9 digits

frac
    . digits

exp
    e digits

digits
    digit
    digit digits

e
    e
    e+
    e-
    E
    E+
    E-

----------------------------------------------------------------------

从文法可以看出，最基础的名词，应该是 value；再由它分为如下几种子类：

    string
    number
    object
    array
    true
    false
    null

其中
	string 类C语言中的字符串常量；
	number 类C语言中数字常量；
	object 则类脚本语言的{}对象——允许key-value对
	array  类脚本语言中的[]对象——内部是用逗号间隔的value序列

	true   特殊常量-类C的true；
	false  特殊常量-类C的false;
	null   特殊常量-类C的NULL；

----------------------------------------------------------------------

** JSONpp使用构想

为了方便设计JSONpp 的 API，构想一些使用环境是比较快捷的办法：

用例1：
	json是为了不同语言之间，传输数据而设计的，同时便于阅读以及机器解析和生成
	。

	那么，用例的第一个要求，就是能从json字符序列中创建json对象；并能将内存中
	的json对象，序列化为字符串序列；

	jsonpp::value * jv = jsonpp::parser("...");
	std::cout << *jv << std::endl;

用例2：
	基本类型判断：

	jsonpp::value * jv = jsonpp::parser("...");
	jv->is_null();
	jv->is_boolean();

	jv->is_true();
	jv->is_false();

	jv->is_object();
	jv->is_array();

	jv->is_number();
	jv->is_string();

	jv->get_type();
	jv->get_type_str();


用例3：
	字对象引用：

	std::cout << (*jv)[1] << std::endl; 	// 数组下标
	std::cout << jv->at(1) << std::endl; 	// 数组下标

	std::cout << (*jv)["id"] << std::endl; 	// 对象下标
	std::cout << jv->key("id") << std::endl; 	// 对象下标

	std::cout << jv->has_key("id") << std::endl;	// 对象属性
	std::cout << jv->size() << std::endl;		// 数组长度


----------------------------------------------------------------------
用例4：
	json 对象之间的运算，比如合并。

----------------------------------------------------------------------

** TODO

还未生成？
