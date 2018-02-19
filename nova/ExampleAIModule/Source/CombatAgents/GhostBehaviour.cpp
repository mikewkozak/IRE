#include "GhostBehaviour.h"
#include "SquadAgent.h"

using namespace BWAPI;

GhostBehaviour::GhostBehaviour(Unit unit, class SquadAgent* squad)
	: _unit(unit),
	_squad(squad),
	_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent.Ghost")),
	_lastPosition(Positions::None)
{

}

void GhostBehaviour::byDefault()
{

}

void GhostBehaviour::onGetPosition(Position targetPosition)
{
	if (_unit->getOrder() == Orders::PlayerGuard) {
		_unit->attack(targetPosition);
	}
}

void GhostBehaviour::onGetNewPosition(Position targetPosition)
{
	_unit->attack(targetPosition);
}


void GhostBehaviour::onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies)
{
	BWAPI::Unitset UnitsInRange;
	if (Broodwar->self()->hasResearched(TechTypes::Lockdown)) {
		UnitsInRange = _unit->getUnitsInRadius(WeaponTypes::Lockdown.maxRange());
		for(BWAPI::Unitset::iterator i=UnitsInRange.begin();i!=UnitsInRange.end();++i) {
			if ( Broodwar->self()->isEnemy((*i)->getPlayer()) && (*i)->getType().isMechanical() && !(*i)->isLockedDown() && !allreadyFired(*i)) {
				// TODO energy ok?
				_unit->useTech(TechTypes::Lockdown, *i);
			}
		}
	}

	if (Broodwar->self()->hasResearched(TechTypes::Personnel_Cloaking)) {
		if (!_unit->isCloaked() && needCloak()) {
			// TODO energy ok?
			_unit->useTech(TechTypes::Personnel_Cloaking);
		}

		if (_unit->isCloaked() && !needCloak()) {
			_unit->useTech(TechTypes::Personnel_Cloaking);
		}
	}
}

bool GhostBehaviour::needCloak()
{
	BWAPI::Unitset UnitsInRange = _unit->getUnitsInRadius(Broodwar->self()->sightRange(_unit->getType()));
	for(BWAPI::Unitset::iterator i=UnitsInRange.begin();i!=UnitsInRange.end();++i) {
		if ( Broodwar->self()->isEnemy((*i)->getPlayer()) && (*i)->getType().canAttack() && !(*i)->isLockedDown() ) {
			return true;
		}
	}
	return false;
}

bool GhostBehaviour::allreadyFired(Unit enemy)
{
	BWAPI::Bulletset bullets = Broodwar->getBullets();
	for (BWAPI::Bulletset::iterator i = bullets.begin(); i != bullets.end(); ++i) {
		if ( (*i)->getSource() == _unit && (*i)->getTarget() == enemy) {
			return true;
		}
	}
	return false;
}

void GhostBehaviour::onStop() {};
void GhostBehaviour::onHold() {};