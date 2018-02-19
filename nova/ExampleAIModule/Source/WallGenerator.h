#pragma once

#include "InformationManager.h"
#include "BuildManager.h"

struct wallData_t {
	std::string mapHash;
	BWAPI::TilePosition startTile;
	BWAPI::TilePosition supply1;
	BWAPI::TilePosition supply2;
	BWAPI::TilePosition barracks;
};

struct unitGap_t{
	__int8 top;
	__int8 bottom;
	__int8 left;
	__int8 right;
	__int8 width;
	__int8 heigth;
	unitGap_t(): top(0), bottom(0), left(0), right(0), width(0), heigth(0){}
	unitGap_t(__int8 _top, __int8 _bottom, __int8 _left, __int8 _right, __int8 _width, __int8 _heigth)
		: top(_top), bottom(_bottom), left(_left), right(_right), width(_width), heigth(_heigth){}
};

class WallGenerator{
public:
	WallGenerator();
	int mapW;
	int mapH;

	std::vector<wallData_t> wallData;

	bool WallCalculated; // true when the map calculation is done
	bool WallSound;

	BWAPI::TilePosition BarracksWall;
	BWAPI::TilePosition SupplyWall1;
	BWAPI::TilePosition SupplyWall2;
	BWAPI::TilePosition CC1;
	BWAPI::TilePosition Academy1;

	BWAPI::TilePosition TopLeft;
	BWAPI::TilePosition BottomRight;
	BWAPI::TilePosition Furthest;
	BWAPI::TilePosition Closest;

	int** WallMap;
	int Visited[129 * 4][129 * 4];
	std::vector<BWAPI::TilePosition> BuildingPlace;

	int unitHeight;
	int unitWidth;

	void AddWallData(std::string mapHash, int startX, int startY, int Supply1X, int Supply1Y, int Supply2X, int Supply2Y, int BarrackX, int BarrackY);
	void LoadWallData();

	void WallOff();
	bool CanWall(int BuildType, int xPos, int yPos);
	void RecursiveWall(std::vector<int> Buildings, int depth);
	void mapWall(int xPos, int yPos, int BuildType, int place);
	bool WalledOff(int x, int y, int Ignore, int unitH, int unitW);
	bool WalledOffAstar(int x, int y, int Ignore, int unitH, int unitW);
	int MaxGap(int x, int y, int &Hgap, int &Vgap);
};