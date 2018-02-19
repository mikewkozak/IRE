#include "CombatAgent.h"
#include "SquadAgent.h"
#include "SquadManager.h"

using namespace BWAPI;

#define NON_ATTACKABLE_UNIT	-10000

#define WEIGHT_AGGRO 10000
#define WEIGHT_DISTANCE 0.1
#define WEIGHT_TACTIC 1

// go close to a medic or backwards when low health:
#define MAX_DISTANCE_TO_MEDIC	64

CombatAgent::CombatAgent(Unit unit, SquadAgent* squad)
	: _unit(unit), 
	_lastTarget(0),
	_inCooldown(true),
	_lastPosition(Positions::None),
	_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent")),
	frameCreated(Broodwar->getFrameCount())
{
	LOG4CXX_TRACE(_logger, "Creating Combat Agent " << _unit->getType().c_str());

	if (_unit->getType() == UnitTypes::Terran_Vulture) {
		behaviour = new VultureBehaviour(unit, squad);
	} else if (_unit->getType() == UnitTypes::Terran_Marine) {
		behaviour = new MarineBehaviour(unit, squad);
	} else if (_unit->getType() == UnitTypes::Terran_Science_Vessel) {
		behaviour = new ScienceVesselBehaviour(unit, squad);
	} else if (_unit->getType() == UnitTypes::Terran_Dropship) {
		behaviour = new DropshipBehaviour(unit, squad);
	} else if (_unit->getType() == UnitTypes::Terran_Firebat) {
		behaviour = new FirebatBehaviour(unit, squad);
	} else if (_unit->getType() == UnitTypes::Terran_Ghost) {
		behaviour = new GhostBehaviour(unit, squad);
	} else if (_unit->getType() == UnitTypes::Terran_SCV) {
		behaviour = new SCVBehaviour(unit, squad);
	} else if (_unit->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode || _unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode) {
		behaviour = new TankBehaviour(unit, squad);
	} else if (_unit->getType() == UnitTypes::Terran_Wraith) {
		behaviour = new WraithBehaviour(unit, squad);
	} else if (_unit->getType() == UnitTypes::Terran_Medic) {
		behaviour = new MedicBehaviour(unit, squad);
	} else if (_unit->getType() == UnitTypes::Terran_Goliath) {
		behaviour = new GoliathBehaviour(unit, squad);
	} else if (_unit->getType() == UnitTypes::Terran_Bunker) {
		behaviour = new BunkerBehaviour(unit, squad);
	} else {
		LOG4CXX_ERROR(_logger, "Missing unit type behaviour for " << _unit->getType().c_str());
		behaviour = new MarineBehaviour(unit, squad);
	}

	LOG4CXX_TRACE(_logger, "Creating Combat Agent [DONE]");
}

CombatAgent::~CombatAgent()
{
    delete behaviour;
}

void CombatAgent::defaultBehaviour()
{
	behaviour->byDefault();
}

void CombatAgent::onGetPosition(Position targetPosition)
{
	behaviour->onGetPosition(targetPosition);
}

void CombatAgent::onGetNewPosition(Position targetPosition)
{
	behaviour->onGetNewPosition(targetPosition);
}

void CombatAgent::onStop() {behaviour->onStop();};
void CombatAgent::onHold() {behaviour->onHold();};

void CombatAgent::inCombat(const BWAPI::Unitset &enemies, SquadAgent *squad)
{
	// special micromanagement for units that do not require target selection:
	if (_unit->getType() == UnitTypes::Terran_Medic) {
		behaviour->onCombat(NULL,enemies);
		return;
	} else if (_unit->getType() == UnitTypes::Terran_Dropship) {
		// Dropships micro is called directly from the squad agent, so that it-s always executed, and not only during combat
		return;
	} else if (_unit->getType() == UnitTypes::Terran_SCV) return;

	double bestScore = 0;
	Unit bestTarget = 0;

	for (BWAPI::Unitset::iterator target = enemies.begin(); target != enemies.end(); ++target) {
		if (_unit->getType().isFlyer() && (*target)->getType() == UnitTypes::Terran_Missile_Turret) continue; //ignore missile turret
		if ((*target)->getType().isFlyer() && _unit->getType().airWeapon().damageAmount() == 0) continue;
		double score = computeTargetScore(*target);
		if (bestTarget==0 || score > bestScore) {
			bestScore = score;
			bestTarget = *target;
		}
	}

	if ((_unit->getType() ==  UnitTypes::Terran_Marine || _unit->getType() ==  UnitTypes::Terran_Firebat) && protectTank(bestTarget,squad)) {
		return; //if we are protecting a tank, hold position
	}

	if (bestScore == NON_ATTACKABLE_UNIT) { // ignore non attackable units
		//if (_unit->isSieged()) unsiegeRequest();
		//else _unit->attack(squad->_positionTarget); 
		_unit->attack(squad->_positionTarget); 
	} else if (bestTarget != _lastTarget && bestTarget != 0) {
		if (_unit->getType() ==  UnitTypes::Terran_Vulture) {
			if (bestTarget->getType() != UnitTypes::Protoss_Photon_Cannon && bestTarget->getType() != UnitTypes::Protoss_Dragoon)	{
				int x = _unit->getTilePosition().x;
				int y = _unit->getTilePosition().y;
				if (informationManager->get_enemy_ground_dps(x, y,BWAPI::Broodwar->getFrameCount())==0)  _unit->attack(bestTarget);
			} else isTankNear(bestTarget,squad);
			//else Do nothing (patrol inCombatVulture()
		} else  {
			_unit->attack(bestTarget);
			//Broodwar->printf("Unit %s attacking target %s", _unit->getType().c_str(), bestTarget->getType().getName().c_str());
		}
		_lastTarget = bestTarget;

		// Sanitize target
		if (bestTarget->getType() == UnitTypes::Resource_Vespene_Geyser) {
			squad->_enemies.erase(bestTarget);
		}
	}

	if (bestTarget != NULL && bestTarget->isVisible() && !bestTarget->isDetected() ) {
		//Broodwar->printf("Target %s is cloaked/Burrowed", _lastTarget->getType().getName().c_str());
		informationManager->seenCloakedEnemy(bestTarget->getTilePosition());
		usingCloackUnits = true;
		// TODO if we have detectors, call it, else inform to informationManager
	}

	// special micromanagement for unit that require target selection:
	behaviour->onCombat(bestTarget,enemies);
	if (_unit->getType() == UnitTypes::Terran_Marine || 
		_unit->getType() == UnitTypes::Terran_Ghost ||
		_unit->getType() == UnitTypes::Terran_Firebat) inCombatBiological(bestTarget,enemies,squad);


//  if (_unit->getGroundWeaponCooldown() > 0 && getEnemiesInRange() > 3 && _unit->isUnderAttack()) { //retreat
// 		_inCooldown = true;
// 		_unit->move(Position(6*TILE_SIZE,36*TILE_SIZE));
// 	} else if (_inCooldown || bestTarget != _lastTarget) { //attack
// 		_inCooldown = false;
// 		if (bestScore > 0) _unit->attack(bestTarget);
// 		_lastTarget = bestTarget;
//  }
}



void CombatAgent::inCombatBiological(Unit bestTarget, const BWAPI::Unitset &enemies, SquadAgent *squad)
{
	double health_level = double(_unit->getHitPoints())/double(_unit->getType().maxHitPoints());

	if (!_unit->isAttacking()) {
		// find the nearest medic and the unit with the minimum health level:
		double minimum_hl = 1, hl;
		CombatAgent *minimum_hl_unit = 0;
		int distance = 0, newDistance = 0;
		CombatAgent *closestUnit = 0;
		for(CombatUnitSet::const_iterator i=squad->_squadUnits.begin();i!=squad->_squadUnits.end();++i) {
			if ((*i)->_unit!=0) {
				if ((*i)->_unit->getType() == UnitTypes::Terran_Medic) {
					newDistance = (*i)->_unit->getPosition().getApproxDistance(_unit->getPosition());
					if (closestUnit == 0 || newDistance < distance) {
						distance = newDistance;
						closestUnit = *i;
					}
				}
				hl = double((*i)->_unit->getHitPoints())/double((*i)->_unit->getType().maxHitPoints());
				if (hl<minimum_hl) {
					minimum_hl = hl;
					minimum_hl_unit = *i;
				}
			}
		}

//		if (health_level<0.5 || minimum_hl_unit == this) {
		if (health_level<0.5) {
			if (closestUnit!=0 && distance>MAX_DISTANCE_TO_MEDIC) {
				_unit->move(closestUnit->_unit->getPosition());
			}
		}
	}
}


void CombatAgent::isTankNear(Unit bestTarget, SquadAgent *squad)
{
	LOG4CXX_TRACE(_logger,"Checking tank near");
	// get closest tank
	CombatAgent* tank = squad->getClosestUnitTo(_unit->getPosition(), UnitTypes::Terran_Siege_Tank_Siege_Mode);
	if (tank == 0) tank = squad->getClosestUnitTo(_unit->getPosition(), UnitTypes::Terran_Siege_Tank_Tank_Mode);
	if (tank == 0) {
		// no tank in squad, keep attacking
		int x = _unit->getTilePosition().x;
		int y = _unit->getTilePosition().y;
		if (informationManager->get_enemy_ground_dps(x, y,BWAPI::Broodwar->getFrameCount())==0)  _unit->attack(bestTarget);
	} else {
		// retread to tank
		//Broodwar->printf("Vulture moving next to tank");
		_unit->move(tank->_unit->getPosition());
	}
}

bool CombatAgent::protectTank(Unit bestTarget, SquadAgent *squad)
{
	LOG4CXX_TRACE(_logger,"Checking protecting tank");
	// get closest tank
	//CombatAgent* tank = squad->getClosestUnitTo(_unit->getPosition(), UnitTypes::Terran_Siege_Tank_Siege_Mode, true, false);
	CombatAgent* tank = squad->getClosestUnitTo(squad->_positionTarget, UnitTypes::Terran_Siege_Tank_Siege_Mode, true, false);
	if (tank == 0) {
		// no tank in squad, keep attacking
		//_unit->attack(bestTarget);
		return false;
	} else {
		informationManager->_center = tank->_unit->getPosition();
		informationManager->_radius = 70;
		int distance = tank->_unit->getPosition().getApproxDistance(_unit->getPosition());
		if (distance > 70) {
			//Broodwar->printf("Bio moving next to tank");
			// TODO we are spaming this action
			_unit->move(tank->_unit->getPosition());
		} else if (!_unit->isHoldingPosition()) {
			//Broodwar->printf("Bio hold position");
			_unit->holdPosition();
		}
		return true;
	}
}


bool CombatAgent::onlyBuildingEnemies(const BWAPI::Unitset &enemies)
{
	for (BWAPI::Unitset::iterator it = enemies.begin(); it != enemies.end(); ++it) {
		if ( !(*it)->getType().isBuilding() )
			return false;
	}
	return true;
}


// DPS of unit to target:
double CombatAgent::dps(BWAPI::Unit unit, BWAPI::Unit target)
{
	if (unit==0 || target==0) {
		//DEBUG("[ERROR] No wrong unit in DPS");
		return 0;
	}

	UnitType unit_t = unit->getType();
	UnitType target_t = target->getType();
	double res_dps = 0.0;

	if (target_t.isFlyer()) {
		if (unit_t.airWeapon().damageAmount() != 0) {
//			DEBUG("DAMAGE AIR: " << unit_t.getName() << " " << unit_t.groundWeapon().damageAmount() << " " << unit_t.groundWeapon().damageCooldown()); 
			res_dps = unit_t.airWeapon().damageAmount()*(24.0/unit_t.airWeapon().damageCooldown());
			if (unit_t.airWeapon().damageType() == DamageTypes::Explosive) {
				if (target_t.size() == UnitSizeTypes::Small) res_dps *= 0.5;
				if (target_t.size() == UnitSizeTypes::Medium) res_dps *= 0.75;
			}
			if (unit_t.airWeapon().damageType() == DamageTypes::Concussive) {
				if (target_t.size() == UnitSizeTypes::Large) res_dps *= 0.25;
				if (target_t.size() == UnitSizeTypes::Medium) res_dps *= 0.5;
			}
		}
	} else {
		if (unit_t.groundWeapon().damageAmount() != 0) {
//			DEBUG("DAMAGE GROUND: " << unit_t.getName() << " " << unit_t.groundWeapon().damageAmount() << " " << unit_t.groundWeapon().damageCooldown()); 
			// In the case of Firebats and Zealots, the damage returned by BWAPI is not right, since they have two weapons:
			if (unit_t == UnitTypes::Terran_Firebat ||
				unit_t == UnitTypes::Protoss_Zealot) {
				res_dps = unit_t.groundWeapon().damageAmount()*2*(24.0/unit_t.groundWeapon().damageCooldown());
			} else {
				res_dps = unit_t.groundWeapon().damageAmount()*(24.0/unit_t.groundWeapon().damageCooldown());
			}
			if (unit_t.groundWeapon().damageType() == DamageTypes::Explosive) {
				if (target_t.size() == UnitSizeTypes::Small) res_dps *= 0.5;
				if (target_t.size() == UnitSizeTypes::Medium) res_dps *= 0.75;
			}
			if (unit_t.groundWeapon().damageType() == DamageTypes::Concussive) {
				if (target_t.size() == UnitSizeTypes::Large) res_dps *= 0.25;
				if (target_t.size() == UnitSizeTypes::Medium) res_dps *= 0.5;
			}
		}
	}


/*
	// If it's a transport/bunker/etc, count the DPS of the units inside:
	// This does not work, since we can't see the units inside enemy bunkers or transports
	std::set<Unit *> loadedUnits = unit->getLoadedUnits();
	if (loadedUnits.size()!=0) {
		for (std::set<Unit *>::const_iterator it = loadedUnits.begin(); it != loadedUnits.end(); ++it) {
			res_dps += dps(*it,target);
		}	
		return res_dps;
	}
*/

	// Bunkers are a special case, and we assume there are 4 marines inside:
	if (unit_t == UnitTypes::Terran_Bunker) {
		res_dps+= 4 * 8*(24.0/15);	// 4 units, 6 damage, 15 cool down (i.e. 4 marines)
	}

	// Ignore non-threatening workers
	if (target_t.isWorker() && !(target->isRepairing() || target->getOrder()==Orders::AttackUnit || target->isConstructing() )) {
		res_dps = 1.0/WEIGHT_AGGRO;
	}

	return res_dps;
}


double CombatAgent::tacticalThreat(Unit unit, Unit target)
{
	if (unit==0 || target==0) return 0;

	UnitType ut = unit->getType();
	UnitType tt = target->getType();

	// Healers
	if (tt == UnitTypes::Terran_Medic) return 50; //100
	if (tt == UnitTypes::Terran_SCV) {
		if (target->isRepairing()) return 400;
		else return 250;
	}

	// Workers
	if (tt.isWorker()) {
		if (ut == UnitTypes::Terran_Vulture) return 2500;
		if (target->isConstructing()) return 2500;
		return 250;
	}

	// Detectors
	if (tt == UnitTypes::Terran_Science_Vessel) return 10; //100
	if (tt == UnitTypes::Protoss_Observer) return 500; //100
	// Overlord has more threat as carrier

	// Carriers of other units:
	if (tt == UnitTypes::Terran_Dropship) return 10; //400
	if (tt == UnitTypes::Protoss_Shuttle) return 10; //400
	if (tt == UnitTypes::Zerg_Overlord) return 10; //400

	if (tt == UnitTypes::Protoss_Pylon) return 10; //50
	//if (tt == UnitTypes::Protoss_Shield_Battery) return 50;
	if (tt == UnitTypes::Zerg_Queen) return 10; //100

	// Bunkers are taken into account in the DPS:
//	if (tt == UnitTypes::Terran_Bunker) return 50;

	// Buildings that can attack: (they are already took into account in the DPS)
//	if (ut.isFlyer() && tt == UnitTypes::Terran_Missile_Turret) return 300;
//	if (tt == UnitTypes::Protoss_Photon_Cannon) return 50; //300
//	if (ut.isFlyer() && tt == UnitTypes::Zerg_Spore_Colony) return 300;
	if (!ut.isFlyer() && tt == UnitTypes::Zerg_Sunken_Colony) return 30000;

	// Buildings that can produce units:
	if (tt == UnitTypes::Terran_Barracks) return 10; //200
	if (tt == UnitTypes::Terran_Factory) return 10; //200
	if (tt == UnitTypes::Terran_Starport) return 10; //200
	if (tt == UnitTypes::Protoss_Gateway) return 10; //200
	if (tt == UnitTypes::Protoss_Robotics_Facility) return 10; //200
	if (tt == UnitTypes::Protoss_Stargate) return 10; //200

	if (tt == UnitTypes::Zerg_Hatchery) return 25;  //200
	if (tt == UnitTypes::Zerg_Lair) return 25; //200
	if (tt == UnitTypes::Zerg_Hive) return 25; //200
// 	if (tt == UnitTypes::Protoss_Nexus) return 25;
// 	if (tt == UnitTypes::Terran_Command_Center) return 25;

// 	if (tt == UnitTypes::Zerg_Lurker) return 500;
// 	if (tt == UnitTypes::Protoss_Dark_Templar) return 500;
// 	if (tt == UnitTypes::Protoss_High_Templar) return 500;

	// Command centers:
	if (tt.isResourceDepot()) return 10; //100

	if (tt.isRefinery()) return 10; //75
	if (tt.canProduce()) return 10; //200

	// if we are a flyer and target can attack
	if (ut.isFlyer() && tt.canAttack()) return 2500;

	// if we are a goliath and target is a flyer
	if (ut == UnitTypes::Terran_Goliath && tt.isFlyer()) return 2500;

	if (ut == UnitTypes::Terran_Vulture && tt == UnitTypes::Protoss_Zealot) return 2500;

	return 0.0;
}

double CombatAgent::computeTargetScore(Unit target)
{
	double score = 0;
	UnitType target_t = target->getType();
	UnitType unit_t = _unit->getType();

	// Compute distance only for units that can attack:
	int currentRange = 0;
	//if (!target_t.isBuilding()) currentRange = (int) _unit->getDistance(target->getPosition());
	if (target_t.canAttack()) currentRange = (int) _unit->getDistance(target->getPosition());

	// aggro is proportional to the dps_target_to_unit and inversely proportional to the time it will take to kill the target (hp/dps_unit_to_target)
	double hp = target->getHitPoints() + target->getShields();
	// We don't know the hp of a hidden unit
	if (target->isBurrowed() || target->isCloaked()) hp = target_t.maxHitPoints(); //assume worst case
	double dps_target_to_unit = dps(target,_unit);
	double dps_unit_to_target = dps(_unit,target);

//	Broodwar->drawTextMap(target->getPosition().x, target->getPosition().y-15,"%c%s %0.2f", 0x03, target_t.getName().c_str(), dps_unit_to_target );

	if (dps_unit_to_target == 0.0) return NON_ATTACKABLE_UNIT;	// if we cannot damage the unit, do not attack it!

	double time_to_kill = hp/dps_unit_to_target;

	double aggro = (time_to_kill == 0 ? 0 : dps_target_to_unit/time_to_kill);
	double tactical = tacticalThreat(_unit,target);
	score += aggro * WEIGHT_AGGRO + tactical * WEIGHT_TACTIC - currentRange * WEIGHT_DISTANCE;

	LOG4CXX_DEBUG(_logger,"Unit type: " << target_t.getName() << " aggro: " << aggro << " tactical: " << tactical << " currentRange: " << currentRange << " score: " << score);
	if (unit_t == UnitTypes::Terran_Vulture) {
		// TODO this is ugly hack!!!
		//score -= currentRange * (1-WEIGHT_DISTANCE); //give more importance to distance
		//score -= aggro * WEIGHT_AGGRO; // give less importance to distance
		//score -= tactical * (WEIGHT_TACTIC/2);
		// if melee unit
		if (target_t.canAttack() && target_t.seekRange() < 50) score += 200;
	}
	LOG4CXX_DEBUG(_logger," adjust score: " << score << " seekRange " << target_t.seekRange());

	// if we are a siege tank and friendly units are close to target, penalize score
	// TODO do this for all types of splash damage
	// http://www.the-spoiler.com/STRATEGY/Blizzard/starcraft.expansion.2.html
	if (unit_t == UnitTypes::Terran_Siege_Tank_Siege_Mode) {
		BWAPI::Unitset UnitsInRange = target->getUnitsInRadius(40);
		for(BWAPI::Unitset::iterator i=UnitsInRange.begin();i!=UnitsInRange.end();++i) {
			if (Broodwar->self() == (*i)->getPlayer()) {
				score -= 2000 * WEIGHT_TACTIC;
#ifndef TOURNAMENT
				Broodwar->drawCircleMap((*i)->getPosition().x, (*i)->getPosition().y,8,Colors::Red,true);
#endif
				return NON_ATTACKABLE_UNIT;
			} else if (Broodwar->self()->isEnemy((*i)->getPlayer())) { // enemy unit
				score += 2000 * WEIGHT_TACTIC;
			}
		}
	}

	if ( _unit->isSelected() ) {
		Broodwar->drawTextMap(target->getPosition().x, target->getPosition().y-5,"%d", currentRange);
		Broodwar->drawTextMap(target->getPosition().x, target->getPosition().y-15,"%0.2f", score);
//		Broodwar->drawTextMap(target->getPosition().x, target->getPosition().y-15,"%s %0.2f", target_t.getName().c_str(), score);
	}
//	DEBUG("Score (" << target_t.getName() << "): " << aggro << " " << tactical << " " << currentRange << "(dps " << dps_target_to_unit << "," << dps_unit_to_target << ")");

	return score;
}

int CombatAgent::getEnemiesInRange(const BWAPI::Unitset &enemies)
{
	int enemiesInRange = 0;
	for (BWAPI::Unitset::iterator it = enemies.begin(); it != enemies.end(); ++it)
	{
		Unit target = *it;
		if (target->isInWeaponRange(_unit))
			++enemiesInRange;
	}
	return enemiesInRange;
}
