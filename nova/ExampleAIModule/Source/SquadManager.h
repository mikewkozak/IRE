#pragma once

#include "SquadAgent.h"
#include "InformationManager.h"
#include "WorkerManager.h"
#include "Search/AbstractLayer.h"

typedef std::map<BWAPI::Unit, SquadAgent*> UnitToSquadMap;
typedef std::vector< std::pair<SquadAgent*, SquadAgent*> > UnitPairSet;

class SquadManager
{
public:
	SquadManager();
	~SquadManager();
	void createNewSquad(const BWAPI::Unitset &units);
	void unitTraining(BWAPI::Unit unit);
	void newBunkerSquad(BWAPI::Unit unit);
	void newScoutSquad(BWAPI::Unit unit);
	void newDetectorSquad(BWAPI::Unit unit);
	void newHarassSquad(BWAPI::Unit unit);
	void addUnitToSquad(SquadAgent* squad, BWAPI::Unit unitToAdd);
	void onFrame();
//	void orderGetPosition(BWAPI::Position positionTarget);
	void onUnitDestroy(BWAPI::Unit unit);
	int getUnitFrameCreated(BWAPI::Unit unit);
	void newEnemy(BWAPI::Unit enemy, SquadAgent* oldSquad = nullptr);
	void onEnemyEvade(BWAPI::Unit enemy);
	void onEnemyDestroy(BWAPI::Unit enemy);

	void requestRetreat(SquadAgent* squad);
	BWAPI::Position getBestTarget();
	BWAPI::Position getScoutTarget();
	void removeMergingSquads(SquadAgent *squad);
	int normalSquads();

	SquadAgent* getClosestSquad(BWAPI::Position toPosition, SquadAgent* ignoreSquad, bool ignoreNoMergeable = false, bool ignoreNoAssignable = false);

	BWAPI::Unitset _unitsTraning;
	SquadAgent* _creatingSquad;
	SquadAgent* _leadSquad;
	SquadAgent* _searchSquad;
	SquadAgent* _detectorSquad;
	SquadAgent* _harassSquad;
	std::vector<SquadAgent*> _squads;
	UnitToSquadMap _unitToSquadMap;
	UnitToSquadMap _enemyToSquadMap;
 	BWAPI::Position _positionTarget;
	UnitPairSet _squadsToMerge;

private:
	log4cxx::LoggerPtr _logger;

	SquadAgent* createNewSquadOfType(SquadAgent::Order type);
	void addUnitToSquad(BWAPI::Unit unit);
	void checkAutoMerge();
	void checkMerge();
    void checkHighLevelOrder();
	void mergeUnitsAndEnemies(SquadAgent* squad1, SquadAgent* squad2);
	SquadAgent* getBestSquadLead();
	inline bool isSpecialSquad(SquadAgent* squad);
	bool isSquadNoMergeable(SquadAgent* squad);
	bool isSquadNoEnemyAssignable(SquadAgent* squad);

};