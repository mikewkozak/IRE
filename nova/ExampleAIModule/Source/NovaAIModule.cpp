#include "NovaAIModule.h"

// #include <vld.h> // uncomment this to detect memory leaks

#ifdef NOVA_GUI
int argc = 1;
char* argv = "0";
QApplication QtApp(argc, &argv);
myQtApp* novaGUI;
#endif

using namespace BWAPI;

int totalKitingFrames;

NovaAIModule::NovaAIModule()
	: leavingGame(0)
	, saidGG(false)
    , timeout(0)
{

}

void NovaAIModule::onStart()
{
    loadIniConfig();
	setUpLogging();

	enhancedUI = new EnhancedUI();
	informationManager = new InformationManager();
    informationManager->gameState.loadIniConfig();

	Broodwar->enableFlag(Flag::UserInput);

#ifdef NOVA_GUI
	Broodwar->enableFlag(Flag::CompleteMapInformation);
	Broodwar->pauseGame();
#endif

//     if (HIGH_LEVEL_SEARCH) {
//         Broodwar->enableFlag(Flag::CompleteMapInformation);
//     }

    std::string enableGUI = LoadConfigString("general", "gui", "ON");
    if (enableGUI == "OFF") {
        Broodwar->setGUI(false); // to speed up (skips draw frames)
    }

    timeout = LoadConfigInt("general", "timeout");

	PATHFINDING01 = false;
	usingCloackUnits = false;
	totalKitingFrames = 0;
	vulturesCreated = 0;
	vulturesKilled = 0;
	totalVulutreLife = 0;
	zealotsKilled = 0;

	// if we don't have a command center is a melee map
	ONLY_MICRO = true;
	if (!Broodwar->isReplay()) {
		for (const auto& unit : Broodwar->self()->getUnits()) {
			if (unit->getType().isResourceDepot()) {
				ONLY_MICRO = false;
				break;
			}
		}
	} else ONLY_MICRO = false;

	if ( Broodwar->mapFileName().compare("path01.scx") == 0) {
		PATHFINDING01 = true;
	}
	if ( Broodwar->mapFileName().compare("kiting04.scm") == 0) {
		ONLY_MICRO = true;
		Broodwar->enableFlag(Flag::CompleteMapInformation);
		Broodwar->setLocalSpeed(0);
		// globals
		selfHitPoints = 0;
		selfMaxHitPoints = 0;
		enemyHitPoints = 0;
		enemyMaxHitPoints = 0;
		//microBullets = BWAPI::Bullet(null);
		numShots = 0;
	}

	// Initialize managers
	squadManager = new SquadManager();
	workerManager = new WorkerManager();
	plannerManager = new PlannerManager();
	productionManager = new ProductionManager();
	buildManager = new BuildManager();
	wallGenerator = new WallGenerator();
	if (!Broodwar->isReplay()) {
		strategyManager = new StrategyManager(productionManager);
	}
	
	// Compute max DPS and HP of each unit
	unitStatic = new UnitInfoStatic();
	// set combat simulator
// 	CombatSimulator* combatSim = new CombatSimLanchester(&unitStatic->DPF);
// 	CombatSimulator* combatSim = new CombatSimSustained(&unitStatic->DPF);
	CombatSimulator* combatSim = new CombatSimDecreased(&unitStatic->typeDPF, &unitStatic->DPF);
	informationManager->gameState.changeCombatSimulator(combatSim);

#ifdef TERRAIN_ANALYSIS
		Broodwar->setLocalSpeed(20);

		//Analyze Map
		BWTA::readMap();
		BWTA::analyze();
		informationManager->mapAnalyzed = true;

		BWTA::computeDistanceTransform();
		BWTA::buildChokeNodes();

// 		TilePosition start(12, 101);
// 		TilePosition target(109, 5);
// 		path = BWTA::getShortestPath2(start, target);

//		BWTA::balanceAnalysis();

		// Testing ground distance performance
// 		clock_t clockStart;
// 		clock_t clockEnd;
// 		double seconds;
// 		double distance1;
// 		int distance2;
// 
// 		// Old way
// 		clockStart = clock();
// 		for(int x = 0 ; x <= 10000 ; ++x) {
// 			distance1 = BWTA::getGroundDistance(start, target);
// 		}
// 		clockEnd = clock();
// 		seconds = double(clockEnd-clockStart)/CLOCKS_PER_SEC;
// 		LOG("[BWTA] Ground Distance seconds: " << seconds);
// 
// 		// New way
// 		clockStart = clock();
// 		for(int x = 0 ; x <= 10000 ; ++x) {
// 			distance2 = BWTA::getGroundDistance2(start, target);
// 		}
// 		clockEnd = clock();
// 		seconds = double(clockEnd-clockStart)/CLOCKS_PER_SEC;
// 		LOG("[BWTA] Ground Distance 2 seconds: " << seconds);
// 
// 		Position start2(start);
// 		Position target2(target);
// 
// 		clockStart = clock();
// 		for(int x = 0 ; x <= 10000 ; ++x) {
// 			distance1 = start2.getDistance(target2);
// 		}
// 		clockEnd = clock();
// 		seconds = double(clockEnd-clockStart)/CLOCKS_PER_SEC;
// 		LOG("[BWAPI] Distance seconds: " << seconds);
// 
// 		clockStart = clock();
// 		for(int x = 0 ; x <= 10000 ; ++x) {
// 			distance2 = start2.getApproxDistance(target2);
// 		}
// 		clockEnd = clock();
// 		seconds = double(clockEnd-clockStart)/CLOCKS_PER_SEC;
// 		LOG("[BWAPI] Approx Distance seconds: " << seconds);

#endif
	
	if (PATHFINDING01) {
		//Select SVC
		BWAPI::Unitset::iterator i = Broodwar->self()->getUnits().begin();
		Unit scv = *i;

		Broodwar->setLocalSpeed(20);
		Position target(19*TILE_SIZE, 87*TILE_SIZE);

		//Analyze Map
// 		BWTA::readMap();
		BWTA::analyze();
		informationManager->mapAnalyzed = true;

		//Init walkable map
// 		int mapW = Broodwar->mapWidth()*4;
// 		int mapH = Broodwar->mapHeight()*4;
// 
// 		walkableMap = new int*[mapW];
// 		for(int x = 0 ; x < mapW ; ++x) {
// 			walkableMap[x] = new int[mapH];
// 
// 			//Fill from static map
// 			for (int y = 0; y < mapH; ++y) {
// 				walkableMap[x][y] = (Broodwar->isWalkable(x, y))? WALKABLE : BLOCKED;
// 			}
// 		}
// 
// 		//Block static neutral units
// 		for(BWAPI::Unitset::iterator m = Broodwar->getStaticNeutralUnits().begin(); m != Broodwar->getStaticNeutralUnits().end(); ++m) {
// 			// get area
// 			MyRectangle area;
// 			area.x1 = (*m)->getTilePosition().x*4;
// 			area.y1 = (*m)->getTilePosition().y*4;
// 			area.x2 = (*m)->getTilePosition().x*4 + (*m)->getType().tileWidth()*4;
// 			area.y2 = (*m)->getTilePosition().y*4 + (*m)->getType().tileHeight()*4;
// 			// map area
// 			for (int x = area.x1; x <= area.x2; x++) {
// 				for (int y = area.y1; y <= area.y2; y++) {
// 					if (x >= 0 && x < mapW && y >= 0 && y < mapH) {
// 						walkableMap[x][y] = BLOCKED;
// 					}
// 				}
// 			}
// 		}
// 
// 		//Build ChokeNodeGraph
// 		buildChokeNodes();
// 
// 		//Get walkable path
// 		path = getShortestPath(WalkPosition(scv->getPosition()), WalkPosition(target));

		
		scv->move(target);


	}else if (ONLY_MICRO) {
		// Set enemy start position
		TilePosition::list enemyPosition = Broodwar->getStartLocations();
		for (auto startLocation : Broodwar->getStartLocations()) {
			if (Broodwar->self()->getStartLocation() != startLocation) {
				informationManager->_enemyStartPosition = Position(startLocation.x*TILE_SIZE, startLocation.y*TILE_SIZE);
			}
		}
		// Add units to squad
		squadManager->createNewSquad(Broodwar->self()->getUnits());
	} else {
        int gameSpeed = LoadConfigInt("general", "gameSpeed");
		Broodwar->setLocalSpeed(gameSpeed);

        std::string newThread = LoadConfigString("general", "analyze_map_thread", "ON");
#ifdef NOVA_GUI
        newThread = "OFF";
#endif
        if (newThread == "OFF" ) {
// 		    BWTA::readMap();
		    AnalyzeMapThread();
        } else {
		    informationManager->analyzeMap();
        }
	}
	
#ifdef NOVA_GUI
	novaGUI = new myQtApp;
	novaGUI->show();
#endif
}

NovaAIModule::~NovaAIModule()
{
	delete enhancedUI;
	delete informationManager;
	delete squadManager;
	delete workerManager;
	delete plannerManager;
	delete productionManager;
	delete buildManager;
	if (!Broodwar->isReplay()) delete strategyManager;
	delete unitStatic;
	delete wallGenerator;
	BWTA::cleanMemory();

#ifdef NOVA_GUI
	QtApp.quit();
	delete novaGUI;
#endif
}

void NovaAIModule::onFrame()
{
#ifdef NOVA_GUI
	QtApp.processEvents();
	QtApp.sendPostedEvents();
#endif

	LOG4CXX_TRACE(_logger, "Start Frame");
	
#ifndef TOURNAMENT
	enhancedUI->onFrame();
#endif

	if (Broodwar->isReplay()) { // print branching stats
		AbstractLayer abstractGameState; // import current game state to informationManager->gameState
		abstractGameState.printBranchingStats();
	}

	// Return if the game is a replay or is paused
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self())
		return;

	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
// 	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
// 		return;

    // game timeout when high level search (20 minutes)
    if (timeout > 0 && Broodwar->getFrameCount() >= timeout) {
		LOG4CXX_TRACE(_logger, "Game timeout (" << timeout << ") leaving game");
        Broodwar->leaveGame();
        return;
    }

#ifdef TERRAIN_ANALYSIS
		// Draw path
// 		WalkPosition previousPosition(-1, -1);
// 		for each(const BWTA::Chokepoint* choke in path) {
// 			WalkPosition pos(choke->getCenter());
// 			if(previousPosition.x != -1 && previousPosition.y != -1)
// 				BWAPI::Broodwar->drawLineMap(pos.x * 8 + 4, pos.y * 8 + 4, previousPosition.x * 8 + 4, previousPosition.y * 8 + 4, BWAPI::Colors::Cyan);
// 
// 			previousPosition = pos;
// 		}
#endif

	if (PATHFINDING01){
		// Draw walkable tiles
		//int mapW = Broodwar->mapWidth()*4;
		//int mapH = Broodwar->mapHeight()*4;
		//for(int x=0; x < mapW; ++x) {
		//	for(int y=0; y < mapH; ++y) {
		//		if ( walkableMap[x][y] == WALKABLE) {
		//		//if ( Broodwar->isWalkable(x,y) ) {
		//			Broodwar->drawCircleMap(x*8+4,y*8+4,1,Colors::Green,true);
		//		} else {
		//			Broodwar->drawCircleMap(x*8+4,y*8+4,1,Colors::Red,true);
		//		}
		//	}
		//}

		// Draw path
		BWAPI::Unitset::iterator i = Broodwar->self()->getUnits().begin();
		Unit scv = *i;
		Position target(19*TILE_SIZE, 87*TILE_SIZE);
		BWAPI::WalkPosition previousPosition(-1, -1);
		for each(const BWTA::Chokepoint* choke in path) {
			BWAPI::WalkPosition pos(choke->getCenter());
			if(previousPosition.x != -1 && previousPosition.y != -1)
				BWAPI::Broodwar->drawLineMap(pos.x * 8 + 4, pos.y * 8 + 4, previousPosition.x * 8 + 4, previousPosition.y * 8 + 4, BWAPI::Colors::Cyan);
			else
				BWAPI::Broodwar->drawLineMap(pos.x * 8 + 4, pos.y * 8 + 4, scv->getPosition().x, scv->getPosition().y, BWAPI::Colors::Cyan);

			previousPosition = pos;
		}
		BWAPI::Broodwar->drawLineMap(target.x, target.y, previousPosition.x * 8 + 4, previousPosition.y * 8 + 4, BWAPI::Colors::Cyan);
	}

	if (ONLY_MICRO) {
		//DEBUG("squadManager");
		squadManager->onFrame();

// 		if (Broodwar->getFrameCount() == 1) {
// 			for(BWAPI::Unitset::const_iterator it = Broodwar->self()->getUnits().begin();it!=Broodwar->self()->getUnits().end();it++) {
// 				selfMaxHitPoints += (*it)->getType().maxHitPoints() + (*it)->getType().maxShields();
// 			}
// 
// 			for(BWAPI::Unitset::const_iterator it = Broodwar->enemy()->getUnits().begin();it!=Broodwar->enemy()->getUnits().end();it++) {
// 				enemyMaxHitPoints += (*it)->getType().maxHitPoints() + (*it)->getType().maxShields();
// 			}
// 		}
	} else {
		kitingFrame = false;
		informationManager->_frameMineralSpend = 0;
		informationManager->_frameGasSpend = 0;
		LOG4CXX_TRACE(_logger, "Starting Squad Manager");
		squadManager->onFrame();
		LOG4CXX_TRACE(_logger, "Starting Worker Manager");
		workerManager->onFrame();
		LOG4CXX_TRACE(_logger, "Starting Build Manager");
		buildManager->onFrame();
		LOG4CXX_TRACE(_logger, "Starting Planner Manager");
		plannerManager->onFrame();
		LOG4CXX_TRACE(_logger, "Starting Production Manager");
		productionManager->onFrame();
		LOG4CXX_TRACE(_logger, "Starting Strategy Manager");
		strategyManager->onFrame();
		informationManager->updateLastSeenUnitPosition();
		if (kitingFrame) totalKitingFrames++;

        if (informationManager->_bbs && Broodwar->getFrameCount() == 10) {
			LOG4CXX_TRACE(_logger, "Send worker Rush location in order to build proxy barracks");
			// if we know the enemy location (2 players map) send to the enemy region
			if (informationManager->_scoutedAnEnemyBase) {
				workerManager->scoutPosition(informationManager->_enemyStartPosition);
// 				Unit scv = workerManager->getWorkerForTask(informationManager->_enemyStartPosition);
// 				if (scv) scv->move(informationManager->_enemyStartPosition);
			} else {
				// else, we need to scout all possible enemy bases
// 				Unitset::iterator i = workerManager->_workerUnits.begin();
				for (const auto& enemyBase : informationManager->startLocationCouldContainEnemy) {
					workerManager->scoutPosition(enemyBase->getPosition());
// 					Unit scv = *i;
// 					if (scv) scv->move(enemyBase->getPosition());
// 					++i;
				}
			}
        }

		//Be social
		LOG4CXX_TRACE(_logger, "Be social");
		if (Broodwar->getFrameCount() == 260)
			Broodwar->sendText("gl hf");

		if (leavingGame == 0 && Broodwar->self()->visibleUnitCount(UnitTypes::Terran_SCV) < 3) {
			leavingGame = BWAPI::Broodwar->getFrameCount();
		}
		if(leavingGame != 0) {
			if(BWAPI::Broodwar->getFrameCount() - leavingGame > 24 && !saidGG) {
				saidGG = true;
				BWAPI::Broodwar->sendText("gg");
			} else if (BWAPI::Broodwar->getFrameCount() - leavingGame > 80) {
				BWAPI::Broodwar->leaveGame();
				return;
			}
		}
	}

	if (wallGenerator->WallCalculated) {
		//Draw wall locations
		Broodwar->drawBoxMap(wallGenerator->BarracksWall.x * 32, wallGenerator->BarracksWall.y * 32, wallGenerator->BarracksWall.x * 32 + 4 * 32, wallGenerator->BarracksWall.y * 32 + 3 * 32, Colors::Orange);
		Broodwar->drawBoxMap(wallGenerator->SupplyWall1.x * 32, wallGenerator->SupplyWall1.y * 32, wallGenerator->SupplyWall1.x * 32 + 3 * 32, wallGenerator->SupplyWall1.y * 32 + 2 * 32, Colors::Orange);
		Broodwar->drawBoxMap(wallGenerator->SupplyWall2.x * 32, wallGenerator->SupplyWall2.y * 32, wallGenerator->SupplyWall2.x * 32 + 3 * 32, wallGenerator->SupplyWall2.y * 32 + 2 * 32, Colors::Orange);
		Broodwar->drawCircleMap(BWAPI::Position(wallGenerator->BarracksWall.x * 32, wallGenerator->BarracksWall.y * 32), 256, Colors::Purple);
		
// 		Broodwar->drawBoxMap(wallGenerator->TopLeft.x * 32, wallGenerator->TopLeft.y * 32, wallGenerator->BottomRight.x * 32, wallGenerator->BottomRight.y * 32, Colors::Purple);
// 		Broodwar->drawBoxMap(wallGenerator->Furthest.x * 32, wallGenerator->Furthest.y * 32, wallGenerator->Furthest.x * 32 + 32, wallGenerator->Furthest.y * 32 + 32, Colors::White);
// 		Broodwar->drawBoxMap(wallGenerator->Closest.x * 32, wallGenerator->Closest.y * 32, wallGenerator->Closest.x * 32 + 32, wallGenerator->Closest.y * 32 + 32, Colors::White);
	}

	LOG4CXX_TRACE(_logger, "End Frame");
}

void NovaAIModule::onSendText(std::string text)
{
	if (text == "wall") wallGenerator->WallOff();
	if (text == "pw") {
		LOG(Broodwar->mapHash() << "," << Broodwar->self()->getStartLocation().x << ", " << Broodwar->self()->getStartLocation().y);
		LOG(wallGenerator->SupplyWall1.x << ", " << wallGenerator->SupplyWall1.y);
		LOG(wallGenerator->SupplyWall2.x << ", " << wallGenerator->SupplyWall2.y);
		LOG(wallGenerator->BarracksWall.x << ", " << wallGenerator->BarracksWall.y);
	}
	if (text == "load") { wallGenerator->LoadWallData(); }
	if (text == "p") Broodwar->setLocalSpeed(200);
	if (text=="/bo") PRINT_BUILD_ORDER = !PRINT_BUILD_ORDER;
	if (text=="/bm") PRINT_BUILD_MAP = !PRINT_BUILD_MAP;
	if (text=="/dps1") {
		PRINT_GROUND_DPS = !PRINT_GROUND_DPS;
		Broodwar->printf("Ground DPS map");
	}
	if (text=="/dps2") {
		PRINT_AIR_DPS = !PRINT_AIR_DPS;
		Broodwar->printf("Air DPS map");
	}
	if (text=="/production") PRINT_PRODUCTION = !PRINT_PRODUCTION;
    if (text=="/region") PRINT_REGION_ID_MAP = !PRINT_REGION_ID_MAP;
	if (text=="/search") {
		informationManager->_searchAndDestroy = true;
		Broodwar->printf("Search and Destroy activated!");
		// check if we need the buildings to build Wraiths
		if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Starport) == 0 && !buildManager->alreadyBuilding(UnitTypes::Terran_Starport)) {
			informationManager->buildRequest(UnitTypes::Terran_Science_Facility, true);
			informationManager->buildRequest(UnitTypes::Terran_Starport, true);
		}
	}
	if (text=="/panic") {
		informationManager->_panicMode = true;
		Broodwar->printf("Panic mode activated!");
	}

    std::vector<std::string> parsed = splitString(text, ' ');

	if (parsed[0] == "/size") {
		if (parsed.size() > 1)
			informationManager->_minSquadSize = atoi(parsed[1].c_str());
		Broodwar->printf("Minim squad size: %i", informationManager->_minSquadSize);
    }
}

void NovaAIModule::onReceiveText(BWAPI::Player* player, std::string text)
{
  //Broodwar->printf("%s said '%s'", player->getName().c_str(), text.c_str());
}

void NovaAIModule::onNukeDetect(BWAPI::Position target)
{
//   if (target!=Positions::Unknown)
//     Broodwar->printf("Nuclear Launch Detected at (%d,%d)",target.x,target.y);
//   else
//     Broodwar->printf("Nuclear Launch Detected");
}

void NovaAIModule::onUnitDiscover(BWAPI::Unit unit)
{
// 	DEBUG("UNIT DISCOVER (" << unit << ") " << unit->getType().getName() << " [" << unit->getPlayer()->getName() << "]"); 
}

void NovaAIModule::onUnitEvade(BWAPI::Unit unit)
{
// 	DEBUG("UNIT EVADE (" << unit << ") " << unit->getType().getName() << " [" << unit->getPlayer()->getName() << "]"); 
}

void NovaAIModule::onUnitShow(BWAPI::Unit unit)
{
	if (Broodwar->isReplay()) return;
	LOG4CXX_TRACE(_logger, "UNIT SHOW (" << unit << ") " << unit->getType().getName() << " [" << unit->getPlayer()->getName() << "]");
	
	if (Broodwar->isReplay()) return;
    if (unit->getType().isSpell()) return; // ignore spells
	
	if (!ONLY_MICRO) {
		if (Broodwar->self() == unit->getPlayer()) {
			if (unit->getType() == UnitTypes::Terran_Vulture_Spider_Mine) return; // skip spider mines
			if (unit->getType().isBuilding()) {
				productionManager->onBuildingShow(unit);
				if (!unit->getType().isAddon()) buildManager->constructionPlaced(unit);
				if (unit->getType() == UnitTypes::Terran_Command_Center) informationManager->onCommandCenterShow(unit);
				if (unit->getType() == UnitTypes::Terran_Missile_Turret) informationManager->onMissileTurretShow(unit);
			} else if (unit->getType().isWorker()) {
				// Do nothing (it's captuerd onComplete() method)
			} else {
				if ( unit->getType() == UnitTypes::Terran_Vulture ) vulturesCreated++;
				plannerManager->rebalanceProduction();
				squadManager->unitTraining(unit);
			}
		}
		else {
			if (unit->getType().isBuilding()) {
				informationManager->markBuildingAsSeen(unit);
			}
		}
	}

	if (Broodwar->self()->isEnemy(unit->getPlayer())) {
		// debug info
		if (firstUnit.length() == 0) {
			firstUnit = unit->getType().getName();
			frameFirstUnit = Broodwar->getFrameCount();
		}

		squadManager->newEnemy(unit);
		if ( unit->getType().isResourceDepot() ) {
			informationManager->onEnemyResourceDepotShow(unit);
			// TODO the following instructions will fail if the spotted base isn't in a starting location
			if (!informationManager->_scoutedAnEnemyBase) {
				informationManager->enemyStartFound(unit->getPosition());
			}
			if (!informationManager->_enemyStartVisited) {
				informationManager->_enemyStartVisited = true;
			}
		}
		if (!unit->getType().isWorker() && !informationManager->_firstPush) {
#ifndef TOURNAMENT
			Broodwar->printf("First Push Detected");
#endif
			informationManager->_firstPush = true;
		}
		if (Broodwar->enemy()->getRace() != Races::Zerg) {
			if (unit->getType().isFlyer()) informationManager->_turretDefenses = true;
		}
		if ( unit->getType() == UnitTypes::Terran_Vulture_Spider_Mine && !informationManager->_scienceVesselDetector) {
			informationManager->_scienceVesselDetector = true;
			if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Starport)==0 && !buildManager->alreadyBuilding(UnitTypes::Terran_Starport)) {
				informationManager->criticalBuildRequest(UnitTypes::Terran_Science_Facility, true);
				informationManager->criticalBuildRequest(UnitTypes::Terran_Starport, true);
				informationManager->criticalBuildRequest(UnitTypes::Terran_Factory, true);
			}
		}
	}
	
	LOG4CXX_TRACE(_logger, "UNIT SHOW (END)");
}

void NovaAIModule::onUnitComplete(Unit unit) 
{
	if (Broodwar->isReplay()) return;
	LOG4CXX_TRACE(_logger, "UNIT COMPLETE (START)");
	if (Broodwar->self() == unit->getPlayer()) {
		LOG4CXX_TRACE(_logger, "Unit complete - " << unit->getType().getName().c_str() << " " << Broodwar->getFrameCount());
		if (unit->getType().isBuilding()) workerManager->onBuildingComplete(unit);

		if (unit->getType() == UnitTypes::Terran_Bunker) {
			squadManager->newBunkerSquad(unit);
		} else if (unit->getType().isWorker()) {
			workerManager->addUnit(unit);
// 		} else if (unit->getType() == UnitTypes::Terran_Refinery) {
// 			Broodwar << "Refinery completed!!!" << std::endl;
		}
	}
	LOG4CXX_TRACE(_logger, "UNIT SHOW (END)");
}

void NovaAIModule::onUnitHide(BWAPI::Unit unit)
{
	if (Broodwar->isReplay()) return;
	// WARNING: when a unit hides we don't have access to it: unknown type, owner, position, ...
	LOG4CXX_TRACE(_logger, "UNIT HIDE (" << unit << ") " << unit->getType().getName() << " [" << unit->getPlayer()->getName() << "]");
// 	DEBUG("UNIT HIDE (" << unit << ") " << unit->getType().getName() << " [" << unit->getPlayer()->getName() << "] " << 
// 		unit->getPosition().x << "," << unit->getPosition().y);

	squadManager->onEnemyEvade(unit);
}

void NovaAIModule::onUnitCreate(BWAPI::Unit unit)
{
}

void NovaAIModule::onUnitDestroy(BWAPI::Unit unit)
{
	if (Broodwar->isReplay()) return;
    if (unit->getType().isSpell()) return; // ignore spells

	LOG4CXX_TRACE(_logger, "UNIT DESTROY(" << unit << ") " << unit->getType().getName() << "[" << unit->getPlayer()->getName() << "]"); 
	if (Broodwar->self() == unit->getPlayer()) { // self unit
		if (unit->getType() == UnitTypes::Terran_Vulture_Spider_Mine) return; // skip spider mines
		if (unit->getType().isBuilding()) {
			productionManager->onBuildingDestroy(unit);
			buildManager->onBuildingDestroy(unit);
			if ( unit->getType() == UnitTypes::Terran_Command_Center ) informationManager->onCommandCenterDestroy(unit);
			if ( unit->getType() == UnitTypes::Terran_Missile_Turret ) informationManager->onMissileTurretDestroy(unit);
			if ( unit->getType() == UnitTypes::Terran_Bunker ) squadManager->onUnitDestroy(unit);
		} else if (unit->getType() == UnitTypes::Terran_SCV && unit->isCompleted()) { // we only track completed workers
			bool deleted = workerManager->onUnitDestroy(unit);
			if (!deleted) squadManager->onUnitDestroy(unit); // some SCV can be assigned to squad to repair tanks or bunkers
		} else {
			if ( unit->getType() == UnitTypes::Terran_Vulture ) {
				vulturesKilled++;
				totalVulutreLife += Broodwar->getFrameCount() - squadManager->getUnitFrameCreated(unit);
			}
			plannerManager->rebalanceProduction();
			squadManager->onUnitDestroy(unit);
		}
	} else if (Broodwar->self()->isEnemy(unit->getPlayer())) { // enemy unit
		squadManager->onEnemyDestroy(unit);
		if ( unit->getType().isResourceDepot() ) {
			informationManager->onEnemyResourceDepotDestroy(unit);
		}
		if ( unit->getType() == UnitTypes::Protoss_Zealot ) zealotsKilled++;
	} else if (unit->getType().isMineralField()) {
		workerManager->onMineralDestroy(unit);
		// TODO free space on buildMap (buildManager)
	} else if (unit->getType().isAddon()) { // Neutral add-on, could be self or enemy add-on
		productionManager->onBuildingDestroy(unit);
		squadManager->onEnemyDestroy(unit);
	}
	LOG4CXX_TRACE(_logger, "UNI DESTROY (END)");
}

void NovaAIModule::onUnitMorph(BWAPI::Unit unit)
{
	if (Broodwar->isReplay()) return;
	LOG4CXX_TRACE(_logger, "UNI MORPH (START)");

	if ( Broodwar->self() == unit->getPlayer() ) {
		if (unit->getType() == UnitTypes::Terran_Refinery) {
			buildManager->constructionPlaced(unit);
		}
	} else if (unit->getType().isRefinery()) {
		strategyManager->checkGasSteal(unit);
		squadManager->newEnemy(unit);
	} else if (unit->getType() == UnitTypes::Resource_Vespene_Geyser) {
		bool ourRefinery = workerManager->onRefineryDestroy(unit);
		if (ourRefinery) {
			// TODO rebuild it
		} else {
			squadManager->onEnemyDestroy(unit);
		}
	}

	LOG4CXX_TRACE(_logger, "UNI MORPH (END)");
}

void NovaAIModule::onUnitRenegade(BWAPI::Unit unit)
{
	//Broodwar->printf("Unit renegade");
}

void NovaAIModule::onEnd(bool isWinner)
{
	if (Broodwar->isReplay()) return;
	LOG4CXX_TRACE(_logger, "START onEND");


	BWAPI::Unitset enemyUnits = Broodwar->enemy()->getUnits();
	UnitToCache history = informationManager->seenEnemyHistory;
	
	//BWAPI::Unitset::iterator enemyIter;
	UnitToCache::iterator enemyIter;
	LOG("Enemy Units:");
	//for (enemyIter = enemyUnits.begin(); enemyIter != enemyUnits.end(); enemyIter++) {
	for (enemyIter = history.begin(); enemyIter != history.end(); enemyIter++) {
		BWAPI::Unit unit = enemyIter->first;
		if (enemyIter->second.type.isBuilding()) {
			LOG("    B: " << enemyIter->second.type << "    ID: " << unit->getID() << "    HP: " << unit->getHitPoints());
		}
		else {
			LOG("    " << enemyIter->second.type << "    ID: " << unit->getID() << "    HP: " << unit->getHitPoints());
		}
	}
	
	AbstractLayer search(squadManager->_squads); // import current game state to informationManager->gameState
	if (!ONLY_MICRO) {
		LOG("Frames: " << Broodwar->getFrameCount() << " winner: " << isWinner << " enemy: " << Broodwar->enemy()->getRace() <<
			" startPosition (" << informationManager->home->getCenter().x << "," << informationManager->home->getCenter().y << ")" <<
			" myKillScore: " << Broodwar->self()->getKillScore() << " enemyKillScore: " << Broodwar->enemy()->getKillScore() <<
			" EvaluationLastState: " << search.getEvaluation() << " map: " << Broodwar->mapFileName());// << Broodwar->enemy()->getUnits());
	} else {
		LOG("Frames: " << Broodwar->getFrameCount() << " winner: " << isWinner << " enemy: " << Broodwar->enemy()->getRace() <<
			" myKillScore: " << Broodwar->self()->getKillScore() << " enemyKillScore: " << Broodwar->enemy()->getKillScore() << 
			" map: " << Broodwar->mapFileName());
	}

	if (HIGH_LEVEL_SEARCH) {

		if (!stats.groupFrequency.empty()) {
// 			LOG("EvaluationCurrentState: " << search.getEvaluation() << " myKillScore: " << Broodwar->self()->getKillScore() << " enemyKillScore: " << Broodwar->enemy()->getKillScore() );

			if (SEARCH_ALGORITHM == "ABCD") {
// 				LOG("Avg stats per group (groupSize, frequency, AvgTime, AvgMinBranch, AvgMaxBranch, AvgAvgBranch, AvgTiemouts, AvgDownSamplings");
// 				auto groupFrequencyIt = stats.groupFrequency.begin();
// 				auto groupTimeIt = stats.groupTime.begin();
// 				auto groupBranchingIt = stats.groupBranching.begin();
// 				auto groupTiemoutsIt = stats.groupTimeouts.begin();
// 				auto groupDownSamplingsIt = stats.groupDownSamplings.begin();
// 				for (unsigned int i = 0; i < stats.groupTime.size(); ++i) {
// 					LOG((*groupFrequencyIt).first << "," <<
// 						(*groupFrequencyIt).second << "," <<
// 						(*groupTimeIt).second.getMean() << "," <<
// 						(*groupBranchingIt).second.getMin() << "," <<
// 						(*groupBranchingIt).second.getMax() << "," <<
// 						(*groupBranchingIt).second.getMean() << "," <<
// 						(*groupTiemoutsIt).second / (*groupFrequencyIt).second << "," <<
// 						(*groupDownSamplingsIt).second / (*groupFrequencyIt).second);
// 
// 					groupFrequencyIt++;
// 					groupTimeIt++;
// 					groupBranchingIt++;
// 					groupTiemoutsIt++;
// 					groupDownSamplingsIt++;
// 				}
			} else if (SEARCH_ALGORITHM == "MCTSCD") {
// 				LOG("Avg stats per group (groupSize, frequency, AvgTime)");
// 				auto groupFrequencyIt = stats.groupFrequency.begin();
// 				auto groupTimeIt = stats.groupTime.begin();
// 				for (unsigned int i = 0; i < stats.groupTime.size(); ++i) {
// 					LOG((*groupFrequencyIt).first << "," <<
// 						(*groupFrequencyIt).second << "," <<
// 						(*groupTimeIt).second / (*groupFrequencyIt).second);
// 
// 					groupFrequencyIt++;
// 					groupTimeIt++;
// 				}
			}
			LOG("Orders: " << stats.orders << " overwritten: " << stats.ordersOverwritten);
		}
    }

	
	if (HighLevelChangeRateExperiment) {
		LOG("High Level State change frequency [frameChange,Frequency]");
		for (std::map<unsigned int, unsigned long>::const_iterator it = stats.stateChange.begin(); it != stats.stateChange.end(); ++it) {
			LOG((*it).first << "," << (*it).second);
		}
	}

	/*
	int strategyConfig = LoadConfigInt("general", "strategy", 0);
	if (strategyConfig == 0) {
		// TODO warning, there is an error here!!
		// Check I/O files for learning
		// save winning strategy
		if (isWinner) {
			int index = (Broodwar->getStartLocations().size() - 2) * 3;
			informationManager->learningData[index + informationManager->strategySelected]++;
		}

		std::string writeFilename = LoadConfigString("learning", "write", "");

		// Get the opponent name and transform it in lower case
		std::string nameEnemy = Broodwar->enemy()->getName();
		std::transform(nameEnemy.begin(), nameEnemy.end(), nameEnemy.begin(), ::tolower);
		writeFilename += nameEnemy + ".txt";

		std::ofstream writeFile((char*)writeFilename.c_str(), std::ofstream::trunc);

		writeFile << ++informationManager->gamesSaved << std::endl;
		for (int i = 0; i < informationManager->learningDataSize; ++i) {
			writeFile << informationManager->learningData[i] << std::endl;
			writeFile.flush();
		}
		writeFile.close();
		delete informationManager->learningData;
	}
	*/
	
	fileLog.close();
	LOG4CXX_TRACE(_logger, "FINISH onEND");
}

void NovaAIModule::setUpLogging()
{
	//fileLog.open("bwapi-data\\logs\\NovaAIModule.log");
	fileLog.open("bwapi-data\\logs\\NovaAIModule.log", std::ios_base::app); //append the output
	LOG("==========================================================================");
	LOG("                               NEW GAME                                   ");

	if (HIGH_LEVEL_SEARCH) {
		std::ostringstream config;
		config << SEARCH_ALGORITHM;
		if (SEARCH_ALGORITHM == "ABCD") {
			config << " max_depth: " << LoadConfigInt("ABCD", "depth", 1);
			config << " downsampling: " << LoadConfigInt("ABCD", "downsampling");
			config << " time_limit: " << LoadConfigInt("ABCD", "time_limit", 1);
		} else if (SEARCH_ALGORITHM == "MCTSCD") {
			config << " max_depth: " << LoadConfigInt("MCTSCD", "depth", 1);
			config << " max_iterations: " << LoadConfigInt("MCTSCD", "iterations");
			config << " max_simulation_time: " << LoadConfigInt("MCTSCD", "max_simulation_time");
		}
		std::ostringstream abstraction;
		abstraction << "space_partition: " << LoadConfigString("high_level_search", "space_partition", "REGIONS_AND_CHOKEPOINTS");
		abstraction << " buildings: " << LoadConfigString("high_level_search", "buildings", "RESOURCE_DEPOT");
	
		LOG("High level refresh: " << HIGH_LEVEL_REFRESH);
		LOG(abstraction.str());
		LOG(config.str());
	}

	LOG("==========================================================================");
#ifdef LOG4CXX_STATIC
	_logger = log4cxx::Logger::getLogger("Nova");
	log4cxx::PropertyConfigurator::configure("bwapi-data/AI/logger.config");
	LOG4CXX_INFO(_logger, "Logging set up!");
#endif
}

void NovaAIModule::loadIniConfig()
{
    TCHAR currentPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentPath);
    configPath = std::string(currentPath) + "\\bwapi-data\\AI\\Nova.ini";

    std::string highLevel = LoadConfigString("high_level_search", "high_level_search", "OFF");
    if (highLevel == "ON") {
        HIGH_LEVEL_SEARCH = true;
		HIGH_LEVEL_REFRESH = LoadConfigInt("high_level_search", "refresh", 400);
		SEARCH_ALGORITHM = LoadConfigString("high_level_search", "algorithm", "ABCD");
    }
    
}