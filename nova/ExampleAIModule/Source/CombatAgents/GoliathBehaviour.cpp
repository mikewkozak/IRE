#include "GoliathBehaviour.h"
#include "SquadAgent.h"

using namespace BWAPI;

GoliathBehaviour::GoliathBehaviour(Unit unit, class SquadAgent* squad)
	: _unit(unit),
	_squad(squad),
_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent.Goliath")),
_lastPosition(Positions::None)
{

}

void GoliathBehaviour::byDefault()
{

}

void GoliathBehaviour::onGetPosition(Position targetPosition)
{
	if (_unit->getOrder() == Orders::PlayerGuard) {
		_unit->attack(targetPosition);
	}
}

void GoliathBehaviour::onGetNewPosition(Position targetPosition)
{
	_unit->attack(targetPosition);
}


void GoliathBehaviour::onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies)
{

}

void GoliathBehaviour::onStop() {};
void GoliathBehaviour::onHold() {};