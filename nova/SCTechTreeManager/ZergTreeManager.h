/*
ZergTreeManager.h
Class representing the full tech tree for the Zerg Race.
*/
#include <list>

//Graph structure and strategy axes
#include "StrategySpace.h"
#include "StrategyReader.h"
#include "GraphUtils.h"

#pragma once
/*
Class representing the full tech tree for the Zerg Race.
*/
class ZergTreeManager
{
public:
	ZergTreeManager();
	~ZergTreeManager();


	/*
	Given a unit, building, or upgrade, traverse up the tree to the root and strengthen every edge along the way
	*/
	void strengthenTree(BWAPI::UnitType type);

	SCGraph& getTree();

	/*
	Identifies the current strategy most likely being performed by the enemy. Acts as a passthrough to the Strategy Space to avoid circular dependencies
	*/
	void identifyStrategy();

private:
	// The Graph object
	SCGraph techTree;


	//Handles to strategy space and a reader for reading in the strategies
	StrategySpace strategies;
	StrategyReader reader;

	void buildTree();
};

