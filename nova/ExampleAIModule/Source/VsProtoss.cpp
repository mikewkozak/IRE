#include "VsProtoss.h"

using namespace BWAPI;

//------------------------------------------------------------------------
// Methods for 2 Fact Vult/Mines
//------------------------------------------------------------------------

TwoFactMines* TwoFactMines::Instance()
{
	static TwoFactMines instance;
	return &instance;
}

void TwoFactMines::Enter(StrategyManager* strategyManager)
{
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Barracks);
	informationManager->buildRequest(UnitTypes::Terran_Refinery);
	informationManager->buildRequest(UnitTypes::Terran_Factory);
	informationManager->buildRequest(UnitTypes::Terran_Machine_Shop);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Factory);
	informationManager->buildRequest(UnitTypes::Terran_Machine_Shop);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);

	informationManager->researchRequest(TechTypes::Spider_Mines);
	informationManager->upgradeRequest(UpgradeTypes::Ion_Thrusters);

	informationManager->researchRequest(TechTypes::Tank_Siege_Mode);

	informationManager->_percentList[UnitTypes::Terran_Vulture] = 100;

	informationManager->_retreatDisabled = true;
	informationManager->_minSquadSize = 2;
	informationManager->_firstPush = true; // don't wait until first push
}


void TwoFactMines::Execute(StrategyManager* strategyManager)
{
	if (workerManager->anyWorkerDefending() && Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Marine)==0) {
		informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::Terran_Marine;
	} else {
		informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::None;
	}

	//if ( Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Factory) > 2) {
	if (Broodwar->self()->supplyUsed()/2 >= 30) {
		informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 100;
	}
	if (informationManager->minerals() > 450) {
		strategyManager->GetFSM()->ChangeState(TankTransition::Instance());
	}
}


void TwoFactMines::Exit(StrategyManager* strategyManager)
{
	// 	informationManager->researchRequest(TechTypes::Tank_Siege_Mode);
	// 	informationManager->naturalExpandRequest();
	// 	if (Broodwar->enemy()->getRace() == Races::Protoss) {
	// 		informationManager->_percentList[UnitTypes::Terran_Vulture] = 25;
	// 		informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 75;
	// 	} else {
	// 		informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 100;
	// 	}
	informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::None;
}


//------------------------------------------------------------------------
// Methods for Tank transition
//------------------------------------------------------------------------

TankTransition* TankTransition::Instance()
{
	static TankTransition instance;
	return &instance;
}

void TankTransition::Enter(StrategyManager* strategyManager)
{  
	informationManager->_minSquadSize = 6;
	informationManager->_autoBuildSuplies = true;
	//informationManager->researchRequest(TechTypes::Tank_Siege_Mode);
	informationManager->naturalExpandRequest();

	informationManager->upgradeRequest(UpgradeTypes::Terran_Vehicle_Weapons, 1);
	informationManager->_autoVehicleUpgrade = true;

// 	informationManager->_percentList[UnitTypes::Terran_Vulture] = 90;
// 	informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 10;

	informationManager->_percentList[UnitTypes::Terran_Vulture] = 25;
	informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 75;
}


void TankTransition::Execute(StrategyManager* strategyManager)
{
// 	if ( Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Vulture)+Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Siege_Tank_Tank_Mode) == 8 || 
// 		Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Factory) > 3) {
	if ( Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Factory) > 3 || !workerManager->needWorkers()) {
		strategyManager->GetFSM()->ChangeState(FullTank::Instance());      
	}
}


void TankTransition::Exit(StrategyManager* strategyManager)
{

}

//------------------------------------------------------------------------
// Methods for Full Tank
//------------------------------------------------------------------------

FullTank* FullTank::Instance()
{
	static FullTank instance;
	return &instance;
}

void FullTank::Enter(StrategyManager* strategyManager)
{  
	//informationManager->_autoBuildSuplies = true;
	//informationManager->_minSquadSize = 10;
	//informationManager->buildRequest(UnitTypes::Terran_Armory); // second armory

	informationManager->upgradeRequest(UpgradeTypes::Terran_Vehicle_Weapons, 3);
	//informationManager->upgradeRequest(UpgradeTypes::Terran_Vehicle_Plating, 3);
	informationManager->_autoVehicleUpgrade = true;

// 	informationManager->researchRequest(TechTypes::Spider_Mines);
// 	informationManager->upgradeRequest(UpgradeTypes::Ion_Thrusters);

// 	informationManager->_percentList[UnitTypes::Terran_Vulture] = 25;
	informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 100;
}


void FullTank::Execute(StrategyManager* strategyManager)
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

	// check air enemy force
	// *************** PROTOSS RULES ****************************************************
// 	if (informationManager->_enemyPhotonCanon == 0 && (informationManager->_enemyObserver == 0 || informationManager->_enemyAirDPS < 1) ) {
// 		informationManager->buildRequest(UnitTypes::Terran_Starport, true);
// 		informationManager->researchRequest(TechTypes::Cloaking_Field);
// 		informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::Terran_Wraith;
// 	} else {
// 		informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::None;
// 	}

	// Have 2 Science Vessel in total
// 	if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Engineering_Bay) > 0) {
// 		if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Science_Vessel) < 2) {
// 			if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Factory)==0 && !buildManager->alreadyBuilding(UnitTypes::Terran_Factory)) {
// 				informationManager->criticalBuildRequest(UnitTypes::Terran_Science_Facility, true);
// 				informationManager->criticalBuildRequest(UnitTypes::Terran_Starport, true);
// 				informationManager->criticalBuildRequest(UnitTypes::Terran_Factory, true);
// 			}
// 			informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::Terran_Science_Vessel;   
// 		} else if (informationManager->_trainOrder[UnitTypes::Terran_Starport] == UnitTypes::Terran_Science_Vessel) {
// 			informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::None;  
// 		}
// 	}
}


void FullTank::Exit(StrategyManager* strategyManager)
{

}

//------------------------------------------------------------------------
// Methods for Sparks
//------------------------------------------------------------------------

Sparks* Sparks::Instance()
{
	static Sparks instance;
	return &instance;
}

void Sparks::Enter(StrategyManager* strategyManager)
{  
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Barracks);
	informationManager->buildRequest(UnitTypes::Terran_Barracks);

	informationManager->_percentList[UnitTypes::Terran_Marine] = 100;

	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);

	informationManager->buildRequest(UnitTypes::Terran_Refinery);


	informationManager->buildRequest(UnitTypes::Terran_Academy);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);

	informationManager->_percentList[UnitTypes::Terran_Medic] = 30;
	informationManager->_percentList[UnitTypes::Terran_Marine] = 70;

	informationManager->buildRequest(UnitTypes::Terran_Barracks);

	informationManager->researchRequest(TechTypes::Stim_Packs);
	informationManager->buildRequest(UnitTypes::Terran_Comsat_Station, true);
	informationManager->upgradeRequest(UpgradeTypes::U_238_Shells);

	informationManager->_autoBuildSuplies = true;
	informationManager->_minSquadSize = 24;
}


void Sparks::Execute(StrategyManager* strategyManager)
{
	//if (informationManager->_marines > 24 && informationManager->_medics > 6 ) { informationManager->_attack_goahead = true; }
	//else { informationManager->_attack_goahead = false; }

	if (informationManager->_armySize > 60) {

		if (!buildManager->alreadyBuilding(UnitTypes::Terran_Command_Center) && !workerManager->needWorkers()) {
			if ( (informationManager->minerals() > 1000 || (informationManager->minerals() > 500 && informationManager->gas() < 100) ) &&
				informationManager->existGasExpand() ) {
					informationManager->gasExpandRequest(true);
			}
			if (informationManager->minerals()*2 < informationManager->gas() && informationManager->existNaturalExpand())  {
				informationManager->naturalExpandRequest(true);
			}
		}

	}


	if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Command_Center) >= 2 ){

		informationManager->_autoInfanteryUpgrade = true;

		if ((Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Barracks) < 6 )) {
			informationManager->buildRequest(UnitTypes::Terran_Barracks);
		}

		if ((Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Factory) < 2 )) {

			informationManager->buildRequest(UnitTypes::Terran_Factory);
			informationManager->buildRequest(UnitTypes::Terran_Machine_Shop);
			informationManager->researchRequest(TechTypes::Tank_Siege_Mode);

			informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 20;

		}
	}
}


void Sparks::Exit(StrategyManager* strategyManager)
{

}

//------------------------------------------------------------------------
// Methods for 2 Factory Tanks Phase 1
//------------------------------------------------------------------------

TwoFactTanks1* TwoFactTanks1::Instance()
{
	static TwoFactTanks1 instance;
	return &instance;
}

void TwoFactTanks1::Enter(StrategyManager* strategyManager)
{  
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Barracks);
	informationManager->buildRequest(UnitTypes::Terran_Refinery);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Factory);
// 	informationManager->buildRequest(UnitTypes::Terran_Bunker);

 	//informationManager->upgradeRequest(UpgradeTypes::U_238_Shells); //marine range
	informationManager->researchRequest(TechTypes::Tank_Siege_Mode);

	informationManager->buildRequest(UnitTypes::Terran_Factory);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->_autoBuildSuplies = true;

	informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 100;

	informationManager->_retreatDisabled = true;
	informationManager->_minSquadSize = 5;
}


void TwoFactTanks1::Execute(StrategyManager* strategyManager)
{
	// max marine population
	if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Marine)<4) {
		informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::Terran_Marine;
	} else {
		informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::None;
	}

	// 
	if ( Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Siege_Tank_Tank_Mode) >= 2) {
		strategyManager->GetFSM()->ChangeState(TwoFactTanks2::Instance()); 
	}
// 	if ( Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Command_Center) == 2) {
// 		strategyManager->GetFSM()->ChangeState(TwoFactTanks2::Instance()); 
// 	}
}


void TwoFactTanks1::Exit(StrategyManager* strategyManager)
{
	//informationManager->researchRequest(TechTypes::Tank_Siege_Mode);
	//informationManager->naturalExpandRequest();
}

//------------------------------------------------------------------------
// Methods for 2 Factory Tanks Phase 2
//------------------------------------------------------------------------

TwoFactTanks2* TwoFactTanks2::Instance()
{
	static TwoFactTanks2 instance;
	return &instance;
}

void TwoFactTanks2::Enter(StrategyManager* strategyManager)
{  
//	informationManager->gasExpandRequest();
	informationManager->naturalExpandRequest(true);
 
	informationManager->upgradeRequest(UpgradeTypes::Terran_Vehicle_Weapons, 1);
	informationManager->upgradeRequest(UpgradeTypes::Terran_Vehicle_Plating, 1);
	informationManager->_autoVehicleUpgrade = true;
// 
// 	informationManager->buildRequest(UnitTypes::Terran_Factory);
// 	informationManager->buildRequest(UnitTypes::Terran_Factory);
// 
// 	informationManager->gasExpandRequest();
// 	informationManager->buildRequest(UnitTypes::Terran_Armory); // second armory
//  	informationManager->_percentList[UnitTypes::Terran_Medic] = 35;
//  	informationManager->_percentList[UnitTypes::Terran_Marine] = 65;
//	informationManager->_percentList[UnitTypes::Terran_Firebat] = 65;
//	informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 100;

	informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::None;
	informationManager->_percentList[UnitTypes::Terran_Vulture] = 25;
	informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 75;

//	informationManager->researchRequest(TechTypes::Stim_Packs);
}


void TwoFactTanks2::Execute(StrategyManager* strategyManager)
{
	// max marine population
// 	if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Marine)<=7) {
// 		informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::Terran_Marine;
// 	} else {
// 		informationManager->_trainOrder[UnitTypes::Terran_Barracks] = UnitTypes::None;
// 	}

	if ( Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Factory) >= 3) {
		strategyManager->GetFSM()->ChangeState(FullTank::Instance());      
	}

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

	// check air enemy force
// 	if (informationManager->_enemyPhotonCanon == 0 && (informationManager->_enemyObserver == 0 || informationManager->_enemyAirDPS < 1) ) {
// 		informationManager->buildRequest(UnitTypes::Terran_Starport, true);
// 		informationManager->researchRequest(TechTypes::Cloaking_Field);
// 		informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::Terran_Wraith;
// 	} else {
// 		informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::None;
// 	}
}


void TwoFactTanks2::Exit(StrategyManager* strategyManager)
{

}