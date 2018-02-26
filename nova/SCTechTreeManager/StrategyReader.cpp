#include "stdafx.h"
#include "StrategyReader.h"


StrategyReader::StrategyReader()
{
}


StrategyReader::~StrategyReader()
{
}

void StrategyReader::init() {
	//TOY PROBLEM: Create Vulture Rush Strategy
	printf("buildTree()\n");
	// Populates the graph.
	//root
	printf("buildTree() - creating root\n");
	VertexDescriptor command_center = addNode(vultureRush, BWAPI::UnitTypes::Terran_Command_Center, "Command Center", 1);

	//depth1
	printf("buildTree() - creating depth1\n");
	VertexDescriptor barracks = addNode(vultureRush, BWAPI::UnitTypes::Terran_Barracks, "Barracks", 1);
	boost::add_edge(command_center, barracks, 1, vultureRush);

	//depth2
	printf("buildTree() - creating depth2\n");
	VertexDescriptor factory = addNode(vultureRush, BWAPI::UnitTypes::Terran_Factory, "Factory", 1);
	boost::add_edge(barracks, factory, 1, vultureRush);

	VertexDescriptor vulture = addNode(vultureRush, BWAPI::UnitTypes::Terran_Vulture, "Vulture", 1);
	boost::add_edge(factory, vulture, 1, vultureRush);

	//Calculate depth for all nodes
	dijkstra_shortest_paths(vultureRush, command_center, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, vultureRush)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = vultureRush;
	strat.name = "Vulture Rush";
	strat.aggressive_defensive_intensity = 1;
	strat.air_aa_intensity = 0;
	strat.ground_ag_intensity = 1;
}


VertexDescriptor StrategyReader::addNode(Graph graph, BWAPI::UnitType unitType, std::string name, int initialWeight) {
	VertexDescriptor node = boost::add_vertex(graph);
	graph[node].node = BWAPI::UnitType(unitType);
	graph[node].name = name;
	graph[node].strength = 1;
	graph[node].depth = 0;

	return node;
}
