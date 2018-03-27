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

void StrategySpace::addStrategy(BWAPI::Race race, Strategy strat) {
	std::cout << "StrategySpace::addStrategy() - Adding Strategy " << strat.name << " of depth " << strat.maxDepth
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
			
			//Set the position of the node in strategy space as a factor of depth and intensities. The lowest leaf in the tree will be
			//scaled to be the same value as the intensity, with all other nodes linearly scaled based on percent depth

			double xpos = (strat.techTree[*it.first].depth / strat.maxDepth) * strat.air_aa_intensity;
			strat.techTree[*it.first].air_aa_pos = xpos;

			double ypos = (strat.techTree[*it.first].depth / strat.maxDepth) * strat.ground_ag_intensity;
			strat.techTree[*it.first].ground_ag_pos = ypos;

			double zpos = (strat.techTree[*it.first].depth / strat.maxDepth) * strat.aggressive_defensive_intensity;
			strat.techTree[*it.first].aggressive_defensive_pos = zpos;

			//std::cout << "Setting  " << strat.techTree[*it.first].name << " with depth " << strat.techTree[*it.first].depth 
				//<< " to Position to (" << xpos << "," << ypos << "," << zpos << ")\n";
		}
	}

	//add strat tech tree to race strategy space by copying the new tree into the root tree
	IndexMap mapIndex;
	boost::associative_property_map<IndexMap> propmapIndex(mapIndex);
	int i = 0;
	//Copy each vertex into the strategy space graph
	BGL_FORALL_VERTICES(v, strat.techTree, SCGraph)
	{
		put(propmapIndex, v, i++);
	}

	//Add the new strategy to the root tech tree
	copy_graph(strat.techTree, techTree, vertex_index_map(propmapIndex));
	//techTree = strat.techTree;
}



void StrategySpace::strengthenTree(BWAPI::Race race, BWAPI::UnitType type) {
	std::cout << "strengthenTree() - Strengthing tree for observed UnitType " << type.getID() << std::endl;

	//add strategy to the correct race graph
	SCGraph& techTree = getTechTree(race);

	//Search for matching unit types (may appear multiple times)
	for (std::pair<VertexIterator, VertexIterator> it = boost::vertices(techTree); it.first != it.second; ++it.first) {
		//std::cout << "Examining " << techTree[*it.first].name << std::endl;

		if (techTree[*it.first].node == type) {
			//std::cout << "Match found!!\n";

			//Increment the strength of the node for use in detecting strategies
			techTree[*it.first].strength++;

			//Originally I had the code below included to walk up the tree and strengthen the whole strategy as part of an observation.
			//I realize this isn't properly in the spirit of VonMises which uses points as observations 
			/*
			//Once you've found it, you need to reverse the directed to walk it to the root in boost since the graph is directed
			Rgraph rgraph(techTree);
			Rgraph::adjacency_iterator rbegin, rend;
			boost::tie(rbegin, rend) = boost::adjacent_vertices(*it.first, rgraph);

			//Walk the graph from the current node to the root and strengthen each one
			while (rbegin != rend) {
				//for (boost::tie(rbegin, rend) = boost::adjacent_vertices(*it.first, rgraph); rbegin != rend; ++rbegin) {

				//std::cout << "Strengthening " << techTree[*rbegin].name << std::endl;
				techTree[*rbegin].strength++;
				boost::tie(rbegin, rend) = boost::adjacent_vertices(*rbegin, rgraph);

				//Strengthen the edges for visualization purposes
				//std::pair<Vertex,Vertex> ed = boost::edge(*it.first, *rbegin, techTree);
				//int weight = get(boost::edge_weight, techTree, ed.first);
				//int weightToAdd = 1;
				//boost::put(boost::edge_weight, techTree, ed.first, weight + weightToAdd);
			}
			std::cout << std::endl;
			*/
		}

	}
}


void StrategySpace::identifyStrategy(BWAPI::Race race) {
	printf("identifyStrategy()\n");

	//ID strategy for the correct race 
	SCGraph& techTree = getTechTree(race);

	//keep track of the top 5 largest vertices, but ignore any vertex with current_max values (these are the common nodes)
	std::list<Vertex> vertices;

	//add all vertices in the tree to the list... there's probably a more efficient way to do this, perhaps by relying on the graph structure itself
	for (std::pair<VertexIterator, VertexIterator> it = boost::vertices(techTree); it.first != it.second; ++it.first) {
		//std::cout << "Examining " << techTree[*it.first].name << std::endl;
		vertices.push_back(techTree[*it.first]);
	}

	//Sort the vertices from largest to smallest by strength
	vertices.sort(VertexComparator());

	//once the list has been populated, identify in which regions the vertices are
	//std::cout << "Num vertices: " << vertices.size() << std::endl;
	std::list<Vertex>::iterator iter;
	int count = 0;

	//These will become the average of all the top nodes
	double proposedAirAggressiveness = 0;
	double proposedGroundAggressiveness = 0;
	double proposedOverallAggressiveness = 0;

	//For the top NUM_STRATEGY_NODES vertices or all vertices, whichever comes first
	for (iter = vertices.begin(); (iter != vertices.end() && count < NUM_STRATEGY_NODES); iter++) {

		//Find the node in the tree that matches this vertex
		Vertex strategyNode = findNode(race, (*iter));

		//We want to ignore all the "common" nodes
		if ((strategyNode.node == BWAPI::UnitTypes::Terran_Command_Center) ||
			(strategyNode.node == BWAPI::UnitTypes::Protoss_Nexus) ||
			(strategyNode.node == BWAPI::UnitTypes::Zerg_Hatchery) ||
			
			(strategyNode.node == BWAPI::UnitTypes::Terran_SCV) ||
			(strategyNode.node == BWAPI::UnitTypes::Protoss_Probe) ||
			(strategyNode.node == BWAPI::UnitTypes::Zerg_Drone)
			) {
			continue;
		}

		if (strategyNode.name == "") {
			std::cout << "No Node Match for " << (*iter).name << std::endl;
			//We should never get here
		}
		else {
			//Get the intensities of the strategy along the axes of the strategy space
			double airAggressiveness = strategyNode.air_aa_pos;
			double groundAggressiveness = strategyNode.ground_ag_pos;
			double overallAggressiveness = strategyNode.aggressive_defensive_pos;

			//Add them to the running total
			proposedAirAggressiveness += airAggressiveness;
			proposedGroundAggressiveness += groundAggressiveness;
			proposedOverallAggressiveness += overallAggressiveness;

			std::cout << strategyNode.name << "  POS A: " << airAggressiveness << "   G: " << groundAggressiveness << "   O: " << overallAggressiveness << std::endl;
		}
		count++;
	}

	//Take the average intensity of all identified strategy nodes
	proposedAirAggressiveness /= (NUM_STRATEGY_NODES * 1.0);
	proposedGroundAggressiveness /= (NUM_STRATEGY_NODES * 1.0);
	proposedOverallAggressiveness /= (NUM_STRATEGY_NODES * 1.0);

	//The propose counter-stragey should be the opposite intensity along each axis
	std::cout << "Proposed Counter Strategy:\n"
		<< "    Air / AA Aggressiveness: " << (proposedAirAggressiveness * -1) << std::endl
		<< "    Ground / AG Aggressiveness: " << (proposedGroundAggressiveness * -1) << std::endl
		<< "    Overall Defensiveness vs Aggressive: " << proposedOverallAggressiveness << std::endl;
}


Vertex StrategySpace::findNode(BWAPI::Race race, Vertex node) {
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


SCGraph& StrategySpace::getTechTree(BWAPI::Race race) {
	//std::cout<< "StrategySpace::getTechTree() - Getting tech tree for " << race.c_str() << std::endl;

	if (race == BWAPI::Races::Terran) {
		//printf("Retrieving TERRAN tech tree\n");
		return terranStrategySpace;
	}
	else if (race == BWAPI::Races::Protoss) {
		//printf("Retrieving PROTOSS tech tree\n");
		return protossStrategySpace;
	}
	else if (race == BWAPI::Races::Zerg) {
		//printf("Retrieving ZERG tech tree\n");
		return zergStrategySpace;
	}
	else {
		printf("UNKNOWN RACE. RETURNING TERRAN BY DEFAULT\n");
		return terranStrategySpace;
	}
}