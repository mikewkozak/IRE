//Includes for Tree representation of tech tree
#include <iostream> // std::cout
#include <utility> // std::pair
#include <algorithm>                 // for std::for_each
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/reverse_graph.hpp>

//Includes for BWAPI types
#include <BWAPI.h>

#include <boost/geometry.hpp>
namespace bg = boost::geometry;


// create a typedef for the Graph type
//Vertex container
struct Vertex {
	BWAPI::UnitType node;
	std::string name;
	int strength;
	bg::model::point<double, 3, bg::cs::cartesian> location;
};

// Graph.
typedef boost::adjacency_list< boost::vecS,
	boost::vecS,
	boost::bidirectionalS,
	Vertex,
	int
> Graph;

//Reverse graph. Necessary to traverse to root
typedef boost::reverse_graph<Graph> Rgraph;

//Graph descriptors
typedef Graph::edge_descriptor EdgeDescriptor;
typedef typename boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;

//Graph iterators
typedef boost::graph_traits<Graph>::edge_iterator EdgeIterator;
typedef boost::graph_traits<Graph>::vertex_iterator VertexIterator;


#pragma once
class StrategySpace
{
public:
	//x axis
	const int AIR_AA_AXIS = 0;
	const int AIR_AA_AXIS_MAX = 1;
	const int AIR_AA_AXIS_MIN = -1;

	//y axis
	const int GROUND_AG_AXIS = 1;
	const int GROUND_AG_AXIS_MAX = 1;
	const int GROUND_AG_AXIS_MIN = -1;

	//z axis
	const int AGGRESSIVE_DEFENSIVE_AXIS = 2;
	const int AGGRESSIVE_DEFENSIVE_AXIS_MAX = 1;
	const int AGGRESSIVE_DEFENSIVE_AXIS_MIN = -1;


	StrategySpace();
	~StrategySpace();

	void layOutStrategy();

private:
	Graph strategySpace;
};

