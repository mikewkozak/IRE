#pragma once

#include "InformationManager.h"
#include "BuildManager.h"
#include "WorkerManager.h"

class ProductionManager
{
public:
	ProductionManager();
	void onBuildingShow(BWAPI::Unit unit);
	void onFrame();
	void onBuildingDestroy(BWAPI::Unit unit);

	BWAPI::Unitset _commandCenters;
	BWAPI::Unitset _barracks;
	BWAPI::Unitset _factories;
	BWAPI::Unitset _starports;
	BWAPI::Unit _academy;
	BWAPI::Unit _engineeringBay;
	BWAPI::Unitset _machineShop;
	BWAPI::Unitset _controlTower;
	BWAPI::Unitset _armory;
	BWAPI::Unit _scienceFacility;
	BWAPI::Unit _physicsLab;
	BWAPI::Unit _covertOps;
	BWAPI::Unitset _nuclearSilo;

private:
	bool canTrain(BWAPI::UnitType type);
	bool onProduction(BWAPI::Unitset buildings);
	BWAPI::UnitType needAddon(BWAPI::UnitType toTrain);
	BWAPI::UpgradeType getNextArmoryUpgrade();
	BWAPI::UpgradeType getNextInfanteryUpgrade();
	BWAPI::Position getRallyPosition(BWAPI::Unit unit);
	void printBlockedProduction(BWAPI::Unitset buildings, std::string message, int &counter);
	void printBlockedProduction(BWAPI::Unit unit, std::string message, int &counter);

	log4cxx::LoggerPtr _logger;

};