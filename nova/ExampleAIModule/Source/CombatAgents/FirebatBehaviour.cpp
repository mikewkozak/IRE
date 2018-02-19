#include "FirebatBehaviour.h"
#include "SquadAgent.h"

using namespace BWAPI;

FirebatBehaviour::FirebatBehaviour(Unit unit, class SquadAgent* squad)
	: _unit(unit),
	_squad(squad),
_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent.Firebat")),
_lastPosition(Positions::None)
{

}

void FirebatBehaviour::byDefault()
{

}

void FirebatBehaviour::onGetPosition(Position targetPosition)
{
	if (_unit->getOrder() == Orders::PlayerGuard) {
		_unit->attack(targetPosition);
	}
}

void FirebatBehaviour::onGetNewPosition(Position targetPosition)
{
	_unit->attack(targetPosition);
}


void FirebatBehaviour::onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies)
{

}

void FirebatBehaviour::onStop() {};
void FirebatBehaviour::onHold() {};