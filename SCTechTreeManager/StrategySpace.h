//Includes for Tree representation of tech tree
#include <iostream> // std::cout
#include <utility> // std::pair
#include <algorithm>                 // for std::for_each
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/reverse_graph.hpp>


// create a typedef for the Graph type
// Edge weight.
typedef boost::property<boost::edge_weight_t, int> EdgeWeightProperty;

//Vertex container
struct Vertex {
	BWAPI::UnitType node;
	int strength;

};

// Graph.
typedef boost::adjacency_list< boost::vecS,
	boost::vecS,
	boost::bidirectionalS,
	BWAPI::UnitType,
	EdgeWeightProperty
> Graph;

//Graph edge
typedef Graph::edge_descriptor Edge;

//Reverse graph. Necessary to traverse to root
typedef boost::reverse_graph<Graph> Rgraph;

// Vertex descriptor.
typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;


#pragma once
class StrategySpace
{
public:
	//x axis
	const int AIR_AA_AXIS_MAX = 1;
	const int AIR_AA_AXIS_MIN = -1;
	const int GROUND_AG_AXIS_MAX = 1;
	const int GROUND_AG_AXIS_MIN = -1;
	const int AGGRESSIVE_DEFENSIVE_AXIS_MAX = 1;
	const int AGGRESSIVE_DEFENSIVE_AXIS_MIN = -1;


	StrategySpace();
	~StrategySpace();

	void layOutStrategy();

private:
	Graph strategySpace;
};

