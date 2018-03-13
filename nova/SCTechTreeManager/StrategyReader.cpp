#include  <libs/graph/src/read_graphviz_new.cpp>

#include "stdafx.h"
#include "StrategyReader.h"

using namespace boost::filesystem;

//Define path consts
const std::string StrategyReader::TERRAN_STRATEGY_PATH = "Strategies/Templates/Terran/";

StrategyReader::StrategyReader()
{
	//getTerranStrategies();
	buildTerranStrategies();
}


StrategyReader::~StrategyReader()
{
}

void StrategyReader::getTerranStrategies() {
	// list all files in current directory.
	//You could put any file path in here, e.g. "/home/me/mwah" to list that directory
	path p(TERRAN_STRATEGY_PATH);

	directory_iterator end_itr;

	// cycle through the directory
	for (directory_iterator itr(p); itr != end_itr; ++itr)
	{
		// If it's not a directory, list it. If you want to list directories too, just remove this check.
		if (is_regular_file(itr->path())) {
			// assign current file name to current_file and echo it out to the console.
			std::string current_file = itr->path().string();
			std::cout << "Reading Strategy File: " << current_file << std::endl;

			//Define the properties to read in to the nodes of the graph
			SCGraph strategy;
			boost::dynamic_properties dp;// (boost::ignore_other_properties);
			dp.property("node_id", get(&Vertex::name, strategy));
			dp.property("strength", get(&Vertex::strength, strategy));
			dp.property("depth", get(&Vertex::depth, strategy));

			
			std::ifstream dot(current_file);
			read_graphviz(dot, strategy, dp);
		}
	}
}

std::vector<Strategy> StrategyReader::buildTerranStrategies() {
	std::vector<Strategy> strategies;

	// Populates the graph.
	//root
	SCGraph bioStrategyGraph;

	VertexDescriptor command_center = GraphUtils::addNode(bioStrategyGraph, BWAPI::UnitTypes::Terran_Command_Center, "Command Center", 1);
	VertexDescriptor supply_depot = GraphUtils::addNode(bioStrategyGraph, BWAPI::UnitTypes::Terran_Supply_Depot, "Supply Depot", 1);
	VertexDescriptor refinery = GraphUtils::addNode(bioStrategyGraph, BWAPI::UnitTypes::Terran_Refinery, "Refinery", 1);

	//depth1
	VertexDescriptor barracks = GraphUtils::addNode(bioStrategyGraph, BWAPI::UnitTypes::Terran_Barracks, "Barracks", 1);
	boost::add_edge(command_center, barracks, 1, bioStrategyGraph);

	VertexDescriptor engineering_bay = GraphUtils::addNode(bioStrategyGraph, BWAPI::UnitTypes::Terran_Engineering_Bay, "Engineering Bay", 1);
	boost::add_edge(command_center, engineering_bay, 1, bioStrategyGraph);

	//depth2
	VertexDescriptor academy = GraphUtils::addNode(bioStrategyGraph, BWAPI::UnitTypes::Terran_Academy, "Academy", 1);
	boost::add_edge(barracks, academy, 1, bioStrategyGraph);

	VertexDescriptor marine = GraphUtils::addNode(bioStrategyGraph, BWAPI::UnitTypes::Terran_Marine, "Marine", 1);
	boost::add_edge(barracks, marine, 1, bioStrategyGraph);

	//depth3
	VertexDescriptor medic = GraphUtils::addNode(bioStrategyGraph, BWAPI::UnitTypes::Terran_Medic, "Medic", 1);
	boost::add_edge(academy, medic, 1, bioStrategyGraph);

	dijkstra_shortest_paths(bioStrategyGraph, command_center, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, bioStrategyGraph)));


	//read in strategy biases (air, ground, aggressive, etc)
	Strategy bioStrategy;
	bioStrategy.techTree = bioStrategyGraph;
	bioStrategy.name = "Bio";
	bioStrategy.aggressive_defensive_intensity = 1;
	bioStrategy.air_aa_intensity = -0.25;
	bioStrategy.ground_ag_intensity = 0.75;
	bioStrategy.maxDepth = 3;

	//Write the strategy to file
	GraphUtils::printTree(bioStrategyGraph, "Strategies/Templates/Terran/bio.dot", false);

	//Add the strategy to the list of strategies
	strategies.push_back(bioStrategy);



	// Populates the graph.
	//root
	printf("buildTree() - creating root\n");
	SCGraph raxFeStrategyGraph;
	VertexDescriptor cc2 = GraphUtils::addNode(raxFeStrategyGraph, BWAPI::UnitTypes::Terran_Command_Center, "Command Center", 1);
	VertexDescriptor sd2 = GraphUtils::addNode(raxFeStrategyGraph, BWAPI::UnitTypes::Terran_Supply_Depot, "Supply Depot", 1);
	VertexDescriptor r2 = GraphUtils::addNode(raxFeStrategyGraph, BWAPI::UnitTypes::Terran_Refinery, "Refinery", 1);

	//depth1
	VertexDescriptor scv2 = GraphUtils::addNode(raxFeStrategyGraph, BWAPI::UnitTypes::Terran_SCV, "SCV", 1);
	boost::add_edge(cc2, scv2, 1, raxFeStrategyGraph);

	dijkstra_shortest_paths(raxFeStrategyGraph, cc2, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, raxFeStrategyGraph)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy raxFeStrategy;
	raxFeStrategy.techTree = raxFeStrategyGraph;
	raxFeStrategy.name = "Rax_fe";
	raxFeStrategy.aggressive_defensive_intensity = -1;
	raxFeStrategy.air_aa_intensity = 0;
	raxFeStrategy.ground_ag_intensity = 0;
	raxFeStrategy.maxDepth = 1;

	GraphUtils::printTree(raxFeStrategyGraph, "Strategies/Templates/Terran/rax_fe.dot", false);	
	strategies.push_back(raxFeStrategy);



	SCGraph wraithStrategyGraph;

	VertexDescriptor cc3 = GraphUtils::addNode(wraithStrategyGraph, BWAPI::UnitTypes::Terran_Command_Center, "Command Center", 1);
	
	//depth1
	VertexDescriptor b3 = GraphUtils::addNode(wraithStrategyGraph, BWAPI::UnitTypes::Terran_Barracks, "Barracks", 1);
	boost::add_edge(cc3, b3, 1, wraithStrategyGraph);

	//depth2
	VertexDescriptor f3 = GraphUtils::addNode(wraithStrategyGraph, BWAPI::UnitTypes::Terran_Factory, "Factory", 1);
	boost::add_edge(b3, f3, 1, wraithStrategyGraph);

	//depth3
	VertexDescriptor s3 = GraphUtils::addNode(wraithStrategyGraph, BWAPI::UnitTypes::Terran_Starport, "Starport", 1);
	boost::add_edge(f3, s3, 1, wraithStrategyGraph);
	
	//depth 4
	VertexDescriptor ct3 = GraphUtils::addNode(wraithStrategyGraph, BWAPI::UnitTypes::Terran_Control_Tower, "Control Tower", 1);
	boost::add_edge(s3, ct3, 1, wraithStrategyGraph);
	
	VertexDescriptor w3 = GraphUtils::addNode(wraithStrategyGraph, BWAPI::UnitTypes::Terran_Wraith, "Wraith", 1);
	boost::add_edge(s3, w3, 1, wraithStrategyGraph);

	//Calculate depth for all nodes
	dijkstra_shortest_paths(wraithStrategyGraph, cc3, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, wraithStrategyGraph)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy wraithStrategy;
	wraithStrategy.techTree = wraithStrategyGraph;
	wraithStrategy.name = "Air";
	wraithStrategy.aggressive_defensive_intensity = 0.25;
	wraithStrategy.air_aa_intensity = 0.75;
	wraithStrategy.ground_ag_intensity = -0.5;
	wraithStrategy.maxDepth = 4;
	strategies.push_back(wraithStrategy);

	GraphUtils::printTree(wraithStrategyGraph, "Strategies/Templates/Terran/wraith.dot", false);


	//TOY PROBLEM: Create Vulture Rush Strategy
	SCGraph vultureRush;
	// Populates the graph.
	//root
	VertexDescriptor cc4 = GraphUtils::addNode(vultureRush, BWAPI::UnitTypes::Terran_Command_Center, "Command Center", 1);

	//depth1
	VertexDescriptor b4 = GraphUtils::addNode(vultureRush, BWAPI::UnitTypes::Terran_Barracks, "Barracks", 1);
	boost::add_edge(cc4, b4, 1, vultureRush);

	//depth2
	VertexDescriptor f4 = GraphUtils::addNode(vultureRush, BWAPI::UnitTypes::Terran_Factory, "Factory", 1);
	boost::add_edge(b4, f4, 1, vultureRush);

	VertexDescriptor v4 = GraphUtils::addNode(vultureRush, BWAPI::UnitTypes::Terran_Vulture, "Vulture", 1);
	boost::add_edge(f4, v4, 1, vultureRush);

	//Calculate depth for all nodes
	dijkstra_shortest_paths(vultureRush, command_center, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, vultureRush)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = vultureRush;
	strat.name = "Vulture Rush";
	strat.aggressive_defensive_intensity = 1;
	strat.air_aa_intensity = 0;
	strat.ground_ag_intensity = 1;
	strat.maxDepth = 2;


	GraphUtils::printTree(vultureRush, "Strategies/Templates/Terran/vultureRush.dot", false);
	strategies.push_back(strat);

	return strategies;
} 