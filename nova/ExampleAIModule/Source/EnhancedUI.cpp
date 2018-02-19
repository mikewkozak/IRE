#include "EnhancedUI.h"

using namespace BWAPI;

typedef std::set<BWTA::Region*> RegionSet;

void EnhancedUI::onFrame()
{
#ifdef TERRAIN_ANALYSIS
	// Draw distance transform map
// 	int mapW = Broodwar->mapWidth()*4;
// 	int mapH = Broodwar->mapHeight()*4;
// 
// 	float red, green, blue;
// 	BWTA::RectangleArray<int>* distanceMap2 = BWTA::getDistanceTransformMap();
// 	for(int x=0; x < mapW; ++x) {
// 		for(int y=0; y < mapH; ++y) {
// 			float normalized = (float)distanceMap2->getItem(x,y)/(float)BWTA::getMaxDistanceTransform();
// 			getHeatMapColor(normalized, red, green, blue );
// 			Color heatColor = Color((int)red, (int)green, (int)blue);
// 			Broodwar->drawCircleMap(x*8+4,y*8+4,1,heatColor,true);
// 		}
// 	}
#endif

	if (informationManager->_scoutedAnEnemyBase) {
		Broodwar->drawCircleMap(informationManager->_enemyStartPosition.x,informationManager->_enemyStartPosition.y,20,Colors::Red,true);
	}

	// draw BWTA info
	if (informationManager->mapAnalyzed) {
		drawBases();
		drawTerrain();
	}

	if (PRINT_SIEGE_MAP) {
		int mapW = Broodwar->mapWidth()*4;
		int mapH = Broodwar->mapHeight()*4;
		for(int x=0; x < mapW; ++x) {
			for(int y=0; y < mapH; ++y) {
				if ( informationManager->_tankSiegeMap[x][y] )
					//Broodwar->drawCircleMap(x*TILE_SIZE+16,y*TILE_SIZE+16,3,Colors::Green,true);
					Broodwar->drawCircleMap(x*8+4,y*8+4,1,Colors::Green,true);
				else
					//Broodwar->drawCircleMap(x*TILE_SIZE+16,y*TILE_SIZE+16,3,Colors::Red,true);
					Broodwar->drawCircleMap(x*8+4,y*8+4,1,Colors::Red,true);
			}
		}
	}

	// draw units health
	for(BWAPI::Unitset::iterator i=Broodwar->getAllUnits().begin();i!=Broodwar->getAllUnits().end();i++) {
		// FIXME: only combat units (not buildings)
		drawUnitHealth(*i);
// 		Unit targetUnit = (*i)->getOrderTarget();
// 		Position targetPosition = (*i)->getTargetPosition();
// 		if (targetUnit != NULL) {
// 			Broodwar->drawLineMap((*i)->getPosition().x,(*i)->getPosition().y,targetUnit->getPosition().x,targetUnit->getPosition().y,Colors::Red);
// 		}
// 		if (targetPosition != Positions::None) {
// 			Broodwar->drawLineMap((*i)->getPosition().x,(*i)->getPosition().y,targetPosition.x,targetPosition.y,Colors::Yellow);
// 		}
	}
	//drawVisibilityData();

	// Army stats
// 	Broodwar->drawTextScreen(290,13,"Marines: %d", Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Marine) );
// 	Broodwar->drawTextScreen(290,26,"Medics: %d", Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Medic) );

	// Search debug
//  	Broodwar->drawTextScreen(290,13,"Corner: %i", informationManager->_searchCorner );
//  	Broodwar->drawTextScreen(290,26,"Iter: %i", informationManager->_searchIter );

	// Production stats
	if (PRINT_PRODUCTION) {
 		Broodwar->drawTextScreen(280,13,"Blocked production by");
 		Broodwar->drawTextScreen(280,26,"Command: %c%i", 0x11, informationManager->_wastedProductionFramesByCommandCenter );
 		Broodwar->drawTextScreen(280,39,"Research: %c%i", 0x11, informationManager->_wastedProductionFramesByResearch );
 		Broodwar->drawTextScreen(280,52,"Money: %c%i", 0x11, informationManager->_wastedProductionFramesByMoney );
 		Broodwar->drawTextScreen(280,65,"Supply: %c%i", 0x11, informationManager->_wastedProductionFramesBySupply );
	}

	// Home Rally Position
	if (informationManager->_initialRallyPosition != Positions::None) {
		Broodwar->drawCircleMap(informationManager->_initialRallyPosition,8,Colors::Green,true);
// 		Broodwar->setScreenPosition(informationManager->_initialRallyPosition);
	}

	// APM
	//Broodwar->drawTextScreen(575,15,"APM: %d", Broodwar->getAPM());

	// DPS Map
	if (PRINT_AIR_DPS) informationManager->drawAirDPSMap();
	if (PRINT_GROUND_DPS) informationManager->drawGroundDPSMap();

	// Debug box
	if (informationManager->getDebugBoxTopLeft() != Positions::None) {
		Broodwar->drawBoxMap(informationManager->getDebugBoxTopLeft().x, informationManager->getDebugBoxTopLeft().y,
			informationManager->getDebugBoxBottomRight().x, informationManager->getDebugBoxBottomRight().y, Colors::Red);
		Broodwar->drawLineMap(informationManager->getDebugBoxTopLeft(), informationManager->getDebugBoxBottomRight(), Colors::Red);
	}
	// Debug circle
	if (informationManager->_center.x != 0) {
		Broodwar->drawCircleMap(informationManager->_center.x,informationManager->_center.y,informationManager->_radius,Colors::Red,false);
		Broodwar->drawDotMap(informationManager->_center.x,informationManager->_center.y,Colors::Yellow);
	}

	Broodwar->drawTextScreen(310,0,"Seen enemies: %d", informationManager->seenEnemies.size());
	for (const auto& mapSeenEnemy : informationManager->seenEnemies) {
		const unitCache_t* u = &mapSeenEnemy.second;
		Position leftTop(u->position.x - u->type.dimensionLeft(), u->position.y - u->type.dimensionUp());
		Position rightBottom(u->position.x + u->type.dimensionRight(), u->position.y + u->type.dimensionDown());
		Position leftBottom(leftTop.x, rightBottom.y);
		// TODO if position visible, print red
		Broodwar->drawBoxMap(leftTop, rightBottom, Colors::Yellow);
		Broodwar->drawTextMap(leftBottom, u->type.c_str());
	}
}

void EnhancedUI::drawUnitHealth(Unit unit)
{
	// Hit points
	if (!unit->isVisible()) return;

	// print health
	double maxHealth = unit->getType().maxHitPoints();
	double health = unit->getHitPoints();

	Color barColor = Colors::Green;
	if (unit->isUnderAttack())
		barColor = Colors::Yellow;

	Broodwar->drawBox(CoordinateType::Map, unit->getPosition().x - 7 + (int) (20 * health / maxHealth), unit->getPosition().y + 7,
		unit->getPosition().x - 7 + 20,
		unit->getPosition().y + 10,
		Colors::Orange, true);

	Broodwar->drawBox(CoordinateType::Map, unit->getPosition().x - 7, unit->getPosition().y + 7,
		unit->getPosition().x - 7 + (int) (20 * health / maxHealth),
		unit->getPosition().y + 10,
		barColor, true);

	// print shields
	if (unit->getType().maxShields() > 0) {
		double maxShields = unit->getType().maxShields();
		double shields = unit->getShields();

		barColor = Colors::Cyan;
		if (unit->isUnderAttack())
			barColor = Colors::Yellow;

		Broodwar->drawBox(CoordinateType::Map, unit->getPosition().x - 7 + (int) (20 * shields / maxShields), unit->getPosition().y + 10,
			unit->getPosition().x - 7 + 20,
			unit->getPosition().y + 13,
			Colors::Orange, true);

		Broodwar->drawBox(CoordinateType::Map, unit->getPosition().x - 7, unit->getPosition().y + 10,
			unit->getPosition().x - 7 + (int) (20 * shields / maxShields),
			unit->getPosition().y + 13,
			barColor, true);
	}
}

void EnhancedUI::drawVisibilityData()
{
  for(int x=0;x<Broodwar->mapWidth();++x)
  {
    for(int y=0;y<Broodwar->mapHeight();++y)
    {
      if (Broodwar->isExplored(x,y))
      {
        if (Broodwar->isVisible(x,y))
          Broodwar->drawDotMap(x*TILE_SIZE+16,y*TILE_SIZE+16,Colors::Green);
        else
          Broodwar->drawDotMap(x*TILE_SIZE+16,y*TILE_SIZE+16,Colors::Blue);
      }
      else
        Broodwar->drawDotMap(x*TILE_SIZE+16,y*TILE_SIZE+16,Colors::Red);
    }
  }
}

void EnhancedUI::drawBases() const
{
	//we will iterate through all the base locations, and draw their outlines.
	// Our bases
	for(std::map<BWTA::BaseLocation*, BWAPI::TilePosition>::const_iterator i=informationManager->_ourBases.begin();i!=informationManager->_ourBases.end();++i) {
		TilePosition p = i->first->getTilePosition();
		Broodwar->drawBox(CoordinateType::Map,p.x*TILE_SIZE,p.y*TILE_SIZE,p.x*TILE_SIZE+4*TILE_SIZE,p.y*TILE_SIZE+3*TILE_SIZE,Colors::Green,false);
	}
	// Mineral patch
	workerManager->_assignedWorkersToMinerals;
	for(ResourceToWorkerMap::const_iterator j = workerManager->_assignedWorkersToMinerals.begin(); j != workerManager->_assignedWorkersToMinerals.end(); ++j) {
		Position q=j->first->getPosition();
		Broodwar->drawCircleMap(q.x,q.y,30,Colors::Cyan,false);
		Broodwar->drawTextMap(q.x,q.y-5,"%d", j->second);
	}

	// Empty bases
	for(std::set<BWTA::BaseLocation*>::const_iterator i=informationManager->_emptyBases.begin();i!=informationManager->_emptyBases.end();++i) {
		TilePosition p=(*i)->getTilePosition();
		Broodwar->drawBox(CoordinateType::Map,p.x*TILE_SIZE,p.y*TILE_SIZE,p.x*TILE_SIZE+4*TILE_SIZE,p.y*TILE_SIZE+3*TILE_SIZE,Colors::Yellow,false);
	}

	// Ignore bases
	for(std::set<TilePosition>::const_iterator i=informationManager->_ignoreBases.begin();i!=informationManager->_ignoreBases.end();++i) {
		TilePosition p=(*i);
		Broodwar->drawBox(CoordinateType::Map,p.x*TILE_SIZE,p.y*TILE_SIZE,p.x*TILE_SIZE+4*TILE_SIZE,p.y*TILE_SIZE+3*TILE_SIZE,Colors::Orange,false);
	}

	// Enemy bases
	for(std::set<BWTA::BaseLocation*>::const_iterator i=informationManager->_enemyBases.begin();i!=informationManager->_enemyBases.end();++i) {
		TilePosition p=(*i)->getTilePosition();
		Broodwar->drawBox(CoordinateType::Map,p.x*TILE_SIZE,p.y*TILE_SIZE,p.x*TILE_SIZE+4*TILE_SIZE,p.y*TILE_SIZE+3*TILE_SIZE,Colors::Red,false);
	}
}

void EnhancedUI::drawTerrain() const
{
	//we will iterate through all the regions and ...
// 	const RegionSet &regions = BWTA::getRegions();
	for (const auto& r : BWTA::getRegions()) {
// 	for(RegionSet::const_iterator r = regions.begin(); r != regions.end(); ++r)
// 	{
		// Draw the polygon outline of it in green
		const BWTA::Polygon &p = r->getPolygon();
		for(int j=0;j<(int)p.size();j++)
		{
			Position point1=p[j];
			Position point2=p[(j+1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map,point1.x,point1.y,point2.x,point2.y,Colors::Green);
		}

		// Draw the chokepoints with yellow lines
		const std::set<BWTA::Chokepoint*> &chokepoints = r->getChokepoints();
		for(std::set<BWTA::Chokepoint*>::const_iterator c = chokepoints.begin(); c != chokepoints.end(); ++c)
		{
			const Position &point1=(*c)->getSides().first;
			const Position &point2=(*c)->getSides().second;
			Broodwar->drawLine(CoordinateType::Map,point1.x,point1.y,point2.x,point2.y,Colors::Yellow);
		}

		// Draw center
// 		Position center = (*r)->getCenter();
// 		Broodwar->drawCircleMap(center.x,center.y,30,Colors::Blue,true);

		
	}

	if (PRINT_REGION_ID_MAP) {
		for(int x=0;x<Broodwar->mapWidth();++x) {
			for(int y=0;y<Broodwar->mapHeight();++y) {
				Broodwar->drawTextMap(x*TILE_SIZE+16,y*TILE_SIZE+16,"%i", informationManager->_regionIdMap[x][y] );
			}
		}
	}
}

void EnhancedUI::getHeatMapColor( float value, float &red, float &green, float &blue )
{
	const int NUM_COLORS = 3;
	static float color[NUM_COLORS][3] = { {255,0,0}, {0,255,0}, {0,0,255} };
	// a static array of 3 colors:  (red, green, blue,   green)

	int idx1;        // |-- our desired color will be between these two indexes in "color"
	int idx2;        // |
	float fractBetween = 0;  // fraction between "idx1" and "idx2" where our value is

	if(value <= 0)      {  idx1 = idx2 = 0;            }    // accounts for an input <=0
	else if(value >= 1)  {  idx1 = idx2 = NUM_COLORS-1; }    // accounts for an input >=0
	else
	{
		value = value * (NUM_COLORS-1);        // will multiply value by 3
		idx1  = (int)floor(value);                  // our desired color will be after this index
		idx2  = idx1+1;                        // ... and before this index (inclusive)
		fractBetween = value - float(idx1);    // distance between the two indexes (0-1)
	}

	red   = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
	green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
	blue  = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
}

