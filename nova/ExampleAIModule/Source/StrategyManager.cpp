#include "StrategyManager.h"

using namespace BWAPI;

StrategyManager::StrategyManager(ProductionManager *productionManager)
	: _logger(log4cxx::Logger::getLogger("StrategyManager")),
	_StateMachine(nullptr)
{
	_lastSweepFrame.clear();
	_productionManager = productionManager;
	_liftBuildings.clear();
	_hiddingCorners = true;
	_inferenceCounter = 0;

	// Marine rush
//	informationManager->_percentList[UnitTypes::Terran_Marine] = 100;

	// Marine+Medic rush
// 	informationManager->_percentList[UnitTypes::Terran_Marine] = 80;
// 	informationManager->_percentList[UnitTypes::Terran_Medic] = 20;
// 	informationManager->researchRequest(TechTypes::Stim_Packs);
//	informationManager->researchRequest(TechTypes::Optical_Flare);
// 	informationManager->upgradeRequest(UpgradeTypes::U_238_Shells);
// 	informationManager->upgradeRequest(UpgradeTypes::Terran_Infantry_Weapons);
// 	informationManager->buildRequest(UnitTypes::Terran_Comsat_Station, true);

	//BBS
// 	informationManager->buildRequest(UnitTypes::Terran_Barracks);
// 	informationManager->buildRequest(UnitTypes::Terran_Barracks);
// 	informationManager->buildRequest(UnitTypes::Terran_Supply_Depot);
// 	informationManager->_percentList[UnitTypes::Terran_Marine] = 100;
// 	informationManager->_useProxyPosition = true;
// 	informationManager->_bbs = true;
//     informationManager->_minSquadSize = 7;

	// Vulture rush
// 	informationManager->_percentList[UnitTypes::Terran_Vulture] = 100;
// 	informationManager->upgradeRequest(UpgradeTypes::Ion_Thrusters);
// 	informationManager->upgradeRequest(UpgradeTypes::Terran_Vehicle_Weapons, 3);
// 	informationManager->upgradeRequest(UpgradeTypes::Terran_Vehicle_Plating, 3);

	// Tank rush
// 	informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 100;
// 	informationManager->researchRequest(TechTypes::Tank_Siege_Mode);

	// Tank + dropship rush
// 	informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 90;
// 	informationManager->_percentList[UnitTypes::Terran_Dropship] = 10;
// 	informationManager->researchRequest(TechTypes::Tank_Siege_Mode);

	// Wraith rush
// 	informationManager->_percentList[UnitTypes::Terran_Wraith] = 100;
// 	informationManager->upgradeRequest(UpgradeTypes::Terran_Ship_Weapons, 3);
// 	informationManager->buildRequest(UnitTypes::Terran_Control_Tower, true);

	int strategyConfig = LoadConfigInt("general", "strategy", 0);

	if (strategyConfig == 0) { //LEARN
		// Check I/O files for learning--------------------------------------
		std::string readFilename = LoadConfigString("learning", "read", "");

		// Get the opponent name and transform it in lower case
		std::string nameEnemy = Broodwar->enemy()->getName();
		std::transform(nameEnemy.begin(), nameEnemy.end(), nameEnemy.begin(), ::tolower);
		readFilename += nameEnemy + ".txt";

		const int maxStrategies = 3;		// for now we consider 3 different strategies
		const int minStartingLocations = 2; // StarCraft maps are from 2 ...
		const int maxStartingLocations = 8; // ... till 8 players
		informationManager->learningData.resize(maxStartingLocations * maxStrategies);

		std::ifstream readFile((char*)readFilename.c_str(), std::ifstream::in);

		if (readFile.peek() != std::ifstream::traits_type::eof()) {
			int i = 0;
			readFile >> informationManager->gamesSaved;
			while (readFile >> informationManager->learningData[i]) {
				++i;
			}
		} else { // If the read-file is empty use default data
			for (auto& data : informationManager->learningData) {
				data = 0;
			}
		}

		// Select the best strategy using e-greedy ----------------------------
		int index = (Broodwar->getStartLocations().size() - minStartingLocations) * maxStrategies;
		std::uniform_real_distribution<> uniformDist(0, 1);
		double randomNumber = uniformDist(gen);
		double epsilon = 1.0 / (1 + informationManager->learningData[index] +
			informationManager->learningData[index + 1] +
			informationManager->learningData[index + 2]);

		if (randomNumber < epsilon) { // select random
			std::uniform_int_distribution<int> uniformDist(0, 2);
			informationManager->strategySelected = uniformDist(gen);
// 			LOG("Random Strategy: " << informationManager->strategySelected);
		} else { // select best strategy
// 			LOG("Learning Data (" << index << "): " << informationManager->learningData[index] << "," << informationManager->learningData[index+1] << "," << informationManager->learningData[index+2]);
			if (informationManager->learningData[index + 2] > informationManager->learningData[index] &&
				informationManager->learningData[index + 2] > informationManager->learningData[index + 1]) {
				informationManager->strategySelected = 2;
			} else if (informationManager->learningData[index] > informationManager->learningData[index + 1] &&
				informationManager->learningData[index] > informationManager->learningData[index + 2]) {
				informationManager->strategySelected = 0;
			} else {
				informationManager->strategySelected = 1;
			}
			//LOG("Best Strategy: " << informationManager->strategySelected);
		}
	} else {
		// get strategy from INI file
		informationManager->strategySelected = strategyConfig - 1;
	}

	std::string walling = LoadConfigString("general", "walling", "OFF");
	if (walling == "ON") {
		wallGenerator->LoadWallData();
		Broodwar << "Using walling" << std::endl;
	}
	
    // Initialize strategy-----------------------------------------------------
    if (informationManager->strategySelected == 0) { //BBS
		informationManager->buildRequest(UnitTypes::Terran_Barracks);
		informationManager->buildRequest(UnitTypes::Terran_Bunker);
		informationManager->_percentList[UnitTypes::Terran_Marine] = 100;
		informationManager->_useProxyPosition = true;
		informationManager->_workersNeededToBuild = 4;
		informationManager->_maxWorkersMining = 4;
		informationManager->_bbs = true;
		informationManager->_minSquadSize = 1;
		informationManager->_harassing = true;
		Broodwar->printf("Strategy selected: Bunker rush");
    } else {
        _StateMachine = new StateMachine<StrategyManager>(this);
        if (Broodwar->enemy()->getRace() == Races::Zerg) {
			if (informationManager->strategySelected == 1) {
				_StateMachine->ChangeState(OneRaxFE::Instance());
				Broodwar->printf("Strategy selected: OneRaxFE");
			} else {
				_StateMachine->ChangeState(TwoPortWraith::Instance());
				Broodwar->printf("Strategy selected: TwoPortWraith");
			}
		} else if (Broodwar->enemy()->getRace() == Races::Protoss || Broodwar->enemy()->getRace() == Races::Random) {
			if (informationManager->strategySelected == 1)  {
				_StateMachine->ChangeState(TwoFactTanks1::Instance());
				Broodwar->printf("Strategy selected: TwoFactTanks1");
			} else {
				_StateMachine->ChangeState(TwoFactMines::Instance());
				Broodwar->printf("Strategy selected: TwoFactMines");
			}
	    } else {
			if (informationManager->strategySelected == 1)  {
				_StateMachine->ChangeState(OneFactory::Instance());
				Broodwar->printf("Strategy selected: OneFactory");
			} else {
				_StateMachine->ChangeState(TwoPortWraith::Instance());
				Broodwar->printf("Strategy selected: TwoPortWraith");
			}
	    }
    }


};

StrategyManager::~StrategyManager()
{
	if (_StateMachine!=nullptr) delete _StateMachine;
}

void StrategyManager::onFrame()
{
	LOG4CXX_TRACE(_logger, "Update state machine");
	if (_StateMachine!=0) _StateMachine->Update();
    else if (informationManager->_bbs) {
        // enable auto supplies
		// TODO this is dangerous, we can end blocked by resources if we never have enough marines 
		// and getWorkersMining() is not accurate
		if (!informationManager->_autoBuildSuplies && Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Marine) >= 5) {
			informationManager->_autoBuildSuplies = true;
			informationManager->_maxWorkersMining = 0; // auto train workers
			Broodwar << "Auto train workers enabled" << std::endl;
		}
    }

	// Check reactive events ------------------------------------------------------
	// ************** Detected cloaked Units? ********************
	LOG4CXX_TRACE(_logger, "Handle cloaked units");
	if (!informationManager->_cloakedEnemyPositions.empty())
		handleCloakedEnemy();

	// ************** Gas steal in our base? **************
// 	LOG4CXX_TRACE(_logger, "Gas steal in our base?");
// 	if (_productionManager->_commandCenters.size() == 1) {
// 		UnitToCache allEnemies = informationManager->visibleEnemies;
// 		allEnemies.insert(informationManager->seenEnemies.begin(), informationManager->seenEnemies.end());
// 
// 		for (auto unitCache : allEnemies) {
// 			Unit enemy = unitCache.first;
// 			if (enemy->getType().isWorker()) {
// 				// check if is in the same region of a base
// 				BWTA::Region* workerRegion = BWTA::getRegion(enemy->getTilePosition());
// 				BWAPI::Unitset::iterator base=_productionManager->_commandCenters.begin();
// 				BWTA::Region* baseRegion = BWTA::getRegion( (*base)->getTilePosition() );
// 				if (workerRegion == baseRegion) {
// 					// if we don't have Refinery try to build it
// // 					if (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Refinery) == 0)
// // 						informationManager->criticalBuildRequest(UnitTypes::Terran_Refinery);
// 
// 					if (!workerManager->anyWorkerAttacking(enemy)) {
// 						workerManager->defenseBase(enemy);
// 					}
// 				}
// 			}
// 		}
// 	}

	// ************** Defense base from zergling rush and scout **************
	LOG4CXX_TRACE(_logger, "Defense base from any scout (and zergling rush)");
	// get all enemies in home region without workers attacking it
	BWAPI::Unitset enemiesAtHomeRegion;
	for (auto unitCache : informationManager->seenEnemies) {
		Unit enemy = unitCache.first;
		if (enemy->exists()) {
			BWTA::Region* enemyRegion = BWTA::getRegion(enemy->getTilePosition());
			if (enemyRegion == informationManager->home && !workerManager->anyWorkerAttacking(enemy)) {
				enemiesAtHomeRegion.insert(enemy);
			}
		}
	}
	for (auto squad : squadManager->_squads) {
		for (auto enemy : squad->_enemies) {
			if (enemy->exists()) {
				BWTA::Region* enemyRegion = BWTA::getRegion(enemy->getTilePosition());
				if (enemyRegion == informationManager->home && !workerManager->anyWorkerAttacking(enemy)) {
					enemiesAtHomeRegion.insert(enemy);
				}
			}
		}
	}
	// assign workers defenders to units
	bool workerSent;
	for (auto enemy : enemiesAtHomeRegion) {
		if (enemy->getType() == UnitTypes::Zerg_Zergling) {
			workerSent = workerManager->defenseBase(enemy);
			if (!workerSent) break; // we don't have more workers to defend
			workerSent = workerManager->defenseBase(enemy);
			if (!workerSent) break; // we don't have more workers to defend
		} else if (enemy->getType().isWorker()) {
			workerSent = workerManager->defenseBase(enemy);
			if (!workerSent) break; // we don't have more workers to defend
		}
	}


	// ************** scan empty bases for enemy **************
	LOG4CXX_TRACE(_logger, "Scan empty bases for enemy");
	informationManager->scanBases();

	// ************** need anti air units? **************
	LOG4CXX_TRACE(_logger, "Need anti air units?");
	double timeToKillEnemyAir = (informationManager->_enemyAirHP>0)? (informationManager->_ourAirDPS == 0)? 99999 : informationManager->_enemyAirHP/informationManager->_ourAirDPS : 0;
	//double timeToKillAntiAir = (totalSelfAirHP>0)? (totalEnemyAirDPS == 0 )? 99999 : totalSelfAirHP/totalEnemyAirDPS : 0;
	// if enemy is Zerg check for mutalsik
	if (Broodwar->enemy()->getRace() == Races::Zerg) {
		// TODO count mutalisk vs wraith
	} else {
		if (timeToKillEnemyAir > 5) {
			informationManager->_needAntiAirUnits = true;
			informationManager->checkRequirements(UnitTypes::Terran_Goliath);
			informationManager->upgradeRequest(UpgradeTypes::Charon_Boosters);
		} else {
			informationManager->_needAntiAirUnits = false;
		}
	}

	// ********** Search and destroy ********
	LOG4CXX_TRACE(_logger, "Search and destroy conditions");
	if (!informationManager->_searchAndDestroy && Broodwar->isVisible(TilePosition(informationManager->_enemyStartPosition))) {
		Unitset enemyUnits(Broodwar->getUnitsOnTile(TilePosition(informationManager->_enemyStartPosition), Filter::IsEnemy));
		if (enemyUnits.empty()) { // no enemy base found
			informationManager->_searchAndDestroy = true;
			LOG("Search and Destroy activated!");
			Broodwar->printf("Search and Destroy activated!");
			// check if we need the buildings to build Wraiths
			if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Starport) == 0 && !buildManager->alreadyBuilding(UnitTypes::Terran_Starport)) {
				informationManager->buildRequest(UnitTypes::Terran_Science_Facility, true);
				informationManager->buildRequest(UnitTypes::Terran_Starport, true);
			}
		}
	}

	// ********** Panic mode ********
	LOG4CXX_TRACE(_logger, "Panic mode conditions");
	if (informationManager->_panicMode) {
		if (_liftBuildings.size() == 0) { // search buildings that can lift
			BWAPI::Unitset allUnits = Broodwar->self()->getUnits();
			for (BWAPI::Unitset::iterator i = allUnits.begin(); i != allUnits.end(); ++i) {
				if ((*i)->getType().isFlyingBuilding() && !(*i)->isLifted()) {
					_liftBuildings.insert(*i);
					if ((*i)->isTraining()) (*i)->cancelTrain();
					if ((*i)->isResearching()) (*i)->cancelResearch();
					if ((*i)->isUpgrading()) (*i)->cancelUpgrade();
				}
			}
		}
		else { // hide buildings
			for (BWAPI::Unitset::iterator i = _liftBuildings.begin(); i != _liftBuildings.end();) {
				if (!(*i)->isLifted()) { // check if we are lifted
					(*i)->lift();
					++i;
				}
				else if (!(*i)->isIdle()) {
					// search best position to hide (map corners or middle corners)
					Position myBase = BWTA::getStartLocation(Broodwar->self())->getPosition();
					Position enemyBase = informationManager->_enemyStartPosition;
					std::set<BWAPI::Position> hiddingPositions;
					double newDistance;
					Position bestPlace;
					if (_hiddingCorners) {
						bestPlace = Position(0, 0);
						hiddingPositions.insert(Position(Broodwar->mapWidth()*TILE_SIZE, 0));
						hiddingPositions.insert(Position(0, Broodwar->mapHeight()*TILE_SIZE));
						hiddingPositions.insert(Position(Broodwar->mapWidth()*TILE_SIZE, Broodwar->mapHeight()*TILE_SIZE));
						_hiddingCorners = false;
					}
					else {
						bestPlace = Position(Broodwar->mapWidth()*TILE_SIZE, Broodwar->mapHeight()*TILE_SIZE / 2);
						hiddingPositions.insert(Position(Broodwar->mapWidth()*TILE_SIZE / 2, 0));
						hiddingPositions.insert(Position(0, Broodwar->mapHeight()*TILE_SIZE / 2));
						hiddingPositions.insert(Position(Broodwar->mapWidth()*TILE_SIZE / 2, Broodwar->mapHeight()*TILE_SIZE));
						_hiddingCorners = true;
					}
					double bestDistance = bestPlace.getDistance(myBase);
					bestDistance += bestPlace.getDistance(enemyBase);

					for (std::set<BWAPI::Position>::iterator pos = hiddingPositions.begin(); pos != hiddingPositions.end(); ++pos) {
						newDistance = (*pos).getDistance(myBase);
						newDistance += (*pos).getDistance(enemyBase);
						if (newDistance > bestDistance) {
							bestDistance = newDistance;
							bestPlace = (*pos);
						}
					}

					(*i)->move(bestPlace);
					++i;
				}
				else {
					i = _liftBuildings.erase(i);
				}
			}
		}
	}
	

	//mkozak - now that we're outside panic mode: Identify the likely enemy strategy based on observations
	_inferenceCounter++;
	if ((_inferenceCounter % 1000) == 0) {//only do this every 1000 frames

		//First let's check to see if we've even SEEN the enemy yet
		if(Broodwar->enemy()->getRace() != Races::Unknown) {
			Broodwar->sendText("Making Strategy Prediction");
			_inferenceCounter = 0;

			//Print the results of all observations
			Broodwar->sendText(Broodwar->enemy()->getRace().c_str());
			//Let's evaluate strategy by calling the information manager
			ITreeManager &mgr = informationManager->getTreeManager(Broodwar->enemy()->getRace());
			//GraphUtils::printTree(mgr.getTree(), "Strategies/Observed/TerranTechTree-Strengthened.dot", false);


			StrategyRecommendation recommendation = mgr.identifyStrategy();
			double air = (recommendation.proposedAirAggressiveness);
			double ground = (recommendation.proposedGroundAggressiveness);
			double attack = recommendation.proposedOverallAggressiveness;

			//tag the returned strategy as the current one
			_currentEnemyStrategy = recommendation.strategyIdentified;
			Broodwar->sendText(_currentEnemyStrategy.c_str());
			Broodwar->sendText(std::to_string(air).c_str());
			Broodwar->sendText(std::to_string(ground).c_str());
			Broodwar->sendText(std::to_string(attack).c_str());

			//recommend results
			if (air > 0.5) {//if we need to be aggressively air
				//_StateMachine->ChangeState(TwoPortWraith::Instance());
				informationManager->_percentList[UnitTypes::Terran_Marine] = 100;
				informationManager->_percentList[UnitTypes::Terran_Firebat] = 0;
				informationManager->_percentList[UnitTypes::Terran_Medic] = 0;
			}
			else if ((attack > 0.5) && (air > 0)) {//if we need to be aggressive and consider air
				//_StateMachine->ChangeState(OneRaxFE::Instance());
				informationManager->_percentList[UnitTypes::Terran_Marine] = 100;
				informationManager->_percentList[UnitTypes::Terran_Firebat] = 00;
				informationManager->_percentList[UnitTypes::Terran_Medic] = 0;
			}
			else if ((attack > 0.5) && (air < 0)) {//if we need to be aggressive and DON'T need to consider air
				//_StateMachine->ChangeState(TwoFactTanks1::Instance());
				informationManager->_percentList[UnitTypes::Terran_Marine] = 50;
				informationManager->_percentList[UnitTypes::Terran_Firebat] = 50;
				informationManager->_percentList[UnitTypes::Terran_Medic] = 0;
			}
			else if ((attack > 0) && (air > 0)) {//need to be able to attack air but stay offensive
												 //BBS
				informationManager->_percentList[UnitTypes::Terran_Marine] = 100;
				informationManager->_percentList[UnitTypes::Terran_Firebat] = 0;
				informationManager->_percentList[UnitTypes::Terran_Medic] = 0;
			}
			else if ((attack < 0) && (air > 0)) {//need to be able to attack air but stay defensive
				//_StateMachine->ChangeState(Sparks::Instance()); //Sparks - bioball
				informationManager->_percentList[UnitTypes::Terran_Marine] = 75;
				informationManager->_percentList[UnitTypes::Terran_Firebat] = 0;
				informationManager->_percentList[UnitTypes::Terran_Medic] = 25;
			}
			else if ((attack < 0) && (air < 0)) {//don't need to be able to attack air but still defend
				//_StateMachine->ChangeState(TwoFactMines::Instance());
				informationManager->_percentList[UnitTypes::Terran_Marine] = 25;
				informationManager->_percentList[UnitTypes::Terran_Firebat] = 50;
				informationManager->_percentList[UnitTypes::Terran_Medic] = 25;
			}
			else {//bbs
				informationManager->_percentList[UnitTypes::Terran_Marine] = 100;
				informationManager->_percentList[UnitTypes::Terran_Firebat] = 0;
				informationManager->_percentList[UnitTypes::Terran_Medic] = 0;
			}

			//bias build orders
			/*
			informationManager->_percentList[UnitTypes::Terran_Marine] = 100;
			informationManager->_percentList[UnitTypes::Terran_Firebat] = 100;
			informationManager->_percentList[UnitTypes::Terran_Vulture] = 100;
			informationManager->_percentList[UnitTypes::Terran_Siege_Tank_Siege_Mode] = 100;
			informationManager->_percentList[UnitTypes::Terran_Wraith] = 100;
			informationManager->_percentList[UnitTypes::Terran_Valkyrie] = 100;
			informationManager->_percentList[UnitTypes::Terran_Dropship] = 100;
			informationManager->_percentList[UnitTypes::Terran_Goliath] = 100;
			informationManager->_percentList[UnitTypes::Terran_Missile_Turret] = 100;
			informationManager->_percentList[UnitTypes::Terran_Science_Vessel] = 100;
			*/
		}
	}

// 	Broodwar->drawTextScreen(290,52,"Enemy Air DPS: %0.2f", informationManager->_enemyAirDPS);
// 	Broodwar->drawTextScreen(290,62,"Time Kill Enemy Air: %0.2f", timeToKillEnemyAir);
// 	Broodwar->drawTextScreen(290,72,"Barrack Production: %s", informationManager->_trainOrder[UnitTypes::Terran_Barracks].getName().c_str());
// 	Broodwar->drawTextScreen(290,82,"Factory Production: %s", informationManager->_trainOrder[UnitTypes::Terran_Factory].getName().c_str());
// 	Broodwar->drawTextScreen(290,92,"Starport Production: %s", informationManager->_trainOrder[UnitTypes::Terran_Starport].getName().c_str());

	// State info
	if (_StateMachine!=0) {
		Broodwar->drawTextScreen(437,17,"%cState: %c%s", 0x07, 0x03, _StateMachine->GetCurrentState()->GetName());
	}
	//Broodwar->drawTextScreen(437,17,"M: %d G: %d", informationManager->minerals(), informationManager->gas());
	LOG4CXX_TRACE(_logger, "DONE");
}


void StrategyManager::handleCloakedEnemy() {
	if (!informationManager->_comsatStation.empty() && !informationManager->_cloakedEnemyPositions.empty()) {
		for (UnitToTimeMap::iterator unit = informationManager->_comsatStation.begin(); unit != informationManager->_comsatStation.end(); ++unit) {
			// TODO select best position (at the moment we only get the first)
			TilePositionSet::iterator posIter = informationManager->_cloakedEnemyPositions.begin();
			TilePosition pos = *posIter;

			if (_lastSweepFrame.find(pos) != _lastSweepFrame.end()) {
				if (Broodwar->getFrameCount() - _lastSweepFrame[pos] < SCANNER_SWEEP_FREQUENCY) {
					break;
				}
			}

			// Use scanner sweep if possible
			if (unit->first->getEnergy() >= 50) {
				bool scanned = informationManager->useScanner(unit->first, Position(pos));
				if (scanned) {
					_lastSweepFrame[pos] = Broodwar->getFrameCount();
					break;
				}
			}
		}
	} else if ( !buildManager->alreadyBuilding(UnitTypes::Terran_Comsat_Station) ) { // Build Comsat Station
		informationManager->criticalBuildRequest(UnitTypes::Terran_Comsat_Station);
	}

	// Execute Science Vessel plan
	informationManager->_scienceVesselDetector = true;
	if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Starport)==0 && !buildManager->alreadyBuilding(UnitTypes::Terran_Starport)) {
		informationManager->criticalBuildRequest(UnitTypes::Terran_Science_Facility, true);
		informationManager->criticalBuildRequest(UnitTypes::Terran_Starport, true);
		informationManager->criticalBuildRequest(UnitTypes::Terran_Factory, true);
	}

	// Execute missile turret defense plan
	if (Broodwar->enemy()->getRace() != Races::Zerg) {
		informationManager->_turretDefenses = true;
	} else if (informationManager->_enemyMutalisk > 3) {
		informationManager->_turretDefenses = true;
	}

	// Clear notifications
	informationManager->_cloakedEnemyPositions.clear();
}

void StrategyManager::checkGasSteal(Unit unit)
{
	if (_productionManager->_commandCenters.size() == 1) {
		// check if is in the same region of a base
		BWTA::Region* refineryRegion = BWTA::getRegion( unit->getTilePosition() );
		BWAPI::Unitset::iterator base=_productionManager->_commandCenters.begin();
		BWTA::Region* baseRegion = BWTA::getRegion( (*base)->getTilePosition() );
		if (refineryRegion == baseRegion) {
			Broodwar->sendText("Bastard you stole my gas!");
			// send workers to destroy it
			workerManager->attackBuilding(unit);

			TilePosition geyserLocation = unit->getTilePosition();
			// check if refinery is already build it
			bool refineryExist = false;
			BWAPI::Unitset unitsOnGeyser = BWAPI::Broodwar->getUnitsOnTile(geyserLocation.x, geyserLocation.y);
			
			if (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Barracks) == 0 && Broodwar->self()->incompleteUnitCount(UnitTypes::Terran_Barracks) == 0 )
				informationManager->criticalBuildRequest(UnitTypes::Terran_Barracks);
		}
	}
}