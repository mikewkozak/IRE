#include "DropshipBehaviour.h"
#include "SquadAgent.h"
#include "SquadManager.h"

using namespace BWAPI;

DropshipBehaviour::DropshipBehaviour(Unit unit, class SquadAgent* squad)
	: _unit(unit),
	_squad(squad),
	_logger(log4cxx::Logger::getLogger("SquadManager.CombatAgent.Dropship")),
	_lastPosition(Positions::None),
	loadedSquad(0),
	doneLoading(false),
	unloadTimer(0)
{

}

void DropshipBehaviour::byDefault()
{
	//DEBUG("DropshipMicro start");
	BWAPI::Unitset loadedUnits = _unit->getLoadedUnits();

	if (loadedUnits.size() == 0 && loadedSquad == 0) {
//		DEBUG("DropshipMicro empty Dropship");
		// state 0: empty Dropship
		// Look for a set of units to load: store them in some local variable
		
		// If the Dropship is injured, head to a base for repair
		if (_unit->getHitPoints()<_unit->getInitialHitPoints()/2) {
			// Where do Dropship go to repair?!?!?!
			// ...
			return;
		}

		double longestDistance = 0;
		SquadAgent *furthestSquad = 0;
		for (const auto& squad : squadManager->_squads) {
// 		for(SquadSet::const_iterator sq=squadManager->_squads.begin();sq!=squadManager->_squads.end();++sq) {
			// Make sure it has enough non flying units:
			int nonFlyerSize = 0;
			for (const auto& su : squad->_squadUnits) {
				if (!su->_unit->getType().isFlyer()) {
					if (su->_unit->getType().size() == UnitSizeTypes::Small) nonFlyerSize++;
					if (su->_unit->getType().size() == UnitSizeTypes::Medium) nonFlyerSize += 2;
					if (su->_unit->getType().size() == UnitSizeTypes::Large) nonFlyerSize += 4;
				}
			}
			if (nonFlyerSize>=8) {	// Make sure we can fully load the Dropship
				BWAPI::Position target = squad->_positionTarget;
				BWAPI::Position current = squad->_center;
				double distance = target.getDistance(current);
				if (furthestSquad == 0 || longestDistance<distance) {
					furthestSquad = squad;
					longestDistance = distance;
				}
			}
		}

		if (furthestSquad!=0) {
			if (longestDistance>1000) {	// If units are reasonably far
				//Broodwar->printf("Dropship going for squad at distance %g",longestDistance);
				loadedSquad = furthestSquad;
				doneLoading = false;
			}
		}
		
	} else if (!doneLoading && loadedSquad != 0) {
//		DEBUG("DropshipMicro waiting for load");
		// state 1: half-full Dropship
		// check that the squad still exists
		auto it = std::find(squadManager->_squads.begin(), squadManager->_squads.end(), loadedSquad);
		
		if (it != squadManager->_squads.end()) {
			int spaceLeft = 8;
			for(BWAPI::Unitset::iterator i=loadedUnits.begin();i!=loadedUnits.end();++i) {
				if ((*i)->getType().size() == UnitSizeTypes::Small) spaceLeft--;
				if ((*i)->getType().size() == UnitSizeTypes::Medium) spaceLeft-=2;
				if ((*i)->getType().size() == UnitSizeTypes::Large) spaceLeft-=4;
			}
			bool sentAnyCommand = false;
			for(CombatUnitSet::const_iterator i=loadedSquad->_squadUnits.begin();i!=loadedSquad->_squadUnits.end();++i) {
				int uSize = 1;
				if ((*i)->_unit->getType().size() == UnitSizeTypes::Medium) uSize = 2;
				if ((*i)->_unit->getType().size() == UnitSizeTypes::Large) uSize = 4;
				if (!(*i)->_unit->getType().isFlyer() && !(*i)->_unit->isLoaded() && spaceLeft>=uSize) {
					spaceLeft-=uSize;
					if ((*i)->_unit->getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode) {
						(*i)->_unit->unsiege();
					} else {
						(*i)->_unit->load(_unit);
					}
					sentAnyCommand = true;
					if (spaceLeft<=0) break;
				}
			}
			if (!sentAnyCommand) {
				doneLoading = true;
				unloadTimer = 0;
			}
		} else {
			//Broodwar->printf("Dropship: Squad being loadded was destroyed!!");
			loadedSquad = 0;
		}

	} else {
//		DEBUG("DropshipMicro full Dropship");
		// state 2: full Dropship
		// If we are close by, or injured, unload!
		if (loadedSquad==0) {
			//Broodwar->printf("Error! Squad in Dropship disappeared!");
			_unit->unloadAll();
		} else {
			if (_unit->getPosition().getDistance(loadedSquad->_positionTarget)<1000 || _unit->getHitPoints()<_unit->getInitialHitPoints()/2) {
				// find a position to land:
				if (loadedUnits.size()>0) {
					if (unloadTimer==0) {
						_unit->unloadAll();
						unloadTimer = 300;	// give some time for the unload action to take place...
					} else {
						unloadTimer--;
					}
				} else {
					loadedSquad = 0;
				}
			} else {
	//			Broodwar->printf("Dropship: Full dropship at distance %g",_unit->getPosition().getDistance(loadedSquad->_positionTarget));
				_unit->move(loadedSquad->_positionTarget);
			}
		}
	}
	//DEBUG("DropshipMicro end");
}

void DropshipBehaviour::onGetPosition(Position targetPosition)
{

}

void DropshipBehaviour::onGetNewPosition(Position targetPosition)
{

}


void DropshipBehaviour::onCombat(BWAPI::Unit bestTarget, const BWAPI::Unitset &enemies)
{

}

void DropshipBehaviour::onStop() {};
void DropshipBehaviour::onHold() {};