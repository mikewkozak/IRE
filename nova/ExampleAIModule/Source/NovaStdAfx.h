#pragma message("Compiling precompiled headers (you should see this only once)")
// #pragma once

#define WIN32_LEAN_AND_MEAN	// exclude rarely-used stuff from Windows headers
#define NOMINMAX			// avoid collisions with min() max()

#ifdef LOG4CXX_STATIC
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"
#else
namespace log4cxx
{
	struct LoggerPtr{ LoggerPtr(bool dummy){} LoggerPtr(){} };
	namespace Logger {
		const bool getLogger(char* dummy);
	}
}
#define LOG4CXX_TRACE(Logfile, Message) 0
#define LOG4CXX_ERROR(Logfile, Message) 0
#define LOG4CXX_FATAL(Logfile, Message) 0
#define LOG4CXX_DEBUG(Logfile, Message) 0
#define LOG4CXX_WARN(Logfile, Message) 0
#define LOG4CXX_INFO(Logfile, Message) 0
#endif

// standard includes
#include <windows.h>
#include <algorithm>
#include <list>
#include <iostream>
#include <iomanip>
#include <limits>
#include <fstream>
#include <cstdint>
#include <ctime>
#include <random>

// 3rd party libraries
#include <BWAPI.h>
#include <BWTA.h>
#ifdef NOVA_GUI
#include <QtGui> // including <QtGui> saves us to include every class user, <QString>, <QFileDialog>,...
#endif

#include "Utils/Timer.h"
#include "Utils/Statistic.h"

// Custom typedef and struct
// ==========================================
typedef std::set<BWAPI::TilePosition> TilePositionSet; //TODO use BWAPI::VectorSet??
struct HighLevelStats_t {
	std::map<size_t, Statistic> groupTime;
	std::map<size_t, Statistic> groupBranching;
// 	std::map<size_t, double> groupBranchingMin;
// 	std::map<size_t, double> groupBranchingMax;
// 	std::map<size_t, double> groupBranchingAvg;
	std::map<size_t, size_t> groupFrequency;
	std::map<size_t, size_t> groupTimeouts;
	std::map<size_t, size_t> groupDownSamplings;
	unsigned int orders;
	unsigned int ordersOverwritten;
	std::map<unsigned int, unsigned long> stateChange;
	unsigned int lastFrameStateChange;
};

// FILE LOG
// ==========================================
extern std::ofstream fileLog;
#define DEBUG(Message) fileLog << __FILE__ ":" << __LINE__ << ": " << Message << std::endl
#define LOG(Message) fileLog << Message << std::endl

// A "promise" of classes that we will have
// ==========================================
class InformationManager;
class WorkerManager;
class SquadManager;
class BuildManager;
class WallGenerator;
class ProductionManager;
class UnitInfoStatic;

// A "promise" of global variables
// ==========================================
#ifdef NOVA_GUI
class myQtApp;
extern QApplication QtApp;
extern myQtApp* novaGUI;
#endif

extern std::mt19937 gen; // random number generator

extern InformationManager* informationManager;
extern WorkerManager* workerManager;
extern SquadManager* squadManager;
extern BuildManager* buildManager;
extern WallGenerator* wallGenerator;
extern ProductionManager* productionManager;
extern UnitInfoStatic* unitStatic;

extern bool ONLY_MICRO;

// Broodwar GUI flags
extern bool PRINT_BUILD_ORDER;
extern bool PRINT_AIR_DPS;
extern bool PRINT_GROUND_DPS;
extern bool PRINT_PRODUCTION;
extern bool PRINT_BUILD_MAP;
extern bool PRINT_REGION_ID_MAP;
extern bool PRINT_SIEGE_MAP;

extern bool HIGH_LEVEL_SEARCH;
extern std::string SEARCH_ALGORITHM;
extern int HIGH_LEVEL_REFRESH; //in game frames

extern bool usingCloackUnits;
extern bool kitingFrame;
extern unsigned int combatsSimulated;

extern HighLevelStats_t stats;
extern const bool HighLevelChangeRateExperiment;

extern std::string configPath;

// Global functions
// ==========================================
BWAPI::Position rotatePosition(int degree, BWAPI::Position position, BWAPI::Position origen);
BWAPI::Position getPositionInDirection(BWAPI::Position origen, BWAPI::Position direction, int distance);
std::vector<std::string> &splitString(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> splitString(const std::string &s, char delim);
std::string intToString(const int& number); // TODO we don't need this with C++11
// TODO make this way to check canAttackAir standard
bool canAttackAirUnits(BWAPI::UnitType unitType);
bool canAttackGroundUnits(BWAPI::UnitType unitType);
bool canAttackType(BWAPI::UnitType unitTypeAttacking, BWAPI::UnitType unitTypeTarget);
std::string LoadConfigString(const char *pszKey, const char *pszItem, const char *pszDefault = NULL);
int LoadConfigInt(const char *pszKey, const char *pszItem, const int iDefault = 0);
bool AlmostEqualRelative(float A, float B, float maxRelDiff = FLT_EPSILON);
bool AlmostEqualRelative(double A, double B, double maxRelDiff = DBL_EPSILON);
