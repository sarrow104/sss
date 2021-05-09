#ifndef __UTF8RAPID_HPP_1476972067__
#define __UTF8RAPID_HPP_1476972067__

namespace sss {
namespace util {

template <typename TChar = char>
struct UTF8rapid {
    typedef TChar Ch;

    enum { supportUnicode = 1 };

    /*
     * U-00000000 - U-0000007F : 0xxxxxxx (00000000 - 01111111) ( 0-127 )
     * U-00000080 - U-000007FF : 110xxxxx 10xxxxxx (11000010 10000000 - ...) (128-2047)
     * U-00000800 - U-0000FFFF : 1110xxxx 10xxxxxx 10xxxxxx (11100000 10100000 10000000 - ...) (2048-65535)
     * U-00010000 - U-001FFFFF : 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx (11110000 10010000 10000000 10000000 - ...)(65536-2097151)
     * U-00200000 - U-02FFFFFF : 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx (11111000 10001000 10000000 10000000 10000000 - ...)(2097152-50331647)
     * U-03000000 - U-7FFFFFFF : 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx (11111100 10000111 10000000 10000000 10000000 10000000 - ...)(50331648-2147483647)
     */
    // 首字节，搭配尾字节分别是
    // h0 00000000 - 01111111 0x00-0x7F 128
    // ta 10000000 - 10111111 0x80-0xBF 64
    // h2 11000010 - 11011111 0xC2-0xDF 30
    // h3 11100000 - 11101111 0xE0-0xEF 16
    // h4 11110000 - 11110111 0xF0-0xF7 8
    // h5 11111000 - 11111011 0xF8-0xFB 4
    // h6 11111100 - 11111101 0xFC-0xFD 2
    template <typename OutputStream>
    static void Encode(OutputStream& os, unsigned codepoint)
    {
        if (codepoint <= 0x7F)
            os.putChar(static_cast<Ch>(codepoint & 0xFF));
        else if (codepoint <= 0x7FF) {
            os.putChar(static_cast<Ch>(0xC0 | ((codepoint >> 6) & 0xFF)));
            os.putChar(static_cast<Ch>(0x80 | ((codepoint & 0x3F))));
        }
        else if (codepoint <= 0xFFFF) {
            os.putChar(static_cast<Ch>(0xE0 | ((codepoint >> 12) & 0xFF)));
            os.putChar(static_cast<Ch>(0x80 | ((codepoint >> 6) & 0x3F)));
            os.putChar(static_cast<Ch>(0x80 | (codepoint & 0x3F)));
        }
        else if (codepoint <= 0x1FFFFF) {
            os.putChar(static_cast<Ch>(0xF0 | ((codepoint >> 18) & 0xFF)));
            os.putChar(static_cast<Ch>(0x80 | ((codepoint >> 12) & 0x3F)));
            os.putChar(static_cast<Ch>(0x80 | ((codepoint >> 6) & 0x3F)));
            os.putChar(static_cast<Ch>(0x80 | (codepoint & 0x3F)));
        }
        else if (codepoint <= 0x2FFFFFF) {
            os.putChar(static_cast<Ch>(0xF4 | ((codepoint >> 24) & 0xFF)));
            os.putChar(static_cast<Ch>(0x80 | ((codepoint >> 18) & 0x3F)));
            os.putChar(static_cast<Ch>(0x80 | ((codepoint >> 12) & 0x3F)));
            os.putChar(static_cast<Ch>(0x80 | ((codepoint >> 6) & 0x3F)));
            os.putChar(static_cast<Ch>(0x80 | (codepoint & 0x3F)));
        }
        else { // codepoint <= 0x7FFFFFFF
            os.putChar(static_cast<Ch>(0xF6 | ((codepoint >> 30) & 0xFF)));
            os.putChar(static_cast<Ch>(0x80 | ((codepoint >> 24) & 0x3F)));
            os.putChar(static_cast<Ch>(0x80 | ((codepoint >> 18) & 0x3F)));
            os.putChar(static_cast<Ch>(0x80 | ((codepoint >> 12) & 0x3F)));
            os.putChar(static_cast<Ch>(0x80 | ((codepoint >> 6) & 0x3F)));
            os.putChar(static_cast<Ch>(0x80 | (codepoint & 0x3F)));
        }
    }

    // template <typename OutputStream>
    // static void EncodeUnsafe(OutputStream& os, unsigned codepoint)
    // {
    //     if (codepoint <= 0x7F)
    //         PutUnsafe(os, static_cast<Ch>(codepoint & 0xFF));
    //     else if (codepoint <= 0x7FF) {
    //         PutUnsafe(os, static_cast<Ch>(0xC0 | ((codepoint >> 6) & 0xFF)));
    //         PutUnsafe(os, static_cast<Ch>(0x80 | ((codepoint & 0x3F))));
    //     }
    //     else if (codepoint <= 0xFFFF) {
    //         PutUnsafe(os, static_cast<Ch>(0xE0 | ((codepoint >> 12) & 0xFF)));
    //         PutUnsafe(os, static_cast<Ch>(0x80 | ((codepoint >> 6) & 0x3F)));
    //         PutUnsafe(os, static_cast<Ch>(0x80 | (codepoint & 0x3F)));
    //     }
    //     else {
    //         PutUnsafe(os, static_cast<Ch>(0xF0 | ((codepoint >> 18) & 0xFF)));
    //         PutUnsafe(os, static_cast<Ch>(0x80 | ((codepoint >> 12) & 0x3F)));
    //         PutUnsafe(os, static_cast<Ch>(0x80 | ((codepoint >> 6) & 0x3F)));
    //         PutUnsafe(os, static_cast<Ch>(0x80 | (codepoint & 0x3F)));
    //     }
    // }

    template <typename InputStream>
    static bool Decode(InputStream& is, unsigned* codepoint)
    {
        // NOTE TRANS()
        // 宏，是一个状态机检查器——它用来检查，当前Toke到的字节，是否符合验算标准；
        // 算式中，c 保存的是当前获取的字节；算式保存的值是 bool
        // result；这个值只有false,true两种结果。即，仅当 result 以及
        // 右侧算出来的值，都是 true的时候，最后得到的结果才是，true。
        //! h2
        // 比如，对于tail-byte，其二进制形如
        // 10xxxxxx，即，需要对最高两个bit，进行确认。
        // TAIL()宏，分为两步，分包是 COPY()和TRANS(0x70);
        // COPY()宏，就不用说了，就是从流中读取一个字节，存放到c中；再从c中，
        // 截取最后6个bit，再push到*codepoint中。
        // 接着，进入TRANS(0x70)；它会对获取到的最后一个字节，也就是c的值，进行一次校验。
        // 此时，该c有三种情况，分别是0xxxxxxx,10xxxxxx,11xxxxxx;
        // 对于校验tail-byte来说，要求仅第二种情况，得到true的结果；其余两种，得到false；
        // 按数学函数中，定义域、值域的说法为：
        // 1. [0x00, 0x7F] false
        // 2. [0x80, 0xBF] true
        // 3. [0xC0, 0xFF] false
        // 第一种情况下，GetRange()返回的都是0，(0 & 0x70) != 0 得到
        // false，进行&运算。
        // 第二种情况下，GetRange()返回值为 0x10-0x40不等，于是，右侧算得true。
        // 第三种情况，GetRange() 返回值，都在0x70已下；于是，右侧算得false。
        // 即，以0x70为掩码，TRANS()宏，可以保证result值，为所求。
        //
        //! h3 11100000 - 11101111
        // 按跳转表"值"的情况，分为4个区段，分别是
        //    0xE0      10 { COPY(); TRANS(0x20); TAIL(); }
        //    0xE1-0xEC 3  { TAIL(); TAIL(); }
        //    0xED      4  1110 1101 { COPY(); TRANS(0x50); TAIL(); }
        //                 —— 有效值，仅32组(每组64个，tail)；应该是unicode的空缺
        //    0xEE-0xEF 3  { TAIL(); TAIL(); }
        //
        //! h4 11110000 - 11110111 0xF0-0xF7
        //    0xF0      11 { COPY(); TRANS(0x60); TAIL(); TAIL(); }
        //    0xF1-0xF3 6  { TAIL(); TAIL(); TAIL(); }
        //    0xF4      5  1111 0100 { COPY(); TRANS(0x10); TAIL(); TAIL(); }
        //                 —— 有效值，仅32组(每组64个，tail)；应该是unicode的空缺
        //    0xF5-0xF7 8  { false }
        //
        //! h5,h6
        //  {false}
        // 按跳转表"值"的情况，分为4个区段，分别是
        //-------
        // COPY() = { push 6 bits }
        // TRANS() = { valided current byte; to result }
        // TAIL() = { COPY(); TRANS(0x70); }
#define COPY()     \
    c = is.Take(); \
    *codepoint = (*codepoint << 6) | (static_cast<unsigned char>(c) & 0x3Fu)
#define TRANS(mask) \
    result &= ((GetRange(static_cast<unsigned char>(c)) & mask) != 0)
#define TAIL() \
    COPY();    \
    TRANS(0x70)
        typename InputStream::Ch c = is.Take();
        // ASC7
        if (!(c & 0x80)) {
            *codepoint = static_cast<unsigned char>(c);
            return true;
        }

        unsigned char type = GetRange(static_cast<unsigned char>(c));
        if (type >= 32) { // type >= 0x20
            *codepoint = 0;
        }
        else {
            *codepoint = (0xFF >> type) & static_cast<unsigned char>(c);
        }
        bool result = true;
        switch (type) {
            case 2:
                TAIL();
                return result;
            case 3:
                TAIL();
                TAIL();
                return result;
            case 4:
                COPY();
                TRANS(0x50);
                TAIL();
                return result;
            case 5:
                COPY();
                TRANS(0x10);
                TAIL();
                TAIL();
                return result;
            case 6:
                TAIL();
                TAIL();
                TAIL();
                return result;
            case 10:
                COPY();
                TRANS(0x20);
                TAIL();
                return result;
            case 11:
                COPY();
                TRANS(0x60);
                TAIL();
                TAIL();
                return result;
            default:
                return false;
        }
#undef COPY
#undef TRANS
#undef TAIL
    }

    template <typename InputStream, typename OutputStream>
    static bool Validate(InputStream& is, OutputStream& os)
    {
#define COPY() os.putChar(c = is.Take())
#define TRANS(mask) \
    result &= ((GetRange(static_cast<unsigned char>(c)) & mask) != 0)
#define TAIL() \
    COPY();    \
    TRANS(0x70)
        Ch c;
        COPY();
        if (!(c & 0x80)) return true;

        bool result = true;
        switch (GetRange(static_cast<unsigned char>(c))) {
            case 2:
                TAIL();
                return result;
            case 3:
                TAIL();
                TAIL();
                return result;
            case 4:
                COPY();
                TRANS(0x50);
                TAIL();
                return result;
            case 5:
                COPY();
                TRANS(0x10);
                TAIL();
                TAIL();
                return result;
            case 6:
                TAIL();
                TAIL();
                TAIL();
                return result;
            case 10:
                COPY();
                TRANS(0x20);
                TAIL();
                return result;
            case 11:
                COPY();
                TRANS(0x60);
                TAIL();
                TAIL();
                return result;
            default:
                return false;
        }
#undef COPY
#undef TRANS
#undef TAIL
    }

    // 可以这样操作，用我的库，再生成一个256的数组，以对比一遍。
    static unsigned char GetRange(unsigned char c)
    {
        // Referring to DFA of http://bjoern.hoehrmann.de/utf-8/decoder/dfa/
        // With new mapping 1 -> 0x10, 7 -> 0x20, 9 -> 0x40, such that AND
        // operation can test multiple types.
        static const unsigned char type[] = {
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0-31   0x00-0x1F
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 32-63  0x20-0x3F
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 64-95  0x40-0x5F
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 96-127 0x60-0x7F
            0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10, // 128-143 0x80-0x8F
            0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40, // 144-159 0x90-0x9F
            0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20, // 160-175 0xA0-0xAF
            0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20, // 176-191 0xB0-0xBF
            8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // 192-223  0xC0-0xDF
            10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8, // 224-255 0xE0-0xFF
        };
        return type[c];
    }

    template <typename InputByteStream>
    static TChar TakeBOM(InputByteStream& is)
    {
        typename InputByteStream::Ch c = Take(is);
        if (static_cast<unsigned char>(c) != 0xEFu) return c;
        c = is.Take();
        if (static_cast<unsigned char>(c) != 0xBBu) return c;
        c = is.Take();
        if (static_cast<unsigned char>(c) != 0xBFu) return c;
        c = is.Take();
        return c;
    }

    template <typename InputByteStream>
    static Ch Take(InputByteStream& is)
    {
        return static_cast<Ch>(is.Take());
    }

    template <typename OutputByteStream>
    static void PutBOM(OutputByteStream& os)
    {
        os.putChar(static_cast<typename OutputByteStream::Ch>(0xEFu));
        os.putChar(static_cast<typename OutputByteStream::Ch>(0xBBu));
        os.putChar(static_cast<typename OutputByteStream::Ch>(0xBFu));
    }

    template <typename OutputByteStream>
    static void Put(OutputByteStream& os, Ch c)
    {
        os.putChar(static_cast<typename OutputByteStream::Ch>(c));
    }
};

} // namespace util
} // namespace sss


#endif /* __UTF8RAPID_HPP_1476972067__ */
