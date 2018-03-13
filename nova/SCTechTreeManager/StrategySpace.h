/*
StrategySpace.h
This class represents an N-dimensional graph in which all enemy strategies are stored. The axes of this graph each represent a single feature of a strategy
such as air vs anti-air or agressiveness versus defensiveness. When an enemy unit or building is detected, the "strategy space" is informed and it increases
the "strength" of strategies that rely on this unit. Over time, the strategies most likely to be engaged by the enemy will be the "strongest" and IRE can
use that information to build a counter-strategy. 
*/

#include "GraphUtils.h"

#pragma once
/*
This class represents an N-dimensional graph in which all enemy strategies are stored. The axes of this graph each represent a single feature of a strategy
such as air vs anti-air or agressiveness versus defensiveness. When an enemy unit or building is detected, the "strategy space" is informed and it increases
the "strength" of strategies that rely on this unit. Over time, the strategies most likely to be engaged by the enemy will be the "strongest" and IRE can
use that information to build a counter-strategy.
*/
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

	/*
	Support function that finds a particular unit/building type within the strategy space and returns its location
	*/
	Vertex findNode(int race, Vertex node);

	//Returns the root of the particular races' strategy trees. This root represents the "center" of that races' strategy space and is located at (0,0,0)
	VertexDescriptor& getTerranStrategyRoot();
	VertexDescriptor& getProtossStrategyRoot();
	VertexDescriptor& getZergStrategyRoot();

	/*
	Support function that prints all the strategy spaces to file using the GraphUtil static functions
	*/
	void printStrategySpaces();

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

	/*
	Support function that returns the strategy space associated with the given race
	*/
	SCGraph& getTechTree(int race);
};

