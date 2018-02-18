#include "stdafx.h"
#include "StrategyReader.h"


StrategyReader::StrategyReader()
{
}


StrategyReader::~StrategyReader()
{
}

void StrategyReader::init() {
	//TOY PROBLEM: Create Vulture Rush Strategy
	printf("buildTree()\n");
	// Populates the graph.
	//root
	printf("buildTree() - creating root\n");
	BWAPI::UnitType command_center(BWAPI::UnitTypes::Terran_Command_Center);

	//depth1
	printf("buildTree() - creating depth1\n");
	BWAPI::UnitType barracks(BWAPI::UnitTypes::Terran_Barracks);
	boost::add_edge(command_center, barracks, EdgeWeightProperty(1), vultureRush);

	//depth2
	printf("buildTree() - creating depth2\n");
	BWAPI::UnitType factory(BWAPI::UnitTypes::Terran_Factory);
	
	boost::add_edge(barracks, factory, EdgeWeightProperty(1), vultureRush);

	//read in strategy biases (air, ground, aggressive, etc)
	//rotate node X/Y positions around axes based on these biases
}
