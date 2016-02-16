#ifndef  __ENCODING_HPP_1443750867__
#define  __ENCODING_HPP_1443750867__

// {-o -luchardet}

// NOTE
// uchardet检测正确率堪忧！；
//
// $ uchardet test.txt
// IBM855
// 13:24:23 sarrow@sarrow-T61:~/project/retab
// $ chardet test.txt
// test.txt: GB2312 (confidence: 0.99)
//
// test.txt
// > 中3     8
// > 2345    end
// >         中文    
//
// NOTE 另外，下面这个源自火狐的库，效果基本没变化，也测试到IBM855的结果
// sad！
//! https://github.com/batterseapower/libcharsetdetect.git
//
// 根据测试 python的 chardet模块，最准确；但是效率不是很高；
// 并且，是python外部程序；
//
//! https://github.com/zhang-xzhi/encodingchecker/blob/master/src/main/java/allen/encoding/ExtEncodingUtil.java
//
// 上面这个java版，更奇葩；它利用String(data, realEncoding)构造函数，是否抛出异常，来判断编码是否正确的；
//
// sumblime text 插件：
//! https://github.com/titoBouzout/EncodingHelper

#include <string>

namespace sss {
    class Encoding {
    public:
        static bool isCompatibleWith(std::string from, std::string to);

        //static bool isConvertable(std::string from, std::string to);

        // 确保编码满足——有些标准是包含关系，所以，有些时候，就算编码不同，也没有必要转化；
        static void ensure(std::string& str, std::string encoding);

        // 返回编码信息；
        static std::string dectect(const std::string& str);

        static std::string fencoding(const std::string& fname);

        /**
         * @brief 文件编码检测
         *
         * @param [in] content
         *       目标待检测内容；(允许bom)
         * @param [in] encodings
         *       以逗号为间隔的备选编码字符串(允许插入空格)
         *
         * @return 匹配的编码
         *       如果都匹配失败，则返回空串；
         *
         * 具体流程，依次枚举编码，然后让iconv(charset, charset)这样转换，看是否出错；
         */
        static std::string encodings(const std::string& content, const std::string& encodings);
    };
}



#endif  /* __ENCODING_HPP_1443750867__ */
