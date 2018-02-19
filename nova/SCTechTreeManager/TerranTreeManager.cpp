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

	// The Graph object
	Graph techTree;

	// Populates the graph.
	//root
	printf("buildTree() - creating root\n");
	VertexDescriptor command_center = boost::add_vertex(techTree);
	techTree[command_center].node = BWAPI::UnitType(BWAPI::UnitTypes::Terran_Command_Center);
	BWAPI::UnitType supply_depot(BWAPI::UnitTypes::Terran_Supply_Depot);

	//depth1
	printf("buildTree() - creating depth1\n");
	VertexDescriptor barracks = boost::add_vertex(techTree);
	techTree[barracks].node = BWAPI::UnitType(BWAPI::UnitTypes::Terran_Barracks);
	boost::add_edge(command_center, barracks, 1, techTree);

	VertexDescriptor engineering_bay = boost::add_vertex(techTree);
	techTree[engineering_bay].node = BWAPI::UnitType(BWAPI::UnitTypes::Terran_Engineering_Bay);
	boost::add_edge(command_center, engineering_bay, 1, techTree);

	//depth2
	printf("buildTree() - creating depth2\n");
	VertexDescriptor factory = boost::add_vertex(techTree);
	techTree[factory].node = BWAPI::UnitType(BWAPI::UnitTypes::Terran_Factory);
	VertexDescriptor bunker = boost::add_vertex(techTree);
	techTree[bunker].node = BWAPI::UnitType(BWAPI::UnitTypes::Terran_Bunker);
	VertexDescriptor academy = boost::add_vertex(techTree);
	techTree[academy].node = BWAPI::UnitType(BWAPI::UnitTypes::Terran_Academy);
	boost::add_edge(barracks, factory, 1, techTree);
	boost::add_edge(barracks, bunker, 1, techTree);
	boost::add_edge(barracks, academy, 1, techTree);


	VertexDescriptor missile_turret = boost::add_vertex(techTree);
	techTree[missile_turret].node = BWAPI::UnitType(BWAPI::UnitTypes::Terran_Missile_Turret);
	boost::add_edge(engineering_bay, missile_turret, 1, techTree);

	//Name the vertices
	techTree[command_center].name = "Command Center";

	techTree[barracks].name = "Barracks";
	techTree[engineering_bay].name = "Engineering Bay";

	techTree[factory].name = "Factory";
	techTree[bunker].name = "Bunker";
	techTree[academy].name = "Academy";
	techTree[missile_turret].name = "Missile Turret";
}

void TerranTreeManager::printTree() {
	printf("printTree()\n");

	// Access them when displaying edges :
	printf("Vertices:\n");
	for (std::pair<VertexIterator, VertexIterator> it = boost::vertices(techTree); it.first != it.second; ++it.first) {
		std::cout << techTree[*it.first].name << std::endl;
	}

	//Write out the graph to a standard format for visualization
	boost::dynamic_properties dp;
	dp.property("id", get(&Vertex::name, techTree));
	dp.property("weight", get(&Vertex::strength, techTree));

	//std::ofstream out("TerranTechTree.dot");
	//write_graphviz(std::cout, techTree);
	write_graphviz_dp(std::cout, techTree, dp);
	//write_graphviz_dp(out, techTree, dp, std::string("id"));

	//out.close();
}

void TerranTreeManager::strengthenTree(UnitType type) {
	printf("strengthenTree()\n");
	/*
	boost::graph_traits<Graph> ::vertex_iterator vertCurrItr, vertPrevItr, vertEndItr;

	boost::property_map<Graph, boost::vertex_property_tag> ::type propMap1;
	boost::vertex_property_tag kind;

	propMap1 = boost::get(kind, techTree);

	//find the matching node in the tech tree
	for (
		boost::tie(vertCurrItr, vertEndItr) = vertices(techTree);
		vertCurrItr != vertEndItr;
		vertCurrItr++
		)
	{
		if (propMap1[*vertCurrItr] == type) {
		std::cout << "found it!!\n";
		//Once you've found it, you need to reverse the directed to walk it to the root
		Rgraph rgraph(techTree);
		Rgraph::adjacency_iterator rbegin, rend;
		for (boost::tie(rbegin, rend) = boost::adjacent_vertices(*vertCurrItr, rgraph); rbegin != rend; ++rbegin)
		{
			//traverse up to root and strengthen the nodes and edges along the way
			std::pair<Edge, bool> ed = boost::edge(*vertCurrItr, *vertEndItr, techTree);
			int weight = get(EdgeWeightProperty(), techTree, ed.first);
			int weightToAdd = 1;
			boost::put(EdgeWeightProperty(), techTree, ed.first, weight + weightToAdd);

			std::cout << *rbegin << std::endl;
		}
		std::cout << std::endl;
		}
	}

	//do so for all strategy trees
	//TODO:
	*/
}

void TerranTreeManager::identifyStrategy() {
	printf("identifyStrategy()\n");
	//for all vertices in the tree
	//evaluate the height associated with the vertex
	//keep track of the top 5 largest vertices, but ignore any vertex with current_max values (these are the common nodes)
	//once the list has been populated, identify in which regions the vertices are
	//for each axis, produce an average scalar value among all top vertices
	//these values are now the intensity along each strategic axis for the enemy strategy
	//potential solution: have NOVA produce units in the exact opposite of these intensities
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