#include "MCTSCD.h"

const double UCB_C = 5;		// this is the constant that regulates exploration vs exploitation, it must be tuned for each domain
const double EPSILON = 0.2; // e-greedy strategy

#ifdef HTML_LOG
#include "HTMLoutput.h"
#endif

MCTSCD::MCTSCD(int maxDepth, EvaluationFunction* ef, int maxSimulations, int maxSimulationTime)
	:_maxDepth(maxDepth),
	_ef(ef),
	_maxSimulations(maxSimulations),
	_maxSimulationTime(maxSimulationTime),
#ifdef DEPTH_STATS
	_maxDepthReached(0),
	_maxDepthRolloutReached(0),
#endif
	_maxMissplacedUnits(0)
{
}

playerActions_t MCTSCD::start(GameState gs)
{
	_rootGameState = &gs;
	combatsSimulated = 0;

	timerUCT.start();
	playerActions_t bestAction = startSearch(_rootGameState->_time + _maxSimulationTime);
	double timeUCT = timerUCT.stopAndGetTime();

	// save stats
	int friendlyGroups = gs.getFriendlyGroupsSize();
	int friendlyUnits = gs.getFriendlyUnitsSize();
	int enemyGroups = gs.getEnemyGroupsSize();
	int enemyUnits = gs.getEnemyUnitsSize();
	auto oldPrecision = fileLog.precision(6);
	auto oldFlags = fileLog.flags();
	fileLog.setf(std::ios::fixed, std::ios::floatfield);
	LOG("Groups.F: " << std::setw(2) << friendlyGroups << " Groups.E: " << std::setw(2) << enemyGroups
		<< " Units.F: " << std::setw(3) << friendlyUnits << " Units.E: " << std::setw(3) << enemyUnits
		<< " seconds: " << std::setw(10) << timeUCT
		<< " combats: " << std::setw(6) << combatsSimulated);
#ifdef BRANCHING_STATS
		<< " maxBranching: " << std::setw(6) << (int)_branching.getMax()
		<< " avgBranching: " << _branching.getMean());
#endif
		
	fileLog.precision(oldPrecision);
	fileLog.flags(oldFlags);
#ifdef DEPTH_STATS
	LOG(" - MaxDepth: " << _maxDepthReached << " MaxRolloutDepth: " << _maxDepthRolloutReached);
#endif

    return bestAction;
}

playerActions_t MCTSCD::startSearch(int cutOffTime)
{
    // create root node
	gameNode_t* tree = newGameNode(*_rootGameState);

	// if root node only has one possible action, return it
	ActionGenerator moveGenerator(_rootGameState, true); // by default MAX player (friendly)
	if (moveGenerator._size == 1) {
// 		return moveGenerator.getNextAction();
		return moveGenerator.getUniqueRandomAction();
	}

    // while withing computational budget
	for (int i = 0; i<_maxSimulations; ++i) {
        // tree policy, get best child
        gameNode_t* leaf = bestChild(tree);
        
        if (leaf) {
            // default policy, run simulation
// 			LOG("SIMULATION");
            GameState gs2 = leaf->gs; // copy the game state to run simulation
			simulate(&gs2, cutOffTime, leaf->nextPlayerInSimultaneousNode);
#ifdef HTML_LOG
			leaf->simulations.push_back(gs2);
#endif
            
			// use game frame time as a reduction factor
			int time = gs2._time - _rootGameState->_time;
			double evaluation = _ef->evaluate(gs2)*pow(0.999, time / 10.0);
// 			double evaluation = _ef->evaluate(gs2);

            // backup
			while (leaf != nullptr) {
				leaf->totalEvaluation += evaluation;
				leaf->totalVisits++;
				leaf = leaf->parent;
			}
        }
    }
    
    // return best child
    int mostVisitedIdx = -1;
    gameNode_t* mostVisited = nullptr;
	for (unsigned int i = 0; i<tree->children.size(); ++i) {
        gameNode_t* child = tree->children[i];
        if (mostVisited == nullptr || child->totalVisits > mostVisited->totalVisits) {
            mostVisited = child;
            mostVisitedIdx = i;
        }
    }

    playerActions_t bestActions;
	if (mostVisitedIdx != -1) {
        bestActions = tree->actions[mostVisitedIdx];
    }

#ifdef DEBUG_ORDERS
	// DEBUG check if actions are friendly actions
	for (const auto& actions : tree->actions) {
		for (const auto& action : actions) {
			if (!action.isFriendly) {
				DEBUG("Root actions are for enemey!!");
				// print game state
				DEBUG(_rootGameState->toString());
				// print possible actions
				ActionGenerator testActions(_rootGameState, true);
				DEBUG(testActions.toString());
			}
		}
	}
#endif

#ifdef HTML_LOG
	HTMLoutput printMCTS(tree, _ef);
#endif

    deleteAllChildren(tree); // free memory
    return bestActions;
}

void MCTSCD::simulate(GameState* gs, int time, int nextSimultaneous)
{
    int nextPlayerInSimultaneousNode = nextSimultaneous;
    int depth = 0;
	ActionGenerator moveGenerator;
	int nextPlayer = gs->getNextPlayerToMove(nextPlayerInSimultaneousNode);
	while (nextPlayer != -1 && gs->_time < time) {
		moveGenerator = ActionGenerator(gs, nextPlayer != 0);
#ifdef BRANCHING_STATS
		_branchingRollout.add(moveGenerator._size);
#endif

        // chose random action
// 		playerActions_t unitsAction = moveGenerator.getRandomAction();
		playerActions_t unitsAction = moveGenerator.getBiasAction();

        // execute action
        gs->execute(unitsAction, moveGenerator._player);
	    gs->moveForward();
        depth++;

		// look next player to move
		nextPlayer = gs->getNextPlayerToMove(nextPlayerInSimultaneousNode);
    }
#ifdef DEPTH_STATS
    if (_maxDepthRolloutReached < depth) _maxDepthRolloutReached = depth;
#endif
}

MCTSCD::gameNode_t* MCTSCD::newGameNode(GameState &gs, gameNode_t* parent)
{
    gameNode_t* newNode = new gameNode_t(parent, gs);

	if (parent != nullptr) {
		newNode->depth = parent->depth + 1;
		newNode->nextPlayerInSimultaneousNode = parent->nextPlayerInSimultaneousNode;
#ifdef DEPTH_STATS
		if (_maxDepthReached < newGameNode->depth) _maxDepthReached = (int)newGameNode->depth;
#endif
    }

	// get the next player to move
	newNode->player = newNode->gs.getNextPlayerToMove(newNode->nextPlayerInSimultaneousNode);
	// if it's a leaf we are done
	if (newNode->player == -1) return newNode;
	// generate the player possible moves
	newNode->moveGenerator = ActionGenerator(&newNode->gs, newNode->player != 0);

	// if we only have one possible action, execute it and move forward
	int iterations = 0;
	while (newNode->moveGenerator._size == 1) {
		iterations++;
		
		// execute the only action
		playerActions_t unitsAction = newNode->moveGenerator.getNextAction();
		newNode->gs.execute(unitsAction, newNode->moveGenerator._player);
		newNode->gs.moveForward();

		// prepare the node
		// get the next player to move
		newNode->player = newNode->gs.getNextPlayerToMove(newNode->nextPlayerInSimultaneousNode);
		// if it's a leaf we are done
		if (newNode->player == -1) return newNode;
		// generate the player possible moves
		newNode->moveGenerator = ActionGenerator(&newNode->gs, newNode->player != 0);

#ifdef _DEBUG
		// DEBUG if still only one action, we can end in an infinite loop
		if (newNode->moveGenerator._size == 1 && iterations > 1) {
			DEBUG("Still only 1 action at iteration " << iterations);
		}
		if (newNode->moveGenerator._size < 1) {
			DEBUG("PARENT GAME STATE");
			DEBUG(gs.toString());
			DEBUG("LAST GAME STATE");
			DEBUG(newNode->gs.toString());
			DEBUG("Wrong number of actions");
		}
#endif
	}

#ifdef BRANCHING_STATS
	_branching.add(newNode->moveGenerator._size);
#endif
	return newNode;
}

// recursively deletes all children
void MCTSCD::deleteAllChildren(gameNode_t* node)
{
	for (auto& child : node->children) deleteAllChildren(child);
    delete node;
}

MCTSCD::gameNode_t* MCTSCD::bestChild(gameNode_t* currentNode)
{
    // Cut the tree policy at a predefined depth
	if (_maxDepth && currentNode->depth >= _maxDepth) return currentNode;

    // if gameover return this node
    if (currentNode->player == -1) return currentNode;

	// if first time here return this node
	if (currentNode->totalVisits == 0) return currentNode;

    // Bandit policy (aka Tree policy)
// 	gameNode_t* best = UCB(currentNode);
// 	gameNode_t* best = eGreedy(currentNode);
	gameNode_t* best = eGreedyInformed(currentNode);
// 	gameNode_t* best = PUCB(currentNode);
    
    if (best == nullptr) {
        // No more leafs because this node has no children!
        return currentNode;
    }
	return bestChild(best);
}

MCTSCD::gameNode_t* MCTSCD::createChild(gameNode_t* node, playerActions_t action)
{
	if (!action.empty()) {
		node->actions.push_back(action);
		GameState gs2 = node->gs.cloneIssue(action, node->moveGenerator._player);
		gameNode_t* newChild = newGameNode(gs2, node);
		node->children.push_back(newChild);
		return newChild;
	} else {
		printNodeError("Error generating action for first child", node);
		return node;
	}
}

void MCTSCD::printNodeError(std::string errorMsg, gameNode_t* node)
{
	DEBUG(errorMsg);
	LOG(node->gs.toString());
	LOG("Player: " << node->player);
}

MCTSCD::gameNode_t* MCTSCD::eGreedyInformed(gameNode_t* currentNode)
{
	// if no children yet, create one
	if (currentNode->children.empty()) {
		if (!currentNode->moveGenerator.hasMoreActions()) {
			printNodeError("Error creating first child", currentNode);
			return currentNode;
		}
// 		playerActions_t action = currentNode->moveGenerator.getBiasAction();
		playerActions_t action = currentNode->moveGenerator.getMostProbAction();
		return createChild(currentNode, action);

	}

	std::uniform_real_distribution<> uniformDist(0, 1);
	double randomNumber = uniformDist(gen);
    //LOG("Random number: " << randomNumber);
    gameNode_t* best = nullptr;

    if (randomNumber < EPSILON) { // select bias random
		playerActions_t action = currentNode->moveGenerator.getBiasAction();
		// look if node already generated
		for (size_t i = 0; i < currentNode->actions.size(); ++i) {
			if (action == currentNode->actions.at(i)) {
				return currentNode->children.at(i);
			}
		}
		// else, create the new child
		return createChild(currentNode, action);
    } else { // select max reward
        double bestScore = 0;
        double tmpScore;
		for (const auto& child : currentNode->children) {
			tmpScore = child->totalEvaluation / child->totalVisits;
			if (currentNode->player == 0) tmpScore = -tmpScore; // if min node, reverse score
			if (best == nullptr || tmpScore > bestScore) {
				best = child;
				bestScore = tmpScore;
			}
		}
    }
        
    return best;
}

MCTSCD::gameNode_t* MCTSCD::eGreedy(gameNode_t* currentNode)
{
	// if no children yet, create one
	if (currentNode->children.empty()) {
		if (!currentNode->moveGenerator.hasMoreActions()) {
			printNodeError("Error creating first child", currentNode);
			return currentNode;
		}
		playerActions_t action = currentNode->moveGenerator.getUniqueRandomAction();
		return createChild(currentNode, action);

	}

	gameNode_t* best = nullptr;

	std::uniform_real_distribution<> uniformDist(0, 1);
	double randomNumber = uniformDist(gen);

	if (randomNumber < EPSILON) { 
		// select random child
		// -------------------------
		unsigned int totalChildren;
		double maxUnsignedInt = std::numeric_limits<unsigned int>::max();
		if (currentNode->moveGenerator._size > maxUnsignedInt) totalChildren = (unsigned int)maxUnsignedInt;
		else totalChildren = (unsigned int)currentNode->moveGenerator._size;

		unsigned int createdChildren = currentNode->children.size();
		std::uniform_int_distribution<int> uniformDist(0, totalChildren - 1);
		int randomChoice = uniformDist(gen);

		//if (randomChoice > createdChildren) { // create a new child if random choice is outside created children
		if (totalChildren > createdChildren) { // create a new child if we still have children to create
			playerActions_t action = currentNode->moveGenerator.getUniqueRandomAction();
			return createChild(currentNode, action);
		} else { // pick one of the created children
			best = currentNode->children.at(randomChoice);
		}
	} else { 
		// select max reward child
		// -------------------------
		double bestScore = 0;
		double tmpScore;
		for (const auto& child : currentNode->children) {
			tmpScore = child->totalEvaluation / child->totalVisits;
			if (currentNode->player == 0) tmpScore = -tmpScore; // if min node, reverse score
			if (best == nullptr || tmpScore > bestScore) {
				best = child;
				bestScore = tmpScore;
			}
		}
	}

	return best;
}

MCTSCD::gameNode_t* MCTSCD::UCB(gameNode_t* currentNode)
{
    // WARNING if branching factor too high we will stuck at this depth
    // if non visited children, visit
    if ( currentNode->moveGenerator.hasMoreActions() ) {
		playerActions_t action = currentNode->moveGenerator.getUniqueRandomAction();
		return createChild(currentNode, action);
    }

    double bestScore = 0;
    double tmpScore;
    gameNode_t* best = nullptr;
	for (const auto& child : currentNode->children) {
		tmpScore = nodeValue(child);
		if (best == nullptr || tmpScore > bestScore) {
			best = child;
			bestScore = tmpScore;
		}
	}

    return best;
}

double MCTSCD::nodeValue(MCTSCD::gameNode_t* node)
{
    double exploitation = node->totalEvaluation / node->totalVisits;
    double exploration = sqrt(log(node->parent->totalVisits/node->totalVisits));
	if (node->parent->player == 1) { // max node:
        //exploitation = (exploitation + evaluation_bound)/(2*evaluation_bound);
    } else {
        //exploitation = - (exploitation - evaluation_bound)/(2*evaluation_bound);
        exploitation = - exploitation;
    }

    double tmp = UCB_C*exploitation + exploration;
    return tmp;
}

MCTSCD::gameNode_t* MCTSCD::PUCB(gameNode_t* currentNode)
{
	// if no children yet, create them
	if (currentNode->children.empty()) {
// 		if (currentNode->moveGenerator._size > 10000) LOG("Creating " << currentNode->moveGenerator._size << " nodes");
		double prob;
		while (currentNode->moveGenerator.hasMoreActions()) {
			playerActions_t action = currentNode->moveGenerator.getNextActionProbability(prob);
			gameNode_t* child = createChild(currentNode, action);
			child->prob = prob;
		}
// 		if (currentNode->moveGenerator._size > 10000) LOG("Done.");
	}

	double bestScore = 0;
	double tmpScore;
	gameNode_t* best = nullptr;
// 	LOG("NEW NODE");
	for (const auto& child : currentNode->children) {
		// compute score
		double avgReward = 1.0;
		double upperBound = 0.0;
		if (child->totalVisits > 0) {
			avgReward = child->totalEvaluation / child->totalVisits;
			if (child->parent->player == 0) avgReward = -avgReward; // min node
			// since our reward is form -1 to 1, we need to normalize to 0 to 1
			avgReward = (avgReward + 1) / 2.0;
			upperBound = sqrt((3 * log(child->parent->totalVisits)) / (2 * child->totalVisits));
		}
		// TODO the following equation is wrong, should be 2 / (sqrt(P)/sum(sqrt(P))
		double bonusPenalty = 2.0 / child->prob;
		if (child->parent->totalVisits > 1) {
			bonusPenalty *= sqrt(log(child->parent->totalVisits) / child->parent->totalVisits);
		}

		tmpScore = avgReward + upperBound - bonusPenalty;
// 		LOG("Score: " << avgReward << " + " << upperBound << " - " << bonusPenalty << " = " << tmpScore);

		if (best == nullptr || tmpScore > bestScore) {
			best = child;
			bestScore = tmpScore;
		}
	}

	return best;
}