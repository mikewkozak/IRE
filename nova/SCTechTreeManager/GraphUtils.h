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
#include <boost/graph/copy.hpp>

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
> SCGraph;

//Reverse graph. Necessary to traverse to root
typedef boost::reverse_graph<SCGraph> Rgraph;

//Graph descriptors
typedef SCGraph::edge_descriptor EdgeDescriptor;
typedef typename boost::graph_traits<SCGraph>::vertex_descriptor VertexDescriptor;

//Graph iterators
typedef boost::graph_traits<SCGraph>::edge_iterator EdgeIterator;
typedef boost::graph_traits<SCGraph>::vertex_iterator VertexIterator;

struct Strategy {
	SCGraph techTree;
	std::string name;
	double air_aa_intensity;//x -1 to 1
	double ground_ag_intensity;//y -1 to 1
	double aggressive_defensive_intensity;//z -1 to 1
	double maxDepth;
};

#pragma once
class GraphUtils
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

	GraphUtils();
	~GraphUtils();


	/*
	Support function for adding nodes to the graph
	*/
	static VertexDescriptor addNode(SCGraph& graph, BWAPI::UnitType unitType, std::string name, int initialWeight) {
		VertexDescriptor node = boost::add_vertex(graph);
		graph[node].node = BWAPI::UnitType(unitType);
		graph[node].name = name;
		graph[node].strength = 1;
		graph[node].depth = 0;

		graph[node].location.set<AIR_AA_AXIS>(0);
		graph[node].location.set<GROUND_AG_AXIS>(0);
		graph[node].location.set<AGGRESSIVE_DEFENSIVE_AXIS>(0);

		return node;
	}

	static void printTree(SCGraph tree, std::string filename, bool printToConsole) {
		printf("printTree()\n");

		// Access them when displaying edges :

		//Write out the graph to a standard format for visualization
		boost::dynamic_properties dp;
		dp.property("node_id", get(&Vertex::name, tree));
		//dp.property("name", get(&Vertex::name, tree));
		dp.property("strength", get(&Vertex::strength, tree));
		dp.property("penwidth", get(&Vertex::strength, tree));
		dp.property("depth", get(&Vertex::depth, tree));
		//dp.property("pos", get(&Vertex::location, tree));

		if (printToConsole) {
			write_graphviz_dp(std::cout, tree, dp);
			//write_graphviz(std::cout, tree);
			//write_graphviz_dp(std::cout, tree, dp, std::string("name"), std::string("strength"));
		}

		std::ofstream out(filename);
		//write_graphviz(out, tree);
		write_graphviz_dp(out, tree, dp);
		out.close();
	}

};

