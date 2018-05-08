/*
ITreeManager.h
Abstract Class (interface) representing the full tech tree for any arbitrary race.
*/
#include <list>

//Graph structure and strategy axes
#include "Strategy/StrategySpace.h"
#include "Strategy/StrategyReader.h"
#include "Utils/GraphUtils.h"

#pragma once
/*
Abstract Class (interface) representing the full tech tree for any arbitrary race.
*/
class ITreeManager
{
public:
	ITreeManager() {}
	virtual ~ITreeManager() {}

	/*
	Given a unit, building, or upgrade, traverse up the tree to the root and strengthen every edge along the way
	*/
	virtual void strengthenTree(BWAPI::UnitType type) = 0;

	/*
	Returns the complete Tech Tree graph in its current state based on all observations
	*/
	virtual SCGraph& getTree() = 0;

	/*
	Identifies the current strategy most likely being performed by the enemy. Acts as a passthrough to the Strategy Space to avoid circular dependencies
	*/
	virtual StrategyRecommendation identifyStrategy() = 0;

protected:
	// The Graph object
	SCGraph techTree;


	//Handles to strategy space and a reader for reading in the strategies
	StrategySpace strategies;
	StrategyReader reader;

	virtual void buildTree() = 0;
};

