#include "xml_selector.hpp"

#include <stdexcept>
#include <iostream>

#include <sss/utlstring.hpp>

#include "xml_node.hpp"

namespace sss {
    namespace xml3 {

        bool Selector::match(node* apNode)
        {
            switch (mOp)
            {
            case EDummy:
                return true;

            case EEmpty: // 本身是 tag 节点；但是内部最多只能有注释！
                {
                    if (!apNode->is_type(sss::xml3::type_node)) {
                        return false;
                    }
                    for (node * n = apNode->first_child(); n; n = n->next_sibling()) {
                        if (n->is_type(sss::xml3::type_text) || n->is_type(sss::xml3::type_node)) {
                            return false;
                        }
                    }
                    return true;
                }

            case EOnlyChild: // 是父节点的唯一子节点；
                {
                    if (!apNode->is_type(sss::xml3::type_node))
                    {
                        return false;
                    }
                    node * parent = apNode->parent();
                    if (parent == NULL)
                    {
                        return false;
                    }

                    unsigned int count = 0;
                    for (node * n = parent->findChild(sss::xml3::type_node);
                         n;
                         n = n->findSibling(sss::xml3::type_node))
                    {
                        if (mOfType && apNode->get_data() == n->get_data())
                        {
                            continue;
                        }
                        count++;
                        if (count > 1)
                        {
                            return false;
                        }
                    }

                    return count == 1;
                }

            case ENthChild:
                {
                    if (!apNode->is_type(sss::xml3::type_node))
                    {
                        return false;
                    }

                    node* parent = apNode->parent();
                    if (parent == NULL)
                    {
                        return false;
                    }

                    unsigned int i = 0;
                    unsigned int count = 0;
                    for (node * n = parent->findChild(sss::xml3::type_node);
                         n;
                         n = n->findSibling(sss::xml3::type_node))
                    {
                        if (mOfType && apNode->get_data() == n->get_data())
                        {
                            continue;
                        }
                        count++;
                        if (apNode == n)
                        {
                            i = count;
                            if (!mLast)
                            {
                                break;
                            }
                        }
                    }

                    if (mLast)
                    {
                        i = count - i + 1;
                    }
                    i -= mB;
                    if (mA == 0)
                    {
                        return i == 0;
                    }

                    return i % mA == 0 && i / mA > 0;
                }

            case ETag:
                return apNode->is_type(sss::xml3::type_node) && apNode->get_data() == mTag;

            default:
                return false;
            }
        }

        // NOTE
        // 没有调用本函数的函数！
        node_list_t Selector::filter(node_list_t nodes)
        {
            node_list_t ret;
            for (node_list_t::iterator it = nodes.begin(); it != nodes.end(); it++)
            {
                node * n = *it;
                if (match(n))
                {
                    ret.push_back(n);
                }
            }
            return ret;
        }

        node_list_t Selector::matchAll(node* apNode)
        {
            node_list_t ret;
            matchAllInto(apNode, ret);
            return ret;
        }

        // 将apNode本身，以及下属所有子节点，如果符合检索条件，则以先根节点的顺序，附加到nodes中；
        void Selector::matchAllInto(node* apNode, node_list_t& nodes)
        {
            if (match(apNode))
            {
                nodes.push_back(apNode);
            }

            if (!apNode->is_type(sss::xml3::type_node))
            {
                return;
            }

            for (node * child = apNode->first_child();
                 child;
                 child = child->next_sibling())
            {
                matchAllInto(child, nodes);
            }
        }

        BinarySelector::BinarySelector(TOperator aOp, Selector* apS1, Selector* apS2)
        {
            mpS1 = apS1;
            // mpS1->retain();
            mpS2 = apS2;
            // mpS2->retain();
            mOp = aOp;
            mAdjacent = false;
        }

        BinarySelector::~BinarySelector()
        {
            if (mpS1 != NULL)
            {
                delete mpS1;
                mpS1 = NULL;
            }

            if (mpS2 != NULL)
            {
                delete mpS2;
                mpS2 = NULL;
            }
        }

        BinarySelector::BinarySelector(Selector* apS1, Selector* apS2, bool aAdjacent)
        {
            mpS1 = apS1;
            // mpS1->retain();
            mpS2 = apS2;
            // mpS2->retain();
            mOp = EAdjacent;
            mAdjacent = aAdjacent;
        }

        bool BinarySelector::match(node* apNode)
        {
            switch (mOp)
            {
            case EUnion:
                return mpS1->match(apNode) || mpS2->match(apNode);

            case EIntersection:
                return mpS1->match(apNode) && mpS2->match(apNode);

            case EChild:
                return mpS2->match(apNode) && apNode->parent() && mpS1->match(apNode->parent());

            case EDescendant:
                {
                    if (!mpS2->match(apNode))
                    {
                        return false;
                    }

                    for (node* p = apNode->parent(); p; p = p->parent())
                    {
                        if (mpS1->match(p))
                        {
                            return true;
                        }
                    }
                    return false;
                }

            case EAdjacent:
                {
                    if (!mpS2->match(apNode))
                    {
                        return false;
                    }

                    if (!apNode->is_type(type_node))
                    {
                        return false;
                    }

                    if (mAdjacent)
                    {
                        for (node * n = apNode; n; n = n->prev_sibling())
                        {
                            if (n->is_type(type_text) ||
                                n->is_type(type_comment))
                            {
                                continue;
                            }

                            return mpS1->match(n);
                        }
                        return false;
                    }

                    for (node * n = apNode; n; n = n->prev_sibling())
                    {
                        if (mpS1->match(n))
                        {
                            return true;
                        }
                    }
                    return false;
                }

            default:
                return false;
            }

            return false;
        }

        UnarySelector::UnarySelector(TOperator aOp, Selector* apS)
        {
            mpS = apS;
            // mpS->retain();
            mOp = aOp;
        }

        UnarySelector::~UnarySelector()
        {
            if (mpS != NULL)
            {
                delete mpS;
                mpS = NULL;
            }
        }

        bool UnarySelector::hasDescendantMatch(node* apNode, Selector* apS)
        {
            for (node * child = apNode->first_child(); child; child = child->next_sibling())
            {
                if (apS->match(child) ||
                    (child->is_type(type_node) && hasDescendantMatch(child, apS)))
                {
                    return true;
                }
            }
            return false;
        }

        bool UnarySelector::hasChildMatch(node* apNode, Selector* apS)
        {
            for (node * child = apNode->first_child(); child; child = child->next_sibling())
            {
                if (apS->match(child))
                {
                    return true;
                }
            }
            return false;
        }

        bool UnarySelector::match(node* apNode)
        {

            switch (mOp)
            {
            case ENot:
                return !mpS->match(apNode);

            case EHasDescendant:
                if (!apNode->is_type(type_node))
                {
                    return false;
                }
                return hasDescendantMatch(apNode, mpS);

            case EHasChild:
                if (!apNode->is_type(type_node))
                {
                    return false;
                }
                return hasChildMatch(apNode, mpS);

            default:
                return false;
            }
        }

        AttributeSelector::AttributeSelector(TOperator aOp, std::string aKey, std::string aValue)
        {
            mKey = aKey;
            mValue = aValue;
            mOp = aOp;
        }

        bool AttributeSelector::match(node* apNode)
        {
            if (!apNode->is_type(type_node))
            {
                return false;
            }

            if (apNode->has_key(mKey)) {
                std::string value = apNode->get(mKey);
                switch (mOp)
                {
                case EExists:
                    return true;

                case EEquals:
                    {
#ifdef _DEBUG_
                        std::cout
                            << "AttributeSelector::" << __func__
                            << "(" << mKey << "=" << mValue << ")"
                            << std::endl;
#endif
                    }
                    return mValue == value;

                case EIncludes:
                    for (unsigned int i = 0, j = 0; i < value.size(); i++)
                    {
                        if (value[i] == ' ' || value[i] == '\t' || value[i] == '\r' ||
                            value[i] == '\n' || value[i] == '\f' || i == value.size() - 1)
                        {
                            unsigned int length = i - j;
                            if (i == value.size() - 1)
                            {
                                length++;
                            }
                            std::string segment = value.substr(j, length);
                            if (segment == mValue)
                            {
                                return true;
                            }
                            j = i + 1;
                        }
                    }
                    return false;

                case EDashMatch:
                    if (mValue == value)
                    {
                        return true;
                    }
                    if (value.size() < mValue.size())
                    {
                        return false;
                    }
                    return
                        value.substr(0, mValue.size()) == mValue &&
                        value[mValue.size()] == '-';

                case EPrefix:
                    return
                        value.size() >= mValue.size() &&
                        value.substr(0, mValue.size()) == mValue;

                case ESuffix:
                    return
                        value.size() >= mValue.size() &&
                        value.substr(value.size() - mValue.size(), mValue.size()) == mValue;

                case ESubString:
                    return value.find(mValue) != std::string::npos;

                default:
                    return false;
                }
            }
            return false;
        }

        bool TextSelector::match(node* apNode)
        {
            std::string text;
            switch (mOp)
            {
            case EContains:
                text = apNode->text();
                break;

            case EOwnContains:
                throw std::runtime_error("EOwnContains not implied.");
                break;

            default:
                return false;
            }

            sss::to_lower(text);
            return text.find(mValue) != std::string::npos;
        }

    }
}
