#include "WorkerManager.h"
#include "SquadManager.h"

using namespace BWAPI;

WorkerManager::WorkerManager()
	:_logger(log4cxx::Logger::getLogger("WorkerManager")),
	_workersMining(0),
	_workersExtractingGas(0),
	_workersBuilding(0)
{
	// save initial minerals
	for (const auto& mineral : BWAPI::Broodwar->getMinerals()) {
		if (mineral->isVisible()) _assignedWorkersToMinerals[mineral] = 0;
	}

// 	_workers.clear();
};

WorkerManager::~WorkerManager()
{
	for (auto& worker : _workers) delete worker;
}

void WorkerManager::addUnit(Unit worker) 
{
	WorkerData* newWorker = new WorkerData(worker);
	_workers.push_back(newWorker);
	// By default a new worker is assigned to gather minerals
	setJobAndExecute(State::Gathering_Mineral, newWorker);
}

// If the job can be assigned it will execute it
bool WorkerManager::setJobAndExecute(State job, WorkerData* workerData, BWAPI::Unit target, BWAPI::TilePosition buildPosition, BWAPI::UnitType buildType)
{
	// if no worker given it will try to find one
	if (!workerData) {
		// define seed position
		Position seedPosition(Positions::None);
		if (job == State::Building) {
			seedPosition = Position(buildPosition);
			// Map "The Fortress" has trap base locations
			if ("The Fortress 1.1" == Broodwar->mapName() && (buildType == UnitTypes::Terran_Command_Center || buildType == UnitTypes::Terran_Refinery)) {
				seedPosition = Position(Broodwar->self()->getStartLocation());
			}
		} else if (target != nullptr) {
			seedPosition = target->getPosition();
		}
		// search idle worker near seed position
		workerData = getWorkerDataForTask(seedPosition);
	}

	if (workerData) {
		if (setJob(job, workerData, target, buildPosition, buildType)) {
			// after finishing a building, the worker is in a state that cannot receive a command
			if (workerData->worker->getOrder() == Orders::ResetCollision) return false;
			
			bool ok = true;

			switch (job) {
			case State::Idle:
				ok = workerData->worker->stop();
				break;
			case State::Gathering_Mineral:
				ok = workerData->worker->rightClick(workerData->target);
				break;
			case State::Gathering_Gas:
				ok = workerData->worker->rightClick(workerData->target);
				break;
			case State::Building:
				if (workerData->target) { // the order is to continue an unfinished building
					ok = workerData->worker->rightClick(workerData->target);
				} else {
					LOG4CXX_TRACE(_logger, "Build order - " << workerData->buildType.getName().c_str() << " " << Broodwar->getFrameCount());
					if (isBuildPositionVisible(workerData->buildPosition, workerData->buildType)) {
						ok = workerData->worker->build(workerData->buildType, workerData->buildPosition);
					} else {
						ok = workerData->worker->move(Position(workerData->buildPosition));
					}
				}
				break;
			case State::Defending:
				ok = workerData->worker->attack(workerData->target);
				break;
			case State::Scouting:
				ok = workerData->worker->move(Position(workerData->buildPosition));
				break;
			case State::Repairing:
				ok = workerData->worker->repair(workerData->target);
				break;
			}

			if (!ok) {
				LOG4CXX_ERROR(_logger, "[WORKER ORDER ERROR] " << Broodwar->getLastError().toString());
				LOG4CXX_ERROR(_logger, "  worker CURRENT order " << workerData->worker->getOrder().c_str());
				LOG4CXX_ERROR(_logger, "  worker DESIRED state " << workerData->state);
				if (workerData->target)
					LOG4CXX_ERROR(_logger, "  workerData->target " << workerData->target->getType().c_str());
				LOG4CXX_ERROR(_logger, "  workerData->buildPosition " << workerData->buildPosition);
				LOG4CXX_ERROR(_logger, "  workerData->buildType " << workerData->buildType);

				if (Broodwar->getLastError() == Errors::Insufficient_Minerals) {
					LOG4CXX_ERROR(_logger, "Minerals " << Broodwar->self()->minerals() << " - "
						<< informationManager->_mineralsReserved << " - " 
						<< informationManager->_frameMineralSpend << " = " << informationManager->minerals());
					LOG4CXX_ERROR(_logger, "Build cost " << workerData->buildType.mineralPrice());
#ifndef TOURNAMENT
// 					Broodwar->setScreenPosition(workerData->worker->getLeft(), workerData->worker->getTop());
// 					Broodwar->setLocalSpeed(200);
#endif
				}
			}
			return ok;
		}
	}

	return false;
}

void WorkerManager::cleanLastJob(WorkerData* workerData)
{
	switch (workerData->state) {
	case State::Gathering_Mineral:
		if (workerData->target != nullptr) {
			_assignedWorkersToMinerals[workerData->target] = _assignedWorkersToMinerals[workerData->target]--;
		}
		--_workersMining;
		break;
	case State::Gathering_Gas:
		if (workerData->target != nullptr) {
			_assignedWorkersToGas[workerData->target] = _assignedWorkersToGas[workerData->target]--;
		}
		--_workersExtractingGas;
		break;
	case State::Building:
		--_workersBuilding;
		break;
	}
}

bool WorkerManager::setJob(State job, WorkerData* workerData, BWAPI::Unit target, BWAPI::TilePosition buildPosition, BWAPI::UnitType buildType)
{
	cleanLastJob(workerData);

	// assign new job
	workerData->state = job;
	workerData->target = target;
	workerData->buildPosition = buildPosition;
	workerData->buildType = buildType;

	switch (job) {
	case State::Gathering_Mineral:
	{
		Unit bestMineral = getBestMineral();
		if (bestMineral != nullptr) {
			workerData->target = bestMineral;
			_assignedWorkersToMinerals[bestMineral] = _assignedWorkersToMinerals[bestMineral]++;
			++_workersMining;
		} else {
			workerData->state = State::Idle;
			LOG4CXX_FATAL(_logger, "No more minerals");
			return false;
		}
	}
		break;
	case State::Gathering_Gas:
		_assignedWorkersToGas[target] = _assignedWorkersToGas[target]++;
		++_workersExtractingGas;
		break;
	case State::Building:
		workerData->taskStarted = Broodwar->getFrameCount();
		++_workersBuilding;
		break;
	case State::Defending:
		workerData->taskStarted = Broodwar->getFrameCount();
		break;
	case State::Scouting:
	case State::Repairing:
		break;
	default:
		LOG4CXX_FATAL(_logger, "Undefined job for a worker");
		return false;
		break;
	}

	return true;
}

// Search IDLE or MINING worker closest to Position
WorkerManager::WorkerData* WorkerManager::getWorkerDataForTask(BWAPI::Position toPosition)
{
	int currentDistance;
	int minDistanceIdle = std::numeric_limits<int>::max();
	int minDistanceMining = std::numeric_limits<int>::max();
	int minDistanceMiningCarrying = std::numeric_limits<int>::max();
	WorkerData* bestWorkerIdle = nullptr;
	WorkerData* bestWorkerMining = nullptr;
	WorkerData* bestWorkerMiningCarrying = nullptr;
	for (auto& workerData : _workers) {
		if (workerData->state == State::Idle || workerData->state == State::Scouting) {
			currentDistance = workerData->worker->getPosition().getApproxDistance(toPosition);
			if (currentDistance < minDistanceIdle) {
				minDistanceIdle = currentDistance;
				bestWorkerIdle = workerData;
			}
		} else if (workerData->state == State::Gathering_Mineral) {
			currentDistance = workerData->worker->getPosition().getApproxDistance(toPosition);
			if (workerData->worker->isCarryingMinerals() && currentDistance < minDistanceMiningCarrying) {
				minDistanceMiningCarrying = currentDistance;
				bestWorkerMiningCarrying = workerData;
			} else if (currentDistance < minDistanceMining) {
				minDistanceMining = currentDistance;
				bestWorkerMining = workerData;
			}
		}
	}

	if (bestWorkerIdle != nullptr) return bestWorkerIdle;
	else if (bestWorkerMining != nullptr) return bestWorkerMining;
	else if (bestWorkerMiningCarrying != nullptr) return bestWorkerMiningCarrying;

	LOG4CXX_WARN(_logger, "We didn't find any worker for task");
	return nullptr;
}

Unit WorkerManager::getWorkerForTask(Position toPosition)
{
	WorkerData* workerData = getWorkerDataForTask(toPosition);
	if (workerData) return workerData->worker;
	else return nullptr;
}

void WorkerManager::onBuildingComplete(BWAPI::Unit building)
{
	// look for the worker was building and assign it to gather minerals
	for (auto& workerData : _workers) {
		if (workerData->state == State::Building &&
			workerData->buildType == building->getType() && 
			workerData->buildPosition == building->getTilePosition()) {
			LOG4CXX_TRACE(_logger, "Worker building " << building->getType().c_str() << " set to mine");
			setJobAndExecute(State::Gathering_Mineral, workerData);
		}
	}

	// if it is a refinery we need to "create it" in order to assign workers to extract gas
	if (building->getType() == UnitTypes::Terran_Refinery) {
		_assignedWorkersToGas[building] = 0;
	}

	// if it is a base we need to add the minerals (except for initial base)
	if (building->getType().isResourceDepot() && Broodwar->self()->getStartLocation() != building->getTilePosition()) {
		BWTA::BaseLocation* base = BWTA::getNearestBaseLocation(building->getTilePosition());
		for (const auto& mineral : base->getMinerals()) {
			_assignedWorkersToMinerals[mineral] = 0;
		}
	}
}

bool WorkerManager::onUnitDestroy(Unit unit)
{
	auto it = _workers.begin();
	for (auto& workerData : _workers) {
		if (workerData->worker == unit) {
			cleanLastJob(workerData);
			delete workerData;
			_workers.erase(it);
			return true;
		}
		++it;
	}
	return false; // SCV wasn't in the workerManager, maybe in the squadManager?
}

void WorkerManager::reassignWorkersFromRemovedTarget(BWAPI::Unit removedTarget)
{
	for (auto& workerData : _workers) {
		if (workerData->target == removedTarget) {
// 			LOG4CXX_ERROR(_logger, "Worker target removed: " << workerData->target);
			workerData->target = nullptr;
			setJobAndExecute(State::Gathering_Mineral, workerData);
// 			LOG4CXX_ERROR(_logger, "New worker target: " << workerData->target);
		}
	}
}

void WorkerManager::onMineralDestroy(Unit mineralField)
{
// 	LOG4CXX_ERROR(_logger, "Deleting mineral: " << mineralField);
	auto deleted = _assignedWorkersToMinerals.erase(mineralField);
// 	for (const auto& workersAssigned : _assignedWorkersToMinerals) {
// 		LOG4CXX_ERROR(_logger, "Mineral " << workersAssigned.first << " has " << workersAssigned.second << " workers");
// 	}
	if (deleted) reassignWorkersFromRemovedTarget(mineralField);
}

// returns true if the refinery was ours
bool WorkerManager::onRefineryDestroy(BWAPI::Unit vespeneGeyser)
{
	auto deleted = _assignedWorkersToGas.erase(vespeneGeyser);
	if (deleted) {
// 		LOG4CXX_ERROR(_logger, "Deleting Refinery: " << vespeneGeyser);
		reassignWorkersFromRemovedTarget(vespeneGeyser);
		return true; // the refinery was ours
	} else {
		return false; // the refinery was NOT ours
	}
}

void WorkerManager::onBaseDestroy(BWTA::BaseLocation* base)
{
	auto& minerals = base->getMinerals();
	for (const auto& mineral : minerals) {
		auto deleted = _assignedWorkersToMinerals.erase(mineral);
		if (deleted) reassignWorkersFromRemovedTarget(mineral);
	}
}

Unit WorkerManager::getBestMineral()
{
	Unit bestMineral = nullptr;
	int bestScore = 99;

	for (const auto& workersAssigned : _assignedWorkersToMinerals) {
		if (workersAssigned.second < bestScore) {
			bestScore = workersAssigned.second;
			bestMineral = workersAssigned.first;
		}
	}
	return bestMineral;
}

void WorkerManager::rebalanceGathering()
{
	//TODO
}

void WorkerManager::onFrame() 
{
	LOG4CXX_TRACE(_logger, "Update workers state");
	Unitset workersToDelete; // used to delete worker assigned to scout

	for(auto& workerData : _workers) {
		Unit worker = workerData->worker;

		// auto-self-defense from workers
		LOG4CXX_TRACE(_logger, "[WORKER] Auto-self-defense");
		if (worker->isUnderAttack() && !worker->isAttacking()) {
			Unitset unitsInRange = worker->getUnitsInRadius(35, Filter::IsWorker && Filter::IsEnemy);
			for (const auto& unit : unitsInRange) {
				// if is constructing we need to stop first
				if (worker->isConstructing()) {
					worker->stop();
					continue; // TODO watch out it was return...
				}
				worker->attack(unit);
				workerData->taskStarted = BWAPI::Broodwar->getFrameCount();
			}
		}
		// If worker is attacking but target not, stop attacking.
		if (worker->isAttacking() && BWAPI::Broodwar->getFrameCount() - workerData->taskStarted > 1 * 24) {
			Unit target = worker->getTarget();
			if ( target != nullptr && !target->isAttacking() && !target->getType().isBuilding() ) {
				worker->stop();
			}
		}

		switch (workerData->state)
		{
		// ---------- WORKERS GATHERING -----------------------------
		case State::Gathering_Mineral:
		case State::Gathering_Gas:
			if (worker->getOrder() == Orders::PlayerGuard) {
				if (workerData->target) worker->rightClick(workerData->target);
				else LOG4CXX_ERROR(_logger, "Worker assigned to null resource");
			}
			break;
		case State::Scouting:
			if (worker->getOrder() == Orders::PlayerGuard) {
				setJobAndExecute(State::Gathering_Mineral, workerData);
			}
			break;
		// ---------- WORKERS BUILDING -----------------------------
		case State::Building:
		{
			LOG4CXX_TRACE(_logger, "[WORKER] Building");
#ifndef TOURNAMENT
			Position uPos(worker->getPosition());
			Position bPos1(workerData->buildPosition);
			Position bPos2(bPos1.x + (workerData->buildType.tileWidth()*TILE_SIZE), bPos1.y + (workerData->buildType.tileHeight()*TILE_SIZE));
			Broodwar->drawCircleMap(uPos, 30, Colors::Orange);
			Broodwar->drawLineMap(uPos, bPos1, Colors::Orange);
			Broodwar->drawBoxMap(bPos1, bPos2, Colors::Orange);
#endif

			// To prevent pathfinding problems, if we cannot reach a new base location switch to another one
			bool timeout = Broodwar->getFrameCount() - workerData->taskStarted > 50 * 24;
			if (workerData->buildType == UnitTypes::Terran_Command_Center && worker->getOrder() == Orders::Move && timeout) {
				informationManager->_ignoreBases.insert(workerData->buildPosition);
				workerData->buildPosition = informationManager->getExpandPosition();
				LOG4CXX_ERROR(_logger, "Timout building Base, looking for another position");
				setJobAndExecute(State::Building, workerData, nullptr, workerData->buildPosition, workerData->buildType);
			}

			if (worker->getOrder() == Orders::PlayerGuard) {
				bool buildExist = false;
				for (const auto& u : Broodwar->getUnitsOnTile(workerData->buildPosition)) {
					if (u->getType() == workerData->buildType) buildExist = true;
				}
				if (buildExist) { // if build finished change worker state to gather
					LOG4CXX_ERROR(_logger, "Building completed not captured in onBuildingComplete: " << workerData->buildType);
					setJobAndExecute(State::Gathering_Mineral, workerData);
				} else { // worker is idle but the building is not built
					// if we can build send again order
					if (Broodwar->canBuildHere(workerData->buildPosition, workerData->buildType, worker)) {
						LOG4CXX_TRACE(_logger, "Builder idle, try again: " << workerData->buildType);
						setJobAndExecute(State::Building, workerData, nullptr, workerData->buildPosition, workerData->buildType);
					} else { // we cannot build, search reason
						Position bPos1(workerData->buildPosition);
						Position bPos2(bPos1.x + (workerData->buildType.tileWidth()*TILE_SIZE), bPos1.y + (workerData->buildType.tileHeight()*TILE_SIZE));
						bool wait = true;
						for (const auto& u : Broodwar->getUnitsInRectangle(bPos1, bPos2)) {
							if (!u->getType().canMove() || Broodwar->self()->isEnemy(u->getPlayer())) {
								wait = false;
								break;
							}
						}
						if (wait && !timeout) { // if only movable friendly units wait until timeout
							LOG4CXX_TRACE(_logger, "Builder idle, waiting: " << workerData->buildType);
						} else { // otherwise, cancel build order
							LOG4CXX_WARN(_logger, "Builder idle, imposible to build: " << workerData->buildType);
							// TODO detect edge cases like enemy mine in base location
							setJobAndExecute(State::Gathering_Mineral, workerData);
						}
					}
				}
			} else if (worker->getOrder() == Orders::ConstructingBuilding) {
				// After building the firs Supply Depot, send worker to scout TODO: rethink this
				if (workerData->buildType == UnitTypes::Terran_Supply_Depot && 
					Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Supply_Depot) == 1) {
					if (informationManager->startLocationCouldContainEnemy.size() > 1) {
						squadManager->newScoutSquad(worker);
						// mark worker to delete from WrokerManager (we cannot do it here because we will screw up the iterator!!)
						workersToDelete.insert(worker);
						LOG4CXX_TRACE(_logger, "Adding worker to delete " << worker);
					}
				}
			} else if (worker->getOrder() == Orders::Move && isBuildPositionVisible(workerData->buildPosition, workerData->buildType)) {
				if (Broodwar->canBuildHere(workerData->buildPosition, workerData->buildType, worker)) {
					if (!worker->build(workerData->buildType, workerData->buildPosition)) {
						LOG4CXX_ERROR(_logger, "Build position reached but: " << Broodwar->getLastError().toString());
					}
				}
			} else if (worker->getOrder() == Orders::MiningMinerals) { // why are you doing this!!!!
				LOG4CXX_ERROR(_logger, "Worker assigned to build was mining");
				worker->stop();
			}
		}
			break;
		// ---------- WORKERS DEFENDING -----------------------------
		case State::Defending:
		{
			LOG4CXX_TRACE(_logger, "[WORKER] Defending");
#ifndef TOURNAMENT
			Position uPos = worker->getPosition();
			Broodwar->drawCircleMap(uPos, 30, Colors::Red);
#endif
			// if out of the region or no enemy
			BWTA::Region* workerRegion = BWTA::getRegion(worker->getTilePosition());
			if (workerRegion != informationManager->home || worker->getOrder() == Orders::PlayerGuard) {
				setJobAndExecute(State::Gathering_Mineral, workerData);
			}
		}
			break;
		// ---------- WORKERS REPARING -----------------------------
		case State::Repairing:
#ifndef TOURNAMENT
			Position uPos(worker->getPosition());
			Position bPos1(workerData->target->getLeft(), workerData->target->getTop());
			Position bPos2(workerData->target->getRight(), workerData->target->getBottom());
			Broodwar->drawCircleMap(uPos, 30, Colors::Green);
			Broodwar->drawLineMap(uPos, bPos1, Colors::Green);
			Broodwar->drawBoxMap(bPos1, bPos2, Colors::Green);
#endif
			if (worker->getOrder() == Orders::PlayerGuard) {
				if (workerData->target->getHitPoints() != workerData->target->getType().maxHitPoints()) {
					if (Broodwar->self()->minerals() > 0) {
						setJobAndExecute(State::Repairing, workerData, workerData->target); // keep repairing
					}
				} else {
					setJobAndExecute(State::Gathering_Mineral, workerData);
				}
			}
			break;
		}
	} // end workers loop

	// check if we have workers to delete
	for (auto worker : workersToDelete) {
		LOG4CXX_TRACE(_logger, "Deleting worker " << worker);
		onUnitDestroy(worker);
	}

	// check for complete Refinery in order to assign more workers -------------------------------------------------------
	LOG4CXX_TRACE(_logger, "Checking workes assigned to refineries");
	handleRefineryWorkers();

	// check for incomplete buildings ----------------------------------------------------------------------------------------
	LOG4CXX_TRACE(_logger, "Check incomplete buidings");
	handleBuildingsIncompleted();

	// check builds needing repair ----------------------------------------------------------------------------------------
	LOG4CXX_TRACE(_logger, "Check builds needing repair");
	handleBuildingsDamaged();

	Broodwar->drawTextScreen(310, 16, "Workers mining: %d (limit: %d)", getNumWorkersMining(), informationManager->_maxWorkersMining);
	Broodwar->drawTextScreen(310, 16 * 2, "Workers gas: %d", getNumWorkersExtractingGas());
	Broodwar->drawTextScreen(310, 16 * 3, "Workers building: %d", getNumWorkersBuilding());
}

void WorkerManager::handleRefineryWorkers()
{
	for (auto& refineryExploited : _assignedWorkersToGas) {
		Unit refinery = refineryExploited.first;
		int workersAssigned = refineryExploited.second;
		for (int i = workersAssigned; i < WORKERS_PER_REFINERY; ++i) {
			setJobAndExecute(State::Gathering_Gas, nullptr, refinery);
// 			Broodwar << "Sending worker to extract gas" << std::endl;
		}
	}
}

void WorkerManager::handleBuildingsIncompleted()
{
	for (const auto& myUnit : Broodwar->self()->getUnits()) {
		if (!myUnit->isCompleted() && myUnit->getType().isBuilding() && !myUnit->getBuildUnit() &&
			!isAnyWorkerBuilding(myUnit->getTilePosition(), myUnit->getType())) {
			// only finish the building if there are not enemies near
			Unitset enemies = myUnit->getUnitsInRadius(390, Filter::IsEnemy);
			if (enemies.empty()) {
				setJobAndExecute(State::Building, nullptr, myUnit, myUnit->getTilePosition(), myUnit->getType());
// 				Broodwar << "SCV sent to finish building at frame " << Broodwar->getFrameCount() << std::endl;
			}
		}
	}
}

void WorkerManager::handleBuildingsDamaged()
{
	BWAPI::Unitset damagedBuildings;
	for (const auto& unit : Broodwar->self()->getUnits()) {
		if (unit->isCompleted() && unit->getType().isBuilding() &&
			unit->getType() != UnitTypes::Terran_Bunker && // Bunkers have their own SCV assigned
			unit->getHitPoints() != unit->getType().maxHitPoints()) {
			damagedBuildings.insert(unit);
		}
	}

	for (const auto& building : damagedBuildings) {
		bool isBeingRepared = false;
		// is building part of the wall?
		bool damagedWall = false;
		if (building->getDistance(BWAPI::Position(wallGenerator->BarracksWall)) <= 256) damagedWall = true;
		// are enemies near?
		Unitset enemies = building->getUnitsInRadius(390, Filter::IsEnemy);

		if (enemies.empty() || damagedWall) {
			// if no worker repairing
			for (const auto& workerData : _workers) {
				if (workerData->target == building && workerData->state == State::Repairing) {
					isBeingRepared = true;
					break;
				}
			}
			if (!isBeingRepared) setJobAndExecute(State::Repairing, nullptr, building);
		}
	}
}

bool WorkerManager::needWorkers()
{
	if (informationManager->_maxWorkersMining == 0) {
		return ((_assignedWorkersToMinerals.size() * WORKERS_PER_MINERAL) > getNumWorkersMining());
	} else {
		return (informationManager->_maxWorkersMining > getNumWorkersMining());
	}
}

bool WorkerManager::isUnderConstruction(UnitType type)
{
	for (const auto& workerData : _workers) {
		if (workerData->worker->getOrder() == Orders::ConstructingBuilding && 
			workerData->worker->getBuildUnit()->getType() == type) {
			return true;
		}
	}
	return false;
}

bool WorkerManager::isBuildPositionVisible(TilePosition buildPosition, UnitType buildType)
{
	return (
		Broodwar->isVisible(buildPosition) &&
		Broodwar->isVisible(buildPosition.x + buildType.tileWidth(), buildPosition.y) &&
		Broodwar->isVisible(buildPosition.x, buildPosition.y + buildType.tileHeight()) &&
		Broodwar->isVisible(buildPosition.x + buildType.tileWidth(), buildPosition.y + buildType.tileHeight())
		);
}

bool WorkerManager::isAnyWorkerBuilding(BWAPI::TilePosition buildPosition, BWAPI::UnitType buildType)
{
	for (const auto& workerData : _workers) {
		if (workerData->buildPosition == buildPosition && workerData->buildType == buildType) {
			return true;
		}
	}
	return false;
}

void WorkerManager::buildRequest(TilePosition locationToBuild, UnitType buildType)
{
	setJobAndExecute(State::Building, nullptr, nullptr, locationToBuild, buildType);
}

bool WorkerManager::anyWorkerDefending()
{
	for (const auto& workerData : _workers) {
		if (workerData->state == State::Defending) return true;
	}
	return false;
}

bool WorkerManager::anyWorkerAttacking(Unit target)
{
	for (const auto& workerData : _workers) {
		if (workerData->worker->getTarget() == target) return true;
	}
	return false;
}


bool WorkerManager::defenseBase(Unit unit)
{
	return setJobAndExecute(State::Defending, nullptr, unit);
}

void WorkerManager::tryMiningTrick(Unit worker)
{
// 	TilePosition buildPosition = _workerBuildOrder[worker].first;
// 	UnitType buildType = _workerBuildOrder[worker].second;
// 	BWAPI::Unitset UnitsInRange;
// 	if (buildType == UnitTypes::Terran_Command_Center) {
// 		UnitsInRange = worker->getUnitsInRadius(5*TILE_SIZE);
// 		int minerals = 0;
// 		for(BWAPI::Unitset::iterator i=UnitsInRange.begin();i!=UnitsInRange.end();++i) {
// 			if ( (*i)->getType().isMineralField() ) minerals++;
// 		}
// 		//Broodwar->printf("Minerals in range %d", minerals);
// 		if (minerals > 2) return;
// 	} else {
// 		UnitsInRange = Broodwar->getUnitsInRadius(Position(buildPosition), 7*TILE_SIZE);
// 	}
// 	for(BWAPI::Unitset::iterator i=UnitsInRange.begin();i!=UnitsInRange.end();++i) {
// 		if ( (*i)->getType().isMineralField() && buildPosition.getDistance((*i)->getTilePosition()) < buildPosition.getDistance(worker->getTilePosition()) &&
// 			(*i)->getTilePosition().getDistance(worker->getTilePosition()) > 2 ) {
// 				//Broodwar->printf("Mineral distance: %0.2f", (*i)->getTilePosition().getDistance(worker->getTilePosition()));
// 			worker->rightClick(*i);
// 		}
// 	}
}

void WorkerManager::scoutPosition(BWAPI::Position position)
{
	setJobAndExecute(State::Scouting, nullptr, nullptr, TilePosition(position));
}

void WorkerManager::attackBuilding(BWAPI::Unit target)
{
	// By default we defend from buildings with 2 workers
	setJobAndExecute(State::Defending, nullptr, target);
	setJobAndExecute(State::Defending, nullptr, target);
}
