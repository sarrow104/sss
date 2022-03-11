#ifndef __INISTREAMREADER_HPP_1465309881__
#define __INISTREAMREADER_HPP_1465309881__

#include <cassert>
#include <string>

namespace sss {
// dosini 的流式处理工具。
// 让 sss::dosini的构造，依赖于流式处理工具IniStreamReader；
// 并且，这个工具可以设置skip模式，以便自动过滤注释、空行等类型；
// 本质上，dosini是以行为处理对象的。
//
// 另外，api命名，参考 Qtskd的 QXmlStreamReader，以及
// QXmlStreamWriter
class IniStreamReader {
public:
    enum elementType {
        TYPE_INITIATE  = 0,
        TYPE_ATEND     = 1,
        TYPE_UNKOWN    = TYPE_ATEND     << 1U,
        TYPE_SECTION   = TYPE_UNKOWN    << 1U,
        TYPE_KEYVALUE  = TYPE_SECTION   << 1U,
        TYPE_EMPTYLINE = TYPE_KEYVALUE  << 1U,
        TYPE_COMMENT   = TYPE_EMPTYLINE << 1U,
    };

public:
    IniStreamReader();
    ~IniStreamReader();

public:
    void setSkipType(elementType);
    elementType skipType() const;

    bool atEnd() const;

    bool isTypeOfElement(elementType type) const;
    bool isSectionElement() const;
    bool isEmptyLineElement() const;
    bool isKeyValueElement() const;
    bool isCommentElement() const;

    // 当isSectionElement的时候，返回section()名字；
    // 当isKeyValueElement的时候，返回key()
    std::string name() const;
    std::string value() const;

    void readNext();
    void readNextKeyValue();
    void readNextSection();

    // 将重置内部状态；并设置m_in为&in
    void setDevice(std::istream& in);

    std::string currentLine() const { return m_current_line; }
protected:
    void setTypeUknown();
    void readNext(elementType type);

private:
    std::istream* m_in;
    std::string m_current_line;
    int m_ranges[4];
    elementType m_type_state;
    elementType m_type_to_skip;
};

inline IniStreamReader::elementType operator|(IniStreamReader::elementType l,
                                              IniStreamReader::elementType r)
{
    return IniStreamReader::elementType(unsigned(l) | unsigned(r));
}
// std::ostream
// 有两种，一种是可以回溯(std::ifstream&)；一种是不可回溯(std::cin&)
// 对比：
// sss::xml_parser 这是基于字符串的；
// sss::dosini 则是基于流的(构造函数中，从外部文件中读取，并按行解析)
//
// 现在问题来了，我应该如何设计？

// NOTE 使用的时候，先读，再判断 atEnd()；然后取值或退出；
// IniStreamReader i {std::ifstream{"/path/to/dosini.ini"}};
// do {
//   i.readNext();
// } while (!i.atEnd())
}  // namespace sss

#endif /* __INISTREAMREADER_HPP_1465309881__ */
