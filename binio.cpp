#include "binio.hpp"

#include <string>
#include <vector>
#include <cstring>
#include <sstream>

#include <fstream>
#include <streambuf>

void sss::read_require(std::FILE * fp, const void * pData, size_t len, const std::string& msg)
{
#ifdef MSG
    printf("%s\n", __func__);
    long int pos = ftell(fp);
#endif
    std::vector<char> buffer(len);
    if (1 != fread(buffer.data(), len, 1, fp))
    {
        throw(msg);
    }
#ifdef MSG
    for ( int i = 0; i < static_cast<int>(buffer.size()); ++i)
        printf("%02x", buffer.data()[i]);
    putchar('|');
    for ( int i = 0; i < static_cast<int>(len); ++i)
        printf("%02x", reinterpret_cast<const char*>(pData)[i]);
    printf("|%ld\n", pos);
#endif
    if (std::memcmp(buffer.data(), pData, len))
    {
        fseek(fp, -static_cast<int>(len), SEEK_CUR);
        throw(msg);
    }
}

std::string sss::read_stop_at(std::FILE * fp, char delimiter)
{
    // FIXME 注意结尾的 '\x00' 是否需要叫到字符串里面！
    std::string ret;
    char ch;
    while (fread(&ch, 1, 1, fp) == 1 && !feof(fp) && ch != delimiter)
    {
        ret += ch;
    }
    return ret;
}

std::string sss::read_stop_at_times(std::FILE * fp, const void * pattern, int pat_len, int times)
{
#ifdef MSG
    printf("%s\n", __func__);
#endif
    std::vector<char> buffer(pat_len);
    int pos = ftell(fp);
    if (fread(buffer.data(), pat_len, 1, fp) == 0)
    {
        buffer.resize(0);
        fseek(fp, pos, SEEK_SET);
    }

    int match_count = 0;
    while (match_count < times)
    {
        if (static_cast<int>(buffer.size()) >= pat_len &&
            memcmp(buffer.data() + buffer.size() - pat_len, pattern, pat_len) == 0)
        {
            ++match_count;
            if (match_count >= times)
            {
                break;
            }
            buffer.resize(buffer.size() + pat_len);
            pos = ftell(fp);
            if (fread(buffer.data() + buffer.size() - pat_len, pat_len, 1, fp) == 1)
            {
                continue;
            }

            // read block faild; so seek back; and fall through to read one char by one char
            fseek(fp, pos, SEEK_SET);
            buffer.resize(buffer.size() - pat_len);
        }
        char ch;
        if (fread(&ch, 1, 1, fp) == 1)
        {
            buffer.push_back(ch);
        }
        else
        {
            break;
        }
    }
#ifdef MSG
    printf("match find = %d\n", match_count);
#endif
    return std::string(buffer.begin(), buffer.end());
}

void sss::require_fseek_at_end(std::FILE * fp, const std::string& msg)
{
    long int pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long int fin = ftell(fp);
    fseek(fp, pos, SEEK_SET);
#ifdef MSG
    printf("%ld %ld\n", pos, fin);
#endif
    if (pos != fin)
        throw(msg);
}

//! http://blog.csdn.net/tulip527/article/details/7976471
long long sss::file2string_c(const std::string& fname, std::string& buffer)
{
    std::ifstream t;
    int length;
    t.open(fname.c_str(), std::ios::binary);         // open input file
    t.seekg(0, std::ios::end);                  // go to the end
    length = t.tellg();                         // report location (this is the length)
    t.seekg(0, std::ios::beg);                  // go back to the beginning
    buffer.resize(length);
    t.read(&buffer[0], length);                 // read the whole file into the buffer

    return length;
}

long long sss::file2string_cpp1(const std::string& fname, std::string& buffer)
{
    std::ifstream t(fname.c_str());
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    buffer.swap(str);
    return buffer.length();
}

long long sss::file2string_cpp2(const std::string& fname, std::string& buffer)
{

    std::ifstream t(fname.c_str());
    std::ostringstream oss;
    oss << t.rdbuf();
    buffer.assign(oss.str());
    return buffer.length();
}
