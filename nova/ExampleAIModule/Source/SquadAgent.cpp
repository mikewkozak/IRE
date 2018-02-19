#include "SquadAgent.h"
#include "SquadManager.h"

using namespace BWAPI;

#define MIN_SPREAD_BASE 30
#define MAX_SPREAD_BASE 50
#define SMALL_SPREAD 2
#define MEDIUM_SPREAD 4
#define LARGE_SPREAD 4

SquadAgent::SquadAgent()
	//: _maxSpread( GetPrivateProfileIntA("squad","spread",50,"bwapi-data\\AI\\novaAI.ini") ),
	//_compacting(false),
	: _movement(SquadAgent::Normal),
	_positionTarget(Positions::None),
	_holdCenterPosition(Positions::None),
	_state(Idle),
	_squadToMerge(0),
	_waitingNewUnits(true),
	_squadMaxSpread(0),
	_logger(log4cxx::Logger::getLogger("SquadManager.SquadAgent"))
{
	_waitingReason = "";
	_squadStats.airDPS = 0;
	_squadStats.groundDPS = 0;
	_squadStats.airHP = 0;
	_squadStats.groundHP = 0;
	_enemyStats.airDPS = 0;
	_enemyStats.groundDPS = 0;
	_enemyStats.airHP = 0;
	_enemyStats.groundHP = 0;

	_maxEnemyUnitsSeen = 0;
};

SquadAgent::~SquadAgent()
{
	for(auto& unit : _squadUnits) {
		delete unit;
	}
	_squadUnits.clear();
}

std::string SquadAgent::getState()
{
	switch (_state) {
		case Idle:			{ return "Idle";		}
		case GetPosition:	{ return "GetPosition"; }
		case Fight:			{ return "Fight";		}
		case MergeSquads:	{ return "MergeSquads"; }
		case Bunker:		{ return "Bunker";		}
		case Search:		{ return "Search";		}
		case Scout:			{ return "Scout";		}
		case Detector:		{ return "Detector";	}
		case HoldCenter:	{ return "Hold Center";	}
		case Harass:		{ return "Harass";		}
	}
	return "InvalidState";
}

void SquadAgent::addUnit( Unit unit ) 
{
	CombatAgent *newUnit = new CombatAgent( unit, this );
	_squadUnits.insert( newUnit );
	_unitToCombatAgentMap.insert( UnitToCombatAgentMap::value_type( unit, newUnit ) );
	if ( unit->getType().size() == UnitSizeTypes::Small ) {
		_squadMaxSpread += SMALL_SPREAD;
	} else if ( unit->getType().size() == UnitSizeTypes::Medium ) {
		_squadMaxSpread += MEDIUM_SPREAD;
	} else if ( unit->getType().size() == UnitSizeTypes::Large ) {
		_squadMaxSpread += LARGE_SPREAD;
	}
	insertSquadThreat( unit );
}

void SquadAgent::addUnits( BWAPI::Unitset units ) 
{
	for( BWAPI::Unitset::iterator i=units.begin(); i!=units.end(); ++i ) {
		addUnit( *i );
	}
}

template<typename Func>
void SquadAgent::updateThreat( BWAPI::Unit unit, stats_t &stats, Func f )
{
	if (!unit->getType().isWorker() && (unit->getType().canAttack() || unit->getType().isDetector()) ) {
		if (unit->getType().airWeapon().damageAmount() > 0 )
			stats.airDPS = f( stats.airDPS, unit->getType().airWeapon().damageAmount()*(24.0/unit->getType().airWeapon().damageCooldown()) );
		if (unit->getType().groundWeapon().damageAmount() > 0 )
			stats.groundDPS = f( stats.groundDPS, unit->getType().groundWeapon().damageAmount()*(24.0/unit->getType().groundWeapon().damageCooldown()) );
		// In the case of Firebats and Zealots, the damage returned by BWAPI is not right, since they have two weapons:
		if (unit->getType() == UnitTypes::Terran_Firebat || unit->getType() == UnitTypes::Protoss_Zealot)
			stats.groundDPS = f( stats.groundDPS, (double)unit->getType().groundWeapon().damageAmount() );
		if (unit->getType().isFlyer())
			stats.airHP = f( stats.airHP, (double)(unit->getType().maxShields() + unit->getType().maxHitPoints()) );
		else
			stats.groundHP = f( stats.groundHP, (double)(unit->getType().maxShields() + unit->getType().maxHitPoints()) );
	}
	// Sanitize
	if (stats.airDPS < 0) stats.airDPS = 0;
	if (stats.groundDPS < 0) stats.groundDPS = 0;
	if (stats.airHP < 0) stats.airHP = 0;
	if (stats.groundHP < 0) stats.groundHP = 0;
}

void SquadAgent::onFrame() 
{
	LOG4CXX_TRACE(_logger,"[onFrame] START (Update Area)");
	squadArea();

	// Some special units have their micro
	for (const auto & u : this->_squadUnits){
		UnitType type(u->_unit->getType());
		if (type == UnitTypes::Terran_Dropship ||
			type == UnitTypes::Terran_Science_Vessel ||
			type == UnitTypes::Terran_SCV ||
			type == UnitTypes::Terran_Bunker) {
			u->defaultBehaviour();
		}
	}

	LOG4CXX_TRACE(_logger,"Check fight state");
	// if we have enemies target it
	bool imposibleAttack = _enemyStats.airHP > 0 && _squadStats.airDPS == 0 && _enemyStats.groundHP == 0;
	if ( _enemies.size() > 0 && !imposibleAttack) {
		//Broodwar->printf("Change to Fight state from %s", getState().c_str());
		switch (_state) {
			case Idle:
			case GetPosition: 
				_state = Fight; 
				break;
		}
	}


	// DEBUG TANK SIEGE CONDITIONS
	/*for(CombatUnitSet::const_iterator i = this->_squadUnits.begin(); i!=this->_squadUnits.end(); ++i) {
		if ((*i)->_unit->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode || (*i)->_unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode) {
			if ((*i)->_siegeState == CombatAgent::NullTarget)
				Broodwar->drawCircleMap((*i)->_unit->getPosition().x, (*i)->_unit->getPosition().y-15,3,Colors::Cyan,true);
			if ((*i)->_siegeState == CombatAgent::Enemies)
				Broodwar->drawCircleMap((*i)->_unit->getPosition().x, (*i)->_unit->getPosition().y-15,3,Colors::Red,true);
			if ((*i)->_siegeState == CombatAgent::NoEnemies)
				Broodwar->drawCircleMap((*i)->_unit->getPosition().x, (*i)->_unit->getPosition().y-15,3,Colors::Green,true);
		}
	}*/
#ifndef TOURNAMENT
	Broodwar->drawLineMap(_center, _positionTarget, Colors::Yellow);
	Broodwar->drawCircleMap(_positionTarget, 3, Colors::Yellow, true);
#endif

	switch (_state) {
		case HoldCenter:
			if (this == squadManager->_leadSquad) holdCenter();
			else _state = GetPosition;
			break;
		case MergeSquads:
			LOG4CXX_TRACE(_logger,"Update MergeSquads");
			inMerge();
			break;
		case GetPosition:
			if (isSquadBio()) {
				LOG4CXX_TRACE(_logger,"Update GetPosition BIO (size: " << _squadUnits.size() << ")");
				for(CombatUnitSet::const_iterator i = this->_squadUnits.begin(); i!=this->_squadUnits.end(); ++i) {
					(*i)->onGetPosition(_positionTarget);
				}
				checkSpread(); // check to compact squad
			} else if (!needWait()) {
				LOG4CXX_TRACE(_logger,"Update GetPosition MECHA");
			    if (!ONLY_MICRO && !HIGH_LEVEL_SEARCH) {
				    /*if (this == squadManager->_leadSquad &&
					    _squadUnits.size() < 30 &&
					    informationManager->home->getCenter().getApproxDistance(_center) > _positionTarget.getApproxDistance(_center)) {
						    // Try to control map center
						    _holdCenterPosition = getClosestUnitTo(_positionTarget)->_unit->getPosition();
						    for(CombatUnitSet::const_iterator i=this->_squadUnits.begin();i!=this->_squadUnits.end();++i) {
							    (*i)->_unit->move(_holdCenterPosition);
						    }
						    _state = HoldCenter;
						    return;
				    }*/
			    }
			    Position actualTarget = _positionTarget;
				if (this != squadManager->_leadSquad && squadManager->_leadSquad != nullptr &&
					_positionTarget == squadManager->_leadSquad->_positionTarget &&
					_center.getApproxDistance(_positionTarget) < squadManager->_leadSquad->_center.getApproxDistance(_positionTarget)) {
					actualTarget = squadManager->_leadSquad->getClosestUnitTo(_positionTarget, UnitTypes::None, true)->_unit->getPosition();
				}
				
				for(const auto& combatUnit : _squadUnits) {
					combatUnit->onGetPosition(actualTarget);
				}
			} else { // we need wait
				for(CombatUnitSet::const_iterator i = this->_squadUnits.begin(); i!=this->_squadUnits.end(); ++i) {
					if (Broodwar->self()->hasResearched(TechTypes::Tank_Siege_Mode) && (*i)->_unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode) {
						if ((*i)->_unit->getOrder() == Orders::PlayerGuard || (*i)->_unit->isHoldingPosition()  ) {
							//Broodwar->printf("Siege tank requested while getPosition (waiting)!!");
							(*i)->onStop();
							//(*i)->siegeRequest();
							//(*i)->_siegeState = CombatAgent::None;
						}
					} else if ((*i)->_unit->getType() == UnitTypes::Terran_Vulture && (*i)->_unit->isMoving() &&
							   (*i)->_unit->getHitPoints()!=(*i)->_unit->getType().maxHitPoints()) {
						(*i)->_unit->stop();
					}
				}
			}

			// check if we have an empty bunker near unit
			if (this == squadManager->_leadSquad) {
				for (const auto & i : _squadUnits) {
					if (i->_unit->getType() == UnitTypes::Terran_Marine) {
						loadToEmptyBunker(i->_unit);
					}
				}
			}
		
			if (!ONLY_MICRO && !HIGH_LEVEL_SEARCH) {
				if ( !_positionTarget.isValid() || _center.getDistance(_positionTarget) < 75 ) {
					_positionTarget = squadManager->getBestTarget();
				}
			}

			break;
		case Fight:
			LOG4CXX_TRACE(_logger,"Fight State");
			if ( canWin() ) {
				LOG4CXX_TRACE(_logger,"Starting inCombat");
				inCombat();
			} else {
				LOG4CXX_TRACE(_logger,"Request Retreat");
				squadManager->requestRetreat(this);
			}

			if (_enemies.size() == 0 || imposibleAttack) {
				checkFormation();
				_state = GetPosition;
			}
			break;
		case Idle:
			for(CombatUnitSet::const_iterator i = this->_squadUnits.begin(); i!=this->_squadUnits.end(); ++i) {
				if ((*i)->_unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode) {
					if (!(*i)->_unit->isHoldingPosition() && informationManager->_initialRallyPosition.getApproxDistance((*i)->_unit->getPosition()) < 75 ) {
						(*i)->_unit->holdPosition();
					}

					if (Broodwar->self()->hasResearched(TechTypes::Tank_Siege_Mode) && 
						((*i)->_unit->getOrder() == Orders::PlayerGuard || (*i)->_unit->isHoldingPosition())  ) {
						//Broodwar->printf("Siege tank requested while IDLE!!");
						(*i)->onStop();
						//(*i)->siegeRequest();
						//(*i)->_siegeState = CombatAgent::None;
					}
				} else if ((*i)->_unit->getType() == UnitTypes::Terran_Marine) {
					loadToEmptyBunker((*i)->_unit);
				}
			}
			break;
		case Scout: scouting(); break;
		case Harass: harassing(); break;
		case Bunker:
			for (const auto & u : this->_squadUnits){
				UnitType type(u->_unit->getType());
				// if SCV and rushing
				if (type == UnitTypes::Terran_SCV && informationManager->_bbs) {
					// if not repairing move to "secure" position
					if (u->_unit->isIdle()) {
						Unit closestBarracks = u->_unit->getClosestUnit(Filter::IsOwned && Filter::GetType == UnitTypes::Terran_Barracks);
						if (closestBarracks && closestBarracks->getDistance(u->_unit) > 20) {
							u->onGetNewPosition(closestBarracks->getPosition());
						}
					}
					// TODO if being attacked, flee
					// if we are repairing bring another SCV
					if (u->_unit->isRepairing() && _squadUnits.size() <= 2) {
						Unit scv = workerManager->getWorkerForTask(_center);
						if (scv != nullptr) {
							squadManager->addUnitToSquad(this, scv);
							workerManager->onUnitDestroy(scv);
							scv->stop();
						} else LOG4CXX_ERROR(_logger, "No worker to send to bunker squad");
					}
				}
			}
			break;
		case Search:
			LOG4CXX_TRACE(_logger,"Update Search");
			CombatUnitSet::const_iterator unit = _squadUnits.begin();
			Unit wraith = (*unit)->_unit;
			if (wraith->getOrder() == Orders::PlayerGuard) {
				Position centerMap(Broodwar->mapWidth() * TILE_SIZE/2, Broodwar->mapHeight() * TILE_SIZE/2);
				if (wraith->getPosition().getDistance(centerMap) < 192) {
					LOG4CXX_TRACE(_logger,"[onFrame] wraith restart search");
					_positionTarget = Position(192,192);
					informationManager->_searchCorner = 1;
					informationManager->_searchIter = 0;
				} else if (wraith->getPosition().getDistance(_positionTarget) < 192) {
					LOG4CXX_TRACE(_logger,"[onFrame] Wraith next corner");
					// look for next corner
					int x,y;
					switch (informationManager->_searchCorner) {
						case 1: // we are in top left
							informationManager->_searchCorner++;
							x = (Broodwar->mapWidth()*TILE_SIZE) - (384 * informationManager->_searchIter) - 192;
							y = (384 * informationManager->_searchIter) + 192;
							_positionTarget = Position(x,y);
							break;
						case 2: // we are in top left
							informationManager->_searchCorner++;
							x = (Broodwar->mapWidth()*TILE_SIZE) - (384 * informationManager->_searchIter) - 192;
							y = (Broodwar->mapHeight()*TILE_SIZE) - (384 * informationManager->_searchIter) - 192;
							_positionTarget = Position(x,y);
							break;
						case 3: // we are in top left
							informationManager->_searchCorner++;
							x = (384 * informationManager->_searchIter) + 192;
							y = (Broodwar->mapHeight()*TILE_SIZE) - (384 * informationManager->_searchIter) - 192;
							_positionTarget = Position(x,y);
							break;
						case 4: // we are in top left
							informationManager->_searchIter++;
							x = (384 * (informationManager->_searchIter - 1)) + 192;
							y = (384 * informationManager->_searchIter)+ 192;
							_positionTarget = Position(x,y);
							informationManager->_searchCorner = 1;
							break;
					}
				}
				wraith->attack(_positionTarget);
			} else if (wraith->isAttacking()) { // tell to other squads to go to this position
				LOG4CXX_TRACE(_logger,"[onFrame] wraith attacking");
				for (auto& squad : squadManager->_squads) {
					if (squad->_state == SquadAgent::GetPosition) {
						squad->_positionTarget = Position(wraith->getPosition());
					}
				}
			}
			break;
	}
	LOG4CXX_TRACE(_logger,"[onFrame] END");
}

void SquadAgent::holdCenter()
{
	// get unit closest to target
	//Unit closestUnit = getClosestUnitTo(_positionTarget)->_unit;

	for(CombatUnitSet::const_iterator i=this->_squadUnits.begin();i!=this->_squadUnits.end();++i) {
		Broodwar->drawTextMap((*i)->_unit->getPosition().x, (*i)->_unit->getPosition().y-5,"%d", (*i)->_unit->getPosition().getApproxDistance(_holdCenterPosition));
		//if ( (*i)->_unit == closestUnit )  (*i)->_unit->stop();
		if ((*i)->_unit->getPosition().getApproxDistance(_holdCenterPosition) > 300) {
			(*i)->onGetPosition(_holdCenterPosition);
			//if ((*i)->_unit->isSieged()) (*i)->unsiegeRequest();
			//else if (!(*i)->_unit->isMoving()) (*i)->_unit->move(_holdCenterPosition);
		} else {
			if (!(*i)->_unit->isMoving() && !(*i)->_unit->isHoldingPosition() && !(*i)->_unit->isSieged() ) {
				(*i)->_unit->holdPosition();
			} 
		}
		// siege while waiting
		if ( Broodwar->self()->hasResearched(TechTypes::Tank_Siege_Mode) && ( (*i)->_unit->getOrder() == Orders::Stop || (*i)->_unit->getOrder() == Orders::PlayerGuard )
			&& (*i)->_unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode && !(*i)->_unit->isSieged()) {
				//Broodwar->printf("Siege tank requested while cheking waiting!!");
				(*i)->onHold();
				//(*i)->siegeRequest();
				//(*i)->_siegeState = CombatAgent::None;
		}
	}
	// if we are really big keep going
	if (_squadUnits.size() >= 30) {
		_state = GetPosition;
	}
}

void SquadAgent::loadToEmptyBunker(BWAPI::Unit unit)
{
	Unitset bunkersInRange = unit->getUnitsInRadius(500, Filter::IsOwned && Filter::GetType == UnitTypes::Terran_Bunker);
	for (const auto & bunker : bunkersInRange) {
		if (bunker->isCompleted() &&
			bunker->getLoadedUnits().size() < 4) {
			unit->load(bunker);
		} else {
			// TODO not the best option, it will make all marines near a bunker to stop...
			unit->stop();
		}
	}
}

void SquadAgent::orderGetPosition(Position positionTarget) 
{
	_positionTarget = positionTarget;
    //LOG("New squad target (" << _positionTarget.x << "," << _positionTarget.y << ")");
	if (_state != MergeSquads) {
		for(CombatUnitSet::const_iterator i = this->_squadUnits.begin(); i!=this->_squadUnits.end(); ++i) {
			(*i)->onGetNewPosition(_positionTarget);
		}
		_state = GetPosition;
		_positionToMerge = Positions::None;
	}

	// check formation
	checkFormation();
}

void SquadAgent::squadArea()
{
	// Compute squad Area
	_center = Position(0, 0);
	int squadSize = 0;
	for (CombatUnitSet::const_iterator it = this->_squadUnits.begin(); it!=this->_squadUnits.end(); ++it) {
		if ((*it)->_unit->getType().isFlyer()) continue; //ignore special micro
		_center += (*it)->_unit->getPosition();
		squadSize++;
	}
	if (squadSize > 0)
		_center = BWAPI::Position(_center.x / squadSize, _center.y / squadSize);
	_spread = 0;
	for (CombatUnitSet::const_iterator it = this->_squadUnits.begin(); it!=this->_squadUnits.end(); ++it) {
		if ((*it)->_unit->getType().isFlyer()) continue; //ignore special micro
		_spread += _center.getDistance((*it)->_unit->getPosition());
	}
	if (squadSize > 0)
		_spread /= squadSize;

	// Draw squad Area
#ifndef TOURNAMENT
	Color squadFormationColor = Colors::White;
	if (_movement == SquadAgent::Cohesion) squadFormationColor = Colors::Yellow;
	if (this == squadManager->_leadSquad) {
		Broodwar->drawCircleMap(_center, 5, Colors::Orange, true);
		squadFormationColor = Colors::Orange;
	}
	Broodwar->drawCircleMap(_center, (int) _spread, squadFormationColor);
	// Draw units in squad
	for (const auto& combatUnit : _squadUnits) {
		Broodwar->drawLineMap(_center, combatUnit->_unit->getPosition(), squadFormationColor);
	}
#endif
}

// TODO make this constant checking each time we add or delete a unit
bool SquadAgent::isSquadBio()
{
	for(CombatUnitSet::const_iterator i=this->_squadUnits.begin();i!=this->_squadUnits.end();++i) {
        if ((*i)->_unit->getType().isMechanical() && 
            ((*i)->_unit->getType() != UnitTypes::Terran_Science_Vessel || 
             (*i)->_unit->getType() != UnitTypes::Terran_SCV)) {
             return false;
        }
	}
	return true;
}

#define ATTACK_SQUAD_MAX_SPREAD_BASE 50
#define ATTACK_SQUAD_MAX_SPREAD_PER_UNIT 2
#define ATTACK_SQUAD_MIN_SPREAD_BASE 30

void SquadAgent::checkFormation()
{
	_movement = SquadAgent::Normal;
}

void SquadAgent::checkSpread()
{
	if (!ONLY_MICRO) {
		// if we are close to our base, don't checkSpread
		if (informationManager->home->getCenter().getApproxDistance(_center) < 30*TILE_SIZE) return;
	}

	// if the squad has more than 30 units, don't checkSpread
	if (_squadUnits.size() >= 40) return;

	//double maxSpread = ATTACK_SQUAD_MAX_SPREAD_BASE + ATTACK_SQUAD_MAX_SPREAD_PER_UNIT * _squadUnits.size();
	//double minSpread = ATTACK_SQUAD_MIN_SPREAD_BASE + ATTACK_SQUAD_MAX_SPREAD_PER_UNIT * _squadUnits.size();
	double maxSpread = MAX_SPREAD_BASE + _squadUnits.size() + _squadMaxSpread;
	double minSpread = MIN_SPREAD_BASE + _squadMaxSpread;

// 	Broodwar->drawTextScreen(5,26,"Max spread: %.2f", maxSpread );
// 	Broodwar->drawTextScreen(5,39,"Min spread: %.2f", minSpread );

	if ( _movement == SquadAgent::Normal && _spread >  maxSpread) {
		// get nearest chokepoint
		LOG4CXX_TRACE(_logger,"Spread 1");
		BWTA::Chokepoint* bestChokepoint = BWTA::getNearestChokepoint(getClosestUnitTo(_positionTarget, UnitTypes::None, true)->_unit->getPosition());
		// get unit closest to chokepoint
		Position location;
		if (bestChokepoint != NULL) {
			LOG4CXX_TRACE(_logger,"Spread 2");
			location = getClosestUnitTo(bestChokepoint->getCenter(), UnitTypes::None, true)->_unit->getPosition();
		} else {
			LOG4CXX_TRACE(_logger,"Spread 3");
			location = getClosestUnitTo(_positionTarget, UnitTypes::None, true)->_unit->getPosition();
		}
		for(CombatUnitSet::const_iterator i=this->_squadUnits.begin();i!=this->_squadUnits.end();++i) {
			if ((*i)->_unit->getType() == UnitTypes::Terran_Dropship || (*i)->_unit->getType() == UnitTypes::Terran_Science_Vessel) continue; //ignore special micro
			(*i)->_unit->move(location);
		}
		_movement = SquadAgent::Cohesion;
	}
	if ( _movement == SquadAgent::Cohesion && _spread < minSpread ){
		for(CombatUnitSet::const_iterator i=this->_squadUnits.begin();i!=this->_squadUnits.end();++i) {
			if ((*i)->_unit->getType() == UnitTypes::Terran_Dropship || (*i)->_unit->getType() == UnitTypes::Terran_Science_Vessel) continue; //ignore special micro
			(*i)->_unit->attack(_positionTarget);
		}
		_movement = SquadAgent::Normal;
	}

	//CombatUnitSet::const_iterator i=this->_squadUnits.begin();
	LOG4CXX_TRACE(_logger,"Spread 4");
	Unit closest = getClosestUnitTo(_positionTarget, UnitTypes::None, true)->_unit;
	if ( (closest->getOrder() == Orders::AttackMove || closest->getOrder() == Orders::AttackTile || closest->getOrder() == Orders::AttackUnit) && _movement == SquadAgent::Cohesion ) {
		_movement = SquadAgent::Normal;
	}

#ifndef TOURNAMENT
	Broodwar->drawCircleMap(_center.x, _center.y, (int) maxSpread, Colors::Red, false);
	Broodwar->drawCircleMap(_center.x, _center.y, (int) minSpread, Colors::Green, false);
#endif
}

bool SquadAgent::needWait()
{
	if (ONLY_MICRO) {
		_waitingReason = "Move: only micro";
		return false;
	}

    if (HIGH_LEVEL_SEARCH) {
        _waitingReason = "Move: high level search";
        return false;
    }

	if (_state != HoldCenter) {
		if (_squadUnits.size() > 11) {
			_waitingReason = "Move: squads too big";
			return false;
		}

		if (squadManager->normalSquads() > 1 && this != squadManager->_leadSquad) {
			_waitingReason = "Move: squads > 1";
			return false;
		}
	
		// TODO if we have SCV repairing: wait
		for(CombatUnitSet::const_iterator i=_squadUnits.begin();i!=_squadUnits.end();++i) {
			if ((*i)->_unit->isRepairing()) {
				_waitingReason = "Wait: SCV repairing";
				orderWait( (*i)->_unit );
				return true;
			}
		}

		// if we are squad leader, wait for at least one Vulture
		if (Broodwar->enemy()->getRace() == Races::Protoss && this == squadManager->_leadSquad && !hasUnitOfType(UnitTypes::Terran_Vulture)) {
			_waitingReason = "Wait: need Vultures";
			orderWait();
			return true;
		}
	}

	// get unit closest to target
	LOG4CXX_TRACE(_logger,"wait 1");
	Unit closestUnit = getClosestUnitTo(_positionTarget, UnitTypes::None, true)->_unit;
#ifndef TOURNAMENT
	Broodwar->drawCircleMap(closestUnit->getPosition().x,closestUnit->getPosition().y,300,Colors::Purple,false);
#endif

	// only wait if closestUnit is more close to enemyBase than homeBase
// 	if (closestUnit->getPosition().getApproxDistance(informationManager->home->getCenter()) < (int)(closestUnit->getPosition().getApproxDistance(informationManager->_enemyStartPosition)/3)) {
// 		_waitingReason = "Move: close to home";
// 		return false;
// 	}

	BWAPI::Unitset UnitsInRange = closestUnit->getUnitsInRadius(300);
	for(BWAPI::Unitset::iterator i=UnitsInRange.begin();i!=UnitsInRange.end();) {
		// remove units not belonging to our squad
		UnitToSquadMap::iterator found = squadManager->_unitToSquadMap.find(*i);
		if(found != squadManager->_unitToSquadMap.end() && found->second == this) {
			++i;
		} else {
			i = UnitsInRange.erase(i);
		}
	}
	// add the closestUnit
	UnitsInRange.insert(closestUnit);

	std::ostringstream oss;
	oss << UnitsInRange.size() << " of " << informationManager->_minSquadSize;

	if (UnitsInRange.size() < informationManager->_minSquadSize) {
		for(CombatUnitSet::const_iterator i=this->_squadUnits.begin();i!=this->_squadUnits.end();++i) {
			if ((*i)->_unit->getType() == UnitTypes::Terran_Dropship || (*i)->_unit->getType() == UnitTypes::Terran_Science_Vessel) continue; //ignore special micro
			//if ((*i)->_unit->getType() == UnitTypes::Terran_SCV) continue; //ignore special micro
			Broodwar->drawTextMap((*i)->_unit->getPosition().x, (*i)->_unit->getPosition().y-5,"%d", (*i)->_unit->getPosition().getApproxDistance(closestUnit->getPosition()));
			if ( (*i)->_unit == closestUnit && (*i)->_unit->isMoving() )  (*i)->_unit->stop();
			else if ((*i)->_unit->getPosition().getApproxDistance(closestUnit->getPosition()) > 300) {
				(*i)->onGetPosition(closestUnit->getPosition());
				//if ((*i)->_unit->isSieged()) (*i)->unsiegeRequest();
				//else if (!(*i)->_unit->isMoving()) (*i)->_unit->move(closestUnit->getPosition());
			}
			// siege while waiting
			if ( Broodwar->self()->hasResearched(TechTypes::Tank_Siege_Mode) && ( (*i)->_unit->getOrder() == Orders::Stop || (*i)->_unit->getOrder() == Orders::PlayerGuard )
				&& (*i)->_unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode && UnitsInRange.size() < informationManager->_minSquadSize-1 && !(*i)->_unit->isSieged()) {
				//Broodwar->printf("Siege tank requested while cheking waiting!!");
				(*i)->onStop();
				//(*i)->siegeRequest();
				//(*i)->_siegeState = CombatAgent::None;
			}
		}
		_waitingReason = "Wait: " + oss.str() + " in squad";
		return true;	
	}
	_waitingReason = "Move: " + oss.str() + " in squad";
	return false;
}

void SquadAgent::orderWait(BWAPI::Unit unitToSkip)
{
	for(CombatUnitSet::const_iterator i = this->_squadUnits.begin(); i!=this->_squadUnits.end(); ++i) {
		if ( (*i)->_unit == unitToSkip ) continue;
		if ( (*i)->_unit->isMoving() ) {
			(*i)->_unit->stop();
		}
	}
}

void SquadAgent::inCombat()
{
	for(CombatUnitSet::const_iterator i=this->_squadUnits.begin();i!=this->_squadUnits.end();++i)
	{
		if ((*i)->_unit->getType() == UnitTypes::Terran_Dropship || (*i)->_unit->getType() == UnitTypes::Terran_Science_Vessel) continue; //ignore special micro
		// find and attack best target
		(*i)->inCombat(_enemies, this);

		// Debug info
		Color colorAttack = Colors::Blue;
		if ((*i)->_unit->isStartingAttack())
			colorAttack = Colors::Yellow;
		else if ((*i)->_unit->isAttacking())
			colorAttack = Colors::Red;

		Unit targetSelected = (*i)->_unit->getOrderTarget();
		if (targetSelected != NULL && targetSelected != (*i)->_lastTarget) {
			//Broodwar->printf("Automatic targeting");
			//(*i)->_unit->attack((*i)->_lastTarget); //FIXME: Force to attack right target. Search why happens this
			Broodwar->drawLineMap((*i)->_unit->getPosition().x, (*i)->_unit->getPosition().y, targetSelected->getPosition().x, targetSelected->getPosition().y, Colors::Orange);
		}

		if ((*i)->_unit->isIdle() && (*i)->_unit->getType() == UnitTypes::Terran_Marine) {
// 			Broodwar->printf("WARNING: marine in combat idle");
// 			DEBUG("WARNING: marine in combat idle");
			(*i)->_unit->attack((*i)->_lastTarget);
		}

		if ((*i)->_lastTarget!=0) {
// 			if ( (*i)->_lastTarget->getPosition().x > (Broodwar->mapWidth()*TILE_SIZE - 5)) {
// 				Broodwar->setLocalSpeed(500);
// 				Broodwar->printf("Last target position %i,%i",(*i)->_lastTarget->getPosition().x, (*i)->_lastTarget->getPosition().y);
// 			}
			Broodwar->drawLineMap((*i)->_unit->getPosition().x, (*i)->_unit->getPosition().y, (*i)->_lastTarget->getPosition().x, (*i)->_lastTarget->getPosition().y, colorAttack);
			// If unit is Tank and is in siege mode, draw splash damage area of effect
			// 10, 25, 40 pixels
			//if ( (*i)->_unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode || (*i)->_unit->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode) {
			if ( (*i)->_unit->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode) {
				Broodwar->drawCircleMap((*i)->_lastTarget->getPosition().x, (*i)->_lastTarget->getPosition().y,40,Colors::Orange);
			}
			// calculate 15 degrees near point
			//Position rotated = rotatePosition(20, (*i)->_lastTarget->getPosition(), (*i)->_unit->getPosition());
			//Broodwar->drawLineMap((*i)->_unit->getPosition().x, (*i)->_unit->getPosition().y, rotated.x, rotated.y, Colors::Purple);
		} else {
			//Broodwar->printf("[ERROR] No target selected!!!!!!!");
		}
		
		if ((*i)->_lastPosition!=Positions::None) {
			Broodwar->drawLineMap((*i)->_unit->getPosition().x, (*i)->_unit->getPosition().y, (*i)->_lastPosition.x, (*i)->_lastPosition.y, Colors::Cyan);
			//Broodwar->drawCircleMap((*i)->_lastPosition.x, (*i)->_lastPosition.y,3,Colors::Cyan,true);
		}
		//Broodwar->drawTextMap((*i)->_unit->getPosition().x, (*i)->_unit->getPosition().y-5,"%d", (*i)->getEnemiesInRange(_enemies));
		//Broodwar->drawTextMap((*i)->_unit->getPosition().x, (*i)->_unit->getPosition().y-5,"%d", (*i)->_unit->getGroundWeaponCooldown());
		//Broodwar->drawCircleMap((*i)->_unit->getPosition().x, (*i)->_unit->getPosition().y, (*i)->_unit->getType().seekRange(), Colors::Yellow, false);

		if ((*i)->_unit->isHoldingPosition())
			Broodwar->drawBoxMap((*i)->_unit->getLeft(), (*i)->_unit->getTop(), (*i)->_unit->getRight(), (*i)->_unit->getBottom(), Colors::Red);
	}
}

void SquadAgent::onUnitDestroy(Unit unit)
{
	UnitToCombatAgentMap::iterator found = _unitToCombatAgentMap.find(unit);
	if(found != _unitToCombatAgentMap.end()) {
		CombatAgent *unitToDelete = found->second;
		if (unitToDelete->_unit->isSieged()) {
			// Update siege map
			int x = (int)(unitToDelete->_unit->getLeft()/8);
			int y = (int)(unitToDelete->_unit->getTop()/8);
			//DEBUG("UNSIEGE (" << unitToDelete->_unit << ") " << x << "," << y); 
			// Tanks dimensions = 32x32 -> 4 walktiles (8x8) (sometimes the position during siege is not equal during unsiege)
			informationManager->_tankSiegeMap.setRectangleTo(x-1, y-1, x+6, y+6, 1);	
		}
		_squadUnits.erase(unitToDelete);
		_unitToCombatAgentMap.erase(found);
		delete unitToDelete;
		// remove from spread calculation
		if ( unit->getType().size() == UnitSizeTypes::Small ) {
			_squadMaxSpread -= SMALL_SPREAD;
		} else if ( unit->getType().size() == UnitSizeTypes::Medium ) {
			_squadMaxSpread -= MEDIUM_SPREAD;
		} else if ( unit->getType().size() == UnitSizeTypes::Large ) {
			_squadMaxSpread -= LARGE_SPREAD;
		}
		removeSquadThreat(unit);
	} else {
		//DEBUG("[ERROR] Unit (" << unit << ") not found");
		//DEBUG("   UNITS ON SQUAD " << this);
		//for(CombatUnitSet::const_iterator unit2=_squadUnits.begin();unit2!=_squadUnits.end();++unit2) {
		//	DEBUG("    Unit (" << (*unit2)->_unit << ")");
		//}
	}
}

int SquadAgent::getUnitFrameCreated(Unit unit)
{
	UnitToCombatAgentMap::iterator found = _unitToCombatAgentMap.find(unit);
	if(found != _unitToCombatAgentMap.end()) {
		CombatAgent *combatUnit = found->second;
		return combatUnit->frameCreated;
	} else {
		//DEBUG("[ERROR] unit not found in _unitToCombatAgentMap");
		return Broodwar->getFrameCount();
	}
}

bool SquadAgent::canWin()
{
	if (informationManager->_retreatDisabled) return true;

	// calculate time to kill
	double timeToKillEnemyAir = (_enemyStats.airHP>0)? (_squadStats.airDPS == 0)? 99999 : _enemyStats.airHP/_squadStats.airDPS : 0;
	double timeToKillEnemyGround = (_enemyStats.groundHP>0)? (_squadStats.groundDPS == 0)? 99999 : _enemyStats.groundHP/_squadStats.groundDPS : 0;
	double timeToKillSquadAir = (_squadStats.airHP>0)? (_enemyStats.airDPS == 0 )? 99999 : _squadStats.airHP/_enemyStats.airDPS : 0;
	double timeToKillSquadGround = (_squadStats.groundHP>0)? (_enemyStats.groundDPS == 0)? 99999 : _squadStats.groundHP/_enemyStats.groundDPS : 0;

// 	Broodwar->drawTextScreen(100,100,"%0.2f > %0.2f || %0.2f > %0.2f",timeToKillSquadAir,timeToKillEnemyAir,timeToKillSquadGround,timeToKillEnemyGround );

	return (timeToKillSquadAir>timeToKillEnemyAir || timeToKillSquadGround>timeToKillEnemyGround);
}

#define MERGE_DISTANCE 100

void SquadAgent::inMerge()
{
	//DEBUG("-Get first unit");
	CombatUnitSet::const_iterator i=this->_squadUnits.begin();
	if ( (*i)->_unit->getOrder() == Orders::PlayerGuard ) {
		//DEBUG("-Update own squad");
		inMerge(_squadToMerge);
		//DEBUG("-Update target squad");
		_squadToMerge->inMerge(this);
	}
}

void SquadAgent::inMerge(SquadAgent* squadToMerge, Position toMerge)
{
	LOG4CXX_TRACE(_logger,"[inMerge] Updating vars");
	_state = MergeSquads;
	_waitingNewUnits = false;
	_squadToMerge = squadToMerge;

	LOG4CXX_TRACE(_logger,"[inMerge] Updating position");
	if (toMerge == Positions::None) {
		LOG4CXX_TRACE(_logger,"merge 1");
		_positionToMerge = _squadToMerge->getClosestUnitTo(_positionTarget, UnitTypes::None, true)->_unit->getPosition();
	} else {
		_positionToMerge = toMerge;
	}

	LOG4CXX_TRACE(_logger,"[inMerge] Moving order");
	for(CombatUnitSet::const_iterator i=this->_squadUnits.begin();i!=this->_squadUnits.end();++i) {
		if ((*i)->_unit->getType() == UnitTypes::Terran_Dropship || (*i)->_unit->getType() == UnitTypes::Terran_Science_Vessel) continue; //ignore special micro
		(*i)->_unit->move(_positionToMerge);
	}
	//inMerge();
}

CombatAgent* SquadAgent::getClosestUnitTo(Position toPosition, UnitType type, bool ignoreFlyers, bool forceResult)
{
	int dist = 9999999;
	int newDist;
	CombatAgent *bestUnit = 0;
	for(CombatUnitSet::const_iterator i=this->_squadUnits.begin();i!=this->_squadUnits.end();++i) {
		if ((*i)->_unit->getType() == UnitTypes::Terran_Dropship || (*i)->_unit->getType() == UnitTypes::Terran_Science_Vessel) continue; //ignore special micro
		if (ignoreFlyers && (*i)->_unit->getType().isFlyer() ) continue;
		if (type == UnitTypes::None || type == (*i)->_unit->getType()) {
			newDist = (*i)->_unit->getPosition().getApproxDistance(toPosition);
			if (newDist < dist) {
				dist = newDist;
				bestUnit = *i;
			}
		}
	}
	if (bestUnit == 0) {
		LOG4CXX_WARN(_logger,"Unit not found in getClosestUnitTo");
		if (forceResult) bestUnit = *(_squadUnits.begin());
		LOG4CXX_WARN(_logger,"return closest unit: " << bestUnit);
	}
	return bestUnit;
}


bool SquadAgent::hasUnitOfType(const UnitType &type)
{
	for(CombatUnitSet::const_iterator i=this->_squadUnits.begin();i!=this->_squadUnits.end();++i) {
		if ((*i)->_unit->getType() == type) return true;
	}
	return false;
}

bool SquadAgent::squadNeedSCV()
{
	if (_squadUnits.size() >= 2 && !isSquadBio()) {
		bool onlyVultures = true;
		int sizeSCV = 0;
		for(CombatUnitSet::const_iterator i=this->_squadUnits.begin();i!=this->_squadUnits.end();++i) {
			if ((*i)->_unit->getType() == UnitTypes::Terran_SCV) sizeSCV++;
			else if ((*i)->_unit->getType() != UnitTypes::Terran_Vulture) onlyVultures = false;
		}
		if (onlyVultures) return false;
		int scvRequired = 1;
		if (_squadUnits.size() >= informationManager->_minSquadSize) scvRequired = 2;
		if (sizeSCV < scvRequired) return true;
	}
	return false;
}

void SquadAgent::scouting()
{
	CombatUnitSet::const_iterator it = _squadUnits.begin();
	Unit unit = (*it)->_unit;

	Position unitPos = unit->getPosition();
	if ( unit->getOrder() == Orders::PlayerGuard && !Broodwar->isVisible(TilePosition(_positionTarget)) ) {
		unit->move(_positionTarget);
	}

	// check if there is a base //TODO I just can check the enemies assigned to the squad
	if ( Broodwar->isVisible(TilePosition(_positionTarget)) ) {
		Position topLeft(_positionTarget.x-(2*TILE_SIZE), _positionTarget.y-(1*TILE_SIZE) );
		Position bottomRight( _positionTarget.x+(2*TILE_SIZE), _positionTarget.y+(2*TILE_SIZE) );
		topLeft.makeValid(); bottomRight.makeValid(); // TODO do we really need this?
// #ifndef TOURNAMENT
// 		informationManager->printDebugBox(topLeft, bottomRight, "[Scouting] Looking for a base inside here");
// #endif

		BWAPI::Unitset unitsOnLocation = Broodwar->getUnitsInRectangle(topLeft, bottomRight);
		bool enemyBase = false;
		for(BWAPI::Unitset::iterator checkUnit=unitsOnLocation.begin();checkUnit!=unitsOnLocation.end();++checkUnit) {
			if ( (*checkUnit)->getType().isResourceDepot() ) enemyBase = true;
		}
		if (!enemyBase) {
			informationManager->deleteEnemyStartLocation(_positionTarget);
			// get next position to scout
			_positionTarget = squadManager->getScoutTarget();
			unit->move(_positionTarget);
// 		} else {
// 			informationManager->enemyStartFound(_positionTarget);
		}
	}

	BWTA::Region* unitRegion = BWTA::getRegion( unit->getTilePosition() );
	if (unitRegion == BWTA::getRegion(TilePosition(_positionTarget))) {
		unit->move(getPositionToScout(unitPos, unitRegion, _positionTarget));
	}
#ifndef TOURNAMENT
	// Debug info
	Broodwar->drawCircleMap(unitPos.x,unitPos.y,30,Colors::Yellow,false);
	Broodwar->drawLineMap(unitPos.x, unitPos.y, _positionTarget.x, _positionTarget.y, Colors::Yellow);
	Broodwar->drawCircleMap(_positionTarget.x,_positionTarget.y,3,Colors::Yellow,true);
#endif
}

void SquadAgent::harassing()
{
	for (auto & combatUnit : _squadUnits) {
		Unit unit = combatUnit->_unit;

		if (unit->getOrder() == Orders::PlayerGuard && !Broodwar->isVisible(TilePosition(_positionTarget))) {
			unit->move(_positionTarget);
		}

		// if we are near destination, attack
		if (Broodwar->isVisible(TilePosition(_positionTarget))) {
			inCombat();
		}

		// if is a organic unit ...
		if (unit->getType() == UnitTypes::Terran_Marine || unit->getType() == UnitTypes::Terran_Firebat) {
			Unit closestBunker = unit->getClosestUnit(Filter::IsOwned && Filter::GetType == UnitTypes::Terran_Bunker && Filter::IsCompleted);
			// ... and there is a bunker near
			if (closestBunker != nullptr) {
				// retreat to the bunker if an enemy is near
				Unit closestEnemy = unit->getClosestUnit(Filter::IsEnemy && Filter::CanAttack, unit->getType().sightRange());
				if (closestEnemy != nullptr && 
					closestEnemy->getDistance(unit) < closestEnemy->getType().groundWeapon().maxRange() + 20) {
					if (closestBunker->getLoadedUnits().size() < 4) {
						unit->load(closestBunker);
					} else {
						unit->move(closestBunker->getPosition());
					}
				}
			}
		}
	}

	// if we are at the harassing location and there isn't an enemy base, transition to "normal" squad
	if (Broodwar->isVisible(TilePosition(_positionTarget))) {
		Unitset enemyBase = Broodwar->getUnitsInRadius(_positionTarget, 50, Filter::IsEnemy && Filter::IsResourceDepot);
		if (enemyBase.empty()) {
			_state = GetPosition;
		}
	}
}

BWAPI::Position SquadAgent::getPositionToScout(Position myPos, BWTA::Region* myRegion, Position basePos, bool checkVisible)
{
	Position returnPosition;
	int maxDist = 17;
	Broodwar->drawCircleMap(basePos.x,basePos.y,maxDist*TILE_SIZE,Colors::Yellow,false);
	TilePosition seedTilePos = TilePosition(myPos);
	TilePosition baseTilePos = TilePosition(basePos);
	int x      = seedTilePos.x;
	int y      = seedTilePos.y;
	int length = 1;
	int j      = 0;
	bool first = true;
	int dx     = 0;
	int dy     = 1;	
	while (length < Broodwar->mapWidth()) {
		returnPosition = Position(x*TILE_SIZE, y*TILE_SIZE);
		//check max distance
		if (returnPosition.getDistance(myPos) > maxDist*TILE_SIZE) {
			//if (x > baseTilePos.x+maxDist && y > baseTilePos.y+maxDist) {
			if (!checkVisible) return getPositionToScout(myPos, myRegion, basePos, true);
			else return basePos;
		}

		if (x >= 0 && x < Broodwar->mapWidth() && y >= 0 && y < Broodwar->mapHeight() && 
			myRegion == BWTA::getRegion(x,y) && Broodwar->hasPath(myPos,returnPosition) ) {
				//if (x <= baseTilePos.x+maxDist && y <= baseTilePos.y+maxDist) {
				if (!checkVisible) {
					if (!Broodwar->isExplored(x,y)) return returnPosition;
				} else {
					if (!Broodwar->isVisible(x,y)) return returnPosition;
				}
				//}
		}

		//otherwise, move to another position
		x = x + dx;
		y = y + dy;
		//count how many steps we take in this direction
		j++;
		if (j == length) { //if we've reached the end, its time to turn
			j = 0;	//reset step counter

			//Spiral out. Keep going.
			if (!first)
				length++; //increment step counter if needed

			first =! first; //first=true for every other turn so we spiral out at the right rate

			//turn counter clockwise 90 degrees:
			if (dx == 0) {
				dx = dy;
				dy = 0;
			} else {
				dy = -dx;
				dx = 0;
			}
		}
		//Spiral out. Keep going.
	}

	return basePos;
}
