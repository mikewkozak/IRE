#include "UnitInfoLearner.h"

UnitInfoLearner::UnitInfoLearner(std::vector<std::vector<double> > unitTypeDPFstatic)
	:armyDestroyed(0),
	armyReinforcement(0),
	armyPeace(0),
	armyGameEnd(0),
	moreThanTwoArmies(0),
	armyKilledWithoutKills(0),
	passiveArmy(0),
	corruptedCombats(0),
	unitTypeDPFbwapi(unitTypeDPFstatic)
{
	// first we need to populate the structure to get the UnitType enum from string
	for (auto unitType : BWAPI::UnitTypes::allUnitTypes()) {
		getUnitTypeID[unitType.c_str()] = unitType;
	}

	// init DPF
	unitTypeDPF.resize(MAX_UNIT_TYPE);
	unitTypeStats.resize(MAX_UNIT_TYPE);
	for (int i = 0; i < MAX_UNIT_TYPE; ++i) {
		unitTypeDPF[i].resize(MAX_UNIT_TYPE);
		unitTypeStats[i].resize(MAX_UNIT_TYPE);
	}
	unitDPF.resize(MAX_UNIT_TYPE);

	// init priority
	bordaCount.resize(MAX_UNIT_TYPE);
	bordaCountAir.resize(MAX_UNIT_TYPE);
	bordaCountGround.resize(MAX_UNIT_TYPE);
	bordaCountBoth.resize(MAX_UNIT_TYPE);
	typePriority.resize(MAX_UNIT_TYPE);
	typePriorityAir.resize(MAX_UNIT_TYPE);
	typePriorityGround.resize(MAX_UNIT_TYPE);
	typePriorityBoth.resize(MAX_UNIT_TYPE);
}


void UnitInfoLearner::parseReplayFile(std::string replayPath)
{
	std::ifstream currentFile;
	std::string line;
	std::stringstream stream;
	std::streampos pos;

	combatInfo newCombat;
	int parserState = ParserState::COMBAT_START;
	std::string dummy;
	bool army1;
	bool combatStart;
	int lineNumber;

	currentFile.open(replayPath.c_str());
	lineNumber = 0;
	while (std::getline(currentFile, line)) {
		lineNumber++;
// 		LOG(line.c_str());
		if (parserState == ParserState::COMBAT_START) {
			if (line.find("ARMY_DESTROYED") != std::string::npos) {
				armyDestroyed++;
				// clear newCombat
				newCombat.armySize1.clear();
				newCombat.armySize2.clear();
				newCombat.armyUnits1.clear();
				newCombat.armyUnitsEnd1.clear();
				newCombat.armyUnits2.clear();
				newCombat.armyUnitsEnd2.clear();
				newCombat.kills.clear();

// 				LOG(line.c_str());
				// get start and end frames
				stream << line;
				std::getline(stream, dummy, ','); // NEW_COMBAT
				std::getline(stream, dummy, ','); newCombat.startFrame = atoi(dummy.c_str());
				std::getline(stream, dummy, ','); newCombat.endFrame = atoi(dummy.c_str());
// 				LOG("start: " << startFrame << " end: " << endFrame);
				stream.str(std::string()); // reset the stream
				stream.clear();
				parserState = ParserState::ARMIES;
				army1 = true;
				combatStart = true;
// 				LOG("--------- Army 1 START");
			} else if (line.find("REINFORCEMENT") != std::string::npos) {
				armyReinforcement++;
			} else if (line.find("PEACE") != std::string::npos) {
				armyPeace++;
			} else if (line.find("GAME_END") != std::string::npos) {
// 				LOG("Army end found it at line " << lineNumber);
				armyGameEnd++;
			}
		} else if (parserState == ParserState::ARMIES) {
			if (line.find("ARMY_START") != std::string::npos) {
				while (std::getline(currentFile, line)) {
					lineNumber++;
					if (line.find("ARMY") == std::string::npos
						&& line.find("KILLS") == std::string::npos
						&& line.find("NEW_COMBAT") == std::string::npos) {
						// collect unit data
// 						LOG(line.c_str());
						stream << line;
						unitInfo unit;
						std::getline(stream, dummy, ','); unit.ID = atoi(dummy.c_str());
						std::getline(stream, unit.typeName, ',');
						std::getline(stream, dummy, ','); unit.x = atoi(dummy.c_str());
						std::getline(stream, dummy, ','); unit.y = atoi(dummy.c_str());
						std::getline(stream, dummy, ','); unit.HP = atoi(dummy.c_str());
						std::getline(stream, dummy, ','); unit.shield = atoi(dummy.c_str());
						std::getline(stream, dummy, ','); // energy
						stream.str(std::string()); // reset the stream
						stream.clear();
						unit.typeID = getUnitTypeID[unit.typeName];
						if (unit.typeID > BWAPI::UnitTypes::AllUnits) {
							LOG("Unkwon unit type: " << unit.typeID << " " << unit.typeName);
						}
						if (army1) {
							if (combatStart) {
								newCombat.armyUnits1[unit.ID] = unit;
								newCombat.armySize1[unit.typeID]++;
							} else newCombat.armyUnitsEnd1[unit.ID] = unit;
						} else {
							if (combatStart) {
								newCombat.armyUnits2[unit.ID] = unit;
								newCombat.armySize2[unit.typeID]++;
							} else newCombat.armyUnitsEnd2[unit.ID] = unit;
						}
					} else { // new army
						if (army1) {
							army1 = false;
// 							LOG("--------- Army 2");
						} else if (combatStart) {
							army1 = true;
							combatStart = false;
// 							LOG("--------- Army 1 END");
						} else {
							// sometimes we do not have kills (Scanner sweep)
							if (line.find("NEW_COMBAT") != std::string::npos) {
// 								LOG("--------- END COMBAT");
								armyKilledWithoutKills++;
								parserState = ParserState::COMBAT_START;
								// sometimes we have more than two armies
							} else if (line.find("ARMY") != std::string::npos) {
// 								LOG("--------- FOUND A THIRD ARMY");
								moreThanTwoArmies++;
								// look until the next NEW_COMBAT
								while (std::getline(currentFile, line)) {
									lineNumber++;
									if (line.find("NEW_COMBAT") != std::string::npos) break;
								}
								parserState = ParserState::COMBAT_START;
							} else if (line.find("KILLS") != std::string::npos) {
// 								LOG("--------- KILLS");
								parserState = ParserState::KILLS;
							} else if (line.find("UNITS_NOT_PARTICIPATED") != std::string::npos) {
// 								LOG("--------- UNITS_NOT_PARTICIPATED");
								parserState = ParserState::UNITS_NOT_PARTICIPATED;
							} else {
								DEBUG("Unexpected combat log line:" << line);
							}
							// move line pointer to previous line
							currentFile.seekg(pos, std::ios::beg);
							lineNumber--;
							break;
						}
					}
					pos = currentFile.tellg();
				}
			}
		} else if (parserState == ParserState::KILLS) {
			while (std::getline(currentFile, line)) {
				lineNumber++;
				if (line.find("UNITS_NOT_PARTICIPATED") == std::string::npos){
// 					LOG(line.c_str());
					unitKilledInfo kill;
					stream << line;
					std::getline(stream, dummy, ','); kill.unitID = std::stoi(dummy);
					std::getline(stream, dummy, ','); kill.frame = std::stoi(dummy);
					stream.str(std::string()); stream.clear(); // reset the stream
					// check what army belongs
					if (newCombat.armyUnits1.find(kill.unitID) != newCombat.armyUnits1.end()) kill.armyID = 1;
					else if (newCombat.armyUnits2.find(kill.unitID) != newCombat.armyUnits2.end()) kill.armyID = 2;
					else DEBUG("Killed not present in combat");
					newCombat.kills.push_back(kill);
				} else {
// 					LOG("--------- UNITS_NOT_PARTICIPATED");
					parserState = ParserState::UNITS_NOT_PARTICIPATED;
					// move line pointer to previous line
					currentFile.seekg(pos, std::ios::beg);
					lineNumber--;
					break;
				}
				pos = currentFile.tellg();
			}
		} else if (parserState == ParserState::UNITS_NOT_PARTICIPATED) {
			std::vector<int> passiveUnits;
			while (std::getline(currentFile, line)) {
				lineNumber++;
				if (line.find("NEW_COMBAT") == std::string::npos){
// 					LOG(line.c_str());
					std::stringstream lineStream(line);
					std::string item;
					while (std::getline(lineStream, item, ',')) {
						if (item == "\r" || item == "\n" || item == "\r\n") continue; // skip new line character
						int val = std::stoi(item);
						passiveUnits.push_back(val);
					}
				} else {
// 					LOG("--------- END COMBAT");
					// remove passive Units from combat
					for (const auto& unitId : passiveUnits) {
						auto unitFound = newCombat.armyUnits1.find(unitId);
						if (unitFound != newCombat.armyUnits1.end()) {
							if (newCombat.armySize1[(*unitFound).second.typeID] == 0) DEBUG("This should not happen");
							newCombat.armySize1[(*unitFound).second.typeID]--;
							if (newCombat.armySize1[(*unitFound).second.typeID] == 0) newCombat.armySize1.erase((*unitFound).second.typeID);
							newCombat.armyUnits1.erase(unitFound);
						}
						auto unitFound2 = newCombat.armyUnits2.find(unitId);
						if (unitFound2 != newCombat.armyUnits2.end()) {
							if (newCombat.armySize2[(*unitFound2).second.typeID] == 0) DEBUG("This should not happen");
							newCombat.armySize2[(*unitFound2).second.typeID]--;
							if (newCombat.armySize2[(*unitFound2).second.typeID] == 0) newCombat.armySize2.erase((*unitFound2).second.typeID);
							newCombat.armyUnits2.erase(unitFound2);
						}
						auto unitFound3 = newCombat.armyUnitsEnd1.find(unitId);
						if (unitFound3 != newCombat.armyUnitsEnd1.end()) newCombat.armyUnitsEnd1.erase(unitFound3);
						auto unitFound4 = newCombat.armyUnitsEnd2.find(unitId);
						if (unitFound4 != newCombat.armyUnitsEnd2.end()) newCombat.armyUnitsEnd2.erase(unitFound4);
					}
					// remove passive Units from kills
// 					for (auto it = newCombat.kills.begin(); it != newCombat.kills.end();) {
// 						if (std::find(passiveUnits.begin(), passiveUnits.end(), (*it).unitID) != passiveUnits.end()) {
// 							it = newCombat.kills.erase(it);
// 						} else {
// 							++it;
// 						}
// 					}

					if (newCombat.armyUnits1.empty() || newCombat.armyUnits2.empty()) {
						passiveArmy++;
					} else {
						allCombats.push_back(newCombat);
					}

					// start next combat
					parserState = ParserState::COMBAT_START;
					// move line pointer to previous line
					currentFile.seekg(pos, std::ios::beg);
					lineNumber--;
					break;
				}
				pos = currentFile.tellg();
			}
		}
	}
	currentFile.close();
// 	LOG("Finished parsing files");
}

// clean corrupted combats
void UnitInfoLearner::combatsSanityCheck()
{
	bool corrupted;
	for (auto it = allCombats.begin(); it != allCombats.end();) {
		corrupted = false;
		// check if a unit was killed by an "empty" army
		// =============================================
		// count the total units in the army depends on target type
		int sizeArmy1canAttackGround, sizeArmy1canAttackAir, sizeArmy1canAttackBoth;
		int sizeArmy2canAttackGround, sizeArmy2canAttackAir, sizeArmy2canAttackBoth;
		sizeArmy1canAttackGround = sizeArmy1canAttackAir = sizeArmy1canAttackBoth = 0;
		sizeArmy2canAttackGround = sizeArmy2canAttackAir = sizeArmy2canAttackBoth = 0;
		for (const auto& army : it->armySize1) {
			if (canAttackAirUnits(army.first)) {
				if (canAttackGroundUnits(army.first)) sizeArmy1canAttackBoth += army.second;
				else sizeArmy1canAttackAir += army.second;
			} else if (canAttackGroundUnits(army.first)) sizeArmy1canAttackGround += army.second;
			// if the unit cannot attack air or ground (bunker, spell caster, ...) consider attack type both
			else sizeArmy1canAttackBoth += army.second;
		}
		for (const auto& army : it->armySize2) {
			if (canAttackAirUnits(army.first)) {
				if (canAttackGroundUnits(army.first)) sizeArmy2canAttackBoth += army.second;
				else sizeArmy2canAttackAir += army.second;
			} else if (canAttackGroundUnits(army.first)) sizeArmy2canAttackGround += army.second;
			// if the unit cannot attack air or ground (bunker, spell caster, ...) consider attack type both
			else sizeArmy2canAttackBoth += army.second;
		}

		for (const auto& killInfo : it->kills) {
			if (killInfo.armyID == 1 && it->armyUnits1.find(killInfo.unitID) != it->armyUnits1.end()) {
				int unitTypeIdKilled = it->armyUnits1[killInfo.unitID].typeID;
				// calculate how many units where able to attack the killed unit
				BWAPI::UnitType unitTypeKilled(unitTypeIdKilled);
				double totalUnitsAttacking = sizeArmy2canAttackBoth;
				if (unitTypeKilled.isFlyer()) totalUnitsAttacking += sizeArmy2canAttackAir;
				else totalUnitsAttacking += sizeArmy2canAttackGround;
				if (totalUnitsAttacking == 0) { corrupted = true; break; }
				else { // remove lost
					if (canAttackAirUnits(unitTypeKilled)) {
						if (canAttackGroundUnits(unitTypeKilled)) --sizeArmy1canAttackBoth;
						else --sizeArmy1canAttackAir;
					} else if (canAttackGroundUnits(unitTypeKilled)) --sizeArmy1canAttackGround;
				}
			}
			if (killInfo.armyID == 2 && it->armyUnits2.find(killInfo.unitID) != it->armyUnits2.end()) {
				int unitTypeIdKilled = it->armyUnits2[killInfo.unitID].typeID;
				// calculate how many units where able to attack the killed unit
				BWAPI::UnitType unitTypeKilled(unitTypeIdKilled);
				double totalUnitsAttacking = sizeArmy1canAttackBoth;
				if (unitTypeKilled.isFlyer()) totalUnitsAttacking += sizeArmy1canAttackAir;
				else totalUnitsAttacking += sizeArmy1canAttackGround;
				if (totalUnitsAttacking == 0) { corrupted = true; break; } else { // remove lost
					if (canAttackAirUnits(unitTypeKilled)) {
						if (canAttackGroundUnits(unitTypeKilled)) --sizeArmy2canAttackBoth;
						else --sizeArmy2canAttackAir;
					} else if (canAttackGroundUnits(unitTypeKilled)) --sizeArmy2canAttackGround;
				}
			}
		}

		// =============================================
		
		if (corrupted) {
			corruptedCombats++;
			it = allCombats.erase(it);
		} else {
			++it;
		}
	}
}

void UnitInfoLearner::calculateDPF(bool skipTransports, bool onlyOneType)
{
	LOG("Start calculating DPS from " << allCombats.size() << " combats, skip transports: " << skipTransports << " onleyOneType: " << onlyOneType );
	
	combatsProcessed = 0;
	for (auto& combat : allCombats) {
		parseCombat(combat, skipTransports, onlyOneType);
	}

	processDPF();
}

void UnitInfoLearner::calculateDPF(std::vector<size_t> trainingIndices)
{
	clear(); // be sure the learn stuff is clear
	combatsProcessed = 0;

	for (const auto& i : trainingIndices) {
		parseCombat(allCombats[i]);
	}

	processDPF();
}

void UnitInfoLearner::parseCombat(combatInfo &combat, bool skipTransports, bool onlyOneType)
{
	if (onlyOneType) {
		// we only process combats of 1 type vs 1 type
		if (combat.armySize1.size() > 1) return;
		if (combat.armySize2.size() > 1) return;
	}
	
	// feed stats from parser data
	int lastFrameArmy1, lastFrameArmy2;
	int sizeArmy1canAttackGround, sizeArmy1canAttackAir, sizeArmy1canAttackBoth;
	int sizeArmy2canAttackGround, sizeArmy2canAttackAir, sizeArmy2canAttackBoth;

	lastFrameArmy1 = combat.startFrame;
	lastFrameArmy2 = combat.startFrame;
	// count the total units in the army depends on target type
	sizeArmy1canAttackGround = sizeArmy1canAttackAir = sizeArmy1canAttackBoth = 0;
	sizeArmy2canAttackGround = sizeArmy2canAttackAir = sizeArmy2canAttackBoth = 0;

	// use a temporal armySizes
	std::map<BWAPI::UnitType, int> tmpArmySize1(combat.armySize1);
	std::map<BWAPI::UnitType, int> tmpArmySize2(combat.armySize2);
	
	for (auto army : tmpArmySize1) {
		if (skipTransports && army.first.spaceProvided() > 0) return;
		if (canAttackAirUnits(army.first)) {
			if (canAttackGroundUnits(army.first)) sizeArmy1canAttackBoth += army.second;
			else sizeArmy1canAttackAir += army.second;
		} else if (canAttackGroundUnits(army.first)) sizeArmy1canAttackGround += army.second;
		// if the unit cannot attack air or ground (bunker, spell caster, ...) consider attack type both
		else sizeArmy1canAttackBoth += army.second;
	}
	for (auto army : tmpArmySize2) {
		if (skipTransports && army.first.spaceProvided() > 0) return;
		if (canAttackAirUnits(army.first)) {
			if (canAttackGroundUnits(army.first)) sizeArmy2canAttackBoth += army.second;
			else sizeArmy2canAttackAir += army.second;
		} else if (canAttackGroundUnits(army.first)) sizeArmy2canAttackGround += army.second;
		// if the unit cannot attack air or ground (bunker, spell caster, ...) consider attack type both
		else sizeArmy2canAttackBoth += army.second;
	}

	// TODO we don't count the DPS of killed units (i.e. the survivor's DPS are overestimated)
	for (auto killed : combat.kills) {
		if (killed.armyID == 1) {
			if (combat.armyUnits1.find(killed.unitID) != combat.armyUnits1.end()) {
				// army1 unit killed
				unitKilled(killed, lastFrameArmy2, combat.armyUnits1,
					sizeArmy2canAttackGround, sizeArmy2canAttackAir, sizeArmy2canAttackBoth,
					tmpArmySize2, tmpArmySize1,
					sizeArmy1canAttackGround, sizeArmy1canAttackAir, sizeArmy1canAttackBoth);
			} // if not found, usually it's a passive unit
			lastFrameArmy2 = killed.frame;
		} else if (killed.armyID == 2) {
			if (combat.armyUnits2.find(killed.unitID) != combat.armyUnits2.end()) {
				// army2 unit killed
				unitKilled(killed, lastFrameArmy1, combat.armyUnits2,
					sizeArmy1canAttackGround, sizeArmy1canAttackAir, sizeArmy1canAttackBoth,
					tmpArmySize1, tmpArmySize2,
					sizeArmy2canAttackGround, sizeArmy2canAttackAir, sizeArmy2canAttackBoth);
			} // if not found, usually it's a passive unit
			lastFrameArmy1 = killed.frame;
		} else {
			DEBUG("Wrong armyID in kills list");
		}
	}
	++combatsProcessed;
}

void UnitInfoLearner::unitKilled(unitKilledInfo killed, int lastFrame, std::map<int, unitInfo> armyUnits,
	int sizeArmyCanAttackGround, int sizeArmyCanAttackAir, int sizeArmyCanAttackBoth,
	std::map<BWAPI::UnitType, int> armySizeAttacking, std::map<BWAPI::UnitType, int> &armySizeDefending,
	int &sizeArmyCanAttackGround2, int &sizeArmyCanAttackAir2, int &sizeArmyCanAttackBoth2)
{
	int framesToKill = killed.frame - lastFrame;
	
	int unitTypeIdKilled = armyUnits[killed.unitID].typeID;
	// calculate how many units where able to attack the killed unit
	BWAPI::UnitType unitTypeKilled(unitTypeIdKilled);
	double totalUnitsAttacking = sizeArmyCanAttackBoth;
	if (unitTypeKilled.isFlyer()) totalUnitsAttacking += sizeArmyCanAttackAir;
	else totalUnitsAttacking += sizeArmyCanAttackGround;
	if (totalUnitsAttacking == 0) {
		DEBUG("Damage done without units"); // usually this happens when a unit was killed from a unit "outside" the combat
		return;
	}
	// split total damage between the units
	double damageDone = armyUnits[killed.unitID].HP + armyUnits[killed.unitID].shield;
	double damageSplit = damageDone / totalUnitsAttacking;

	for (const auto& typeSize : armySizeAttacking) {
		if (typeSize.second <= 0) {
			DEBUG("we have less/or 0 units of type " << typeSize.first);
			continue;
		}
		if (typeSize.first > BWAPI::UnitTypes::AllUnits) {
			DEBUG("unknown unitType " << typeSize.first);
			continue;
		}
		// check if we can attack the target to apply damage
		BWAPI::UnitType unitType(typeSize.first);
		if ((unitTypeKilled.isFlyer() && canAttackAirUnits(unitType))
			|| (!unitTypeKilled.isFlyer() && canAttackGroundUnits(unitType))
			|| (!canAttackAirUnits(unitType) && !canAttackGroundUnits(unitType))) { // bunker, spell caster, transporter, ...
			double damageUsed = damageSplit * (double)typeSize.second;
			unitTypeStats[typeSize.first][unitTypeIdKilled].totalDamage += damageUsed;
			unitTypeStats[typeSize.first][unitTypeIdKilled].totalTime += framesToKill;
			unitTypeStats[typeSize.first][unitTypeIdKilled].numCombats++;
			// Asserts
			if (unitTypeStats[typeSize.first][unitTypeIdKilled].totalDamage < 0)
				DEBUG("Damage Overflow! unitKilledID " << killed.unitID << " frame: " << killed.frame);
			if (unitTypeStats[typeSize.first][unitTypeIdKilled].totalTime < 0)
				DEBUG("Frames Overflow! unitKilledID " << killed.unitID << " frame: " << killed.frame);

		}
	}
	// remove lost
	armySizeDefending[unitTypeIdKilled]--;
	if (armySizeDefending[unitTypeIdKilled] == 0) armySizeDefending.erase(unitTypeIdKilled);
	if (canAttackAirUnits(unitTypeKilled)) {
		if (canAttackGroundUnits(unitTypeKilled)) --sizeArmyCanAttackBoth2;
		else --sizeArmyCanAttackAir2;
	} else if (canAttackGroundUnits(unitTypeKilled)) --sizeArmyCanAttackGround2;
}

bool isMilitarUnit(BWAPI::UnitType type)
{
	if (type.getID() > 163) return false;  // no interesting units farther this point
	if (type.isHero()) return false;
	if (type == BWAPI::UnitTypes::Terran_Comsat_Station) return false;
	return true;
}

void UnitInfoLearner::processDPF()
{
	// cleaning stats
	DPFcases.clear();
	for (int matchupInt = TVT; matchupInt != NONE; matchupInt++) {
		DPFcases.insert(std::pair<Matchups, DPFstats>(static_cast<Matchups>(matchupInt), DPFstats()));
	}

	for (auto unitType1 : BWAPI::UnitTypes::allUnitTypes()) {
		if (!isMilitarUnit(unitType1)) continue;
		if (unitType1.canAttack() || unitType1.isSpellcaster() || unitType1 == BWAPI::UnitTypes::Terran_Bunker) {
			DPF_t dpfByOneType;
			for (auto unitType2 : BWAPI::UnitTypes::allUnitTypes()) {
				if (!isMilitarUnit(unitType2)) continue;
				if (unitType2.canAttack() || unitType2.isSpellcaster() || unitType2.spaceProvided() > 0) {
					// identify the matchup
					Matchups vsType = getMatchupType(unitType1, unitType2);
					// update the DPF
					if (unitTypeStats[unitType1][unitType2].totalTime == 0) {
						if (unitTypeDPFbwapi[unitType1][unitType2] > 0) {
							DPFcases[vsType].size++;
							DPFcases[vsType].noCases++;
							unitTypeDPF[unitType1][unitType2] = unitTypeDPFbwapi[unitType1][unitType2];
// 							LOG("Missing " << unitType1.c_str() << " vs " << unitType2.c_str());
						}
					} else {
// 						LOG("Learned " << unitType1.c_str() << " vs " << unitType2.c_str());
						DPFcases[vsType].size++;
						unitTypeDPF[unitType1][unitType2] = unitTypeStats[unitType1][unitType2].totalDamage / (double)unitTypeStats[unitType1][unitType2].totalTime;

						// always get the min for dpfByOneType
						if (unitType1.airWeapon().damageAmount() > 0) {
							if (dpfByOneType.air == 0) dpfByOneType.air = unitTypeDPF[unitType1][unitType2];
							else dpfByOneType.air = std::min(dpfByOneType.air, unitTypeDPF[unitType1][unitType2]);
						}
						if (unitType1.groundWeapon().damageAmount() > 0) {
							if (dpfByOneType.ground == 0) dpfByOneType.ground = unitTypeDPF[unitType1][unitType2];
							else dpfByOneType.ground = std::min(dpfByOneType.ground, unitTypeDPF[unitType1][unitType2]);
						}
						if (unitType1.airWeapon().damageAmount() > 0 && unitType1.groundWeapon().damageAmount() > 0) {
							dpfByOneType.bothGround = dpfByOneType.ground;
							dpfByOneType.bothAir = dpfByOneType.air;
						}
					}
				}
			}
			unitDPF[unitType1] = dpfByOneType;
		}
	}
}

UnitInfoLearner::Matchups UnitInfoLearner::getMatchupType(BWAPI::UnitType type1, BWAPI::UnitType type2)
{
	if (type1.getRace() == BWAPI::Races::Terran && type2.getRace() == BWAPI::Races::Terran) {
		return TVT;
	}
	if ((type1.getRace() == BWAPI::Races::Terran && type2.getRace() == BWAPI::Races::Protoss) ||
		(type2.getRace() == BWAPI::Races::Terran && type1.getRace() == BWAPI::Races::Protoss)) {
		return TVP;
	}
	if ((type1.getRace() == BWAPI::Races::Terran && type2.getRace() == BWAPI::Races::Zerg) ||
		(type2.getRace() == BWAPI::Races::Terran && type1.getRace() == BWAPI::Races::Zerg)) {
		return TVZ;
	}
	if (type1.getRace() == BWAPI::Races::Protoss && type2.getRace() == BWAPI::Races::Protoss) {
		return PVP;
	}
	if ((type1.getRace() == BWAPI::Races::Protoss && type2.getRace() == BWAPI::Races::Zerg) ||
		(type2.getRace() == BWAPI::Races::Protoss && type1.getRace() == BWAPI::Races::Zerg)) {
		return PVZ;
	}
	if (type1.getRace() == BWAPI::Races::Zerg && type2.getRace() == BWAPI::Races::Zerg) {
		return PVP;
	}
	return UnitInfoLearner::Matchups::NONE;
}

std::string UnitInfoLearner::getMatchupName(Matchups matchId)
{
	switch (matchId) {
	case TVT: return "TVT"; break;
	case TVP: return "TVP"; break;
	case TVZ: return "TVZ"; break;
	case PVP: return "PVP"; break;
	case PVZ: return "PVZ"; break;
	case ZVZ: return "ZVZ"; break;
	default: return "NONE"; break;
	}
}

void UnitInfoLearner::damageUpperBound()
{
	// cleaning stats
	DPFbounded.clear();
	for (int matchupInt = TVT; matchupInt != NONE; matchupInt++) {
		DPFbounded.insert(std::pair<Matchups, int>(static_cast<Matchups>(matchupInt), 0));
	}

	for (auto unitType1 : BWAPI::UnitTypes::allUnitTypes()) {
		for (auto unitType2 : BWAPI::UnitTypes::allUnitTypes()) {
			if (unitTypeDPFbwapi[unitType1][unitType2] > 0 &&
				unitTypeDPFbwapi[unitType1][unitType2] < unitTypeDPF[unitType1][unitType2]) {
				unitTypeDPF[unitType1][unitType2] = unitTypeDPFbwapi[unitType1][unitType2];

				Matchups vsType = getMatchupType(unitType1, unitType2);
				DPFbounded[vsType]++;
			}
		}
	}
}

// computes type priority using Borda Count http://en.wikipedia.org/wiki/Borda_count
// we give points when first unit type is killed

void UnitInfoLearner::gatherBordaCountStats()
{
	bordaCount.clear();
	bordaCountAir.clear();
	bordaCountGround.clear();
	bordaCountBoth.clear();
	bordaCount.resize(MAX_UNIT_TYPE);
	bordaCountAir.resize(MAX_UNIT_TYPE);
	bordaCountGround.resize(MAX_UNIT_TYPE);
	bordaCountBoth.resize(MAX_UNIT_TYPE);

	for (auto combat : allCombats) {
		std::map<BWAPI::UnitType, int> typesToDestroy1(combat.armySize1);
		std::map<BWAPI::UnitType, int> typesToDestroy2(combat.armySize2);

		// check if we need to decide targets
		bool army1NeedsToDecideTarget = true;
		bool army2NeedsToDecideTarget = true;
		if (typesToDestroy1.size() < 2) army2NeedsToDecideTarget = false;
		if (typesToDestroy2.size() < 2) army1NeedsToDecideTarget = false;

		// if target is obvious skip it
		if (!army1NeedsToDecideTarget && !army2NeedsToDecideTarget) continue;

		// skip combats with mines or transports
		if (hasUnwantedUnitTypes(typesToDestroy1)) continue;
		if (hasUnwantedUnitTypes(typesToDestroy2)) continue;

		// get groupDiversity
		GroupDiversity groupDiversity1 = getGroupDiversity(combat.armySize1);
		GroupDiversity groupDiversity2 = getGroupDiversity(combat.armySize2);

		// give points when a new type is destroyed
		for (const auto& killedInfo : combat.kills) {
			if (killedInfo.armyID == 1 && combat.armyUnits1.find(killedInfo.unitID) != combat.armyUnits1.end()) {
				// unit killed is from army1
				if (army2NeedsToDecideTarget) {
					int unitTypeID = combat.armyUnits1[killedInfo.unitID].typeID;
					BWAPI::UnitType unitType(unitTypeID);
					// look if the type is still there to kill it
					if (typesToDestroy1.find(unitType) != typesToDestroy1.end()) {
						typesToDestroy1.erase(unitType);
						bordaCount[unitTypeID].score += typesToDestroy1.size();
						bordaCount[unitTypeID].frequency += 1;
						std::vector<bordaCountFrequency>* specificBorda;
						switch (groupDiversity2) {
						case UnitInfoLearner::AIR:	  specificBorda = &bordaCountAir; break;
						case UnitInfoLearner::GROUND: specificBorda = &bordaCountGround; break;
						case UnitInfoLearner::BOTH:	  specificBorda = &bordaCountBoth; break;
						}
						specificBorda->at(unitTypeID).score += typesToDestroy1.size();
						specificBorda->at(unitTypeID).frequency += 1;
						if (typesToDestroy1.size() == 0) army2NeedsToDecideTarget = false;
					}
				}
			} else if (killedInfo.armyID == 2 && combat.armyUnits2.find(killedInfo.unitID) != combat.armyUnits2.end()) {
				// unit killed is from army2
				if (army1NeedsToDecideTarget) {
					int unitTypeID = combat.armyUnits2[killedInfo.unitID].typeID;
					BWAPI::UnitType unitType(unitTypeID);
					// look if the type is still there to kill it
					if (typesToDestroy2.find(unitType) != typesToDestroy2.end()) {
						typesToDestroy2.erase(unitType);
						bordaCount[unitTypeID].score += typesToDestroy2.size();
						bordaCount[unitTypeID].frequency += 1;
						std::vector<bordaCountFrequency>* specificBorda;
						switch (groupDiversity1) {
						case UnitInfoLearner::AIR:	  specificBorda = &bordaCountAir; break;
						case UnitInfoLearner::GROUND: specificBorda = &bordaCountGround; break;
						case UnitInfoLearner::BOTH:	  specificBorda = &bordaCountBoth; break;
						}
						specificBorda->at(unitTypeID).score += typesToDestroy1.size();
						specificBorda->at(unitTypeID).frequency += 1;
						if (typesToDestroy2.size() == 0) army1NeedsToDecideTarget = false;
					}
				}
			}

			// keep giving points?
			if (!army1NeedsToDecideTarget && !army2NeedsToDecideTarget) break;
		}
	}
}

void UnitInfoLearner::gatherBordaCountStats(std::vector<size_t> trainingIndices)
{
	bordaCount.clear();
	bordaCountAir.clear();
	bordaCountGround.clear();
	bordaCountBoth.clear();
	bordaCount.resize(MAX_UNIT_TYPE);
	bordaCountAir.resize(MAX_UNIT_TYPE);
	bordaCountGround.resize(MAX_UNIT_TYPE);
	bordaCountBoth.resize(MAX_UNIT_TYPE);

	for (const auto& i : trainingIndices) {
		const combatInfo& combat = allCombats[i];
		std::map<BWAPI::UnitType, int> typesToDestroy1(combat.armySize1);
		std::map<BWAPI::UnitType, int> typesToDestroy2(combat.armySize2);

		// check if we need to decide targets
		bool army1NeedsToDecideTarget = true;
		bool army2NeedsToDecideTarget = true;
		if (typesToDestroy1.size() < 2) army2NeedsToDecideTarget = false;
		if (typesToDestroy2.size() < 2) army1NeedsToDecideTarget = false;

		// if target is obvious skip it
		if (!army1NeedsToDecideTarget && !army2NeedsToDecideTarget) continue;

		// skip combats with mines or transports
		if (hasUnwantedUnitTypes(typesToDestroy1)) continue;
		if (hasUnwantedUnitTypes(typesToDestroy2)) continue;

		// get groupDiversity
		GroupDiversity groupDiversity1 = getGroupDiversity(combat.armySize1);
		GroupDiversity groupDiversity2 = getGroupDiversity(combat.armySize2);

		// give points when a new type is destroyed
		for (const auto& killedInfo : combat.kills) {
			if (killedInfo.armyID == 1 && combat.armyUnits1.find(killedInfo.unitID) != combat.armyUnits1.end()) {
				// unit killed is from army1
				if (army2NeedsToDecideTarget) {
					int unitTypeID = combat.armyUnits1.at(killedInfo.unitID).typeID;
					BWAPI::UnitType unitType(unitTypeID);
					// look if the type is still there to kill it
					if (typesToDestroy1.find(unitType) != typesToDestroy1.end()) {
						typesToDestroy1.erase(unitType);
						bordaCount[unitTypeID].score += typesToDestroy1.size();
						bordaCount[unitTypeID].frequency += 1;
						std::vector<bordaCountFrequency>* specificBorda;
						switch (groupDiversity2) {
						case UnitInfoLearner::AIR:	  specificBorda = &bordaCountAir; break;
						case UnitInfoLearner::GROUND: specificBorda = &bordaCountGround; break;
						case UnitInfoLearner::BOTH:	  specificBorda = &bordaCountBoth; break;
						}
						specificBorda->at(unitTypeID).score += typesToDestroy1.size();
						specificBorda->at(unitTypeID).frequency += 1;
						if (typesToDestroy1.size() == 0) army2NeedsToDecideTarget = false;
					}
				}
			} else if (killedInfo.armyID == 2 && combat.armyUnits2.find(killedInfo.unitID) != combat.armyUnits2.end()) {
				// unit killed is from army2
				if (army1NeedsToDecideTarget) {
					int unitTypeID = combat.armyUnits2.at(killedInfo.unitID).typeID;
					BWAPI::UnitType unitType(unitTypeID);
					// look if the type is still there to kill it
					if (typesToDestroy2.find(unitType) != typesToDestroy2.end()) {
						typesToDestroy2.erase(unitType);
						bordaCount[unitTypeID].score += typesToDestroy2.size();
						bordaCount[unitTypeID].frequency += 1;
						std::vector<bordaCountFrequency>* specificBorda;
						switch (groupDiversity1) {
						case UnitInfoLearner::AIR:	  specificBorda = &bordaCountAir; break;
						case UnitInfoLearner::GROUND: specificBorda = &bordaCountGround; break;
						case UnitInfoLearner::BOTH:	  specificBorda = &bordaCountBoth; break;
						}
						specificBorda->at(unitTypeID).score += typesToDestroy1.size();
						specificBorda->at(unitTypeID).frequency += 1;
						if (typesToDestroy2.size() == 0) army1NeedsToDecideTarget = false;
					}
				}
			}

			// keep giving points?
			if (!army1NeedsToDecideTarget && !army2NeedsToDecideTarget) break;
		}
	}
}

void UnitInfoLearner::calculateAvgBordaCount()
{
	typePriority.clear();
	typePriorityAir.clear();
	typePriorityGround.clear();
	typePriorityBoth.clear();
	typePriority.resize(MAX_UNIT_TYPE);
	typePriorityAir.resize(MAX_UNIT_TYPE);
	typePriorityGround.resize(MAX_UNIT_TYPE);
	typePriorityBoth.resize(MAX_UNIT_TYPE);

	for (auto unitType : BWAPI::UnitTypes::allUnitTypes()) {
		if (bordaCount[unitType].frequency != 0) {
			typePriority[unitType] = (float)bordaCount[unitType].score / (float)bordaCount[unitType].frequency;
			// 			LOG("Unit: " << unitType << " Priority: " << typePriority[unitType]);
		}
		if (bordaCountAir[unitType].frequency != 0) {
			typePriorityAir[unitType] = (float)bordaCountAir[unitType].score / (float)bordaCountAir[unitType].frequency;
			// 			LOG("Unit: " << unitType << " PriorityAir: " << typePriorityAir[unitType]);
		}
		if (bordaCountGround[unitType].frequency != 0) {
			typePriorityGround[unitType] = (float)bordaCountGround[unitType].score / (float)bordaCountGround[unitType].frequency;
			// 			LOG("Unit: " << unitType << " PriorityGround: " << typePriorityGround[unitType]);
		}
		if (bordaCountBoth[unitType].frequency != 0) {
			typePriorityBoth[unitType] = (float)bordaCountBoth[unitType].score / (float)bordaCountBoth[unitType].frequency;
			// 			LOG("Unit: " << unitType << " PriorityBoth: " << typePriorityBoth[unitType]);
		}
	}
}

void UnitInfoLearner::printBordaCount()
{
	std::ostringstream stream;

	stream << "typePriorityAir = {";
	std::copy(typePriorityAir.begin(), typePriorityAir.end() - 1, std::ostream_iterator<float>(stream, ","));
	stream << typePriorityAir.back() << "}";
	LOG(stream.str());
	stream.str(std::string()); // reset the stream
	stream.clear();

	stream << "typePriorityGround = {";
	std::copy(typePriorityGround.begin(), typePriorityGround.end() - 1, std::ostream_iterator<float>(stream, ","));
	stream << typePriorityGround.back() << "}";
	LOG(stream.str());
	stream.str(std::string()); // reset the stream
	stream.clear();

	stream << "typePriorityBoth = {";
	std::copy(typePriorityBoth.begin(), typePriorityBoth.end() - 1, std::ostream_iterator<float>(stream, ","));
	stream << typePriorityBoth.back() << "}";
	LOG(stream.str());
	stream.str(std::string()); // reset the stream
	stream.clear();
}

void UnitInfoLearner::learnTargetBordaCount()
{
	gatherBordaCountStats();
	calculateAvgBordaCount();
// 	printBordaCount();
}

void UnitInfoLearner::learnTrainingSet(std::vector<size_t> trainingIndices)
{
	calculateDPF(trainingIndices);
	gatherBordaCountStats(trainingIndices);
	calculateAvgBordaCount();
}

bool UnitInfoLearner::hasUnwantedUnitTypes(const std::map<BWAPI::UnitType, int> &typesSize)
{
	for (const auto& typeSize : typesSize) {
		if (typeSize.first.spaceProvided() > 0 || 
			typeSize.first == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine ||
			typeSize.first == BWAPI::UnitTypes::Zerg_Scourge) {
			return true;
		}
	}
	return false;
}

UnitInfoLearner::GroupDiversity UnitInfoLearner::getGroupDiversity(const std::map<BWAPI::UnitType, int> &typesSize)
{
	bool hasAirUnits = false;
	bool hasGroundUnits = false;
	for (const auto& typeSize : typesSize) {
		if (typeSize.first.isFlyer()) hasAirUnits = true;
		else hasGroundUnits = true;
	}
	if (hasAirUnits && hasGroundUnits) return GroupDiversity::BOTH;
	if (hasAirUnits) return GroupDiversity::AIR;
	return GroupDiversity::GROUND;
}

void UnitInfoLearner::clear()
{
	for (int i = 0; i < MAX_UNIT_TYPE; ++i) {
		unitTypeDPF[i].clear();
		unitTypeDPF[i].resize(MAX_UNIT_TYPE);
		unitTypeStats[i].clear();
		unitTypeStats[i].resize(MAX_UNIT_TYPE);
	}
	typePriority.clear();
	typePriority.resize(MAX_UNIT_TYPE);
	bordaCount.clear();
	bordaCount.resize(MAX_UNIT_TYPE);

	armyDestroyed = 0;
	armyReinforcement = 0;
	armyPeace = 0;
	armyGameEnd = 0;
}

const std::string UnitInfoLearner::toString(combatInfo combat)
{
	std::stringstream tmp;

	tmp << "Initial armies\n";
	for (const auto& unit : combat.armyUnits1) tmp << "P0 " << toString(unit.second) << "\n";
	for (const auto& unit : combat.armyUnits2) tmp << "P1 " << toString(unit.second) << "\n";
	tmp << "Final armies\n";
	for (const auto& unit : combat.armyUnitsEnd1) tmp << "P0 " << toString(unit.second) << "\n";
	for (const auto& unit : combat.armyUnitsEnd2) tmp << "P1 " << toString(unit.second) << "\n";

	return tmp.str();
}

const std::string UnitInfoLearner::toString(unitInfo unit)
{
	std::stringstream tmp;
	float dpfSingle = std::max(unitDPF[unit.typeID].air, unitDPF[unit.typeID].ground);
	float dpfBoth = std::min(unitDPF[unit.typeID].bothAir, unitDPF[unit.typeID].bothGround);
	float dpf = std::max(dpfSingle, dpfBoth);
	tmp << (unit.HP + unit.shield) << " " << dpf << " " << unit.typeName;
	return tmp.str();
}
