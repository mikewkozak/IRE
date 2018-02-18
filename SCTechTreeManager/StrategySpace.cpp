#include "stdafx.h"
#include "StrategySpace.h"


StrategySpace::StrategySpace()
{
}


StrategySpace::~StrategySpace()
{
}

void StrategySpace::layOutStrategy() {
	//all strategy vertices begin at 0,0,0
	//based on the intensity of the strategy along the various axes, produce a vector
	//for all vertices, apply the vector to the positions and multiple the intensity by the depth in the tree and spread out by number of children
	typedef boost::property_map<Graph, boost::vertex_index_t>::type IndexMap;
	IndexMap index = get(boost::vertex_index, strategySpace);

	std::cout << "vertices(g) = ";
	typedef boost::graph_traits<Graph>::vertex_iterator vertex_iter;
	std::pair<vertex_iter, vertex_iter> vp;
	
	for (vp = vertices(strategySpace); vp.first != vp.second; ++vp.first)
		std::cout << index[*vp.first] << " ";
	
	std::cout << std::endl;

	}
}