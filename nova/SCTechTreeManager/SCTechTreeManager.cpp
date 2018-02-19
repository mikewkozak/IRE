// SCTechTreeManager.cpp : Defines the entry point for the console application.
//Necessary for boost in VS
#define _WEBSOCKETPP_CPP11_THREAD_
#define _WEBSOCKETPP_CPP11_FUNCTIONAL_
#define _WEBSOCKETPP_CPP11_SYSTEM_ERROR_
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define _WEBSOCKETPP_CPP11_MEMORY_

#include "stdafx.h"
#include "TerranTreeManager.h"


int main()
{
	printf("main() - Launching SC Tech Tree Manager\n");

	printf("main() - Creating Terran Tech Tree Manager\n");
	TerranTreeManager mgr;

	printf("main() - Building Terran Tech Tree\n");
	mgr.buildTree();
	mgr.printTree();

	printf("main() - Strengthening Tree for Terran Vulture\n");
	mgr.strengthenTree(BWAPI::UnitTypes::Terran_Vulture);
	mgr.printTree();

    return 0;
}

