#include "ProductionManager.h"
using namespace BWAPI;

ProductionManager::ProductionManager()
	: _academy(0),
	_engineeringBay(0),
	_scienceFacility(0),
	_physicsLab(0),
	_covertOps(0),
	_logger(log4cxx::Logger::getLogger("ProductionManager"))
{
	_commandCenters.clear();
	_barracks.clear();
	_factories.clear();
	_starports.clear();
	_machineShop.clear();
	_armory.clear();
	_controlTower.clear();
	_nuclearSilo.clear();
};

//TODO make destructor

void ProductionManager::onBuildingShow(Unit unit)
{

	UnitType parentType = unit->getType().whatBuilds().first;

	if (unit->getType() == UnitTypes::Terran_Command_Center) {
		_commandCenters.insert(unit);
		if (workerManager->needWorkers())
			unit->train(UnitTypes::Terran_SCV);
	} else if (unit->getType() == UnitTypes::Terran_Barracks) {
		_barracks.insert(unit);
        if (!HIGH_LEVEL_SEARCH) {
		    unit->setRallyPoint(getRallyPosition(unit));
        }
	} else if (unit->getType() == UnitTypes::Terran_Factory) {
		_factories.insert(unit);
        if (!HIGH_LEVEL_SEARCH) {
		    unit->setRallyPoint(getRallyPosition(unit));
        }
	} else if (unit->getType() == UnitTypes::Terran_Starport)
		_starports.insert(unit);
	else if (unit->getType() == UnitTypes::Terran_Academy)
		_academy = unit;
	else if (unit->getType() == UnitTypes::Terran_Engineering_Bay)
		_engineeringBay = unit;
	else if (unit->getType() == UnitTypes::Terran_Machine_Shop) {
		_machineShop.insert(unit);
		if (!informationManager->_addonOrder[parentType].empty())
			informationManager->_addonOrder[parentType].erase(informationManager->_addonOrder[parentType].begin());
	} else if (unit->getType() == UnitTypes::Terran_Control_Tower) {
		_controlTower.insert(unit);
		if (!informationManager->_addonOrder[parentType].empty())
			informationManager->_addonOrder[parentType].erase(informationManager->_addonOrder[parentType].begin());
	} else if (unit->getType() == UnitTypes::Terran_Armory)
		_armory.insert(unit);
	else if (unit->getType() == UnitTypes::Terran_Science_Facility)
		_scienceFacility = unit;
	else if (unit->getType() == UnitTypes::Terran_Physics_Lab) {
		_physicsLab = unit;
		if (!informationManager->_addonOrder[parentType].empty())
			informationManager->_addonOrder[parentType].erase(informationManager->_addonOrder[parentType].begin());
	} else if (unit->getType() == UnitTypes::Terran_Covert_Ops) {
		_covertOps = unit;
		if (!informationManager->_addonOrder[parentType].empty())
			informationManager->_addonOrder[parentType].erase(informationManager->_addonOrder[parentType].begin());
	} else if (unit->getType() == UnitTypes::Terran_Comsat_Station) {
		//informationManager->_comsatStation.insert(unit);
		informationManager->_comsatStation[unit] = Broodwar->getFrameCount();
		if (!informationManager->_addonOrder[parentType].empty())
			informationManager->_addonOrder[parentType].erase(informationManager->_addonOrder[parentType].begin());
	} else if (unit->getType() == UnitTypes::Terran_Nuclear_Silo) {
		_nuclearSilo.insert(unit);
		if (!informationManager->_addonOrder[parentType].empty())
			informationManager->_addonOrder[parentType].erase(informationManager->_addonOrder[parentType].begin());
	}
}

Position ProductionManager::getRallyPosition(Unit unit)
{
	// TODO save region->bestChokepoint to avoid compute each time
	BWTA::Region* region = BWTA::getRegion(unit->getPosition());
	std::set<BWTA::Chokepoint*> chokes = region->getChokepoints();
	// get choke point closest to the center of the map
	double distance = 999999;
	double bestRegionDist = 0;

	Position centerMap(Broodwar->mapWidth() * TILE_SIZE/2, Broodwar->mapHeight() * TILE_SIZE/2);
	BWTA::Chokepoint* bestChokepoint = *chokes.begin();
	for (std::set<BWTA::Chokepoint*>::const_iterator it = chokes.begin(); it != chokes.end(); ++it) {
		double chokeToRegionDist = region->getCenter().getDistance((*it)->getCenter());
		if (bestRegionDist == 0) bestRegionDist = chokeToRegionDist;
		double chokeToMapDist = BWTA::getGroundDistance(TilePosition(centerMap), TilePosition((*it)->getCenter()));
		if (chokeToMapDist < distance && chokeToRegionDist < bestRegionDist*2 ) { // second part to avoid problems where there is a "wrong" chokepoint
			distance = chokeToMapDist;
			bestRegionDist = chokeToRegionDist;
			bestChokepoint = *it;
		}
	}

	if (region == informationManager->home) {
		if (informationManager->_initialRallyPosition == Positions::None) {
			// TODO this use to be bunker seed and rally position, review to be only rally position
			// search for home Rally Position
			BWTA::Region* nextRegion = region;
			int searchDepth = 2;
			for (int j=1;j<searchDepth;++j) {
				std::pair<BWTA::Region*,BWTA::Region*> sides = bestChokepoint->getRegions();
				int groundHeight1 = Broodwar->getGroundHeight(TilePosition(sides.second->getCenter()));
				int groundHeight2 = Broodwar->getGroundHeight(TilePosition(sides.second->getCenter()));
				if (groundHeight1 != groundHeight1) break; // we have a ramp chokepoint
				if (bestChokepoint->getWidth() < 90) break; // we have a ramp chokepoint
				if (sides.first == nextRegion) {
					nextRegion = sides.second;
				} else {
					nextRegion = sides.first;
				}
				chokes = nextRegion->getChokepoints();
				distance = 999999;
				for (std::set<BWTA::Chokepoint*>::const_iterator it = chokes.begin(); it != chokes.end(); ++it) {
					double chokeToMapDist = BWTA::getGroundDistance(TilePosition(centerMap), TilePosition((*it)->getCenter()));
					if (chokeToMapDist < distance) {
						distance = chokeToMapDist;
						bestChokepoint = *it;
					}
				}
			}
			//informationManager->_initialRallyPosition = bestChokepoint->getCenter();
			informationManager->_initialRallyPosition = Position(buildManager->getBuildLocationNear(TilePosition(bestChokepoint->getCenter()), UnitTypes::Terran_Bunker, nextRegion));
		} else {
			return informationManager->_initialRallyPosition;
		}
	}

	if (distance == 999999) {
		LOG4CXX_ERROR(_logger, "Not best chokepoint found");
		return unit->getPosition();
	} else {
		return bestChokepoint->getCenter();
	}
}

void ProductionManager::onBuildingDestroy(Unit unit)
{
	if (unit->getType() == UnitTypes::Terran_Command_Center)
		_commandCenters.erase(unit);
	else if (unit->getType() == UnitTypes::Terran_Barracks)
		_barracks.erase(unit);
	else if (unit->getType() == UnitTypes::Terran_Factory)
		_factories.erase(unit);
	else if (unit->getType() == UnitTypes::Terran_Starport)
		_starports.erase(unit);
	else if (unit->getType() == UnitTypes::Terran_Academy)
		_academy = 0;
	else if (unit->getType() == UnitTypes::Terran_Engineering_Bay)
		_engineeringBay = 0;
	else if (unit->getType() == UnitTypes::Terran_Machine_Shop) {
		if ( _machineShop.find(unit) != _machineShop.end()) {
			_machineShop.erase(unit);
		}
	} else if (unit->getType() == UnitTypes::Terran_Control_Tower) {
		if ( _controlTower.find(unit) != _controlTower.end())
			_controlTower.erase(unit);
	} else if (unit->getType() == UnitTypes::Terran_Armory)
		if ( _armory.find(unit) != _armory.end()) {
			_armory.erase(unit);
		}
	else if (unit->getType() == UnitTypes::Terran_Science_Facility)
		_scienceFacility = 0;
	else if (unit->getType() == UnitTypes::Terran_Physics_Lab) {
		if (_physicsLab == unit)
			_physicsLab = 0;
	} else if (unit->getType() == UnitTypes::Terran_Covert_Ops) {
		if (_covertOps == unit)
			_covertOps = 0;
	} else if (unit->getType() == UnitTypes::Terran_Comsat_Station) {
		if ( informationManager->_comsatStation.find(unit) != informationManager->_comsatStation.end())
			informationManager->_comsatStation.erase(unit);
	} else if (unit->getType() == UnitTypes::Terran_Nuclear_Silo) {
		if ( _nuclearSilo.find(unit) != _nuclearSilo.end())
			_nuclearSilo.erase(unit);
	}
}

void ProductionManager::onFrame()
{
	if (informationManager->_panicMode) return;
	// Control PRODUCTION on command centers -----------------------------------------------------------
	for (BWAPI::Unitset::iterator unit = _commandCenters.begin(); unit != _commandCenters.end(); ++unit) {	
		if ((*unit)->isTraining() || !(*unit)->isCompleted()) continue; //look to next building
		if ( !informationManager->_addonOrder[(*unit)->getType()].empty() && (*unit)->getAddon() == NULL) {
			// Addon request
			UnitType addonType = informationManager->_addonOrder[(*unit)->getType()].at(0);
			if ( informationManager->haveResources(addonType) && ( (addonType==UnitTypes::Terran_Comsat_Station && Broodwar->self()->completedUnitCount(UnitTypes::Terran_Academy) > 0) || 
				(addonType==UnitTypes::Terran_Nuclear_Silo && Broodwar->self()->completedUnitCount(UnitTypes::Terran_Covert_Ops) > 0) ) ) {
					bool ok = (*unit)->buildAddon(addonType);
					if (!ok) {
						if (PRINT_PRODUCTION) Broodwar->printf("[COMMAND ERROR] %s", Broodwar->getLastError().toString().c_str() );
						informationManager->frameSpend(addonType);
					}
			}
		} else if ( workerManager->needWorkers() ) {
			if (!(*unit)->isTraining() && (*unit)->isCompleted()) {
				if (canTrain(UnitTypes::Terran_SCV)) {
					if ( (*unit)->getAddon()!=NULL && !(*unit)->getAddon()->isCompleted() ) continue;
					bool ok = (*unit)->train(UnitTypes::Terran_SCV);
					if (!ok) {
						if (PRINT_PRODUCTION) Broodwar->printf("[COMMAND ERROR] %s", Broodwar->getLastError().toString().c_str() );
						informationManager->frameSpend(UnitTypes::Terran_SCV);
					}
				}
			}
		} else {
			// If we are plenty of resources, build Terran_Comsat_Station
			if ((*unit)->getAddon() == NULL && !buildManager->alreadyBuilding(UnitTypes::Terran_Comsat_Station)) {
				if (informationManager->minerals() >= 500 || _commandCenters.size() > 2) {
					informationManager->buildRequest(UnitTypes::Terran_Comsat_Station);
				}
// 				if ( !buildManager->alreadyBuilding(UnitTypes::Terran_Comsat_Station) && (informationManager->minerals() >= 500 || _commandCenters.size() > 2) ) {
// 					informationManager->buildRequest(UnitTypes::Terran_Comsat_Station);
// 				} else if (informationManager->minerals() >= 200) {
// 					informationManager->buildRequest(UnitTypes::Terran_Comsat_Station);
// 				}
			}
		}
	}

	// avoid production if we want to build a Command Center
	if (buildManager->_buildOrder.size() > 0  && buildManager->_buildOrder.at(0).isResourceDepot() && informationManager->minerals() < 500 && informationManager->_priorCommandCenters) {
		#ifndef TOURNAMENT
			printBlockedProduction(_barracks,"Blocked by new Command Center",informationManager->_wastedProductionFramesByCommandCenter);
			printBlockedProduction(_starports,"Blocked by new Command Center",informationManager->_wastedProductionFramesByCommandCenter);
			printBlockedProduction(_factories,"Blocked by new Command Center",informationManager->_wastedProductionFramesByCommandCenter);
		#endif
		return;
	}

	// RESEARCH - UPGRADES ---------------------------------------------------------------------------------------------------

	bool blockUnitProduction = false;

	if (informationManager->_armySize != 0) { // prioritize produce units
		// Control RESEARCH/UPGRADE on Academy
		if (_academy != 0 && _academy->isCompleted() && !_academy->isResearching() && !_academy->isUpgrading() ) {
			if (informationManager->_researchOrder[_academy->getType()].size() > 0) {
				TechType toResearch = informationManager->_researchOrder[_academy->getType()].at(0);
				if (informationManager->haveResources(toResearch) ) {
					bool ok = _academy->research(toResearch);
					if (!ok) {
						if (PRINT_PRODUCTION) Broodwar->printf("[ACADEMY ERROR] %s", Broodwar->getLastError().toString().c_str() );
						informationManager->frameSpend(toResearch);
					} else {
						informationManager->_researchOrder[_academy->getType()].erase(informationManager->_researchOrder[_academy->getType()].begin());
					}
				} else {
					blockUnitProduction = true;
				}
			} else if (informationManager->_upgradeOrder[_academy->getType()].size() > 0) {
 				UpgradeType toUpgrade = informationManager->_upgradeOrder[_academy->getType()].at(0);
				if (informationManager->haveResources(toUpgrade) ) {
					bool ok = _academy->upgrade(toUpgrade);
					if (!ok) {
						if (PRINT_PRODUCTION) Broodwar->printf("[ACADEMY ERROR] %s", Broodwar->getLastError().toString().c_str() );
						informationManager->frameSpend(toUpgrade);
					} else {
						informationManager->_upgradeOrder[_academy->getType()].erase(informationManager->_upgradeOrder[_academy->getType()].begin());
					}
				} else {
					blockUnitProduction = true;
				}
			}
		}

		// Control UPGRADE on Engineering Bay
		if (_engineeringBay != 0 && _engineeringBay->isCompleted() && !_engineeringBay->isUpgrading() ) {
			UpgradeType toUpgrade = getNextInfanteryUpgrade();
			if (toUpgrade != UpgradeTypes::None) {
				if (informationManager->haveResources(toUpgrade) ) {
					bool ok = _engineeringBay->upgrade(toUpgrade);
					if (!ok) {
						if (PRINT_PRODUCTION) Broodwar->printf("[BAY ERROR] %s", Broodwar->getLastError().toString().c_str() );
						informationManager->frameSpend(toUpgrade);
					} else {
						//DEBUG("E-bay block production");
						blockUnitProduction = true;
					}
				}
			}
		}

		// Control UPGRADE on Machine Shop
		if (!_machineShop.empty()) {
			for (BWAPI::Unitset::iterator unit = _machineShop.begin(); unit != _machineShop.end(); ++unit) {
				if ((*unit)->isCompleted() && !(*unit)->isResearching() && !(*unit)->isUpgrading() ) {
					if (informationManager->_researchOrder[(*unit)->getType()].size() > 0) {
						TechType toResearch = informationManager->_researchOrder[(*unit)->getType()].at(0);
						if (informationManager->haveResources(toResearch) ) {
							bool ok = (*unit)->research(toResearch);
							if (!ok) {
								if (PRINT_PRODUCTION) Broodwar->printf("[MACHINE SHOP ERROR] %s", Broodwar->getLastError().toString().c_str() );
								informationManager->frameSpend(toResearch);
							} else {
								informationManager->_researchOrder[(*unit)->getType()].erase(informationManager->_researchOrder[(*unit)->getType()].begin());
							}
						} else {
							blockUnitProduction = true;
						}
					} else if (informationManager->_upgradeOrder[(*unit)->getType()].size() > 0) {
						UpgradeType toUpgrade = informationManager->_upgradeOrder[(*unit)->getType()].at(0);
						if (informationManager->haveResources(toUpgrade) && (toUpgrade!=UpgradeTypes::Charon_Boosters || Broodwar->self()->completedUnitCount(UnitTypes::Terran_Armory)>0) ) {
							bool ok = (*unit)->upgrade(toUpgrade);
							if (!ok) {
								if (PRINT_PRODUCTION) Broodwar->printf("[MACHINE SHOP ERROR] %s", Broodwar->getLastError().toString().c_str() );
								informationManager->frameSpend(toUpgrade);
							} else {
								informationManager->_upgradeOrder[(*unit)->getType()].erase(informationManager->_upgradeOrder[(*unit)->getType()].begin());
							}
						}
					}
				}
			}
		}

		// Control UPGRADE on Control Tower
		if (!_controlTower.empty()) {
			for (BWAPI::Unitset::iterator unit = _controlTower.begin(); unit != _controlTower.end(); ++unit) {
				if ((*unit)->isCompleted() && !(*unit)->isResearching() && !(*unit)->isUpgrading() ) {
					if (informationManager->_researchOrder[(*unit)->getType()].size() > 0) {
						TechType toResearch = informationManager->_researchOrder[(*unit)->getType()].at(0);
						if (informationManager->haveResources(toResearch) ) {
							bool ok = (*unit)->research(toResearch);
							if (!ok) {
								if (PRINT_PRODUCTION) Broodwar->printf("[CONTROL TOWER ERROR] %s", Broodwar->getLastError().toString().c_str() );
								informationManager->frameSpend(toResearch);
							} else {
								informationManager->_researchOrder[(*unit)->getType()].erase(informationManager->_researchOrder[(*unit)->getType()].begin());
							}
						} else {
							blockUnitProduction = true;
						}
					} else if (informationManager->_upgradeOrder[(*unit)->getType()].size() > 0) {
						UpgradeType toUpgrade = informationManager->_upgradeOrder[(*unit)->getType()].at(0);
						if (informationManager->haveResources(toUpgrade) ) {
							bool ok = (*unit)->upgrade(toUpgrade);
							if (!ok) {
								if (PRINT_PRODUCTION) Broodwar->printf("[CONTROL TOWER ERROR] %s", Broodwar->getLastError().toString().c_str() );
								informationManager->frameSpend(toUpgrade);
							} else {
								informationManager->_upgradeOrder[(*unit)->getType()].erase(informationManager->_upgradeOrder[(*unit)->getType()].begin());
							}
						} else {
							blockUnitProduction = true;
						}
					}
				}
			}
		}

		// Control RESEARCH/UPGRADE on Science Facility
		if (_scienceFacility != 0 && _scienceFacility->isCompleted() && !_scienceFacility->isResearching() && !_scienceFacility->isUpgrading() ) {
			if (informationManager->_researchOrder[_scienceFacility->getType()].size() > 0) {
				TechType toResearch = informationManager->_researchOrder[_scienceFacility->getType()].at(0);
				if (informationManager->haveResources(toResearch) ) {
					bool ok = _scienceFacility->research(toResearch);
					if (!ok) {
						if (PRINT_PRODUCTION) Broodwar->printf("[SCIENCE ERROR] %s", Broodwar->getLastError().toString().c_str() );
						informationManager->frameSpend(toResearch);
					} else {
						informationManager->_researchOrder[_scienceFacility->getType()].erase(informationManager->_researchOrder[_scienceFacility->getType()].begin());
					}
				} else {
					blockUnitProduction = true;
				}
			} else if (informationManager->_upgradeOrder[_scienceFacility->getType()].size() > 0) {
				UpgradeType toUpgrade = informationManager->_upgradeOrder[_scienceFacility->getType()].at(0);
				if (informationManager->haveResources(toUpgrade) ) {
					bool ok = _scienceFacility->upgrade(toUpgrade);
					if (!ok) {
						if (PRINT_PRODUCTION) Broodwar->printf("[SCIENCE ERROR] %s", Broodwar->getLastError().toString().c_str() );
						informationManager->frameSpend(toUpgrade);
					} else {
						informationManager->_upgradeOrder[_scienceFacility->getType()].erase(informationManager->_upgradeOrder[_scienceFacility->getType()].begin());
					}
				} else {
					blockUnitProduction = true;
				}
			} else if ( !informationManager->_addonOrder[_scienceFacility->getType()].empty() && _scienceFacility->getAddon() == NULL) {
				// Addon request
				UnitType addonType = informationManager->_addonOrder[_scienceFacility->getType()].at(0);
				if ( informationManager->haveResources(addonType) ) {
					bool ok = _scienceFacility->buildAddon(addonType);
					if (!ok) {
						if (PRINT_PRODUCTION) Broodwar->printf("[SCIENCE ERROR] %s", Broodwar->getLastError().toString().c_str() );
						informationManager->frameSpend(addonType);
					}
				}
			}
		}

		// Control RESEARCH/UPGRADE on Armory
		if (!_armory.empty()) {
			for (BWAPI::Unitset::iterator unit = _armory.begin(); unit != _armory.end(); ++unit) {
				if ((*unit)->isCompleted() && !(*unit)->isUpgrading() ) {
					UpgradeType toUpgrade = getNextArmoryUpgrade();
					if (toUpgrade != UpgradeTypes::None) {
						if (informationManager->haveResources(toUpgrade)) {
							bool ok = (*unit)->upgrade(toUpgrade);
							if (!ok) {
								if (PRINT_PRODUCTION) Broodwar->printf("[ARMORY ERROR] %s", Broodwar->getLastError().toString().c_str() );
								informationManager->frameSpend(toUpgrade);
							}
						} else {
							//DEBUG("Armory block production");
							blockUnitProduction = true;
						}
					}
				}
			}
		}

		// Control UPGRADE on Physic Lab
		if (_physicsLab != 0 && _physicsLab->isCompleted() && !_physicsLab->isResearching() && !_physicsLab->isUpgrading() ) {
			if (informationManager->_researchOrder[_physicsLab->getType()].size() > 0) {
				TechType toResearch = informationManager->_researchOrder[_physicsLab->getType()].at(0);
				if (informationManager->haveResources(toResearch) ) {
					bool ok = _physicsLab->research(toResearch);
					if (!ok) {
						if (PRINT_PRODUCTION) Broodwar->printf("[PHYSIC LAB ERROR] %s", Broodwar->getLastError().toString().c_str() );
						informationManager->frameSpend(toResearch);
					} else {
						informationManager->_researchOrder[_physicsLab->getType()].erase(informationManager->_researchOrder[_physicsLab->getType()].begin());
					}
				} else {
					blockUnitProduction = true;
				}
			} else if (informationManager->_upgradeOrder[_physicsLab->getType()].size() > 0) {
				UpgradeType toUpgrade = informationManager->_upgradeOrder[_physicsLab->getType()].at(0);
				if (informationManager->haveResources(toUpgrade) ) {
					bool ok = _physicsLab->upgrade(toUpgrade);
					if (!ok) {
						if (PRINT_PRODUCTION) Broodwar->printf("[PHYSIC LAB ERROR] %s", Broodwar->getLastError().toString().c_str() );
						informationManager->frameSpend(toUpgrade);
					} else {
						informationManager->_upgradeOrder[_physicsLab->getType()].erase(informationManager->_upgradeOrder[_physicsLab->getType()].begin());
					}
				}
			}
		}

		// Control UPGRADE on Covert Ops
		if (_covertOps != 0 && _covertOps->isCompleted() && !_covertOps->isResearching() && !_covertOps->isUpgrading() ) {
			if (informationManager->_researchOrder[_covertOps->getType()].size() > 0) {
				TechType toResearch = informationManager->_researchOrder[_covertOps->getType()].at(0);
				if (informationManager->haveResources(toResearch) ) {
					bool ok = _covertOps->research(toResearch);
					if (!ok) {
						if (PRINT_PRODUCTION) Broodwar->printf("[COVERT OPS ERROR] %s", Broodwar->getLastError().toString().c_str() );
						informationManager->frameSpend(toResearch);
					} else {
						informationManager->_researchOrder[_covertOps->getType()].erase(informationManager->_researchOrder[_covertOps->getType()].begin());
					}
				} else {
					blockUnitProduction = true;
				}
			} else if (informationManager->_upgradeOrder[_covertOps->getType()].size() > 0) {
				UpgradeType toUpgrade = informationManager->_upgradeOrder[_covertOps->getType()].at(0);
				if (informationManager->haveResources(toUpgrade) ) {
					bool ok = _covertOps->upgrade(toUpgrade);
					if (!ok) {
						if (PRINT_PRODUCTION) Broodwar->printf("[COVERT OPS ERROR] %s", Broodwar->getLastError().toString().c_str() );
						informationManager->frameSpend(toUpgrade);
					} else {
						informationManager->_upgradeOrder[_covertOps->getType()].erase(informationManager->_upgradeOrder[_covertOps->getType()].begin());
					}
				} else {
					blockUnitProduction = true;
				}
			}
		}
	}

	// UNIT PRODUCTION -------------------------------------------------------------------------------------------------------
	
	if (!blockUnitProduction) {
		// Control PRODUCTION on Barracks
		if (!_barracks.empty()) {
			bool productionLimited = onProduction(_barracks);
			if (productionLimited) {
				int moneyNeeded = UnitTypes::Terran_Supply_Depot.mineralPrice() + UnitTypes::Terran_Barracks.mineralPrice() + informationManager->_trainOrder[UnitTypes::Terran_Barracks].mineralPrice();
				int gasNeeded = informationManager->_trainOrder[UnitTypes::Terran_Barracks].gasPrice() + informationManager->_trainOrder[UnitTypes::Terran_Factory].gasPrice();
				if ( informationManager->minerals() >= moneyNeeded && informationManager->gas() >= gasNeeded && !buildManager->alreadyBuilding(UnitTypes::Terran_Barracks)) {
					informationManager->buildRequest(UnitTypes::Terran_Barracks);
				}
			}
		}

		// Control PRODUCTION on Starports
		if (!_starports.empty()) {
			bool productionLimited = onProduction(_starports);
			if (productionLimited) { 
				int moneyNeeded = UnitTypes::Terran_Supply_Depot.mineralPrice() + UnitTypes::Terran_Starport.mineralPrice() + informationManager->_trainOrder[UnitTypes::Terran_Starport].mineralPrice();
				int gasNeeded = UnitTypes::Terran_Starport.gasPrice() + informationManager->_trainOrder[UnitTypes::Terran_Starport].gasPrice();
				if ( informationManager->minerals() >= moneyNeeded && informationManager->gas() >= gasNeeded && !buildManager->alreadyBuilding(UnitTypes::Terran_Starport)) {
					informationManager->buildRequest(UnitTypes::Terran_Starport);
				}
			}
		}

		// Control PRODUCTION on Factories
		if (!_factories.empty()) {
			bool productionLimited = onProduction(_factories);
			if (productionLimited) {
				bool needAddon = informationManager->_trainOrder[UnitTypes::Terran_Factory] == UnitTypes::Terran_Siege_Tank_Tank_Mode;
				if (!needAddon || (needAddon && Broodwar->self()->incompleteUnitCount(UnitTypes::Terran_Machine_Shop) == 0)) { // if there are addons under construction avoid build another factory (outcome will increase)
					int moneyNeeded = UnitTypes::Terran_Supply_Depot.mineralPrice() + UnitTypes::Terran_Factory.mineralPrice() + informationManager->_trainOrder[UnitTypes::Terran_Factory].mineralPrice();
					int gasNeeded = UnitTypes::Terran_Factory.gasPrice() + informationManager->_trainOrder[UnitTypes::Terran_Factory].gasPrice();
					if (needAddon) {
						moneyNeeded += UnitTypes::Terran_Machine_Shop.mineralPrice();
						gasNeeded += UnitTypes::Terran_Machine_Shop.gasPrice();
					}
					if ( informationManager->minerals() >= moneyNeeded && informationManager->gas() >= gasNeeded && !buildManager->alreadyBuilding(UnitTypes::Terran_Factory)) {
						informationManager->buildRequest(UnitTypes::Terran_Factory);
					}
				}
			}
		}
#ifndef TOURNAMENT
	} else {
		printBlockedProduction(_barracks,"Blocked by research",informationManager->_wastedProductionFramesByResearch);
		printBlockedProduction(_starports,"Blocked by research",informationManager->_wastedProductionFramesByResearch);
		printBlockedProduction(_factories,"Blocked by research",informationManager->_wastedProductionFramesByResearch);
#endif
	}
}

void ProductionManager::printBlockedProduction(BWAPI::Unitset buildings, std::string message, int &counter)
{
	if (buildings.empty()) return;
	BWAPI::Unitset::iterator building = buildings.begin();
	if (informationManager->_trainOrder[(*building)->getType()] != UnitTypes::None) {
		for (BWAPI::Unitset::iterator unit = buildings.begin(); unit != buildings.end(); ++unit) {
			printBlockedProduction(*unit, message, counter);
		}
	}
}

void ProductionManager::printBlockedProduction(Unit unit, std::string message, int &counter)
{
	if (!unit->isTraining() && unit->isCompleted() && !unit->isConstructing()) {
		Broodwar->drawBoxMap(unit->getLeft(), unit->getTop(), unit->getRight(), unit->getBottom(), Colors::Red);
		Broodwar->drawTextMap(unit->getLeft()+2, unit->getBottom()-15, message.c_str());
		counter++;
	}
}

bool ProductionManager::canTrain(UnitType type) 
{
	return ( informationManager->haveResources(type)  &&
			 Broodwar->self()->supplyTotal()-Broodwar->self()->supplyUsed() >=  type.supplyRequired()/2 );
}

bool ProductionManager::onProduction(BWAPI::Unitset buildings) {
	bool productionLimited = true;
	for (BWAPI::Unitset::iterator unit = buildings.begin(); unit != buildings.end(); ++unit) {
		if ((*unit)->isTraining()) { Broodwar->drawTextMap((*unit)->getLeft()+2, (*unit)->getBottom()-15,"Training"); continue;}
		if (!(*unit)->isCompleted()) { Broodwar->drawTextMap((*unit)->getLeft()+2, (*unit)->getBottom()-15,"Under construction"); continue;}
		if ((*unit)->isConstructing()) { Broodwar->drawTextMap((*unit)->getLeft()+2, (*unit)->getBottom()-15,"Wait Addon"); continue;}
		productionLimited = false;
		UnitType toTrain = informationManager->_trainOrder[(*unit)->getType()];
		// Addon request?
		UnitType addonType;
		if ( !informationManager->_addonOrder[(*unit)->getType()].empty() ) {
			addonType = informationManager->_addonOrder[(*unit)->getType()].at(0);
		} else {
			addonType = needAddon(toTrain);
		}
		if ( (*unit)->getAddon()==NULL && addonType!=UnitTypes::None ) {
			if ( informationManager->haveResources(addonType) ) {
				bool ok = (*unit)->buildAddon(addonType);
				if (!ok) {
					if (PRINT_PRODUCTION) Broodwar->printf("[%s ERROR] %s", (*unit)->getType().getName().c_str(), Broodwar->getLastError().toString().c_str() );
					informationManager->frameSpend(addonType);
				}
			}
			continue;
		}
		// Unit request?
		if (toTrain == UnitTypes::None) {
			Broodwar->drawTextMap((*unit)->getLeft()+2, (*unit)->getBottom()-15,"Nothing to produce");
			break;
		} 
		//if (canTrain(toTrain)) {
		if (informationManager->haveResources(toTrain)) {
			if (Broodwar->self()->supplyTotal()-Broodwar->self()->supplyUsed() >=  toTrain.supplyRequired()/2 ) {
				bool ok = (*unit)->train(toTrain);
				if (!ok) {
					if (PRINT_PRODUCTION) Broodwar->printf("[%s ERROR] %s", (*unit)->getType().getName().c_str(), Broodwar->getLastError().toString().c_str() );
					informationManager->frameSpend(toTrain);
				}
			} else {
				#ifndef TOURNAMENT
					printBlockedProduction(*unit, "Need Supplies", informationManager->_wastedProductionFramesBySupply);
					continue;
				#endif
				break;
			}
		} else {
			#ifndef TOURNAMENT
				printBlockedProduction(*unit, "Need Money", informationManager->_wastedProductionFramesByMoney);
				continue;
			#endif
			break;
		}
	}

	return productionLimited;
}

UnitType ProductionManager::needAddon(UnitType toTrain) {
	if (toTrain == UnitTypes::Terran_Siege_Tank_Tank_Mode) {
		return UnitTypes::Terran_Machine_Shop;
	} else if (toTrain == UnitTypes::Terran_Dropship) {
		return UnitTypes::Terran_Machine_Shop;
	} else if (toTrain == UnitTypes::Terran_Science_Vessel || toTrain == UnitTypes::Terran_Battlecruiser) {
		return UnitTypes::Terran_Control_Tower;
	}
	return UnitTypes::None;
}

UpgradeType ProductionManager::getNextArmoryUpgrade()
{
	int weaponLevel;
	int plateLevel;
	if (informationManager->_autoVehicleUpgrade) {
		weaponLevel = Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Vehicle_Weapons);
		if (Broodwar->self()->isUpgrading(UpgradeTypes::Terran_Vehicle_Weapons)) weaponLevel++;

		plateLevel = Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Vehicle_Plating);
		if (Broodwar->self()->isUpgrading(UpgradeTypes::Terran_Vehicle_Plating)) plateLevel++;

		if (plateLevel < 3 && (plateLevel == 0 || (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Science_Facility)>0)  ) ) {
			if (plateLevel < weaponLevel) return UpgradeTypes::Terran_Vehicle_Plating;
			else return UpgradeTypes::Terran_Vehicle_Weapons;
		}

	}

	if (informationManager->_autoShipUpgrade) {
		weaponLevel = Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Ship_Weapons);
		if (Broodwar->self()->isUpgrading(UpgradeTypes::Terran_Ship_Weapons)) weaponLevel++;

		plateLevel = Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Ship_Plating);
		if (Broodwar->self()->isUpgrading(UpgradeTypes::Terran_Ship_Plating)) plateLevel++;

		if (plateLevel < 3 && (plateLevel == 0 || (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Science_Facility)>0)  ) ) {
			if (plateLevel < weaponLevel) return UpgradeTypes::Terran_Ship_Plating;
			else return UpgradeTypes::Terran_Ship_Weapons;
		}

	}

	return UpgradeTypes::None;
}

UpgradeType ProductionManager::getNextInfanteryUpgrade()
{
	int weaponLevel;
	int armorLevel;
	if (informationManager->_autoInfanteryUpgrade) {
		weaponLevel = Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Infantry_Weapons);
		if (Broodwar->self()->isUpgrading(UpgradeTypes::Terran_Infantry_Weapons)) weaponLevel++;

		armorLevel = Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Infantry_Armor);
		if (Broodwar->self()->isUpgrading(UpgradeTypes::Terran_Infantry_Armor)) armorLevel++;

		if (armorLevel < 3 && (armorLevel == 0 || (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Science_Facility)>0)  ) ) {
			if (armorLevel < weaponLevel) return UpgradeTypes::Terran_Infantry_Armor;
			else return UpgradeTypes::Terran_Infantry_Weapons;
		}

	}

	return UpgradeTypes::None;
}
