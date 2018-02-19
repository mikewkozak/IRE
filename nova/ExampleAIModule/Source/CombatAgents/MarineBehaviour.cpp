#include "MarineBehaviour.h"
#include "SquadAgent.h"

using namespace BWAPI;

MarineBehaviour::MarineBehaviour(Unit unit, class SquadAgent* squad)
	: _unit(unit),
	_squad(squad),
	_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent.Marine")),
	_lastPosition(Positions::None)
{

}

void MarineBehaviour::byDefault()
{

}

void MarineBehaviour::onGetPosition(Position targetPosition)
{
	if (_unit->getOrder() == Orders::PlayerGuard) {
		_unit->attack(targetPosition);
	}
}

void MarineBehaviour::onGetNewPosition(Position targetPosition)
{
	_unit->attack(targetPosition);
}


void MarineBehaviour::onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies)
{
	// Use stimpacks when there are medics around
	if (Broodwar->self()->hasResearched(TechTypes::Stim_Packs) && _squad->hasUnitOfType(UnitTypes::Terran_Medic)) {
		if (bestTarget!=0 && !_unit->isStimmed() && _unit->getHitPoints() == 40 && _unit->isAttacking()) {
			_unit->useTech(TechTypes::Stim_Packs);
		}
	}
}

void MarineBehaviour::onStop() {};
void MarineBehaviour::onHold() {};