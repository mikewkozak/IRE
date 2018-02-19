#include "BuildManager.h"

using namespace BWAPI;

#define BLOCKED 0
#define BUILDABLE 1
#define TURRETS_PER_BASE 2
#define WALL_KITE_AREA 2

// The building queue has 4 lists:
//	- _buildRequest: someone requested a building, it will be added at the END of _buildOrder
//  - _criticalBuildRequest: critical building requested, it will be added at the BEGINNING of _buildOrder
//  - _buildOrder: build order accepted and waiting for resources or required buildings (the second thing shouldn't happen)
//  - _toConstruct: buildings under construction (a worker should be taking care of)

BuildManager::BuildManager()
	:_logger(log4cxx::Logger::getLogger("BuildManager")),
	_workerBuildingRefinery(0),
	gateBuilding(nullptr)
{
	//Init build map
	mapW = Broodwar->mapWidth();
	mapH = Broodwar->mapHeight();

	buildMap = new int*[mapW];
	for(int x = 0 ; x < mapW ; ++x) {
		buildMap[x] = new int[mapH];
		for (int y = 0; y < mapH; ++y) {
			if (Broodwar->isBuildable(x, y)) buildMap[x][y] = BUILDABLE;
			else buildMap[x][y] = BLOCKED;
		}
	}

	//Block minerals locations
	for (const auto& m : Broodwar->getMinerals()) {
		MyRectangle mineral = getBuildRectangle(m->getTilePosition(), m->getType());
		setBuildMapRectangle(mineral, BLOCKED, 2);
	}

	//Block gas locations
	for (const auto& g : Broodwar->getGeysers()) {
		MyRectangle gas = getBuildRectangle(g->getTilePosition(), g->getType());
		setBuildMapRectangle(gas, BLOCKED, 2);
	}
};

BuildManager::~BuildManager()
{
	for (int i = 0; i < mapW; ++i) {
		delete[] buildMap[i];
	}
	delete[] buildMap;
}

void BuildManager::reserveBaseLocations()
{
	// Our bases
	for (const auto& i : informationManager->_ourBases) {
		MyRectangle base = getBuildRectangle(i.first->getTilePosition(), UnitTypes::Terran_Command_Center);
		setBuildMapRectangle(base, BLOCKED);
	}

	// Empty bases
	for (const auto& baseLocation : informationManager->_emptyBases) {
		MyRectangle base = getBuildRectangle(baseLocation->getTilePosition(), UnitTypes::Terran_Command_Center);
		setBuildMapRectangle(base, BLOCKED);
	}

	// Ignore bases
	for (const auto& tilePosition : informationManager->_ignoreBases) {
		MyRectangle base = getBuildRectangle(tilePosition, UnitTypes::Terran_Command_Center);
		setBuildMapRectangle(base, BLOCKED);
	}

	// Enemy bases
	for (const auto& baseLocation : informationManager->_enemyBases) {
		MyRectangle base = getBuildRectangle(baseLocation->getTilePosition(), UnitTypes::Terran_Command_Center);
		setBuildMapRectangle(base, BLOCKED);
	}
}

MyRectangle BuildManager::getBuildRectangle(TilePosition position, UnitType type)
{
	MyRectangle area;
	area.x1 = position.x;
	area.y1 = position.y;
	area.x2 = position.x + type.tileWidth() - 1;
	area.y2 = position.y + type.tileHeight() - 1;

	//make sure we leave space for add-ons
	if (type==BWAPI::UnitTypes::Terran_Command_Center ||
		type==BWAPI::UnitTypes::Terran_Factory || 
		type==BWAPI::UnitTypes::Terran_Starport ||
		type==BWAPI::UnitTypes::Terran_Science_Facility)
	{
		area.x2 += 2;
	}

	//leave more space for production buildings
	if (type==BWAPI::UnitTypes::Terran_Command_Center ||
		type==BWAPI::UnitTypes::Terran_Factory || 
		type==BWAPI::UnitTypes::Terran_Barracks)
	{
		area.x1--;
		area.y2++;
	}

	return area;
}

void BuildManager::setBuildMapRectangle(MyRectangle c, int label, int expand) {
	for (int x = c.x1-expand; x <= c.x2+expand; x++) {
		for (int y = c.y1-expand; y <= c.y2+expand; y++) {
			if (x >= 0 && x < mapW && y >= 0 && y < mapH) {
				buildMap[x][y] = label;
			}
		}
	}
}

void BuildManager::onFrame()
{
	// Debug info
	if (PRINT_BUILD_MAP) drawBuildMap();
	if (PRINT_BUILD_ORDER) drawBuildOrder();

	//Debug info
// 	Broodwar->drawTextScreen(290,39,"Workers building: %d", workerManager->_workerBuildOrder.size() );
// 	for(WorkerToBuildOrderMap::const_iterator workerMap=workerManager->_workerBuildOrder.begin();workerMap!=workerManager->_workerBuildOrder.end();++workerMap) {
// 		Position q = workerMap->first->getPosition();
// 		Broodwar->drawCircle(CoordinateType::Map,q.x,q.y,30,Colors::Yellow,false);
// 	}
	// Check if we have buildings under construction list and any building worker
	// TODO this can be improved
	if (!_toConstruct.empty() && workerManager->getNumWorkersBuilding() == 0) {
		for (const auto& unitType : _toConstruct) {
			LOG4CXX_WARN(_logger, "No worker set to build " << unitType.c_str() << " frame: " << Broodwar->getFrameCount());
			_buildOrder.insert(_buildOrder.begin(), unitType);
			informationManager->removeReservedMinerals(unitType.mineralPrice());
			informationManager->removeReservedGas(unitType.gasPrice());
		}
		_toConstruct.clear();
	}

	// Process build requests
	while ( !informationManager->_buildRequest.empty() ) {
		UnitType buildType = informationManager->_buildRequest.at(0);
		_buildOrder.push_back(buildType);
		informationManager->_buildRequest.erase(informationManager->_buildRequest.begin()); //pop
	}

	// Process Supply Depots
	if (needSupply()) {
		_buildOrder.insert(_buildOrder.begin(), UnitTypes::Terran_Supply_Depot);
	}

	// Process critical build requests
	while ( !informationManager->_criticalBuildRequest.empty() ) {
		UnitType buildType = informationManager->_criticalBuildRequest.at(0);
		if (!alreadyBuilding(buildType)) {
			_buildOrder.insert(_buildOrder.begin(), buildType);
		} else { // look if there is on buildOrder list and put it first
			UnitTypeVector::iterator found = std::find(_buildOrder.begin(), _buildOrder.end(), buildType);
			if(found != _buildOrder.end()) {
				_buildOrder.erase(found);
				_buildOrder.insert(_buildOrder.begin(), buildType);
			}
		}

		informationManager->_criticalBuildRequest.erase(informationManager->_criticalBuildRequest.begin()); //pop
	}

	// Try to execute first in order list
	if (_buildOrder.size() > 0 && workerManager->getWorkersSize() >= informationManager->_workersNeededToBuild) {
		UnitType buildType = _buildOrder.at(0);
		if (buildType.isAddon()) {
			informationManager->addonRequest(buildType);
			_buildOrder.erase(_buildOrder.begin());
		} else if ( canBuild(buildType) ) {
			if ( !executeBuildOrder(buildType) && _buildOrder.size() > 1) {
				//to prevent build order block, check if we can execute second order
				UnitType buildType2 = _buildOrder.at(1);
				int moneyNeeded = buildType.mineralPrice() + buildType2.mineralPrice();
				int gasNeeded = buildType.gasPrice() + buildType2.gasPrice();
				if ( informationManager->minerals() >= moneyNeeded && informationManager->gas() >= gasNeeded) {
					executeBuildOrder(buildType2);
				}
			}
		}
	}

	// Check if we need build turret defenses
	if (informationManager->_turretDefenses) {
		int numTurrets = Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Missile_Turret); //turrets already build
		numTurrets += std::count( _buildOrder.begin(), _buildOrder.end(), UnitTypes::Terran_Missile_Turret ); //turrets on build order list
		int numBases = informationManager->_ourBases.size();
		int ratio = (numBases*TURRETS_PER_BASE) - numTurrets;
		if (ratio > 0) {
			informationManager->criticalBuildRequest(UnitTypes::Terran_Missile_Turret);

			// Check if we are already building the Engineering Bay
			if ( std::find(_toConstruct.begin(), _toConstruct.end(), UnitTypes::Terran_Engineering_Bay) == _toConstruct.end() ) { 
				// remove any previous Engineering Bay on build order
				UnitTypeVector::iterator found = std::find(_buildOrder.begin(), _buildOrder.end(), UnitTypes::Terran_Engineering_Bay);
				if (found != _buildOrder.end()) _buildOrder.erase(found);
				informationManager->criticalBuildRequest(UnitTypes::Terran_Engineering_Bay, true);
			}
		}
	}

	// Check if we need to open/close the gate of the wall
	if (wallGenerator->WallCalculated && Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Barracks) == 1) {
		if (!gateBuilding) gateBuilding = Broodwar->getClosestUnit(BWAPI::Position(wallGenerator->BarracksWall));

		// count units that want to cross the wall
		bool unitNeedToCross = false;
		Unitset unitsNearWall = Broodwar->getUnitsInRadius(BWAPI::Position(wallGenerator->BarracksWall), 512, Filter::IsOwned && Filter::CanMove);
// 		Broodwar->printf("Units near wall: %d", unitsNearWall.size());
		for (auto unit : unitsNearWall) {
// 				Broodwar->printf("%s Region: %d Target %d", unit->getType().c_str(), BWTA::getRegion(unit->getPosition()), BWTA::getRegion(unit->getTargetPosition()));
			if (BWTA::getRegion(unit->getPosition()) != BWTA::getRegion(unit->getTargetPosition())) {
				unitNeedToCross = true;
				break;
			}
		}

		// count enemies near wall
		Unitset enemiesNear = Broodwar->getUnitsInRadius(BWAPI::Position(wallGenerator->BarracksWall), 512, Filter::IsEnemy);

		if (gateOpened && !enemiesNear.empty()) {
			closeGate();
		}
		if (!gateOpened && enemiesNear.empty() && unitNeedToCross) {
			openGate();
		}
		// try to keep gate closed at the beginning...
		if (Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Refinery) <= 1 && gateOpened && !unitNeedToCross) {
			closeGate();
		}
		// ...and open later
// 		Broodwar->printf("Refinery: %d lifted: %d enemisEmpty: %d", Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Refinery), gateBuilding->isLifted(), enemiesNear.empty());
		// TODO there is an error in this condition between Release and Tournament (on Tournament visibleUnitCount and completedUnitCount return wrong number!!!!)
		//Unitset commandCenterNearWall = Broodwar->getUnitsInRadius(BWAPI::Position(wallGenerator->BarracksWall), 1125, Filter::IsOwned && Filter::GetType == UnitTypes::Terran_Command_Center);
// 		LOG("CC: " << productionManager->_commandCenters.size() << " lifted: " << gateBuilding->isLifted() << " enemisEmpty : " << enemiesNear.empty());
		if (productionManager->_commandCenters.size() >= 2 && !gateBuilding->isLifted() && enemiesNear.empty()) {
			openGate();
		}
	}
}

bool BuildManager::executeBuildOrder(UnitType buildType)
{
	// Select a seed position
	// ========================================================================
	TilePosition seedPosition = Broodwar->self()->getStartLocation(); // Default seed position
// 	LOG("Default seedPosition " << seedPosition);

	// check for proxy seed
	if (buildType == UnitTypes::Terran_Barracks && informationManager->_useProxyPosition) {
		if (informationManager->_proxyPosition != TilePositions::None) {
			seedPosition = informationManager->_proxyPosition;
		} else { // wee need to check if we can calculate the proxy seed
			findProxyPosition();
			return false;
		}
	}
	// check for missile turret seed
	if (buildType == UnitTypes::Terran_Missile_Turret) {
		seedPosition = getSeedForTurret();
	}
	// check for Bunker seed
	if (buildType == UnitTypes::Terran_Bunker) {
		seedPosition = TilePosition(informationManager->_bunkerSeedPosition);
	}

	// Find a location near a seed position
	// ========================================================================
	TilePosition locationToBuild = TilePositions::None;
	if (buildType == UnitTypes::Terran_Command_Center) {
		locationToBuild = informationManager->getExpandPosition();
	} else if (buildType == UnitTypes::Terran_Refinery) {
		locationToBuild = getGeyserTilePosition();
	} else if (buildType.isAddon()) { // get building without add-on
		informationManager->addonRequest(buildType);
		LOG4CXX_ERROR(_logger, "Removing addon from buildOrder " << buildType);
		for (const auto& bt : _buildOrder) LOG4CXX_ERROR(_logger, " - " << buildType);
		_buildOrder.erase(_buildOrder.begin()); // TODO check if this is safe!!!!
		LOG4CXX_ERROR(_logger, "First remove");
		for (const auto& bt : _buildOrder) LOG4CXX_ERROR(_logger, " - " << buildType);

	// if wall calculated we know the positions
	} else if (wallGenerator->WallCalculated && buildType == UnitTypes::Terran_Supply_Depot &&
		Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Supply_Depot) == 0) {

		// TODO why order sent multiple times?
// 		Broodwar->printf("Wall Supply 1");
		locationToBuild = wallGenerator->SupplyWall1;

	} else if (wallGenerator->WallCalculated && buildType == UnitTypes::Terran_Supply_Depot &&
		Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Supply_Depot) == 1 &&
		wallGenerator->SupplyWall2.x != 0 && wallGenerator->SupplyWall2.y != 0) {

// 		Broodwar->printf("Wall Supply 2");
		locationToBuild = wallGenerator->SupplyWall2;

	} else if (wallGenerator->WallCalculated && buildType == UnitTypes::Terran_Barracks &&
		Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Barracks) == 0) {

// 		Broodwar->printf("Wall Barracks");
		locationToBuild = wallGenerator->BarracksWall;
		gateOpened = false;

	} else {
		locationToBuild = getBuildLocationNear(seedPosition,buildType);
	}
// 	LOG("locationToBuild " << locationToBuild);

	// Send build order to worker (if we have a location)
	// ========================================================================
	if (locationToBuild != TilePositions::None) {
		workerManager->buildRequest(locationToBuild, buildType);
		// remove from _buildOrder vector
		auto it = std::find(_buildOrder.begin(), _buildOrder.end(), buildType);
		if (it != _buildOrder.end()) _buildOrder.erase(it);
		if (buildType.isAddon()) {
			LOG4CXX_ERROR(_logger, "Second remove");
			for (const auto& bt : _buildOrder) LOG4CXX_ERROR(_logger, " - " << buildType);
		}
		// reserve resources
		informationManager->reserveMinerals(buildType.mineralPrice());
		informationManager->reserveGas(buildType.gasPrice());
		// add to _toConstruct vector
		LOG4CXX_TRACE(_logger, "New Build order: " << buildType.c_str() << " frame: " << Broodwar->getFrameCount());
		_toConstruct.push_back(buildType); 
		return true;
	} else {
		return false;
	}
}

TilePosition BuildManager::getGeyserTilePosition()
{
	bool enemyRefinery = false;
	for(std::map<BWTA::BaseLocation*, BWAPI::TilePosition>::const_iterator base = informationManager->_ourBases.begin(); base != informationManager->_ourBases.end(); ++base) {
		if ( base->second == TilePositions::None ) continue;
		TilePosition geyserLocation = base->second;
		// check if refinery is already build it
		bool refineryExist = false;
		BWAPI::Unitset unitsOnGeyser = BWAPI::Broodwar->getUnitsOnTile(geyserLocation.x, geyserLocation.y);
		for (BWAPI::Unitset::iterator u = unitsOnGeyser.begin(); u != unitsOnGeyser.end(); ++u) {
			if ((*u)->getType().isRefinery()) {
				refineryExist = true;
				if ((*u)->getPlayer() != BWAPI::Broodwar->self())
					enemyRefinery = true;
			}
		}
		if (!refineryExist)
			return geyserLocation;
	}

	if (!enemyRefinery) { // we don't have any free Geyser, erase build order
		_buildOrder.erase(_buildOrder.begin());
	} else {
		// be sure that we have barracks
		informationManager->criticalBuildRequest(UnitTypes::Terran_Barracks, true);
		// reset supply depot priority
		UnitTypeVector::iterator found = std::find(_buildOrder.begin(), _buildOrder.end(), UnitTypes::Terran_Supply_Depot);
		if(found != _buildOrder.end()) _buildOrder.erase(found);
	}
	return TilePositions::None;	
}

TilePosition BuildManager::getSeedForTurret()
{
	int turrets;
	BWTA::Region* baseRegion;
	TilePosition bestPlace = TilePositions::None;
	for(std::map<BWTA::BaseLocation*, BWAPI::TilePosition>::iterator ourBase=informationManager->_ourBases.begin();ourBase!=informationManager->_ourBases.end();++ourBase) {
		turrets = 0;
		baseRegion = ourBase->first->getRegion();
		for(std::map<BWAPI::Unit, BWTA::Region*>::iterator turret=informationManager->_missileTurrets.begin();turret!=informationManager->_missileTurrets.end();++turret) {
			if (baseRegion == turret->second) turrets++;
		}
		if (turrets == 0) return ourBase->first->getTilePosition();
		if (turrets < TURRETS_PER_BASE) bestPlace = ourBase->first->getTilePosition();
	}

	if (bestPlace!=TilePositions::None) return bestPlace;
	else return Broodwar->self()->getStartLocation();
}

bool BuildManager::canBuild(UnitType type)
{
	if (!informationManager->haveResources(type)) {
// 		Broodwar << "Need resources";
		return false;
	}

	if (type.isAddon() && Broodwar->self()->completedUnitCount( type.whatBuilds().first ) <= Broodwar->self()->completedUnitCount(type) ) { // do we have the build for the addon?
// 		Broodwar << "Missing required building " << type.whatBuilds().first.c_str();
		return false;
	} else {
		const std::map<UnitType,int> requiredBuildings = type.requiredUnits();
		for(std::map<UnitType,int>::const_iterator build = requiredBuildings.begin(); build != requiredBuildings.end(); ++build) {
			if (Broodwar->self()->completedUnitCount( (*build).first ) == 0)
// 				Broodwar << "Missing required building " << (*build).first.c_str();
				return false;
		}
	}

	return true;
}

bool BuildManager::needSupply()
{
	// If command center is next in queue
// 	if (_buildOrder.size() > 0) {
// 		if (_buildOrder.at(0).isResourceDepot()) {
// 			return false;
// 		}
// 	}

	if (!informationManager->_autoBuildSuplies) return false;

	// Check if we need supplies
	int supplyTotal = Broodwar->self()->supplyTotal() / 2;
	int supplyUsed = Broodwar->self()->supplyUsed() / 2;
	int maxProduction = Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Command_Center);
	maxProduction += Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Barracks)*2;
	maxProduction += Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Factory)*2;
	maxProduction += Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Starport)*2;
	if (maxProduction < 4) maxProduction = 4;
	if (supplyTotal - supplyUsed > maxProduction) {
		return false;
	}

	if (supplyTotal >= 200) { //Reached max supply
		return false;
	}

	// Check if there is a supply already building
	if ( alreadyBuilding(UnitTypes::Terran_Supply_Depot) ) {
		if (!alreadyRequested(UnitTypes::Terran_Supply_Depot) && Broodwar->self()->incompleteUnitCount(UnitTypes::Terran_Supply_Depot)==1) {
			return true;
		} else {
			return false;
		}
	}

	return true;
}

bool BuildManager::alreadyBuilding(UnitType type)
{
	// Check if there is in the build order list
	if ( alreadyRequested(type) ) {
		return true;
	}

	if (Broodwar->self()->incompleteUnitCount(type) > 0)// TODO	if (supplyTotal == supplyUsed) return true;
		return true;

	return false;
}

bool BuildManager::alreadyRequested(UnitType type)
{
	// Check if there is in the build order list
	if ( std::find(_buildOrder.begin(), _buildOrder.end(), type)!= _buildOrder.end() ) {
		return true;
	}

	// Check if we are already building it
	if ( std::find(_toConstruct.begin(), _toConstruct.end(), type)!= _toConstruct.end() ) {
		return true;
	}

	return false;
}

TilePosition BuildManager::getBuildLocationNear(TilePosition position, UnitType type, BWTA::Region* inRegion)
{
	//returns a valid build location near the specified tile position.
	//searches outward in a spiral.
	int x      = position.x;
	int y      = position.y;
	int length = 1;
	int j      = 0;
	bool first = true;
	int dx     = 0;
	int dy     = 1;	
	while (length < Broodwar->mapWidth()) //We'll ride the spiral to the end
	{
		//if we can build here, return this tile position
		if (x >= 0 && x < Broodwar->mapWidth() && y >= 0 && y < Broodwar->mapHeight())
			if (inRegion==0 || inRegion==BWTA::getRegion(TilePosition(x, y)))
				if (canBuildHere(TilePosition(x, y), type))
					return TilePosition(x, y);

		//otherwise, move to another position
		x = x + dx;
		y = y + dy;
		//count how many steps we take in this direction
		j++;
		if (j == length) { //if we've reached the end, its time to turn
			j = 0;	//reset step counter

			//Spiral out. Keep going.
			if (!first)
				length++; //increment step counter if needed

			
			first =! first; //first=true for every other turn so we spiral out at the right rate

			//turn counter clockwise 90 degrees:
			if (dx == 0) {
				dx = dy;
				dy = 0;
			} else {
				dy = -dx;
				dx = 0;
			}
		}
		//Spiral out. Keep going.
	}
	return TilePositions::None;
}

bool BuildManager::canBuildHere(TilePosition buildPosition, UnitType type) {
	if (!Broodwar->canBuildHere(buildPosition, type))
		return false;

	if (Broodwar->mapHash() == "4e24f217d2fe4dbfa6799bc57f74d8dc939d425b") { // Map Destination
		BWTA::Region* buildRegion = BWTA::getRegion(buildPosition);
		if (BWTA::getRegion(TilePosition(80,5)) == buildRegion ||
			BWTA::getRegion(TilePosition(20,125)) == buildRegion) {
			return false;
		}
	}

	MyRectangle area = getBuildRectangle(buildPosition, type);

	//Check buildMap
	for (int x = area.x1; x <= area.x2; x++) {
		for (int y = area.y1; y <= area.y2; y++) {
			if (x >= 0 && x < mapW && y >= 0 && y < mapH) {
				if (buildMap[x][y] != BUILDABLE) {
					return false; //Cant build here.
				}
			} else {
				return false; //Out of map
			}
		}
	}

	//Check if we have free space surrounding us
	MyRectangle buildArea;
	buildArea.x1 = buildPosition.x - 1;
	buildArea.y1 = buildPosition.y - 1;
	buildArea.x2 = buildPosition.x + type.tileWidth();
	buildArea.y2 = buildPosition.y + type.tileHeight();
// #ifndef TOURNAMENT
// 	informationManager->printDebugBox(Position(buildArea.x1 * TILE_SIZE, buildArea.y1 * TILE_SIZE),
// 		Position(buildArea.x2 * TILE_SIZE, buildArea.y2 * TILE_SIZE), "[BuildManager] Testing if we can build here");
// #endif

	// Check top and bottom
	bool topFree = true;
	for (int y = buildArea.y1; y <= buildArea.y2; y++) {
		if (buildArea.x1 >= 0 && buildArea.x1 < mapW && y >= 0 && y < mapH) {
			//if (!Broodwar->isBuildable(buildArea.x1, y)) {
			if (buildMap[buildArea.x1][y] != BUILDABLE) {
				topFree = false; //Cant build here.
				break;
			}
		} else {
			topFree = false; //Out of map
			break;
		}
	}
	if (!topFree) {
		for (int y = buildArea.y1; y <= buildArea.y2; y++) {
			if (buildArea.x2 >= 0 && buildArea.x2 < mapW && y >= 0 && y < mapH) {
				//if (!Broodwar->isBuildable(buildArea.x2, y)) {
				if (buildMap[buildArea.x2][y] != BUILDABLE) {
					return false; //Cant build here.
				}
			} else {
				return false; //Out of map
			}
		}
	}
	// Check left and right
	bool leftFree = true;
	for (int x = buildArea.x1; x <= buildArea.x2; x++) {
		if (buildArea.y1 >= 0 && buildArea.y1 < mapH && x >= 0 && x < mapW) {
			//if (!Broodwar->isBuildable(x, buildArea.y1)) {
			if (buildMap[x][buildArea.y1] != BUILDABLE) {
				leftFree = false; //Cant build here.
				break;
			}
		} else {
			leftFree = false; //Out of map
			break;
		}
	}
	if (!leftFree) {
		for (int x = buildArea.x1; x <= buildArea.x2; x++) {
			if (buildArea.y2 >= 0 && buildArea.y2 < mapH && x >= 0 && x < mapW) {
				//if (!Broodwar->isBuildable(x, buildArea.y2)) {
				if (buildMap[x][buildArea.y2] != BUILDABLE) {
					return false; //Cant build here.
				}
			} else {
				return false; //Out of map
			}
		}
	}

	//check turret proximity
	if (type == UnitTypes::Terran_Missile_Turret) {
		return anyMissileTurretsNear(buildPosition);
	}

	return true;
}

bool BuildManager::anyMissileTurretsNear(TilePosition buildPosition)
{
	BWAPI::Unitset units = Broodwar->getUnitsInRadius(Position((buildPosition.x*TILE_SIZE) + TILE_SIZE, (buildPosition.y*TILE_SIZE) + TILE_SIZE), 3 * TILE_SIZE);
	informationManager->_center = Position((buildPosition.x*TILE_SIZE)+TILE_SIZE, (buildPosition.y*TILE_SIZE)+TILE_SIZE);
	informationManager->_radius = 3*TILE_SIZE;
	for (BWAPI::Unitset::iterator it = units.begin(); it != units.end(); ++it) {
		if ( (*it)->getType() == UnitTypes::Terran_Missile_Turret ) {
			return false;
		}
	}
	return true;
}

void BuildManager::constructionPlaced(Unit build)
{
	LOG4CXX_TRACE(_logger, "New Build placed: " << build->getType().c_str() << " frame: " << Broodwar->getFrameCount());
	//Block buildMap
	MyRectangle area = getBuildRectangle(build->getTilePosition(), build->getType());
	setBuildMapRectangle(area, BLOCKED);

	//Remove from toConstruct list
	for (std::vector<UnitType>::iterator it = _toConstruct.begin(); it != _toConstruct.end(); ++it) {
		if (it->getID() == build->getType().getID()) {
			_toConstruct.erase(it);
			// release resources
// 			if (!build->getType().isAddon()) {
				informationManager->removeReservedMinerals(build->getType().mineralPrice());
				informationManager->removeReservedGas(build->getType().gasPrice());
// 			}
			return;
		}
	}
	if (Broodwar->getFrameCount() > 0) { // to avoid initial buildings
		LOG4CXX_ERROR(_logger, "Build placed not in _toConstruct " << build->getType().c_str() << " frame: " << Broodwar->getFrameCount());
	}
}

void BuildManager::onBuildingDestroy(BWAPI::Unit build)
{
	//Free buildMap
	if (build->getType() == UnitTypes::Terran_Refinery || //don't free space form refinery, command center or add-ons
		build->getType() == UnitTypes::Terran_Comsat_Station ||
		build->getType() == UnitTypes::Terran_Nuclear_Silo ||
		build->getType() == UnitTypes::Terran_Machine_Shop ||
		build->getType() == UnitTypes::Terran_Control_Tower ||
		build->getType() == UnitTypes::Terran_Physics_Lab ||
		build->getType() == UnitTypes::Terran_Covert_Ops ||
		build->getType() == UnitTypes::Terran_Command_Center) 
		return;
	MyRectangle rectangle = getBuildRectangle(build->getTilePosition(), build->getType());
	setBuildMapRectangle(rectangle, BUILDABLE);
}

void BuildManager::drawBuildMap()
{
	for(int x=0; x < mapW; ++x) {
		for(int y=0; y < mapH; ++y) {
			if ( buildMap[x][y] == BUILDABLE)
				Broodwar->drawCircleMap(x*TILE_SIZE+16,y*TILE_SIZE+16,3,Colors::Green,true);
			else
				Broodwar->drawCircleMap(x*TILE_SIZE+16,y*TILE_SIZE+16,3,Colors::Red,true);
		}
	}
}

void BuildManager::drawBuildOrder()
{
	int line = 1;
	// Build order list
	Broodwar->drawTextScreen(437,27,"%cBuild Order List", 0x07);
	for (std::vector<UnitType>::iterator it = _toConstruct.begin(); it != _toConstruct.end(); ++it) {
		Broodwar->drawTextScreen(437,27+(10*line),"%c%s", 0x03, (*it).getName().c_str() );
		++line;
	}
	for (std::vector<UnitType>::iterator it = _buildOrder.begin(); it != _buildOrder.end(); ++it) {
		Broodwar->drawTextScreen(437,27+(10*line),"%c%s", 0x11, (*it).getName().c_str() );
		++line;
	}
	for (std::vector<UnitType>::iterator it = informationManager->_buildRequest.begin(); it != informationManager->_buildRequest.end(); ++it) {
		Broodwar->drawTextScreen(437,27+(10*line),"%s", (*it).getName().c_str() );
		++line;
	}

	// Research order list
	if (!informationManager->_researchOrder.empty()) {
		for (BuildingToTechMap::iterator it = informationManager->_researchOrder.begin(); it != informationManager->_researchOrder.end(); ++it) {
			for (std::vector<TechType>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
				Broodwar->drawTextScreen(437,27+(10*line),"%c[Research] %s", 0x11, (*it2).getName().c_str() );
				++line;
			}
		}
	}

	// Upgrade order list
	if (!informationManager->_upgradeOrder.empty()) {
		for (BuildingToUpgradeMap::iterator it = informationManager->_upgradeOrder.begin(); it != informationManager->_upgradeOrder.end(); ++it) {
			for (std::vector<UpgradeType>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
				Broodwar->drawTextScreen(437,27+(10*line),"%c[Upgrade] %s", 0x11, (*it2).getName().c_str() );
				++line;
			}
		}
	}
}

bool BuildManager::wallNear(TilePosition destination)
{
	//Check if we have free space surrounding us
	MyRectangle buildArea;
	buildArea.x1 = destination.x - WALL_KITE_AREA;
	buildArea.y1 = destination.y - WALL_KITE_AREA;
	buildArea.x2 = destination.x + WALL_KITE_AREA;
	buildArea.y2 = destination.y + WALL_KITE_AREA;
// #ifndef TOURNAMENT
// 	informationManager->printDebugBox(Position(buildArea.x1, buildArea.y1), Position(buildArea.x2, buildArea.y2),
// 		"[Build Manager] Look if we have a wall near here before building something");
// #endif

	// Check top and bottom
	bool topFree = true;
	for (int y = buildArea.y1; y <= buildArea.y2; y++) {
		if (buildArea.x1 >= 0 && buildArea.x1 < mapW && y >= 0 && y < mapH) {
			//if (!Broodwar->isBuildable(buildArea.x1, y)) {
			if (buildMap[buildArea.x1][y] != BUILDABLE) {
				topFree = false; //Cant build here.
				break;
			}
		} else {
			topFree = false; //Out of map
			break;
		}
	}
	if (!topFree) {
		for (int y = buildArea.y1; y <= buildArea.y2; y++) {
			if (buildArea.x2 >= 0 && buildArea.x2 < mapW && y >= 0 && y < mapH) {
				//if (!Broodwar->isBuildable(buildArea.x2, y)) {
				if (buildMap[buildArea.x2][y] != BUILDABLE) {
					return true; //Cant build here.
				}
			} else {
				return true; //Out of map
			}
		}
	}
	// Check left and right
	bool leftFree = true;
	for (int x = buildArea.x1; x <= buildArea.x2; x++) {
		if (buildArea.y1 >= 0 && buildArea.y1 < mapH && x >= 0 && x < mapW) {
			//if (!Broodwar->isBuildable(x, buildArea.y1)) {
			if (buildMap[x][buildArea.y1] != BUILDABLE) {
				leftFree = false; //Cant build here.
				break;
			}
		} else {
			leftFree = false; //Out of map
			break;
		}
	}
	if (!leftFree) {
		for (int x = buildArea.x1; x <= buildArea.x2; x++) {
			if (buildArea.y2 >= 0 && buildArea.y2 < mapH && x >= 0 && x < mapW) {
				//if (!Broodwar->isBuildable(x, buildArea.y2)) {
				if (buildMap[x][buildArea.y2] != BUILDABLE) {
					return true; //Cant build here.
				}
			} else {
				return true; //Out of map
			}
		}
	}

	return false;
}

void BuildManager::findProxyPosition()
{
	// Find best proxy position
	// ========================================================================
	if (informationManager->_scoutedAnEnemyBase) {
		BWTA::Region* enemyStartRegion = BWTA::getRegion(TilePosition(informationManager->_enemyStartPosition));
		std::set<BWTA::Chokepoint*> enemyStartChokepoint = enemyStartRegion->getChokepoints();
		for (std::set<BWTA::Chokepoint*>::iterator c = enemyStartChokepoint.begin(); c != enemyStartChokepoint.end(); ++c) {
			std::pair<BWTA::Region*, BWTA::Region*> sides = (*c)->getRegions();
			if (sides.first == enemyStartRegion) {
				informationManager->_proxyPosition = TilePosition(sides.second->getCenter());
			} else {
				informationManager->_proxyPosition = TilePosition(sides.first->getCenter());
			}
			if (buildManager->getBuildLocationNear(informationManager->_proxyPosition, UnitTypes::Terran_Barracks) != TilePositions::None)
				break;
		}
#ifndef TOURNAMENT
// 		informationManager->printDebugBox(Position(informationManager->_proxyPosition), "Proxy position calculated");
#endif

		// Find best bunker seed position (_bunkerSeedPosition)
		// ========================================================================
		// if rush use enemy's home region as a seed
		if (informationManager->_bbs) findBunkerSeedPosition(BWTA::getRegion(informationManager->_enemyStartPosition));
	}
}

void BuildManager::findBunkerSeedPosition(BWTA::Region* seedRegion)
{
	// get choke point closest to the center of the map
	std::set<BWTA::Chokepoint*> chokes = seedRegion->getChokepoints();
	const TilePosition centerMap(Broodwar->mapWidth() / 2, Broodwar->mapHeight() / 2);
	double minDistance = std::numeric_limits<double>::max();
	double chokepointDistanceToCenterMap;
	BWTA::Chokepoint* bestChokepoint = nullptr;

	for (const auto & chokepoint : chokes) {
		chokepointDistanceToCenterMap = BWTA::getGroundDistance(centerMap, TilePosition(chokepoint->getCenter()));
		if (chokepointDistanceToCenterMap < minDistance) {
			minDistance = chokepointDistanceToCenterMap;
			bestChokepoint = chokepoint;
		}
	}

	// set the region with highest "ground height" as a seedRegion (don't change if they are equal)
	std::pair<BWTA::Region*, BWTA::Region*> chokeRegions = bestChokepoint->getRegions();
	int groundHeight1 = Broodwar->getGroundHeight(TilePosition(chokeRegions.first->getCenter()));
	int groundHeight2 = Broodwar->getGroundHeight(TilePosition(chokeRegions.second->getCenter()));
	if (groundHeight1 > groundHeight2) seedRegion = chokeRegions.first;
	else if (groundHeight1 < groundHeight2) seedRegion = chokeRegions.second;

	// compute best seed position
	informationManager->_bunkerSeedPosition = Position(getBuildLocationNear(TilePosition(bestChokepoint->getCenter()), UnitTypes::Terran_Bunker, seedRegion));
#ifndef TOURNAMENT
// 	informationManager->printDebugBox(Position(informationManager->_bunkerSeedPosition), "Bunker position calculated");
#endif
}

void BuildManager::openGate()
{
	if (gateBuilding) {
		bool ok = gateBuilding->lift();
		if (ok) {
			gateOpened = true;
			Broodwar->printf("Opening Gate!");
		}
	}
}

void BuildManager::closeGate()
{
	if (gateBuilding) {
		bool ok = gateBuilding->land(wallGenerator->BarracksWall);
		if (ok) {
			gateOpened = false;
			Broodwar->printf("Closing Gate!");
		}
	}
}