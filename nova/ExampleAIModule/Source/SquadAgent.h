#pragma once

#include "InformationManager.h"
#include "CombatAgent.h"
#include "WorkerManager.h"

typedef std::set<CombatAgent*> CombatUnitSet;
typedef std::map<BWAPI::Unit, CombatAgent*> UnitToCombatAgentMap;

class SquadAgent
{
public:
	struct stats_t {
		double airDPS;
		double groundDPS;
		double airHP;
		double groundHP;
	};
	enum Order {
		Idle,
		GetPosition,
		Fight,
		MergeSquads,
		Bunker,
		Search,
		Scout,
		Detector,
		HoldCenter,
		Harass
	};
	enum Formation {
		Normal,
		Cohesion
	};

	SquadAgent();
	~SquadAgent();
	std::string getState();
	void addUnit(BWAPI::Unit unit);
	void addUnits(BWAPI::Unitset units);
	void onFrame();
	void orderGetPosition(BWAPI::Position positionTarget);
	void orderWait(BWAPI::Unit unitToSkip = NULL);
	void inCombat();
	void onUnitDestroy(BWAPI::Unit unit);
	int getUnitFrameCreated(BWAPI::Unit unit);
	void inMerge();
	void inMerge(SquadAgent* squadToMerge, BWAPI::Position toMerge = BWAPI::Positions::None);
	bool hasUnitOfType(const BWAPI::UnitType &type);
	void insertEnemyThreat(BWAPI::Unit unit){ updateThreat( unit, _enemyStats, std::plus<double>() ); };
	void removeEnemyThreat(BWAPI::Unit unit){ updateThreat( unit, _enemyStats, std::minus<double>() ); };
	CombatAgent* getClosestUnitTo(BWAPI::Position toPosition, BWAPI::UnitType type = BWAPI::UnitTypes::None, bool ignoreFlyers = false, bool forceResult = true);
	bool isSquadBio();
	bool squadNeedSCV();

	CombatUnitSet _squadUnits;
	UnitToCombatAgentMap _unitToCombatAgentMap;
	BWAPI::Unitset _enemies;
	BWAPI::Position _center;
	double _spread; // Average distance to center.
	Order _state;
	BWAPI::Position _positionTarget;
	SquadAgent* _squadToMerge;
	bool _waitingNewUnits;
	BWAPI::Position _positionToMerge;
	std::string _waitingReason;
	BWAPI::Position _holdCenterPosition;

	unsigned int _maxEnemyUnitsSeen;
	stats_t _enemyStats;
	stats_t _squadStats;

private:
	void squadArea();
	bool canWin();
	void checkFormation();
	void checkSpread();
	bool needWait();
	void insertSquadThreat(BWAPI::Unit unit){ updateThreat( unit, _squadStats, std::plus<double>() ); };
	void removeSquadThreat(BWAPI::Unit unit){ updateThreat( unit, _squadStats, std::minus<double>() ); };
	template<typename Func>
	void updateThreat( BWAPI::Unit unit, stats_t &stats, Func f );
	void loadToEmptyBunker(BWAPI::Unit unit);
	void scouting();
	void holdCenter();
	void harassing();
	BWAPI::Position getPositionToScout(BWAPI::Position seedPos, BWTA::Region* myRegion, BWAPI::Position basePos, bool checkVisible = false);

	//int _maxSpread;
	//bool _compacting;
	Formation _movement;
	int _squadMaxSpread;
	log4cxx::LoggerPtr _logger;
};
