#include "AbstractLayer.h"

AbstractLayer::AbstractLayer()
{
	// import general game state
	informationManager->gameState.cleanArmyData();
	informationManager->gameState.importCurrentGameState();
}

AbstractLayer::AbstractLayer(std::vector<SquadAgent*> squads)
{
	informationManager->gameState.cleanArmyData();		// clear lists
	informationManager->gameState.addAllEnemyUnits();	// add only enemy units
	informationManager->gameState.addSeenEnemyUnits();	// add seen enemy units
    informationManager->gameState.addSelfBuildings();	// add our buildings

	// translate squads to high-level squads
	for (const auto& squad : squads) {
		if (squad->_state == SquadAgent::Scout) continue; // ignore scout squad
		if (squad->_state == SquadAgent::Search) continue; // ignore squad searching for enemy
		addSquadToGameState(squad);
	}

	// add technologies researched
	informationManager->gameState.friendlySiegeTankResearched = BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Tank_Siege_Mode);
	informationManager->gameState.enemySiegeTankResearched = BWAPI::Broodwar->enemy()->hasResearched(BWAPI::TechTypes::Tank_Siege_Mode);
}

// Add squad to game state and store reference ID
void AbstractLayer::addSquadToGameState(SquadAgent* squad)
{
	std::map<unsigned short, unsigned int> groupIdFrequency;
	
	unsigned short abstractGroupID;
    // for each unit on the squad, add it to the game state and save id reference
	for (const auto& squadUnit : squad->_squadUnits) {
		if (squadUnit->_unit->getType().isWorker()) continue; // ignore workers
		abstractGroupID = informationManager->gameState.addFriendlyUnit(squadUnit->_unit);
        groupIdFrequency[abstractGroupID]++;
    }

    unsigned int maxFrequency = 0;
	unsigned short bestGroup;
    // assign to the squad the most common group ID
	for (const auto& frequency : groupIdFrequency) {
		if (frequency.second > maxFrequency) {
			bestGroup = frequency.first;
			maxFrequency = frequency.second;
        }
    }

    // one idGroup can have many squads!!
    _idToSquad[bestGroup].insert(squad);
    //LOG("Best group for squad (" << squad << "): " << bestGroup);
}

// Execute search and return best targetPosition for each squad
std::map<SquadAgent*, BWAPI::Position> AbstractLayer::searchBestOrders()
{
    // compare game states
//     if (!informationManager->lastGameState.gameover()) {
//         int misplacedUnits = 0;
//         int totalUnits = 0;
// 		informationManager->gameState.compareFriendlyUnits(informationManager->lastGameState, misplacedUnits, totalUnits);
//         LOG("Correct Units: " << totalUnits-misplacedUnits << " of " << totalUnits << " jaccard: " << float(totalUnits-misplacedUnits)/float(totalUnits));
//     }

    // now that we have all the units in the game state, compute expected end frame
    informationManager->gameState.calculateExpectedEndFrameForAllGroups();
    // and forward until next point decision
	//informationManager->gameState.moveForward(); // we can move Forward ...
	informationManager->gameState.resetFriendlyActions(); // ...or set our orders to 0 to find the best immediate action
    //LOG(informationManager->gameState.toString());

    // Search algorithm
    playerActions_t bestActions;
    EvaluationFunctionBasic ef;
    if (SEARCH_ALGORITHM == "ABCD") {
        int depth = LoadConfigInt("ABCD", "depth", 1);
        ABCD searchAlg = ABCD(depth, &ef);
        bestActions = searchAlg.start(true, informationManager->gameState);
    } else if (SEARCH_ALGORITHM == "MCTSCD") {
        int depth = LoadConfigInt("MCTSCD", "depth", 1);
        int iterations = LoadConfigInt("MCTSCD", "iterations");
        int maxSimTime = LoadConfigInt("MCTSCD", "max_simulation_time");
        MCTSCD searchAlg = MCTSCD(depth, &ef, iterations, maxSimTime);
        bestActions = searchAlg.start(informationManager->gameState);
    } else {
        // get random actions
        ActionGenerator moveGenerator = ActionGenerator(&informationManager->gameState);
        bestActions = moveGenerator.getRandomAction();
    }

    // update last gameState
//     informationManager->lastGameState = informationManager->gameState;
//     informationManager->lastGameState.execute(bestActions, true); // it's "true" because bestActions should be our actions (MAX player)
//     informationManager->lastGameState.moveForward(HIGH_LEVEL_REFRESH);

    //LOG("Best actions: ");
    std::map<SquadAgent*, BWAPI::Position> bestOrders;
    BWAPI::Position targetPosition;
	for (const auto& groupAction : bestActions) {
		std::set<SquadAgent*> squadSet = _idToSquad[groupAction.pos];

		for (auto& squad : squadSet) {
			targetPosition = informationManager->gameState.getCenterRegionId((int)groupAction.action.targetRegion);
			bestOrders[squad] = targetPosition;
//             LOG("  - Group ID: " << groupID << ", action: " << informationManager->gameState.getAbstractOrderName((int)orderId) << " region: " << (int)targetRegionId 
//                 << "(" << targetPosition.x << "," << targetPosition.y << ") squad (" << *squad << ")");
        }
    }

    return bestOrders;
}

bool AbstractLayer::hasFriendlyUnits()
{
	return !(informationManager->gameState._army.friendly.empty() || informationManager->gameState.hasOnlyBuildings(informationManager->gameState._army.friendly));
}

float AbstractLayer::getEvaluation()
{
    EvaluationFunctionBasic ef;
    return ef.evaluate(informationManager->gameState);
}

void AbstractLayer::printBranchingStats()
{
   // branching factor start for root node
    ActionGenerator actions = ActionGenerator(&informationManager->gameState);
    double highLevelFriendly = actions.getHighLevelFriendlyActions();
    double highLevelEnemy = actions.getHighLevelEnemyActions();
    double highLevelTotal = highLevelFriendly * highLevelEnemy;

	double lowLevelTotal = 1;
	double sparcraftTotal = 1;
	double lowLevelLogTotal = 0;
	double sparcraftLogTotal = 0;
	for (auto player : BWAPI::Broodwar->getPlayers()) {
		if (player->isNeutral()) continue;
		double lowLevelActions = actions.getLowLevelActions(player->getUnits());
		double sparCraftActions = actions.getSparcraftActions(player->getUnits());
		lowLevelTotal *= lowLevelActions;
		sparcraftTotal *= sparCraftActions;
		lowLevelLogTotal += std::log10(lowLevelActions);
		sparcraftLogTotal += std::log10(sparCraftActions);
	}

	LOG("LL: " << lowLevelTotal << " LL-log: " << lowLevelLogTotal
		<< " SparCraft: " << sparcraftTotal << " SparCraft-log: " << sparcraftLogTotal
		<< " HL: " << highLevelTotal << " HL-log: " << std::log10(highLevelTotal));
}