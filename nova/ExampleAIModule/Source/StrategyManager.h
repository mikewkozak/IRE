#pragma once

#include "InformationManager.h"
#include "ProductionManager.h"
#include "BuildManager.h"
#include "SquadManager.h"
#include "WallGenerator.h"

#include "StateMachine.h"
#include "VsTerran.h"
#include "VsZerg.h"
#include "VsProtoss.h"

typedef std::map<BWAPI::TilePosition, int> TilePositionToFrame;

class StrategyManager
{
public:
	StrategyManager(ProductionManager *productionManager);
	~StrategyManager();
	void onFrame();
	void handleCloakedEnemy();
	void checkGasSteal(BWAPI::Unit unit);
	StateMachine<StrategyManager>*  GetFSM() { return _StateMachine; }
	std::vector<std::string>  getCurrentEnemyStrategies() { return _currentEnemyStrategy; }

private:
	log4cxx::LoggerPtr _logger;

	TilePositionToFrame _lastSweepFrame;
	ProductionManager *_productionManager;

	StateMachine<StrategyManager>*  _StateMachine;

	unsigned int _inferenceCounter;//used to limit the frequency of strategy predictions
	std::vector<std::string> _currentEnemyStrategy;//the current strategy being pursued according to IRE

	BWAPI::Unitset _liftBuildings;
	bool _hiddingCorners;
};