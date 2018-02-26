//Includes for Tree representation of tech tree
#include <iostream> // std::cout
#include <utility> // std::pair
#include <algorithm>                 // for std::for_each
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

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
	int depth;
};

struct VertexComparator {
	bool operator() (const Vertex& lhs, const Vertex& rhs) const {
		return lhs.strength > rhs.strength;
	}
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

struct Strategy {
	Graph techTree;
	std::string name;
	double air_aa_intensity;//x -1 to 1
	double ground_ag_intensity;//y -1 to 1
	double aggressive_defensive_intensity;//z -1 to 1
};

#pragma once
class StrategySpace
{
public:
	//x axis
	static const int AIR_AA_AXIS = 0;
	static const int AIR_AA_AXIS_MAX = 1;
	static const int AIR_AA_AXIS_MIN = -1;

	//y axis
	static const int GROUND_AG_AXIS = 1;
	static const int GROUND_AG_AXIS_MAX = 1;
	static const int GROUND_AG_AXIS_MIN = -1;

	//z axis
	static const int AGGRESSIVE_DEFENSIVE_AXIS = 2;
	static const int AGGRESSIVE_DEFENSIVE_AXIS_MAX = 1;
	static const int AGGRESSIVE_DEFENSIVE_AXIS_MIN = -1;


	StrategySpace();
	~StrategySpace();

	void addStrategy(int race, Strategy strat);

private:
	//Graph representing the combined strategies of the Terran Race
	Graph terranStrategySpace;

	//Graph representing the combined strategies of the Protoss Race
	Graph protossStrategySpace;

	//Graph representing the combined strategies of the Zerg Race
	Graph zergStrategySpace;
};

