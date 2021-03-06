// SCTechTreeManager.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "TerranTreeManager.h"
#include "ZergTreeManager.h"
#include "ProtossTreeManager.h"


int main() {
	printf("main() - Launching SC Tech Tree Manager\n");

	printf("main() - Creating Tech Tree Managers\n");
	TerranTreeManager terranMgr;
	ZergTreeManager zergMgr;
	ProtossTreeManager protossMgr;

	GraphUtils::printTree(terranMgr.getTree(), "Strategies/TechTrees/TerranTechTree.dot", false);
	GraphUtils::printTree(zergMgr.getTree(), "Strategies/TechTrees/ZergTechTree.dot", false);
	GraphUtils::printTree(protossMgr.getTree(), "Strategies/TechTrees/ProtossTechTree.dot", false);

	printf("main() - Strengthening Tree for Observed Enemy Units\n");
	/*
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Vulture);
	GraphUtils::printTree(terranMgr.getTree(), "Strategies/Observed/TerranTechTree-Strengthened.dot", false);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Dragoon);
	GraphUtils::printTree(protossMgr.getTree(), "Strategies/Observed/ProtossTechTree-Strengthened.dot", false);
	zergMgr.strengthenTree(BWAPI::UnitTypes::Zerg_Hydralisk);
	GraphUtils::printTree(zergMgr.getTree(), "Strategies/Observed/ZergTechTree-Strengthened.dot", false);
	*/

	//AAIDE 2017 Iron vs ZZZKBot minute 3
	/*
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);

	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Factory);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Barracks);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Vulture);

	//Print the results of all observations
	GraphUtils::printTree(terranMgr.getTree(), "Strategies/Observed/TerranTechTree-Strengthened.dot", false);

	//Identify the likely enemy strategy based on observations
	terranMgr.identifyStrategy();
	*/

	//AAIDE 2011 Man vs Machine 4:22 mark

	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);

	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Factory);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Machine_Shop);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Barracks);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);

	//Print the results of all observations
	GraphUtils::printTree(terranMgr.getTree(), "Strategies/Observed/TerranTechTree-Strengthened.dot", false);

	//Identify the likely enemy strategy based on observations
	terranMgr.identifyStrategy();


	//Vs IronBot 2017 edition*********************************************************************************************
	/*
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Command_Center);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Refinery);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Supply_Depot);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Barracks);

	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV); terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV); terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV); terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV); terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV); terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV); terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_SCV);

	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Vulture);

	//SECOND SCAN
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Supply_Depot);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Supply_Depot);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Supply_Depot);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Factory);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Factory);

	//THIRD SCAN
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Starport);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Starport);


	GraphUtils::printTree(terranMgr.getTree(), "Strategies/Observed/Iron_Observations.dot", false);
	//Identify the likely enemy strategy based on observations
	terranMgr.identifyStrategy();
	*/
	//Vs KillerBot 2017 edition*********************************************************************************************
	/*
	zergMgr.strengthenTree(BWAPI::UnitTypes::Zerg_Hatchery);
	zergMgr.strengthenTree(BWAPI::UnitTypes::Zerg_Spawning_Pool);
	zergMgr.strengthenTree(BWAPI::UnitTypes::Zerg_Creep_Colony);
	zergMgr.strengthenTree(BWAPI::UnitTypes::Zerg_Extractor);

	//SECOND SCAN
	zergMgr.strengthenTree(BWAPI::UnitTypes::Zerg_Hydralisk);

	GraphUtils::printTree(zergMgr.getTree(), "Strategies/Observed/Killer_Observations.dot", false);
	//Identify the likely enemy strategy based on observations
	zergMgr.identifyStrategy();
	*/

	//Vs NiteKat 2017 edition*********************************************************************************************
	/*
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine); terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine); terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine); terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);

	//SECOND SCAN
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Command_Center);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Refinery);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Supply_Depot);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Supply_Depot);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Supply_Depot);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Barracks);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Barracks);

	//THIRD SCAN
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Academy);


	GraphUtils::printTree(terranMgr.getTree(), "Strategies/Observed/Iron_Observations.dot", false);
	//Identify the likely enemy strategy based on observations
	terranMgr.identifyStrategy();
	*/


	//Vs Pibb Protoss 2017 edition*********************************************************************************************
/*
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Nexus);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Gateway);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Forge);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Pylon);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Pylon);

	//SECOND SCAN
	//protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Gateway);
	//protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Pylon);

	//THIRD SCAN
	//protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Robotics_Facility);
	//protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Citadel_of_Adun);
	//protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Photon_Cannon); protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	//protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Photon_Cannon); protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	//protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	//protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Zealot);

	GraphUtils::printTree(protossMgr.getTree(), "Strategies/Observed/Pibb_Protoss_Observations.dot", false);
	//Identify the likely enemy strategy based on observations
	protossMgr.identifyStrategy();
	*/

	//Vs Pibb Terran 2017 edition*********************************************************************************************
/*
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine); 

	//SECOND SCAN
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine); terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine); terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Marine);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Command_Center);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Refinery);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Supply_Depot);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Barracks);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Factory);

	//THIRD SCAN
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Machine_Shop);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Barracks);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Bunker);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode);

	//FOURTH SCAN
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Command_Center);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Starport);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Engineering_Bay);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Bunker);
	//terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Bunker);



	GraphUtils::printTree(terranMgr.getTree(), "Strategies/Observed/Pibb_Terran_Observations.dot", false);
	//Identify the likely enemy strategy based on observations
	terranMgr.identifyStrategy();
	*/

	//Vs Skynet Protoss 2017 edition*********************************************************************************************
	/*
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Nexus);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Gateway);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Gateway);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Assimilator);

	GraphUtils::printTree(protossMgr.getTree(), "Strategies/Observed/Skynet_Observations.dot", false);
	//Identify the likely enemy strategy based on observations
	protossMgr.identifyStrategy();
	*/

	//Vs Wuli Protoss 2017 edition*********************************************************************************************
	/*
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Nexus);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Gateway);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Gateway);
	protossMgr.strengthenTree(BWAPI::UnitTypes::Protoss_Gateway);

	GraphUtils::printTree(protossMgr.getTree(), "Strategies/Observed/Wuli_Observations.dot", false);
	//Identify the likely enemy strategy based on observations
	protossMgr.identifyStrategy();
	*/


    return 0;
}

