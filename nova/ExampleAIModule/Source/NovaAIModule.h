#pragma once

#ifdef LOG4CXX_STATIC
#include "log4cxx/propertyconfigurator.h"
#include <log4cxx/helpers/pool.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/simplelayout.h>
#include <log4cxx/logstring.h>
#endif

#ifdef NOVA_GUI
#include "GUI/QtWindow.h"
#endif

#include "EnhancedUI.h"
#include "SquadManager.h"
#include "InformationManager.h"
#include "WorkerManager.h"
#include "ProductionManager.h"
#include "BuildManager.h"
#include "WallGenerator.h"
#include "PlannerManager.h"
#include "StrategyManager.h"

#include "Search/AbstractLayer.h"
#include "Search/UnitInfoStatic.h"
#include "Search/CombatSimSustained.h"
#include "Search/CombatSimDecreased.h"
#include "Search/CombatSimLanchester.h"
#include "Search/TargetSorting.h"

class NovaAIModule : public BWAPI::AIModule
{
public:
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(std::string text);
	virtual void onReceiveText(BWAPI::Player* player, std::string text);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit unit);
	virtual void onUnitEvade(BWAPI::Unit unit);
	virtual void onUnitShow(BWAPI::Unit unit);
	virtual void onUnitHide(BWAPI::Unit unit);
	virtual void onUnitCreate(BWAPI::Unit unit);
	virtual void onUnitComplete(BWAPI::Unit unit);
	virtual void onUnitDestroy(BWAPI::Unit unit);
	virtual void onUnitMorph(BWAPI::Unit unit);
	virtual void onUnitRenegade(BWAPI::Unit unit);

	NovaAIModule();
	~NovaAIModule();
	EnhancedUI* enhancedUI;
	// ProductionManager* productionManager;
	PlannerManager* plannerManager;
	StrategyManager* strategyManager;

	// Pathfinding test
	bool PATHFINDING01;
	std::list<BWTA::Chokepoint*> path;

	//Kiting tests
	int selfHitPoints;
	int selfMaxHitPoints;
	int enemyHitPoints;
	int enemyMaxHitPoints;
	std::set<BWAPI::Bullet*> microBullets;
	int numShots;
	std::string firstUnit;
	int frameFirstUnit;
	int vulturesCreated;
	int vulturesKilled;
	int totalVulutreLife;
	int zealotsKilled;

private:
	log4cxx::LoggerPtr _logger;
	int leavingGame;
	bool saidGG;

    int timeout;

    void setUpLogging();
    void loadIniConfig();
};
