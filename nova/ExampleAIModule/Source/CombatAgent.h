#pragma once

#include "InformationManager.h"
#include "BuildManager.h"

#include "CombatAgents/InterfaceBehaviour.h"
#include "CombatAgents/VultureBehaviour.h"
#include "CombatAgents/MarineBehaviour.h"
#include "CombatAgents/ScienceVesselBehaviour.h"
#include "CombatAgents/DropshipBehaviour.h"
#include "CombatAgents/FirebatBehaviour.h"
#include "CombatAgents/GhostBehaviour.h"
#include "CombatAgents/SCVBehaviour.h"
#include "CombatAgents/TankBehaviour.h"
#include "CombatAgents/WraithBehaviour.h"
#include "CombatAgents/MedicBehaviour.h"
#include "CombatAgents/GoliathBehaviour.h"
#include "CombatAgents/BunkerBehaviour.h"

class CombatAgent
{
public:
	CombatAgent(BWAPI::Unit unit, class SquadAgent *squad);
	~CombatAgent();

	InterfaceBehaviour* behaviour;

	void defaultBehaviour();
	void onGetPosition(BWAPI::Position targetPosition);
    void onGetNewPosition(BWAPI::Position targetPosition);
	void onStop();
	void onHold();
	void inCombat(const BWAPI::Unitset &enemies, class SquadAgent *squad);
	void inCombatBiological(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies, class SquadAgent *squad);

	double computeTargetScore(BWAPI::Unit target);
	double dps(BWAPI::Unit unit, BWAPI::Unit target);
	double tacticalThreat(BWAPI::Unit unit, BWAPI::Unit target);

	BWAPI::Unit _unit;
	BWAPI::Unit _lastTarget;
	bool _inCooldown;
	BWAPI::Position _lastPosition;
	int frameCreated;

	int getEnemiesInRange(const BWAPI::Unitset &enemies);


private:
	log4cxx::LoggerPtr _logger;

	bool onlyBuildingEnemies(const BWAPI::Unitset &enemies);
	void isTankNear(BWAPI::Unit bestTarget, class SquadAgent *squad);
	bool protectTank(BWAPI::Unit bestTarget, class SquadAgent *squad);
};