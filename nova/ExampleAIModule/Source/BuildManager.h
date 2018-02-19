#pragma once

#include "InformationManager.h"
#include "WorkerManager.h"
#include "WallGenerator.h"
#include "ProductionManager.h"

typedef std::set<BWAPI::TilePosition> TilePositionSet;
typedef std::vector<BWAPI::UnitType> UnitTypeVector;

struct MyRectangle {
	int x1;
	int y1;
	int x2;
	int y2;
};

class BuildManager
{
public:
	BuildManager();
	~BuildManager();
	void onFrame();
	BWAPI::TilePosition BuildManager::getBuildLocationNear(BWAPI::TilePosition position, BWAPI::UnitType type, BWTA::Region* inRegion = 0);
	void constructionPlaced(BWAPI::Unit build);
	void onBuildingDestroy(BWAPI::Unit build);
	bool alreadyBuilding(BWAPI::UnitType type);
	bool alreadyRequested(BWAPI::UnitType type);
	bool wallNear(BWAPI::TilePosition destination);
	void reserveBaseLocations();
	void findBunkerSeedPosition(BWTA::Region* seedRegion);
	int** getBuildMap() { return buildMap; }

	UnitTypeVector _buildOrder;
	UnitTypeVector _toConstruct;
	
private:
	log4cxx::LoggerPtr _logger;

	bool executeBuildOrder(BWAPI::UnitType buildType);
	BWAPI::TilePosition getGeyserTilePosition();
	bool needSupply();
	BWAPI::TilePosition getSeedForTurret();
	MyRectangle getBuildRectangle(BWAPI::TilePosition position, BWAPI::UnitType type);
	void setBuildMapRectangle(MyRectangle c, int label, int expand=0);
	bool canBuildHere(BWAPI::TilePosition buildPosition, BWAPI::UnitType type);
	bool anyMissileTurretsNear(BWAPI::TilePosition buildPosition);
	void drawBuildMap();
	void drawBuildOrder();
	bool canBuild(BWAPI::UnitType type);
	void findProxyPosition();
	void openGate();
	void closeGate();

	BWAPI::Unit _workerBuildingRefinery;
	int mapW;
	int mapH;
	int** buildMap;
	bool gateOpened;
	BWAPI::Unit gateBuilding;

};