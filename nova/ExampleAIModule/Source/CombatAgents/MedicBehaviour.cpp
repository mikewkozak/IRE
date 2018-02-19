#include "MedicBehaviour.h"
#include "SquadAgent.h"

using namespace BWAPI;

// make sure medics stay with the group:
#define MAX_DISTNACE_TO_NONMEDIC	64

MedicBehaviour::MedicBehaviour(Unit unit, class SquadAgent* squad)
	: _unit(unit),
	_squad(squad),
	_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent.Medic")),
	_lastPosition(Positions::None)
{

}

void MedicBehaviour::byDefault()
{

}

void MedicBehaviour::onGetPosition(Position targetPosition)
{
	if (_unit->getOrder() == Orders::PlayerGuard) {
		_unit->move(targetPosition);
	}
}

void MedicBehaviour::onGetNewPosition(Position targetPosition)
{
	_unit->move(targetPosition);
}


void MedicBehaviour::onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies)
{
	// First, use optical flare:
	BWAPI::Unitset UnitsInRange;
	if (Broodwar->self()->hasResearched(TechTypes::Optical_Flare) && _unit->getEnergy()>75) {
		UnitsInRange = _unit->getUnitsInRadius(WeaponTypes::Lockdown.maxRange());
		for(BWAPI::Unitset::iterator i=UnitsInRange.begin();i!=UnitsInRange.end();++i) {
			if ( Broodwar->self()->isEnemy((*i)->getPlayer()) && !(*i)->isBlind() && !allreadyFired(*i)) {
				_unit->useTech(TechTypes::Optical_Flare, *i);
			}
		}
	}


	// compute distance to nearest healable non-medic unit (for now, ignore the fact that medics can heal each other):
	int distance = 0, newDistance = 0;
	CombatAgent *closestUnit = 0;
	for(CombatUnitSet::const_iterator i=_squad->_squadUnits.begin();i!=_squad->_squadUnits.end();++i) {
		if ((*i)->_unit!=0 &&
			((*i)->_unit->getType() == UnitTypes::Terran_Marine ||
			 (*i)->_unit->getType() == UnitTypes::Terran_Firebat ||
			 (*i)->_unit->getType() == UnitTypes::Terran_Ghost ||
			 (*i)->_unit->getType() == UnitTypes::Terran_SCV)) {
			newDistance = (*i)->_unit->getPosition().getApproxDistance(_unit->getPosition());
			if (closestUnit == 0 || newDistance < distance) {
				distance = newDistance;
				closestUnit = *i;
			}
		}
	}

	// if it-s beyond the threshold, find closest unit, and send there:
	if (closestUnit!=0 && distance>MAX_DISTNACE_TO_NONMEDIC) {
		_unit->move(closestUnit->_unit->getPosition());
	}
}

bool MedicBehaviour::allreadyFired(Unit enemy)
{
	BWAPI::Bulletset bullets = Broodwar->getBullets();
	for (BWAPI::Bulletset::iterator i = bullets.begin(); i != bullets.end(); ++i) {
		if ( (*i)->getSource() == _unit && (*i)->getTarget() == enemy) {
			return true;
		}
	}
	return false;
}

void MedicBehaviour::onStop() {};
void MedicBehaviour::onHold() {};