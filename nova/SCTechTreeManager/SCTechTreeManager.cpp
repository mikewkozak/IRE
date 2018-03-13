// SCTechTreeManager.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "TerranTreeManager.h"


int main() {
	printf("main() - Launching SC Tech Tree Manager\n");

	printf("main() - Creating Terran Tech Tree Manager\n");
	TerranTreeManager mgr;

	GraphUtils::printTree(mgr.getTree(), "Strategies/TechTrees/TerranTechTree.dot", false);

	printf("main() - Strengthening Tree for Observed Enemy Units\n");
	//mgr.strengthenTree(BWAPI::UnitTypes::Terran_Vulture);
	mgr.strengthenTree(BWAPI::UnitTypes::Terran_Wraith);
	mgr.strengthenTree(BWAPI::UnitTypes::Terran_Wraith);
	mgr.strengthenTree(BWAPI::UnitTypes::Terran_Wraith);
	mgr.strengthenTree(BWAPI::UnitTypes::Terran_Wraith);
	mgr.strengthenTree(BWAPI::UnitTypes::Terran_Wraith);

	//Print the results of all observations
	GraphUtils::printTree(mgr.getTree(), "Strategies/Observed/TerranTechTree-Strengthened.dot", false);

	//Identify the likely enemy strategy based on observations
	mgr.identifyStrategy();

    return 0;
}

