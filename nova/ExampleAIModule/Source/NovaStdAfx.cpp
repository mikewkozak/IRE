// #include "NovaStdAfx.h" // this is auto-included

#ifndef LOG4CXX_STATIC
namespace log4cxx {
	namespace Logger { const bool getLogger(char* dummy) { return true; } }
}
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// random generator
std::random_device rd; // seed
std::mt19937 gen(rd());

std::ofstream fileLog;

InformationManager* informationManager;
WorkerManager* workerManager;
SquadManager* squadManager;
BuildManager* buildManager;
WallGenerator* wallGenerator;
ProductionManager* productionManager;
UnitInfoStatic* unitStatic;

bool ONLY_MICRO;
bool PRINT_BUILD_ORDER = false;
bool PRINT_AIR_DPS = false;
bool PRINT_GROUND_DPS = false;
bool PRINT_PRODUCTION = false;
bool PRINT_BUILD_MAP = false;
bool PRINT_REGION_ID_MAP = false;
bool PRINT_SIEGE_MAP = false;

bool HIGH_LEVEL_SEARCH = false;
std::string SEARCH_ALGORITHM = "ABCD";

bool usingCloackUnits = false;
bool kitingFrame;
unsigned int combatsSimulated = 0;

HighLevelStats_t stats;
const bool HighLevelChangeRateExperiment = false;
int HIGH_LEVEL_REFRESH = 400;

BWAPI::Position rotatePosition(int degree, BWAPI::Position position, BWAPI::Position origen)
{
	//BWAPI::Positon finalPosition;
	double radians = degree * M_PI / 180;
	// subtract origin
	int xT1 = position.x - origen.x;
	int yT1 = position.y - origen.y;
	// rotate
	int xRotated = (int)((xT1*cos(radians)) - (yT1*sin(radians)));
	int yRotated = (int)((yT1*cos(radians)) - (xT1*sin(radians)));
	// back to origin
	int xT2 = xRotated + origen.x;
	int yT2 = yRotated + origen.y;
	return BWAPI::Position(xT2, yT2);
}

BWAPI::Position getPositionInDirection(BWAPI::Position origen, BWAPI::Position direction, int distance)
{
	// get the angle of the direction
	// 	double angle = atan2((double)(direction.y - origen.y), (double)(direction.x - origen.x));
	// 	// move the point with the given distance and direction
	// 	int newX = (int)(origen.x + cos(angle) * distance);
	// 	int newY = (int)(origen.y + sin(angle) * distance);

	// get unitary vector on right direction
	double dx = (double)(direction.x - origen.x);
	double dy = (double)(direction.y - origen.y);
	double norma = sqrt(dx*dx + dy*dy);
	dx /= norma;
	dy /= norma;
	// scale vector with "distance" and add to origin:
	int newX = (int)(origen.x + dx * distance);
	int newY = (int)(origen.y + dy * distance);

	return BWAPI::Position(newX, newY);
}

std::vector<std::string> &splitString(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


std::vector<std::string> splitString(const std::string &s, char delim) {
	std::vector<std::string> elems;
	splitString(s, delim, elems);
	return elems;
}

std::string intToString(const int& number)
{
	std::ostringstream oss;
	oss << number;
	return oss.str();
}

// some spellcaster abilities don't deal damage
bool isAggressiveSpellcaster(BWAPI::UnitType unitType)
{
	return unitType.isSpellcaster() &&
		unitType != BWAPI::UnitTypes::Terran_Medic &&
		unitType != BWAPI::UnitTypes::Terran_Science_Vessel &&
		unitType != BWAPI::UnitTypes::Protoss_Dark_Archon && // although, Feedback spell damage shielded units
		unitType != BWAPI::UnitTypes::Zerg_Queen; // although, Spawn Broodlings spell insta-kill a unit
}

bool canAttackAirUnits(BWAPI::UnitType unitType)
{
	return unitType.airWeapon() != BWAPI::WeaponTypes::None ||
		isAggressiveSpellcaster(unitType) ||
		unitType == BWAPI::UnitTypes::Terran_Bunker;
}

bool canAttackGroundUnits(BWAPI::UnitType unitType)
{
	return unitType.groundWeapon() != BWAPI::WeaponTypes::None ||
		isAggressiveSpellcaster(unitType) ||
		unitType == BWAPI::UnitTypes::Terran_Bunker;
}

bool canAttackType(BWAPI::UnitType unitTypeAttacking, BWAPI::UnitType unitTypeTarget)
{
	if (unitTypeTarget.isFlyer()) return canAttackAirUnits(unitTypeAttacking);
	else return canAttackGroundUnits(unitTypeAttacking);
}

//----------------------------- LOAD CONFIG FXNS ------------------------------------------
std::string configPath;
std::string LoadConfigString(const char *pszKey, const char *pszItem, const char *pszDefault)
{
	char buffer[MAX_PATH];
	GetPrivateProfileString(pszKey, pszItem, pszDefault ? pszDefault : "", buffer, MAX_PATH, configPath.c_str());
	return std::string(buffer);
}
int LoadConfigInt(const char *pszKey, const char *pszItem, const int iDefault)
{
	return GetPrivateProfileInt(pszKey, pszItem, iDefault, configPath.c_str());
}

bool AlmostEqualRelative(float A, float B, float maxRelDiff)
{
	return (std::abs(A - B) <= std::max(std::abs(A), std::abs(B)) * maxRelDiff);
}

bool AlmostEqualRelative(double A, double B, double maxRelDiff)
{
	return (std::abs(A - B) <= std::max(std::abs(A), std::abs(B)) * maxRelDiff);
}