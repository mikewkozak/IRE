#pragma once

#include "CombatInfo.h"

class EvalCombat
{
public:
	static float LTD(const std::vector<DPF_t> &unitDPF, const combatInfo &combat, const float &k)
	{
//		LOG("LTD(" << k <<"): " << EvalCombat::LTD(unitDPF, combat.armyUnits1, k) << " - " << EvalCombat::LTD(unitDPF, combat.armyUnits2, k));
		return EvalCombat::LTD(unitDPF, combat.armyUnits1, k) - EvalCombat::LTD(unitDPF, combat.armyUnits2, k);
	};

	static float LTD(const std::vector<DPF_t> &unitDPF, const std::map<int, unitInfo> &army, const float &k)
	{
		if (army.empty()) return 0;

		float sum(0);

		for (const auto& unit : army) {
			float dpfSingle = std::max(unitDPF[unit.second.typeID].air, unitDPF[unit.second.typeID].ground);
			float dpfBoth = std::min(unitDPF[unit.second.typeID].bothAir, unitDPF[unit.second.typeID].bothGround);
			float dpf = std::max(dpfSingle, dpfBoth);
			sum += std::pow(unit.second.HP + unit.second.shield, 1.0 / k) * dpf;
		}

		return sum;
	};

};