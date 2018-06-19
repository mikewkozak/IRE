/*
StrategyRecommendation.h
This class wraps the data produced from the identified strategy necessary to bias unit production
*/


#pragma once
/*
Class that wraps the data produced from the identified strategy necessary to bias unit production
*/
class StrategyRecommendation {
public:
	StrategyRecommendation();

	std::vector<std::string> strategyIdentified;
	double proposedAirAggressiveness;
	double proposedGroundAggressiveness;
	double proposedOverallAggressiveness;
};