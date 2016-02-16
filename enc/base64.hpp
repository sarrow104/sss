#ifndef  __BASE64_HPP_1333729465__
#define  __BASE64_HPP_1333729465__

// 编码
//base64，准确说，应该是一种编码方式，谈不上什么加密解密。因为不存在什么'密钥'
//
// 规则说明：
// 将需要编码的连续三个字节，共24bit，再按6个bit拆分，做成4个二进制数；然后，映
// 射到64个可打印字符的空间中，然后输出；这就是base64编码的精髓了。
//
// 这64个可打印字符，一般是
// "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "0123456789" "+/"
// 如果，变换这个字符集，就能产生base64编码的变体。
// 另外，从描述可知：
// 1. 编码后，字节序列长度变为以前的4/3。
// 2. 可将二进制数据文件，如压缩包、jpg图片等等，都转换成可打印字符序列文件。
//
// 当然，上述规则，还有些bug——这里默认了输入序列长度是3的整数倍。针对现实，这
// 是不可能的。所以，上述编码，还有一个补丁过程：

// 当不是3的整数倍长度时，然后在后面补充为0的字节。注意，此时，余1个字节，将产生
// 的2个base64编码（8 bit % 6 bit = 2 bit; 再补上4 bit的0，再得到一个base64字符
// 。）；若余2个字节，则会产生3个base64字符（(8 bit + 8 bit) % 6 bit = 4 bit；需
// 要再补上2个bit，才能凑足6 bit）
//
// 至于纯由补足的0bit产生的base64编码，就不应该加到产生的序列中。当前的做法是，
// 补上，1到2个，不在以上64个字符空间中的字符'='，以便让生成的字符序列还是4个整
// 数倍。
//
// 至于解码，按上面的方法，逆推即可。
//
// --------
// TODO
// 增加'流'式的处理：
// int encode(C begin, C end, C out);
// int decode(C begin, C end, C out);
//
// 另外，编码、解码过程中，不应该使用assert——这样的语句直接导致程序退出；而应
// 该使用异常，或者返回错误代码；另外，提供类似Win32Api方式的，C语言接口，也是一
// 个选择——第一次调用，返回需要的缓存长度；第二次进行实际的转换。

#include <sss/enc/encbase.hpp>

#include <string>

namespace sss { namespace enc {
    class Base64 : public EncBase {

    public:
        std::string encode (const std::string& s);
        std::string decode (const std::string& s);
    };
} }


#endif  /* __BASE64_HPP_1333729465__ */
