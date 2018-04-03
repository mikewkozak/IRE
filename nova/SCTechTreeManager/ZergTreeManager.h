/*
ZergTreeManager.h
Class representing the full tech tree for the Zerg Race.
*/
#include "ITreeManager.h"

#pragma once
/*
Class representing the full tech tree for the Zerg Race.
*/
class ZergTreeManager : public ITreeManager
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

protected:
	void buildTree();
};

