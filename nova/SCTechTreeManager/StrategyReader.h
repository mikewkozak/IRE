//for getting all strategy files in directory
#include <boost/filesystem.hpp>
#include <vector>

//Graph structure and strategy axes
#include "StrategySpace.h"
#include "GraphUtils.h"

#pragma once
class StrategyReader
{
public:
	StrategyReader();
	~StrategyReader();

	//Read in strategy files and store in strategy map
	void init();

	void getTerranStrategies();

	std::vector<Strategy> buildTerranStrategies();

private:
	static const std::string TERRAN_STRATEGY_PATH;
};

