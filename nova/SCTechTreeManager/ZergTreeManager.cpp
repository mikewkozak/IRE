#include "stdafx.h"
#include "ZergTreeManager.h"


using namespace BWAPI;

ZergTreeManager::ZergTreeManager() {
	printf("ZergTreeManager()\n");

	buildTree();

	//TOY PROBLEM
	//std::vector<Strategy> strats = reader.buildTerranStrategies(strategies.getTerranStrategyRoot());
	std::vector<Strategy> strats = reader.buildZergStrategies();

	for (unsigned int i = 0; i < strats.size(); i++) {
		std::cout << "ZergTreeManager() - Adding Strategy: " << strats[i].name << std::endl;
		strategies.addStrategy(BWAPI::Races::Zerg, strats[i]);
	}

	strategies.printStrategySpaces();
}


ZergTreeManager::~ZergTreeManager(){}



void ZergTreeManager::buildTree() {
	printf("buildTree()\n");

	// Populates the graph.
	//root
	printf("buildTree() - creating root\n");

	VertexDescriptor hatchery = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Hatchery, "Hatchery", 1);
	VertexDescriptor extractor = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Extractor, "Extractor", 1);
	VertexDescriptor creep_colony = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Creep_Colony, "Creep Colony", 1);


	//depth1
	printf("buildTree() - creating depth1\n");
	VertexDescriptor spawning_pool = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Spawning_Pool, "Spawning Pool", 1);
	VertexDescriptor evolution_chamber = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Evolution_Chamber, "Evolution Chamber", 1);
	boost::add_edge(hatchery, spawning_pool, 1, techTree);
	boost::add_edge(hatchery, evolution_chamber, 1, techTree);

	VertexDescriptor larva = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Larva, "Larva", 1);
	VertexDescriptor drone = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Drone, "Drone", 1);
	VertexDescriptor overlord = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Overlord, "Overlord", 1);
	boost::add_edge(hatchery, larva, 1, techTree);
	boost::add_edge(hatchery, overlord, 1, techTree);
	boost::add_edge(hatchery, drone, 1, techTree);

	

	//depth2
	printf("buildTree() - creating depth2\n");
	VertexDescriptor spore_colony = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Spore_Colony, "Spore Colony", 1);
	boost::add_edge(evolution_chamber, spore_colony, 1, techTree);

	VertexDescriptor lair = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Lair, "Lair", 1);
	VertexDescriptor hydralisk_den = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Hydralisk_Den, "Hydralisk Den", 1);
	VertexDescriptor sunken_colony = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Sunken_Colony, "Sunken Colony", 1);
	boost::add_edge(spawning_pool, lair, 1, techTree);
	boost::add_edge(spawning_pool, hydralisk_den, 1, techTree);
	boost::add_edge(spawning_pool, sunken_colony, 1, techTree);

	VertexDescriptor zergling = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Zergling, "Zergling", 1);
	boost::add_edge(spawning_pool, zergling, 1, techTree);
	

	//depth3
	printf("buildTree() - creating depth3\n");
	VertexDescriptor queens_nest = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Queens_Nest, "Queen's Nest", 1);
	VertexDescriptor spire = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Spire, "Spire", 1);
	boost::add_edge(lair, queens_nest, 1, techTree);
	boost::add_edge(lair, spire, 1, techTree);

	VertexDescriptor hydralisk = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Hydralisk, "Hydralisk", 1);
	boost::add_edge(hydralisk_den, hydralisk, 1, techTree);

	//depth 4
	printf("buildTree() - creating depth4\n");
	VertexDescriptor hive = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Hive, "Hive", 1);
	VertexDescriptor infested_command_center = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Infested_Command_Center, "Infested Command Center", 1);
	boost::add_edge(queens_nest, hive, 1, techTree);
	boost::add_edge(queens_nest, infested_command_center, 1, techTree);


	VertexDescriptor queen = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Queen, "Queen", 1);
	VertexDescriptor broodling = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Broodling, "Broodling", 1);
	boost::add_edge(queens_nest, queen, 1, techTree);
	boost::add_edge(queens_nest, broodling, 1, techTree);

	VertexDescriptor mutalisk = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Mutalisk, "Mutalisk", 1);
	boost::add_edge(spire, mutalisk, 1, techTree);


	//depth 5
	printf("buildTree() - creating depth5\n");
	VertexDescriptor ultralisk_cavern = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Ultralisk_Cavern, "Ultralisk Cavern", 1);
	VertexDescriptor greater_spire = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Greater_Spire, "Greater Spire", 1);
	VertexDescriptor nydus_canal = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Nydus_Canal, "Nydus Canal", 1);
	VertexDescriptor defiler_mound = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Defiler_Mound, "Defiler Mound", 1);
	boost::add_edge(hive, ultralisk_cavern, 1, techTree);
	boost::add_edge(hive, greater_spire, 1, techTree);
	boost::add_edge(hive, nydus_canal, 1, techTree);
	boost::add_edge(hive, defiler_mound, 1, techTree);


	VertexDescriptor infested_terran = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Infested_Terran, "Infested Terran", 1);
	boost::add_edge(infested_command_center, infested_terran, 1, techTree);


	//depth 6
	VertexDescriptor ultralisk = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Ultralisk, "Ultralisk", 1);
	boost::add_edge(ultralisk_cavern, ultralisk, 1, techTree);


	VertexDescriptor guardian = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Guardian, "Guardian", 1);
	VertexDescriptor devourer = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Devourer, "Devourer", 1);
	boost::add_edge(greater_spire, guardian, 1, techTree);
	boost::add_edge(greater_spire, devourer, 1, techTree);
	
	VertexDescriptor defiler = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Zerg_Defiler, "Defiler", 1);
	boost::add_edge(defiler_mound, defiler, 1, techTree);


	//Calculate depth for all nodes	
	dijkstra_shortest_paths(techTree, hatchery, boost::weight_map(boost::make_constant_property<EdgeDescriptor>(1)).distance_map(get(&Vertex::depth, techTree)));

}

SCGraph& ZergTreeManager::getTree() {
	return techTree;
}


void ZergTreeManager::strengthenTree(UnitType type) {
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
	strategies.strengthenTree(BWAPI::Races::Zerg, type);
}

void ZergTreeManager::identifyStrategy() {
	strategies.identifyStrategy(BWAPI::Races::Zerg);
}