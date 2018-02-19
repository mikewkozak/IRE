#include "PlannerManager.h"

using namespace BWAPI;

/* overload the less-than operator so priority queues know how to compare two Height objects */
bool UnitTypePercent::operator<(const UnitTypePercent& right) const
{
	return percent < right.percent;
}

PlannerManager::PlannerManager()
{

};

void PlannerManager::onFrame()
{
	if (!informationManager->_percentList.empty()) {
		setBalance(informationManager->_percentList);
		informationManager->_percentList.clear();
	}

	// Print percent lists
	if (PRINT_PRODUCTION) {
		double barracksArmySize = informationManager->_marines + informationManager->_medics + informationManager->_firebats;
		double marinePercent = 0;
		double medicsPercent = 0;
		double firebatsPercent = 0;
		if (barracksArmySize != 0) {
			marinePercent = (informationManager->_marines/barracksArmySize) * 100;
			medicsPercent = (informationManager->_medics/barracksArmySize) * 100;
			firebatsPercent = (informationManager->_firebats/barracksArmySize) * 100;
		}
		double factoryArmySize = informationManager->_tank + informationManager->_vultures;
		double tankPercent = 0;
		double vulturePercent = 0;
		if (factoryArmySize != 0) {
			tankPercent = (informationManager->_tank/factoryArmySize) * 100;
			vulturePercent = (informationManager->_vultures/factoryArmySize) * 100;
		}

		Broodwar->drawTextScreen(280,78," Marines: %2.0f/%i", marinePercent, _percentList[UnitTypes::Terran_Marine] );
		Broodwar->drawTextScreen(280,91,"  Medics: %2.0f/%i", medicsPercent, _percentList[UnitTypes::Terran_Medic] );
		Broodwar->drawTextScreen(280,104,"Firebats: %2.0f/%i", firebatsPercent, _percentList[UnitTypes::Terran_Firebat] );
		Broodwar->drawTextScreen(280,117,"Next train: %s", informationManager->_trainOrder[UnitTypes::Terran_Barracks].c_str() );
		Broodwar->drawTextScreen(280,130,"   Tank: %2.0f/%i", tankPercent, _percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] );
		Broodwar->drawTextScreen(280,143,"Vulture: %2.0f/%i", vulturePercent, _percentList[UnitTypes::Terran_Vulture] );
		Broodwar->drawTextScreen(280,156,"Next train: %s", informationManager->_trainOrder[UnitTypes::Terran_Factory].c_str() );
		Broodwar->drawTextScreen(280,169,"Wraith: %i", Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Wraith) );
		Broodwar->drawTextScreen(280,182,"Vessel: %i", Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Science_Vessel) );
		Broodwar->drawTextScreen(280,195,"Next train: %s", informationManager->_trainOrder[UnitTypes::Terran_Starport].c_str() );

	}
}

void PlannerManager::setBalance(UnitToPercent percentList)
{
	// clean old percentList
	_percentList.clear();
 	_percentList[UnitTypes::Terran_Barracks] = 0;
 	_percentList[UnitTypes::Terran_Factory]	= 0;
 	_percentList[UnitTypes::Terran_Starport] = 0;
	// reset orders
	informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::None;
	informationManager->_trainOrder[UnitTypes::Terran_Factory]	= UnitTypes::None;
	informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::None;


	UnitType type;
	int percent;
	for(UnitToPercent::const_iterator it=percentList.begin();it!=percentList.end();++it) {
		type = it->first;
		percent = it->second;
		informationManager->checkRequirements(type); // Check building requirements
		if (percent > _percentList[type.whatBuilds().first]) {
			informationManager->_trainOrder[type.whatBuilds().first] = type;
			_percentList[type.whatBuilds().first] = percent;
		}
		_percentList[type] = percent;
		//Broodwar->printf("New unit order: %s rate %d", type.getName().c_str(), percent);
	}
}

void PlannerManager::rebalanceProduction()
{
	updateSelfArmy();
	updateEnemyArmy();

	double marines = informationManager->_marines;
	double medics = informationManager->_medics;
	double firebats = informationManager->_firebats;
	double ghosts = informationManager->_ghosts;
	double vultures = informationManager->_vultures;
	double tank = informationManager->_tank;
	double goliath = informationManager->_goliath;
	double wraiths = informationManager->_wraiths;
	double armySize = informationManager->_armySize;

	// avoid divide by 0
	if (armySize == 0) armySize = 0.1;

	if (_percentList[UnitTypes::Terran_Marine]+_percentList[UnitTypes::Terran_Firebat]+_percentList[UnitTypes::Terran_Medic] > 0) {
		double barracksArmySize = marines + medics + firebats + ghosts;
		if (barracksArmySize == 0) barracksArmySize = 0.1;
		
		// Balance Barrack production
		std::priority_queue <UnitTypePercent> pq;      //pq is a priority queue of UnitTypePercent objects
		double deviation;

		if (_percentList[UnitTypes::Terran_Marine]==0) deviation = -10;
		else deviation = ((double)_percentList[UnitTypes::Terran_Marine] / 100)-(marines/barracksArmySize);
		pq.push( UnitTypePercent(UnitTypes::Terran_Marine, deviation));
		if (_percentList[UnitTypes::Terran_Firebat]==0) deviation = -10;
		else deviation = ((double)_percentList[UnitTypes::Terran_Firebat] / 100)-(firebats/barracksArmySize);
		pq.push( UnitTypePercent(UnitTypes::Terran_Firebat, deviation));
		if (_percentList[UnitTypes::Terran_Medic]==0) deviation = -10;
		else deviation = ((double)_percentList[UnitTypes::Terran_Medic] / 100)-(medics/barracksArmySize);
		pq.push( UnitTypePercent(UnitTypes::Terran_Medic, deviation));

		informationManager->_trainOrder[UnitTypes::Terran_Barracks] = pq.top().type;
	}


// 	if ( (marines/barracksArmySize) < ((double)_percentList[UnitTypes::Terran_Marine] / 100) )
// 		informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::Terran_Marine;
// 	else if (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Academy) > 0)  {
// 		if ( (medics/barracksArmySize) < ((double)_percentList[UnitTypes::Terran_Medic] / 100) )
// 			informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::Terran_Medic;
// 		else {
// 			//informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::None;  //Stop production???
// 		}
// 	} else {
// 		//informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::None; //Stop production???
// 	}

	// Rebalance Factory production
	double factoryArmySize = tank + vultures;
	if (factoryArmySize == 0) factoryArmySize = 0.1;
	if ( (tank/factoryArmySize) < ((double)_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] / 100) )
		informationManager->_trainOrder[UnitTypes::Terran_Factory] = UnitTypes::Terran_Siege_Tank_Tank_Mode;
	else if ( (vultures/factoryArmySize) < ((double)_percentList[UnitTypes::Terran_Vulture] / 100) )
		informationManager->_trainOrder[UnitTypes::Terran_Factory] = UnitTypes::Terran_Vulture;
	else {
		//informationManager->_trainOrder[UnitTypes::Terran_Factory] = UnitTypes::None; //Stop production???
	}
	if (informationManager->_needAntiAirUnits) {
		informationManager->_trainOrder[UnitTypes::Terran_Factory] = UnitTypes::Terran_Goliath;
	}

	// Rebalance Starports production
	if ( (wraiths/armySize) < ((double)_percentList[UnitTypes::Terran_Wraith] / 100) )
		informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::Terran_Wraith;
	else {
		//informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::None; //Stop production???
	}
	if (informationManager->_scienceVesselDetector && Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Science_Vessel) < 2) {
		informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::Terran_Science_Vessel;
	} else if (informationManager->_trainOrder[UnitTypes::Terran_Starport] == UnitTypes::Terran_Science_Vessel) {
		informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::None;  
	}
	if (informationManager->_searchAndDestroy && Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Wraith) < 1) {
		informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::Terran_Wraith;
	} else if (_percentList[UnitTypes::Terran_Wraith] == 0) {
		informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::None;  
	}

}

void PlannerManager::updateSelfArmy()
{
	// Barrack units
	informationManager->_marines = (double)Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Marine);
	informationManager->_medics = (double)Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Medic);
	informationManager->_firebats = (double)Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Firebat);
	informationManager->_ghosts = (double)Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Ghost);
	informationManager->_armySize = informationManager->_marines + informationManager->_medics + informationManager->_firebats + informationManager->_ghosts;

	// Factory units
	informationManager->_vultures = (double)Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Vulture);
	informationManager->_tank = (double)Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Siege_Tank_Tank_Mode);
	informationManager->_tank += (double)Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Siege_Tank_Siege_Mode);
	informationManager->_goliath = (double)Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Goliath);
	informationManager->_armySize += informationManager->_vultures + informationManager->_tank + informationManager->_goliath;

	// Starport units
	informationManager->_wraiths = (double)Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Wraith);
	informationManager->_armySize += informationManager->_wraiths;

	informationManager->_ourAirDPS = 0;
	informationManager->_ourAntiAirHP = 0;
	informationManager->_ourGroundDPS = 0;
	informationManager->_ourAirHP = 0;
	informationManager->_ourGroundHP = 0;
	updateSelfArmyStats(UnitTypes::Terran_Marine, informationManager->_marines);
	updateSelfArmyStats(UnitTypes::Terran_Ghost, informationManager->_ghosts);
	updateSelfArmyStats(UnitTypes::Terran_Goliath, informationManager->_goliath);
	updateSelfArmyStats(UnitTypes::Terran_Wraith, informationManager->_wraiths);
}

void PlannerManager::updateEnemyArmy()
{
	// TERRAN ENEMY
	if (Broodwar->enemy()->getRace() == Races::Terran) {
		// Visible units
		// Barrack units
		informationManager->_enemyMarines = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Marine);
		informationManager->_enemyMedics = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Medic);
		informationManager->_enemyFirebats = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Firebat);
		informationManager->_enemyGhosts = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Ghost);
		// Factory units
		informationManager->_enemyVultures = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Vulture);
		informationManager->_enemyTank = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Siege_Tank_Tank_Mode);
		informationManager->_enemyTank += (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Siege_Tank_Siege_Mode);
		informationManager->_enemyGoliath = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Goliath);
		// Starport units
		informationManager->_enemyWraiths = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Wraith);
		informationManager->_enemyDropship = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Dropship);
		informationManager->_enemyScienceVessel = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Science_Vessel);
		informationManager->_enemyBattlecruiser = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Battlecruiser);
		informationManager->_enemyValkyrie = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Valkyrie);
		// Buildings can attack
		informationManager->_enemyTurrets = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Missile_Turret);

		// Seen units
		for (auto enemy : informationManager->seenEnemies) {
			if (enemy.second.type == UnitTypes::Terran_Marine) { informationManager->_enemyMarines++; continue; }
			if (enemy.second.type == UnitTypes::Terran_Medic) { informationManager->_enemyMedics++; continue; }
			if (enemy.second.type == UnitTypes::Terran_Firebat) { informationManager->_enemyFirebats++; continue; }
			if (enemy.second.type == UnitTypes::Terran_Ghost) { informationManager->_enemyGhosts++; continue; }
			// Factory units
			if (enemy.second.type == UnitTypes::Terran_Vulture) { informationManager->_enemyVultures++; continue; }
			if (enemy.second.type == UnitTypes::Terran_Siege_Tank_Tank_Mode) { informationManager->_enemyTank++; continue; }
			if (enemy.second.type == UnitTypes::Terran_Siege_Tank_Siege_Mode) { informationManager->_enemyTank++; continue; }
			if (enemy.second.type == UnitTypes::Terran_Goliath) { informationManager->_enemyGoliath++; continue; }
			// Starport units
			if (enemy.second.type == UnitTypes::Terran_Wraith) { informationManager->_enemyWraiths++; continue; }
			if (enemy.second.type == UnitTypes::Terran_Dropship) { informationManager->_enemyDropship++; continue; }
			if (enemy.second.type == UnitTypes::Terran_Science_Vessel) { informationManager->_enemyScienceVessel++; continue; }
			if (enemy.second.type == UnitTypes::Terran_Battlecruiser) { informationManager->_enemyBattlecruiser++; continue; }
			if (enemy.second.type == UnitTypes::Terran_Valkyrie) { informationManager->_enemyValkyrie++; continue; }
			// Buildings can attack
			if (enemy.second.type == UnitTypes::Terran_Missile_Turret) { informationManager->_enemyTurrets++; continue; }
		}

		// Update stats
		informationManager->_enemyArmySize = informationManager->_enemyMarines + informationManager->_enemyMedics + informationManager->_enemyFirebats + informationManager->_enemyGhosts;
		informationManager->_enemyArmySize += informationManager->_enemyVultures + informationManager->_enemyTank + informationManager->_enemyGoliath;
		informationManager->_enemyArmySize += informationManager->_enemyWraiths + informationManager->_enemyDropship + informationManager->_enemyScienceVessel + informationManager->_enemyBattlecruiser + informationManager->_enemyValkyrie;
		informationManager->_enemyArmySize += informationManager->_enemyTurrets;
		informationManager->_enemyAirDPS = 0;
		informationManager->_enemyAntiAirHP = 0;
		informationManager->_enemyGroundDPS = 0;
		informationManager->_enemyAirHP = 0;
		informationManager->_enemyGroundHP = 0;
		updateEnemyArmyStats(UnitTypes::Terran_Marine, informationManager->_enemyMarines);
		updateEnemyArmyStats(UnitTypes::Terran_Ghost, informationManager->_enemyGhosts);
		updateEnemyArmyStats(UnitTypes::Terran_Goliath, informationManager->_enemyGoliath);
		updateEnemyArmyStats(UnitTypes::Terran_Wraith, informationManager->_enemyWraiths);
		updateEnemyArmyStats(UnitTypes::Terran_Dropship, informationManager->_enemyDropship);
		updateEnemyArmyStats(UnitTypes::Terran_Science_Vessel, informationManager->_enemyScienceVessel);
		updateEnemyArmyStats(UnitTypes::Terran_Battlecruiser, informationManager->_enemyBattlecruiser);
		updateEnemyArmyStats(UnitTypes::Terran_Valkyrie, informationManager->_enemyValkyrie);
		updateEnemyArmyStats(UnitTypes::Terran_Missile_Turret, informationManager->_enemyTurrets);
	}
	// PROTOS ENEMY
	else if (Broodwar->enemy()->getRace() == Races::Protoss) {
		// Visible units
		// Gateway units
		informationManager->_enemyZealot = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Zealot);
		informationManager->_enemyDragoon = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Dragoon);
		informationManager->_enemyHTemplar = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_High_Templar);
		informationManager->_enemyDTemplar = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Dark_Templar);
		informationManager->_enemyArchon = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Archon);
		informationManager->_enemyDArchon = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Dark_Archon);
		// Robotics facility units
		informationManager->_enemyReaver = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Reaver);
		informationManager->_enemyObserver = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Observer);
		informationManager->_enemyShuttle = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Shuttle);
		// Stargate units
		informationManager->_enemyScout = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Scout);
		informationManager->_enemyCarrier = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Carrier);
		informationManager->_enemyArbiter = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Arbiter);
		informationManager->_enemyCorsair = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Corsair);
		// Buildings can attack
		informationManager->_enemyPhotonCanon = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Protoss_Photon_Cannon);

		// Seen units
		for (auto enemy : informationManager->seenEnemies) {
			if (enemy.second.type == UnitTypes::Protoss_Zealot) { informationManager->_enemyZealot++; continue; }
			if (enemy.second.type == UnitTypes::Protoss_Dragoon) { informationManager->_enemyDragoon++; continue; }
			if (enemy.second.type == UnitTypes::Protoss_High_Templar) { informationManager->_enemyHTemplar++; continue; }
			if (enemy.second.type == UnitTypes::Protoss_Dark_Templar) { informationManager->_enemyDTemplar++; continue; }
			if (enemy.second.type == UnitTypes::Protoss_Archon) { informationManager->_enemyArchon++; continue; }
			if (enemy.second.type == UnitTypes::Protoss_Dark_Archon) { informationManager->_enemyDArchon++; continue; }
			// Robotics facility units
			if (enemy.second.type == UnitTypes::Protoss_Reaver) { informationManager->_enemyReaver++; continue; }
			if (enemy.second.type == UnitTypes::Protoss_Observer) { informationManager->_enemyObserver++; continue; }
			if (enemy.second.type == UnitTypes::Protoss_Shuttle) { informationManager->_enemyShuttle++; continue; }
			// Stargate units
			if (enemy.second.type == UnitTypes::Protoss_Scout) { informationManager->_enemyScout++; continue; }
			if (enemy.second.type == UnitTypes::Protoss_Carrier) { informationManager->_enemyCarrier++; continue; }
			if (enemy.second.type == UnitTypes::Protoss_Arbiter) { informationManager->_enemyArbiter++; continue; }
			if (enemy.second.type == UnitTypes::Protoss_Corsair) { informationManager->_enemyCorsair++; continue; }
			// Buildings can attack
			if (enemy.second.type == UnitTypes::Protoss_Photon_Cannon) { informationManager->_enemyPhotonCanon++; continue; }
		}

		// Update stats
		informationManager->_enemyArmySize = informationManager->_enemyZealot + informationManager->_enemyDragoon + informationManager->_enemyHTemplar + informationManager->_enemyDTemplar;
		informationManager->_enemyArmySize += informationManager->_enemyArchon + informationManager->_enemyDArchon;
		informationManager->_enemyArmySize += informationManager->_enemyReaver + informationManager->_enemyObserver + informationManager->_enemyShuttle;
		informationManager->_enemyArmySize += informationManager->_enemyScout + informationManager->_enemyCarrier + informationManager->_enemyArbiter + informationManager->_enemyCorsair;
		informationManager->_enemyArmySize += informationManager->_enemyPhotonCanon;
		informationManager->_enemyAirDPS = 0;
		informationManager->_enemyAntiAirHP = 0;
		informationManager->_enemyGroundDPS = 0;
		informationManager->_enemyAirHP = 0;
		informationManager->_enemyGroundHP = 0;
		updateEnemyArmyStats(UnitTypes::Protoss_Zealot, informationManager->_enemyZealot);
		updateEnemyArmyStats(UnitTypes::Protoss_Dragoon, informationManager->_enemyDragoon);
		updateEnemyArmyStats(UnitTypes::Protoss_High_Templar, informationManager->_enemyHTemplar);
		updateEnemyArmyStats(UnitTypes::Protoss_Dark_Templar, informationManager->_enemyDTemplar);
		updateEnemyArmyStats(UnitTypes::Protoss_Archon, informationManager->_enemyArchon);
		updateEnemyArmyStats(UnitTypes::Protoss_Dark_Archon, informationManager->_enemyDArchon);
		updateEnemyArmyStats(UnitTypes::Protoss_Reaver, informationManager->_enemyReaver);
		updateEnemyArmyStats(UnitTypes::Protoss_Observer, informationManager->_enemyObserver);
		updateEnemyArmyStats(UnitTypes::Protoss_Shuttle, informationManager->_enemyShuttle);
		updateEnemyArmyStats(UnitTypes::Protoss_Scout, informationManager->_enemyScout);
		updateEnemyArmyStats(UnitTypes::Protoss_Carrier, informationManager->_enemyCarrier);
		updateEnemyArmyStats(UnitTypes::Protoss_Arbiter, informationManager->_enemyArbiter);
		updateEnemyArmyStats(UnitTypes::Protoss_Corsair, informationManager->_enemyCorsair);
		updateEnemyArmyStats(UnitTypes::Protoss_Photon_Cannon, informationManager->_enemyPhotonCanon);


	}
	// ZERG ENEMY
	else if (Broodwar->enemy()->getRace() == Races::Zerg) {
		// Visible units
		// Hatchery units
		informationManager->_enemyZergling = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Zerg_Zergling);
		informationManager->_enemyHydralisk = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Zerg_Hydralisk);
		informationManager->_enemyOverlord = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Zerg_Overlord);
		// Lair units
		informationManager->_enemyLurker = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Zerg_Lurker);
		informationManager->_enemyMutalisk = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Zerg_Mutalisk);
		// Hive units
		informationManager->_enemyUltralisk = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Zerg_Ultralisk);
		informationManager->_enemyGuardian = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Zerg_Guardian);
		informationManager->_enemyDevourer = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Zerg_Devourer);
		// Buildings can attack
		informationManager->_enemySunkenColony = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Zerg_Sunken_Colony);
		informationManager->_enemySporeColony = (double)Broodwar->enemy()->visibleUnitCount(UnitTypes::Zerg_Spore_Colony);

		// Seen units
		for (auto enemy : informationManager->seenEnemies) {
			if (enemy.second.type == UnitTypes::Zerg_Zergling) { informationManager->_enemyZergling++; continue; }
			if (enemy.second.type == UnitTypes::Zerg_Hydralisk) { informationManager->_enemyHydralisk++; continue; }
			if (enemy.second.type == UnitTypes::Zerg_Overlord) { informationManager->_enemyOverlord++; continue; }
			// Robotics facility units
			if (enemy.second.type == UnitTypes::Zerg_Lurker) { informationManager->_enemyLurker++; continue; }
			if (enemy.second.type == UnitTypes::Zerg_Mutalisk) { informationManager->_enemyMutalisk++; continue; }
			// Stargate units
			if (enemy.second.type == UnitTypes::Zerg_Ultralisk) { informationManager->_enemyUltralisk++; continue; }
			if (enemy.second.type == UnitTypes::Zerg_Guardian) { informationManager->_enemyGuardian++; continue; }
			if (enemy.second.type == UnitTypes::Zerg_Devourer) { informationManager->_enemyDevourer++; continue; }
			// Buildings can attack
			if (enemy.second.type == UnitTypes::Zerg_Sunken_Colony) { informationManager->_enemySunkenColony++; continue; }
			if (enemy.second.type == UnitTypes::Zerg_Spore_Colony) { informationManager->_enemySporeColony++; continue; }
		}

		// Update stats
		informationManager->_enemyArmySize = informationManager->_enemyZergling + informationManager->_enemyHydralisk + informationManager->_enemyOverlord;
		informationManager->_enemyArmySize += informationManager->_enemyLurker + informationManager->_enemyMutalisk;
		informationManager->_enemyArmySize += informationManager->_enemyUltralisk + informationManager->_enemyGuardian + informationManager->_enemyDevourer;
		informationManager->_enemyArmySize += informationManager->_enemySunkenColony + informationManager->_enemySporeColony;
		informationManager->_enemyAirDPS = 0;
		informationManager->_enemyAntiAirHP = 0;
		informationManager->_enemyGroundDPS = 0;
		informationManager->_enemyAirHP = 0;
		informationManager->_enemyGroundHP = 0;
		updateEnemyArmyStats(UnitTypes::Zerg_Zergling, informationManager->_enemyZergling);
		updateEnemyArmyStats(UnitTypes::Zerg_Hydralisk, informationManager->_enemyHydralisk);
		updateEnemyArmyStats(UnitTypes::Zerg_Overlord, informationManager->_enemyOverlord);
		updateEnemyArmyStats(UnitTypes::Zerg_Lurker, informationManager->_enemyLurker);
		updateEnemyArmyStats(UnitTypes::Zerg_Mutalisk, informationManager->_enemyMutalisk);
		updateEnemyArmyStats(UnitTypes::Zerg_Ultralisk, informationManager->_enemyUltralisk);
		updateEnemyArmyStats(UnitTypes::Zerg_Guardian, informationManager->_enemyGuardian);
		updateEnemyArmyStats(UnitTypes::Zerg_Devourer, informationManager->_enemyDevourer);
		updateEnemyArmyStats(UnitTypes::Zerg_Sunken_Colony, informationManager->_enemySunkenColony);
		updateEnemyArmyStats(UnitTypes::Zerg_Spore_Colony, informationManager->_enemySporeColony);


	}

}

void PlannerManager::updateEnemyArmyStats(UnitType type, double size)
{
	if (size==0) return;

	if (type.airWeapon().damageAmount() > 0 ) {
		informationManager->_enemyAirDPS += (type.airWeapon().damageAmount()*(24.0/type.airWeapon().damageCooldown())) * size;
		informationManager->_enemyAntiAirHP += ((type.maxShields() + type.maxHitPoints())) * size;
	}
	if (type.groundWeapon().damageAmount() > 0 )
		informationManager->_enemyGroundDPS += (type.groundWeapon().damageAmount()*(24.0/type.groundWeapon().damageCooldown())) * size;
	// In the case of Firebats and Zealots, the damage returned by BWAPI is not right, since they have two weapons:
	if (type == UnitTypes::Terran_Firebat || type == UnitTypes::Protoss_Zealot)
		informationManager->_enemyGroundDPS += type.groundWeapon().damageAmount();
	if (type.isFlyer())
		informationManager->_enemyAirHP += ((type.maxShields() + type.maxHitPoints())) * size;
	else
		informationManager->_enemyGroundHP += ((type.maxShields() + type.maxHitPoints())) * size;
}

void PlannerManager::updateSelfArmyStats(UnitType type, double size)
{
	if (size==0) return;

	if (type.airWeapon().damageAmount() > 0 ) {
		informationManager->_ourAirDPS += (type.airWeapon().damageAmount()*(24.0/type.airWeapon().damageCooldown())) * size;
		informationManager->_ourAntiAirHP += ((type.maxShields() + type.maxHitPoints())) * size;
	}
	if (type.groundWeapon().damageAmount() > 0 )
		informationManager->_ourGroundDPS += (type.groundWeapon().damageAmount()*(24.0/type.groundWeapon().damageCooldown())) * size;
	// In the case of Firebats, the damage returned by BWAPI is not right, since they have two weapons:
	if (type == UnitTypes::Terran_Firebat)
		informationManager->_ourGroundDPS += type.groundWeapon().damageAmount();
	if (type.isFlyer())
		informationManager->_ourAirHP += ((type.maxShields() + type.maxHitPoints())) * size;
	else
		informationManager->_ourGroundHP += ((type.maxShields() + type.maxHitPoints())) * size;
}