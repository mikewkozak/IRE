#pragma once

#include "InformationManager.h"
#include "WorkerManager.h"

class EnhancedUI
{
public:
	void onFrame();
	void drawUnitHealth(BWAPI::Unit unit);
	void drawVisibilityData();
	void drawBases() const;
	void drawTerrain() const;
	void getHeatMapColor( float value, float &red, float &green, float &blue );
};