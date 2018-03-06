#include "GraphUtils.h"

#pragma once
class StrategySpace
{
public:
	//x axis
	static const int AIR_AA_AXIS = 0;
	static const int AIR_AA_AXIS_MAX = 1;
	static const int AIR_AA_AXIS_MIN = -1;

	//y axis
	static const int GROUND_AG_AXIS = 1;
	static const int GROUND_AG_AXIS_MAX = 1;
	static const int GROUND_AG_AXIS_MIN = -1;

	//z axis
	static const int AGGRESSIVE_DEFENSIVE_AXIS = 2;
	static const int AGGRESSIVE_DEFENSIVE_AXIS_MAX = 1;
	static const int AGGRESSIVE_DEFENSIVE_AXIS_MIN = -1;


	StrategySpace();
	~StrategySpace();

	/*
	Adds a new tree to the strategy space and lays out the nodes based on the intensity of the
	strategy along each major axis in the space
	*/
	void addStrategy(int race, Strategy strat);

	/*
	Given a unit, building, or upgrade, traverse up the tree to the root and strengthen every edge along the way
	*/
	void strengthenTree(int race, BWAPI::UnitType type);


	/*
	Given the current state of the strategy space, ID which strategies are likely being used
	*/
	void identifyStrategy(int race);


	Vertex findNode(int race, Vertex node);

private:
	//Graph representing the combined strategies of the Terran Race
	SCGraph terranStrategySpace;
	VertexDescriptor terranStrategySpaceRoot;

	//Graph representing the combined strategies of the Protoss Race
	SCGraph protossStrategySpace;
	VertexDescriptor protossStrategySpaceRoot;

	//Graph representing the combined strategies of the Zerg Race
	SCGraph zergStrategySpace;
	VertexDescriptor zergStrategySpaceRoot;

	SCGraph& getTechTree(int race);
};

