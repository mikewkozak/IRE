#pragma once

#include "InformationManager.h"
#include "SquadAgent.h"
#include "EvaluationFunctionBasic.h"
// Search algorithms
#include "ABCD.h"
#include "MCTSCD.h"

class AbstractLayer
{
public:
	AbstractLayer();
	AbstractLayer(std::vector<SquadAgent*> squads);
    inline void addSquadToGameState(SquadAgent* squad);
    std::map<SquadAgent*, BWAPI::Position> searchBestOrders();
    bool hasFriendlyUnits();
    float getEvaluation();
    void printBranchingStats();

private:
	std::map<unsigned short, std::set<SquadAgent*> > _idToSquad;

};
