#ifndef  __SELECTOR_PARSER_HPP_1445774468__
#define  __SELECTOR_PARSER_HPP_1445774468__

#include <string>

namespace sss{
    namespace xml3 {

class Selector;

class SelectorParser
{
private:

    SelectorParser(std::string aInput);

public:

    virtual ~SelectorParser();

public:

    static Selector* create(std::string aInput);

private:

    /**
     * \code
     * SelectorGroup:
     *  -> Selector ( ',' Selector ) *
     * \endcode
     */
    Selector* parseSelectorGroup();

    Selector* parseSelector();

    Selector* parseSimpleSelectorSequence();

    void parseNth(int& aA, int& aB);

    int parseInteger();

    Selector* parsePseudoclassSelector();

    Selector* parseAttributeSelector();

    Selector* parseClassSelector();

    Selector* parseIDSelector();

    Selector* parseTypeSelector();

    bool consumeClosingParenthesis();

    bool consumeParenthesis();

    bool skipWhitespace();

    std::string parseString();

    std::string parseName();

    std::string parseIdentifier();

    bool nameChar(char c);

    bool nameStart(char c);

    bool hexDigit(char c);

    std::string parseEscape();

    // 根据当前解析到的位置，还有给出的错误信息，构造附带出错位置的，异常用信息
    // ；
    std::string error(std::string message);

private:

    std::string mInput;

    size_t mOffset;
};

    }
}

#endif  /* __SELECTOR_PARSER_HPP_1445774468__ */
