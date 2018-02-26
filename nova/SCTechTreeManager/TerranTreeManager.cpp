#include "stdafx.h"
#include "TerranTreeManager.h"

using namespace BWAPI;

TerranTreeManager::TerranTreeManager()
{
	printf("TerranTreeManager()\n");
}


TerranTreeManager::~TerranTreeManager()
{
}

void TerranTreeManager::buildTree() {
	printf("buildTree()\n");

	// Populates the graph.
	//root
	printf("buildTree() - creating root\n");

	VertexDescriptor command_center = addNode(BWAPI::UnitTypes::Terran_Command_Center, "Command Center", 1);
	VertexDescriptor supply_depot = addNode(BWAPI::UnitTypes::Terran_Supply_Depot, "Supply Depot", 1);
	VertexDescriptor refinery = addNode(BWAPI::UnitTypes::Terran_Refinery, "Refinery", 1);

	//depth1
	printf("buildTree() - creating depth1\n");
	VertexDescriptor barracks = addNode(BWAPI::UnitTypes::Terran_Barracks, "Barracks", 1);
	boost::add_edge(command_center, barracks, 1, techTree);

	VertexDescriptor engineering_bay = addNode(BWAPI::UnitTypes::Terran_Engineering_Bay, "Engineering Bay", 1);
	boost::add_edge(command_center, engineering_bay, 1, techTree);

	VertexDescriptor scv = addNode(BWAPI::UnitTypes::Terran_SCV, "SCV", 1);
	boost::add_edge(command_center, scv, 1, techTree);

	//depth2
	printf("buildTree() - creating depth2\n");
	VertexDescriptor factory = addNode(BWAPI::UnitTypes::Terran_Factory, "Factory", 1);
	VertexDescriptor bunker = addNode(BWAPI::UnitTypes::Terran_Bunker, "Bunker", 1);
	VertexDescriptor academy = addNode(BWAPI::UnitTypes::Terran_Academy, "Academy", 1);
	boost::add_edge(barracks, factory, 1, techTree);
	boost::add_edge(barracks, bunker, 1, techTree);
	boost::add_edge(barracks, academy, 1, techTree);

	VertexDescriptor missile_turret = addNode(BWAPI::UnitTypes::Terran_Missile_Turret, "Missile Turret", 1);
	boost::add_edge(engineering_bay, missile_turret, 1, techTree);

	VertexDescriptor marine = addNode(BWAPI::UnitTypes::Terran_Marine, "Marine", 1);
	boost::add_edge(barracks, marine, 1, techTree);

	//depth3
	VertexDescriptor comsat_station = addNode(BWAPI::UnitTypes::Terran_Comsat_Station, "Comsat Station", 1);
	boost::add_edge(academy, comsat_station, 1, techTree);

	VertexDescriptor starport = addNode(BWAPI::UnitTypes::Terran_Starport, "Starport", 1);
	VertexDescriptor armory = addNode(BWAPI::UnitTypes::Terran_Armory, "Armory", 1);
	VertexDescriptor machine_shop = addNode(BWAPI::UnitTypes::Terran_Machine_Shop, "Machine Shop", 1);
	boost::add_edge(factory, starport, 1, techTree);
	boost::add_edge(factory, armory, 1, techTree);
	boost::add_edge(factory, machine_shop, 1, techTree);

	VertexDescriptor vulture = addNode(BWAPI::UnitTypes::Terran_Vulture, "Vulture", 1);
	boost::add_edge(factory, vulture, 1, techTree);

	VertexDescriptor firebat = addNode(BWAPI::UnitTypes::Terran_Firebat, "Firebat", 1);
	VertexDescriptor medic = addNode(BWAPI::UnitTypes::Terran_Medic, "Medic", 1);
	boost::add_edge(academy, firebat, 1, techTree);
	boost::add_edge(academy, medic, 1, techTree);

	//depth 4
	VertexDescriptor control_tower = addNode(BWAPI::UnitTypes::Terran_Control_Tower, "Control Tower", 1);
	VertexDescriptor science_facility = addNode(BWAPI::UnitTypes::Terran_Science_Facility, "Science Facility", 1);
	boost::add_edge(starport, control_tower, 1, techTree);
	boost::add_edge(starport, science_facility, 1, techTree);

	VertexDescriptor wraith = addNode(BWAPI::UnitTypes::Terran_Wraith, "Wraith", 1);
	boost::add_edge(starport, wraith, 1, techTree);

	VertexDescriptor goliath = addNode(BWAPI::UnitTypes::Terran_Goliath, "Goliath", 1);
	VertexDescriptor valkyrie = addNode(BWAPI::UnitTypes::Terran_Valkyrie, "Valkyrie", 1);
	boost::add_edge(armory, goliath, 1, techTree);
	boost::add_edge(armory, valkyrie, 1, techTree);

	VertexDescriptor siege_tank_tank = addNode(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, "Siege Tank (Tank)", 1);
	VertexDescriptor siege_tank_siege = addNode(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode, "Siege Tank (Siege)", 1);
	boost::add_edge(machine_shop, siege_tank_tank, 1, techTree);
	boost::add_edge(machine_shop, siege_tank_siege, 1, techTree);

	VertexDescriptor science_vessel = addNode(BWAPI::UnitTypes::Terran_Science_Vessel, "Science Vessel", 1);
	boost::add_edge(science_facility, science_vessel, 1, techTree);

	VertexDescriptor dropship = addNode(BWAPI::UnitTypes::Terran_Dropship, "Dropship", 1);
	boost::add_edge(control_tower, dropship, 1, techTree);
	boost::add_edge(control_tower, valkyrie, 1, techTree);

	//depth 5
	VertexDescriptor physics_lab = addNode(BWAPI::UnitTypes::Terran_Physics_Lab, "Physics Lab", 1);
	VertexDescriptor covert_ops = addNode(BWAPI::UnitTypes::Terran_Covert_Ops, "Covert Ops", 1);
	boost::add_edge(science_facility, physics_lab, 1, techTree);
	boost::add_edge(science_facility, covert_ops, 1, techTree);

	VertexDescriptor battlecruiser = addNode(BWAPI::UnitTypes::Terran_Battlecruiser, "Battlecruiser", 1);
	boost::add_edge(physics_lab, battlecruiser, 1, techTree);

	//depth 6
	VertexDescriptor nuclear_silo = addNode(BWAPI::UnitTypes::Terran_Nuclear_Silo, "Nuclear Silo", 1);
	boost::add_edge(covert_ops, nuclear_silo, 1, techTree);

	VertexDescriptor ghost = addNode(BWAPI::UnitTypes::Terran_Ghost, "Ghost", 1);
	boost::add_edge(covert_ops, ghost, 1, techTree);

	//Calculate depth for all nodes
	dijkstra_shortest_paths(techTree, command_center, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

}

VertexDescriptor TerranTreeManager::addNode(BWAPI::UnitType unitType, std::string name, int initialWeight) {
	VertexDescriptor node = boost::add_vertex(techTree);
	techTree[node].node = BWAPI::UnitType(unitType);
	techTree[node].name = name;
	techTree[node].strength = 1;
	techTree[node].location.set<StrategySpace::AIR_AA_AXIS>(0);
	techTree[node].location.set<StrategySpace::GROUND_AG_AXIS>(0);
	techTree[node].location.set<StrategySpace::AGGRESSIVE_DEFENSIVE_AXIS>(0);
	techTree[node].depth = 0;

	return node;
}

void TerranTreeManager::printTree(std::string filename) {
	printf("printTree()\n");

	// Access them when displaying edges :

	//Write out the graph to a standard format for visualization
	boost::dynamic_properties dp;
	dp.property("node_id", get(boost::vertex_index, techTree));
	dp.property("name", get(&Vertex::name, techTree));
	dp.property("strength", get(&Vertex::strength, techTree));

	std::ofstream out(filename);
	//write_graphviz(std::cout, techTree);
	//write_graphviz(out, techTree);
	write_graphviz_dp(std::cout, techTree, dp);
	write_graphviz_dp(out, techTree, dp);
	//write_graphviz_dp(std::cout, techTree, dp, std::string("name"), std::string("strength"));
	
	out.close();
}

void TerranTreeManager::strengthenTree(UnitType type) {
	std::cout << "strengthenTree()\n";// -search for match to " << type.getName() << std::endl;

	for (std::pair<VertexIterator, VertexIterator> it = boost::vertices(techTree); it.first != it.second; ++it.first) {
		std::cout << "Examining " << techTree[*it.first].name << std::endl;
		if (techTree[*it.first].node == type) {
			std::cout << "Match found!!\n";
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
				boost::tie(rbegin, rend) = boost::adjacent_vertices(*rbegin, rgraph);

				//Strengthen the edges for visualization purposes
				/*
				std::pair<Vertex,Vertex> ed = boost::edge(*it.first, *rbegin, techTree);
				int weight = get(boost::edge_weight, techTree, ed.first);
				int weightToAdd = 1;
				boost::put(boost::edge_weight, techTree, ed.first, weight + weightToAdd);
				*/
			}
			std::cout << std::endl;
		}
		
	}
	
	//do so for all strategy trees
	//TODO:
}

void TerranTreeManager::identifyStrategy() {
	printf("identifyStrategy()\n");

	//keep track of the top 5 largest vertices, but ignore any vertex with current_max values (these are the common nodes)
	std::list<Vertex> vertices;

	//for all vertices in the tree
	for (std::pair<VertexIterator, VertexIterator> it = boost::vertices(techTree); it.first != it.second; ++it.first) {
		std::cout << "Examining " << techTree[*it.first].name << std::endl;
		vertices.push_back(techTree[*it.first]);
	}

	vertices.sort(VertexComparator());

	//once the list has been populated, identify in which regions the vertices are
	std::cout << "Num vertices: " << vertices.size() << std::endl;
	std::list<Vertex>::iterator iter;
	int count = 0;
	for (iter = vertices.begin(); (iter != vertices.end() && count < 5); iter++) {
		
		double airAggressiveness = (*iter).location.get<StrategySpace::AIR_AA_AXIS>();
		double groundAggressiveness = (*iter).location.get<StrategySpace::GROUND_AG_AXIS>();
		double overallAggressiveness = (*iter).location.get<StrategySpace::AGGRESSIVE_DEFENSIVE_AXIS>();
		std::cout << (*iter).name << "  Strategy A: " << airAggressiveness << "   G: " << groundAggressiveness << "   O: " << overallAggressiveness << std::endl;
		count++;
	}
}

void TerranTreeManager::buildRequest(UnitType type, bool checkUnic) {}

void TerranTreeManager::checkRequirements(UnitType type)
{
	buildRequest(UnitTypes::Terran_Barracks, true);
	if (type == UnitTypes::Terran_Medic || type == UnitTypes::Terran_Firebat) {
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Academy, true);
	}
	else if (type == UnitTypes::Terran_Vulture) {
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
	}
	else if (type == UnitTypes::Terran_Goliath) {
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
		buildRequest(UnitTypes::Terran_Armory, true);
	}
	else if (type == UnitTypes::Terran_Siege_Tank_Tank_Mode) {
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
	}
	else if (type == UnitTypes::Terran_Wraith) {
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
		buildRequest(UnitTypes::Terran_Starport, true);
	}
	else if (type == UnitTypes::Terran_Dropship) {
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
		buildRequest(UnitTypes::Terran_Starport, true);
		//informationManager->buildRequest(UnitTypes::Terran_Control_Tower, true);
	}
	else if (type == UnitTypes::Terran_Science_Vessel) {
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
		buildRequest(UnitTypes::Terran_Starport, true);
		//informationManager->buildRequest(UnitTypes::Terran_Control_Tower, true);
		buildRequest(UnitTypes::Terran_Science_Facility, true);
	}
	else if (type == UnitTypes::Terran_Ghost) {
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Academy, true);
		buildRequest(UnitTypes::Terran_Factory, true);
		buildRequest(UnitTypes::Terran_Starport, true);
		buildRequest(UnitTypes::Terran_Science_Facility, true);
		buildRequest(UnitTypes::Terran_Covert_Ops, true);
	}
	else if (type == UnitTypes::Terran_Battlecruiser) {
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
		buildRequest(UnitTypes::Terran_Starport, true);
		//informationManager->buildRequest(UnitTypes::Terran_Control_Tower, true);
		buildRequest(UnitTypes::Terran_Science_Facility, true);
		buildRequest(UnitTypes::Terran_Physics_Lab, true);
	}


	// check dependencies
	/*
	if (upgradeType == UpgradeTypes::U_238_Shells || upgradeType == UpgradeTypes::Caduceus_Reactor) {
		buildRequest(UnitTypes::Terran_Barracks, true);
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Academy, true);
	}
	else if (upgradeType == UpgradeTypes::Terran_Infantry_Weapons || upgradeType == UpgradeTypes::Terran_Infantry_Armor) {
		buildRequest(UnitTypes::Terran_Engineering_Bay, true);
		if (level > 1) {
			buildRequest(UnitTypes::Terran_Engineering_Bay, true);
			buildRequest(UnitTypes::Terran_Barracks, true);
			buildRequest(UnitTypes::Terran_Refinery, true);
			buildRequest(UnitTypes::Terran_Factory, true);
			buildRequest(UnitTypes::Terran_Starport, true);
			buildRequest(UnitTypes::Terran_Science_Facility, true);
		}
	}
	else if (upgradeType == UpgradeTypes::Ion_Thrusters || upgradeType == UpgradeTypes::Charon_Boosters) {
		//Machine Shop
		buildRequest(UnitTypes::Terran_Barracks, true);
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
		buildRequest(UnitTypes::Terran_Machine_Shop, true);
		if (upgradeType == UpgradeTypes::Charon_Boosters)
			buildRequest(UnitTypes::Terran_Armory, true);
	}
	else if (upgradeType == UpgradeTypes::Terran_Vehicle_Weapons || upgradeType == UpgradeTypes::Terran_Vehicle_Plating ||
		upgradeType == UpgradeTypes::Terran_Ship_Weapons || upgradeType == UpgradeTypes::Terran_Ship_Plating) {
		//Armory
		buildRequest(UnitTypes::Terran_Barracks, true);
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
		buildRequest(UnitTypes::Terran_Armory, true);
		if (level > 1) {
			buildRequest(UnitTypes::Terran_Starport, true);
			buildRequest(UnitTypes::Terran_Science_Facility, true);
		}
	}
	else if (upgradeType == UpgradeTypes::Apollo_Reactor) {
		//Control Tower
		buildRequest(UnitTypes::Terran_Barracks, true);
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
		buildRequest(UnitTypes::Terran_Starport, true);
		buildRequest(UnitTypes::Terran_Control_Tower, true);
	}
	else if (upgradeType == UpgradeTypes::Titan_Reactor) {
		//Science Facility
		buildRequest(UnitTypes::Terran_Barracks, true);
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
		buildRequest(UnitTypes::Terran_Starport, true);
		buildRequest(UnitTypes::Terran_Science_Facility, true);
	}
	else if (upgradeType == UpgradeTypes::Colossus_Reactor) {
		//Physics Lab
		buildRequest(UnitTypes::Terran_Barracks, true);
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
		buildRequest(UnitTypes::Terran_Starport, true);
		buildRequest(UnitTypes::Terran_Science_Facility, true);
		buildRequest(UnitTypes::Terran_Physics_Lab, true);
	}
	else if (upgradeType == UpgradeTypes::Ocular_Implants || upgradeType == UpgradeTypes::Moebius_Reactor) {
		//Covert Ops
		buildRequest(UnitTypes::Terran_Barracks, true);
		buildRequest(UnitTypes::Terran_Refinery, true);
		buildRequest(UnitTypes::Terran_Factory, true);
		buildRequest(UnitTypes::Terran_Starport, true);
		buildRequest(UnitTypes::Terran_Science_Facility, true);
		buildRequest(UnitTypes::Terran_Covert_Ops, true);
	}*/
}