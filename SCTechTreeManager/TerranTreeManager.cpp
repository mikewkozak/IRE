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
	BWAPI::UnitType command_center(BWAPI::UnitTypes::Terran_Command_Center);
	BWAPI::UnitType supply_depot(BWAPI::UnitTypes::Terran_Supply_Depot);

	//depth1
	printf("buildTree() - creating depth1\n");
	BWAPI::UnitType barracks(BWAPI::UnitTypes::Terran_Barracks);
	boost::add_edge(command_center, barracks, EdgeWeightProperty(1), techTree);

	BWAPI::UnitType engineering_bay(BWAPI::UnitTypes::Terran_Engineering_Bay);
	boost::add_edge(command_center, engineering_bay, EdgeWeightProperty(1), techTree);

	//depth2
	printf("buildTree() - creating depth2\n");
	BWAPI::UnitType factory(BWAPI::UnitTypes::Terran_Factory);
	BWAPI::UnitType bunker(BWAPI::UnitTypes::Terran_Bunker);
	BWAPI::UnitType academy(BWAPI::UnitTypes::Terran_Academy);
	boost::add_edge(barracks, factory, EdgeWeightProperty(1), techTree);
	boost::add_edge(barracks, factory, EdgeWeightProperty(1), techTree);
	boost::add_edge(barracks, academy, EdgeWeightProperty(1), techTree);


	BWAPI::UnitType missile_turret(BWAPI::UnitTypes::Terran_Missile_Turret);
	boost::add_edge(engineering_bay, missile_turret, EdgeWeightProperty(1), techTree);


	//Write out the graph to a standard format for visualization
	boost::dynamic_properties dp;
	//dp.property("id", get(boost::vertex_name, techTree));
	//dp.property("weight", get(boost::edge_weight, techTree));

	std::ofstream out("TerranTechTree.dot");
	write_graphviz(std::cout, techTree);
	//write_graphviz_dp(std::cout, techTree, dp, std::string("id"));
	//write_graphviz_dp(out, techTree, dp, std::string("id"));


	// The property map associated with the weights.
	/*
	boost::property_map < Graph,
		boost::edge_weight_t >::type EdgeWeightMap = get(boost::edge_weight, techTree);

	// Loops over all edges and add 10 to their weight.
	boost::graph_traits< Graph >::edge_iterator e_it, e_end;
	for (std::tie(e_it, e_end) = boost::edges(techTree); e_it != e_end; ++e_it)
	{
		EdgeWeightMap[*e_it] += 10;
	}

	// Prints the weighted edgelist.
	for (std::tie(e_it, e_end) = boost::edges(techTree); e_it != e_end; ++e_it)
	{
		std::cout << boost::source(*e_it, techTree) << " "
			<< boost::target(*e_it, techTree) << " "
			<< EdgeWeightMap[*e_it] << std::endl;
	}
	*/
}

void TerranTreeManager::strengthenTree(UnitType type) {}

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