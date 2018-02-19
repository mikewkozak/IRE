#pragma once
#include "InterfaceBehaviour.h"
#include "InformationManager.h"

class SCVBehaviour : public InterfaceBehaviour
{
public:
	SCVBehaviour(BWAPI::Unit unit, class SquadAgent *squad);
	void onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies);
	void byDefault();
	void onGetPosition(BWAPI::Position targetPosition);
    void onGetNewPosition(BWAPI::Position targetPosition);
	void onStop();
	void onHold();

private:
	log4cxx::LoggerPtr _logger;
	BWAPI::Unit _unit;
	class SquadAgent* _squad;
	BWAPI::Position _lastPosition;
};