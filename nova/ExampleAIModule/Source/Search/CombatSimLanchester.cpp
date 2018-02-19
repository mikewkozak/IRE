#include "CombatSimLanchester.h"

CombatSimLanchester::CombatSimLanchester(std::vector<DPF_t>* maxDPF, comp_f comparator1, comp_f comparator2)
	:_maxDPF(maxDPF),
	_winner(0),
	_easyCombat(false)
{
	_comparator1 = comparator1;
	_comparator2 = comparator2;
}

CombatSimLanchester::combatStats_t CombatSimLanchester::getCombatStats(const UnitGroupVector &army)
{
	combatStats_t combatStats;

	for (auto unitGroup : army) {
		int typeId = unitGroup->unitTypeId;
		// auto siege tanks TODO only if researched!!
		if (typeId == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode) {
			typeId = BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode;
		}
		combatStats.bothAirDPF += unitGroup->numUnits * (*_maxDPF)[typeId].bothAir;
		combatStats.bothGroundDPF += unitGroup->numUnits * (*_maxDPF)[typeId].bothGround;
		combatStats.airDPF += unitGroup->numUnits * (*_maxDPF)[typeId].air;
		combatStats.groundDPF += unitGroup->numUnits * (*_maxDPF)[typeId].ground;

		// if unit cannot attack, we don't consider its HP to "beat" (to exclude dummy buildings)
		BWAPI::UnitType unitType(typeId);
		if (unitType.canAttack() || unitType.isSpellcaster() || unitType.spaceProvided() > 0) {
			if (unitType.isFlyer()) {
				combatStats.airHP += unitGroup->numUnits * unitGroup->HP;
				combatStats.airUnitsSize += unitGroup->numUnits;
			} else	{
				combatStats.groundHP += unitGroup->numUnits * unitGroup->HP;
				combatStats.groundUnitsSize += unitGroup->numUnits;
			}
		} else {
			if (unitType.isFlyer()) {
				combatStats.airHPextra += unitGroup->numUnits * unitGroup->HP;
				combatStats.airUnitsSizeExtra += unitGroup->numUnits;
			} else {
				combatStats.groundHPextra += unitGroup->numUnits * unitGroup->HP;
				combatStats.groundUnitsSizeExtra += unitGroup->numUnits;
			}
		}
	}

	return combatStats;
}

void CombatSimLanchester::getExtraCombatLength(combatStats_t friendStats, combatStats_t enemyStats)
{
	// groups that can attack ground and air help to kill the group that it takes more time to kill
	double timeToKillEnemyAir = (enemyStats.airHPextra > 0) ? (friendStats.airDPF == 0) ? INT_MAX : enemyStats.airHPextra / friendStats.airDPF : 0;
	double timeToKillEnemyGround = (enemyStats.groundHPextra > 0) ? (friendStats.groundDPF == 0) ? INT_MAX : enemyStats.groundHPextra / friendStats.groundDPF : 0;
	if (friendStats.bothAirDPF > 0) {
		if (timeToKillEnemyAir > timeToKillEnemyGround) {
			double combinetDPF = friendStats.airDPF + friendStats.bothAirDPF;
			timeToKillEnemyAir = (enemyStats.airHPextra > 0) ? (combinetDPF == 0) ? INT_MAX : enemyStats.airHPextra / combinetDPF : 0;
		} else {
			double combinetDPF = friendStats.groundDPF + friendStats.bothGroundDPF;
			timeToKillEnemyGround = (enemyStats.groundHPextra > 0) ? (combinetDPF == 0) ? INT_MAX : enemyStats.groundHPextra / combinetDPF : 0;
		}
	}

	double timeToKillFriendAir = (friendStats.airHPextra > 0) ? (enemyStats.airDPF == 0) ? INT_MAX : friendStats.airHPextra / enemyStats.airDPF : 0;
	double timeToKillFriendGround = (friendStats.groundHPextra > 0) ? (enemyStats.groundDPF == 0) ? INT_MAX : friendStats.groundHPextra / enemyStats.groundDPF : 0;
	if (enemyStats.bothAirDPF > 0) {
		if (timeToKillFriendAir > timeToKillEnemyGround) {
			double combinetDPF = enemyStats.airDPF + enemyStats.bothAirDPF;
			timeToKillFriendAir = (friendStats.airHPextra > 0) ? (combinetDPF == 0) ? INT_MAX : friendStats.airHPextra / combinetDPF : 0;
		} else {
			double combinetDPF = enemyStats.groundDPF + enemyStats.bothGroundDPF;
			timeToKillFriendGround = (friendStats.groundHPextra > 0) ? (combinetDPF == 0) ? INT_MAX : friendStats.groundHPextra / combinetDPF : 0;
		}
	}

	if (timeToKillEnemyAir == INT_MAX && timeToKillEnemyGround > 0) _extraTimeToKillEnemy = (int)timeToKillEnemyGround;
	else if (timeToKillEnemyGround == INT_MAX && timeToKillEnemyAir > 0) _extraTimeToKillEnemy = (int)timeToKillEnemyAir;
	else _extraTimeToKillEnemy = (int)(std::max)(timeToKillEnemyAir, timeToKillEnemyGround);

	if (timeToKillFriendAir == INT_MAX && timeToKillFriendGround > 0) _extraTimeToKillFriend = (int)timeToKillFriendGround;
	else if (timeToKillFriendGround == INT_MAX && timeToKillFriendAir > 0) _extraTimeToKillFriend = (int)timeToKillFriendAir;
	else _extraTimeToKillFriend = (int)(std::max)(timeToKillFriendAir, timeToKillFriendGround);
}


int CombatSimLanchester::getCombatLength(GameState::army_t* army)
{
	removeHarmlessIndestructibleUnits(army);
	friendStats = getCombatStats(army->friendly);
	enemyStats = getCombatStats(army->enemy);

	float avgEnemyAirHP = enemyStats.airHP == 0.0f ? 0.0f : float(enemyStats.airHP) / float(enemyStats.airUnitsSize);
	float avgFriendAirHP = friendStats.airHP == 0.0f ? 0.0f : float(friendStats.airHP) / float(friendStats.airUnitsSize);
	float avgEnemyGroundHP = enemyStats.groundHP == 0.0f ? 0.0f : float(enemyStats.groundHP) / float(enemyStats.groundUnitsSize);
	float avgFriendGroundHP = friendStats.groundHP == 0.0f ? 0.0f : float(friendStats.groundHP) / float(friendStats.groundUnitsSize);

	float sumAvgEnemyHP = avgEnemyAirHP + avgEnemyGroundHP;
	float sumAvgFriendHP = avgFriendAirHP + avgFriendGroundHP;

	lanchester.x0 = friendStats.airUnitsSize + friendStats.groundUnitsSize;
	lanchester.y0 = enemyStats.airUnitsSize + enemyStats.groundUnitsSize;

	float avgEnemyHP = float((enemyStats.airHP + enemyStats.groundHP) / lanchester.y0);
	float avgFriendHP = float((friendStats.airHP + friendStats.groundHP) / lanchester.x0);

	// if unit cant attack both both{air|ground}DPF is populated, otherwise {air|ground}DPF is populated
	float avgEnemyAirDPF = float((enemyStats.bothAirDPF + enemyStats.airDPF) / lanchester.y0);
	float avgEnemyGroundDPF = float((enemyStats.bothGroundDPF + enemyStats.groundDPF) / lanchester.y0);
	float avgFriendAirDPF = float((friendStats.bothAirDPF + friendStats.airDPF) / lanchester.x0);
	float avgFriendGroundDPF = float((friendStats.bothGroundDPF + friendStats.groundDPF) / lanchester.x0);

	float avgEnemyDPF = avgEnemyAirDPF * (avgFriendAirHP / sumAvgFriendHP) +
		avgEnemyGroundDPF * (avgFriendGroundHP / sumAvgFriendHP);
	float avgFriendDPF = avgFriendAirDPF * (avgEnemyAirHP / sumAvgEnemyHP) +
		avgFriendGroundDPF * (avgEnemyGroundHP / sumAvgEnemyHP);

	lanchester.a = avgEnemyDPF / avgFriendHP;
	lanchester.b = avgFriendDPF / avgEnemyHP;

	if (lanchester.a == 0 && lanchester.b == 0) {		// combat is impossible
		_winner = 0;
		_combatLength = 0;
	} else if (lanchester.a == 0 && lanchester.b > 0) { // enemy cannot attack
		_easyCombat = true;
		_winner = 1;
		_combatLength = static_cast<int>(lanchester.y0 / lanchester.b);
	} else if (lanchester.a > 0 && lanchester.b == 0) { // friendly cannot attack
		_easyCombat = true;
		_winner = 2;
		_combatLength = static_cast<int>(lanchester.x0 / lanchester.a);
	} else {
		// use Lanchester's equations to predict winner and combatLength
		_easyCombat = false;
		lanchester.I = sqrt(lanchester.a * lanchester.b);
		lanchester.R_a = sqrt(lanchester.a / lanchester.b);
		lanchester.R_b = sqrt(lanchester.b / lanchester.a);
		double combatLength = 0.0;
		if (lanchester.x0 / lanchester.y0 > lanchester.R_a) {
			// X (friendly) win
			_winner = 1;
			double tmp = (lanchester.y0 / lanchester.x0) * lanchester.R_a;
			combatLength = (1 / (2 * lanchester.I)) * log((1 + tmp) / (1 - tmp));
		} else if (lanchester.x0 / lanchester.y0 < lanchester.R_a) {
			// Y (enemy) win 
			_winner = 2;
			double tmp = (lanchester.x0 / lanchester.y0) * lanchester.R_b;
			combatLength = (1 / (2 * lanchester.I)) * log((1 + tmp) / (1 - tmp));
		} else {
			// draw
			_winner = 3;
			combatLength = lanchester.x0 / lanchester.a; // used as a lower bound time
		}

		if (std::isnan(combatLength)) {
			_winner = 0;
			_combatLength = 0;
// 			DEBUG("Trying to simulate a combat between armies that cannot kill each other");
		} else {
			_combatLength = static_cast<int>(std::ceil(combatLength));
		}
	}

	getExtraCombatLength(friendStats, enemyStats);
	switch (_winner) {
	case 0: // combat impossible (look if we can destroy extra targets)
		if (_extraTimeToKillEnemy) { _winner = 1; return _extraTimeToKillEnemy; }
		if (_extraTimeToKillFriend) { _winner = 2; return _extraTimeToKillFriend; }
	case 1: // friendly wins
		return _combatLength + _extraTimeToKillEnemy;
	case 2: // friendly wins
		return _combatLength + _extraTimeToKillFriend;
	case 3: // draw
	default:
		return _combatLength;
	}
}

void CombatSimLanchester::simulateCombat(GameState::army_t* armyInCombat, GameState::army_t* army, int frames)
{
	removeHarmlessIndestructibleUnits(armyInCombat);
	if (!canSimulate(armyInCombat, army)) return;

	// this will define the order to kill units from the set (TODO we only need to sort if the army survives)
	sortGroups(&armyInCombat->friendly, _comparator1, &armyInCombat->enemy);
	sortGroups(&armyInCombat->enemy, _comparator1, &armyInCombat->friendly);

	int combatLength = getCombatLength(armyInCombat); // it also precomputes all Lanchester parameters

	if (combatLength == 0) // combat impossible
		return;

	if (_easyCombat) {
		if (frames >= combatLength || frames == 0) { //simulate until one army destroyed (fight-to-the-finish)
			if (_winner == 1) { // friendly won
				removeAllGroups(&armyInCombat->enemy, &army->enemy);
			} else if (_winner == 2) { // enemy won
				removeAllGroups(&armyInCombat->friendly, &army->friendly);
			} else {
				DEBUG("This should not happen");
			}
		} else { // partial combat
			int xkilled = static_cast<int>(std::floor(lanchester.a * frames));
			removeUnits(xkilled, armyInCombat->friendly, &army->friendly);
			int ykilled = static_cast<int>(std::floor(lanchester.b * frames));
			removeUnits(ykilled, armyInCombat->enemy, &army->enemy);
		}
	} else {
		if (frames >= combatLength || frames == 0) { //simulate until one army destroyed (fight-to-the-finish)
			if (_winner == 1) { // friendly won
				removeAllGroups(&armyInCombat->enemy, &army->enemy);
				double numSurvivors = sqrt((lanchester.x0*lanchester.x0) - ((lanchester.a / lanchester.b)*(lanchester.y0*lanchester.y0)));
				int unitsToRemove = static_cast<int>(lanchester.x0 - std::ceil(numSurvivors));
				removeUnits(unitsToRemove, armyInCombat->friendly, &army->friendly);
			} else if (_winner == 2) { // enemy won
				removeAllGroups(&armyInCombat->friendly, &army->friendly);
				double numSurvivors = sqrt((lanchester.y0*lanchester.y0) - ((lanchester.b / lanchester.a)*(lanchester.x0*lanchester.x0)));
				int unitsToRemove = static_cast<int>(lanchester.y0 - std::ceil(numSurvivors));
				removeUnits(unitsToRemove, armyInCombat->enemy, &army->enemy);
			} else if (_winner == 3) { // draw
				removeAllMilitaryGroups(&armyInCombat->friendly, &army->friendly);
				removeAllMilitaryGroups(&armyInCombat->enemy, &army->enemy);
			} else {
				DEBUG("Unknown winner");
			}
		} else { // partial combat
			// first simulate combat between opposing forces
			int opposingCombatFrames = std::min(_combatLength, frames);
			// cache some computations
			double eIt = std::exp(lanchester.I*opposingCombatFrames);
			double eMinIt = std::exp(-lanchester.I*opposingCombatFrames);
			// x survivors
			double numSurvivors = 0.5 * ((lanchester.x0 - lanchester.R_a * lanchester.y0)*eIt + (lanchester.x0 + lanchester.R_a * lanchester.y0)*eMinIt);
			int unitsToRemove = static_cast<int>(lanchester.x0 - std::ceil(numSurvivors));
			removeUnits(unitsToRemove, armyInCombat->friendly, &army->friendly);
			// y survivors
			numSurvivors = 0.5 * ((lanchester.y0 - lanchester.R_b * lanchester.x0)*eIt + (lanchester.y0 + lanchester.R_b * lanchester.x0)*eMinIt);
			unitsToRemove = static_cast<int>(lanchester.y0 - std::ceil(numSurvivors));
			removeUnits(unitsToRemove, armyInCombat->enemy, &army->enemy);
			if (frames > _combatLength) {
				// now from the harmless survivors, compute how many we had time to kill
				int easyCombatFrames = frames - _combatLength;
				getCombatLength(armyInCombat); // recompute lanchester parameters from survivors
				if (_extraTimeToKillFriend) {
					float killRatio = 1.0f - (((float)_extraTimeToKillFriend - (float)easyCombatFrames) / (float)_extraTimeToKillFriend);
					float friendSize = float(friendStats.airUnitsSizeExtra + friendStats.groundUnitsSizeExtra);
					int xkilled = static_cast<int>(std::floor(killRatio * friendSize));
					removeUnits(xkilled, armyInCombat->friendly, &army->friendly);
				}
				if (_extraTimeToKillEnemy) {
					float killRatio = 1.0f - (((float)_extraTimeToKillEnemy - (float)easyCombatFrames) / (float)_extraTimeToKillEnemy);
					float enemySize = float(enemyStats.airUnitsSizeExtra + enemyStats.groundUnitsSizeExtra);
					int ykilled = static_cast<int>(std::floor(killRatio * enemySize));
					removeUnits(ykilled, armyInCombat->enemy, &army->enemy);
				}
			}
		}
	}
}

void CombatSimLanchester::removeUnits(int numUnitsToRemove, UnitGroupVector &unitsInCombat, UnitGroupVector* generalList)
{
	if (numUnitsToRemove <= 0) return;

	for (auto it = unitsInCombat.begin(); it != unitsInCombat.end();) {
		if (numUnitsToRemove < (*it)->numUnits) {
			(*it)->numUnits -= numUnitsToRemove;
			break;
		} else {
			numUnitsToRemove -= (*it)->numUnits;
			removeGroup(*it, generalList);
			it = unitsInCombat.erase(it);
		}
	}
}

