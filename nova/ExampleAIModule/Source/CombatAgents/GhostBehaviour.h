#pragma once
#include "InterfaceBehaviour.h"
#include "InformationManager.h"

class GhostBehaviour : public InterfaceBehaviour
{
public:
	GhostBehaviour(BWAPI::Unit unit, class SquadAgent *squad);
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

	bool needCloak();
	bool allreadyFired(BWAPI::Unit enemy);
};