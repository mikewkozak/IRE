#include "stdafx.h"
#include "TerranTreeManager.h"

using namespace BWAPI;

TerranTreeManager::TerranTreeManager() {
	printf("TerranTreeManager()\n");

	buildTree();
	
	//TOY PROBLEM
	//std::vector<Strategy> strats = reader.buildTerranStrategies(strategies.getTerranStrategyRoot());
	std::vector<Strategy> strats = reader.buildTerranStrategies();

	for (unsigned int i = 0; i < strats.size(); i++) {
		std::cout << "TerranTreeManager() - Adding Strategy: " << strats[i].name << std::endl;
		strategies.addStrategy(BWAPI::Races::Terran, strats[i]);
	}

	GraphUtils::printTree(strategies.getTechTree(BWAPI::Races::Terran), "Strategies/StrategySpace/TerranStrategies.dot", false);
}


TerranTreeManager::~TerranTreeManager() {}

void TerranTreeManager::buildTree() {
	printf("buildTree()\n");

	// Populates the graph.
	//root
	printf("buildTree() - creating root\n");

	VertexDescriptor command_center = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Command_Center, "Command Center", 1);
	VertexDescriptor supply_depot = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Supply_Depot, "Supply Depot", 1);
	VertexDescriptor refinery = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Refinery, "Refinery", 1);

	//depth1
	printf("buildTree() - creating depth1\n");
	VertexDescriptor barracks = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Barracks, "Barracks", 1);
	boost::add_edge(command_center, barracks, 1, techTree);

	VertexDescriptor engineering_bay = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Engineering_Bay, "Engineering Bay", 1);
	boost::add_edge(command_center, engineering_bay, 1, techTree);

	VertexDescriptor scv = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_SCV, "SCV", 1);
	boost::add_edge(command_center, scv, 1, techTree);

	//depth2
	printf("buildTree() - creating depth2\n");
	VertexDescriptor factory = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Factory, "Factory", 1);
	VertexDescriptor bunker = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Bunker, "Bunker", 1);
	VertexDescriptor academy = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Academy, "Academy", 1);
	boost::add_edge(barracks, factory, 1, techTree);
	boost::add_edge(barracks, bunker, 1, techTree);
	boost::add_edge(barracks, academy, 1, techTree);

	VertexDescriptor missile_turret = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Missile_Turret, "Missile Turret", 1);
	boost::add_edge(engineering_bay, missile_turret, 1, techTree);

	VertexDescriptor marine = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Marine, "Marine", 1);
	boost::add_edge(barracks, marine, 1, techTree);

	//depth3
	printf("buildTree() - creating depth3\n");
	VertexDescriptor comsat_station = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Comsat_Station, "Comsat Station", 1);
	boost::add_edge(academy, comsat_station, 1, techTree);

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
	printf("buildTree() - creating depth4\n");
	VertexDescriptor control_tower = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Control_Tower, "Control Tower", 1);
	VertexDescriptor science_facility = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Science_Facility, "Science Facility", 1);
	boost::add_edge(starport, control_tower, 1, techTree);
	boost::add_edge(starport, science_facility, 1, techTree);

	VertexDescriptor wraith = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Wraith, "Wraith", 1);
	boost::add_edge(starport, wraith, 1, techTree);

	VertexDescriptor goliath = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Goliath, "Goliath", 1);
	VertexDescriptor valkyrie = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Valkyrie, "Valkyrie", 1);
	boost::add_edge(armory, goliath, 1, techTree);
	boost::add_edge(armory, valkyrie, 1, techTree);

	VertexDescriptor siege_tank_tank = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, "Siege Tank (Tank)", 1);
	VertexDescriptor siege_tank_siege = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode, "Siege Tank (Siege)", 1);
	boost::add_edge(machine_shop, siege_tank_tank, 1, techTree);
	boost::add_edge(machine_shop, siege_tank_siege, 1, techTree);

	VertexDescriptor science_vessel = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Science_Vessel, "Science Vessel", 1);
	boost::add_edge(science_facility, science_vessel, 1, techTree);

	VertexDescriptor dropship = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Dropship, "Dropship", 1);
	boost::add_edge(control_tower, dropship, 1, techTree);
	boost::add_edge(control_tower, valkyrie, 1, techTree);

	//depth 5
	printf("buildTree() - creating depth5\n");
	VertexDescriptor physics_lab = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Physics_Lab, "Physics Lab", 1);
	VertexDescriptor covert_ops = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Covert_Ops, "Covert Ops", 1);
	boost::add_edge(science_facility, physics_lab, 1, techTree);
	boost::add_edge(science_facility, covert_ops, 1, techTree);

	VertexDescriptor battlecruiser = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Battlecruiser, "Battlecruiser", 1);
	boost::add_edge(physics_lab, battlecruiser, 1, techTree);

	//depth 6
	printf("buildTree() - creating depth6\n");
	VertexDescriptor nuclear_silo = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Nuclear_Silo, "Nuclear Silo", 1);
	boost::add_edge(covert_ops, nuclear_silo, 1, techTree);

	VertexDescriptor ghost = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Ghost, "Ghost", 1);
	boost::add_edge(covert_ops, ghost, 1, techTree);



	//Calculate depth for all nodes
	dijkstra_shortest_paths(techTree, command_center, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

}

SCGraph& TerranTreeManager::getTree() {
	return techTree;
}


void TerranTreeManager::strengthenTree(UnitType type) {
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
			while(rbegin != rend) {
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
	strategies.strengthenTree(BWAPI::Races::Terran, type);
}

void TerranTreeManager::identifyStrategy() {
	strategies.identifyStrategy(BWAPI::Races::Terran);
}
