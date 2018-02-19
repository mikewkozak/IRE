#include "BunkerBehaviour.h"
#include "SquadAgent.h"
#include "SquadManager.h"

using namespace BWAPI;

BunkerBehaviour::BunkerBehaviour(Unit unit, class SquadAgent* squad)
	: _unit(unit),
	_squad(squad),
_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent.Bunker")),
_lastPosition(Positions::None)
{

}

void BunkerBehaviour::byDefault()
{
	int distanceToHome = _unit->getDistance(informationManager->home->getCenter());
	int distanceToEnemy = _unit->getDistance(informationManager->_enemyStartPosition);

	if (distanceToEnemy < distanceToHome) {
		// unload units if we are rushing (near enemy base) and no enemies near
		Unitset enemies = _unit->getUnitsInRadius(UnitTypes::Terran_Marine.sightRange(), Filter::IsEnemy && Filter::CanAttack);
		if (enemies.size() == 0 && _unit->getLoadedUnits().size() > 0) {
			_unit->unloadAll();
		}
	} else {
		// unload units if we are defending (near home base) and squad lead is closer to enemy
		if (squadManager->_leadSquad) {
			int distanceLeadToEnemy = (int)squadManager->_leadSquad->_center.getDistance(informationManager->_enemyStartPosition);
			if (distanceLeadToEnemy + 500 < distanceToEnemy) {
				_unit->unloadAll();
			}
		}
	}

}

void BunkerBehaviour::onGetPosition(Position targetPosition) {}
void BunkerBehaviour::onGetNewPosition(Position targetPosition) {}
void BunkerBehaviour::onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies) {}
void BunkerBehaviour::onStop() {}
void BunkerBehaviour::onHold() {}