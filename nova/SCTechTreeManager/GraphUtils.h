/*
GraphUtils.h
Collection of structs, typedefs, and static utility functions for representing the Starcraft Tech Trees and the Strategy Graphs
*/


#include <iostream>
#include <utility>
#include <algorithm>

//Includes for Tree representation of tech tree
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/copy.hpp>

//Includes for BWAPI types
#include <BWAPI.h>

//Include for use of Point
#include <boost/geometry.hpp>
namespace bg = boost::geometry;



/*
Container for a node in the graph. These nodes can be a unit or building type
*/
struct Vertex {
	BWAPI::UnitType node;//type of SC object
	std::string name;//name for display purposes
	int strength;//The number of times it has been "seen" by IRE
	//The location in "Strategy Space" representing the relative intensity of this object in terms of strategy
	double air_aa_pos;
	double ground_ag_pos;
	double aggressive_defensive_pos;
	int depth;//The depth from root. Required for generating the location and difficult to obtain on the fly because of boost graphs not being trees
};

/*
Container for the comparator function used for comparing 2 nodes in the graph. This is used primarily to sort nodes in the graph by strength
*/
struct VertexComparator {
	bool operator() (const Vertex& lhs, const Vertex& rhs) const {
		return lhs.strength > rhs.strength;
	}
};

// Graph representing Tech Trees and Strategy Space
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

//Map of indices to the graph for use in BOOST_FORALL loops
typedef std::map<VertexDescriptor, size_t> IndexMap;

/*
Container representing a single AI strategy
*/
struct Strategy {
	SCGraph techTree;//The subgraph of the full tech tree that defines this strategy
	std::string name;//the name for display purposes
	double air_aa_intensity;//x axis in strategy space -1 to 1
	double ground_ag_intensity;//y in strategy space -1 to 1
	double aggressive_defensive_intensity;//z in strategy space -1 to 1
	double maxDepth;//The depth of the lowest leaf in the tree. Used for laying out the strategy in strategy space
};

#pragma once
/*
THe utility class for interacting with the strategy and tech tree graphs. Contains constants and static functions to be used elsewhere for convenience
*/
class GraphUtils
{
public:
	//x axis in strategy space constants
	static const int AIR_AA_AXIS = 0;
	static const int AIR_AA_AXIS_MAX = 1;
	static const int AIR_AA_AXIS_MIN = -1;

	//y axis in strategy space constants
	static const int GROUND_AG_AXIS = 1;
	static const int GROUND_AG_AXIS_MAX = 1;
	static const int GROUND_AG_AXIS_MIN = -1;

	//z axis in strategy space constants
	static const int AGGRESSIVE_DEFENSIVE_AXIS = 2;
	static const int AGGRESSIVE_DEFENSIVE_AXIS_MAX = 1;
	static const int AGGRESSIVE_DEFENSIVE_AXIS_MIN = -1;

	//Constructor (not used)
	GraphUtils();
	//Destructor (not used)
	~GraphUtils();


	/*
	Support function for adding nodes to a graph and setting default values for the Vertex
	*/
	static VertexDescriptor addNode(SCGraph& graph, BWAPI::UnitType unitType, std::string name, int initialWeight) {

		//Add the vertex to the graph and get the handle to it
		VertexDescriptor node = boost::add_vertex(graph);

		//Set the initial values
		graph[node].node = BWAPI::UnitType(unitType);
		graph[node].name = name;
		graph[node].strength = 1;
		graph[node].depth = 0;

		//Set the initial location
		graph[node].air_aa_pos = 0;
		graph[node].ground_ag_pos = 0;
		graph[node].aggressive_defensive_pos = 0;

		//return the vertex created
		return node;
	}

	/*
	Prints the tree to a DOT file and optionally to the command line
	*/
	static void printTree(SCGraph tree, std::string filename, bool printToConsole) {
		printf("printTree()\n");

		//Write out the graph to a standard format for visualization through the use of dynamic properties
		boost::dynamic_properties dp;
		dp.property("node_id", get(boost::vertex_index, tree));
		dp.property("label", get(&Vertex::name, tree));
		dp.property("strength", get(&Vertex::strength, tree));
		dp.property("penwidth", get(&Vertex::strength, tree));
		dp.property("depth", get(&Vertex::depth, tree));
		dp.property("x", get(&Vertex::air_aa_pos, tree));
		dp.property("y", get(&Vertex::ground_ag_pos, tree));
		dp.property("z", get(&Vertex::aggressive_defensive_pos, tree));

		//Also print to command line if the option is set to true
		if (printToConsole) {
			write_graphviz_dp(std::cout, tree, dp);
		}

		//Open the stream, write the file, and then close the stream
		std::ofstream out(filename);
		write_graphviz_dp(out, tree, dp);
		out.close();
	}

};

