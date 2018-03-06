#include <list>

//Graph structure and strategy axes
#include "StrategySpace.h"
#include "StrategyReader.h"
#include "GraphUtils.h"



#pragma once
class TerranTreeManager
{
public:
	TerranTreeManager();
	~TerranTreeManager();

	void buildTree();

	/*
	Given a unit, building, or upgrade, traverse up the tree to the root and strengthen every edge along the way
	*/
	void strengthenTree(BWAPI::UnitType type);

	/*
	Given a strategy subtree, strengthen every edge in the tech tree that matches the strategy tree
	*/
	//void strengthenStrategy();

	SCGraph& getTree();

	void identifyStrategy();

private:
	// The Graph object
	SCGraph techTree;


	//TOY PROBLEM
	StrategySpace strategies;
	StrategyReader reader;
};

