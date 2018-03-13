// SCTechTreeManager.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "TerranTreeManager.h"
#include "ZergTreeManager.h"
#include "ProtossTreeManager.h"


int main() {
	printf("main() - Launching SC Tech Tree Manager\n");

	printf("main() - Creating Terran Tech Tree Manager\n");
	TerranTreeManager terranMgr;
	ZergTreeManager zergMgr;
	ProtossTreeManager protossMgr;

	GraphUtils::printTree(terranMgr.getTree(), "Strategies/TechTrees/TerranTechTree.dot", false);
	GraphUtils::printTree(zergMgr.getTree(), "Strategies/TechTrees/ZergTechTree.dot", false);
	GraphUtils::printTree(protossMgr.getTree(), "Strategies/TechTrees/ProtossTechTree.dot", false);

	printf("main() - Strengthening Tree for Observed Enemy Units\n");
	//mgr.strengthenTree(BWAPI::UnitTypes::Terran_Vulture);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Wraith);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Wraith);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Wraith);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Wraith);
	terranMgr.strengthenTree(BWAPI::UnitTypes::Terran_Wraith);


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

    return 0;
}

