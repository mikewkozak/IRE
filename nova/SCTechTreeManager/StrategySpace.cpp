#include "stdafx.h"
#include "StrategySpace.h"


StrategySpace::StrategySpace()
{
	//Initialize the root vertices
	terranStrategySpaceRoot = GraphUtils::addNode(terranStrategySpace, BWAPI::UnitTypes::Terran_Command_Center, "Command Center", 1);
	zergStrategySpaceRoot = GraphUtils::addNode(zergStrategySpace, BWAPI::UnitTypes::Zerg_Hatchery, "Hatchery", 1);
	protossStrategySpaceRoot = GraphUtils::addNode(protossStrategySpace, BWAPI::UnitTypes::Protoss_Nexus, "Nexus", 1);
}


StrategySpace::~StrategySpace()
{
}


VertexDescriptor& StrategySpace::getTerranStrategyRoot() {
	return terranStrategySpaceRoot;
}

VertexDescriptor& StrategySpace::getProtossStrategyRoot() {
	return protossStrategySpaceRoot;
}

VertexDescriptor& StrategySpace::getZergStrategyRoot() {
	return zergStrategySpaceRoot;
}

void StrategySpace::printStrategySpaces() {
	GraphUtils::printTree(terranStrategySpace, "Strategies/StrategySpace/TerranStrategies.dot", false);
	GraphUtils::printTree(zergStrategySpace, "Strategies/StrategySpace/ZergStrategies.dot", false);
	GraphUtils::printTree(protossStrategySpace, "Strategies/StrategySpace/ProtossStrategies.dot", false);
}

void StrategySpace::addStrategy(int race, Strategy strat) {
	printf("addStrategy()");

	std::cout << " Adding Strategy " << strat.name << " of depth " << strat.maxDepth
		<< " with intensities: "
		<< strat.air_aa_intensity << " "
		<< strat.ground_ag_intensity << " "
		<< strat.aggressive_defensive_intensity << std::endl;

	//add strategy to the correct race graph
	SCGraph& techTree = getTechTree(race);

	//rotate node X/Y positions around axes based on these biases
	for (std::pair<VertexIterator, VertexIterator> it = boost::vertices(strat.techTree); it.first != it.second; ++it.first) {
		//std::cout << "Examining " << strat.techTree[*it.first].name << std::endl;
		if (strat.techTree[*it.first].node != NULL) {
			
			double xpos = (strat.techTree[*it.first].depth / strat.maxDepth) * /*cur_child / max_children*/ strat.air_aa_intensity;
			strat.techTree[*it.first].location.set<StrategySpace::AIR_AA_AXIS>(xpos);

			double ypos = (strat.techTree[*it.first].depth / strat.maxDepth) * strat.ground_ag_intensity;
			strat.techTree[*it.first].location.set<StrategySpace::GROUND_AG_AXIS>(ypos);

			double zpos = strat.aggressive_defensive_intensity;
			strat.techTree[*it.first].location.set<StrategySpace::AGGRESSIVE_DEFENSIVE_AXIS>(zpos);

			std::cout << "Setting  " << strat.techTree[*it.first].name << " with depth " << strat.techTree[*it.first].depth 
				<< " to Position to (" << xpos << "," << ypos << "," << zpos << ")\n";
		}
	}

	//add strat tech tree to terranStrategySpace
	typedef std::map<VertexDescriptor, size_t> IndexMap;
	IndexMap mapIndex;
	boost::associative_property_map<IndexMap> propmapIndex(mapIndex);
	int i = 0;
	BGL_FORALL_VERTICES(v, strat.techTree, SCGraph)
	{
		put(propmapIndex, v, i++);
	}

	//Add the new strategy to the root tech tree
	copy_graph(strat.techTree, techTree, vertex_index_map(propmapIndex));
	//techTree = strat.techTree;
}

void StrategySpace::strengthenTree(int race, BWAPI::UnitType type) {
	std::cout << "strengthenTree()\n";// -search for match to " << type.getName() << std::endl;

	//add strategy to the correct race graph
	SCGraph& techTree = getTechTree(race);

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
}


void StrategySpace::identifyStrategy(int race) {
	printf("identifyStrategy()\n");

	SCGraph& techTree = getTechTree(race);

	//keep track of the top 5 largest vertices, but ignore any vertex with current_max values (these are the common nodes)
	std::list<Vertex> vertices;

	//for all vertices in the tree
	for (std::pair<VertexIterator, VertexIterator> it = boost::vertices(techTree); it.first != it.second; ++it.first) {
		//std::cout << "Examining " << techTree[*it.first].name << std::endl;
		vertices.push_back(techTree[*it.first]);
	}

	vertices.sort(VertexComparator());

	//once the list has been populated, identify in which regions the vertices are
	//std::cout << "Num vertices: " << vertices.size() << std::endl;
	std::list<Vertex>::iterator iter;
	int count = 0;

	double proposedAirAggressiveness = 0;
	double proposedGroundAggressiveness = 0;
	double proposedOverallAggressiveness = 0;
	for (iter = vertices.begin(); (iter != vertices.end() && count < 5); iter++) {
		Vertex strategyNode = findNode(0, (*iter));

		//We want to ignore all the "common" nodes
		if ((strategyNode.node == BWAPI::UnitTypes::Terran_Command_Center) ||
			(strategyNode.node == BWAPI::UnitTypes::Protoss_Nexus) ||
			(strategyNode.node == BWAPI::UnitTypes::Zerg_Hatchery) ) {
			count++;
			continue;
		}

		if (strategyNode.name == "") {
			std::cout << "No Node Match for " << (*iter).name << std::endl;
		}
		else {
			double airAggressiveness = strategyNode.location.get<StrategySpace::AIR_AA_AXIS>();
			double groundAggressiveness = strategyNode.location.get<StrategySpace::GROUND_AG_AXIS>();
			double overallAggressiveness = strategyNode.location.get<StrategySpace::AGGRESSIVE_DEFENSIVE_AXIS>();

			proposedAirAggressiveness += airAggressiveness;
			proposedGroundAggressiveness += groundAggressiveness;
			proposedOverallAggressiveness += overallAggressiveness;

			std::cout << strategyNode.name << "  POS A: " << airAggressiveness << "   G: " << groundAggressiveness << "   O: " << overallAggressiveness << std::endl;
		}
		count++;
	}

	//Take the average intensity of all node points
	proposedAirAggressiveness /= 5.0;
	proposedGroundAggressiveness /= 5.0;
	proposedOverallAggressiveness /= 5.0;

	std::cout << "Proposed Counter Strategy:\n"
		<< "    Air / AA Aggressiveness: " << (proposedAirAggressiveness * -1) << std::endl
		<< "    Ground / AG Aggressiveness: " << (proposedGroundAggressiveness * -1) << std::endl
		<< "    Overall Defensiveness vs Aggressive: " << proposedOverallAggressiveness << std::endl;
}


Vertex StrategySpace::findNode(int race, Vertex node) {
	//printf("findNode()\n");

	SCGraph& techTree = getTechTree(race);

	Vertex match;
	for (std::pair<VertexIterator, VertexIterator> it = boost::vertices(techTree); it.first != it.second; ++it.first) {
		if (techTree[*it.first].node == node.node) {
			match = techTree[*it.first];
		}
	}

	return match;
}

SCGraph& StrategySpace::getTechTree(int race) {
	//printf("getTechTree()\n");

	if (race == 0) {
		return terranStrategySpace;
	}
	else if (race == 1) {
		return protossStrategySpace;
	}
	else if (race == 2) {
		return zergStrategySpace;
	}
	else {
		printf("UNKNOWN RACE. RETURNING TERRAN BY DEFAULT\n");
		return terranStrategySpace;
	}
}