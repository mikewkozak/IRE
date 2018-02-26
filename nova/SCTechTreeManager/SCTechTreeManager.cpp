// SCTechTreeManager.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "TerranTreeManager.h"


int main()
{
	printf("main() - Launching SC Tech Tree Manager\n");

	printf("main() - Creating Terran Tech Tree Manager\n");
	TerranTreeManager mgr;

	printf("main() - Building Terran Tech Tree\n");
	mgr.buildTree();
	mgr.printTree("TerranTechTree.dot");

	printf("main() - Strengthening Tree for Terran Vulture\n");
	mgr.strengthenTree(BWAPI::UnitTypes::Terran_Vulture);
	mgr.printTree("TerranTechTree-Strengthened.dot");

	mgr.identifyStrategy();

    return 0;
}

