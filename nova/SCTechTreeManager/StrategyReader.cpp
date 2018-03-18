#include  <libs/graph/src/read_graphviz_new.cpp>

#include "stdafx.h"
#include "StrategyReader.h"

using namespace boost::filesystem;

//Define path consts
const std::string StrategyReader::TERRAN_STRATEGY_PATH = "Strategies/Templates/Terran/";
const std::string StrategyReader::PROTOSS_STRATEGY_PATH = "Strategies/Templates/Protoss/";
const std::string StrategyReader::ZERG_STRATEGY_PATH = "Strategies/Templates/Zerg/";

StrategyReader::StrategyReader() {}


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

	//Add the strategy to the list of strategies
	strategies.push_back(buildBio());
	strategies.push_back(buildRaxFe());
	strategies.push_back(buildTwoFacto());
	strategies.push_back(buildVultures());
	strategies.push_back(buildAir());
	strategies.push_back(buildDrop());

	return strategies;
} 

std::vector<Strategy> StrategyReader::buildZergStrategies() {
	std::vector<Strategy> strategies;

	strategies.push_back(buildSpeedlings());
	strategies.push_back(buildFastMutas());
	strategies.push_back(buildMutas());
	strategies.push_back(buildLurkers());
	strategies.push_back(buildHydras());
	return strategies;

}


std::vector<Strategy> StrategyReader::buildProtossStrategies() {
	std::vector<Strategy> strategies;

	strategies.push_back(buildTwoGates());
	strategies.push_back(buildFastDT());
	strategies.push_back(buildTemplar());
	strategies.push_back(buildSpeedzeal());
	strategies.push_back(buildCorsair());
	strategies.push_back(buildNony());
	strategies.push_back(buildReaverDrop());

	return strategies;

}



/*******************************************************************************************************************/
/********************************************TERRAN STRATEGIES******************************************************/
/*******************************************************************************************************************/
Strategy StrategyReader::buildBio() {
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
	GraphUtils::printTree(bioStrategyGraph, "Strategies/Templates/Terran/Bio.dot", false);

	return bioStrategy;
}


Strategy StrategyReader::buildRaxFe() {
	// Populates the graph.
	//root
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

	GraphUtils::printTree(raxFeStrategyGraph, "Strategies/Templates/Terran/Rax_Fe.dot", false);

	return raxFeStrategy;
}


Strategy StrategyReader::buildTwoFacto() {
	// Populates the graph.
	//root
	SCGraph techTree;

	// Populates the graph.
	//root
	VertexDescriptor command_center = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Command_Center, "Command Center", 1);

	//depth1
	VertexDescriptor barracks = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Barracks, "Barracks", 1);
	boost::add_edge(command_center, barracks, 1, techTree);

	//depth2
	VertexDescriptor factory = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Factory, "Factory", 1);
	boost::add_edge(barracks, factory, 1, techTree);


	//depth3
	VertexDescriptor machine_shop = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Machine_Shop, "Machine Shop", 1);
	boost::add_edge(factory, machine_shop, 1, techTree);


	//depth 4
	VertexDescriptor siege_tank_tank = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, "Siege Tank (Tank)", 1);
	VertexDescriptor siege_tank_siege = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode, "Siege Tank (Siege)", 1);
	boost::add_edge(machine_shop, siege_tank_tank, 1, techTree);
	boost::add_edge(machine_shop, siege_tank_siege, 1, techTree);



	dijkstra_shortest_paths(techTree, command_center, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));


	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Two Facto";
	strat.aggressive_defensive_intensity = 0.5;
	strat.air_aa_intensity = 0;
	strat.ground_ag_intensity = 1;
	strat.maxDepth = 4;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Terran/TwoFacto.dot", false);

	return strat;
}


Strategy StrategyReader::buildVultures() {
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
	dijkstra_shortest_paths(vultureRush, cc4, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, vultureRush)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = vultureRush;
	strat.name = "Vulture Rush";
	strat.aggressive_defensive_intensity = 1;
	strat.air_aa_intensity = 0;
	strat.ground_ag_intensity = 1;
	strat.maxDepth = 2;


	GraphUtils::printTree(vultureRush, "Strategies/Templates/Terran/Vultures.dot", false);

	return strat;
}

Strategy StrategyReader::buildAir() {
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

	GraphUtils::printTree(wraithStrategyGraph, "Strategies/Templates/Terran/Wraith.dot", false);

	return wraithStrategy;
}


Strategy StrategyReader::buildDrop() {
	// Populates the graph.
	//root
	SCGraph techTree;

	// Populates the graph.
	//root
	VertexDescriptor command_center = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Command_Center, "Command Center", 1);

	//depth1
	VertexDescriptor barracks = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Barracks, "Barracks", 1);
	boost::add_edge(command_center, barracks, 1, techTree);

	VertexDescriptor engineering_bay = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Engineering_Bay, "Engineering Bay", 1);
	boost::add_edge(command_center, engineering_bay, 1, techTree);

	//depth2
	VertexDescriptor factory = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Factory, "Factory", 1);
	VertexDescriptor bunker = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Bunker, "Bunker", 1);
	VertexDescriptor academy = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Academy, "Academy", 1);
	boost::add_edge(barracks, factory, 1, techTree);
	boost::add_edge(barracks, bunker, 1, techTree);
	boost::add_edge(barracks, academy, 1, techTree);

	VertexDescriptor marine = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Marine, "Marine", 1);
	boost::add_edge(barracks, marine, 1, techTree);

	//depth3
	VertexDescriptor starport = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Starport, "Starport", 1);
	VertexDescriptor armory = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Armory, "Armory", 1);
	VertexDescriptor machine_shop = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Machine_Shop, "Machine Shop", 1);
	boost::add_edge(factory, starport, 1, techTree);
	boost::add_edge(factory, armory, 1, techTree);
	boost::add_edge(factory, machine_shop, 1, techTree);

	VertexDescriptor vulture = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Vulture, "Vulture", 1);
	boost::add_edge(factory, vulture, 1, techTree);

	VertexDescriptor firebat = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Firebat, "Firebat", 1);
	VertexDescriptor medic = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Medic, "Medic", 1);
	boost::add_edge(academy, firebat, 1, techTree);
	boost::add_edge(academy, medic, 1, techTree);

	//depth 4
	VertexDescriptor control_tower = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Control_Tower, "Control Tower", 1);
	boost::add_edge(starport, control_tower, 1, techTree);

	VertexDescriptor wraith = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Wraith, "Wraith", 1);
	boost::add_edge(starport, wraith, 1, techTree);

	VertexDescriptor goliath = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Goliath, "Goliath", 1);
	VertexDescriptor valkyrie = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Valkyrie, "Valkyrie", 1);
	boost::add_edge(armory, goliath, 1, techTree);
	boost::add_edge(armory, valkyrie, 1, techTree);

	VertexDescriptor dropship = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Dropship, "Dropship", 1);
	boost::add_edge(control_tower, dropship, 1, techTree);
	boost::add_edge(control_tower, valkyrie, 1, techTree);

	dijkstra_shortest_paths(techTree, command_center, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));


	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Drop";
	strat.aggressive_defensive_intensity = -0.25;
	strat.air_aa_intensity = 0.25;
	strat.ground_ag_intensity = 0.25;
	strat.maxDepth = 4;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Terran/Drop.dot", false);

	return strat;
}



/*******************************************************************************************************************/
/********************************************PROTOSS STRATEGIES*****************************************************/
/*******************************************************************************************************************/
Strategy StrategyReader::buildTwoGates() {
	// Populates the graph.
	SCGraph techTree;

	//root
	VertexDescriptor nexus = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Nexus, "Nexus", 1);


	//depth1
	VertexDescriptor gateway = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Gateway, "Gateway", 1);
	boost::add_edge(nexus, gateway, 1, techTree);

	//depth2
	VertexDescriptor zealot = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Zealot, "Zealot", 1);
	boost::add_edge(gateway, zealot, 1, techTree);



	//Calculate depth for all nodes
	dijkstra_shortest_paths(techTree, nexus, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Two_Gates";
	strat.aggressive_defensive_intensity = 1;
	strat.air_aa_intensity = 0;
	strat.ground_ag_intensity = 1;
	strat.maxDepth = 2;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Protoss/Two_Gates.dot", false);

	return strat;
}

Strategy StrategyReader::buildFastDT() {
	// Populates the graph.
	SCGraph techTree;

	// Populates the graph.
	//root
	VertexDescriptor nexus = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Nexus, "Nexus", 1);

	//depth1
	VertexDescriptor gateway = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Gateway, "Gateway", 1);
	VertexDescriptor forge = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Forge, "Forge", 1);
	boost::add_edge(nexus, gateway, 1, techTree);
	boost::add_edge(nexus, forge, 1, techTree);

	//depth2
	VertexDescriptor cybernetics_core = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Cybernetics_Core, "Cybernetics Core", 1);
	boost::add_edge(gateway, cybernetics_core, 1, techTree);

	VertexDescriptor photon_cannon = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Photon_Cannon, "Photon Cannon", 1);
	boost::add_edge(forge, photon_cannon, 1, techTree);

	//depth3
	VertexDescriptor citadel_of_adun = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Citadel_of_Adun, "Citadel Of Adun", 1);
	boost::add_edge(cybernetics_core, citadel_of_adun, 1, techTree);

	//depth 4
	VertexDescriptor templar_archives = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Templar_Archives, "Templar Archives", 1);
	boost::add_edge(citadel_of_adun, templar_archives, 1, techTree);


	//depth 5
	VertexDescriptor dark_archon = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Dark_Archon, "Dark Archon", 1);
	VertexDescriptor dark_templar = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Dark_Templar, "Dark Templar", 1);
	boost::add_edge(templar_archives, dark_archon, 1, techTree);
	boost::add_edge(templar_archives, dark_templar, 1, techTree);

	//Calculate depth for all nodes
	dijkstra_shortest_paths(techTree, nexus, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Fast_DT";
	strat.aggressive_defensive_intensity = 0;
	strat.air_aa_intensity = -0.25;
	strat.ground_ag_intensity = 0;
	strat.maxDepth = 5;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Protoss/Fast_Dt.dot", false);

	return strat;
}

Strategy StrategyReader::buildTemplar() {
	// Populates the graph.
	SCGraph techTree;

	// Populates the graph.
	//root
	VertexDescriptor nexus = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Nexus, "Nexus", 1);

	//depth1
	VertexDescriptor gateway = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Gateway, "Gateway", 1);
	VertexDescriptor forge = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Forge, "Forge", 1);
	boost::add_edge(nexus, gateway, 1, techTree);
	boost::add_edge(nexus, forge, 1, techTree);

	//depth2
	VertexDescriptor cybernetics_core = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Cybernetics_Core, "Cybernetics Core", 1);
	boost::add_edge(gateway, cybernetics_core, 1, techTree);

	VertexDescriptor photon_cannon = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Photon_Cannon, "Photon Cannon", 1);
	boost::add_edge(forge, photon_cannon, 1, techTree);

	//depth3
	VertexDescriptor citadel_of_adun = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Citadel_of_Adun, "Citadel Of Adun", 1);
	boost::add_edge(cybernetics_core, citadel_of_adun, 1, techTree);

	//depth 4
	VertexDescriptor templar_archives = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Templar_Archives, "Templar Archives", 1);
	boost::add_edge(citadel_of_adun, templar_archives, 1, techTree);


	//depth 5
	VertexDescriptor archon = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Archon, "Archon", 1);
	VertexDescriptor high_templar = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_High_Templar, "High Templar", 1);
	boost::add_edge(templar_archives, archon, 1, techTree);
	boost::add_edge(templar_archives, high_templar, 1, techTree);

	//Calculate depth for all nodes
	dijkstra_shortest_paths(techTree, nexus, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Templar";
	strat.aggressive_defensive_intensity = 0.5;
	strat.air_aa_intensity = -0.5;
	strat.ground_ag_intensity = 0;
	strat.maxDepth = 5;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Protoss/Templar.dot", false);

	return strat;
}
Strategy StrategyReader::buildSpeedzeal() {
	// Populates the graph.
	SCGraph techTree;

	// Populates the graph.
	//root
	VertexDescriptor nexus = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Nexus, "Nexus", 1);


	//depth1
	VertexDescriptor gateway = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Gateway, "Gateway", 1);
	VertexDescriptor forge = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Forge, "Forge", 1);
	boost::add_edge(nexus, gateway, 1, techTree);
	boost::add_edge(nexus, forge, 1, techTree);

	VertexDescriptor probe = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Probe, "Probe", 1);
	boost::add_edge(nexus, probe, 1, techTree);

	//depth2
	VertexDescriptor cybernetics_core = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Cybernetics_Core, "Cybernetics Core", 1);
	VertexDescriptor shield_battery = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Shield_Battery, "Shield Battery", 1);
	boost::add_edge(gateway, cybernetics_core, 1, techTree);
	boost::add_edge(gateway, shield_battery, 1, techTree);

	VertexDescriptor photon_cannon = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Photon_Cannon, "Photon Cannon", 1);
	boost::add_edge(forge, photon_cannon, 1, techTree);


	VertexDescriptor zealot = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Zealot, "Zealot", 1);
	boost::add_edge(gateway, zealot, 1, techTree);

	//depth3
	VertexDescriptor robotics_facility = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Robotics_Facility, "Robotics Facility", 1);
	VertexDescriptor stargate = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Stargate, "Stargate", 1);
	VertexDescriptor citadel_of_adun = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Citadel_of_Adun, "Citadel Of Adun", 1);
	boost::add_edge(cybernetics_core, robotics_facility, 1, techTree);
	boost::add_edge(cybernetics_core, stargate, 1, techTree);
	boost::add_edge(cybernetics_core, citadel_of_adun, 1, techTree);

	VertexDescriptor dragoon = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Dragoon, "Dragoon", 1);
	boost::add_edge(cybernetics_core, dragoon, 1, techTree);


	//Calculate depth for all nodes
	dijkstra_shortest_paths(techTree, nexus, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Speedzeal";
	strat.aggressive_defensive_intensity = 0.75;
	strat.air_aa_intensity = 0;
	strat.ground_ag_intensity = 1;
	strat.maxDepth = 3;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Protoss/Speedzeal.dot", false);

	return strat;
}


Strategy StrategyReader::buildCorsair() {
	// Populates the graph.
	SCGraph techTree;

	// Populates the graph.
	//root
	VertexDescriptor nexus = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Nexus, "Nexus", 1);


	//depth1
	VertexDescriptor gateway = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Gateway, "Gateway", 1);
	boost::add_edge(nexus, gateway, 1, techTree);

	//depth2
	VertexDescriptor cybernetics_core = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Cybernetics_Core, "Cybernetics Core", 1);
	boost::add_edge(gateway, cybernetics_core, 1, techTree);

	//depth3
	VertexDescriptor stargate = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Stargate, "Stargate", 1);
	boost::add_edge(cybernetics_core, stargate, 1, techTree);

	//depth 4
	VertexDescriptor fleet_beacon = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Fleet_Beacon, "Fleet Beacon", 1);
	boost::add_edge(stargate, fleet_beacon, 1, techTree);

	VertexDescriptor corsair = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Corsair, "Corsair", 1);
	boost::add_edge(stargate, corsair, 1, techTree);

	//Calculate depth for all nodes
	dijkstra_shortest_paths(techTree, nexus, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Corsair";
	strat.aggressive_defensive_intensity = -0.75;
	strat.air_aa_intensity = -1;
	strat.ground_ag_intensity = 0;
	strat.maxDepth = 4;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Protoss/Corsair.dot", false);

	return strat;
}


Strategy StrategyReader::buildNony() {
	// Populates the graph.
	SCGraph techTree;

	// Populates the graph.
	//root
	VertexDescriptor nexus = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Nexus, "Nexus", 1);


	//depth1
	VertexDescriptor gateway = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Gateway, "Gateway", 1);
	boost::add_edge(nexus, gateway, 1, techTree);


	//depth2
	VertexDescriptor cybernetics_core = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Cybernetics_Core, "Cybernetics Core", 1);
	boost::add_edge(gateway, cybernetics_core, 1, techTree);

	//depth3
	VertexDescriptor dragoon = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Dragoon, "Dragoon", 1);
	boost::add_edge(cybernetics_core, dragoon, 1, techTree);



	//Calculate depth for all nodes
	dijkstra_shortest_paths(techTree, nexus, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Nony";
	strat.aggressive_defensive_intensity = 0.5;
	strat.air_aa_intensity = 0;
	strat.ground_ag_intensity = 0;
	strat.maxDepth = 3;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Protoss/Nony.dot", false);

	return strat;
}


Strategy StrategyReader::buildReaverDrop() {
	// Populates the graph.
	SCGraph techTree;

	// Populates the graph.
	VertexDescriptor nexus = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Nexus, "Nexus", 1);

	//depth1
	VertexDescriptor gateway = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Gateway, "Gateway", 1);
	boost::add_edge(nexus, gateway, 1, techTree);

	//depth2
	VertexDescriptor cybernetics_core = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Cybernetics_Core, "Cybernetics Core", 1);
	boost::add_edge(gateway, cybernetics_core, 1, techTree);


	//depth3
	VertexDescriptor robotics_facility = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Robotics_Facility, "Robotics Facility", 1);
	boost::add_edge(cybernetics_core, robotics_facility, 1, techTree);


	//depth 4
	VertexDescriptor robotics_support_bay = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Robotics_Support_Bay, "Robotics Support Bay", 1);
	boost::add_edge(robotics_facility, robotics_support_bay, 1, techTree);

	VertexDescriptor reaver = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Reaver, "Reaver", 1);
	boost::add_edge(robotics_facility, reaver, 1, techTree);

	//depth 5
	VertexDescriptor shuttle = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Shuttle, "Shuttle", 1);
	boost::add_edge(robotics_support_bay, shuttle, 1, techTree);


	//Calculate depth for all nodes
	dijkstra_shortest_paths(techTree, nexus, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Reaver_Drop";
	strat.aggressive_defensive_intensity = -0.25;
	strat.air_aa_intensity = 0.25;
	strat.ground_ag_intensity = 0;
	strat.maxDepth = 5;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Protoss/Reaver_Drop.dot", false);

	return strat;
}



/*******************************************************************************************************************/
/**********************************************ZERG STRATEGIES******************************************************/
/*******************************************************************************************************************/
Strategy StrategyReader::buildSpeedlings() {
	// Populates the graph.
	SCGraph techTree;

	// Populates the graph.
	//root
	VertexDescriptor hatchery = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Hatchery, "Hatchery", 1);

	//depth1
	VertexDescriptor spawning_pool = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Spawning_Pool, "Spawning Pool", 1);
	boost::add_edge(hatchery, spawning_pool, 1, techTree);


	//depth2
	VertexDescriptor zergling = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Zergling, "Zergling", 1);
	boost::add_edge(spawning_pool, zergling, 1, techTree);


	//Calculate depth for all nodes
	dijkstra_shortest_paths(techTree, hatchery, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Speedlings";
	strat.aggressive_defensive_intensity = 1;
	strat.air_aa_intensity = 0;
	strat.ground_ag_intensity = 1;
	strat.maxDepth = 2;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Zerg/Speedlings.dot", false);

	return strat;
}


Strategy StrategyReader::buildFastMutas() {
	// Populates the graph.
	SCGraph techTree;

	// Populates the graph.
	//root
	VertexDescriptor hatchery = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Hatchery, "Hatchery", 1);


	//depth1
	printf("buildTree() - creating depth1\n");
	VertexDescriptor spawning_pool = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Spawning_Pool, "Spawning Pool", 1);
	boost::add_edge(hatchery, spawning_pool, 1, techTree);

	//depth2
	VertexDescriptor lair = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Lair, "Lair", 1);
	boost::add_edge(spawning_pool, lair, 1, techTree);

	//depth3
	VertexDescriptor spire = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Spire, "Spire", 1);
	boost::add_edge(lair, spire, 1, techTree);

	//depth 4
	VertexDescriptor mutalisk = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Mutalisk, "Mutalisk", 1);
	boost::add_edge(spire, mutalisk, 1, techTree);


	//Calculate depth for all nodes
	dijkstra_shortest_paths(techTree, hatchery, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Fast_Mutas";
	strat.aggressive_defensive_intensity = 0.25;
	strat.air_aa_intensity = 1;
	strat.ground_ag_intensity = -1;
	strat.maxDepth = 4;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Zerg/Fast_Mutas.dot", false);

	return strat;
}


Strategy StrategyReader::buildMutas() {
	// Populates the graph.
	SCGraph techTree;

	//Calculate depth for all nodes
	//dijkstra_shortest_paths(techTree, hatchery, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Mutas";
	strat.aggressive_defensive_intensity = -0.25;
	strat.air_aa_intensity = 0.75;
	strat.ground_ag_intensity = -0.75;
	strat.maxDepth = 4;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Zerg/Mutas.dot", false);

	return strat;
}


Strategy StrategyReader::buildLurkers() {
	// Populates the graph.
	SCGraph techTree;

	//Calculate depth for all nodes
	//dijkstra_shortest_paths(techTree, hatchery, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Mutas";
	strat.aggressive_defensive_intensity = -0.25;
	strat.air_aa_intensity = 0;
	strat.ground_ag_intensity = 0;
	strat.maxDepth = 4;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Zerg/Mutas.dot", false);

	return strat;
}


Strategy StrategyReader::buildHydras() {
	// Populates the graph.
	SCGraph techTree;

	//Calculate depth for all nodes
	//dijkstra_shortest_paths(techTree, hatchery, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

	//read in strategy biases (air, ground, aggressive, etc)
	Strategy strat;
	strat.techTree = techTree;
	strat.name = "Mutas";
	strat.aggressive_defensive_intensity = -0.75;
	strat.air_aa_intensity = -0.25;
	strat.ground_ag_intensity = 1;
	strat.maxDepth = 4;

	//Write the strategy to file
	GraphUtils::printTree(techTree, "Strategies/Templates/Zerg/Mutas.dot", false);

	return strat;
}