#pragma once

#include "comutil.h" // for _bstr_t (string conversions)
#include "CombatInfo.h"
#include "UnitInfoStatic.h"

// TODO killed units don't count their DPF (i.e. the survivor's DPF are overestimated)
// TODO loaded units shouldn't count to enemy DPF (neither their HP)
// Damage done split equally to all unit types (even if they don't participate in the combat)
// DPF calculated wrong when we have units which don't take part into the combat. Example: 1 marine + 8 SCV vs 1 SCV
// Spider mines damage added to Vultures?
// We don't take into account Medics (they can heal) (SCVs can repair)
// If the unit cannot attack air or ground (bunker, spell caster, transporters!!!, ...) we consider that can attack both types
// TODO transporter shouldn't be considered as "can attack both types" (see previous comment)
// Check "Damage done without units". Probably friend damage

class UnitInfoLearner
{
public:
	struct stats {
		double totalDamage;
		int totalTime;
		int numCombats;
		stats() : totalDamage(0.0), totalTime(0), numCombats(0) {}
	};

	struct bordaCountFrequency {
		int score;
		int frequency;
		bordaCountFrequency() : score(0), frequency(0){}
	};

	enum Matchups { TVT, TVP, TVZ, PVP, PVZ, ZVZ, NONE };
	struct DPFstats {
		int noCases;
		int size;
		DPFstats() : noCases(0), size(0){}
	};

	std::vector<std::vector<stats> > unitTypeStats;
	std::vector<std::vector<double> > unitTypeDPFbwapi;
	std::vector<std::vector<double> > unitTypeDPF;
	std::vector<DPF_t> unitDPF;
	std::vector<combatInfo> allCombats;
	
	enum GroupDiversity { AIR, GROUND, BOTH};
	std::vector<bordaCountFrequency> bordaCount;
	std::vector<bordaCountFrequency> bordaCountGround;
	std::vector<bordaCountFrequency> bordaCountAir;
	std::vector<bordaCountFrequency> bordaCountBoth;
	std::vector<float> typePriority;
	std::vector<float> typePriorityGround;
	std::vector<float> typePriorityAir;
	std::vector<float> typePriorityBoth;

	int combatsProcessed;
	std::map<Matchups, DPFstats> DPFcases;
	std::map<Matchups, int> DPFbounded;

	int armyDestroyed;
	int armyReinforcement;
	int armyPeace;
	int armyGameEnd;
	// corrupted games
	int moreThanTwoArmies;
	int armyKilledWithoutKills;
	int passiveArmy;
	int corruptedCombats;

	UnitInfoLearner(std::vector<std::vector<double> > unitTypeDPFstatic);

	void parseReplayFile(std::string replayPath);
	void combatsSanityCheck();
	void calculateDPF(bool skipTransports, bool onlyOneType);
	void damageUpperBound();
	void learnTargetBordaCount();
	void learnTrainingSet(std::vector<size_t> trainingIndices);
	// TODO we need a method to save and load learned variables instead of learn it each time
	void clear();
	std::string getMatchupName(Matchups matchId);
	const std::string toString(combatInfo combat);
	const std::string toString(unitInfo unit);
	GroupDiversity getGroupDiversity(const std::map<BWAPI::UnitType, int> &typesSize);

private:
	enum ParserState { COMBAT_START, ARMIES, KILLS, UNITS_NOT_PARTICIPATED };
	
	std::map<std::string, BWAPI::UnitType> getUnitTypeID;

	void unitKilled(unitKilledInfo killed, int lastFrame, std::map<int, unitInfo> armyUnits,
		int sizeArmyCanAttackGround, int sizeArmyCanAttackAir, int sizeArmyCanAttackBoth,
		std::map<BWAPI::UnitType, int> armySizeAttacking, std::map<BWAPI::UnitType, int> &armySizeDefending,
		int &sizeArmyCanAttackGround2, int &sizeArmyCanAttackAir2, int &sizeArmyCanAttackBoth2);
	Matchups getMatchupType(BWAPI::UnitType type1, BWAPI::UnitType type2);
	void parseCombat(combatInfo &combat, bool skipTransports = false, bool onlyOneType = false);
	void processDPF();
	bool hasUnwantedUnitTypes(const std::map<BWAPI::UnitType, int> &typesSize);
	void calculateDPF(std::vector<size_t> trainingIndices);
	void gatherBordaCountStats();
	void gatherBordaCountStats(std::vector<size_t> trainingIndices);
	void calculateAvgBordaCount();
	void printBordaCount();
};
