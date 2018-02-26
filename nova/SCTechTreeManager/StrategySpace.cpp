#include "stdafx.h"
#include "StrategySpace.h"


StrategySpace::StrategySpace()
{
}


StrategySpace::~StrategySpace()
{
}

void StrategySpace::addStrategy(int race, Strategy strat) {
	//rotate node X/Y positions around axes based on these biases
	for (std::pair<VertexIterator, VertexIterator> it = boost::vertices(strat.techTree); it.first != it.second; ++it.first) {
		std::cout << "Examining " << strat.techTree[*it.first].name << std::endl;
		if (strat.techTree[*it.first].node != NULL) {
			double xpos = strat.techTree[*it.first].depth * /*cur_child / max_children*/ 100 * strat.air_aa_intensity;
			strat.techTree[*it.first].location.set<StrategySpace::AIR_AA_AXIS>(xpos);

			double ypos = strat.techTree[*it.first].depth * 100 * strat.air_aa_intensity;
			strat.techTree[*it.first].location.set<StrategySpace::GROUND_AG_AXIS>(ypos);

			double zpos = 100 * strat.air_aa_intensity;
			strat.techTree[*it.first].location.set<StrategySpace::AGGRESSIVE_DEFENSIVE_AXIS>(zpos);

		}
	}

	//add strategy to the correct race graph
	if (race == 0) {
		//add strat tech tree to terranStrategySpace
	}
	else if (race == 1) {
		//add strat tech tree to protossStrategySpace
	}
	else if (race == 2) {
		//add strat tech tree to zergStrategySpace
	}
	else {
		printf("UNKNOWN RACE. NOT ADDING STRATEGY");
	}
}