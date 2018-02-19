#include "SCVBehaviour.h"
#include "SquadAgent.h"
#include "CombatAgent.h"

using namespace BWAPI;

SCVBehaviour::SCVBehaviour(Unit unit, class SquadAgent* squad)
	: _unit(unit),
	_squad(squad),
	_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent.SCV")),
	_lastPosition(Positions::None)
{

}

void SCVBehaviour::byDefault()
{
	if (_unit->isIdle() && !_unit->isRepairing()) {
		Unit bunker = nullptr;
		Unit tank = nullptr;
		Unit vulture = nullptr;
		Unit mecha = nullptr;

		// TODO improve this, now it selects a random injured unit
		for (const auto & cu : _squad->_squadUnits) {
			if (cu->_unit && cu->_unit->getHitPoints() != cu->_unit->getType().maxHitPoints()) {
				if (cu->_unit->getType() == UnitTypes::Terran_Bunker)
					bunker = cu->_unit;
				else if (cu->_unit->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode || cu->_unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode)
					tank = cu->_unit;
				else if (_squad->_state == SquadAgent::GetPosition) {
					if (cu->_unit->getType() == UnitTypes::Terran_Vulture)
						vulture = cu->_unit;
					else if (cu->_unit->getType().isMechanical() && cu->_unit != _unit)
						mecha = cu->_unit;
				}
			}
		}

		if (bunker != nullptr) _unit->repair(bunker);
		else if (tank != nullptr) _unit->repair(tank);
		else if (vulture != nullptr) _unit->repair(vulture);
		else if (mecha != nullptr) _unit->repair(mecha);
	}
}

void SCVBehaviour::onGetPosition(Position targetPosition)
{
    LOG4CXX_TRACE(_logger,"SCV get position START");
	if (_unit->getOrder() == Orders::PlayerGuard) {
		// Follow closest tank
		CombatAgent* tank = _squad->getClosestUnitTo(_unit->getPosition(), UnitTypes::Terran_Siege_Tank_Tank_Mode);
		if (tank == 0) tank = _squad->getClosestUnitTo(_unit->getPosition(), UnitTypes::Terran_Siege_Tank_Siege_Mode);
		if (tank == 0) tank = _squad->getClosestUnitTo(_unit->getPosition(), UnitTypes::None);
        if (tank == 0) {
            LOG4CXX_TRACE(_logger,"SCV moving to position");
            _unit->move(targetPosition);
        } else {
            LOG4CXX_TRACE(_logger,"SCV following");
            _unit->follow(tank->_unit);
        }
	}
    LOG4CXX_TRACE(_logger,"SCV get position END");
}

void SCVBehaviour::onGetNewPosition(Position targetPosition)
{
    _unit->move(targetPosition);
}


void SCVBehaviour::onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies)
{

}

void SCVBehaviour::onStop() {};
void SCVBehaviour::onHold() {};