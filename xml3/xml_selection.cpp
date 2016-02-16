#include "xml_selection.hpp"

#include <memory>
#include <set>

#include <sss/util/Memory.hpp>

#include "xml_selector.hpp"
#include "xml_handle.hpp"

#include "selector_parser.hpp"
#include "xml_node.hpp"

namespace {
    void unionInnerSet(const sss::xml3::node_list_t& list_in,
                       sss::xml3::node_list_t& list_out,
                       std::set<sss::xml3::node*>& node_set)
    {
        for (sss::xml3::node_list_t::const_iterator it = list_in.begin(); it != list_in.end(); it++)
        {
            sss::xml3::node* pNode = *it;
            if (node_set.find(pNode) == node_set.end()) {
                node_set.insert(pNode);
                list_out.push_back(pNode);
            }
        }
    }
    sss::xml3::node_list_t unionNodes(const sss::xml3::node_list_t& list1, const sss::xml3::node_list_t& list2)
    {
        sss::xml3::node_list_t ret;
        std::set<sss::xml3::node*> node_set;
        unionInnerSet(list1, ret, node_set);
        unionInnerSet(list2, ret, node_set);
        return ret;
    }
}

namespace sss{
    namespace xml3{

        Selection::Selection(node* apNode)
        {
            mNodes.push_back(apNode);
        }

        Selection::Selection(node_list_t aNodes)
        {
            mNodes = aNodes;
        }

        Selection::~Selection()
        {
        }

        Selection Selection::find(std::string aSelector)
        {
            sss::scoped_ptr<Selector> sel(SelectorParser::create(aSelector));
            node_list_t ret;
            std::set<node*> node_set;
            for (node_list_t::iterator it = mNodes.begin(); it != mNodes.end(); it++)
            {
                node* pNode = *it;
                node_list_t matched = sel->matchAll(pNode);
                ::unionInnerSet(matched, ret, node_set);
            }
            return Selection(ret);
        }

        EleHandle Selection::nodeAt(size_t i)
        {
            if (i >= mNodes.size())
            {
                return EleHandle();
            }

            return EleHandle(mNodes[i]);
        }

        size_t Selection::nodeNum()
        {
            return mNodes.size();
        }

    }
}
