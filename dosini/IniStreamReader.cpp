#include "IniStreamReader.hpp"

#include <cstdio>
#include <cctype>
#include <cstring> // std::memset
#include <iostream>

#include <sss/dosini/IniParser.hpp>

namespace sss {
    IniStreamReader::IniStreamReader()
        : m_in(0), m_type_state(TYPE_INITIATE), m_type_to_skip(TYPE_INITIATE)
    {
    }
    IniStreamReader::~IniStreamReader()
    {
    }

    void IniStreamReader::setSkipType(IniStreamReader::elementType t)
    {
        this->m_type_to_skip = t;
    }

    IniStreamReader::elementType IniStreamReader::skipType() const
    {
        return this->m_type_to_skip;
    }

    bool IniStreamReader::atEnd() const
    {
        return this->m_type_state == TYPE_ATEND;
    }

    bool IniStreamReader::isTypeOfElement(IniStreamReader::elementType type) const
    {
        return this->m_type_state == type;
    }

    bool IniStreamReader::isSectionElement() const
    {
        return this->m_type_state == TYPE_SECTION;
    }

    bool IniStreamReader::isEmptyLineElement() const
    {
        return this->m_type_state == TYPE_EMPTYLINE;
    }
    bool IniStreamReader::isKeyValueElement() const
    {
        return this->m_type_state == TYPE_KEYVALUE;
    }
    bool IniStreamReader::isCommentElement() const
    {
        return this->m_type_state == TYPE_COMMENT;
    }

    // 当isSectionElement的时候，返回section()名字；
    // 当isKeyValueElement的时候，返回key()
    std::string IniStreamReader::name() const
    {
        std::string ret;
        switch (this->m_type_state) {
        case TYPE_SECTION:
        case TYPE_KEYVALUE:
            ret = this->m_current_line.substr(m_ranges[0], m_ranges[1] - m_ranges[0]);

        default:
            break;
        }
        return ret;
    }

    std::string IniStreamReader::value() const
    {
        std::string ret;
        switch (this->m_type_state) {
        case TYPE_KEYVALUE:
            ret = this->m_current_line.substr(m_ranges[2], m_ranges[3] - m_ranges[2]);

        default:
            break;
        }
        return ret;
    }

    void IniStreamReader::readNext()
    {
        if (!this->m_in) {
            this->m_type_state = TYPE_INITIATE;
            m_current_line.resize(0);
            return;
        }
        if (!m_in->good()) {
            this->m_type_state = TYPE_ATEND;
            m_current_line.resize(0);
            return;
        }

        if (!std::getline(*m_in, m_current_line)) {
            this->m_type_state = TYPE_ATEND;
            m_current_line.resize(0);
            return;
        }

        int pos = 0;

        char none_space = sss::firstNoneSpace(m_current_line, pos);

        if (!none_space) {
            this->m_type_state = TYPE_EMPTYLINE;
        }
        else {
            switch(none_space) {
            case '[':
                if (sss::iniParseSection(m_current_line,
                                         m_ranges[0], m_ranges[1],
                                         pos))
                {
                    m_type_state = TYPE_SECTION;
                }
                else {
                    this->setTypeUknown();
                }
                break;

            case ';':
                if (pos == 0) {
                    m_type_state = TYPE_COMMENT;
                    sss::iniParseComment(m_current_line,
                                         m_ranges[0], m_ranges[1]);

                }
                else {
                    this->setTypeUknown();
                }
                break;

            default:
                if (sss::iniParseKeyValue(m_current_line,
                                          m_ranges[0], m_ranges[1],
                                          m_ranges[2], m_ranges[3],
                                          pos))
                {
                    this->m_type_state = TYPE_KEYVALUE;
                }
                else {
                    this->setTypeUknown();
                }

                break;
            }
        }
        if (this->m_type_state & this->m_type_to_skip) {
            this->readNext();
        }
    }

    void IniStreamReader::readNextKeyValue()
    {
        this->readNext(TYPE_KEYVALUE);
    }

    void IniStreamReader::readNextSection()
    {
        this->readNext(TYPE_SECTION);
    }

    void IniStreamReader::setDevice(std::istream& in)
    {
        this->m_type_state = TYPE_INITIATE;
        this->m_in = &in;
        this->m_current_line.resize(0);
        std::memset(m_ranges, 0, sizeof(m_ranges));
    }

    void IniStreamReader::setTypeUknown()
    {
        this->m_type_state = TYPE_UNKOWN;
        std::memset(m_ranges, 0, sizeof(m_ranges));
    }

    void IniStreamReader::readNext(IniStreamReader::elementType type)
    {
        while (true) {
            this->readNext();
            if (this->atEnd()) {
                break;
            }
            if (this->m_type_state == type) {
                break;
            }
            continue;
        }
    }
} // namespace sss
