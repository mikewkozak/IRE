#include "VsZerg.h"

using namespace BWAPI;

//------------------------------------------------------------------------
// Methods for 2 Port Wraith
//------------------------------------------------------------------------

TwoPortWraith* TwoPortWraith::Instance()
{
	static TwoPortWraith instance;
	return &instance;
}

void TwoPortWraith::Enter(StrategyManager* strategyManager)
{  
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Barracks);
	informationManager->buildRequest(UnitTypes::Terran_Refinery);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Factory);
	informationManager->buildRequest(UnitTypes::Terran_Starport);
	informationManager->buildRequest(UnitTypes::Terran_Starport);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);

	informationManager->_percentList[UnitTypes::Terran_Vulture] = 100;
	informationManager->_percentList[UnitTypes::Terran_Wraith] = 100;

	informationManager->upgradeRequest(UpgradeTypes::Ion_Thrusters);
	informationManager->upgradeRequest(UpgradeTypes::Terran_Ship_Plating, 3);
	informationManager->_autoShipUpgrade = true;
	informationManager->researchRequest(TechTypes::Cloaking_Field);
}


void TwoPortWraith::Execute(StrategyManager* strategyManager)
{
	// auto expand
	if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Supply_Depot) >= 4) {
		if (!buildManager->alreadyBuilding(UnitTypes::Terran_Command_Center) && 
			( informationManager->minerals() > 1000 || (informationManager->minerals() > 500 && informationManager->gas() < 100) ) &&
			informationManager->existGasExpand() ) {
				informationManager->gasExpandRequest(true);
		}
		if (!buildManager->alreadyBuilding(UnitTypes::Terran_Command_Center) && informationManager->minerals()*2 < informationManager->gas() &&
			informationManager->existNaturalExpand())  {
				informationManager->naturalExpandRequest(true);
		}
	}
}


void TwoPortWraith::Exit(StrategyManager* strategyManager)
{

}


//------------------------------------------------------------------------
// Methods for 1 Barracks Fast Expansion
//------------------------------------------------------------------------

OneRaxFE* OneRaxFE::Instance()
{
	static OneRaxFE instance;
	return &instance;
}

void OneRaxFE::Enter(StrategyManager* strategyManager)
{  
	informationManager->_firstPush = true; // don't wait until first push

	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
	informationManager->buildRequest(UnitTypes::Terran_Barracks);
	informationManager->buildRequest(UnitTypes::Terran_Barracks);
	informationManager->buildRequest(UnitTypes::Terran_Refinery);
	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);

	//informationManager->buildRequest(UnitTypes::Terran_Bunker);

	informationManager->buildRequest(UnitTypes::Terran_Academy);

	informationManager->_percentList[UnitTypes::Terran_Medic] = 20;
	informationManager->_percentList[UnitTypes::Terran_Marine] = 80;
	informationManager->researchRequest(TechTypes::Stim_Packs);
	//informationManager->researchRequest(TechTypes::Optical_Flare);
	informationManager->upgradeRequest(UpgradeTypes::U_238_Shells);
	informationManager->upgradeRequest(UpgradeTypes::Terran_Infantry_Weapons);
	//informationManager->buildRequest(UnitTypes::Terran_Comsat_Station, true);

	informationManager->_priorCommandCenters = false;
	informationManager->_autoBuildSuplies = true;
	informationManager->_minSquadSize = 10;
	informationManager->_autoInfanteryUpgrade = true;
}


void OneRaxFE::Execute(StrategyManager* strategyManager)
{
	// auto expand
	if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Academy) >= 1) {
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

	// check upgrades
	if (Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Infantry_Armor) == 1) {
		informationManager->buildRequest(UnitTypes::Terran_Science_Facility, true);
	}

	// Have 2 Science Vessel in total
	if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Engineering_Bay) > 0) {
		if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Science_Vessel) < 2) {
			if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Factory)==0 && !buildManager->alreadyBuilding(UnitTypes::Terran_Factory)) {
				informationManager->criticalBuildRequest(UnitTypes::Terran_Science_Facility, true);
				informationManager->criticalBuildRequest(UnitTypes::Terran_Starport, true);
				informationManager->criticalBuildRequest(UnitTypes::Terran_Factory, true);
			}
			informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::Terran_Science_Vessel;   
		} else if (informationManager->_trainOrder[UnitTypes::Terran_Starport] == UnitTypes::Terran_Science_Vessel) {
			informationManager->_trainOrder[UnitTypes::Terran_Starport] = UnitTypes::None;  
		}
	}
}


void OneRaxFE::Exit(StrategyManager* strategyManager)
{

}