#ifndef  __XML_SELECTION_HPP_1445839304__
#define  __XML_SELECTION_HPP_1445839304__

#include "xml_node.hpp"

#include <string>

namespace sss{
	namespace xml3{

class EleHandle;

class Selection
{

public:

	Selection(node* apNode);

	Selection(node_list_t aNodes);

	virtual ~Selection();

public:

	Selection find(std::string aSelector);

	EleHandle nodeAt(size_t i);

	size_t nodeNum();

private:
	node_list_t mNodes;
};
	}
}



#endif  /* __XML_SELECTION_HPP_1445839304__ */
