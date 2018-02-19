#include "ScienceVesselBehaviour.h"
#include "SquadAgent.h"
#include "SquadManager.h"

using namespace BWAPI;

ScienceVesselBehaviour::ScienceVesselBehaviour(Unit unit, class SquadAgent* squad)
	: _unit(unit),
	_squad(squad),
	_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent.ScienceVessel")),
	_lastPosition(Positions::None)
{

}

void ScienceVesselBehaviour::byDefault()
{
	LOG4CXX_TRACE(_logger,"Science Vessel micro START");
	int x = _unit->getTilePosition().x;
	int y = _unit->getTilePosition().y;
	if (informationManager->get_enemy_air_dps(x, y,BWAPI::Broodwar->getFrameCount())>0) {
		LOG4CXX_TRACE(_logger,"Looking for a location to flee");
		// Find best position in spiral
		Position returnPosition;
		int length = 1;
		int j      = 0;
		bool first = true;
		int dx     = 0;
		int dy     = 1;	
		while (length < Broodwar->mapWidth()) {
			returnPosition = Position(x*TILE_SIZE, y*TILE_SIZE);

			if (x >= 0 && x < Broodwar->mapWidth() && y >= 0 && y < Broodwar->mapHeight() ) {
				double dps = informationManager->get_enemy_air_dps(x, y,BWAPI::Broodwar->getFrameCount());
				if (dps == 0) {
					_unit->move(returnPosition);
					_lastPosition = returnPosition;
					LOG4CXX_TRACE(_logger,"Science Vessel micro END");
					return;
				}
			}

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
		LOG4CXX_WARN(_logger,"NOT location to flee!!");
	} else {  //  *********** We are in a save location
		LOG4CXX_TRACE(_logger,"Searching best unit");
		CombatAgent* bestUnit = squadManager->_leadSquad->getClosestUnitTo(squadManager->_leadSquad->_positionTarget, UnitTypes::None, true);
		if (bestUnit != 0) {
			LOG4CXX_TRACE(_logger,"Best unit: " << bestUnit->_unit->getType().c_str());
			Position location = bestUnit->_unit->getPosition(); // get unit closest to target
			Position scoutPosition = getPositionInDirection(location, _squad->_positionTarget, 70 ); // distance approx. 2*TILE_SIZE
			TilePosition scoutTilePosition = TilePosition(scoutPosition);
			if (scoutPosition.isValid() && scoutTilePosition.isValid()) {
				LOG4CXX_TRACE(_logger,"Location valid");
				if (_lastPosition != scoutPosition && informationManager->get_enemy_air_dps(scoutTilePosition.x, scoutTilePosition.y,BWAPI::Broodwar->getFrameCount())==0) {
					_unit->move(scoutPosition);
					_lastPosition = scoutPosition;
				}
			} else {
				LOG4CXX_TRACE(_logger,"Location NOT valid");
			}
		} else {
			// 			// we need to find another squad
			_unit->move(informationManager->home->getCenter());
			// 			LOG4CXX_TRACE(_logger,"Best unit in our squad missing (empty squad)...");
			// 			SquadAgent *bestSquad = squadManager->getClosestSquad(squad->_center, squad);
			// 			if (bestSquad != nullptr) {
			// 				LOG4CXX_TRACE(_logger,"New squad for the Science Vessel " << squadManager->_squads.size());
			// 				// Merging squads TODO: just add unit to the new squad and delete old
			// 				squad->inMerge(bestSquad);
			// 				bestSquad->inMerge(squad, squad->_positionToMerge);
			// 				squadManager->_squadsToMerge.insert(std::make_pair(squad, bestSquad));
			// 				if (squadManager->_creatingSquad == bestSquad) {
			// 					squadManager->_creatingSquad = 0;
			// 				}
			// 			}
			// 			LOG4CXX_TRACE(_logger,"WAITING a squad for the Science Vessel");
		}

	}

	// Debug info
	if (_lastPosition!=Positions::None) {
		Broodwar->drawLineMap(_unit->getPosition().x, _unit->getPosition().y, _lastPosition.x, _lastPosition.y, Colors::Cyan);
		Broodwar->drawCircleMap(_lastPosition.x, _lastPosition.y,3,Colors::Cyan,true);
	}
	LOG4CXX_TRACE(_logger,"Science Vessel micro END");
}

void ScienceVesselBehaviour::onGetPosition(Position targetPosition)
{

}

void ScienceVesselBehaviour::onGetNewPosition(Position targetPosition)
{

}

void ScienceVesselBehaviour::onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies)
{

}

void ScienceVesselBehaviour::onStop() {};
void ScienceVesselBehaviour::onHold() {};