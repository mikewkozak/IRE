/*
StrategyReader.h
This class handles reading the strategies from file and storing them for future access during runtime.
*/

//for getting all strategy files in directory
#include <boost/filesystem.hpp>
#include <vector>

//Graph structure and strategy axes
#include "StrategySpace.h"
#include "Utils/GraphUtils.h"

#pragma once
/*
Class responsible for reading in DOT strategy files and converting them to Strategy structs
*/
class StrategyReader
{
public:
	StrategyReader();
	~StrategyReader();

	/*
	Reads in the list of Terran Strategies and stores them for future use
	*/
	void getTerranStrategies();

	/*
	Function that generates terran strategies and writes them off to DOT files for future reference
	*/
	std::vector<Strategy> buildTerranStrategies();

	/*
	Function that generates Zerg strategies and writes them off to DOT files for future reference
	*/
	std::vector<Strategy> buildZergStrategies();

	/*
	Function that generates Protoss strategies and writes them off to DOT files for future reference
	*/
	std::vector<Strategy> buildProtossStrategies();

private:
	//Path to the Terran Strategy Files
	static const std::string TERRAN_STRATEGY_PATH;

	//Path to the Protoss Strategy Files
	static const std::string PROTOSS_STRATEGY_PATH;

	//Path to the Zerg Strategy Files
	static const std::string ZERG_STRATEGY_PATH;

	//Strategy building support functions for readability
	//Terran Strategies
	Strategy buildBio();
	Strategy buildRaxFe();
	Strategy buildTwoFacto();
	Strategy buildVultures();
	Strategy buildAir();
	Strategy buildDrop();

	//Protoss Strategies
	Strategy buildTwoGates();
	Strategy buildFastDT();
	Strategy buildTemplar();
	Strategy buildSpeedzeal();
	Strategy buildCorsair();
	Strategy buildNony();
	Strategy buildReaverDrop();

	//Zerg Strategies
	Strategy buildSpeedlings();
	Strategy buildFastMutas();
	Strategy buildMutas();
	Strategy buildLurkers();
	Strategy buildHydras();
};

