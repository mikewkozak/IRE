#include "VsTerran.h"

using namespace BWAPI;

//------------------------------------------------------------------------
// Methods for 1Factory
//------------------------------------------------------------------------

OneFactory* OneFactory::Instance()
{
	static OneFactory instance;
	return &instance;
}

void OneFactory::Enter(StrategyManager* strategyManager)
{  
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Barracks);
	informationManager->buildRequest(UnitTypes::Terran_Refinery);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Factory);

	informationManager->_retreatDisabled = true;
}


void OneFactory::Execute(StrategyManager* strategyManager)
{
// 	if (workerManager->anyWorkerDefending() && Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Marine)==0) {
// 		informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::Terran_Marine;
// 	} else {
// 		informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::None;
// 	}

	// After place first factory, decide to prepare rush defenses or keep going
	if ( Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Factory) == 1) {
		int enemyBarracks = 0;
		for (auto enemy : informationManager->seenEnemies) {
			if (enemy.second.type == UnitTypes::Terran_Barracks) enemyBarracks++;
		}
		for (auto enemy : informationManager->visibleEnemies) {
			if (enemy.second.type == UnitTypes::Terran_Barracks) enemyBarracks++;
		}

		if (enemyBarracks == 1) { // All goes right
			strategyManager->GetFSM()->ChangeState(OneFactoryFastExpand::Instance());  
		} else { // prepare for coming rush
			strategyManager->GetFSM()->ChangeState(RushDefense::Instance()); 
		}
	}
}


void OneFactory::Exit(StrategyManager* strategyManager)
{
	//informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::None;
}

//------------------------------------------------------------------------
// Methods for 2 Factories Rush Defense
//------------------------------------------------------------------------

RushDefense* RushDefense::Instance()
{
	static RushDefense instance;
	return &instance;
}

void RushDefense::Enter(StrategyManager* strategyManager)
{  
	informationManager->buildRequest(UnitTypes::Terran_Factory);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->_percentList[UnitTypes::Terran_Vulture] = 100;
}


void RushDefense::Execute(StrategyManager* strategyManager)
{
	if (Broodwar->enemy()->getRace() == Races::Protoss && Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Vulture) == 4) {
		informationManager->researchRequest(TechTypes::Spider_Mines);
		//informationManager->upgradeRequest(UpgradeTypes::Ion_Thrusters);
	}

	if ( Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Vulture) == 8 || Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Siege_Tank_Tank_Mode) > 0) {
		strategyManager->GetFSM()->ChangeState(ThreeFactories::Instance());      
	}
}


void RushDefense::Exit(StrategyManager* strategyManager)
{
	informationManager->researchRequest(TechTypes::Tank_Siege_Mode);
	informationManager->naturalExpandRequest();
	if (Broodwar->enemy()->getRace() == Races::Protoss) {
		informationManager->_percentList[UnitTypes::Terran_Vulture] = 25;
		informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 75;
	} else {
		informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 100;
	}
}

//------------------------------------------------------------------------
// Methods for 1FactoryFastExpand
//------------------------------------------------------------------------

OneFactoryFastExpand* OneFactoryFastExpand::Instance()
{
	static OneFactoryFastExpand instance;
	return &instance;
}

void OneFactoryFastExpand::Enter(StrategyManager* strategyManager)
{  
	informationManager->buildRequest(UnitTypes::Terran_Machine_Shop);
	informationManager->researchRequest(TechTypes::Tank_Siege_Mode);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->naturalExpandRequest();
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Factory);
	informationManager->upgradeRequest(UpgradeTypes::Terran_Vehicle_Weapons, 1);
	informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 100;
	informationManager->_autoVehicleUpgrade = true;
}


void OneFactoryFastExpand::Execute(StrategyManager* strategyManager)
{
	if ( Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Siege_Tank_Tank_Mode)+Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Goliath) == 4 ) {
		strategyManager->GetFSM()->ChangeState(ThreeFactories::Instance());      
	}

	if (Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Marine) > 2) {
		informationManager->_trainOrder[UnitTypes::Terran_Factory] = UnitTypes::Terran_Vulture;
	} else if (informationManager->_trainOrder[UnitTypes::Terran_Factory] == UnitTypes::Terran_Vulture) {
		informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::Terran_Siege_Tank_Tank_Mode;
	}
}


void OneFactoryFastExpand::Exit(StrategyManager* strategyManager)
{
	
}

//------------------------------------------------------------------------
// Methods for 3Factories
//------------------------------------------------------------------------

ThreeFactories* ThreeFactories::Instance()
{
	static ThreeFactories instance;
	return &instance;
}

void ThreeFactories::Enter(StrategyManager* strategyManager)
{  
	informationManager->_autoBuildSuplies = true;
	if (informationManager->existGasExpand() ) {
		informationManager->buildRequest(UnitTypes::Terran_Factory);
		informationManager->gasExpandRequest();
		informationManager->buildRequest(UnitTypes::Terran_Factory);
		informationManager->buildRequest(UnitTypes::Terran_Factory);

		informationManager->upgradeRequest(UpgradeTypes::Terran_Vehicle_Weapons, 3);
		informationManager->upgradeRequest(UpgradeTypes::Terran_Vehicle_Plating, 3);
		informationManager->_autoVehicleUpgrade = true;

		informationManager->buildRequest(UnitTypes::Terran_Armory); // second armory

// 		informationManager->buildRequest(UnitTypes::Terran_Starport, true);
// 		informationManager->researchRequest(TechTypes::Cloaking_Field);
// 		informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::Terran_Wraith;
	}
}


void ThreeFactories::Execute(StrategyManager* strategyManager)
{
	// auto expand
	if (!buildManager->alreadyBuilding(UnitTypes::Terran_Command_Center) && !workerManager->needWorkers()) {
		if ( ( informationManager->minerals() > 1000 || (informationManager->minerals() > 500 && informationManager->gas() < 100) ) &&
			informationManager->existGasExpand() ) {
				informationManager->gasExpandRequest(true);
		}
		if (informationManager->minerals()*2 < informationManager->gas() && informationManager->existNaturalExpand())  {
			informationManager->naturalExpandRequest(true);
		}
	}

	// check marine late rush
	if ( informationManager->_ourBases.size() == 2 && Broodwar->enemy()->visibleUnitCount(UnitTypes::Terran_Marine) > 2) {
		informationManager->_trainOrder[UnitTypes::Terran_Factory] = UnitTypes::Terran_Vulture;
	}
	
	// check air enemy force
	if (Broodwar->enemy()->getRace() == Races::Terran) {   // *************** TERRAN RULES ****************************************************
		if (informationManager->_enemyTurrets == 0 && (informationManager->_enemyScienceVessel == 0 || informationManager->_enemyAirDPS < 1) ) {
			informationManager->buildRequest(UnitTypes::Terran_Starport, true);
			informationManager->researchRequest(TechTypes::Cloaking_Field);
			informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::Terran_Wraith;
		} else {
			informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::None;
		}
 	}


	// Have 4 Dropship in total
	/*if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Factory) >= 5) {
		if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Dropship) < 4) {
			informationManager->checkRequirements(UnitTypes::Terran_Dropship);
			informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::Terran_Dropship;   
		} else if (informationManager->_trainOrder[UnitTypes::Terran_Starport] == UnitTypes::Terran_Dropship) {
			informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::None;  
		}
	}*/



	// if enemy all mecha, produce Ghosts
// 	if (informationManager->_enemyMarines < 4 && informationManager->_enemyFirebats < 4 &&
// 		informationManager->_ghosts < 4) {
// 		// Check needed buildings
// 		informationManager->buildRequest(UnitTypes::Terran_Starport, true);
// 		informationManager->buildRequest(UnitTypes::Terran_Science_Facility, true);
// 		informationManager->buildRequest(UnitTypes::Terran_Covert_Ops, true);
// 		// Research
// 		informationManager->researchRequest(TechTypes::Lockdown);
// 		informationManager->upgradeRequest(UpgradeTypes::Ocular_Implants);
// 		informationManager->researchRequest(TechTypes::Personnel_Cloaking);
// 		if (Broodwar->self()->hasResearched(TechTypes::Lockdown)) {
// 			informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::Terran_Ghost;
// 		}
// 	} else {
// 		informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::None;
// 	}

}


void ThreeFactories::Exit(StrategyManager* strategyManager)
{

}