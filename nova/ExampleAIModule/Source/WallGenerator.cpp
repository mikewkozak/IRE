#include <queue> // for WalledOffAstar
#include "WallGenerator.h"

using namespace BWAPI;

#define BLOCKED 0
#define BUILDABLE 1

int MaxHighGround = 0;
bool useHighGround = true; //set to false when high ground wall fails

unitGap_t SpaceArray[10];

WallGenerator::WallGenerator()
	:WallCalculated(false)
{

}

void WallGenerator::AddWallData(std::string mapHash, int startX, int startY, int Supply1X, int Supply1Y, int Supply2X, int Supply2Y, int BarrackX, int BarrackY){
	wallData_t newWD;
	newWD.mapHash = mapHash;
	newWD.startTile = BWAPI::TilePosition(startX, startY);
	newWD.supply1 = BWAPI::TilePosition(Supply1X, Supply1Y);
	newWD.supply2 = BWAPI::TilePosition(Supply2X, Supply2Y);
	newWD.barracks = BWAPI::TilePosition(BarrackX, BarrackY);
	wallData.push_back(newWD);
}

void WallGenerator::LoadWallData()
{
	wallData.clear();
	// AIIDE Wall maps
	AddWallData("af618ea3ed8a8926ca7b17619eebcb9126f0d8b1", 117, 13, 101, 25, 104, 25, 106, 27);	// Benzene 2 o'clock
	AddWallData("af618ea3ed8a8926ca7b17619eebcb9126f0d8b1", 7, 96, 20, 81, 22, 83, 23, 85);			// Benzene 7 o'clock
	AddWallData("4e24f217d2fe4dbfa6799bc57f74d8dc939d425b", 64, 118, 46, 114, 46, 116, 44, 118);	// Destination 5 o'clock
	AddWallData("4e24f217d2fe4dbfa6799bc57f74d8dc939d425b", 31, 7, 47, 6, 47, 8, 48, 10);			// Destination 11 o'clock
	AddWallData("6f8da3c3cc8d08d9cf882700efa049280aedca8c", 117, 56, 107, 24, 105, 26, 102, 28);	// Heartbreak Ridge 4 o'clock
	AddWallData("6f8da3c3cc8d08d9cf882700efa049280aedca8c", 7, 37, 21, 65, 20, 67, 17, 69);			// Heartbreak Ridge 10 o'clock
	AddWallData("ba2fc0ed637e4ec91cc70424335b3c13e131b75a", 117, 100, 108, 119, 0, 0, 104, 117);	// Aztec 5 o'clock
	AddWallData("ba2fc0ed637e4ec91cc70424335b3c13e131b75a", 7, 83, 6, 63, 9, 63, 2, 64);			// Aztec 8 o'clock
	AddWallData("ba2fc0ed637e4ec91cc70424335b3c13e131b75a", 68, 6, 92, 8, 94, 6, 93, 10);			// Aztec 12 o'clock
	AddWallData("9bfc271360fa5bab3707a29e1326b84d0ff58911", 117, 9, 108, 43, 0, 0, 104, 43);		// Tau Cross 2 o'clock
	AddWallData("9bfc271360fa5bab3707a29e1326b84d0ff58911", 93, 118, 52, 105, 0, 0, 48, 105);		// Tau Cross 5 o'clock
	AddWallData("9bfc271360fa5bab3707a29e1326b84d0ff58911", 7, 44, 30, 18, 0, 0, 26, 18);			// Tau Cross 10 o'clock
	AddWallData("1e983eb6bcfa02ef7d75bd572cb59ad3aab49285", 117, 7, 96, 27, 98, 29, 100, 31);		// Andromeda 2 o'clock
	AddWallData("1e983eb6bcfa02ef7d75bd572cb59ad3aab49285", 117, 119, 101, 95, 99, 97, 96, 99);		// Andromeda 4 o'clock
	AddWallData("1e983eb6bcfa02ef7d75bd572cb59ad3aab49285", 7, 118, 24, 95, 26, 97, 28, 99);		// Andromeda 8 o'clock
	AddWallData("1e983eb6bcfa02ef7d75bd572cb59ad3aab49285", 7, 6, 29, 27, 27, 29, 24, 31);			// Andromeda 10 o'clock
	AddWallData("450a792de0e544b51af5de578061cb8a2f020f32", 117, 9, 125, 25, 122, 24, 118, 23);		// Circuit Breakers 2 o'clock
	AddWallData("450a792de0e544b51af5de578061cb8a2f020f32", 117, 118, 125, 101, 122, 102, 118, 102); // Circuit Breakers 5 o'clock
	AddWallData("450a792de0e544b51af5de578061cb8a2f020f32", 7, 118, 0, 102, 3, 102, 6, 102);		// Circuit Breakers 8 o'clock
	AddWallData("450a792de0e544b51af5de578061cb8a2f020f32", 7, 9, 0, 23, 3, 23, 6, 23);				// Circuit Breakers 11 o'clock
	AddWallData("de2ada75fbc741cfa261ee467bf6416b10f9e301", 83, 6, 60, 2, 63, 2, 64, 4);			// Python 1 o'clock
	AddWallData("de2ada75fbc741cfa261ee467bf6416b10f9e301", 117, 40, 117, 55, 120, 55, 122, 57);	// Python 2 o'clock
	AddWallData("de2ada75fbc741cfa261ee467bf6416b10f9e301", 42, 119, 61, 121, 64, 125, 60, 123);	// Python 7 o'clock
	AddWallData("de2ada75fbc741cfa261ee467bf6416b10f9e301", 7, 86, 7, 71, 4, 71, 0, 71);			// Python 8 o'clock
		

	// Search current map and starting position
	for (auto wallInfo : wallData) {
		if (wallInfo.mapHash == Broodwar->mapHash() && wallInfo.startTile == Broodwar->self()->getStartLocation()) {
			Broodwar->printf("Wall in found in IO");
			SupplyWall1 = wallInfo.supply1;
			SupplyWall2 = wallInfo.supply2;
			BarracksWall = wallInfo.barracks;
			WallCalculated = true;
			WallSound = true;
			break;
		}
	}
}


//Find locations to wall of the choke
void WallGenerator::WallOff(){

	//Init wall map
	int** buildMap = buildManager->getBuildMap();

	mapW = Broodwar->mapWidth();
	mapH = Broodwar->mapHeight();
	
	WallMap = new int*[mapW];
	for (int x = 0; x < mapW; ++x) {
		WallMap[x] = new int[mapH];

		for (int y = 0; y < mapH; ++y) {
			WallMap[x][y] = buildMap[x][y];
		}
	}

	// Gaps data from http://wiki.teamliquid.net/starcraft/List_of_Unit_and_Building_Sizes
	unitGap_t noBuilding(16,16,16,16,1,1); // noBuilding means no gap
	unitGap_t barracks(8,15,16,7,4,3);
	unitGap_t supplyDepot(10,5,10,9,3,2);
	unitGap_t commandCenter(7,6,6,5,4,3);
	unitGap_t academy(0,7,8,3,3,2);

	SpaceArray[0] = noBuilding;
	SpaceArray[1] = noBuilding;
	SpaceArray[2] = barracks;
	SpaceArray[3] = supplyDepot;
	SpaceArray[4] = supplyDepot;
	SpaceArray[5] = academy;

	// get the choke point from home region closest to the center of the map
	std::set<BWTA::Chokepoint*> chokes = informationManager->home->getChokepoints();
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

	// get center tile of the chokepoint
	BWAPI::TilePosition TileChoke(BWAPI::TilePosition(bestChokepoint->getCenter()));

	std::vector<int> Buildings;
	Buildings.push_back(2);//Barracks
	Buildings.push_back(3);//Supply depot
	Buildings.push_back(4);//Supply depot
	//Buildings.push_back(5);// Academy

	int BuildSize = 9;

	int TopX = TileChoke.x - BuildSize;
	if (TopX < 0) TopX = 0;
	int TopY = TileChoke.y - BuildSize;
	if (TopY < 0) TopY = 0;
	TopLeft = TilePosition(TopX, TopY);

	int BottomX = TileChoke.x + BuildSize;
	if (BottomX >= mapW) BottomX = mapW - 1;
	int BottomY = TileChoke.y + BuildSize;
	if (BottomY >= mapH) BottomY = mapH - 1;
	BottomRight = TilePosition(BottomX, BottomY);

	//Get nearest and furthest buildable position
	BWAPI::TilePosition CommandC;
	for (auto unit : Broodwar->self()->getUnits()) {
		if (unit->getType().isResourceDepot()) {
			CommandC = BWAPI::TilePosition(unit->getPosition());
			break;
		}
	}

	//getGroundDistance
	double MaxT = 0;
	double MinT = 9999;
	for (int x = TopLeft.x; x<BottomRight.x; x++){
		for (int y = TopLeft.y; y<BottomRight.y; y++){
			if (!Broodwar->isBuildable(x, y)) {
				continue;
			}
			if (Broodwar->getGroundHeight(x, y) > MaxHighGround && (Broodwar->getGroundHeight(x, y) % 2 == 0)){
				MaxHighGround = Broodwar->getGroundHeight(x, y);
			}
			BWAPI::TilePosition Current = BWAPI::TilePosition(x, y);
			double getDist = BWTA::getGroundDistance(CommandC, Current);
			if (getDist < MinT){
				MinT = getDist;
				Closest = Current;
			}
		}
	}

	for (int x = TopLeft.x; x<BottomRight.x; x++){
		for (int y = TopLeft.y; y<BottomRight.y; y++){
			if (!Broodwar->isBuildable(x, y)){
				continue;
			}
			BWAPI::TilePosition backup = Furthest;
			BWAPI::TilePosition Current = BWAPI::TilePosition(x, y);
			double getDist = BWTA::getGroundDistance(CommandC, Current);
			if (getDist > MaxT){
				//MaxT = getDist;
				Furthest = Current;
				//Check if furthers is connected
				for (int x = (TopLeft.x * 4) - 4; x <= (BottomRight.x * 4) + 4; x++){
					for (int y = (TopLeft.y * 4) - 4; y <= (BottomRight.y * 4) + 4; y++){
						if (x < 0 || y < 0 || x >= mapW * 4 || y >= mapH * 4){
							continue;
						}
						Visited[x][y] = 0;
					}
				}
				bool isWalked = WalledOff(Closest.x * 4, Closest.y * 4, 1, 32, 32);//check if this spot is reachable
				if (isWalked == false){//if not reachable in 32x32, revert
					Furthest = backup;
				} else {
					MaxT = getDist;
				}
			}
		}
	}

	//set the unit height and width
	unitHeight = 19;
	unitWidth = 23;
	//unitHeight = 17;
	//unitWidth = 17;

	RecursiveWall(Buildings, 0);

	if (WallSound == false){
		Broodwar->printf("No wall found");
	}

	if (WallSound == false){
		useHighGround = false;
		RecursiveWall(Buildings, 0);
	}

	if (WallSound == true){
		WallCalculated = true;
	}

// 	if (useWallIO == true && WallSound == true){
// 		WallData newWD;
// 		newWD.mapHash = Broodwar->mapHash();
// 		newWD.startTile = InfoMan->OurBase;
// 		newWD.Supply1 = bManager->SupplyWall1;
// 		newWD.Supply2 = bManager->SupplyWall2;
// 		newWD.Barracks1 = bManager->BarracksWall;
// 		//newWD.CC1 = bManager->CC1;
// 		newWD.Academy1 = bManager->Academy1;
// 		//WD.push_back( newWD );
// 		//in the competition, this should be "bwapi-data/write/wall.txt"
// 		std::ofstream fout("bwapi-data/read/wall.txt", std::fstream::app);//, std::fstream::app
// 		if (!fout.fail()){
// 			fout << Broodwar->mapHash() << "\n";
// 			fout << InfoMan->OurBase.x() << " " << InfoMan->OurBase.y() << "\n";
// 			fout << bManager->SupplyWall1.x() << " " << bManager->SupplyWall1.y() << "\n";
// 			fout << bManager->SupplyWall2.x() << " " << bManager->SupplyWall2.y() << "\n";
// 			fout << bManager->BarracksWall.x() << " " << bManager->BarracksWall.y() << "\n";
// 			//fout<<bManager->CC1.x()<<" "<<bManager->CC1.y()<<"\n";
// 			//fout<<bManager->Academy1.x()<<" "<<bManager->Academy1.y()<<"\n";
// 		}
// 
// 	}

}

//Ignore = check if it is passable when barracks is removed
bool WallGenerator::WalledOff(int x, int y, int Ignore, int unitH, int unitW){
	bool isWalked = false;
	if (Visited[x][y] == 1){
		return false;
	}
	Visited[x][y] = 1;
	if (Furthest.x == (int)(x / 4) && Furthest.y == (int)(y / 4)){
		return true;
	}
	int Xm[4] = { -1, 1, 0, 0 };
	int Ym[4] = { 0, 0, -1, 1 };
	for (int i = 0; i<4; i++){
		int newX = (x + Xm[i]) / 4;
		int newY = (y + Ym[i]) / 4;
		int realX = x + Xm[i];
		int realY = y + Ym[i];
		if (newX < 0 || newX == mapW || newY < 0 || newY == mapH){//bound check
			continue;
		}
		//check 16x16 bound
		if (newX < TopLeft.x - 1 || newX > BottomRight.x + 1 || newY < TopLeft.y - 1 || newY > BottomRight.y + 1){//bound check
			continue;
		}
		if (Broodwar->isWalkable(realX, realY)){
			//Check if there is a building
			bool Walkable = false;
			if (WallMap[newX][newY] <= 1 || WallMap[newX][newY] == Ignore){
				Walkable = true;
			}
			//if there is a building, check if you can walk between the gap
			if (Walkable == false){
				int Hgap, Vgap;
				MaxGap(realX, realY, Hgap, Vgap);
				if (Hgap >= unitH && Vgap >= unitW){
					Walkable = true;
				}
			}
			if (Walkable == true){
				if (WalledOff(realX, realY, Ignore, unitH, unitW)){
					isWalked = true;
				}
			}
		}
	}
	return isWalked;
}

// a 2x2 part of the walkable array
//check for gaps between each of the tiles
//TODO: special case for Zerglings
int WallGenerator::MaxGap(int x, int y, int &Hgap, int &Vgap) {
	int maxGap = 0;
	Hgap = 0;
	Vgap = 0;

	//inside building, not possible to walk here
	if (WallMap[(x / 4)][(y / 4)] == WallMap[((x + 1) / 4)][(y / 4)] && WallMap[(x / 4)][(y / 4)] == WallMap[(x / 4)][((y + 1) / 4)]
		&& WallMap[(x / 4)][(y / 4)] > 1){
		Hgap = 0;
		Vgap = 0;
		return 0;
	}

	bool BGhorizontal = false;//check if there is a horizontal building gap. Needed for diagonal gap
	bool BGvertical = false;//check if there is a vertical building gap. Needed for diagonal gap

	int horizontal1 = 32;
	//building gap
	if (WallMap[(x / 4)][(y / 4)] != WallMap[((x + 1) / 4)][(y / 4)] && WallMap[(x / 4)][(y / 4)] > 1){
		horizontal1 = SpaceArray[WallMap[(x / 4)][(y / 4)]].right + SpaceArray[WallMap[((x + 1) / 4)][(y / 4)]].left;
		BGhorizontal = true;
	}
	if (!Broodwar->isWalkable(x + 1, y)){
		horizontal1 = 16;
	}
	//y+1
	int horizontal2 = 32;
	//building gap
	if (WallMap[(x / 4)][((y + 1) / 4)] != WallMap[((x + 1) / 4)][((y + 1) / 4)] && WallMap[(x / 4)][((y + 1) / 4)] > 1){
		horizontal2 = SpaceArray[WallMap[(x / 4)][((y + 1) / 4)]].right + SpaceArray[WallMap[((x + 1) / 4)][((y + 1) / 4)]].left;
	}
	if (!Broodwar->isWalkable(x + 1, y + 1) || !Broodwar->isWalkable(x, y + 1)){
		horizontal2 = 16;
	}
	//TODO: shorter way of writing this down with  ?  macro
	Hgap = horizontal1;
	if (horizontal2 > horizontal1){
		Hgap = horizontal2;
	}

	//Now vertical gap
	int vertical1 = 32;
	//building gap
	if (WallMap[(x / 4)][(y / 4)] != WallMap[(x / 4)][((y + 1) / 4)] && WallMap[(x / 4)][(y / 4)] > 1){
		vertical1 = SpaceArray[WallMap[(x / 4)][(y / 4)]].bottom + SpaceArray[WallMap[(x / 4)][((y + 1) / 4)]].top;
		BGvertical = true;
	}
	if (!Broodwar->isWalkable(x, y + 1)){
		vertical1 = 16;
	}
	//x+1
	int vertical2 = 32;
	//building gap
	if (WallMap[((x + 1) / 4)][(y / 4)] != WallMap[((x + 1) / 4)][((y + 1) / 4)] && WallMap[((x + 1) / 4)][(y / 4)] > 1){
		vertical2 = SpaceArray[WallMap[((x + 1) / 4)][(y / 4)]].bottom + SpaceArray[WallMap[((x + 1) / 4)][((y + 1) / 4)]].top;
		BGvertical = true;
	}
	if (!Broodwar->isWalkable(x + 1, y + 1) || !Broodwar->isWalkable(x + 1, y)){
		vertical1 = 16;
	}
	//TODO: shorter way of writing this down with  ?  macro
	Vgap = vertical1;
	if (vertical2 > vertical1){
		Vgap = horizontal2;
	}


	//Now diagonal gap
	int diagonal1 = 0;//diagonal gap is only relevant in building gap
	//diagonal gap between buildings is equal to horizontal gap
	if (WallMap[(x / 4)][(y / 4)] != WallMap[((x + 1) / 4)][((y + 1) / 4)] && WallMap[(x / 4)][(y / 4)] > 1 && WallMap[((x + 1) / 4)][((y + 1) / 4)] > 1){
		diagonal1 = SpaceArray[WallMap[(x / 4)][(y / 4)]].right + SpaceArray[WallMap[((x + 1) / 4)][((y + 1) / 4)]].left;
	}
	//x+1
	int diagonal2 = 0;
	if (WallMap[(x / 4)][((y + 1) / 4)] != WallMap[((x + 1) / 4)][(y / 4)] && WallMap[(x / 4)][((y + 1) / 4)] > 1 && WallMap[((x + 1) / 4)][(y / 4)] > 1){
		diagonal2 = SpaceArray[WallMap[(x / 4)][((y + 1) / 4)]].left + SpaceArray[WallMap[((x + 1) / 4)][(y / 4)]].right;
	}
	//TODO: shorter way of writing this down with  ?  macro
	int MinDiagonal = diagonal1;
	if (diagonal2 < diagonal1 || diagonal1 == 0){
		MinDiagonal = diagonal2;
	}


	//diagonal gap counts as a horizontal gap only if no other horizontal gap is present
	if (MinDiagonal > Hgap &&  MinDiagonal != 0 && BGhorizontal == false && BGvertical == false){
		Hgap = MinDiagonal;
	}

	return maxGap;
}


void WallGenerator::RecursiveWall(std::vector<int> Buildings, int depth){

	if (WallSound == true){ //wall in found
		return;
	}

	// is all buildings placed, check the gap
	if (depth == Buildings.size()){
		// reset visited to 0
		for (int x = (TopLeft.x * 4) - 4; x <= (BottomRight.x * 4) + 4; x++){
			for (int y = (TopLeft.y * 4) - 4; y <= (BottomRight.y * 4) + 4; y++){
				if (x < 0 || y < 0 || x >= mapW * 4 || y >= mapH * 4){
					continue;
				}
				Visited[x][y] = 0;
			}
		}

		//check if this is a wall
		bool isWalked = WalledOffAstar(Closest.x * 4, Closest.y * 4, 1, unitHeight, unitWidth);
		if (isWalked == false){
			// reset visited to 0
			for (int x = (TopLeft.x * 4) - 4; x <= (BottomRight.x * 4) + 4; x++){
				for (int y = (TopLeft.y * 4) - 4; y <= (BottomRight.y * 4) + 4; y++){
					if (x < 0 || y < 0 || x >= mapW * 4 || y >= mapH * 4){
						continue;
					}
					Visited[x][y] = 0;
				}
			}

			//Check if lifting the barracks allows passage
			bool Gate = WalledOffAstar(Closest.x * 4, Closest.y * 4, 2, unitHeight, unitWidth);

			if (Gate == true){
				if (WallSound == false){
					Broodwar->printf("Wall in Found");
					WallSound = true;
				}
				BarracksWall = BuildingPlace[0];
				SupplyWall1 = BuildingPlace[1];
				SupplyWall2 = BuildingPlace[2];

				if (WallSound == true){
					return;
				}
			}
		}
		return;
	}

	// Place building recursively
	BWAPI::TilePosition CurrentPos;
	int BuildType = Buildings[depth];
	for (int x = TopLeft.x; x <= BottomRight.x - SpaceArray[BuildType].width; x++){
		for (int y = TopLeft.y; y <= BottomRight.y - SpaceArray[BuildType].heigth; y++){
			if (CanWall(BuildType, x, y)){
				mapWall(x, y, BuildType, BuildType);
				BWAPI::TilePosition thisTile = BWAPI::TilePosition(x, y);
				BuildingPlace.push_back(thisTile);
				RecursiveWall(Buildings, depth + 1);
				BuildingPlace.pop_back();
				mapWall(x, y, BuildType, BUILDABLE);
			}
		}
	}

}

//struct used for A* priority queue
struct PrioAstar{
	int x;
	int y;
	int ManDist; // Manhattan distance
	bool operator()(const PrioAstar& lhs, const PrioAstar& rhs) const
	{
		return lhs.ManDist > rhs.ManDist;
	}

};

bool WallGenerator::WalledOffAstar(int Startx, int Starty, int Ignore, int unitH, int unitW){
	// A* using Manhattan distance only. Don't need path minimized, just find a path
	std::priority_queue<PrioAstar, std::vector<PrioAstar>, PrioAstar> pqAstar;
	PrioAstar start;
	start.x = Startx;
	start.y = Starty;
	int TileX = (int)(Startx / 4);
	int TileY = (int)(Starty / 4);
	start.ManDist = abs(TileX - Furthest.x) + abs(TileY - Furthest.y);
	pqAstar.push(start);
	while (!pqAstar.empty()){
		PrioAstar cur = pqAstar.top();
		pqAstar.pop();
		Visited[cur.x][cur.y] = 1;

		if (Furthest.x == (int)(cur.x / 4) && Furthest.y == (int)(cur.y / 4)){
			return true;
		}
		int Xm[4] = { -1, 1, 0, 0 };
		int Ym[4] = { 0, 0, -1, 1 };
		for (int i = 0; i<4; i++){
			int newX = (cur.x + Xm[i]) / 4;
			int newY = (cur.y + Ym[i]) / 4;
			int realX = cur.x + Xm[i];
			int realY = cur.y + Ym[i];
			if (Visited[realX][realY] == 1){ //already visited
				continue;
			}
			if (newX < 0 || newX == mapW || newY < 0 || newY == mapH){//bound check
				continue;
			}
			//check 16x16 bound
			if (newX < TopLeft.x - 1 || newX > BottomRight.x + 1 || newY < TopLeft.y - 1 || newY > BottomRight.y + 1){//bound check
				continue;
			}
			if (Broodwar->isWalkable(realX, realY)){
				//Check if there is a building
				bool Walkable = false;
				if (WallMap[newX][newY] <= 1 || WallMap[newX][newY] == Ignore){
					Walkable = true;
				}
				//if there is a building, check if you can walk between the gap
				if (Walkable == false){
					int Hgap, Vgap;
					MaxGap(realX, realY, Hgap, Vgap);
					if (Hgap >= unitH && Vgap >= unitW){
						Walkable = true;
					}
				}
				if (Walkable == true){ //can add to prior queue
					PrioAstar newTile;
					newTile.x = realX;
					newTile.y = realY;
					int newTileX = (int)(newTile.x / 4);
					int newTileY = (int)(newTile.y / 4);
					newTile.ManDist = abs(newTileX - Furthest.x) + abs(newTileY - Furthest.y);
					pqAstar.push(newTile);
				}
			}
		}
	}

	//tile not reachable, so no wall
	return false;
}

bool WallGenerator::CanWall(int BuildType, int xPos, int yPos){
	bool canPlace = true;
	bool NextToB = false;//Check if it is next to a building
	for (int x = xPos; x<xPos + SpaceArray[BuildType].width; x++){
		for (int y = yPos; y<yPos + SpaceArray[BuildType].heigth; y++){
			if ((x >= mapW) || (y >= mapH) || (x < 0) || (y < 0)){
				return false;//out of map
			}
			if (WallMap[x][y] != BUILDABLE){
				return false;
			}
			// Do not build over the end goal,otherwise it cannot be reached
			if (Furthest.x == x && Furthest.y == y){
				return false;
			}
			// Do not build over the start goal,otherwise it cannot be reached
			if (Closest.x == x && Closest.y == y){
				return false;
			}

			//Only build on highest ground
			if (useHighGround && Broodwar->getGroundHeight(x, y) != MaxHighGround){
				return false;
			}

			if (BuildType == 2){//Barracks gets put down first
				NextToB = true;//
			}
			//Check if thr building is next to another building
			int dx[8] = { 1, -1, 0, 0, 1, -1, 1, -1 };
			int dy[8] = { 0, 0, 1, -1, 1, -1, -1, 1 };
			for (int i = 0; i<8; i++){
				int newX = x + dx[i];
				int newY = y + dy[i];
				if ((newX < 0) || (newX >= mapW) || (newY < 0) || (newY >= mapH)){
					continue;
				}
				if (WallMap[newX][newY] > 1 && WallMap[newX][newY] != BuildType){
					NextToB = true;
				}
			}
		}
	}
	return NextToB;
}

void WallGenerator::mapWall(int xPos, int yPos, int BuildType, int place){
	for (int x = xPos; x < xPos + SpaceArray[BuildType].width; x++){
		for (int y = yPos; y < yPos + SpaceArray[BuildType].heigth; y++){
			WallMap[x][y] = place;
		}
	}
}