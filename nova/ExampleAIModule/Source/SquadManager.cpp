#include "SquadManager.h"

using namespace BWAPI;

#define MERGE_DISTANCE 300
#define AUTO_MERGE_DISTANCE 385

SquadManager::SquadManager()
	: _creatingSquad(nullptr),
	_leadSquad(nullptr),
	_searchSquad(nullptr),
	_detectorSquad(nullptr),
	_harassSquad(nullptr),
	_logger(log4cxx::Logger::getLogger("SquadManager"))
{
	if (HighLevelChangeRateExperiment) {
		stats.lastFrameStateChange = Broodwar->getFrameCount();
		AbstractLayer search(_squads); // import current game state to informationManager->gameState
		informationManager->lastGameState = informationManager->gameState;
	}
};

SquadManager::~SquadManager()
{
	for (auto& squad : _squads) {
		delete squad;
	}
	_squads.clear();
}

// this is called only in micro maps TODO needs review
void SquadManager::createNewSquad(const BWAPI::Unitset &units) 
{
	SquadAgent *newSquad = new SquadAgent();
	for(BWAPI::Unitset::iterator i=units.begin();i!=units.end();++i) {
		newSquad->addUnit(*i);
		_unitToSquadMap[*i] = newSquad;
	}
	//Position targetPosition = getBestTarget();
	Position targetPosition = informationManager->_enemyStartPosition;
	newSquad->orderGetPosition(targetPosition);
	_squads.push_back(newSquad);
	// if it is first squad, check _seenEnemies
	if (_squads.size()==1) {
		if ( !informationManager->seenEnemies.empty()) {
			for (auto enemyIter = informationManager->seenEnemies.begin(); enemyIter != informationManager->seenEnemies.end();) {
				Unit enemy = enemyIter->first;
				if (enemy->isVisible()) {
					newSquad->_enemies.insert(enemy);
					_enemyToSquadMap[enemy] = newSquad;
					newSquad->insertEnemyThreat(enemy);
					enemyIter = informationManager->deleteSeenEnemy(enemyIter);
				} else {
					++enemyIter;
				}
			}
		}
	}
}

SquadAgent* SquadManager::createNewSquadOfType(SquadAgent::Order type)
{
	SquadAgent* newSquad = new SquadAgent();
	newSquad->_state = type;
	_squads.push_back(newSquad);
	return newSquad;
}

void SquadManager::addUnitToSquad(SquadAgent* squad, Unit unitToAdd)
{
	squad->addUnit(unitToAdd);
	_unitToSquadMap[unitToAdd] = squad;
}

void SquadManager::addUnitToSquad(Unit unit) 
{
	// check if we have an "under construction" squad
	if (_creatingSquad == nullptr) {
		LOG4CXX_TRACE(_logger, "Creating new squad");
		_creatingSquad = createNewSquadOfType(SquadAgent::Idle);
		if (_leadSquad == nullptr) _leadSquad = _creatingSquad;
	}

	addUnitToSquad(_creatingSquad, unit);

	// if it is first squad, check _seenEnemies
	if (_squads.size() == 1) {
		if ( !informationManager->seenEnemies.empty() ) {
			LOG4CXX_TRACE(_logger, "Assigning seen enemies to new squad if they are visible");
			for (auto enemy = informationManager->seenEnemies.begin(); enemy != informationManager->seenEnemies.end();) {
				if (enemy->first->isVisible()) {
					_creatingSquad->_enemies.insert(enemy->first);
					_enemyToSquadMap[enemy->first] = _creatingSquad;
					_creatingSquad->insertEnemyThreat(enemy->first);
					enemy = informationManager->deleteSeenEnemy(enemy);
				} else {
					++enemy;
				}
			}
		}
	}

	// check squad complete
	if (_creatingSquad->_squadUnits.size() >= informationManager->_minSquadSize || normalSquads() > 1 ) { 
		LOG4CXX_TRACE(_logger, "Squad completed with " << _creatingSquad->_squadUnits.size() << " units");
		_creatingSquad->_waitingNewUnits = false;
		Position targetPosition = getBestTarget();
		_creatingSquad->orderGetPosition(targetPosition);
		_creatingSquad = nullptr;
	}
}

int SquadManager::normalSquads()
{
	int squadsSize = 0;
	for (auto & squad : _squads) {
		if (!isSpecialSquad(squad)) squadsSize++;
	}
	return squadsSize;
}

void SquadManager::newBunkerSquad(Unit bunker)
{
	SquadAgent* bunkerSquad = createNewSquadOfType(SquadAgent::Bunker);
	addUnitToSquad(bunkerSquad, bunker);

	// Each bunker has one SCV to repair it
	Unit scv = workerManager->getWorkerForTask(bunker->getPosition());
	if (scv != nullptr) {
		addUnitToSquad(bunkerSquad, scv);
		workerManager->onUnitDestroy(scv);
	} else LOG4CXX_ERROR(_logger, "No worker to send to bunker squad");
	
}

void SquadManager::newScoutSquad(BWAPI::Unit unit)
{
	SquadAgent* scoutSquad = createNewSquadOfType(SquadAgent::Scout);
	addUnitToSquad(scoutSquad, unit);

	// send scout to best scout position
	scoutSquad->_positionTarget = getScoutTarget();
}

void SquadManager::newDetectorSquad(BWAPI::Unit unit)
{
	SquadAgent* detectorSquad = createNewSquadOfType(SquadAgent::Detector);
	addUnitToSquad(detectorSquad, unit);

	// pointer to keep adding detectors to this squad
	_detectorSquad = detectorSquad;

	//detectorSquad->_positionTarget = getScoutTarget();
}

void SquadManager::newHarassSquad(BWAPI::Unit unit)
{
	SquadAgent* harassSquad = createNewSquadOfType(SquadAgent::Harass);
	addUnitToSquad(harassSquad, unit);

	// pointer to keep adding harass units to this squad
	_harassSquad = harassSquad;

	// TODO: warning, this only works if we know the enemy start position (maps for 2 players)
	_harassSquad->_positionTarget = informationManager->_enemyStartPosition;
	unit->move(_harassSquad->_positionTarget);
}

void SquadManager::unitTraining(Unit unit)
{
	_unitsTraning.insert(unit);
}

void SquadManager::onFrame() 
{
	// sanitize squad
	_leadSquad = getBestSquadLead();

	if (_leadSquad != nullptr && _leadSquad->squadNeedSCV() ) {
		Unit scv = workerManager->getWorkerForTask(_leadSquad->_center);
		if (scv != nullptr) {
			addUnitToSquad(_leadSquad, scv);
			workerManager->onUnitDestroy(scv);
		} else LOG4CXX_ERROR(_logger, "No worker to send to squad lead");
	}

	// check if wee need a search squad
	// TODO only check this once when _sarchAndDestroy goes to true, and then after each Wraith creation
	if (informationManager->_searchAndDestroy && _searchSquad == nullptr) {
		// search for one Wraith
		bool wraithFound = false;
		for (const auto& squad : _squads) {
			if (_searchSquad != nullptr) break;
			for (const auto& unit : squad->_squadUnits) {
				Unit wraith = unit->_unit;
				if (wraith->getType() == UnitTypes::Terran_Wraith) {
					onUnitDestroy(wraith); // remove it from its squad to ... 
					//creat its own "search" squad
					SquadAgent* searchSquad = createNewSquadOfType(SquadAgent::Search);
					addUnitToSquad(searchSquad, wraith);
					_searchSquad = searchSquad;
					// order to search
					Position corner = Position(192,192); // send to top-left corner
					searchSquad->_positionTarget = corner;
					wraith->attack(corner);
					wraithFound = true;
					break;
				}
			}
			if (wraithFound) break;
		}
	}

	if (!ONLY_MICRO) {
		// check for units training ready
		for(BWAPI::Unitset::iterator unit=_unitsTraning.begin();unit!=_unitsTraning.end();) {
			if ((*unit)->getOrder() != Orders::Nothing) {
				if ((*unit)->getType() == UnitTypes::Terran_Science_Vessel) {
					if (_detectorSquad != nullptr) addUnitToSquad(_detectorSquad, *unit);
					else newDetectorSquad(*unit);
				} else if (informationManager->_harassing) {
					if (_harassSquad != nullptr) addUnitToSquad(_harassSquad, *unit);
					else newHarassSquad(*unit);
				} else {
					addUnitToSquad(*unit);
				}
				unit = _unitsTraning.erase( unit );
			} else {
				++unit;
			}
		}
	}

#ifndef TOURNAMENT
	int squadsMerging = 0;
	for (const auto& squad : _squads) {
		if (squad->_state == SquadAgent::MergeSquads) {
			++squadsMerging;
			if (squad != squad->_squadToMerge->_squadToMerge)
				LOG4CXX_WARN(_logger, "BAD LINK " << squad->_squadToMerge << " <-> " << squad);
		}
	}
#endif

	// auto-merge
	if (_squads.size() > 1) {
		LOG4CXX_TRACE(_logger, "CHECK AUTO-MERGING (START)");
		checkAutoMerge();
		LOG4CXX_TRACE(_logger, "CHECK AUTO-MERGING (END)");
	}

	// merge squads
	if (_squadsToMerge.size() > 0) {
		LOG4CXX_TRACE(_logger, "CHECK MERGING (START)");
		checkMerge();
		LOG4CXX_TRACE(_logger, "CHECK MERGING (END)");
	}

	// update each squad
#ifndef TOURNAMENT
	int line = 0;
	char squadColor;
#endif

    if (HIGH_LEVEL_SEARCH) {
        if (informationManager->mapAnalyzed && (Broodwar->getFrameCount() % HIGH_LEVEL_REFRESH == 0)&& !Broodwar->isPaused()) {
            LOG4CXX_TRACE(_logger, "CHECK HIGH LEVEL ORDER (START)");
            checkHighLevelOrder();
            LOG4CXX_TRACE(_logger, "CHECK HIGH LEVEL ORDER (END)");
        }
    }


	if (HighLevelChangeRateExperiment) {
		AbstractLayer search(_squads); // import current game state to informationManager->gameState

		// compare game states
		int misplacedUnits = 0;
		int totalUnits = 0;
		informationManager->gameState.compareFriendlyUnits(informationManager->lastGameState, misplacedUnits, totalUnits);
		
		if (misplacedUnits > 0) {
			// save frame interval change
			//LOG("STATE CHANGED AFTER " << (Broodwar->getFrameCount() - stats.lastFrameStateChange) << " FRAMES");
			stats.stateChange[Broodwar->getFrameCount() - stats.lastFrameStateChange]++;

			// reset frame and game state
			stats.lastFrameStateChange = Broodwar->getFrameCount();
			informationManager->lastGameState = informationManager->gameState;
		}
	}


	LOG4CXX_TRACE(_logger, "UPDATE SQUADS (START)");
	for(const auto& squad : _squads) {
		LOG4CXX_TRACE(_logger, "UPDATE squad (" << squad << ") status: " << squad->getState().c_str());
		squad->onFrame();

		// Draw stats
#ifndef TOURNAMENT
			squadColor = '\x02'; // Light Blue
			if (isSpecialSquad(squad)) squadColor = '\x0F'; //Teal
			if (squad == _leadSquad) squadColor = '\x11'; //Orange
			Broodwar->drawTextScreen(5,13*line,"%cunits(%d) enemies(%d) [%s] %s", squadColor,
									squad->_squadUnits.size(), squad->_enemies.size(), squad->getState().c_str(), squad->_waitingReason.c_str() );
			++line;
// 			if (squad->_enemyStats.airHP) {
// 				Broodwar->drawTextScreen(10, 13 * line, "EnemyAirHP %2.f vs SquadAirDPS %2.f", squad->_enemyStats.airHP, squad->_squadStats.airDPS);
// 				++line;
// 			}
// 			if (squad->_enemyStats.groundHP) {
// 				Broodwar->drawTextScreen(10,13*line,"EnemyGroundHP %2.f vs SquadGroundDPS %2.f",squad->_enemyStats.groundHP,squad->_squadStats.groundDPS);
// 				++line;
// 			}
// 			if (squad->_squadStats.airHP) {
// 				Broodwar->drawTextScreen(10,13*line,"SquadAirHP %2.f vs EnemyAirDPS %2.f",squad->_squadStats.airHP,squad->_enemyStats.airDPS);
// 				++line;
// 			}
// 			if (squad->_squadStats.groundHP) {
// 				Broodwar->drawTextScreen(10,13*line,"SquadGroundHP %2.f vs EnemyGroundDPS %2.f",squad->_squadStats.groundHP,squad->_enemyStats.groundDPS);
// 				++line;
// 			}
// 			
// 			for (auto enemy : squad->_enemies) {
// 				if (enemy->getType() == UnitTypes::Unknown) {
// 					std::string typeName = "";
// 					auto it1 = informationManager->visibleEnemies.find(enemy);
// 					if (it1 != informationManager->visibleEnemies.end()) typeName = it1->second.type.getName();
// 					auto it2 = informationManager->seenEnemies.find(enemy);
// 					if (it2 != informationManager->seenEnemies.end()) typeName = it2->second.type.getName();
// 					Broodwar->drawTextScreen(10, 13 * line, "Enemy: %s <-> %s", enemy->getType().getName().c_str(), typeName.c_str());
// 					++line;
// 				}
// 			}
#endif
	}
	LOG4CXX_TRACE(_logger, "UPDATE SQUADS (END)");
}

void SquadManager::checkHighLevelOrder()
{
	AbstractLayer search(_squads); // import current game state to informationManager->gameState

    //search.printBranchingStats();

	if (search.hasFriendlyUnits()) {
		// perform high level search
		std::map<SquadAgent*, BWAPI::Position> bestOrders = search.searchBestOrders();

		// check if we have new orders
		Position bestPosition;
		Position squadtPosition;
		for (const auto& squad : _squads) {
			stats.orders++;
			bestPosition = bestOrders[squad];
			squadtPosition = squad->_positionTarget;
			if (bestPosition != squadtPosition) {
				stats.ordersOverwritten++;
				// if the new position is closer to home it's a FORCE move

				// otherwise it's an attack move
				//LOG(*squad << " Attack move to new region from " << squadtPosition.x << "," << squadtPosition.y << " to " << bestPosition.x << "," << bestPosition.y);
				squad->orderGetPosition(bestPosition);
			}
		}
		//LOG("EvaluationCurrentState: " << search.getEvaluation() << " myKillScore: " << Broodwar->self()->getKillScore() << " enemyKillScore: " << Broodwar->enemy()->getKillScore());
	}
#ifdef NOVA_GUI
    informationManager->_GUIsignal->emitGameStateChanged();
#endif
}

void SquadManager::checkAutoMerge()
{
	SquadAgent *squad1;
	SquadAgent *squad2;
	for (auto it = _squads.begin(); it != _squads.end();) {
		squad2 = *it;
		if (_creatingSquad == squad2) { ++it; continue; }
		if ( squad2->_state == SquadAgent::GetPosition) { 
			squad1 = getClosestSquad(squad2->_center, squad2, true);
			if ( squad1 != nullptr && _creatingSquad != squad1 && squad1->_positionTarget == squad2->_positionTarget) { 
				// TODO improve auto-merge conditions
				int distance = squad2->_center.getApproxDistance(squad1->_center);
				BWTA::Region* squad2Region = BWTA::getRegion(TilePosition(squad2->_center));
				if (distance < AUTO_MERGE_DISTANCE) {
				//if (distance < AUTO_MERGE_DISTANCE || (!squad1->isSquadBio() && !squad2->isSquadBio()) ) {
					// do unobtrusive merge
					LOG4CXX_DEBUG(_logger, "Unobtrusive Merge " << squad1 << " <-> " << squad2);
					if (squad1->_state == SquadAgent::MergeSquads) {
						for(CombatUnitSet::const_iterator i=squad2->_squadUnits.begin();i!=squad2->_squadUnits.end();++i) {
							if ((*i)->_unit->getType() == UnitTypes::Terran_Dropship || (*i)->_unit->getType() == UnitTypes::Terran_Science_Vessel) continue; //ignore special micro
							(*i)->_unit->move(squad1->_positionToMerge);
						}
					} else if (squad1->_state == SquadAgent::HoldCenter) {
						// if we auto-merge with a squad holding center, order to move to hold position
						for(CombatUnitSet::const_iterator i=squad2->_squadUnits.begin();i!=squad2->_squadUnits.end();++i) {
							(*i)->_unit->move(squad1->_holdCenterPosition);
						}
					}
					mergeUnitsAndEnemies(squad1, squad2);
					// delete old squad
					LOG4CXX_DEBUG(_logger, "Delete squad " << squad2);
					it = _squads.erase(it);
					if (_leadSquad == squad2) {
						// find another candidate for squad lead
						_leadSquad = getBestSquadLead();
					}
					delete squad2; squad2 = 0;
					continue;
				}
			}
		}
		++it;
	}
}

void SquadManager::checkMerge()
{
	SquadAgent *squad1;
	SquadAgent *squad2;
	int distance;
	
	for(UnitPairSet::const_iterator squadPair=_squadsToMerge.begin();squadPair!=_squadsToMerge.end();) {
		squad1 = squadPair->first;
		squad2 = squadPair->second;
		LOG4CXX_TRACE(_logger, "Merge " << squad1 << " <-> " << squad2);
		distance = squad1->_center.getApproxDistance(squad2->_center);
		//distance = BWTA::getGroundDistance(TilePosition(squad1->_center.x,squad1->_center.y), TilePosition(squad2->_center.x,squad2->_center.y));
		//Broodwar->drawTextScreen(290,16,"Merge distance: %d", distance);
		Broodwar->drawLine(CoordinateType::Map, squad1->_center.x, squad1->_center.y, squad2->_center.x, squad2->_center.y, Colors::Orange);
		if (distance < MERGE_DISTANCE) {
			mergeUnitsAndEnemies(squad1, squad2);
			//check if any other squad want to merge to squad2 
			bool newOrder = true;
			for(UnitPairSet::iterator oldSquadPair=_squadsToMerge.begin();oldSquadPair!=_squadsToMerge.end();++oldSquadPair) {
				if (oldSquadPair->first  == squad1) continue;
				if (oldSquadPair->second == squad2) {
					oldSquadPair->second = squad1;
					newOrder = false;
					oldSquadPair->first->inMerge(squad1);
					squad1->inMerge(oldSquadPair->first);
				} else if (oldSquadPair->first == squad2) {
					oldSquadPair->first = squad1;
					newOrder = false;
					oldSquadPair->second->inMerge(squad1);
					squad1->inMerge(oldSquadPair->second);
				}
			}
#ifndef TOURNAMENT
			for (const auto& squad : _squads) {
				if (squad == squad1) continue;
				if (squad->_squadToMerge == squad2) {
					LOG4CXX_ERROR(_logger, "Squad " << squad << " want a merge with deleted " << squad2);
				}
			}
#endif
			// delete old squad
			LOG4CXX_TRACE(_logger, "Delete squad " << squad2);
			_squads.erase(std::remove(_squads.begin(), _squads.end(), squad2), _squads.end()); // find and remove
			if (_leadSquad == squad2) { // find another candidate for squad lead
				_leadSquad = getBestSquadLead();
			}
			delete squad2; squad2 = nullptr;
			//check if any other squad want to merge to squad1
			for (const auto& squad : _squads) {
				if (squad == squad1) continue;
				if (squad->_squadToMerge == squad1) {
					LOG4CXX_TRACE(_logger, "Readjust Merge " << squad1 << " <-> " << squad);
					newOrder = false;
					squad1->inMerge(squad);
				}
			}
			if (newOrder) { // else default behavior
				squad1->_state = SquadAgent::Idle;
				squad1->_squadToMerge = 0;
                //LOG("Target after merge (" << squad1->_positionTarget.x << "," << squad1->_positionTarget.y << ")");
				squad1->orderGetPosition(squad1->_positionTarget); // new order TODO default behavior?
			}
			// delete merge operation
			squadPair = _squadsToMerge.erase(squadPair);
		} else {
			++squadPair;
		}
	}
}

void SquadManager::mergeUnitsAndEnemies(SquadAgent* squad1, SquadAgent* squad2)
{
	// merge units
	for (const auto& combatUnit : squad2->_squadUnits) {
		squad1->addUnit(combatUnit->_unit);
		_unitToSquadMap.erase(combatUnit->_unit);
		_unitToSquadMap.insert(UnitToSquadMap::value_type(combatUnit->_unit, squad1));
	}
	// merge enemies
	for (const auto& enemy : squad2->_enemies) {
		squad1->_enemies.insert(enemy);
		squad1->insertEnemyThreat(enemy);
		_enemyToSquadMap[enemy] = squad1;
	}
}

// void SquadManager::orderGetPosition(Position positionTarget) 
// {
// 	_positionTarget = positionTarget;
// 	for(SquadSet::const_iterator squad=_squads.begin();squad!=_squads.end();++squad) {
// 		(*squad)->orderGetPosition(positionTarget);
// 	}
// }

void SquadManager::onUnitDestroy(Unit unit)
{
	UnitToSquadMap::iterator found = _unitToSquadMap.find(unit);
	if (found != _unitToSquadMap.end()) {
		SquadAgent *squad = found->second;
		if (squad == 0) {
			LOG4CXX_ERROR(_logger, "SQUAD in _unitToSquadMap entry was null");
			return;
		}
		squad->onUnitDestroy(unit);
		_unitToSquadMap.erase(found);
		// if no more units on that squad, remove squad
		if (squad->_squadUnits.empty()) {
			// TODO create a method to check "special" squads onDelete
			if (_creatingSquad == squad) _creatingSquad = nullptr;
			if (_detectorSquad == squad) _detectorSquad = nullptr;
			if (_harassSquad == squad) _harassSquad = nullptr;
			if (_searchSquad == squad) _searchSquad = nullptr;
			// reassign enemies
			for (const auto& enemy : squad->_enemies) {
				if (enemy->isVisible() ) {
// 					onEnemyEvade(enemy);
					_enemyToSquadMap.erase(enemy);
					informationManager->markEnemyAsSeen(enemy); // TODO this should be a list of visible but not assigned
 					newEnemy(enemy, squad);
				} else {
					LOG4CXX_ERROR(_logger, "REASSIGN ENEMY LOST !!!! " << enemy->getType());
				}
			}
			removeMergingSquads(squad);
			_squads.erase(std::remove(_squads.begin(), _squads.end(), squad), _squads.end()); // find and remove
			LOG4CXX_TRACE(_logger, "Squad deleted (" << squad << ")");
			//DEBUG("Squad deleted (" << squad << ")");
			if (_leadSquad == squad) {
				// find another candidate for squad lead
				_leadSquad = getBestSquadLead();
				//informationManager->_minSquadSize += 2; // TODO use an evaluation function to do this
				informationManager->_minSquadSize = squad->_maxEnemyUnitsSeen + std::abs((int)squad->_maxEnemyUnitsSeen - (int)informationManager->_minSquadSize);
			}
			delete squad;
		}
	} else {
		// remove from _unitsTraning
		BWAPI::Unitset::iterator foundInTraining = _unitsTraning.find(unit);
		if (foundInTraining != _unitsTraning.end()) {
			LOG4CXX_TRACE(_logger, "Unit (" << unit << ") removed from _unitsTraning");
			_unitsTraning.erase(unit);
		} else {
			LOG4CXX_ERROR(_logger, "Squad Unit (" << unit << ") not found [" << unit->getType() << "]");
		}
	}
}

SquadAgent* SquadManager::getBestSquadLead()
{
	int minDistance = 9999999;
	unsigned int maxSize = 0;
	SquadAgent *bestSquad = nullptr;
	for (const auto& squad : _squads) {
		if (isSpecialSquad(squad)) continue;
		int dist = squad->_center.getApproxDistance(informationManager->_enemyStartPosition);
		if (dist < minDistance || squad->_squadUnits.size() > maxSize) {
			minDistance = dist;
			maxSize = squad->_squadUnits.size();
			bestSquad = squad;
		}
	}
	return bestSquad;
}

bool SquadManager::isSpecialSquad(SquadAgent* squad)
{
	if ( isSquadNoMergeable(squad) || isSquadNoEnemyAssignable(squad) ) return true;
	return false;
}

bool SquadManager::isSquadNoMergeable(SquadAgent* squad)
{
	if (squad->_state == SquadAgent::Bunker ||
		squad->_state == SquadAgent::Search ||
		squad->_state == SquadAgent::Scout ||
		squad->_state == SquadAgent::Detector ||
		squad->_state == SquadAgent::Harass) return true;
	return false;
}

bool SquadManager::isSquadNoEnemyAssignable(SquadAgent* squad)
{
	if (squad->_state == SquadAgent::Bunker ||
		squad->_state == SquadAgent::Detector) return true;
	return false;
}


int SquadManager::getUnitFrameCreated(Unit unit)
{
	UnitToSquadMap::iterator found = _unitToSquadMap.find(unit);
	if(found != _unitToSquadMap.end()) {
		SquadAgent *squad = found->second;
		if (squad == 0) {
			LOG4CXX_ERROR(_logger, "SQUAD in _unitToSquadMap entry was null");
			return Broodwar->getFrameCount();
		}
		 return squad->getUnitFrameCreated(unit);
		_unitToSquadMap.erase(found);
	} else {
		LOG4CXX_ERROR(_logger, "Unit not found in _unitToSquadMap");
		return Broodwar->getFrameCount();
	}
}

void SquadManager::newEnemy(Unit enemy, SquadAgent* oldSquad)
{
	informationManager->markEnemyAsVisible(enemy);

	// try to assign enemy to a squad
	if (!_squads.empty()) {
		// Search the closest squad
		SquadAgent *bestSquad = getClosestSquad(enemy->getPosition(), oldSquad, false, true);
		if (bestSquad != nullptr) {
			// TODO encapsulate this in a function addEnemy ********
			bestSquad->_enemies.insert(enemy);
			if (bestSquad->_maxEnemyUnitsSeen < bestSquad->_enemies.size()) {
				bestSquad->_maxEnemyUnitsSeen = bestSquad->_enemies.size();
			}
			bestSquad->insertEnemyThreat(enemy);
			_enemyToSquadMap[enemy] = bestSquad;
			informationManager->markEnemyAsVisible(enemy);
			return;
		}
	}
	
	// if we don't have squad mark enemy as seen
	// TODO keep another list for units visible but not assigned
	informationManager->markEnemyAsSeen(enemy);
	LOG4CXX_INFO(_logger, "Unit discover but we don't have squads...");
}

void SquadManager::onEnemyEvade(Unit enemy)
{
	informationManager->markEnemyAsSeen(enemy);

	// remove from squad target (if it was assigned)
	UnitToSquadMap::iterator found = _enemyToSquadMap.find(enemy);
	if (found != _enemyToSquadMap.end()) {
		SquadAgent *squad = found->second;
		_enemyToSquadMap.erase(found);	
		if (squad != nullptr) {
			squad->_enemies.erase(enemy);
			squad->removeEnemyThreat(enemy);
			if (squad->_enemies.size() == 0 && squad->_waitingNewUnits == false)
				squad->orderGetPosition(squad->_positionTarget); // new order. TODO: default behavior?
		} else {
			LOG4CXX_ERROR(_logger, "SQUAD in enemyToSquadMap entry was null");
		}
	}
}

void SquadManager::onEnemyDestroy(Unit enemy)
{
	size_t del1 = informationManager->visibleEnemies.erase(enemy);
	size_t del2 = informationManager->seenEnemies.erase(enemy);

	if (del1 + del2 == 0) {
		LOG4CXX_ERROR(_logger, "Enemy (" << enemy << ") type:" << enemy->getType().getName() << " not found");
	}
}

void SquadManager::requestRetreat(SquadAgent* squad)
{
	//DEBUG("REQUEST RETREAT");
	 //if no more squads deny retreat
	if (_squads.size() == 1) {
		LOG4CXX_TRACE(_logger,"[Squad Retreat] deny (only 1 squad)");
		squad->inCombat(); 
		return;
	}

	 //if there are tanks deny retreat
	if (squad->hasUnitOfType(UnitTypes::Terran_Siege_Tank_Tank_Mode) || squad->hasUnitOfType(UnitTypes::Terran_Siege_Tank_Siege_Mode) ) {
		LOG4CXX_TRACE(_logger,"[Squad Retreat] deny (tank present)");
		squad->inCombat(); 
		return;
	}
		
	 //merge with closest squad
	SquadAgent *bestSquad = getClosestSquad(squad->_center, squad, true);
	if (bestSquad != nullptr) {
		LOG4CXX_TRACE(_logger,"[Squad Retreat] OK");
		squad->inMerge(bestSquad);
		LOG4CXX_TRACE(_logger,"[Squad Retreat] Squad 1 done");
		bestSquad->inMerge(squad, squad->_positionToMerge);
		LOG4CXX_TRACE(_logger,"[Squad Retreat] Squad 2 done");
		_squadsToMerge.push_back(std::make_pair(squad, bestSquad));
		LOG4CXX_TRACE(_logger, "[Squad Retreat] Pair updated (" << squad << ") <-> (" << bestSquad << ")");
		// TODO do we need to check other special squad deletion?
		if (_creatingSquad == bestSquad)
			_creatingSquad = nullptr;
		return;
	}
	
	LOG4CXX_TRACE(_logger, "[Squad Retreat] deny (no squads near)");
}

void SquadManager::removeMergingSquads(SquadAgent *squad) {
	SquadAgent *squadStillAlive;
	SquadAgent *squadToMerge = nullptr;
	for(UnitPairSet::const_iterator squadPair=_squadsToMerge.begin();squadPair!=_squadsToMerge.end();) {
		//LOG4CXX_TRACE(_logger, "link1 (" << squadPair->first << ") <-> (" << squadPair->second << ")");
		squadStillAlive = nullptr;
		if (squad == squadPair->first) {
			squadStillAlive = squadPair->second;
		} else if (squad == squadPair->second) {
			squadStillAlive = squadPair->first;
		}

		if (squadStillAlive != nullptr) {
			LOG4CXX_TRACE(_logger, "Remove merge link (" << squadPair->first << ") <-> (" << squadPair->second << ")");
			squadPair = _squadsToMerge.erase(squadPair);
			// Maybe we want to merge to another
			squadToMerge = nullptr;
			for (auto squadPair2 : _squadsToMerge) {
				if (squadStillAlive == squadPair2.first) {
					squadToMerge = squadPair2.second;
					break;
				}
			}
			if (squadToMerge != nullptr) {
				squadStillAlive->_squadToMerge = squadToMerge;
				LOG4CXX_TRACE(_logger, "Already merge link (" << squadStillAlive << ") <-> (" << squadToMerge << ")");
			} else {
				// Or another squad wants to merge us
				squadToMerge = nullptr;
				for (auto squadPair2 : _squadsToMerge) {
					//LOG4CXX_TRACE(_logger, "link2 (" << squadPair2.first << ") <-> (" << squadPair2.second << ")");
					if (squadStillAlive == squadPair2.second) {
						squadToMerge = squadPair2.first;
						break;
					}
				}
				if (squadToMerge != nullptr) {
					squadStillAlive->_squadToMerge = squadToMerge;
					LOG4CXX_TRACE(_logger, "Already merge link (" << squadToMerge << ") <-> (" << squadStillAlive << ")");
				} else {
					// Otherwise move to positionTarget
					squadStillAlive->_state = SquadAgent::Idle;
					squadStillAlive->_squadToMerge = 0;
					squadStillAlive->orderGetPosition(squadStillAlive->_positionTarget);
					LOG4CXX_TRACE(_logger, "Remvoing (" << squadStillAlive << ") to merge list");
				}
			}
			
		} else {
			++squadPair;
		}
	}
}

SquadAgent* SquadManager::getClosestSquad(Position toPosition, SquadAgent* ignoreSquad, bool ignoreNoMergeable, bool ignoreNoAssignable)
{
	// TODO int distance = std::numeric_limits<int>::max();
	int distance = 9999999;
	int newDistance;
	SquadAgent *bestSquad = nullptr;
	for (auto squad : _squads) {
		if (squad == ignoreSquad) continue;
		if (ignoreNoMergeable && isSquadNoMergeable(squad)) continue;
		if (ignoreNoAssignable && isSquadNoEnemyAssignable(squad)) continue;
		newDistance = squad->_center.getApproxDistance(toPosition);
		if (newDistance < distance) {
			distance = newDistance;
			bestSquad = squad;
		}
	}
	return bestSquad;
}

Position SquadManager::getBestTarget()
{
// 	if (!informationManager->_firstPush) {
// 		return informationManager->_initialRallyPosition;
// 	}

	if (informationManager->_enemyBases.size() == 0) {
		return informationManager->_enemyStartPosition;
	}

	if (informationManager->_enemyBases.size() == 1) {
		return (*informationManager->_enemyBases.begin())->getPosition();
	}

	// Ratio units per possible target
	std::map<Position, int> unitsPerTarget;

	// add enemy bases
	for(std::set<BWTA::BaseLocation*>::const_iterator i=informationManager->_enemyBases.begin();i!=informationManager->_enemyBases.end();++i) {
		Position p=(*i)->getPosition();
		unitsPerTarget[p] = 0;
		// Prioritize main base
		if ( p == informationManager->_enemyStartPosition ) {
			unitsPerTarget[p] -= informationManager->_minSquadSize*2;
		}
	}
	// add our bases under attack
	for(std::map<BWTA::BaseLocation*, BWAPI::TilePosition>::const_iterator i=informationManager->_ourBases.begin();i!=informationManager->_ourBases.end();++i) {
		Position p=i->first->getPosition();
		BWAPI::Unitset UnitsInRange = Broodwar->getUnitsInRadius(p, 350);
		for(BWAPI::Unitset::iterator j=UnitsInRange.begin();j!=UnitsInRange.end();++j) {
			if ( Broodwar->self()->isEnemy((*j)->getPlayer()) && (*j)->getType().canAttack() ) {
				//Broodwar->printf("Addeed defense point");
				unitsPerTarget[(*j)->getPosition()] = informationManager->_minSquadSize*2;
				break;
			}
		}

	}

	// Get target with less units
	size_t minUnits;
	Position bestPosition(Positions::None);
	for (const auto& squad : _squads) {
		size_t unitsAssigned = unitsPerTarget[squad->_positionTarget] + squad->_squadUnits.size();
		unitsPerTarget[squad->_positionTarget] = unitsAssigned;
		if (bestPosition == Positions::None || unitsAssigned < minUnits) {
			bestPosition = squad->_positionTarget;
			minUnits = unitsAssigned;
		}
	}

	return bestPosition;
}

Position SquadManager::getScoutTarget()
{
	if (informationManager->_scoutedAnEnemyBase) {
		return informationManager->_enemyStartPosition;
	} else {
		std::set<BWTA::BaseLocation*>::iterator enemyBase = informationManager->startLocationCouldContainEnemy.begin();
		//TODO sanitize check
		return (*enemyBase)->getPosition();
	}
}
