//for getting all strategy files in directory
#include <boost/filesystem.hpp>

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
	Strategy init();

	void getTerranStrategies();

	void buildStrategies();

private:
	static const std::string TERRAN_STRATEGY_PATH;

	//TOY PROBLEM
	SCGraph vultureRush;
};

