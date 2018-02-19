#pragma once

#include "InformationManager.h"
#include "WallGenerator.h"

#define WORKERS_PER_MINERAL 2
#define WORKERS_PER_REFINERY 3

typedef std::map<BWAPI::Unit, int> ResourceToWorkerMap;

class WorkerManager
{
public:

	enum class State
	{
		Idle,
		Gathering_Mineral,
		Gathering_Gas,
		Building,
		Defending,
		Scouting, // "soft" scouting, just to check enemy base location
		Repairing
	};
	friend std::ostream& operator<< (std::ostream& out, const State& state)
	{
		switch (state) {
		case State::Idle: return out << "Idle";
		case State::Gathering_Mineral: return out << "Gathering_Mineral";
		case State::Gathering_Gas: return out << "Gathering_Gas";
		case State::Building: return out << "Building";
		case State::Defending: return out << "Defending";
		case State::Scouting: return out << "Scouting";
		case State::Repairing: return out << "Repairing";
		default: return out << static_cast<int>(state);
		}
	};

	struct WorkerData {
		BWAPI::Unit worker;
		State state;
		BWAPI::Unit target;
		int taskStarted;
		BWAPI::TilePosition buildPosition;
		BWAPI::UnitType buildType;
		WorkerData(BWAPI::Unit newWorker) 
			:worker(newWorker), state(State::Idle), target(nullptr), taskStarted(0), 
			buildPosition(BWAPI::TilePositions::None), buildType(BWAPI::UnitTypes::None) {}
	};

	WorkerManager();
	~WorkerManager();
	void addUnit(BWAPI::Unit worker);
	bool onUnitDestroy(BWAPI::Unit unit);
	void onMineralDestroy(BWAPI::Unit mineralField);
	bool onRefineryDestroy(BWAPI::Unit vespeneGeyser);
	void onBaseDestroy(BWTA::BaseLocation* base);
	void onBuildingComplete(BWAPI::Unit building);
	void onFrame();

	bool needWorkers();
	BWAPI::Unit getWorkerForTask(BWAPI::Position toPosition); // TODO this is dangerous!!!
	bool isUnderConstruction(BWAPI::UnitType type);
	void buildRequest(BWAPI::TilePosition locationToBuild, BWAPI::UnitType buildType);
	bool anyWorkerDefending();
	bool anyWorkerAttacking(BWAPI::Unit target);
	bool defenseBase(BWAPI::Unit unit);
	void attackBuilding(BWAPI::Unit target);
	void rebalanceGathering(); // TODO
	void scoutPosition(BWAPI::Position position);

	unsigned int getWorkersSize()             { return _workers.size(); }
	unsigned int getNumWorkersMining()        { return _workersMining; }
	unsigned int getNumWorkersExtractingGas() { return _workersExtractingGas; }
	unsigned int getNumWorkersBuilding()      { return _workersBuilding; }

	ResourceToWorkerMap _assignedWorkersToMinerals;
	ResourceToWorkerMap _assignedWorkersToGas;

private:
	log4cxx::LoggerPtr _logger;

	void cleanLastJob(WorkerData* workerData);
	bool setJob(State job, WorkerData* workerData = nullptr, BWAPI::Unit target = nullptr,
		BWAPI::TilePosition buildPosition = BWAPI::TilePositions::None, BWAPI::UnitType buildType = BWAPI::UnitTypes::None);
	bool setJobAndExecute(State job, WorkerData* workerData = nullptr, BWAPI::Unit target = nullptr,
		BWAPI::TilePosition buildPosition = BWAPI::TilePositions::None, BWAPI::UnitType buildType = BWAPI::UnitTypes::None);

	WorkerData* getWorkerDataForTask(BWAPI::Position toPosition);
	void reassignWorkersFromRemovedTarget(BWAPI::Unit removedTarget);

	bool isBuildPositionVisible(BWAPI::TilePosition buildPosition, BWAPI::UnitType buildType);
	bool isAnyWorkerBuilding(BWAPI::TilePosition buildPosition, BWAPI::UnitType buildType);

	void handleBuildingsIncompleted();
	void handleBuildingsDamaged();
	void handleRefineryWorkers();

	BWAPI::Unit getBestMineral();
	void tryMiningTrick(BWAPI::Unit worker);

	std::vector<WorkerData*> _workers;
	unsigned int _workersMining;
	unsigned int _workersExtractingGas;
	unsigned int _workersBuilding;
};