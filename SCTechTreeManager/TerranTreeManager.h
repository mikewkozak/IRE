//Includes for BWAPI types
#include <BWAPI.h>

//Includes for Tree representation of tech tree
#include <iostream> // std::cout
#include <utility> // std::pair
#include <algorithm>                 // for std::for_each
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graphviz.hpp>



// create a typedef for the Graph type
// Edge weight.
typedef boost::property<boost::edge_weight_t, int> EdgeWeightProperty;


// Graph.
typedef boost::adjacency_list< boost::vecS,
	boost::vecS,
	boost::bidirectionalS,
	BWAPI::UnitType,
	EdgeWeightProperty
	> Graph;

// Vertex descriptor.
typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;

#pragma once
class TerranTreeManager
{
public:
	TerranTreeManager();
	~TerranTreeManager();

	void buildTree();

	/*
	Given a unit, building, or upgrade, traverse up the tree to the root and strengthen every edge along the way
	*/
	void strengthenTree(BWAPI::UnitType type);

	/*
	Given a strategy subtree, strengthen every edge in the tech tree that matches the strategy tree
	*/
	//void strengthenStrategy();

	void checkRequirements(BWAPI::UnitType type);
	void buildRequest(BWAPI::UnitType type, bool checkUnic);

	Vertex getVertex(int n) {
		return boost::vertex(n, techTree);
	}

private:
	// The Graph object
	Graph techTree;

};

