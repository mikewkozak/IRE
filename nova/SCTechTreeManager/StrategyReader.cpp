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
	VertexDescriptor command_center = boost::add_vertex(vultureRush);
	vultureRush[command_center].node = BWAPI::UnitType(BWAPI::UnitTypes::Terran_Command_Center);

	//depth1
	printf("buildTree() - creating depth1\n");
	VertexDescriptor barracks = boost::add_vertex(vultureRush);
	vultureRush[barracks].node = BWAPI::UnitType(BWAPI::UnitTypes::Terran_Barracks);

	boost::add_edge(command_center, barracks, 1, vultureRush);

	//depth2
	printf("buildTree() - creating depth2\n");
	VertexDescriptor factory = boost::add_vertex(vultureRush);
	vultureRush[factory].node = BWAPI::UnitType(BWAPI::UnitTypes::Terran_Factory);
	
	boost::add_edge(barracks, factory, 1, vultureRush);

	//Name the vertices
	vultureRush[command_center].name = "Command Center";

	vultureRush[barracks].name = "Barracks";

	vultureRush[factory].name = "Factory";


	//read in strategy biases (air, ground, aggressive, etc)
	//rotate node X/Y positions around axes based on these biases
}
