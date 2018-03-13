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
		strategies.addStrategy(0, strats[i]);
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


	//depth1
	printf("buildTree() - creating depth1\n");
	VertexDescriptor barracks = GraphUtils::addNode(techTree, BWAPI::UnitTypes::Terran_Barracks, "Barracks", 1);
	boost::add_edge(hatchery, barracks, 1, techTree);

	

	//depth2
	printf("buildTree() - creating depth2\n");
	

	//depth3
	printf("buildTree() - creating depth3\n");
	

	//depth 4
	printf("buildTree() - creating depth4\n");
	

	//depth 5
	printf("buildTree() - creating depth5\n");
	

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
	strategies.strengthenTree(0, type);
}

void ZergTreeManager::identifyStrategy() {
	strategies.identifyStrategy(0);
}