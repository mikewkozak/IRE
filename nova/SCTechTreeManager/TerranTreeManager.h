//Graph structure and strategy axes
#include "StrategySpace.h"



#pragma once
class TerranTreeManager
{
public:
	TerranTreeManager();
	~TerranTreeManager();

	void buildTree();

	/*
	Prints to the DOT file format for visualization in graph files
	*/
	void printTree(std::string filename);

	/*
	Given a unit, building, or upgrade, traverse up the tree to the root and strengthen every edge along the way
	*/
	void strengthenTree(BWAPI::UnitType type);

	/*
	Browses the strategy space identifying the largest nodes and edges to determine what the likely strategy is
	*/
	void identifyStrategy();

	/*
	Given a strategy subtree, strengthen every edge in the tech tree that matches the strategy tree
	*/
	//void strengthenStrategy();

	void checkRequirements(BWAPI::UnitType type);
	void buildRequest(BWAPI::UnitType type, bool checkUnic);


private:
	// The Graph object
	Graph techTree;

	/*
	Support function for adding nodes to the graph
	*/
	VertexDescriptor addNode(BWAPI::UnitType unitType, std::string name, int initialWeight);

};

