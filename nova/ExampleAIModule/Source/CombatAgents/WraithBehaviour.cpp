#include "WraithBehaviour.h"
#include "SquadAgent.h"

using namespace BWAPI;

WraithBehaviour::WraithBehaviour(Unit unit, class SquadAgent* squad)
	: _unit(unit),
	_squad(squad),
	_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent.Wraith")),
	_lastPosition(Positions::None)
{

}

void WraithBehaviour::byDefault()
{

}

void WraithBehaviour::onGetPosition(Position targetPosition)
{
	if (_unit->getOrder() == Orders::PlayerGuard) {
		_unit->attack(targetPosition);
	}
}

void WraithBehaviour::onGetNewPosition(Position targetPosition)
{
	_unit->attack(targetPosition);
}

void WraithBehaviour::onStop() {};
void WraithBehaviour::onHold() {};


void WraithBehaviour::onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies)
{
	if (bestTarget == NULL) return;

	if (_unit->getGroundWeaponCooldown()==0 && bestTarget!=0) {
		_unit->attack(bestTarget);
		_lastPosition = Positions::None;
		return;
	}

	// if target has better attack range than us, don't flee
	int unitMaxRange = 0;
	if (bestTarget->getType().isFlyer()) unitMaxRange = bestTarget->getType().airWeapon().maxRange();
	else unitMaxRange = bestTarget->getType().groundWeapon().maxRange();
	if (bestTarget->getType().airWeapon().maxRange() > unitMaxRange) return;

	// Safest spot is: minimum dps, furthest from the target enemy, but keeping it in range
	int x = _unit->getTilePosition().x;
	int y = _unit->getTilePosition().y;
	if (!_unit->isCloaked() && informationManager->get_enemy_air_dps(x, y,BWAPI::Broodwar->getFrameCount())>0) {
		// Find best position in spiral
		Position returnPosition;
		int length = 1;
		int j      = 0;
		bool first = true;
		int dx     = 0;
		int dy     = 1;	
		while (length < Broodwar->mapWidth()) {
			returnPosition = Position(x*TILE_SIZE, y*TILE_SIZE);

			if (x >= 0 && x < Broodwar->mapWidth() && y >= 0 && y < Broodwar->mapHeight() ) {
				double dps = informationManager->get_enemy_air_dps(x, y,BWAPI::Broodwar->getFrameCount());
				if (dps == 0) {
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

	if (Broodwar->self()->hasResearched(TechTypes::Cloaking_Field)) {
		if (!_unit->isCloaked() && needCloakWraith()) {
			// TODO energy ok?
			_unit->useTech(TechTypes::Cloaking_Field);
		}

		if (_unit->isCloaked() && !needCloakWraith()) {
			_unit->useTech(TechTypes::Cloaking_Field);
		}
	}
}

bool WraithBehaviour::needCloakWraith()
{
	bool antiAirDPS = false;
	bool detector = false;
	BWAPI::Unitset UnitsInRange = _unit->getUnitsInRadius(Broodwar->self()->sightRange(_unit->getType()));
	for(BWAPI::Unitset::iterator i=UnitsInRange.begin();i!=UnitsInRange.end();++i) {
		if ( Broodwar->self()->isEnemy((*i)->getPlayer()) && (*i)->getType().airWeapon().damageAmount() != 0 && !(*i)->isLockedDown() ) {
			antiAirDPS = true;
		}
		if ( (*i)->getType().isDetector() ) detector = true;
	}

	if (antiAirDPS && !detector) return true;
	else return false;
}