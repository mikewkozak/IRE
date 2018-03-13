/*
StrategyReader.h
This class handles reading the strategies from file and storing them for future access during runtime.
*/

//for getting all strategy files in directory
#include <boost/filesystem.hpp>
#include <vector>

//Graph structure and strategy axes
#include "StrategySpace.h"
#include "GraphUtils.h"

#pragma once
/*
Class responsible for reading in DOT strategy files and converting them to Strategy structs
*/
class StrategyReader
{
public:
	StrategyReader();
	~StrategyReader();

	/*
	Reads in the list of Terran Strategies and stores them for future use
	*/
	void getTerranStrategies();

	/*
	Function that generates terran strategies and writes them off to DOT files for future reference
	*/
	std::vector<Strategy> buildTerranStrategies();

private:
	//Path to the Terran Strategy Files
	static const std::string TERRAN_STRATEGY_PATH;
};

