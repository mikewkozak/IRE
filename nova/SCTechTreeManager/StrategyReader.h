
//Graph structure and strategy axes
#include "StrategySpace.h"

#pragma once
class StrategyReader
{
public:
	StrategyReader();
	~StrategyReader();

	//Read in strategy files and store in strategy map
	void init();

private:
	//Strategy Map

	//TOY PROBLEM
	Graph vultureRush;

	/*
	Support function for adding nodes to the graph
	*/
	VertexDescriptor addNode(Graph graph, BWAPI::UnitType unitType, std::string name, int initialWeight);
};

