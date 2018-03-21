#include "stdafx.h"
#include "ProtossTreeManager.h"

using namespace BWAPI;


ProtossTreeManager::ProtossTreeManager() {
	printf("ProtossTreeManager()\n");

	buildTree();

	//TOY PROBLEM
	//std::vector<Strategy> strats = reader.buildTerranStrategies(strategies.getTerranStrategyRoot());
	std::vector<Strategy> strats = reader.buildProtossStrategies();

	for (unsigned int i = 0; i < strats.size(); i++) {
		std::cout << "ProtossTreeManager() - Adding Strategy: " << strats[i].name << std::endl;
		strategies.addStrategy(BWAPI::Races::Protoss, strats[i]);
	}

	GraphUtils::printTree(strategies.getTechTree(BWAPI::Races::Protoss), "Strategies/StrategySpace/ProtossStrategies.dot", false);
}


ProtossTreeManager::~ProtossTreeManager(){}


void ProtossTreeManager::buildTree() {
	printf("buildTree() - Building Protoss Tech Tree\n");

	// Populates the graph.
	//root
	VertexDescriptor nexus = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Nexus, "Nexus", 1);
	VertexDescriptor pylon = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Pylon, "Pylon", 1);
	VertexDescriptor assimilator = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Assimilator, "Assimilator", 1);


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

	//depth 4
	VertexDescriptor robotics_support_bay = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Robotics_Support_Bay, "Robotics Support Bay", 1);
	VertexDescriptor observatory = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Observatory, "Observatory", 1);
	boost::add_edge(robotics_facility, robotics_support_bay, 1, techTree);
	boost::add_edge(robotics_facility, observatory, 1, techTree);

	VertexDescriptor fleet_beacon = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Fleet_Beacon, "Fleet Beacon", 1);
	boost::add_edge(stargate, fleet_beacon, 1, techTree);

	VertexDescriptor templar_archives = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Templar_Archives, "Templar Archives", 1);
	boost::add_edge(citadel_of_adun, templar_archives, 1, techTree);


	VertexDescriptor reaver = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Reaver, "Reaver", 1);
	boost::add_edge(robotics_facility, reaver, 1, techTree);

	VertexDescriptor corsair = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Corsair, "Corsair", 1);
	VertexDescriptor scout = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Scout, "Scout", 1);
	boost::add_edge(stargate, corsair, 1, techTree);
	boost::add_edge(stargate, scout, 1, techTree);

	//depth 5
	VertexDescriptor arbiter_tribunal = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Arbiter_Tribunal, "Arbiter Tribunal", 1);
	boost::add_edge(templar_archives, arbiter_tribunal, 1, techTree);


	VertexDescriptor shuttle = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Shuttle, "Shuttle", 1);
	boost::add_edge(robotics_support_bay, shuttle, 1, techTree);

	VertexDescriptor observer = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Observer, "Observer", 1);
	boost::add_edge(observatory, observer, 1, techTree);

	VertexDescriptor carrier = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Carrier, "Carrier", 1);
	boost::add_edge(fleet_beacon, carrier, 1, techTree);

	VertexDescriptor archon = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Archon, "Archon", 1);
	VertexDescriptor dark_archon = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Dark_Archon, "Dark Archon", 1);
	VertexDescriptor high_templar = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_High_Templar, "High Templar", 1);
	VertexDescriptor dark_templar = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Dark_Templar, "Dark Templar", 1);
	boost::add_edge(templar_archives, archon, 1, techTree);
	boost::add_edge(templar_archives, dark_archon, 1, techTree);
	boost::add_edge(templar_archives, high_templar, 1, techTree);
	boost::add_edge(templar_archives, dark_templar, 1, techTree);

	//depth 6
	VertexDescriptor arbiter = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Protoss_Arbiter, "Arbiter", 1);
	boost::add_edge(arbiter_tribunal, arbiter, 1, techTree);


	//Calculate depth for all nodes
	dijkstra_shortest_paths(techTree, nexus, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

}

SCGraph& ProtossTreeManager::getTree() {
	return techTree;
}


void ProtossTreeManager::strengthenTree(UnitType type) {
	std::cout << "strengthenTree()\n";// -search for match to " << type.getName() << std::endl;

	for (std::pair<VertexIterator, VertexIterator> it = boost::vertices(techTree); it.first != it.second; ++it.first) {
		//std::cout << "Examining " << techTree[*it.first].name << std::endl;
		if (techTree[*it.first].node == type) {
			//std::cout << "Match found!!\n";
			techTree[*it.first].strength++;
			//Once you've found it, you need to reverse the directed to walk it to the root
			Rgraph rgraph(techTree);
			Rgraph::adjacency_iterator rbegin, rend;
			boost::tie(rbegin, rend) = boost::adjacent_vertices(*it.first, rgraph);
			while (rbegin != rend) {
				//for (boost::tie(rbegin, rend) = boost::adjacent_vertices(*it.first, rgraph); rbegin != rend; ++rbegin) {

				std::cout << "Strengthening " << techTree[*rbegin].name << std::endl;
				//traverse up to root and strengthen the nodes along the way
				techTree[*rbegin].strength++;

				//get the next node up in the tree
				boost::tie(rbegin, rend) = boost::adjacent_vertices(*rbegin, rgraph);
			}
			std::cout << std::endl;
		}

	}

	//do so for all strategy trees
	strategies.strengthenTree(BWAPI::Races::Protoss, type);
}

void ProtossTreeManager::identifyStrategy() {
	strategies.identifyStrategy(BWAPI::Races::Protoss);
}