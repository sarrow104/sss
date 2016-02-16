#include "ArgsStringBuilder.hpp"

#include <istream>

namespace {
    typedef std::string::const_iterator iterator;

    bool parseLBrace(iterator& it_beg, iterator it_end)
    {
        if (it_beg != it_end && *it_beg == '{') {
            it_beg++;
            return true;
        }
        return false;
    }

    bool parseRBrace(iterator& it_beg, iterator it_end)
    {
        if (it_beg != it_end && *it_beg == '}') {
            it_beg++;
            return true;
        }
        return false;
    }

    bool parseUint(iterator& it_beg, iterator it_end, uint32_t& val)
    {
        if (it_beg != it_end && std::isdigit(*it_beg) && *it_beg != '0') {
            val = *it_beg - '0';
            it_beg ++;
            while (it_beg != it_end && std::isdigit(*it_beg)) {
                val *= 10;
                val += *it_beg - '0';
                it_beg ++;
            }
            return true;
        }
        return false;
    }

    void parser(iterator it_beg, iterator it_end)
    {
        if (it_beg == it_end) {
            return;
        }

        uint32_t index = 0;
        while (it_beg != it_end) {
            iterator it_sav = it_beg;
            if (parseLBrace(it_beg, it_end) &&
                parseUint(it_beg, it_end, index) &&
                parseRBrace(it_beg, it_end))
            {
                std::cout << "\n" << sss::util::StringSlice<iterator>(it_sav, it_beg) << ":" << index << std::endl;
            }
            else {
                it_beg = it_sav;
                std::cout << *it_beg++;
            }
        }
    }

}

namespace sss {
    namespace util {
        ArgsStringBuilder::ArgsStringBuilder(const std::string& format)
            : m_format(format)
        {
            //     parser(m_format.begin(), m_format.end());

            typedef std::string::const_iterator iterator;
            iterator it_beg = m_format.begin();
            iterator it_end = m_format.end();

            if (it_beg == it_end) {
                return;
            }

            iterator last_stem_it = it_beg;
            while (it_beg != it_end) {
                uint32_t index = 0;
                iterator it_sav = it_beg;
                if (parseLBrace(it_beg, it_end) &&
                    parseUint(it_beg, it_end, index) &&
                    parseRBrace(it_beg, it_end))
                {
                    this->m_stems.push_back(StringSlice<iterator>(last_stem_it, it_sav));
                    this->m_varibles.push_back(index);
                    last_stem_it = it_beg;
                }
                else {
                    it_beg = it_sav + 1;
                }
            }
            if (last_stem_it != it_end) {
                this->m_stems.push_back(StringSlice<iterator>(last_stem_it, it_end));
                this->m_varibles.push_back(0);
                last_stem_it = it_end;
            }

#ifdef _DEBUG_
            for (size_t i = 0; i != this->m_stems.size(); ++i) {
                std::cout << this->m_stems[i];
                if (this->m_varibles[i]) {
                    std::cout << '{' << this->m_varibles[i] << '}';
                }
                else {
                    std::cout << "{0}";
                }
            }
            std::cout << std::endl;
#endif
        }

        ArgsStringBuilder::~ArgsStringBuilder()
        {
        }

        void ArgsList::print(std::ostream& o) const
        {
            if (m_pbuilder) {

                for (size_t i = 0; i != m_pbuilder->m_stems.size(); ++i) {
                    std::cout << m_pbuilder->m_stems[i];
                    uint32_t idx = m_pbuilder->m_varibles[i];
                    ArgHolder * ph = 0;
                    if (idx && idx <= this->size() && (ph = this->operator[](idx - 1))) {
                        std::cout << *ph;
                    }
                }
            }
            else {
                int cnt = 0;
                for (const_iterator it = this->begin(); it != this->end(); ++it) {
                    o << ++cnt << ":`" << *(*it) << "`" << std::endl;
                }
            }
        }
    }
}
