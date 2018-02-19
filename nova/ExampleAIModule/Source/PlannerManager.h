#pragma once

#include <queue>
#include "InformationManager.h"

class PlannerManager
{
public:
	PlannerManager();

	void setBalance(UnitToPercent percentList);
	void onFrame();
	void rebalanceProduction();
	void updateSelfArmy();
	void updateEnemyArmy();
	void updateSelfArmyStats(BWAPI::UnitType type, double size);
	void updateEnemyArmyStats(BWAPI::UnitType type, double size);

	UnitToPercent _percentList;
};

// class for priority queue
class UnitTypePercent
{
public:
	UnitTypePercent() {};
	UnitTypePercent(BWAPI::UnitType x, double y) { type = x; percent = y; }
	
	bool operator<(const UnitTypePercent&) const; //overloaded < operator

	BWAPI::UnitType type;
	double percent;
};