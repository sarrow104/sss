#include "selector_parser.hpp"
#include "xml_selector.hpp"
#include "exception.hpp"

#include <sss/util/Memory.hpp>
#include <sss/utlstring.hpp>

#include <memory>

namespace sss{
    namespace xml3 {

        SelectorParser::SelectorParser(std::string aInput) // {{{1
        {
            mInput = aInput;
            mOffset = 0;
        }

        SelectorParser::~SelectorParser() // {{{1
        {
        }

        Selector* SelectorParser::create(std::string aInput) // {{{1
        {
            try {
                SelectorParser parser(aInput);
                return parser.parseSelectorGroup();
            }
            catch (const std::string& msg){
                throw sss::xml3::SelectorParseError(msg);
            }
            catch (...) {
                throw;
            }
        }

        /**
         * \code
         * SelectorGroup
         *  -> Selector ( ',' Selector ) *
         * \endcode
         */
        Selector* SelectorParser::parseSelectorGroup() // {{{1
        {
            Selector* ret = parseSelector();
            while (mOffset < mInput.size())
            {
                if (mInput[mOffset] != ',')
                {
                    return ret;
                }

                mOffset++;

                sss::scoped_ptr<Selector> sel(parseSelector());
                sss::scoped_ptr<Selector> oldRet(ret);
                ret = new BinarySelector(BinarySelector::EUnion, ret, sel.release());
                sel.release();
                oldRet.release();
            }

            return ret;
        }

        // Selector
        //  -> SimpleSelectorSequence ( (' '|'>'|'+'|'~') SimpleSelectorSequence ) *
        Selector* SelectorParser::parseSelector() // {{{1
        {
            skipWhitespace();
            Selector* ret = parseSimpleSelectorSequence();
            while (true)
            {
                char combinator = 0;
                if (skipWhitespace())
                {
                    combinator = ' ';
                }

                if (mOffset >= mInput.size())
                {
                    return ret;
                }

                char c = mInput[mOffset];
                if (c == '+' || c == '>' || c == '~')
                {
                    combinator = c;
                    mOffset++;
                    skipWhitespace();
                }
                else if (c == ',' || c == ')')
                {
                    return ret;
                }

                if (combinator == 0)
                {
                    return ret;
                }

                sss::scoped_ptr<Selector> oldRet(ret);
                sss::scoped_ptr<Selector> sel(parseSimpleSelectorSequence());
                bool isOk = true;
                if (combinator == ' ')
                {
                    ret = new BinarySelector(BinarySelector::EDescendant, oldRet.get(), sel.get());
                }
                else if (combinator == '>')
                {
                    ret = new BinarySelector(BinarySelector::EChild, oldRet.get(), sel.get());
                }
                else if (combinator == '+')
                {
                    ret = new BinarySelector(oldRet.get(), sel.get(), true);
                }
                else if (combinator == '~')
                {
                    ret = new BinarySelector(oldRet.get(), sel.get(), true);
                }
                else
                {
                    isOk = false;
                }
                oldRet.release();
                sel.release();
                if(!isOk)
                {
                    throw error("impossible");
                }

            }
        }

        // SimpleSelectorSequence
        //   -> ('*'|TypeSelector)? IDSelector          // begin with '#'
        //   -> ('*'|TypeSelector)? lassSelector        // begin with '.'
        //   -> ('*'|TypeSelector)? AttributeSelector   // begin with '['
        //   -> ('*'|TypeSelector)? PseudoclassSelector // begin with ':'
        Selector* SelectorParser::parseSimpleSelectorSequence() // {{{1
        {
            Selector* ret = NULL;
            if (mOffset >= mInput.size())
            {
                throw error("expected selector, found EOF instead");
            }

            char c = mInput[mOffset];
            if (c == '*')
            {
                mOffset++;
            }
            else if (c == '#' || c == '.' || c == '[' || c == ':')
            {

            }
            else
            {
                ret = parseTypeSelector();
            }

            while (mOffset < mInput.size())
            {
                char c = mInput[mOffset];
                sss::scoped_ptr<Selector> sel;
                if (c == '#')
                {
                    sel.reset(parseIDSelector());
                }
                else if (c == '.')
                {
                    sel.reset(parseClassSelector());
                }
                else if (c == '[')
                {
                    sel.reset(parseAttributeSelector());
                }
                else if (c == ':')
                {
                    sel.reset(parsePseudoclassSelector());
                }
                else
                {
                    break;
                }

                if (ret == NULL)
                {
                    ret = sel.release();
                }
                else
                {
                    sss::scoped_ptr<Selector> oldRet(ret);
                    ret = new BinarySelector(BinarySelector::EIntersection, ret, sel.get());
                    sel.release();
                    oldRet.release();
                }
            }

            if (ret == NULL)
            {
                ret = new Selector();
            }

            return ret;
        }

        // Nth
        //  -> ('+'|'-')? (integer|'n'|"odd"|"even")
        void SelectorParser::parseNth(int& aA, int& aB) // {{{1
        {

            if (mOffset >= mInput.size())
            {
                goto eof;
            }

            {
                char c = mInput[mOffset];
                if (c == '-')
                {
                    mOffset++;
                    goto negativeA;
                }
                else if (c == '+')
                {
                    mOffset++;
                    goto positiveA;
                }
                else if (c >= '0' && c <= '9')
                {
                    goto positiveA;
                }
                else if (c == 'n' || c == 'N')
                {
                    goto readN;
                }
                else if (c == 'o' || c == 'O' || c == 'e' || c == 'E')
                {
                    std::string id = parseName();
                    sss::to_lower(id);
                    if (id == "odd")
                    {
                        aA = 2;
                        aB = 1;
                    }
                    else if (id == "even")
                    {
                        aA = 2;
                        aB = 0;
                    }
                    else
                    {
                        throw error("expected 'odd' or 'even', invalid found");
                    }
                    return;
                }
                else
                {
                    goto invalid;
                }
            }

positiveA:
            {
                if (mOffset >= mInput.size())
                {
                    goto eof;
                }
                char c = mInput[mOffset];
                if (c >= '0' && c <= '9')
                {
                    aA = parseInteger();
                    goto readA;
                }
                else if (c == 'n' || c == 'N')
                {
                    aA = 1;
                    mOffset++;
                    goto readN;
                }
                else
                {
                    goto invalid;
                }
            }

negativeA:
            {
                if (mOffset >= mInput.size())
                {
                    goto eof;
                }
                char c = mInput[mOffset];
                if (c >= '0' && c <= '9')
                {
                    aA = -parseInteger();
                    goto readA;
                }
                else if (c == 'n' || c == 'N')
                {
                    aA = -1;
                    mOffset++;
                    goto readN;
                }
                else
                {
                    goto invalid;
                }
            }

readA:
            {
                if (mOffset >= mInput.size())
                {
                    goto eof;
                }

                char c = mInput[mOffset];
                if (c == 'n' || c == 'N')
                {
                    mOffset++;
                    goto readN;
                }
                else
                {
                    aB = aA;
                    aA = 0;
                    return;
                }

            }

readN:
            {
                skipWhitespace();
                if (mOffset >= mInput.size())
                {
                    goto eof;
                }

                char c = mInput[mOffset];
                if (c == '+')
                {
                    mOffset++;
                    skipWhitespace();
                    aB = parseInteger();
                    return;
                }
                else if (c == '-')
                {
                    mOffset--;
                    skipWhitespace();
                    aB = -parseInteger();
                    return;
                }
                else
                {
                    aB = 0;
                    return;
                }
            }

eof:
            {
                throw error("unexpected EOF while attempting to parse expression of form an+b");
            }

invalid:
            {
                throw error("unexpected character while attempting to parse expression of form an+b");
            }

        }

        // Integer
        //   -> [0-9]+
        int SelectorParser::parseInteger() // {{{1
        {
            size_t offset = mOffset;
            int i = 0;
            for (; offset < mInput.size(); offset++)
            {
                char c = mInput[offset];
                if (c < '0' || c > '9')
                {
                    break;
                }
                i = i * 10 + c - '0';
            }

            if (offset == mOffset)
            {
                throw error("expected integer, but didn't find it.");
            }
            mOffset = offset;

            return i;
        }

        // PseudoclassSelector
        //  -> ':' ("not"|"has"|"haschild") '(' SelectorGroup ')'
        //  -> ':' ("contains"|"containsown") '(' (String | Identifier) ')' // Ã²ËÆÊÇcase sensitive
        //  -> ':' ("matches"|"matchesown") // TODO not support!
        //  -> ':' ("nth-child"|"nth-last-child"|"nth-of-type"|"nth-last-of-type") '(' Nth ')'
        //  -> ':' "first-child"
        //  -> ':' "last-child"
        //  -> ':' "first-of-type"
        //  -> ':' "last-of-type"
        //  -> ':' "only-child"
        //  -> ':' "only-of-type"
        //  -> ':' "empty"
        Selector* SelectorParser::parsePseudoclassSelector() // {{{1
        {
            if (mOffset >= mInput.size() || mInput[mOffset] != ':')
            {
                throw error("expected pseudoclass selector (:pseudoclass), found invalid char");
            }
            mOffset++;
            std::string name = parseIdentifier();
            sss::to_lower(name);
            if (name == "not" || name == "has" || name == "haschild")
            {
                if (!consumeParenthesis())
                {
                    throw error("expected '(' but didn't find it");
                }

                sss::scoped_ptr<Selector> sel(parseSelectorGroup());
                if (!consumeClosingParenthesis())
                {
                    throw error("expected ')' but didn't find it");
                }

                UnarySelector::TOperator op;
                if (name == "not")
                {
                    op = UnarySelector::ENot;
                }
                else if (name == "has")
                {
                    op = UnarySelector::EHasDescendant;
                }
                else if (name == "haschild")
                {
                    op = UnarySelector::EHasChild;
                }
                else
                {
                    throw error("impossbile");
                }
                Selector* ret = new UnarySelector(op, sel.get());
                sel.release();
                return ret;
            }
            else if (name == "contains" || name == "containsown")
            {
                if (!consumeParenthesis() || mOffset >= mInput.size())
                {
                    throw error("expected '(' but didn't find it");
                }

                std::string value;
                char c = mInput[mOffset];
                if (c == '\'' || c == '"')
                {
                    value = parseString();
                }
                else
                {
                    value = parseIdentifier();
                }
                sss::to_lower(value);
                skipWhitespace();

                if (!consumeClosingParenthesis())
                {
                    throw error("expected ')' but didn't find it");
                }

                TextSelector::TOperator op;
                if (name == "contains")
                {
                    op = TextSelector::EContains;
                }
                else if (name == "containsown")
                {
                    op = TextSelector::EOwnContains;
                }
                else
                {
                    throw error("impossibile");
                }
                return new TextSelector(op, value);
            }
            else if (name == "matches" || name == "matchesown")
            {
                //TODO
                throw error("unsupported regex");
            }
            else if (name == "nth-child" || name == "nth-last-child" || name == "nth-of-type"
                     || name == "nth-last-of-type")
            {
                if (!consumeParenthesis())
                {
                    throw error("expected '(' but didn't find it");
                }

                int a, b;
                parseNth(a, b);

                if (!consumeClosingParenthesis())
                {
                    throw error("expected ')' but didn't find it");
                }

                return new Selector(a, b, name == "nth-last-child" || name == "nth-last-of-type",
                                    name == "nth-of-type" || name == "nth-last-of-type");
            }
            else if (name == "first-child")
            {
                return new Selector(0, 1, false, false);
            }
            else if (name == "last-child")
            {
                return new Selector(0, 1, true, false);
            }
            else if (name == "first-of-type")
            {
                return new Selector(0, 1, false, true);
            }
            else if (name == "last-of-type")
            {
                return new Selector(0, 1, true, true);
            }
            else if (name == "only-child")
            {
                return new Selector(false);
            }
            else if (name == "only-of-type")
            {
                return new Selector(true);
            }
            else if (name == "empty")
            {
                return new Selector(Selector::EEmpty);
            }
            else
            {
                throw error("unsupported op:" + name);
            }
        }

        // AttributeSelector
        //  -> '[' Identifier ']'
        //  -> '[' Identifier '=' (String|Identifier)']'
        //  -> '[' Identifier ("~="|"!=" |"^=" |"$=" |"*=") (String|Identifier) ']'
        //  -> '[' Identifier '#' '=' ... // TODO support regex
        Selector* SelectorParser::parseAttributeSelector() // {{{1
        {
            if (mOffset >= mInput.size() || mInput[mOffset] != '[')
            {
                throw error("expected attribute selector ([attribute]), found invalid char");
            }
            mOffset++;
            skipWhitespace();
            std::string key = parseIdentifier();
            skipWhitespace();
            if (mOffset >= mInput.size())
            {
                throw error("unexpected EOF in attribute selector");
            }

            if (mInput[mOffset] == ']')
            {
                mOffset++;
                return new AttributeSelector(AttributeSelector::EExists, key);
            }

            if (mOffset + 2 > mInput.size())
            {
                throw error("unexpected EOF in attribute selector");
            }

            std::string op = mInput.substr(mOffset, 2);
            if (op[0] == '=')
            {
                op = "=";
            }
            else if (op[1] != '=')
            {
                throw error("expected equality operator, found invalid char");
            }

            mOffset += op.size();
            skipWhitespace();
            if (mOffset >= mInput.size())
            {
                throw error("unexpected EOF in attribute selector");
            }

            std::string value;
            if (op == "#=")
            {
                //TODO
                throw error("unsupported regex");
            }
            else
            {
                char c = mInput[mOffset];
                if (c == '\'' || c == '"')
                {
                    value = parseString();
                }
                else
                {
                    value = parseIdentifier();
                }
            }
            skipWhitespace();
            if (mOffset >= mInput.size() || mInput[mOffset] != ']')
            {
                throw error("expected attribute selector ([attribute]), found invalid char");
            }
            mOffset++;

            AttributeSelector::TOperator aop;
            if (op == "=")
            {
                aop = AttributeSelector::EEquals;
            }
            else if (op == "~=")
            {
                aop = AttributeSelector::EIncludes;
            }
            else if (op == "|=")
            {
                aop = AttributeSelector::EDashMatch;
            }
            else if (op == "^=")
            {
                aop = AttributeSelector::EPrefix;
            }
            else if (op == "$=")
            {
                aop = AttributeSelector::ESuffix;
            }
            else if (op == "*=")
            {
                aop = AttributeSelector::ESubString;
            }
            else if (op == "#=")
            {
                //TODO
                throw error("unsupported regex");
            }
            else
            {
                throw error("unsupported op:" + op);
            }
            return new AttributeSelector(aop, key, value);
        }

        // lassSelector
        //  -> '.' Identifier
        Selector* SelectorParser::parseClassSelector() // {{{1
        {
            if (mOffset >= mInput.size() || mInput[mOffset] != '.')
            {
                throw error("expected class selector (.class), found invalid char");
            }
            mOffset++;
            std::string clazz = parseIdentifier();

            return new AttributeSelector(AttributeSelector::EIncludes, "class", clazz);
        }

        // IDSelector
        //  -> '#' Name
        Selector* SelectorParser::parseIDSelector() // {{{1
        {
            if (mOffset >= mInput.size() || mInput[mOffset] != '#')
            {
                throw error("expected id selector (#id), found invalid char");
            }
            mOffset++;
            std::string id = parseName();

            return new AttributeSelector(AttributeSelector::EEquals, "id", id);
        }

        // TypeSelector
        //  -> Identifier //check by "gumbo_tag_enum"
        Selector* SelectorParser::parseTypeSelector() // {{{1
        {
            std::string tag = parseIdentifier();
            return new Selector(tag);
        }

        // ')'
        bool SelectorParser::consumeClosingParenthesis() // {{{1
        {
            size_t offset = mOffset;
            skipWhitespace();
            if (mOffset < mInput.size() && mInput[mOffset] == ')')
            {
                mOffset++;
                return true;
            }
            mOffset = offset;
            return false;
        }

        // '('
        bool SelectorParser::consumeParenthesis() // {{{1
        {
            if (mOffset < mInput.size() && mInput[mOffset] == '(')
            {
                mOffset++;
                skipWhitespace();
                return true;
            }
            return false;
        }

        // "\s"+
        bool SelectorParser::skipWhitespace() // {{{1
        {
            size_t offset = mOffset;
            while (offset < mInput.size())
            {
                char c = mInput[offset];
                if (c == ' ' || c == '\r' || c == '\t' || c == '\n' || c == '\f')
                {
                    offset++;
                    continue;
                }
                else if (c == '/')
                {
                    if (mInput.size() > offset + 1 && mInput[offset + 1] == '*')
                    {
                        size_t pos = mInput.find("*/", offset + 2);
                        if (pos != std::string::npos)
                        {
                            offset = pos + 2;
                            continue;
                        }
                    }
                }
                break;
            }

            if (offset > mOffset)
            {
                mOffset = offset;
                return true;
            }

            return false;
        }

        // String
        //  -> '"' char* '"'
        //  -> "'" char* "'"
        std::string SelectorParser::parseString() // {{{1
        {
            size_t offset = mOffset;
            if (mInput.size() < offset + 2)
            {
                throw error("expected string, found EOF instead");
            }

            char quote = mInput[offset];
            offset++;
            std::string ret;

            while (offset < mInput.size())
            {
                char c = mInput[offset];
                if (c == '\\')
                {
                    if (mInput.size() > offset + 1)
                    {
                        char c = mInput[offset + 1];
                        if (c == '\r')
                        {
                            if (mInput.size() > offset + 2 && mInput[offset + 2] == '\n')
                            {
                                offset += 3;
                                continue;
                            }
                        }

                        if (c == '\r' || c == '\n' || c == '\f')
                        {
                            offset += 2;
                            continue;
                        }
                    }
                    mOffset = offset;
                    ret += parseEscape();
                    offset = mOffset;
                }
                else if (c == quote)
                {
                    break;
                }
                else if (c == '\r' || c == '\n' || c == '\f')
                {
                    throw error("unexpected end of line in string");
                }
                else
                {
                    size_t start = offset;
                    while (offset < mInput.size())
                    {
                        char c = mInput[offset];
                        if (c == quote || c == '\\' || c == '\r' || c == '\n' || c == '\f')
                        {
                            break;
                        }
                        offset++;
                    }
                    ret += mInput.substr(start, offset - start);
                }
            }

            if (offset >= mInput.size())
            {
                throw error("EOF in string");
            }

            offset++;
            mOffset = offset;

            return ret;
        }

        // Name
        //  -> nameChar+ | Escape
        std::string SelectorParser::parseName() // {{{1
        {
            size_t offset = mOffset;
            std::string ret;
            while (offset < mInput.size())
            {
                char c = mInput[offset];
                if (nameChar(c))
                {
                    size_t start = offset;
                    while (offset < mInput.size() && nameChar(mInput[offset]))
                    {
                        offset++;
                    }
                    ret += mInput.substr(start, offset - start);
                }
                else if (c == '\\')
                {
                    mOffset = offset;
                    ret += parseEscape();
                    offset = mOffset;
                }
                else
                {
                    break;
                }
            }

            if (ret == "")
            {
                throw error("expected name, found EOF instead");
            }

            mOffset = offset;
            return ret;
        }

        // Identifier
        //  -> '-'? nameStart? Name
        std::string SelectorParser::parseIdentifier() // {{{1
        {
            bool startingDash = false;
            if (mInput.size() > mOffset && mInput[mOffset] == '-')
            {
                startingDash = true;
                mOffset++;
            }

            if (mInput.size() <= mOffset)
            {
                throw error("expected identifier, found EOF instead");
            }

            char c = mInput[mOffset];
            if (!nameStart(c) && c != '\\')
            {
                throw error("expected identifier, found invalid char");
            }

            std::string name = parseName();
            if (startingDash)
            {
                name = "-" + name;
            }

            return name;
        }

        // Check.nameChar
        //  -> nameStart | '-' | [0-9]
        bool SelectorParser::nameChar(char c) // {{{1
        {
            return nameStart(c) || (c == '-') || (c >= '0' && c <= '9');
        }

        // Check.nameStart
        //  -> [a-z]|[A-Z]|'-'| [\x7F-\xFF]
        bool SelectorParser::nameStart(char c) // {{{1
        {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c & 0x80);
        }

        // Check.hexDigit
        //  -> [0-9] | [a-f] | [A-F]
        bool SelectorParser::hexDigit(char c) // {{{1
        {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        }

        // Escape
        //  -> "\\" (!hexDigit)
        //  -> "\\" hexDigit+
        //  -> "\\" whitespace
        std::string SelectorParser::parseEscape() // {{{1
        {
            if (mInput.size() < mOffset + 2 || mInput[mOffset] != '\\')
            {
                throw error("invalid escape sequence");
            }

            size_t start = mOffset + 1;
            char c = mInput[start];
            if (c == '\r' || c == '\n' || c == '\f')
            {
                throw error("escaped line ending outside string");
            }

            if (!hexDigit(c))
            {
                std::string ret = mInput.substr(start, 1);
                mOffset += 2;
                return ret;
            }

            size_t i = 0;
            std::string ret;
            c = 0;
            for (i = start; i < mOffset + 6 && i < mInput.size() && hexDigit(mInput[i]); i++)
            {
                unsigned int d = 0;
                char ch = mInput[i];
                if (ch >= '0' && ch <= '9')
                {
                    d = ch - '0';
                }
                else if (ch >= 'a' && ch <= 'f')
                {
                    d = ch - 'a' + 10;
                }
                else if (ch >= 'A' && ch <= 'F')
                {
                    d = ch - 'A' + 10;
                }
                else
                {
                    throw error("impossible");
                }

                if ((i - start) % 2)
                {
                    c += d;
                    ret.push_back(c);
                    c = 0;
                }
                else
                {
                    c += (d << 4);
                }
            }

            if (ret.size() == 0 || c != 0)
            {
                throw error("invalid hex digit");
            }

            if (mInput.size() > i)
            {
                switch (mInput[i])
                {
                case '\r':
                    i++;
                    if (mInput.size() > i && mInput[i] == '\n')
                    {
                        i++;
                    }
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\f':
                    i++;
                    break;
                }
            }

            mOffset = i;
            return ret;
        }

        std::string SelectorParser::error(std::string message) // {{{1
        {
            size_t d = mOffset;
            std::string ds;
            if (d == 0)
            {
                ds = '0';
            }

            while (d)
            {
                ds.push_back(d % 10 + '0');
                d /= 10;
            }

            std::string ret = message + " at:";
            for (std::string::reverse_iterator rit = ds.rbegin(); rit != ds.rend(); ++rit) {
                ret.push_back(*rit);
            }

            return ret;
        }
    }
}
