#include "VultureBehaviour.h"
#include "BuildManager.h"
#include "SquadAgent.h"

using namespace BWAPI;

#define MICRO_VULTURE_DEGREES 15

VultureBehaviour::VultureBehaviour(Unit unit, class SquadAgent* squad)
	: _unit(unit),
	_squad(squad),
	_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent.Vulture")),
	_lastPosition(Positions::None),
	_dropMine(false)
{

}

void VultureBehaviour::onGetPosition(Position targetPosition)
{
	if (_unit->getOrder() == Orders::PlayerGuard) {
		_unit->attack(targetPosition);
	}
}

void VultureBehaviour::onGetNewPosition(Position targetPosition)
{
	_unit->attack(targetPosition);
}

void VultureBehaviour::onStop() {};
void VultureBehaviour::onHold() {};

void VultureBehaviour::onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies)
{
	// sanitize... I'm not sure if we need this
	if ( bestTarget == NULL || !bestTarget->exists() ) return;

#ifndef TOURNAMENT
	// Print debug information
	Broodwar->drawTextMap(_unit->getPosition().x, _unit->getPosition().y-15,"%d", _unit->getSpiderMineCount() );
	Broodwar->drawCircleMap(_unit->getPosition().x,_unit->getPosition().y,130,Colors::Yellow);
	Broodwar->drawCircleMap(_unit->getPosition().x,_unit->getPosition().y,300,Colors::Red);
	Broodwar->drawCircleMap(_unit->getPosition().x,_unit->getPosition().y,2000,Colors::Red);
#endif

	if (_unit->getOrder() == Orders::PlaceMine) return; // if we are placing a mine, let it

	if (_unit->getGroundWeaponCooldown()==0) {// if we can attack
		double distanceToTarget = _unit->getPosition().getDistance(bestTarget->getPosition());
		// check drop mine conditions
		if (_dropMine && !isSpiderMineNear() && distanceToTarget > 300 && distanceToTarget < 2000 ) {
			//Broodwar->printf("Droping mine!");
			LOG4CXX_TRACE(_logger,"Droping mine");
			_unit->useTech(TechTypes::Spider_Mines, _unit->getPosition()); 
			_dropMine = false;
			return;
		}

		if (!(bestTarget->getType() == UnitTypes::Protoss_Dragoon && _unit->getSpiderMineCount() == 0 && 
			(_squad->hasUnitOfType(UnitTypes::Terran_Siege_Tank_Tank_Mode) || _squad->hasUnitOfType(UnitTypes::Terran_Siege_Tank_Siege_Mode) ) )) { // keep kiting if we have a Dragoon (and no tanks in our squad)
				// kamikaze attack
				// 		if (bestTarget->getType() == UnitTypes::Protoss_Dragoon && _unit->getSpiderMineCount() == 1) { 
				// 			if (distanceToTarget<50)
				// 				_unit->useTech(TechTypes::Spider_Mines, _unit->getPosition());
				// 			else
				// 				_unit->move(bestTarget->getPosition());
				// 			return;
				// 		}
				// normal attack
				if ( !(bestTarget->getType() == UnitTypes::Protoss_Zealot && distanceToTarget<130) && // if we are too close, keep running
					!(bestTarget->getType() == UnitTypes::Protoss_Dragoon && distanceToTarget<192 && _unit->getSpiderMineCount() > 0) ) { 
						if (_unit->getSpiderMineCount() > 0 && BWTA::getRegion( _unit->getTilePosition() ) != informationManager->home && 
							bestTarget->getType() == UnitTypes::Protoss_Dragoon && !isSpiderMineNear()) {
								_unit->useTech(TechTypes::Spider_Mines, _unit->getPosition()); return;
						}
						if (_unit->getOrder() != Orders::Patrol && _unit->getOrder() != Orders::AttackUnit) {
							if (bestTarget->getType() == UnitTypes::Protoss_Zealot) {
								// calculate 15 degree near point
								Position rotated = rotatePosition(MICRO_VULTURE_DEGREES, bestTarget->getPosition(), _unit->getPosition());
								_unit->patrol(rotated);
							} else {
								_unit->attack(bestTarget);
							}
							_lastPosition = Positions::None;
						}
						return;
				}
		}
	}

	int x = _unit->getTilePosition().x;
	int y = _unit->getTilePosition().y;
	// if we are in a danger position // TODO and no tank near???????????????????????
	if (informationManager->get_enemy_ground_dps(x, y, BWAPI::Broodwar->getFrameCount())>0) {
		if (_unit->getSpiderMineCount() > 0) _dropMine = true; // TODO broadcast drop mine to squad members
		kitingFrame = true;
		// Find Safest position in spiral
		// Safest spot is: minimum dps, furthest from the target enemy, but keeping it in range
		Position returnPosition;
		int length = 1;
		int j      = 0;
		bool first = true;
		int dx     = 0;
		int dy     = 1;	
		while (length < Broodwar->mapWidth()) {
			returnPosition = Position(x*TILE_SIZE, y*TILE_SIZE);

			if (x >= 0 && x < Broodwar->mapWidth() && y >= 0 && y < Broodwar->mapHeight() && Broodwar->hasPath(_unit->getPosition(),returnPosition) ) {
				double dps = informationManager->get_enemy_ground_dps(x, y,BWAPI::Broodwar->getFrameCount());
				if (dps == 0 && !buildManager->wallNear(TilePosition(x,y))) {
					_unit->move(returnPosition);
					_lastPosition = returnPosition;
					return;
				}
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
	}
}

bool VultureBehaviour::isSpiderMineNear()
{
	BWAPI::Unitset UnitsInRange = _unit->getUnitsInRadius(20);
	for(BWAPI::Unitset::iterator i=UnitsInRange.begin();i!=UnitsInRange.end();++i) {
		if ( (*i)->getType() == UnitTypes::Terran_Vulture_Spider_Mine ) {
			return true;
		}
	}
	return false;
}