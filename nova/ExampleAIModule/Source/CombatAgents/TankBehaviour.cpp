#include "TankBehaviour.h"
#include "SquadAgent.h"

using namespace BWAPI;

#define SIEGE_MARGIN 4

TankBehaviour::TankBehaviour(Unit unit, class SquadAgent* squad)
	: _unit(unit),
	_squad(squad),
	_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent.Tank")),
	_lastPosition(Positions::None)
{

}

void TankBehaviour::byDefault()
{

}

void TankBehaviour::onGetPosition(Position targetPosition)
{
	if (_unit->getOrder() == Orders::PlayerGuard) {
		if (_unit->isSieged()) unsiegeRequest();
		else _unit->attack(targetPosition);
	}

	// if we are attacking or being attacked -> change to siege mode //TODO review this part
	if ( Broodwar->self()->hasResearched(TechTypes::Tank_Siege_Mode) && _unit->isStartingAttack() ) {
		if (_unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode) {
			//Broodwar->printf("Siege tank requested while getPosition (no waiting)!!");
			siegeRequest();
			//_siegeState = CombatAgent::None;
		}
	}
}

void TankBehaviour::onGetNewPosition(Position targetPosition)
{
	if (_unit->isSieged()) unsiegeRequest();
	else _unit->attack(targetPosition);

	// if we are attacking or being attacked -> change to siege mode //TODO review this part
	if ( Broodwar->self()->hasResearched(TechTypes::Tank_Siege_Mode) && _unit->isStartingAttack() ) {
		if (_unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode) {
			//Broodwar->printf("Siege tank requested while getPosition (no waiting)!!");
			siegeRequest();
			//_siegeState = CombatAgent::None;
		}
	}
}

void TankBehaviour::onStop()
{ 
	siegeRequest(); 
};

void TankBehaviour::onHold()
{
	siegeRequest();
};


void TankBehaviour::onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies)
{
	// sanitize _lastTarget
	//if ( informationManager->_seenEnemies.find(_lastTarget) != informationManager->_seenEnemies.end() ) {
	//	_lastTarget = NULL;
	//}

	if (Broodwar->self()->hasResearched(TechTypes::Tank_Siege_Mode)) {
		if ( !_unit->isSieged() ) { // tank not in siege mode
			if ( _unit->isStartingAttack() ) 
				siegeRequest();
			if ( _unit->getOrder() != Orders::Sieging && _unit->getOrder() != Orders::Unsieging && enemyInSiegeRange()) {
				siegeRequest();
			}
		} else { // tank in siege mode
			if ( !enemyInSiegeRange(false) || // if no enemy in siege range
				(bestTarget != NULL && _unit->getPosition().getDistance(bestTarget->getPosition()) > 15*TILE_SIZE) ) { // or best target too far
					unsiegeRequest();
			}
		}
	}

	// keep truck of the last bullet
	if (_unit->getGroundWeaponCooldown()!=0) {
		informationManager->_tankSiege[_unit] = Broodwar->getFrameCount();
	}

	if (!_unit->isSieged() && _unit->getOrder() == Orders::PlayerGuard) {
		if (_squad->_positionTarget.isValid()) _unit->attack(_squad->_positionTarget);
	}
}

bool TankBehaviour::enemyInSiegeRange(bool closeBuildings)
{
	//if (_lastTarget == NULL) {
	//	_siegeState = CombatAgent::NullTarget;
	//	return false;
	//}
	//if ( informationManager->_seenEnemies.find(_lastTarget) != informationManager->_seenEnemies.end() ) {
	//	_lastTarget = NULL;
	//}
	//BWAPI::Unitset UnitsInRange = _unit->getUnitsInWeaponRange(WeaponTypes::Arclite_Shock_Cannon);
	BWAPI::Unitset UnitsInRange = _unit->getUnitsInRadius(384); // 384 is the range of Arclite_Shock_Cannon
	// 	LOG4CXX_DEBUG(_logger, "[UNSIEGE] Units in weapon range: " << UnitsInRange.size());
	for(BWAPI::Unitset::iterator i=UnitsInRange.begin();i!=UnitsInRange.end();++i) {
		if ( Broodwar->self()->isEnemy((*i)->getPlayer()) && !(*i)->getType().isFlyer()) {
			// get closes to units that cannot attack
			if (closeBuildings && !(*i)->getType().canAttack() && (*i)->getType() != UnitTypes::Terran_Bunker) {
				if (_unit->getPosition().getDistance((*i)->getPosition()) > 7*TILE_SIZE) {
					continue;
				}
			}
			//_siegeState = CombatAgent::Enemies;
			return true;
		}
	}

	//_siegeState = CombatAgent::NoEnemies;
	return false;
}

void TankBehaviour::siegeRequest()
{
	if (_unit->getOrder() == Orders::Sieging) return;

	// check if we aren't blocking the path
	if (wallNear()) {
		return;
	}

	bool ok = _unit->siege();

	if (!ok) {
		//Broodwar->printf("[SIEGE ERROR] %s", Broodwar->getLastError().toString().c_str() );
	} else {
		informationManager->_tankSiege[_unit] = Broodwar->getFrameCount();

		// Update siege map
		int x = (int)(_unit->getLeft()/8);
		int y = (int)(_unit->getTop()/8);
		//DEBUG("SIEGE (" << _unit << ") " << x << "," << y); 
		// Tanks dimensions = 32x32 -> 4 walktiles (8x8)
		informationManager->_tankSiegeMap.setRectangleTo(x, y, x+4, y+4, 0);
	}
}

// TODO false negative case example
//  ***
//  ***T  <-- detected as wall near (top-bottom)
//  ***
bool TankBehaviour::wallNear()
{
	int mapW = BWAPI::Broodwar->mapWidth()*4;
	int mapH = BWAPI::Broodwar->mapHeight()*4;
	//Check if we have free space surrounding us
	int leftPos = (int)(_unit->getLeft()/8) - SIEGE_MARGIN;
	int topPos = (int)(_unit->getTop()/8) - SIEGE_MARGIN;
	int rightPos = (int)(_unit->getLeft()/8)+ 4 + SIEGE_MARGIN;
	int bottomPos = (int)(_unit->getTop()/8)+ 4 + SIEGE_MARGIN;
	// Check left and right
	bool leftFree = true;
	for (int y = topPos; y <= bottomPos; y++) {
		if (leftPos >= 0 && leftPos < mapW && y >= 0 && y < mapH) {
			if (informationManager->_tankSiegeMap[leftPos][y] == 0) {
				leftFree = false;
				break;
			}
		} else {
			leftFree = false; //Out of map
			break;
		}
	}
	if (!leftFree) {
		for (int y = topPos; y <= bottomPos; y++) {
			if (rightPos >= 0 && rightPos < mapW && y >= 0 && y < mapH) {
				if (informationManager->_tankSiegeMap[rightPos][y] == 0) {
					//					informationManager->_center = Position(rightPos*8,y*8);
					// 					Broodwar->printf("Wall near (left-right)" );
					// 					Broodwar->setScreenPosition(_unit->getLeft(), _unit->getTop());
					// 					Broodwar->setLocalSpeed(200);
					return true;
				}
			} else {
				// 				Broodwar->printf("Wall near (left-right)" );
				// 				Broodwar->setScreenPosition(_unit->getLeft(), _unit->getTop());
				// 				Broodwar->setLocalSpeed(200);
				return true; //Out of map
			}
		}
	}
	// Check top and bottom
	bool topFree = true;
	for (int x = leftPos; x <= rightPos; x++) {
		if (topPos >= 0 && topPos < mapH && x >= 0 && x < mapW) {
			if (informationManager->_tankSiegeMap[x][topPos] == 0) {
				topFree = false; //Cant build here.
				break;
			}
		} else {
			topFree = false; //Out of map
			break;
		}
	}
	if (!topFree) {
		for (int x = leftPos; x <= rightPos; x++) {
			if (bottomPos >= 0 && bottomPos < mapH && x >= 0 && x < mapW) {
				if (informationManager->_tankSiegeMap[x][bottomPos] == 0) {
					//					informationManager->_center = Position(x*8,bottomPos*8);
					// 					Broodwar->printf("Wall near (top-bottom)" );
					// 					Broodwar->setScreenPosition(_unit->getLeft(), _unit->getTop());
					// 					Broodwar->setLocalSpeed(200);
					return true;
				}
			} else {
				// 				Broodwar->printf("Wall near (top-bottom" );
				// 				Broodwar->setScreenPosition(_unit->getLeft(), _unit->getTop());
				// 				Broodwar->setLocalSpeed(200);
				return true; //Out of map
			}
		}
	}

	return false;
}

void TankBehaviour::unsiegeRequest()
{
	if (!_unit->isSieged() || (_unit->getOrder() == Orders::Sieging || _unit->getOrder() == Orders::Unsieging)) {
		//LOG4CXX_ERROR(_logger, "[UNSIEGE] Requested, but we are not sieged.");
		return;
	}
	LOG4CXX_TRACE(_logger, "[UNSIEGE] Requested, when order is: " << _unit->getOrder().c_str());

	if (!enemyInSiegeRange(false)) {
		if (Broodwar->getFrameCount()-informationManager->_tankSiege[_unit] > 5*24) { 
			// 			DEBUG("Avoid needWait dance");
			// 			BWAPI::Unitset UnitsInRange = _unit->getUnitsInRadius(300);
			// 			for(BWAPI::Unitset::const_iterator i=UnitsInRange.begin();i!=UnitsInRange.end();) {
			// 				if ( !(*i)->getType().canAttack() || (*i)->getType().isWorker() ) {
			// 					i = UnitsInRange.erase(i);
			// 				} else {
			// 					++i;
			// 				}
			// 			}
			// 			if (UnitsInRange.size() < informationManager->_minSquadSize) return;
			bool ok = _unit->unsiege();
			if (!ok) {
				//Broodwar->printf("[UNSIEGE ERROR] %s", Broodwar->getLastError().toString().c_str() );
			} else {
				// Update siege map
				int x = (int)(_unit->getLeft()/8);
				int y = (int)(_unit->getTop()/8);
				//DEBUG("UNSIEGE (" << _unit << ") " << x << "," << y); 
				// Tanks dimensions = 32x32 -> 4 walktiles (8x8)(sometimes the position during siege is not equal during unsiege)
				informationManager->_tankSiegeMap.setRectangleTo(x-1, y-1, x+6, y+6, 1);
			}
		}
	}
}